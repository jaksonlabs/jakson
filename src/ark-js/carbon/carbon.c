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

#include <inttypes.h>

#include <ark-js/shared/stdx/varuint.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-object-it.h>
#include <ark-js/carbon/carbon-printers.h>
#include <ark-js/carbon/carbon-int.h>
#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-find.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-revise.h>
#include <ark-js/carbon/carbon-string.h>
#include <ark-js/carbon/carbon-key.h>
#include <ark-js/carbon/carbon-revision.h>
#include <ark-js/carbon/printers/json-printers.h>

#define MIN_DOC_CAPACITY 17 /* minimum number of bytes required to store header and empty document array */

static bool printer_drop(struct printer *printer);

static bool printer_carbon_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_end(struct printer *printer, struct string *builder);

static bool printer_carbon_header_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_header_contents(struct printer *printer, struct string *builder,
                                           enum carbon_key_type key_type, const void *key, u64 key_length,
                                           u64 rev);

static bool printer_carbon_header_end(struct printer *printer, struct string *builder);

static bool printer_carbon_payload_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_payload_end(struct printer *printer, struct string *builder);

static bool printer_carbon_empty_record(struct printer *printer, struct string *builder);

static bool printer_carbon_array_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_array_end(struct printer *printer, struct string *builder);

static bool printer_carbon_unit_array_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_unit_array_end(struct printer *printer, struct string *builder);

static bool printer_carbon_object_begin(struct printer *printer, struct string *builder);

static bool printer_carbon_object_end(struct printer *printer, struct string *builder);

static bool printer_carbon_null(struct printer *printer, struct string *builder);

static bool printer_carbon_true(struct printer *printer, bool is_null, struct string *builder);

static bool printer_carbon_false(struct printer *printer, bool is_null, struct string *builder);

static bool printer_carbon_comma(struct printer *printer, struct string *builder);

static bool printer_carbon_signed(struct printer *printer, struct string *builder, const i64 *value);

static bool printer_carbon_unsigned(struct printer *printer, struct string *builder, const u64 *value);

static bool printer_carbon_float(struct printer *printer, struct string *builder, const float *value);

static bool printer_carbon_string(struct printer *printer, struct string *builder, const char *value, u64 strlen);

static bool printer_carbon_binary(struct printer *printer, struct string *builder, const struct carbon_binary *binary);

static bool printer_carbon_prop_null(struct printer *printer, struct string *builder,
                                     const char *key_name, u64 key_len);

static bool printer_carbon_prop_true(struct printer *printer, struct string *builder,
                                     const char *key_name, u64 key_len);

static bool printer_carbon_prop_false(struct printer *printer, struct string *builder,
                                      const char *key_name, u64 key_len);

static bool printer_carbon_prop_signed(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const i64 *value);

static bool printer_carbon_prop_unsigned(struct printer *printer, struct string *builder,
                                         const char *key_name, u64 key_len, const u64 *value);

static bool printer_carbon_prop_float(struct printer *printer, struct string *builder,
                                      const char *key_name, u64 key_len, const float *value);

static bool printer_carbon_prop_string(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const char *value, u64 strlen);

static bool printer_carbon_prop_binary(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const struct carbon_binary *binary);

static bool printer_carbon_array_prop_name(struct printer *printer, struct string *builder,
                                           const char *key_name, u64 key_len);

static bool printer_carbon_column_prop_name(struct printer *printer, struct string *builder,
                                            const char *key_name, u64 key_len);

static bool printer_carbon_object_prop_name(struct printer *printer, struct string *builder,
                                            const char *key_name, u64 key_len);

static bool print_array(struct carbon_array_it *it, struct printer *printer, struct string *builder,
        bool is_record_container);

static bool print_object(struct carbon_object_it *it, struct printer *printer, struct string *builder);

static bool print_column(struct carbon_column_it *it, struct printer *printer, struct string *builder);

