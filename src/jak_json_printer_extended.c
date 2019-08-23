/**
 * Copyright 2019 Marcus Pinnecke
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

#include <libs/libb64/libb64.h>
#include <jak_carbon.h>
#include <jak_json_printer_extended.h>
#include <jak_carbon_commit.h>

// ---------------------------------------------------------------------------------------------------------------------

#define INIT_BUFFER_LEN 1024
#define NULL_STR "null"

// ---------------------------------------------------------------------------------------------------------------------

struct jak_json_extended_extra {
        char *buffer;
        size_t buffer_size;
};

// ---------------------------------------------------------------------------------------------------------------------

static void _jak_json_printer_extended_drop(jak_carbon_printer *self)
{
        struct jak_json_extended_extra *extra = (struct jak_json_extended_extra *) self->extra;
        free(extra->buffer);
        free(self->extra);
}

static void _jak_json_printer_extended_obj_begin(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "{");
}

static void _jak_json_printer_extended_obj_end(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "}");
}

static void _jak_json_printer_extended_meta_begin(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "\"meta\": {");
}

static void meta_data(jak_carbon_printer *self, jak_string *builder,
                      int key_type, const void *key,
                      jak_u64 key_length, jak_u64 commit_hash)
{
        JAK_UNUSED(self)

        jak_string_add(builder, "\"key\": {");

        switch (key_type) {
                case JAK_CARBON_KEY_NOKEY:
                        jak_string_add(builder, "\"type\": \"nokey\", \"value\": null");
                        break;
                case JAK_CARBON_KEY_AUTOKEY:
                        jak_string_add(builder, "\"type\": \"autokey\", \"value\": ");
                        jak_string_add_u64(builder, *(jak_u64 *) key);
                        break;
                case JAK_CARBON_KEY_UKEY:
                        jak_string_add(builder, "\"type\": \"ukey\", \"value\": ");
                        jak_string_add_u64(builder, *(jak_u64 *) key);
                        break;
                case JAK_CARBON_KEY_IKEY:
                        jak_string_add(builder, "\"type\": \"ikey\", \"value\": ");
                        jak_string_add_u64(builder, *(jak_i64 *) key);
                        break;
                case JAK_CARBON_KEY_SKEY:
                        jak_string_add(builder, "\"type\": \"skey\", \"value\": ");
                        if (key_length > 0) {
                                jak_string_add(builder, "\"");
                                jak_string_add_nchar(builder, key, key_length);
                                jak_string_add(builder, "\"");
                        } else {
                                jak_string_add(builder, "null");
                        }

                        break;
                default: JAK_ERROR_PRINT(JAK_ERR_INTERNALERR);
        }
        jak_string_add(builder, "}, \"commit\": ");
        if (commit_hash) {
                jak_string_add(builder, "\"");
                jak_carbon_commit_hash_append_to_str(builder, commit_hash);
                jak_string_add(builder, "\"");
        } else {
                jak_string_add(builder, "null");
        }
}

static void _jak_json_printer_extended_meta_end(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "}, ");
}

static void _jak_json_printer_extended_doc_begin(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self)

        jak_string_add(builder, "\"doc\": ");
}

static void _jak_json_printer_extended_doc_end(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        JAK_UNUSED(builder);
}

static void _jak_json_printer_extended_empty_record(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "[]");
}

static void _jak_json_printer_extended_array_begin(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "[");
}

static void _jak_json_printer_extended_array_end(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "]");
}

static void _jak_json_printer_extended_const_null(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, "null");
}

static void _jak_json_printer_extended_const_true(jak_carbon_printer *self, bool is_null, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, is_null ? "null" : "true");
}

static void _jak_json_printer_extended_const_false(jak_carbon_printer *self, bool is_null, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, is_null ? "null" : "false");
}

static void _jak_json_printer_extended_val_signed(jak_carbon_printer *self, jak_string *builder, const jak_i64 *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                jak_string_add_i64(builder, *value);
        } else {
                jak_string_add(builder, NULL_STR);
        }

}

static void _jak_json_printer_extended_val_unsigned(jak_carbon_printer *self, jak_string *builder, const jak_u64 *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                jak_string_add_u64(builder, *value);
        } else {
                jak_string_add(builder, NULL_STR);
        }
}

static void _jak_json_printer_extended_val_float(jak_carbon_printer *self, jak_string *builder, const float *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                jak_string_add_float(builder, *value);
        } else {
                jak_string_add(builder, NULL_STR);
        }
}

static void _jak_json_printer_extended_val_string(jak_carbon_printer *self, jak_string *builder, const char *value, jak_u64 strlen)
{
        JAK_UNUSED(self);
        jak_string_add_char(builder, '"');
        jak_string_add_nchar(builder, value, strlen);
        jak_string_add_char(builder, '"');
}

#define code_of(x, data_len)      (x + data_len + 2)
#define data_of(x)      (x)

static void _jak_json_printer_extended_print_binary(jak_carbon_printer *self, jak_string *builder, const jak_carbon_binary *binary)
{
        /* base64 code will be written into the extra's buffer after a null-terminated copy of the binary data */
        struct jak_json_extended_extra *extra = (struct jak_json_extended_extra *) self->extra;
        /* buffer of at least 2x data length for base64 code + 1x data length to hold the null-terminated value */
        size_t required_buff_size = 3 * (binary->blob_len + 1);
        /* increase buffer capacity if needed */
        if (extra->buffer_size < required_buff_size) {
                extra->buffer_size = required_buff_size * 1.7f;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                JAK_ERROR_PRINT_IF(!extra->buffer, JAK_ERR_REALLOCERR);
        }
        /* decrease buffer capacity if needed */
        if (extra->buffer_size * 0.3f > required_buff_size) {
                extra->buffer_size = required_buff_size;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                JAK_ERROR_PRINT_IF(!extra->buffer, JAK_ERR_REALLOCERR);
        }

        JAK_ASSERT(extra->buffer_size >= required_buff_size);
        JAK_ZERO_MEMORY(extra->buffer, extra->buffer_size);
        /* copy binary data into buffer, and leave one (zero'd) byte free; null-termination is required by libb64 */
        memcpy(data_of(extra->buffer), binary->blob, binary->blob_len);

        jak_string_add(builder, "{ ");
        jak_string_add(builder, "\"type\": \"");
        jak_string_add_nchar(builder, binary->mime_type, binary->mime_type_strlen);
        jak_string_add(builder, "\", \"encoding\": \"base64\", \"binary-string\": \"");

        base64_encodestate state;
        base64_init_encodestate(&state);

        jak_u64 code_len = base64_encode_block(data_of(extra->buffer), binary->blob_len + 2,
                                               code_of(extra->buffer, binary->blob_len), &state);
        base64_encode_blockend(code_of(extra->buffer, binary->blob_len), &state);
        jak_string_add_nchar(builder, code_of(extra->buffer, binary->blob_len), code_len);


        jak_string_add(builder, "\" }");
}

