/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <inttypes.h>

#include <jak_uintvar_stream.h>
#include <jak_carbon.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_printers.h>
#include <jak_carbon_int.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_find.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_revise.h>
#include <jak_carbon_string.h>
#include <jak_carbon_key.h>
#include <jak_carbon_commit.h>
#include <jak_json_printer_compact.h>
#include <jak_json_printer_extended.h>

#define MIN_DOC_CAPACITY 17 /* minimum number of bytes required to store header and empty document array */

static bool internal_drop(jak_carbon *doc);

static void carbon_header_init(jak_carbon *doc, jak_carbon_key_e key_type);

// ---------------------------------------------------------------------------------------------------------------------

jak_carbon_insert *jak_carbon_create_begin(jak_carbon_new *context, jak_carbon *doc,
                                           jak_carbon_key_e type, int options)
{
        if (JAK_LIKELY(context != NULL && doc != NULL)) {
                JAK_ERROR_IF (options != JAK_CARBON_KEEP && options != JAK_CARBON_SHRINK && options != JAK_CARBON_COMPACT &&
                          options != JAK_CARBON_OPTIMIZE,
                          &doc->err, JAK_ERR_ILLEGALARG);

                JAK_SUCCESS_ELSE_NULL(jak_error_init(&context->err), &doc->err);
                context->content_it = JAK_MALLOC(sizeof(jak_carbon_array_it));
                context->inserter = JAK_MALLOC(sizeof(jak_carbon_insert));
                context->mode = options;

                JAK_SUCCESS_ELSE_NULL(jak_carbon_create_empty(&context->original, type), &doc->err);
                JAK_SUCCESS_ELSE_NULL(jak_carbon_revise_begin(&context->revision_context, doc, &context->original), &doc->err);
                JAK_SUCCESS_ELSE_NULL(jak_carbon_revise_iterator_open(context->content_it, &context->revision_context),
                                  &doc->err);
                JAK_SUCCESS_ELSE_NULL(jak_carbon_array_it_insert_begin(context->inserter, context->content_it), &doc->err);
                return context->inserter;
        } else {
                return NULL;
        }
}

bool jak_carbon_create_end(jak_carbon_new *context)
{
        bool success = true;
        if (JAK_LIKELY(context != NULL)) {
                success &= jak_carbon_array_it_insert_end(context->inserter);
                success &= jak_carbon_revise_iterator_close(context->content_it);
                if (context->mode & JAK_CARBON_COMPACT) {
                        jak_carbon_revise_pack(&context->revision_context);
                }
                if (context->mode & JAK_CARBON_SHRINK) {
                        jak_carbon_revise_shrink(&context->revision_context);
                }
                success &= jak_carbon_revise_end(&context->revision_context) != NULL;
                free(context->content_it);
                free(context->inserter);
                jak_carbon_drop(&context->original);
                if (JAK_UNLIKELY(!success)) {
                        JAK_ERROR(&context->err, JAK_ERR_CLEANUP);
                        return false;
                } else {
                        return true;
                }
        } else {
                JAK_ERROR_PRINT(JAK_ERR_NULLPTR);
                return false;
        }
}

bool jak_carbon_create_empty(jak_carbon *doc, jak_carbon_key_e type)
{
        return jak_carbon_create_empty_ex(doc, type, 1024, 1);
}

bool jak_carbon_create_empty_ex(jak_carbon *doc, jak_carbon_key_e type, jak_u64 doc_cap,
                                jak_u64 array_cap)
{
        JAK_ERROR_IF_NULL(doc);

        doc_cap = JAK_MAX(MIN_DOC_CAPACITY, doc_cap);

        jak_error_init(&doc->err);
        jak_memblock_create(&doc->memblock, doc_cap);
        jak_memblock_zero_out(doc->memblock);
        jak_memfile_open(&doc->memfile, doc->memblock, JAK_READ_WRITE);

        jak_spinlock_init(&doc->versioning.write_lock);

        doc->versioning.commit_lock = false;
        doc->versioning.is_latest = true;

        carbon_header_init(doc, type);
        jak_carbon_int_insert_array(&doc->memfile, array_cap);

        return true;
}