static bool internal_drop(struct carbon *doc);

static void carbon_header_init(struct carbon *doc, enum carbon_key_type key_type);

// ---------------------------------------------------------------------------------------------------------------------

struct carbon_insert *carbon_create_begin(struct carbon_new *context, struct carbon *doc,
                                          enum carbon_key_type key_type, int mode)
{
        if (likely(context != NULL && doc != NULL)) {
                error_if (mode != CARBON_KEEP && mode != CARBON_SHRINK && mode != CARBON_COMPACT &&
                          mode != CARBON_OPTIMIZE,
                          &doc->err, ARK_ERR_ILLEGALARG);

                success_else_null(error_init(&context->err), &doc->err);
                context->content_it = ark_malloc(sizeof(struct carbon_array_it));
                context->inserter = ark_malloc(sizeof(struct carbon_insert));
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

bool carbon_create_end(struct carbon_new *context)
{
        bool success = true;
        if (likely(context != NULL)) {
                success &= carbon_array_it_insert_end(context->inserter);
                success &= carbon_revise_iterator_close(context->content_it);
                if (context->mode & CARBON_COMPACT) {
                        carbon_revise_pack(&context->revision_context);
                }
                if (context->mode & CARBON_SHRINK) {
                        carbon_revise_shrink(&context->revision_context);
                }
                success &= carbon_revise_end(&context->revision_context) != NULL;
                free(context->content_it);
                free(context->inserter);
                carbon_drop(&context->original);
                if (unlikely(!success)) {
                        error(&context->err, ARK_ERR_CLEANUP);
                        return false;
                } else {
                        return true;
                }
        } else {
                error_print(ARK_ERR_NULLPTR);
                return false;
        }
}

bool carbon_create_empty(struct carbon *doc, enum carbon_key_type key_type)
{
        return carbon_create_empty_ex(doc, key_type, 1024, 1);
}

bool carbon_create_empty_ex(struct carbon *doc, enum carbon_key_type key_type, u64 doc_cap_byte,
                            u64 array_cap_byte)
{
        error_if_null(doc);

        doc_cap_byte = ark_max(MIN_DOC_CAPACITY, doc_cap_byte);

        error_init(&doc->err);
        memblock_create(&doc->memblock, doc_cap_byte);
        memblock_zero_out(doc->memblock);
        memfile_open(&doc->memfile, doc->memblock, READ_WRITE);

        spin_init(&doc->versioning.write_lock);

        doc->versioning.revision_lock = false;
        doc->versioning.is_latest = true;

        carbon_header_init(doc, key_type);
        carbon_int_insert_array(&doc->memfile, array_cap_byte);

        return true;
}

bool carbon_from_json(struct carbon *doc, const char *json, enum carbon_key_type key_type,
                      const void *key, struct err *err)
{
        error_if_null(doc)
        error_if_null(json)

        struct json data;
        struct json_err status;
        struct json_parser parser;

        json_parser_create(&parser);

        if (!(json_parse(&data, &status, &parser, json))) {
                struct string sb;
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

                error_with_details(err, ARK_ERR_JSONPARSEERR, string_cstr(&sb));
                string_drop(&sb);

                return false;
        } else {
                carbon_int_from_json(doc, &data, key_type, key, CARBON_OPTIMIZE);
                json_drop(&data);
                return true;
        }
}

bool carbon_drop(struct carbon *doc)
{
        error_if_null(doc);
        return internal_drop(doc);
}

const void *carbon_raw_data(u64 *len, struct carbon *doc)
{
        if (len && doc) {
                memblock_size(len, doc->memfile.memblock);
                return memblock_raw_data(doc->memfile.memblock);
        } else {
                return NULL;
        }
}

bool carbon_is_up_to_date(struct carbon *doc)
{
        error_if_null(doc);
        return doc->versioning.is_latest;
}

bool carbon_key_get_type(enum carbon_key_type *out, struct carbon *doc)
{
        error_if_null(out)
        error_if_null(doc)
        memfile_save_position(&doc->memfile);
        carbon_key_skip(out, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return true;
}

const void *carbon_key_raw_value(u64 *key_len, enum carbon_key_type *type, struct carbon *doc)
{
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(key_len, type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        return result;
}

bool carbon_key_signed_value(i64 *key, struct carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_signed_type(type))) {
                *key = *((const i64 *) result);
                return true;
        } else {
                error(&doc->err, ARK_ERR_TYPEMISMATCH);
                return false;
        }
}

bool carbon_key_unsigned_value(u64 *key, struct carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(NULL, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_unsigned_type(type))) {
                *key = *((const u64 *) result);
                return true;
        } else {
                error(&doc->err, ARK_ERR_TYPEMISMATCH);
                return false;
        }
}

const char *carbon_key_string_value(u64 *str_len, struct carbon *doc)
{
        enum carbon_key_type type;
        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);
        const void *result = carbon_key_read(str_len, &type, &doc->memfile);
        memfile_restore_position(&doc->memfile);
        if (likely(carbon_key_is_string_type(type))) {
                return result;
        } else {
                error(&doc->err, ARK_ERR_TYPEMISMATCH);
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

bool carbon_clone(struct carbon *clone, struct carbon *doc)
{
        error_if_null(clone);
        error_if_null(doc);
        ark_check_success(memblock_cpy(&clone->memblock, doc->memblock));
        ark_check_success(memfile_open(&clone->memfile, clone->memblock, READ_WRITE));
        ark_check_success(error_init(&clone->err));

        spin_init(&clone->versioning.write_lock);
        clone->versioning.revision_lock = false;
        clone->versioning.is_latest = true;

        return true;
}

bool carbon_revision(u64 *rev, struct carbon *doc)
{
        error_if_null(doc);
        *rev = carbon_int_header_get_rev(doc);
        return true;
}

bool carbon_to_str(struct string *dst, enum carbon_printer_impl printer, struct carbon *doc)
{
        error_if_null(doc);

        struct printer p;
        struct string b;
        enum carbon_key_type key_type;
        u64 key_len;
        u64 rev;

        string_clear(dst);

        memfile_save_position(&doc->memfile);

        ark_zero_memory(&p, sizeof(struct printer));
        string_create(&b);

        carbon_revision(&rev, doc);

        switch (printer) {
                case JSON_EXTENDED:
                        json_extended_printer_create(&p);
                        break;
                case JSON_COMPACT:
                        json_compact_printer_create(&p);
                        break;
                default: error(&doc->err, ARK_ERR_NOTFOUND);
        }

        printer_carbon_begin(&p, &b);
        printer_carbon_header_begin(&p, &b);

        const void *key = carbon_key_raw_value(&key_len, &key_type, doc);
        printer_carbon_header_contents(&p, &b, key_type, key, key_len, rev);

        printer_carbon_header_end(&p, &b);
        printer_carbon_payload_begin(&p, &b);

        struct carbon_array_it it;
        carbon_iterator_open(&it, doc);

        print_array(&it, &p, &b, true);
        carbon_array_it_drop(&it);

        printer_carbon_payload_end(&p, &b);
        printer_carbon_end(&p, &b);

        printer_drop(&p);
        string_add(dst, string_cstr(&b));
        string_drop(&b);

        memfile_restore_position(&doc->memfile);
        return true;
}

const char *carbon_to_json_extended(struct string *dst, struct carbon *doc)
{
        error_if_null(dst)
        error_if_null(doc)
        carbon_to_str(dst, JSON_EXTENDED, doc);
        return string_cstr(dst);
}

const char *carbon_to_json_compact(struct string *dst, struct carbon *doc)
{
        error_if_null(dst)
        error_if_null(doc)
        carbon_to_str(dst, JSON_COMPACT, doc);
        return string_cstr(dst);
}

char *carbon_to_json_extended_dup(struct carbon *doc)
{
        struct string sb;
        string_create(&sb);
        char *result = strdup(carbon_to_json_extended(&sb, doc));
        string_drop(&sb);
        return result;
}

char *carbon_to_json_compact_dup(struct carbon *doc)
{
        struct string sb;
        string_create(&sb);
        char *result = strdup(carbon_to_json_compact(&sb, doc));
        string_drop(&sb);
        return result;
}

bool carbon_iterator_open(struct carbon_array_it *it, struct carbon *doc)
{
        error_if_null(it);
        error_if_null(doc);
        offset_t payload_start = carbon_int_payload_after_header(doc);
        carbon_array_it_create(it, &doc->memfile, &doc->err, payload_start);
        carbon_array_it_readonly(it);
        return true;
}

bool carbon_iterator_close(struct carbon_array_it *it)
{
        error_if_null(it);
        return carbon_array_it_drop(it);
}

bool carbon_print(FILE *file, struct carbon *doc)
{
        error_if_null(file);
        error_if_null(doc);

        struct string builder;
        string_create(&builder);
        carbon_to_str(&builder, JSON_EXTENDED, doc);
        printf("%s\n", string_cstr(&builder));
        string_drop(&builder);

        return true;
}

bool carbon_hexdump_print(FILE *file, struct carbon *doc)
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

static bool internal_drop(struct carbon *doc)
{
        assert(doc);
        memblock_drop(doc->memblock);
        return true;
}

static void carbon_header_init(struct carbon *doc, enum carbon_key_type key_type)
{
        assert(doc);

        memfile_seek(&doc->memfile, 0);
        carbon_key_create(&doc->memfile, key_type, &doc->err);

        if (key_type != CARBON_KEY_NOKEY) {
                carbon_revision_create(&doc->memfile);
        }
}

static bool printer_drop(struct printer *printer)
{
        error_if_null(printer->drop);
        printer->drop(printer);
        return true;
}

static bool printer_carbon_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->record_begin);
        printer->record_begin(printer, builder);
        return true;
}