static void _jak_json_printer_extended_val_binary(jak_carbon_printer *self, jak_string *builder, const jak_carbon_binary *binary)
{
        _jak_json_printer_extended_print_binary(self, builder, binary);
}

static void _jak_json_printer_extended_comma(jak_carbon_printer *self, jak_string *builder)
{
        JAK_UNUSED(self);
        jak_string_add(builder, ", ");
}

static void _jak_json_printer_extended_print_key(jak_string *builder, const char *key_name, jak_u64 key_len)
{
        jak_string_add_char(builder, '"');
        jak_string_add_nchar(builder, key_name, key_len);
        jak_string_add(builder, "\": ");
}

static void _jak_json_printer_extended_prop_null(jak_carbon_printer *self, jak_string *builder,
                      const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add(builder, "null");
}

static void _jak_json_printer_extended_prop_true(jak_carbon_printer *self, jak_string *builder,
                      const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add(builder, "true");
}

static void _jak_json_printer_extended_prop_false(jak_carbon_printer *self, jak_string *builder,
                       const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add(builder, "false");
}

static void _jak_json_printer_extended_prop_signed(jak_carbon_printer *self, jak_string *builder,
                        const char *key_name, jak_u64 key_len, const jak_i64 *value)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add_i64(builder, *value);
}

static void _jak_json_printer_extended_prop_unsigned(jak_carbon_printer *self, jak_string *builder,
                          const char *key_name, jak_u64 key_len, const jak_u64 *value)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add_u64(builder, *value);
}

