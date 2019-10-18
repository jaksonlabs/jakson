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

#include <jakson/forwdecl.h>
#include <jakson/std/uintvar/stream.h>
#include <jakson/carbon.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/printers.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/dot.h>
#include <jakson/carbon/find.h>
#include <jakson/carbon/insert.h>
#include <jakson/carbon/revise.h>
#include <jakson/carbon/string.h>
#include <jakson/carbon/key.h>
#include <jakson/carbon/commit.h>
#include <jakson/carbon/patch.h>
#include <jakson/carbon/printers/compact.h>
#include <jakson/carbon/printers/extended.h>

#define MIN_DOC_CAPACITY 17 /** minimum number of bytes required to store header and empty document array */

static bool internal_drop(carbon *doc);

static void carbon_header_init(carbon *doc, carbon_key_e key_type);

// ---------------------------------------------------------------------------------------------------------------------

carbon_insert * carbon_create_begin(carbon_new *context, carbon *doc,
                                           carbon_key_e type, int options)
{
        if (context && doc) {
                error_init(&context->err);
                context->content_it = MALLOC(sizeof(carbon_array_it));
                context->inserter = MALLOC(sizeof(carbon_insert));
                context->mode = options;

                /** get the annotation type for that records outer-most array from options*/
                carbon_list_derivable_e derivation;

                if (context->mode & CARBON_SORTED_MULTISET) {
                        derivation = CARBON_LIST_SORTED_MULTISET;
                } else if (context->mode & CARBON_UNSORTED_SET) {
                        derivation = CARBON_LIST_UNSORTED_SET;
                } else if (context->mode & CARBON_SORTED_SET) {
                        derivation = CARBON_LIST_SORTED_SET;
                } else { /** CARBON_UNSORTED_MULTISET is default */
                        derivation = CARBON_LIST_UNSORTED_MULTISET;
                }

                FN_IF_NOT_OK_RETURN(carbon_create_empty(&context->original, derivation, type), NULL);
                FN_IF_NOT_OK_RETURN(carbon_revise_begin(&context->revision_context, doc, &context->original), NULL);
                FN_IF_NOT_OK_RETURN(carbon_revise_iterator_open(context->content_it, &context->revision_context), NULL);
                FN_IF_NOT_OK_RETURN(carbon_array_it_insert_begin(context->inserter, context->content_it), NULL);
                return context->inserter;
        } else {
                return NULL;
        }
}

fn_result carbon_create_end(carbon_new *context)
{
        FN_FAIL_IF_NULL(context);

        fn_result ins_end = carbon_array_it_insert_end(context->inserter);
        fn_result it_close = carbon_revise_iterator_close(context->content_it);
        if (context->mode & CARBON_COMPACT) {
                carbon_revise_pack(&context->revision_context);
        }
        if (context->mode & CARBON_SHRINK) {
                carbon_revise_shrink(&context->revision_context);
        }
        fn_result rev_end = carbon_revise_end(&context->revision_context);
        free(context->content_it);
        free(context->inserter);
        carbon_drop(&context->original);
        if (UNLIKELY(!FN_IS_OK(ins_end) || !FN_IS_OK(it_close) || !FN_IS_OK(rev_end))) {
                return FN_FAIL_FORWARD();
        } else {
                return FN_OK();
        }
}

fn_result carbon_create_empty(carbon *doc, carbon_list_derivable_e derivation, carbon_key_e type)
{
        return carbon_create_empty_ex(doc, derivation, type, 1024, 1);
}

fn_result carbon_create_empty_ex(carbon *doc, carbon_list_derivable_e derivation, carbon_key_e type,
                                u64 doc_cap, u64 array_cap)
{
        FN_FAIL_IF_NULL(doc);

        doc_cap = JAK_MAX(MIN_DOC_CAPACITY, doc_cap);

        error_init(&doc->err);
        memblock_create(&doc->memblock, doc_cap);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);

        spinlock_init(&doc->versioning.write_lock);

        doc->versioning.commit_lock = false;
        doc->versioning.is_latest = true;

        carbon_header_init(doc, type);
        carbon_int_insert_array(&doc->memfile, derivation, array_cap);

        return FN_OK();
}