static bool printer_carbon_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->record_end);
        printer->record_end(printer, builder);
        return true;
}

static bool printer_carbon_header_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->meta_begin);
        printer->meta_begin(printer, builder);
        return true;
}

static bool printer_carbon_header_contents(struct printer *printer, struct string *builder,
                                           enum carbon_key_type key_type, const void *key, u64 key_length,
                                           u64 rev)
{
        error_if_null(printer->drop);
        printer->meta_data(printer, builder, key_type, key, key_length, rev);
        return true;
}

static bool printer_carbon_header_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->meta_end);
        printer->meta_end(printer, builder);
        return true;
}

static bool printer_carbon_payload_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->doc_begin);
        printer->doc_begin(printer, builder);
        return true;
}

static bool printer_carbon_payload_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->doc_end);
        printer->doc_end(printer, builder);
        return true;
}

static bool printer_carbon_empty_record(struct printer *printer, struct string *builder)
{
        error_if_null(printer->empty_record);
        printer->empty_record(printer, builder);
        return true;
}

static bool printer_carbon_array_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->array_begin);
        printer->array_begin(printer, builder);
        return true;
}

static bool printer_carbon_array_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->array_end);
        printer->array_end(printer, builder);
        return true;
}

static bool printer_carbon_unit_array_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->unit_array_begin);
        printer->unit_array_begin(printer, builder);
        return true;
}