static void _jak_json_printer_extended_prop_float(jak_carbon_printer *self, jak_string *builder,
                       const char *key_name, jak_u64 key_len, const float *value)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add_float(builder, *value);
}

static void _jak_json_printer_extended_prop_string(jak_carbon_printer *self, jak_string *builder,
                        const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen)
{
        JAK_UNUSED(self);
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        jak_string_add_char(builder, '"');
        jak_string_add_nchar(builder, value, strlen);
        jak_string_add_char(builder, '"');
}

static void _jak_json_printer_extended_prop_binary(jak_carbon_printer *self, jak_string *builder,
                        const char *key_name, jak_u64 key_len, const jak_carbon_binary *binary)
{
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
        _jak_json_printer_extended_print_binary(self, builder, binary);
}

static void _jak_json_printer_extended_array_prop_name(jak_carbon_printer *self, jak_string *builder,
                            const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self)
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
}

static void _jak_json_printer_extended_obj_prop_name(jak_carbon_printer *self, jak_string *builder,
                          const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self)
        _jak_json_printer_extended_print_key(builder, key_name, key_len);
}

// ---------------------------------------------------------------------------------------------------------------------

bool jak_json_extended_printer_create(jak_carbon_printer *printer)
{
        JAK_ERROR_IF_NULL(printer);
        printer->drop = _jak_json_printer_extended_drop;

        printer->record_begin = _jak_json_printer_extended_obj_begin;
        printer->record_end = _jak_json_printer_extended_obj_end;

        printer->meta_begin = _jak_json_printer_extended_meta_begin;
        printer->meta_data = meta_data;
        printer->meta_end = _jak_json_printer_extended_meta_end;

        printer->doc_begin = _jak_json_printer_extended_doc_begin;
        printer->doc_end = _jak_json_printer_extended_doc_end;

        printer->empty_record = _jak_json_printer_extended_empty_record;

        printer->unit_array_begin = _jak_json_printer_extended_array_begin;
        printer->unit_array_end = _jak_json_printer_extended_array_end;

        printer->array_begin = _jak_json_printer_extended_array_begin;
        printer->array_end = _jak_json_printer_extended_array_end;

        printer->const_null = _jak_json_printer_extended_const_null;
        printer->const_true = _jak_json_printer_extended_const_true;
        printer->const_false = _jak_json_printer_extended_const_false;

        printer->val_signed = _jak_json_printer_extended_val_signed;
        printer->val_unsigned = _jak_json_printer_extended_val_unsigned;
        printer->val_float = _jak_json_printer_extended_val_float;
        printer->val_string = _jak_json_printer_extended_val_string;
        printer->val_binary = _jak_json_printer_extended_val_binary;

        printer->comma = _jak_json_printer_extended_comma;

        printer->prop_null = _jak_json_printer_extended_prop_null;
        printer->prop_true = _jak_json_printer_extended_prop_true;
        printer->prop_false = _jak_json_printer_extended_prop_false;
        printer->prop_signed = _jak_json_printer_extended_prop_signed;
        printer->prop_unsigned = _jak_json_printer_extended_prop_unsigned;
        printer->prop_float = _jak_json_printer_extended_prop_float;
        printer->prop_string = _jak_json_printer_extended_prop_string;
        printer->prop_binary = _jak_json_printer_extended_prop_binary;
        printer->array_prop_name = _jak_json_printer_extended_array_prop_name;
        printer->column_prop_name = _jak_json_printer_extended_array_prop_name;
        printer->obj_prop_name = _jak_json_printer_extended_obj_prop_name;
        printer->obj_begin = _jak_json_printer_extended_obj_begin;
        printer->obj_end = _jak_json_printer_extended_obj_end;

        printer->extra = JAK_MALLOC(sizeof(struct jak_json_extended_extra));
        struct jak_json_extended_extra *extra = (struct jak_json_extended_extra *) printer->extra;
        *extra = (struct jak_json_extended_extra) {
                .buffer_size = INIT_BUFFER_LEN,
                .buffer = JAK_MALLOC(INIT_BUFFER_LEN)
        };
        JAK_ZERO_MEMORY(extra->buffer, extra->buffer_size);

        return true;
}