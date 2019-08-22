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
#include <jak_json_printer.h>

#define MIN_DOC_CAPACITY 17 /* minimum number of bytes required to store header and empty document array */

static bool internal_drop(struct jak_carbon *doc);

static void carbon_header_init(struct jak_carbon *doc, enum carbon_key_type key_type);

// ---------------------------------------------------------------------------------------------------------------------

struct jak_carbon_insert *carbon_create_begin(struct jak_carbon_new *context, struct jak_carbon *doc,
                                          enum carbon_key_type key_type, int mode)
{
        if (likely(context != NULL && doc != NULL)) {
                error_if (mode != JAK_CARBON_KEEP && mode != JAK_CARBON_SHRINK && mode != JAK_CARBON_COMPACT &&
                          mode != JAK_CARBON_OPTIMIZE,
                          &doc->err, JAK_ERR_ILLEGALARG);

                success_else_null(error_init(&context->err), &doc->err);
                context->content_it = JAK_MALLOC(sizeof(struct jak_carbon_array_it));
                context->inserter = JAK_MALLOC(sizeof(struct jak_carbon_insert));
                context->mode = mode;

                success_else_null(carbon_create_empty(&context->original, key_type), &doc->err);
                success_else_null(carbon_revise_begin(&context->revision_context, doc, &context->original), &doc->err);
                success_else_null(carbon_revise_iterator_open(context->content_it, &context->revision_context),
                                  &doc->err);
                success_else_null(carbon_array_it_insert_begin(context->inserter, context->content_it), &doc->err);
                return context->inserter;
        } else {
                return NULL;
        }
}

bool carbon_create_end(struct jak_carbon_new *context)
{
        bool success = true;
        if (likely(context != NULL)) {
                success &= carbon_array_it_insert_end(context->inserter);
                success &= carbon_revise_iterator_close(context->content_it);
                if (context->mode & JAK_CARBON_COMPACT) {
                        carbon_revise_pack(&context->revision_context);
                }
                if (context->mode & JAK_CARBON_SHRINK) {
                        carbon_revise_shrink(&context->revision_context);
                }
                success &= carbon_revise_end(&context->revision_context) != NULL;
                free(context->content_it);
                free(context->inserter);
                carbon_drop(&context->original);
                if (unlikely(!success)) {
                        error(&context->err, JAK_ERR_CLEANUP);
                        return false;
                } else {
                        return true;
                }
        } else {
                error_print(JAK_ERR_NULLPTR);
                return false;
        }
}

bool carbon_create_empty(struct jak_carbon *doc, enum carbon_key_type key_type)
{
        return carbon_create_empty_ex(doc, key_type, 1024, 1);
}

bool carbon_create_empty_ex(struct jak_carbon *doc, enum carbon_key_type key_type, jak_u64 doc_cap_byte,
                            jak_u64 array_cap_byte)
{
        error_if_null(doc);

        doc_cap_byte = JAK_max(MIN_DOC_CAPACITY, doc_cap_byte);

        error_init(&doc->err);
        memblock_create(&doc->memblock, doc_cap_byte);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);

        spin_init(&doc->versioning.write_lock);

        doc->versioning.commit_lock = false;
        doc->versioning.is_latest = true;

        carbon_header_init(doc, key_type);
        carbon_int_insert_array(&doc->memfile, array_cap_byte);

        return true;
}

bool carbon_from_json(struct jak_carbon *doc, const char *json, enum carbon_key_type key_type,
                      const void *key, struct jak_error *err)
{
        error_if_null(doc)
        error_if_null(json)

        struct jak_json data;
        struct jak_json_err status;
        struct jak_json_parser parser;

        json_parser_create(&parser);

        if (!(json_parse(&data, &status, &parser, json))) {
                struct jak_string sb;
                string_create(&sb);

                if (status.token) {
                        string_add(&sb, status.msg);
                        string_add(&sb, " but token ");
                        string_add(&sb, status.token_type_str);
                        string_add(&sb, " was found in line ");
                        string_add_u32(&sb, status.token->line);
                        string_add(&sb, " column ");
                        string_add_u32(&sb, status.token->column);
                } else {
                        string_add(&sb, status.msg);
                }

                error_with_details(err, JAK_ERR_JSONPARSEERR, string_cstr(&sb));
                string_drop(&sb);

                return false;
        } else {
                carbon_int_from_json(doc, &data, key_type, key, JAK_CARBON_OPTIMIZE);
                json_drop(&data);
                return true;
        }
}

bool carbon_drop(struct jak_carbon *doc)
{
        error_if_null(doc);
        return internal_drop(doc);
}

const void *carbon_raw_data(jak_u64 *len, struct jak_carbon *doc)
{
        if (len && doc) {
                memblock_size(len, doc->memfile.memblock);
                return memblock_raw_data(doc->memfile.memblock);
        } else {
                return NULL;
        }
}

bool carbon_is_up_to_date(struct jak_carbon *doc)
{
        error_if_null(doc);
        return doc->versioning.is_latest;
}