static bool printer_carbon_unit_array_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->unit_array_end);
        printer->unit_array_end(printer, builder);
        return true;
}

static bool printer_carbon_object_begin(struct printer *printer, struct string *builder)
{
        error_if_null(printer->obj_begin);
        printer->obj_begin(printer, builder);
        return true;
}

static bool printer_carbon_object_end(struct printer *printer, struct string *builder)
{
        error_if_null(printer->obj_end);
        printer->obj_end(printer, builder);
        return true;
}

static bool printer_carbon_null(struct printer *printer, struct string *builder)
{
        error_if_null(printer->const_null);
        printer->const_null(printer, builder);
        return true;
}

static bool printer_carbon_true(struct printer *printer, bool is_null, struct string *builder)
{
        error_if_null(printer->const_true);
        printer->const_true(printer, is_null, builder);
        return true;
}

static bool printer_carbon_false(struct printer *printer, bool is_null, struct string *builder)
{
        error_if_null(printer->const_false);
        printer->const_false(printer, is_null, builder);
        return true;
}

static bool printer_carbon_comma(struct printer *printer, struct string *builder)
{
        error_if_null(printer->comma);
        printer->comma(printer, builder);
        return true;
}

static bool printer_carbon_signed(struct printer *printer, struct string *builder, const i64 *value)
{
        error_if_null(printer->val_signed);
        printer->val_signed(printer, builder, value);
        return true;
}

