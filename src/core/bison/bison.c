/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <inttypes.h>

#include "stdx/varuint.h"

#include "core/bison/bison.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-printers.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-dot.h"
#include "core/bison/bison-find.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-revise.h"
#include "core/bison/bison-string.h"
#include "core/bison/bison-key.h"
#include "core/bison/bison-revision.h"

#define MIN_DOC_CAPACITY 17 /* minimum number of bytes required to store header and empty document array */

static bool printer_drop(struct bison_printer *printer);
static bool printer_bison_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_header_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_header_contents(struct bison_printer *printer, struct string_builder *builder,
        enum bison_primary_key_type key_type, const void *key, u64 key_length, u64 rev);
static bool printer_bison_header_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_payload_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_payload_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_array_begin(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_array_end(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_null(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_true(struct bison_printer *printer, bool is_null, struct string_builder *builder);
static bool printer_bison_false(struct bison_printer *printer, bool is_null, struct string_builder *builder);
static bool printer_bison_comma(struct bison_printer *printer, struct string_builder *builder);
static bool printer_bison_signed(struct bison_printer *printer, struct string_builder *builder, const i64 *value);
static bool printer_bison_unsigned(struct bison_printer *printer, struct string_builder *builder, const u64 *value);
static bool printer_bison_float(struct bison_printer *printer, struct string_builder *builder, const float *value);
static bool printer_bison_string(struct bison_printer *printer, struct string_builder *builder, const char *value, u64 strlen);
static bool printer_bison_binary(struct bison_printer *printer, struct string_builder *builder, const struct bison_binary *binary);

static bool print_array(struct bison_array_it *it, struct bison_printer *printer, struct string_builder *builder);
static bool print_column(struct bison_column_it *it, struct bison_printer *printer, struct string_builder *builder);

static bool internal_drop(struct bison *doc);

static void bison_header_init(struct bison *doc, enum bison_primary_key_type key_type);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(struct bison_insert *) bison_create_begin(struct bison_new *context, struct bison *doc,
        enum bison_primary_key_type key_type, int mode)
{
        if (likely(context != NULL && doc != NULL)) {
                error_if (mode != BISON_KEEP && mode != BISON_SHRINK && mode != BISON_COMPACT && mode != BISON_OPTIMIZE,
                          &doc->err, NG5_ERR_ILLEGALARG);

                success_else_null(error_init(&context->err), &doc->err);
                context->content_it = malloc(sizeof(struct bison_array_it));
                context->inserter = malloc(sizeof(struct bison_insert));
                context->mode = mode;

                success_else_null(bison_create_empty(&context->original, key_type), &doc->err);
                success_else_null(bison_revise_begin(&context->revision_context, doc, &context->original), &doc->err);
                success_else_null(bison_revise_iterator_open(context->content_it, &context->revision_context), &doc->err);
                success_else_null(bison_array_it_insert_begin(context->inserter, context->content_it), &doc->err);
                return context->inserter;
        } else {
                return NULL;
        }
}

NG5_EXPORT(bool) bison_create_end(struct bison_new *context)
{
        bool success = true;
        if (likely(context != NULL)) {
                success &= bison_array_it_insert_end(context->inserter);
                success &= bison_revise_iterator_close(context->content_it);
                if (context->mode & BISON_COMPACT) {
                        bison_revise_pack(&context->revision_context);
                }
                if (context->mode & BISON_SHRINK) {
                        bison_revise_shrink(&context->revision_context);
                }
                success &= bison_revise_end(&context->revision_context) != NULL;
                free (context->content_it);
                free (context->inserter);
                if (unlikely(!success)) {
                        error(&context->err, NG5_ERR_CLEANUP);
                        return false;
                } else {
                        return true;
                }
        } else {
                error_print(NG5_ERR_NULLPTR);
                return false;
        }
}

NG5_EXPORT(bool) bison_create_empty(struct bison *doc, enum bison_primary_key_type key_type)
{
        return bison_create_empty_ex(doc, key_type, 1024, 1);
}

NG5_EXPORT(bool) bison_create_empty_ex(struct bison *doc, enum bison_primary_key_type key_type, u64 doc_cap_byte,
                                       u64 array_cap_byte)
{
        error_if_null(doc);

        doc_cap_byte = ng5_max(MIN_DOC_CAPACITY, doc_cap_byte);

        error_init(&doc->err);
        memblock_create(&doc->memblock, doc_cap_byte);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);
        vec_create(&doc->handler, NULL, sizeof(struct bison_handler), 1);

        spin_init(&doc->versioning.write_lock);

        doc->versioning.revision_lock = false;
        doc->versioning.is_latest = true;

        bison_header_init(doc, key_type);
        bison_int_insert_array(&doc->memfile, array_cap_byte);

        return true;
}

NG5_EXPORT(bool) bison_drop(struct bison *doc)
{
        error_if_null(doc);
        return internal_drop(doc);
}

NG5_EXPORT(bool) bison_is_up_to_date(struct bison *doc)
{
        error_if_null(doc);
        return doc->versioning.is_latest;
}

NG5_EXPORT(bool) bison_key_get_type(enum bison_primary_key_type *out, struct bison *doc)
{
        error_if_null(out)
        error_if_null(doc)
        memfile_save_position(&doc->memfile);
        bison_key_skip(out, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return true;
}

NG5_EXPORT(const void *) bison_key_raw_value(u64 *key_len, enum bison_primary_key_type *type, struct bison *doc)
{
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = bison_key_read(key_len, type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return result;
}

NG5_EXPORT(bool) bison_key_signed_value(i64 *key, struct bison *doc)
{
        enum bison_primary_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = bison_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(bison_key_is_signed_type(type))) {
                *key = *((const i64 *) result);
                return true;
        } else {
                error(&doc->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
}

NG5_EXPORT(bool) bison_key_unsigned_value(u64 *key, struct bison *doc)
{
        enum bison_primary_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = bison_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(bison_key_is_unsigned_type(type))) {
                *key = *((const u64 *) result);
                return true;
        } else {
                error(&doc->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
}

NG5_EXPORT(const char *) bison_key_string_value(u64 *str_len, struct bison *doc)
{
        enum bison_primary_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = bison_key_read(str_len, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(bison_key_is_string_type(type))) {
                return result;
        } else {
                error(&doc->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
}

NG5_EXPORT(bool) bison_key_is_unsigned_type(enum bison_primary_key_type type)
{
        return type == BISON_KEY_UKEY || type == BISON_KEY_AUTOKEY;
}

NG5_EXPORT(bool) bison_key_is_signed_type(enum bison_primary_key_type type)
{
        return type == BISON_KEY_IKEY;
}

NG5_EXPORT(bool) bison_key_is_string_type(enum bison_primary_key_type type)
{
        return type == BISON_KEY_SKEY;
}

NG5_EXPORT(bool) bison_has_key(enum bison_primary_key_type type)
{
        return type != BISON_KEY_NOKEY;
}

NG5_EXPORT(bool) bison_register_listener(listener_handle_t *handle, struct bison_event_listener *listener, struct bison *doc)
{
        error_if_null(listener);
        error_if_null(doc);

        u32 pos = 0;
        struct bison_handler *handler;

        for (u32 pos = 0; pos < doc->handler.num_elems; pos++) {
                handler = vec_get(&doc->handler, pos, struct bison_handler);
                if (!handler->in_use) {
                        break;
                }
        }

        if (pos == doc->handler.num_elems) {
                /* append new handler */
                handler = vec_new_and_get(&doc->handler, struct bison_handler);
        }

        handler->in_use = true;
        handler->listener = *listener;
        ng5_optional_call(listener, clone, &handler->listener, listener);
        ng5_optional_set(handle, pos);
        return true;
}

NG5_EXPORT(bool) bison_unregister_listener(struct bison *doc, listener_handle_t handle)
{
        error_if_null(doc);
        if (likely(handle < doc->handler.num_elems)) {
                struct bison_handler *handler = vec_get(&doc->handler, handle, struct bison_handler);
                if (likely(handler->in_use)) {
                        handler->in_use = false;
                        ng5_optional_call(&handler->listener, drop, &handler->listener);
                        return true;
                } else {
                        error(&doc->err, NG5_ERR_NOTFOUND);
                        return false;
                }
        } else {
                error(&doc->err, NG5_ERR_OUTOFBOUNDS);
                return false;
        }
}

NG5_EXPORT(bool) bison_clone(struct bison *clone, struct bison *doc)
{
        error_if_null(clone);
        error_if_null(doc);
        ng5_check_success(memblock_cpy(&clone->memblock, doc->memblock));
        ng5_check_success(memfile_open(&clone->memfile, clone->memblock, READ_WRITE));
        ng5_check_success(error_init(&clone->err));

        vec_create(&clone->handler, NULL, sizeof(struct bison_handler), doc->handler.num_elems);
        for (u32 i = 0; i < doc->handler.num_elems; i++) {
                struct bison_handler *original = vec_get(&doc->handler, i, struct bison_handler);
                struct bison_handler *copy = vec_new_and_get(&clone->handler, struct bison_handler);
                copy->in_use = original->in_use;
                if (original->in_use) {
                        copy->listener = original->listener;
                        ng5_optional_call(&original->listener, clone, &copy->listener, &original->listener);
                }
        }

        spin_init(&clone->versioning.write_lock);
        clone->versioning.revision_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

NG5_EXPORT(bool) bison_revision(u64 *rev, struct bison *doc)
{
        error_if_null(doc);
        *rev = bison_int_header_get_rev(doc);
        return true;
}

NG5_EXPORT(bool) bison_to_str(struct string_builder *dst, enum bison_printer_impl printer, struct bison *doc)
{
        error_if_null(doc);

        struct bison_printer p;
        struct string_builder b;
        enum bison_primary_key_type key_type;
        u64 key_len;
        u64 rev;

        string_builder_clear(dst);

        memfile_save_position(&doc->memfile);

        ng5_zero_memory(&p, sizeof(struct bison_printer));
        string_builder_create(&b);

        bison_revision(&rev, doc);

        switch (printer) {
        case JSON_FORMATTER:
                bison_json_formatter_create(&p);
                break;
        default:
                error(&doc->err, NG5_ERR_NOTFOUND);
        }

        printer_bison_begin(&p, &b);
        printer_bison_header_begin(&p, &b);

        const void *key = bison_key_raw_value(&key_len, &key_type, doc);
        printer_bison_header_contents(&p, &b, key_type, key, key_len, rev);

        printer_bison_header_end(&p, &b);
        printer_bison_payload_begin(&p, &b);

        struct bison_array_it it;
        bison_iterator_open(&it, doc);

        print_array(&it, &p, &b);
        bison_array_it_drop(&it);

        printer_bison_payload_end(&p, &b);
        printer_bison_end(&p, &b);

        printer_drop(&p);
        string_builder_append(dst, string_builder_cstr(&b));
        string_builder_drop(&b);

        memfile_restore_position(&doc->memfile);
        return true;
}

NG5_EXPORT(const char *) bison_to_json(struct string_builder *dst, struct bison *doc)
{
        error_if_null(dst)
        error_if_null(doc)
        bison_to_str(dst, JSON_FORMATTER, doc);
        return string_builder_cstr(dst);
}

NG5_EXPORT(bool) bison_iterator_open(struct bison_array_it *it, struct bison *doc)
{
        error_if_null(it);
        error_if_null(doc);
        offset_t payload_start = bison_int_payload_after_header(doc);
        bison_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        bison_array_it_readonly(it);
        return true;
}

NG5_EXPORT(bool) bison_iterator_close(struct bison_array_it *it)
{
        error_if_null(it);
        return bison_array_it_drop(it);
}

NG5_EXPORT(bool) bison_print(FILE *file, struct bison *doc)
{
        error_if_null(file);
        error_if_null(doc);

        struct string_builder builder;
        string_builder_create(&builder);
        bison_to_str(&builder, JSON_FORMATTER, doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        return true;
}

NG5_EXPORT(bool) bison_hexdump_print(FILE *file, struct bison *doc)
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

static bool internal_drop(struct bison *doc)
{
        assert(doc);
        memblock_drop(doc->memblock);
        for (u32 i = 0; i < doc->handler.num_elems; i++) {
                struct bison_handler *handler = vec_get(&doc->handler, i, struct bison_handler);
                if (handler->in_use) {
                        ng5_optional_call(&handler->listener, drop, &handler->listener);
                }

        }
        vec_drop(&doc->handler);
        return true;
}

static void bison_header_init(struct bison *doc, enum bison_primary_key_type key_type)
{
        assert(doc);

        memfile_seek(&doc->memfile, 0);
        bison_key_create(&doc->memfile, key_type, &doc->err);

        if (key_type != BISON_KEY_NOKEY) {
                bison_revision_create(&doc->memfile);
        }
}

static bool printer_drop(struct bison_printer *printer)
{
        error_if_null(printer->drop);
        printer->drop(printer);
        return true;
}

static bool printer_bison_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_begin);
        printer->print_bison_begin(printer, builder);
        return true;
}

static bool printer_bison_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_end);
        printer->print_bison_end(printer, builder);
        return true;
}

static bool printer_bison_header_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_header_begin);
        printer->print_bison_header_begin(printer, builder);
        return true;
}

static bool printer_bison_header_contents(struct bison_printer *printer, struct string_builder *builder,
        enum bison_primary_key_type key_type, const void *key, u64 key_length, u64 rev)
{
        error_if_null(printer->drop);
        printer->print_bison_header_contents(printer, builder, key_type, key, key_length, rev);
        return true;
}

static bool printer_bison_header_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_header_end);
        printer->print_bison_header_end(printer, builder);
        return true;
}

static bool printer_bison_payload_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_payload_begin);
        printer->print_bison_payload_begin(printer, builder);
        return true;
}

