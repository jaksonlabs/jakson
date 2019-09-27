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
#include <jakson/carbon/printers/extended.h>
#include <jakson/carbon/array_it.h>

// ---------------------------------------------------------------------------------------------------------------------

#define INIT_BUFFER_LEN 1024
#define NULL_STR "null"

// ---------------------------------------------------------------------------------------------------------------------

struct extra {
        char *buffer;
        size_t buffer_size;
};

// ---------------------------------------------------------------------------------------------------------------------

#define INIT_BUFFER_LEN 1024
#define NULL_STR "null"

// ---------------------------------------------------------------------------------------------------------------------

struct json_compact_extra {
        char *buffer;
        size_t buffer_size;
};

// ---------------------------------------------------------------------------------------------------------------------

static void _json_printer_compact_nop(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self)
        UNUSED(builder)
}

static void _json_printer_compact_drop(carbon_printer *self)
{
        struct json_compact_extra *extra = (struct json_compact_extra *) self->extra;
        free(extra->buffer);
        free(self->extra);
}

static void _json_printer_compact_obj_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "{");
}

static void _json_printer_compact_obj_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "}");
}

static void meta_data_nop(carbon_printer *self, string_buffer *builder, int key_type, const void *key,
                          u64 key_length, u64 rev)
{
        UNUSED(self)
        UNUSED(builder)
        UNUSED(key_type)
        UNUSED(key)
        UNUSED(key_length)
        UNUSED(rev)
}

static void _json_printer_compact_empty_record(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "{}");
}

static void _json_printer_compact_array_begin(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);

        string_buffer_add(builder, "[");
}

static void _json_printer_compact_array_end(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "]");
}

static void _json_printer_compact_const_null(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, "null");
}

static void _json_printer_compact_const_true(carbon_printer *self, bool is_null, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, is_null ? "null" : "true");
}

static void _json_printer_compact_const_false(carbon_printer *self, bool is_null, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, is_null ? "null" : "false");
}

static void _json_printer_compact_val_signed(carbon_printer *self, string_buffer *builder, const i64 *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_i64(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }

}

static void _json_printer_compact_val_unsigned(carbon_printer *self, string_buffer *builder, const u64 *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_u64(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }
}

static void _json_printer_compact_val_float(carbon_printer *self, string_buffer *builder, const float *value)
{
        UNUSED(self);
        if (LIKELY(value != NULL)) {
                string_buffer_add_float(builder, *value);
        } else {
                string_buffer_add(builder, NULL_STR);
        }
}

static void _json_printer_compact_val_string(carbon_printer *self, string_buffer *builder, const char *value, u64 strlen)
{
        UNUSED(self);
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, value, strlen);
        string_buffer_add_char(builder, '"');
}

#define code_of(x, data_len)      (x + data_len + 2)
#define data_of(x)      (x)

static void _json_printer_compact_print_binary(carbon_printer *self, string_buffer *builder, const carbon_binary *binary)
{
        /** base64 code will be written into the extra's buffer after a null-terminated copy of the binary data */
        struct json_compact_extra *extra = (struct json_compact_extra *) self->extra;
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

static void _json_printer_compact_val_binary(carbon_printer *self, string_buffer *builder, const carbon_binary *binary)
{
        _json_printer_compact_print_binary(self, builder, binary);
}

static void _json_printer_compact_comma(carbon_printer *self, string_buffer *builder)
{
        UNUSED(self);
        string_buffer_add(builder, ", ");
}

static void print_key(string_buffer *builder, const char *key_name, u64 key_len)
{
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, key_name, key_len);
        string_buffer_add(builder, "\": ");
}

static void _json_printer_compact_prop_null(carbon_printer *self, string_buffer *builder,
                      const char *key_name, u64 key_len)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add(builder, "null");
}

static void _json_printer_compact_prop_true(carbon_printer *self, string_buffer *builder,
                      const char *key_name, u64 key_len)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add(builder, "true");
}

static void _json_printer_compact_prop_false(carbon_printer *self, string_buffer *builder,
                       const char *key_name, u64 key_len)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add(builder, "false");
}