static bool printer_carbon_unsigned(struct printer *printer, struct string *builder, const u64 *value)
{
        error_if_null(printer->val_unsigned);
        printer->val_unsigned(printer, builder, value);
        return true;
}

static bool printer_carbon_float(struct printer *printer, struct string *builder, const float *value)
{
        error_if_null(printer->val_float);
        printer->val_float(printer, builder, value);
        return true;
}

static bool printer_carbon_string(struct printer *printer, struct string *builder, const char *value, u64 strlen)
{
        error_if_null(printer->val_string);
        printer->val_string(printer, builder, value, strlen);
        return true;
}

static bool printer_carbon_binary(struct printer *printer, struct string *builder, const struct carbon_binary *binary)
{
        error_if_null(printer->val_binary);
        printer->val_binary(printer, builder, binary);
        return true;
}

static bool printer_carbon_prop_null(struct printer *printer, struct string *builder,
                                     const char *key_name, u64 key_len)
{
        error_if_null(printer->prop_null);
        printer->prop_null(printer, builder, key_name, key_len);
        return true;
}

static bool printer_carbon_prop_true(struct printer *printer, struct string *builder,
                                     const char *key_name, u64 key_len)
{
        error_if_null(printer->prop_true);
        printer->prop_true(printer, builder, key_name, key_len);
        return true;
}

static bool printer_carbon_prop_false(struct printer *printer, struct string *builder,
                                      const char *key_name, u64 key_len)
{
        error_if_null(printer->prop_false);
        printer->prop_false(printer, builder, key_name, key_len);
        return true;
}

static bool printer_carbon_prop_signed(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const i64 *value)
{
        error_if_null(printer->prop_signed);
        printer->prop_signed(printer, builder, key_name, key_len, value);
        return true;
}

static bool printer_carbon_prop_unsigned(struct printer *printer, struct string *builder,
                                         const char *key_name, u64 key_len, const u64 *value)
{
        error_if_null(printer->prop_unsigned);
        printer->prop_unsigned(printer, builder, key_name, key_len, value);
        return true;
}

static bool printer_carbon_prop_float(struct printer *printer, struct string *builder,
                                      const char *key_name, u64 key_len, const float *value)
{
        error_if_null(printer->prop_float);
        printer->prop_float(printer, builder, key_name, key_len, value);
        return true;
}

static bool printer_carbon_prop_string(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const char *value, u64 strlen)
{
        error_if_null(printer->prop_string);
        printer->prop_string(printer, builder, key_name, key_len, value, strlen);
        return true;
}

static bool printer_carbon_prop_binary(struct printer *printer, struct string *builder,
                                       const char *key_name, u64 key_len, const struct carbon_binary *binary)
{
        error_if_null(printer->prop_binary);
        printer->prop_binary(printer, builder, key_name, key_len, binary);
        return true;
}