bool carbon_key_type(enum carbon_key_type *out, struct jak_carbon *doc)
{
        error_if_null(out)
        error_if_null(doc)
        memfile_save_position(&doc->memfile);
        carbon_key_skip(out, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return true;
}

const void *carbon_key_raw_value(jak_u64 *key_len, enum carbon_key_type *type, struct jak_carbon *doc)
{
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(key_len, type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return result;
}

bool carbon_key_signed_value(jak_i64 *key, struct jak_carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_signed_type(type))) {
                *key = *((const jak_i64 *) result);
                return true;
        } else {
                error(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

bool carbon_key_unsigned_value(jak_u64 *key, struct jak_carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_unsigned_type(type))) {
                *key = *((const jak_u64 *) result);
                return true;
        } else {
                error(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

const char *carbon_key_string_value(jak_u64 *str_len, struct jak_carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(str_len, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_string_type(type))) {
                return result;
        } else {
                error(&doc->err, JAK_ERR_TYPEMISMATCH);
                return false;
        }
}

bool carbon_key_is_unsigned_type(enum carbon_key_type type)
{
        return type == CARBON_KEY_UKEY || type == CARBON_KEY_AUTOKEY;
}

bool carbon_key_is_signed_type(enum carbon_key_type type)
{
        return type == CARBON_KEY_IKEY;
}

bool carbon_key_is_string_type(enum carbon_key_type type)
{
        return type == CARBON_KEY_SKEY;
}

bool carbon_has_key(enum carbon_key_type type)
{
        return type != CARBON_KEY_NOKEY;
}

bool carbon_clone(struct jak_carbon *clone, struct jak_carbon *doc)
{
        error_if_null(clone);
        error_if_null(doc);
        JAK_check_success(memblock_cpy(&clone->memblock, doc->memblock));
        JAK_check_success(memfile_open(&clone->memfile, clone->memblock, READ_WRITE));
        JAK_check_success(error_init(&clone->err));

        spin_init(&clone->versioning.write_lock);
        clone->versioning.commit_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

bool carbon_commit_hash(jak_u64 *commit_hash, struct jak_carbon *doc)
{
        error_if_null(doc);
        *commit_hash = carbon_int_header_get_commit_hash(doc);
        return true;
}

bool carbon_to_str(struct jak_string *dst, enum carbon_printer_impl printer, struct jak_carbon *doc)
{
        error_if_null(doc);

        struct printer p;
        struct jak_string b;
        enum carbon_key_type key_type;
        jak_u64 key_len;
        jak_u64 rev;

        string_clear(dst);

        memfile_save_position(&doc->memfile);

        JAK_zero_memory(&p, sizeof(struct printer));
        string_create(&b);

        carbon_commit_hash(&rev, doc);

        carbon_printer_by_type(&p, printer);

        carbon_printer_begin(&p, &b);
        carbon_printer_header_begin(&p, &b);

        const void *key = carbon_key_raw_value(&key_len, &key_type, doc);
        carbon_printer_header_contents(&p, &b, key_type, key, key_len, rev);

        carbon_printer_header_end(&p, &b);
        carbon_printer_payload_begin(&p, &b);

        struct jak_carbon_array_it it;
        carbon_iterator_open(&it, doc);

        carbon_printer_print_array(&it, &p, &b, true);
        carbon_array_it_drop(&it);

        carbon_printer_payload_end(&p, &b);
        carbon_printer_end(&p, &b);

        carbon_printer_drop(&p);
        string_add(dst, string_cstr(&b));
        string_drop(&b);

        memfile_restore_position(&doc->memfile);
        return true;
}

const char *carbon_to_json_extended(struct jak_string *dst, struct jak_carbon *doc)
{
        error_if_null(dst)
        error_if_null(doc)
        carbon_to_str(dst, JSON_EXTENDED, doc);
        return string_cstr(dst);
}

const char *carbon_to_json_compact(struct jak_string *dst, struct jak_carbon *doc)
{
        error_if_null(dst)
        error_if_null(doc)
        carbon_to_str(dst, JSON_COMPACT, doc);
        return string_cstr(dst);
}

char *carbon_to_json_extended_dup(struct jak_carbon *doc)
{
        struct jak_string sb;
        string_create(&sb);
        char *result = strdup(carbon_to_json_extended(&sb, doc));
        string_drop(&sb);
        return result;
}

char *carbon_to_json_compact_dup(struct jak_carbon *doc)
{
        struct jak_string sb;
        string_create(&sb);
        char *result = strdup(carbon_to_json_compact(&sb, doc));
        string_drop(&sb);
        return result;
}

bool carbon_iterator_open(struct jak_carbon_array_it *it, struct jak_carbon *doc)
{
        error_if_null(it);
        error_if_null(doc);
        jak_offset_t payload_start = carbon_int_payload_after_header(doc);
        carbon_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        carbon_array_it_readonly(it);
        return true;
}

bool carbon_iterator_close(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        return carbon_array_it_drop(it);
}

bool carbon_print(FILE *file, enum carbon_printer_impl printer, struct jak_carbon *doc)
{
        error_if_null(file);
        error_if_null(doc);

        struct jak_string builder;
        string_create(&builder);
        carbon_to_str(&builder, printer, doc);
        printf("%s\n", string_cstr(&builder));
        string_drop(&builder);

        return true;
}

bool carbon_hexdump_print(FILE *file, struct jak_carbon *doc)
{
        error_if_null(file);
        error_if_null(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        bool status = hexdump_print(file, memfile_peek(&doc->memfile, 1), memfile_size(&doc->memfile));
        memfile_restore_position(&doc->memfile);
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

static bool internal_drop(struct jak_carbon *doc)
{
        assert(doc);
        memblock_drop(doc->memblock);
        return true;
}

static void carbon_header_init(struct jak_carbon *doc, enum carbon_key_type key_type)
{
        assert(doc);

        memfile_seek(&doc->memfile, 0);
        carbon_key_create(&doc->memfile, key_type, &doc->err);

        if (key_type != CARBON_KEY_NOKEY) {
                carbon_commit_hash_create(&doc->memfile);
        }
}