bool jak_carbon_from_json(jak_carbon *doc, const char *json, jak_carbon_key_e type,
                      const void *key, jak_error *err)
{
        JAK_ERROR_IF_NULL(doc)
        JAK_ERROR_IF_NULL(json)

        jak_json data;
        jak_json_err status;
        jak_json_parser parser;

        jak_json_parser_create(&parser);

        if (!(jak_json_parse(&data, &status, &parser, json))) {
                jak_string sb;
                jak_string_create(&sb);

                if (status.token) {
                        jak_string_add(&sb, status.msg);
                        jak_string_add(&sb, " but token ");
                        jak_string_add(&sb, status.token_type_str);
                        jak_string_add(&sb, " was found in line ");
                        jak_string_add_u32(&sb, status.token->line);
                        jak_string_add(&sb, " column ");
                        jak_string_add_u32(&sb, status.token->column);
                } else {
                        jak_string_add(&sb, status.msg);
                }

                JAK_ERROR_WDETAILS(err, JAK_ERR_JSONPARSEERR, jak_string_cstr(&sb));
                jak_string_drop(&sb);

                return false;
        } else {
                jak_carbon_int_from_json(doc, &data, type, key, JAK_CARBON_OPTIMIZE);
                jak_json_drop(&data);
                return true;
        }
}

bool jak_carbon_from_raw_data(jak_carbon *doc, jak_error *err, const void *data, jak_u64 len)
{
        JAK_ERROR_IF_NULL(doc);
        JAK_ERROR_IF_NULL(err);
        JAK_ERROR_IF_NULL(data);
        JAK_ERROR_IF_NULL(len);

        jak_error_init(&doc->err);
        jak_memblock_from_raw_data(&doc->memblock, data, len);
        jak_memfile_open(&doc->memfile, doc->memblock, JAK_READ_WRITE);

        jak_spinlock_init(&doc->versioning.write_lock);

        doc->versioning.commit_lock = false;
        doc->versioning.is_latest = true;

        return true;
}

bool jak_carbon_drop(jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(doc);
        return internal_drop(doc);
}

const void *jak_carbon_raw_data(jak_u64 *len, jak_carbon *doc)
{
        if (len && doc) {
                jak_memblock_size(len, doc->memfile.memblock);
                return jak_memblock_raw_data(doc->memfile.memblock);
        } else {
                return NULL;
        }
}

bool jak_carbon_is_up_to_date(jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(doc);
        return doc->versioning.is_latest;
}

bool jak_carbon_key_type(jak_carbon_key_e *out, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(doc)
        jak_memfile_save_position(&doc->memfile);
        jak_carbon_key_skip(out, &doc->memfile);
        jak_memfile_restore_position(&doc->memfile);
        return true;
}

const void *jak_carbon_key_raw_value(jak_u64 *len, jak_carbon_key_e *type, jak_carbon *doc)
{
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        const void *result = jak_carbon_key_read(len, type, &doc->memfile);
        jak_memfile_restore_position(&doc->memfile);
        return result;
}

