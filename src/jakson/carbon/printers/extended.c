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
#include <jakson/carbon.h>
#include <jakson/carbon/printers/extended.h>
#include <jakson/carbon/commit.h>

// ---------------------------------------------------------------------------------------------------------------------

#define INIT_BUFFER_LEN 1024
#define NULL_STR "null"

// ---------------------------------------------------------------------------------------------------------------------

struct json_extended_extra {
        char *buffer;
        size_t buffer_size;
};

// ---------------------------------------------------------------------------------------------------------------------

static void _json_printer_extended_drop(carbon_printer *self)
{
        struct json_extended_extra *extra = (struct json_extended_extra *) self->extra;
        free(extra->buffer);
        free(self->extra);
}

static void _json_printer_extended_obj_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "{");
}

static void _json_printer_extended_obj_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "}");
}

static void _json_printer_extended_meta_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "\"meta\": {");
}

static void meta_data(carbon_printer *self, string_buffer *builder,
                      int key_type, const void *key,
                      u64 key_length, u64 commit_hash)
{
        UNUSED(self)

        string_buffer_add(builder, "\"key\": {");

        switch (key_type) {
                case CARBON_KEY_NOKEY:
                        string_buffer_add(builder, "\"type\": \"nokey\", \"value\": null");
                        break;
                case CARBON_KEY_AUTOKEY:
                        string_buffer_add(builder, "\"type\": \"autokey\", \"value\": ");
                        string_buffer_add_u64(builder, *(u64 *) key);
                        break;
                case CARBON_KEY_UKEY:
                        string_buffer_add(builder, "\"type\": \"ukey\", \"value\": ");
                        string_buffer_add_u64(builder, *(u64 *) key);
                        break;
                case CARBON_KEY_IKEY:
                        string_buffer_add(builder, "\"type\": \"ikey\", \"value\": ");
                        string_buffer_add_u64(builder, *(i64 *) key);
                        break;
                case CARBON_KEY_SKEY:
                        string_buffer_add(builder, "\"type\": \"skey\", \"value\": ");
                        if (key_length > 0) {
                                string_buffer_add(builder, "\"");
                                string_buffer_add_nchar(builder, key, key_length);
                                string_buffer_add(builder, "\"");
                        } else {
                                string_buffer_add(builder, "null");
                        }

                        break;
                default: ERROR_PRINT(ERR_INTERNALERR);
        }
        string_buffer_add(builder, "}, \"commit\": ");
        if (commit_hash) {
                string_buffer_add(builder, "\"");
                carbon_commit_hash_append_to_str(builder, commit_hash);
                string_buffer_add(builder, "\"");
        } else {
                string_buffer_add(builder, "null");
        }
}

static void _json_printer_extended_meta_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "}, ");
}

static void _json_printer_extended_doc_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self)

        string_buffer_add(builder, "\"doc\": ");
}

static void _json_printer_extended_doc_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        UNUSED(builder);
}

static void _json_printer_extended_empty_record(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "[]");
}

static void _json_printer_extended_array_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "[");
}

static void _json_printer_extended_array_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "]");
}

static void _json_printer_extended_const_null(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "null");
}

static void _json_printer_extended_const_true(carbon_printer *self, bool is_null, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, is_null ? "null" : "true");
}

static void _json_printer_extended_const_false(carbon_printer *self, bool is_null, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, is_null ? "null" : "false");
}

static void _json_printer_extended_val_signed(carbon_printer *self, string_buffer *builder, const i64 *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_i64(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }

}

static void _json_printer_extended_val_unsigned(carbon_printer *self, string_buffer *builder, const u64 *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_u64(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }
}

static void _json_printer_extended_val_float(carbon_printer *self, string_buffer *builder, const float *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_float(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }
}

static void _json_printer_extended_val_string(carbon_printer *self, string_buffer *builder, const char *value, u64 strlen)
{
        UNUSED(self);
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, value, strlen);
        string_buffer_add_char(builder, '"');
}

#define code_of(x, data_len)      (x + data_len + 2)
#define data_of(x)      (x)

static void _json_printer_extended_print_binary(carbon_printer *self, string_buffer *builder, const carbon_binary *binary)
{
        /** base64 code will be written into the extra's buffer after a null-terminated copy of the binary data */
        struct json_extended_extra *extra = (struct json_extended_extra *) self->extra;
        /** buffer of at least 2x data length for base64 code + 1x data length to hold the null-terminated value */
        size_t required_buff_size = 3 * (binary->blob_len + 1);
        /** increase buffer capacity if needed */
        if (extra->buffer_size < required_buff_size) {
                extra->buffer_size = required_buff_size * 1.7f;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                ERROR_PRINT_IF(!extra->buffer, ERR_REALLOCERR);
        }
        /** decrease buffer capacity if needed */
        if (extra->buffer_size * 0.3f > required_buff_size) {
                extra->buffer_size = required_buff_size;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                ERROR_PRINT_IF(!extra->buffer, ERR_REALLOCERR);
        }

        JAK_ASSERT(extra->buffer_size >= required_buff_size);
        ZERO_MEMORY(extra->buffer, extra->buffer_size);
        /** copy binary data into buffer, and leave one (zero'd) byte free; null-termination is required by libb64 */
        memcpy(data_of(extra->buffer), binary->blob, binary->blob_len);

        string_buffer_add(builder, "{ ");
        string_buffer_add(builder, "\"type\": \"");
        string_buffer_add_nchar(builder, binary->mime_type, binary->mime_type_strlen);
        string_buffer_add(builder, "\", \"encoding\": \"base64\", \"binary-string_buffer\": \"");

        base64_encodestate state;
        base64_init_encodestate(&state);

        u64 code_len = base64_encode_block(data_of(extra->buffer), binary->blob_len + 2,
                                               code_of(extra->buffer, binary->blob_len), &state);
        base64_encode_blockend(code_of(extra->buffer, binary->blob_len), &state);
        string_buffer_add_nchar(builder, code_of(extra->buffer, binary->blob_len), code_len);


        string_buffer_add(builder, "\" }");
}