static void _json_printer_compact_prop_signed(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const i64 *value)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add_i64(builder, *value);
}

static void _json_printer_compact_prop_unsigned(carbon_printer *self, string_buffer *builder,
                          const char *key_name, u64 key_len, const u64 *value)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add_u64(builder, *value);
}

static void _json_printer_compact_prop_float(carbon_printer *self, string_buffer *builder,
                       const char *key_name, u64 key_len, const float *value)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add_float(builder, *value);
}

static void _json_printer_compact_prop_string(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const char *value, u64 strlen)
{
        UNUSED(self);
        print_key(builder, key_name, key_len);
        string_buffer_add_char(builder, '"');
        string_buffer_add_nchar(builder, value, strlen);
        string_buffer_add_char(builder, '"');
}

static void _json_printer_compact_prop_binary(carbon_printer *self, string_buffer *builder,
                        const char *key_name, u64 key_len, const carbon_binary *binary)
{
        print_key(builder, key_name, key_len);
        _json_printer_compact_print_binary(self, builder, binary);
}

static void _json_printer_compact_array_prop_name(carbon_printer *self, string_buffer *builder,
                            const char *key_name, u64 key_len)
{
        UNUSED(self)
        print_key(builder, key_name, key_len);
}

static void _json_printer_compact_obj_prop_name(carbon_printer *self, string_buffer *builder,
                          const char *key_name, u64 key_len)
{
        UNUSED(self)
        print_key(builder, key_name, key_len);
}

// ---------------------------------------------------------------------------------------------------------------------

bool json_compact_printer_create(carbon_printer *printer)
{
        ERROR_IF_NULL(printer);
        printer->drop = _json_printer_compact_drop;

        printer->record_begin = _json_printer_compact_nop;
        printer->record_end = _json_printer_compact_nop;

        printer->meta_begin = _json_printer_compact_nop;
        printer->meta_data = meta_data_nop;
        printer->meta_end = _json_printer_compact_nop;

        printer->doc_begin = _json_printer_compact_nop;
        printer->doc_end = _json_printer_compact_nop;

        printer->empty_record = _json_printer_compact_empty_record;

        printer->unit_array_begin = _json_printer_compact_nop;
        printer->unit_array_end = _json_printer_compact_nop;

        printer->array_begin = _json_printer_compact_array_begin;
        printer->array_end = _json_printer_compact_array_end;

        printer->const_null = _json_printer_compact_const_null;
        printer->const_true = _json_printer_compact_const_true;
        printer->const_false = _json_printer_compact_const_false;

        printer->val_signed = _json_printer_compact_val_signed;
        printer->val_unsigned = _json_printer_compact_val_unsigned;
        printer->val_float = _json_printer_compact_val_float;
        printer->val_string = _json_printer_compact_val_string;
        printer->val_binary = _json_printer_compact_val_binary;

        printer->comma = _json_printer_compact_comma;

        printer->prop_null = _json_printer_compact_prop_null;
        printer->prop_true = _json_printer_compact_prop_true;
        printer->prop_false = _json_printer_compact_prop_false;
        printer->prop_signed = _json_printer_compact_prop_signed;
        printer->prop_unsigned = _json_printer_compact_prop_unsigned;
        printer->prop_float = _json_printer_compact_prop_float;
        printer->prop_string = _json_printer_compact_prop_string;
        printer->prop_binary = _json_printer_compact_prop_binary;
        printer->array_prop_name = _json_printer_compact_array_prop_name;
        printer->column_prop_name = _json_printer_compact_array_prop_name;
        printer->obj_prop_name = _json_printer_compact_obj_prop_name;
        printer->obj_begin = _json_printer_compact_obj_begin;
        printer->obj_end = _json_printer_compact_obj_end;

        printer->extra = MALLOC(sizeof(struct json_compact_extra));
        struct json_compact_extra *extra = (struct json_compact_extra *) printer->extra;
        *extra = (struct json_compact_extra) {
                .buffer_size = INIT_BUFFER_LEN,
                .buffer = MALLOC(INIT_BUFFER_LEN)
        };
        ZERO_MEMORY(extra->buffer, extra->buffer_size);

        return true;
}