static bool printer_bison_payload_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_payload_end);
        printer->print_bison_payload_end(printer, builder);
        return true;
}

static bool printer_bison_array_begin(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_array_begin);
        printer->print_bison_array_begin(printer, builder);
        return true;
}

static bool printer_bison_array_end(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_array_end);
        printer->print_bison_array_end(printer, builder);
        return true;
}

static bool printer_bison_null(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_null);
        printer->print_bison_null(printer, builder);
        return true;
}

static bool printer_bison_true(struct bison_printer *printer, bool is_null, struct string_builder *builder)
{
        error_if_null(printer->print_bison_true);
        printer->print_bison_true(printer, is_null, builder);
        return true;
}

static bool printer_bison_false(struct bison_printer *printer, bool is_null, struct string_builder *builder)
{
        error_if_null(printer->print_bison_false);
        printer->print_bison_false(printer, is_null, builder);
        return true;
}

static bool printer_bison_comma(struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(printer->print_bison_comma);
        printer->print_bison_comma(printer, builder);
        return true;
}

static bool printer_bison_signed(struct bison_printer *printer, struct string_builder *builder, const i64 *value)
{
        error_if_null(printer->print_bison_signed);
        printer->print_bison_signed(printer, builder, value);
        return true;
}

static bool printer_bison_unsigned(struct bison_printer *printer, struct string_builder *builder, const u64 *value)
{
        error_if_null(printer->print_bison_unsigned);
        printer->print_bison_unsigned(printer, builder, value);
        return true;
}