bool carbon_from_json(carbon *doc, const char *json, carbon_key_e type,
                      const void *key, err *err)
{
        ERROR_IF_NULL(doc)
        ERROR_IF_NULL(json)

        struct json data;
        json_err status;
        json_parser parser;

        json_parser_create(&parser);

        if (!(json_parse(&data, &status, &parser, json))) {
                string_buffer sb;
                string_buffer_create(&sb);

                if (status.token) {
                        string_buffer_add(&sb, status.msg);
                        string_buffer_add(&sb, "in line ");
                        string_buffer_add_u32(&sb, status.token->line);
                        string_buffer_add(&sb, ", column ");
                        string_buffer_add_u32(&sb, status.token->column);
                } else {
                        string_buffer_add(&sb, status.msg);
                }

                ERROR_WDETAILS(err, ERR_JSONPARSEERR, string_cstr(&sb));
                string_buffer_drop(&sb);

                return false;
        } else {
                carbon_int_from_json(doc, &data, type, key, CARBON_OPTIMIZE);
                json_drop(&data);
                return true;
        }
}

bool carbon_from_raw_data(carbon *doc, err *err, const void *data, u64 len)
{
        ERROR_IF_NULL(doc);
        ERROR_IF_NULL(err);
        ERROR_IF_NULL(data);
        ERROR_IF_NULL(len);

        error_init(&doc->err);
        memblock_from_raw_data(&doc->memblock, data, len);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);

        spinlock_init(&doc->versioning.write_lock);

        doc->versioning.commit_lock = false;
        doc->versioning.is_latest = true;

        return true;
}

bool carbon_drop(carbon *doc)
{
        ERROR_IF_NULL(doc);
        return internal_drop(doc);
}

const void *carbon_raw_data(u64 *len, carbon *doc)
{
        if (len && doc) {
                memblock_size(len, doc->memfile.memblock);
                return memblock_raw_data(doc->memfile.memblock);
        } else {
                return NULL;
        }
}

bool carbon_is_up_to_date(carbon *doc)
{
        ERROR_IF_NULL(doc);
        return doc->versioning.is_latest;
}