static void _json_printer_extended_val_binary(carbon_printer *self, string_buffer *builder, const carbon_binary *binary)
{
        _json_printer_extended_print_binary(self, builder, binary);
}

static void _json_printer_extended_comma(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, ", ");
}

static void _json_printer_extended_print_key(string_buffer *builder, const char *key_name, u64 key_len)
{
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, key_name, key_len);
        string_buffer_add(builder, "\": ");
}

static void _json_printer_extended_prop_null(carbon_printer *self, string_buffer *builder,
                      const char *key_name, u64 key_len)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add(builder, "null");
}

static void _json_printer_extended_prop_true(carbon_printer *self, string_buffer *builder,
                      const char *key_name, u64 key_len)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add(builder, "true");
}

static void _json_printer_extended_prop_false(carbon_printer *self, string_buffer *builder,
                       const char *key_name, u64 key_len)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add(builder, "false");
}

static void _json_printer_extended_prop_signed(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const i64 *value)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add_i64(builder, *value);
}

static void _json_printer_extended_prop_unsigned(carbon_printer *self, string_buffer *builder,
                          const char *key_name, u64 key_len, const u64 *value)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add_u64(builder, *value);
}

static void _json_printer_extended_prop_float(carbon_printer *self, string_buffer *builder,
                       const char *key_name, u64 key_len, const float *value)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add_float(builder, *value);
}

static void _json_printer_extended_prop_string(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const char *value, u64 strlen)
{
        UNUSED(self);
        _json_printer_extended_print_key(builder, key_name, key_len);
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, value, strlen);
        string_buffer_add_char(builder, '"');
}

static void _json_printer_extended_prop_binary(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const carbon_binary *binary)
{
        _json_printer_extended_print_key(builder, key_name, key_len);
        _json_printer_extended_print_binary(self, builder, binary);
}

static void _json_printer_extended_array_prop_name(carbon_printer *self, string_buffer *builder,
                            const char *key_name, u64 key_len)
{
        UNUSED(self)
        _json_printer_extended_print_key(builder, key_name, key_len);
}

static void _json_printer_extended_obj_prop_name(carbon_printer *self, string_buffer *builder,
                          const char *key_name, u64 key_len)
{
        UNUSED(self)
        _json_printer_extended_print_key(builder, key_name, key_len);
}

// ---------------------------------------------------------------------------------------------------------------------

bool json_extended_printer_create(carbon_printer *printer)
{
        ERROR_IF_NULL(printer);
        printer->drop = _json_printer_extended_drop;

        printer->record_begin = _json_printer_extended_obj_begin;
        printer->record_end = _json_printer_extended_obj_end;

        printer->meta_begin = _json_printer_extended_meta_begin;
        printer->meta_data = meta_data;
        printer->meta_end = _json_printer_extended_meta_end;

        printer->doc_begin = _json_printer_extended_doc_begin;
        printer->doc_end = _json_printer_extended_doc_end;

        printer->empty_record = _json_printer_extended_empty_record;

        printer->unit_array_begin = _json_printer_extended_array_begin;
        printer->unit_array_end = _json_printer_extended_array_end;

        printer->array_begin = _json_printer_extended_array_begin;
        printer->array_end = _json_printer_extended_array_end;

        printer->const_null = _json_printer_extended_const_null;
        printer->const_true = _json_printer_extended_const_true;
        printer->const_false = _json_printer_extended_const_false;

        printer->val_signed = _json_printer_extended_val_signed;
        printer->val_unsigned = _json_printer_extended_val_unsigned;
        printer->val_float = _json_printer_extended_val_float;
        printer->val_string = _json_printer_extended_val_string;
        printer->val_binary = _json_printer_extended_val_binary;

        printer->comma = _json_printer_extended_comma;

        printer->prop_null = _json_printer_extended_prop_null;
        printer->prop_true = _json_printer_extended_prop_true;
        printer->prop_false = _json_printer_extended_prop_false;
        printer->prop_signed = _json_printer_extended_prop_signed;
        printer->prop_unsigned = _json_printer_extended_prop_unsigned;
        printer->prop_float = _json_printer_extended_prop_float;
        printer->prop_string = _json_printer_extended_prop_string;
        printer->prop_binary = _json_printer_extended_prop_binary;
        printer->array_prop_name = _json_printer_extended_array_prop_name;
        printer->column_prop_name = _json_printer_extended_array_prop_name;
        printer->obj_prop_name = _json_printer_extended_obj_prop_name;
        printer->obj_begin = _json_printer_extended_obj_begin;
        printer->obj_end = _json_printer_extended_obj_end;

        printer->extra = MALLOC(sizeof(struct json_extended_extra));
        struct json_extended_extra *extra = (struct json_extended_extra *) printer->extra;
        *extra = (struct json_extended_extra) {
                .buffer_size = INIT_BUFFER_LEN,
                .buffer = MALLOC(INIT_BUFFER_LEN)
        };
        ZERO_MEMORY(extra->buffer, extra->buffer_size);

        return true;
}