bool jak_carbon_key_signed_value(jak_i64 *key, jak_carbon *doc)
{
        jak_carbon_key_e type;
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        const void *result = jak_carbon_key_read(NULL, &type, &doc->memfile);
        jak_memfile_restore_position(&doc->memfile);
        if (JAK_LIKELY(jak_carbon_key_is_signed(type))) {
                *key = *((const jak_i64 *) result);
                return true;
        } else {
                JAK_ERROR(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

bool jak_carbon_key_unsigned_value(jak_u64 *key, jak_carbon *doc)
{
        jak_carbon_key_e type;
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        const void *result = jak_carbon_key_read(NULL, &type, &doc->memfile);
        jak_memfile_restore_position(&doc->memfile);
        if (JAK_LIKELY(jak_carbon_key_is_unsigned(type))) {
                *key = *((const jak_u64 *) result);
                return true;
        } else {
                JAK_ERROR(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

const char *jak_carbon_key_jak_string_value(jak_u64 *len, jak_carbon *doc)
{
        jak_carbon_key_e type;
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        const void *result = jak_carbon_key_read(len, &type, &doc->memfile);
        jak_memfile_restore_position(&doc->memfile);
        if (JAK_LIKELY(jak_carbon_key_is_string(type))) {
                return result;
        } else {
                JAK_ERROR(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

bool jak_carbon_key_is_unsigned(jak_carbon_key_e type)
{
        return type == JAK_CARBON_KEY_UKEY || type == JAK_CARBON_KEY_AUTOKEY;
}

bool jak_carbon_key_is_signed(jak_carbon_key_e type)
{
        return type == JAK_CARBON_KEY_IKEY;
}

bool jak_carbon_key_is_string(jak_carbon_key_e type)
{
        return type == JAK_CARBON_KEY_SKEY;
}

bool jak_carbon_has_key(jak_carbon_key_e type)
{
        return type != JAK_CARBON_KEY_NOKEY;
}

bool jak_carbon_clone(jak_carbon *clone, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(clone);
        JAK_ERROR_IF_NULL(doc);
        JAK_CHECK_SUCCESS(jak_memblock_cpy(&clone->memblock, doc->memblock));
        JAK_CHECK_SUCCESS(jak_memfile_open(&clone->memfile, clone->memblock, JAK_READ_WRITE));
        JAK_CHECK_SUCCESS(jak_error_init(&clone->err));

        jak_spinlock_init(&clone->versioning.write_lock);
        clone->versioning.commit_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

bool jak_carbon_commit_hash(jak_u64 *hash, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(doc);
        *hash = jak_carbon_int_header_get_commit_hash(doc);
        return true;
}

bool jak_carbon_to_str(jak_string *dst, jak_carbon_printer_impl_e printer, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(doc);

        jak_carbon_printer p;
        jak_string b;
        jak_carbon_key_e key_type;
        jak_u64 key_len;
        jak_u64 rev;

        jak_string_clear(dst);

        jak_memfile_save_position(&doc->memfile);

        JAK_ZERO_MEMORY(&p, sizeof(jak_carbon_printer));
        jak_string_create(&b);

        jak_carbon_commit_hash(&rev, doc);

        jak_carbon_printer_by_type(&p, printer);

        jak_carbon_printer_begin(&p, &b);
        jak_carbon_printer_header_begin(&p, &b);

        const void *key = jak_carbon_key_raw_value(&key_len, &key_type, doc);
        jak_carbon_printer_header_contents(&p, &b, key_type, key, key_len, rev);

        jak_carbon_printer_header_end(&p, &b);
        jak_carbon_printer_payload_begin(&p, &b);

        jak_carbon_array_it it;
        jak_carbon_iterator_open(&it, doc);

        jak_carbon_printer_print_array(&it, &p, &b, true);
        jak_carbon_array_it_drop(&it);

        jak_carbon_printer_payload_end(&p, &b);
        jak_carbon_printer_end(&p, &b);

        jak_carbon_printer_drop(&p);
        jak_string_add(dst, jak_string_cstr(&b));
        jak_string_drop(&b);

        jak_memfile_restore_position(&doc->memfile);
        return true;
}

const char *jak_carbon_to_json_extended(jak_string *dst, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(doc)
        jak_carbon_to_str(dst, JAK_JSON_EXTENDED, doc);
        return jak_string_cstr(dst);
}

const char *jak_carbon_to_json_compact(jak_string *dst, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(doc)
        jak_carbon_to_str(dst, JAK_JSON_COMPACT, doc);
        return jak_string_cstr(dst);
}

char *jak_carbon_to_json_extended_dup(jak_carbon *doc)
{
        jak_string sb;
        jak_string_create(&sb);
        char *result = strdup(jak_carbon_to_json_extended(&sb, doc));
        jak_string_drop(&sb);
        return result;
}

char *jak_carbon_to_json_compact_dup(jak_carbon *doc)
{
        jak_string sb;
        jak_string_create(&sb);
        char *result = strdup(jak_carbon_to_json_compact(&sb, doc));
        jak_string_drop(&sb);
        return result;
}

bool jak_carbon_iterator_open(jak_carbon_array_it *it, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(doc);
        jak_offset_t payload_start = jak_carbon_int_payload_after_header(doc);
        jak_carbon_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        jak_carbon_array_it_readonly(it);
        return true;
}

bool jak_carbon_iterator_close(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        return jak_carbon_array_it_drop(it);
}

bool jak_carbon_print(FILE *file, jak_carbon_printer_impl_e printer, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(doc);

        jak_string builder;
        jak_string_create(&builder);
        jak_carbon_to_str(&builder, printer, doc);
        printf("%s\n", jak_string_cstr(&builder));
        jak_string_drop(&builder);

        return true;
}

bool jak_carbon_hexdump_print(FILE *file, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(doc);
        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);
        bool status = jak_hexdump_print(file, jak_memfile_peek(&doc->memfile, 1), jak_memfile_size(&doc->memfile));
        jak_memfile_restore_position(&doc->memfile);
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

static bool internal_drop(jak_carbon *doc)
{
        JAK_ASSERT(doc);
        jak_memblock_drop(doc->memblock);
        return true;
}

static void carbon_header_init(jak_carbon *doc, jak_carbon_key_e key_type)
{
        JAK_ASSERT(doc);

        jak_memfile_seek(&doc->memfile, 0);
        jak_carbon_key_create(&doc->memfile, key_type, &doc->err);

        if (key_type != JAK_CARBON_KEY_NOKEY) {
                jak_carbon_commit_hash_create(&doc->memfile);
        }
}