static bool printer_carbon_array_prop_name(struct printer *printer, struct string *builder,
                                           const char *key_name, u64 key_len)
{
        error_if_null(printer->array_prop_name);
        printer->array_prop_name(printer, builder, key_name, key_len);
        return true;
}

static bool printer_carbon_column_prop_name(struct printer *printer, struct string *builder,
                                            const char *key_name, u64 key_len)
{
        error_if_null(printer->column_prop_name);
        printer->column_prop_name(printer, builder, key_name, key_len);
        return true;
}

static bool printer_carbon_object_prop_name(struct printer *printer, struct string *builder,
                                            const char *key_name, u64 key_len)
{
        error_if_null(printer->obj_prop_name);
        printer->obj_prop_name(printer, builder, key_name, key_len);
        return true;
}

static bool print_object(struct carbon_object_it *it, struct printer *printer, struct string *builder)
{
        assert(it);
        assert(printer);
        assert(builder);
        bool is_null_value;
        bool first_entry = true;
        printer_carbon_object_begin(printer, builder);

        while (carbon_object_it_next(it)) {
                if (likely(!first_entry)) {
                        printer_carbon_comma(printer, builder);
                }
                enum carbon_field_type type;
                u64 key_len;
                const char *key_name = carbon_object_it_prop_name(&key_len, it);

                carbon_object_it_prop_type(&type, it);
                switch (type) {
                        case CARBON_FIELD_TYPE_NULL:
                                printer_carbon_prop_null(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_TYPE_TRUE:
                                /* in an array, there is no TRUE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                printer_carbon_prop_true(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_TYPE_FALSE:
                                /* in an array, there is no FALSE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                printer_carbon_prop_false(printer, builder, key_name, key_len);
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_U8:
                        case CARBON_FIELD_TYPE_NUMBER_U16:
                        case CARBON_FIELD_TYPE_NUMBER_U32:
                        case CARBON_FIELD_TYPE_NUMBER_U64: {
                                u64 value;
                                carbon_object_it_unsigned_value(&is_null_value, &value, it);
                                printer_carbon_prop_unsigned(printer, builder, key_name, key_len,
                                                             is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_I8:
                        case CARBON_FIELD_TYPE_NUMBER_I16:
                        case CARBON_FIELD_TYPE_NUMBER_I32:
                        case CARBON_FIELD_TYPE_NUMBER_I64: {
                                i64 value;
                                carbon_object_it_signed_value(&is_null_value, &value, it);
                                printer_carbon_prop_signed(printer, builder, key_name, key_len,
                                                           is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_FLOAT: {
                                float value;
                                carbon_object_it_float_value(&is_null_value, &value, it);
                                printer_carbon_prop_float(printer, builder, key_name, key_len,
                                                          is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_STRING: {
                                u64 strlen;
                                const char *value = carbon_object_it_string_value(&strlen, it);
                                printer_carbon_prop_string(printer, builder, key_name, key_len, value, strlen);
                        }
                                break;
                        case CARBON_FIELD_TYPE_BINARY:
                        case CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                                struct carbon_binary binary;
                                carbon_object_it_binary_value(&binary, it);
                                printer_carbon_prop_binary(printer, builder, key_name, key_len, &binary);
                        }
                                break;
                        case CARBON_FIELD_TYPE_ARRAY: {
                                struct carbon_array_it *array = carbon_object_it_array_value(it);
                                printer_carbon_array_prop_name(printer, builder, key_name, key_len);
                                print_array(array, printer, builder, false);
                                carbon_array_it_drop(array);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U8:
                        case CARBON_FIELD_TYPE_COLUMN_U16:
                        case CARBON_FIELD_TYPE_COLUMN_U32:
                        case CARBON_FIELD_TYPE_COLUMN_U64:
                        case CARBON_FIELD_TYPE_COLUMN_I8:
                        case CARBON_FIELD_TYPE_COLUMN_I16:
                        case CARBON_FIELD_TYPE_COLUMN_I32:
                        case CARBON_FIELD_TYPE_COLUMN_I64:
                        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                struct carbon_column_it *column = carbon_object_it_column_value(it);
                                printer_carbon_column_prop_name(printer, builder, key_name, key_len);
                                print_column(column, printer, builder);
                        }
                                break;
                        case CARBON_FIELD_TYPE_OBJECT: {
                                struct carbon_object_it *object = carbon_object_it_object_value(it);
                                printer_carbon_object_prop_name(printer, builder, key_name, key_len);
                                print_object(object, printer, builder);
                                carbon_object_it_drop(object);
                        }
                                break;
                        default:
                                printer_carbon_object_end(printer, builder);
                                error(&it->err, ARK_ERR_CORRUPTED);
                                return false;
                }
                first_entry = false;
        }

        printer_carbon_object_end(printer, builder);
        return true;
}


static bool print_array(struct carbon_array_it *it, struct printer *printer, struct string *builder,
        bool is_record_container)
{
        assert(it);
        assert(printer);
        assert(builder);

        bool first_entry = true;
        bool has_entries = false;
        bool is_single_entry_array = carbon_array_it_is_unit(it);

        while (carbon_array_it_next(it)) {
                bool is_null_value;

                if (likely(!first_entry)) {
                        printer_carbon_comma(printer, builder);
                } else {
                        if (is_single_entry_array) {
                                printer_carbon_unit_array_begin(printer, builder);
                        } else {
                                printer_carbon_array_begin(printer, builder);
                        }
                        has_entries = true;
                }
                enum carbon_field_type type;
                carbon_array_it_field_type(&type, it);
                switch (type) {
                        case CARBON_FIELD_TYPE_NULL:
                                printer_carbon_null(printer, builder);
                                break;
                        case CARBON_FIELD_TYPE_TRUE:
                                /* in an array, there is no TRUE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                printer_carbon_true(printer, false, builder);
                                break;
                        case CARBON_FIELD_TYPE_FALSE:
                                /* in an array, there is no FALSE constant that is set to NULL because it will be replaced with
                                 * a constant NULL. In columns, there might be a NULL-encoded value */
                                printer_carbon_false(printer, false, builder);
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_U8:
                        case CARBON_FIELD_TYPE_NUMBER_U16:
                        case CARBON_FIELD_TYPE_NUMBER_U32:
                        case CARBON_FIELD_TYPE_NUMBER_U64: {
                                u64 value;
                                carbon_array_it_unsigned_value(&is_null_value, &value, it);
                                printer_carbon_unsigned(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_I8:
                        case CARBON_FIELD_TYPE_NUMBER_I16:
                        case CARBON_FIELD_TYPE_NUMBER_I32:
                        case CARBON_FIELD_TYPE_NUMBER_I64: {
                                i64 value;
                                carbon_array_it_signed_value(&is_null_value, &value, it);
                                printer_carbon_signed(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_NUMBER_FLOAT: {
                                float value;
                                carbon_array_it_float_value(&is_null_value, &value, it);
                                printer_carbon_float(printer, builder, is_null_value ? NULL : &value);
                        }
                                break;
                        case CARBON_FIELD_TYPE_STRING: {
                                u64 strlen;
                                const char *value = carbon_array_it_string_value(&strlen, it);
                                printer_carbon_string(printer, builder, value, strlen);
                        }
                                break;
                        case CARBON_FIELD_TYPE_BINARY:
                        case CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                                struct carbon_binary binary;
                                carbon_array_it_binary_value(&binary, it);
                                printer_carbon_binary(printer, builder, &binary);
                        }
                                break;
                        case CARBON_FIELD_TYPE_ARRAY: {
                                struct carbon_array_it *array = carbon_array_it_array_value(it);
                                print_array(array, printer, builder, false);
                                carbon_array_it_drop(array);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U8:
                        case CARBON_FIELD_TYPE_COLUMN_U16:
                        case CARBON_FIELD_TYPE_COLUMN_U32:
                        case CARBON_FIELD_TYPE_COLUMN_U64:
                        case CARBON_FIELD_TYPE_COLUMN_I8:
                        case CARBON_FIELD_TYPE_COLUMN_I16:
                        case CARBON_FIELD_TYPE_COLUMN_I32:
                        case CARBON_FIELD_TYPE_COLUMN_I64:
                        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                struct carbon_column_it *column = carbon_array_it_column_value(it);
                                print_column(column, printer, builder);
                        }
                                break;
                        case CARBON_FIELD_TYPE_OBJECT: {
                                struct carbon_object_it *object = carbon_array_it_object_value(it);
                                print_object(object, printer, builder);
                                carbon_object_it_drop(object);
                        }
                                break;
                        default:
                                printer_carbon_array_end(printer, builder);
                                error(&it->err, ARK_ERR_CORRUPTED);
                                return false;
                }
                first_entry = false;
        }

        if (has_entries) {
                if (is_single_entry_array) {
                        printer_carbon_unit_array_end(printer, builder);
                } else {
                        printer_carbon_array_end(printer, builder);
                }
        } else {
                if (is_record_container) {
                        printer_carbon_empty_record(printer, builder);
                } else {
                        printer_carbon_array_begin(printer, builder);
                        printer_carbon_array_end(printer, builder);
                }
        }

        return true;
}

static bool print_column(struct carbon_column_it *it, struct printer *printer, struct string *builder)
{
        error_if_null(it)
        error_if_null(printer)
        error_if_null(builder)

        enum carbon_field_type type;
        u32 nvalues;
        const void *values = carbon_column_it_values(&type, &nvalues, it);

        printer_carbon_array_begin(printer, builder);
        for (u32 i = 0; i < nvalues; i++) {
                switch (type) {
                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                u8 value = ((u8 *) values)[i];
                                if (is_null_boolean(value)) {
                                        printer_carbon_null(printer, builder);
                                } else if (value == CARBON_BOOLEAN_COLUMN_TRUE) {
                                        printer_carbon_true(printer, false, builder);
                                } else {
                                        printer_carbon_false(printer, false, builder);
                                }
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U8: {
                                u64 number = ((u8 *) values)[i];
                                printer_carbon_unsigned(printer, builder, is_null_u8(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U16: {
                                u64 number = ((u16 *) values)[i];
                                printer_carbon_unsigned(printer, builder, is_null_u16(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U32: {
                                u64 number = ((u32 *) values)[i];
                                printer_carbon_unsigned(printer, builder, is_null_u32(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_U64: {
                                u64 number = ((u64 *) values)[i];
                                printer_carbon_unsigned(printer, builder, is_null_u64(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_I8: {
                                i64 number = ((i8 *) values)[i];
                                printer_carbon_signed(printer, builder, is_null_i8(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_I16: {
                                i64 number = ((i16 *) values)[i];
                                printer_carbon_signed(printer, builder, is_null_i16(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_I32: {
                                i64 number = ((i32 *) values)[i];
                                printer_carbon_signed(printer, builder, is_null_i32(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_I64: {
                                i64 number = ((i64 *) values)[i];
                                printer_carbon_signed(printer, builder, is_null_i64(number) ? NULL : &number);
                        }
                                break;
                        case CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                                float number = ((float *) values)[i];
                                printer_carbon_float(printer, builder, is_null_float(number) ? NULL : &number);
                        }
                                break;
                        default:
                                printer_carbon_array_end(printer, builder);
                                error(&it->err, ARK_ERR_CORRUPTED);
                                return false;
                }
                if (i + 1 < nvalues) {
                        printer_carbon_comma(printer, builder);
                }
        }
        printer_carbon_array_end(printer, builder);

        return true;
}