bool carbon_key_type(carbon_key_e *out, carbon *doc)
{
        ERROR_IF_NULL(out)
        ERROR_IF_NULL(doc)
        memfile_save_position(&doc->memfile);
        carbon_key_skip(out, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return true;
}

const void *carbon_key_raw_value(u64 *len, carbon_key_e *type, carbon *doc)
{
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(len, type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return result;
}

bool carbon_key_signed_value(i64 *key, carbon *doc)
{
        carbon_key_e type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (LIKELY(carbon_key_is_signed(type))) {
                *key = *((const i64 *) result);
                return true;
        } else {
                ERROR(&doc->err, ERR_TYPEMISMATCH);
                return false;
        }
}

bool carbon_key_unsigned_value(u64 *key, carbon *doc)
{
        carbon_key_e type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (LIKELY(carbon_key_is_unsigned(type))) {
                *key = *((const u64 *) result);
                return true;
        } else {
                ERROR(&doc->err, ERR_TYPEMISMATCH);
                return false;
        }
}

const char *carbon_key_string_value(u64 *len, carbon *doc)
{
        carbon_key_e type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(len, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (LIKELY(carbon_key_is_string(type))) {
                return result;
        } else {
                ERROR(&doc->err, ERR_TYPEMISMATCH);
                return false;
        }
}

bool carbon_key_is_unsigned(carbon_key_e type)
{
        return type == CARBON_KEY_UKEY || type == CARBON_KEY_AUTOKEY;
}

bool carbon_key_is_signed(carbon_key_e type)
{
        return type == CARBON_KEY_IKEY;
}

bool carbon_key_is_string(carbon_key_e type)
{
        return type == CARBON_KEY_SKEY;
}

bool carbon_has_key(carbon_key_e type)
{
        return type != CARBON_KEY_NOKEY;
}

bool carbon_clone(carbon *clone, carbon *doc)
{
        ERROR_IF_NULL(clone);
        ERROR_IF_NULL(doc);
        CHECK_SUCCESS(memblock_cpy(&clone->memblock, doc->memblock));
        CHECK_SUCCESS(memfile_open(&clone->memfile, clone->memblock, READ_WRITE));
        CHECK_SUCCESS(error_init(&clone->err));

        spinlock_init(&clone->versioning.write_lock);
        clone->versioning.commit_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

bool carbon_commit_hash(u64 *hash, carbon *doc)
{
        ERROR_IF_NULL(doc);
        *hash = carbon_int_header_get_commit_hash(doc);
        return true;
}

fn_result ofType(bool) carbon_is_multiset(carbon *doc)
{
        FN_FAIL_IF_NULL(doc)

        carbon_array_it it;
        carbon_read_begin(&it, doc);
        fn_result ofType(bool) ret = carbon_array_it_is_multiset(&it);
        carbon_read_end(&it);

        return FN_IS_OK(ret) ? ret : FN_FAIL_FORWARD();
}

fn_result ofType(bool) carbon_is_sorted(carbon *doc)
{
        FN_FAIL_IF_NULL(doc)

        carbon_array_it it;
        carbon_read_begin(&it, doc);
        fn_result ofType(bool) ret = carbon_array_it_is_sorted(&it);
        carbon_read_end(&it);

        return FN_IS_OK(ret) ? ret : FN_FAIL_FORWARD();
}

fn_result carbon_update_list_type(carbon *revised_doc, carbon *doc, carbon_list_derivable_e derivation)
{
        FN_FAIL_IF_NULL(revised_doc, doc);
        carbon_revise context;
        carbon_revise_begin(&context, revised_doc, doc);
        carbon_revise_set_list_type(&context, derivation);
        carbon_revise_end(&context);
        return FN_OK();
}

bool carbon_to_str(string_buffer *dst, carbon_printer_impl_e printer, carbon *doc)
{
        ERROR_IF_NULL(doc);

        carbon_printer p;
        string_buffer b;
        carbon_key_e key_type;
        u64 key_len;
        u64 rev;

        string_buffer_clear(dst);

        memfile_save_position(&doc->memfile);

        ZERO_MEMORY(&p, sizeof(carbon_printer));
        string_buffer_create(&b);

        carbon_commit_hash(&rev, doc);

        carbon_printer_by_type(&p, printer);

        carbon_printer_begin(&p, &b);
        carbon_printer_header_begin(&p, &b);

        const void *key = carbon_key_raw_value(&key_len, &key_type, doc);
        carbon_printer_header_contents(&p, &b, key_type, key, key_len, rev);

        carbon_printer_header_end(&p, &b);
        carbon_printer_payload_begin(&p, &b);

        carbon_array_it it;
        carbon_read_begin(&it, doc);

        carbon_printer_print_array(&it, &p, &b, true);
        carbon_array_it_drop(&it);

        carbon_printer_payload_end(&p, &b);
        carbon_printer_end(&p, &b);

        carbon_printer_drop(&p);
        string_buffer_add(dst, string_cstr(&b));
        string_buffer_drop(&b);

        memfile_restore_position(&doc->memfile);
        return true;
}

const char *carbon_to_json_extended(string_buffer *dst, carbon *doc)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(doc)
        carbon_to_str(dst, JSON_EXTENDED, doc);
        return string_cstr(dst);
}

const char *carbon_to_json_compact(string_buffer *dst, carbon *doc)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(doc)
        carbon_to_str(dst, JSON_COMPACT, doc);
        return string_cstr(dst);
}

char *carbon_to_json_extended_dup(carbon *doc)
{
        string_buffer sb;
        string_buffer_create(&sb);
        char *result = strdup(carbon_to_json_extended(&sb, doc));
        string_buffer_drop(&sb);
        return result;
}

char *carbon_to_json_compact_dup(carbon *doc)
{
        string_buffer sb;
        string_buffer_create(&sb);
        char *result = strdup(carbon_to_json_compact(&sb, doc));
        string_buffer_drop(&sb);
        return result;
}

fn_result carbon_read_begin(carbon_array_it *it, carbon *doc)
{
        fn_result ret = carbon_patch_begin(it, doc);
        carbon_array_it_set_mode(it, READ_ONLY);
        return ret;
}

fn_result carbon_read_end(carbon_array_it *it)
{
        return carbon_patch_end(it);
}

bool carbon_print(FILE *file, carbon_printer_impl_e printer, carbon *doc)
{
        ERROR_IF_NULL(file);
        ERROR_IF_NULL(doc);

        string_buffer builder;
        string_buffer_create(&builder);
        carbon_to_str(&builder, printer, doc);
        printf("%s\n", string_cstr(&builder));
        string_buffer_drop(&builder);

        return true;
}

bool carbon_hexdump_print(FILE *file, carbon *doc)
{
        ERROR_IF_NULL(file);
        ERROR_IF_NULL(doc);
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        bool status = hexdump_print(file, memfile_peek(&doc->memfile, 1), memfile_size(&doc->memfile));
        memfile_restore_position(&doc->memfile);
        return status;
}

// ---------------------------------------------------------------------------------------------------------------------

static bool internal_drop(carbon *doc)
{
        JAK_ASSERT(doc);
        memblock_drop(doc->memblock);
        return true;
}

static void carbon_header_init(carbon *doc, carbon_key_e key_type)
{
        JAK_ASSERT(doc);

        memfile_seek(&doc->memfile, 0);
        carbon_key_create(&doc->memfile, key_type, &doc->err);

        if (key_type != CARBON_KEY_NOKEY) {
                carbon_commit_hash_create(&doc->memfile);
        }
}