static bool printer_bison_float(struct bison_printer *printer, struct string_builder *builder, const float *value)
{
        error_if_null(printer->print_bison_float);
        printer->print_bison_float(printer, builder, value);
        return true;
}

static bool printer_bison_string(struct bison_printer *printer, struct string_builder *builder, const char *value, u64 strlen)
{
        error_if_null(printer->print_bison_string);
        printer->print_bison_string(printer, builder, value, strlen);
        return true;
}

static bool printer_bison_binary(struct bison_printer *printer, struct string_builder *builder, const struct bison_binary *binary)
{
        error_if_null(printer->print_bison_binary);
        printer->print_bison_binary(printer, builder, binary);
        return true;
}

static bool print_array(struct bison_array_it *it, struct bison_printer *printer, struct string_builder *builder)
{
        assert(it);
        assert(printer);
        assert(builder);
        bool is_null_value;
        bool first_entry = true;
        printer_bison_array_begin(printer, builder);

        while (bison_array_it_next(it)) {
                if (likely(!first_entry)) {
                        printer_bison_comma(printer, builder);
                }
                enum bison_field_type type;
                bison_array_it_field_type(&type, it);
                switch (type) {
                case BISON_FIELD_TYPE_NULL:
                        printer_bison_null(printer, builder);
                        break;
                case BISON_FIELD_TYPE_TRUE:
                        /* in an array, there is no TRUE constant that is set to NULL because it will be replaced with
                         * a constant NULL. In columns, there might be a NULL-encoded value */
                        printer_bison_true(printer, false, builder);
                        break;
                case BISON_FIELD_TYPE_FALSE:
                        /* in an array, there is no FALSE constant that is set to NULL because it will be replaced with
                         * a constant NULL. In columns, there might be a NULL-encoded value */
                        printer_bison_false(printer, false, builder);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8:
                case BISON_FIELD_TYPE_NUMBER_U16:
                case BISON_FIELD_TYPE_NUMBER_U32:
                case BISON_FIELD_TYPE_NUMBER_U64: {
                        u64 value;
                        bison_array_it_unsigned_value(&is_null_value, &value, it);
                        printer_bison_unsigned(printer, builder, is_null_value ? NULL : &value);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I8:
                case BISON_FIELD_TYPE_NUMBER_I16:
                case BISON_FIELD_TYPE_NUMBER_I32:
                case BISON_FIELD_TYPE_NUMBER_I64: {
                        i64 value;
                        bison_array_it_signed_value(&is_null_value, &value, it);
                        printer_bison_signed(printer, builder, is_null_value ? NULL : &value);
                } break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT: {
                        float value;
                        bison_array_it_float_value(&is_null_value, &value, it);
                        printer_bison_float(printer, builder, is_null_value ? NULL : &value);
                } break;
                case BISON_FIELD_TYPE_STRING: {
                        u64 strlen;
                        const char *value = bison_array_it_string_value(&strlen, it);
                        printer_bison_string(printer, builder, value, strlen);
                } break;
                case BISON_FIELD_TYPE_BINARY:
                case BISON_FIELD_TYPE_BINARY_CUSTOM: {
                        struct bison_binary binary;
                        bison_array_it_binary_value(&binary, it);
                        printer_bison_binary(printer, builder, &binary);
                } break;
                case BISON_FIELD_TYPE_ARRAY: {
                        struct bison_array_it *array = bison_array_it_array_value(it);
                        print_array(array, printer, builder);
                        bison_array_it_drop(array);
                } break;
                case BISON_FIELD_TYPE_COLUMN: {
                        struct bison_column_it *column = bison_array_it_column_value(it);
                        print_column(column, printer, builder);
                } break;
                case BISON_FIELD_TYPE_OBJECT:
                default:
                        printer_bison_array_end(printer, builder);
                        error(&it->err, NG5_ERR_CORRUPTED);
                        return false;
                }
                first_entry = false;
        }

        printer_bison_array_end(printer, builder);
        return true;
}

static bool print_column(struct bison_column_it *it, struct bison_printer *printer, struct string_builder *builder)
{
        error_if_null(it)
        error_if_null(printer)
        error_if_null(builder)

        enum bison_field_type type;
        u32 nvalues;
        const void *values = bison_column_it_values(&type, &nvalues, it);

        printer_bison_array_begin(printer, builder);
        for (u32 i = 0; i < nvalues; i++) {
                switch (type) {
                case BISON_FIELD_TYPE_NULL:
                        printer_bison_null(printer, builder);
                        break;
                case BISON_FIELD_TYPE_TRUE: {
                        u8 value = ((u8*) values)[i];
                        printer_bison_true(printer, is_null_boolean(value), builder);
                } break;
                case BISON_FIELD_TYPE_FALSE: {
                        u8 value = ((u8*) values)[i];
                        printer_bison_false(printer, is_null_boolean(value), builder);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U8: {
                        u64 number = ((u8*) values)[i];
                        printer_bison_unsigned(printer, builder, is_null_u8(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U16: {
                        u64 number = ((u16*) values)[i];
                        printer_bison_unsigned(printer, builder, is_null_u16(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U32: {
                        u64 number = ((u32*) values)[i];
                        printer_bison_unsigned(printer, builder, is_null_u32(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_U64: {
                        u64 number = ((u64*) values)[i];
                        printer_bison_unsigned(printer, builder, is_null_u64(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I8: {
                        i64 number = ((i8*) values)[i];
                        printer_bison_signed(printer, builder, is_null_i8(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I16: {
                        i64 number = ((i16*) values)[i];
                        printer_bison_signed(printer, builder, is_null_i16(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I32: {
                        i64 number = ((i32*) values)[i];
                        printer_bison_signed(printer, builder, is_null_i32(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_I64: {
                        i64 number = ((i64*) values)[i];
                        printer_bison_signed(printer, builder, is_null_i64(number) ? NULL : &number);
                } break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT: {
                        float number = ((float*) values)[i];
                        printer_bison_float(printer, builder, is_null_float(number) ? NULL : &number);
                } break;
                default:
                        printer_bison_array_end(printer, builder);
                        error(&it->err, NG5_ERR_CORRUPTED);
                        return false;
                }
                if (i + 1 < nvalues) {
                        printer_bison_comma(printer, builder);
                }
        }
        printer_bison_array_end(printer, builder);

        return true;
}