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
#include <jak_json_printer_extended.h>
#include <jak_carbon_array_it.h>

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

struct jak_json_compact_extra {
        char *buffer;
        size_t buffer_size;
};

// ---------------------------------------------------------------------------------------------------------------------

static void nop(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self)
        JAK_UNUSED(builder)
}

static void drop(jak_carbon_printer *self)
{
        struct jak_json_compact_extra *extra = (struct jak_json_compact_extra *) self->extra;
        free(extra->buffer);
        free(self->extra);
}

static void obj_begin(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, "{");
}

static void obj_end(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, "}");
}

static void meta_data_nop(jak_carbon_printer *self, struct jak_string *builder, int key_type, const void *key,
                          jak_u64 key_length, jak_u64 rev)
{
        JAK_UNUSED(self)
        JAK_UNUSED(builder)
        JAK_UNUSED(key_type)
        JAK_UNUSED(key)
        JAK_UNUSED(key_length)
        JAK_UNUSED(rev)
}

static void empty_record(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, "{}");
}

static void array_begin(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);

        string_add(builder, "[");
}

static void array_end(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, "]");
}

static void const_null(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, "null");
}

static void const_true(jak_carbon_printer *self, bool is_null, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, is_null ? "null" : "true");
}

static void const_false(jak_carbon_printer *self, bool is_null, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, is_null ? "null" : "false");
}

static void val_signed(jak_carbon_printer *self, struct jak_string *builder, const jak_i64 *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                string_add_i64(builder, *value);
        } else {
                string_add(builder, NULL_STR);
        }

}

static void val_unsigned(jak_carbon_printer *self, struct jak_string *builder, const jak_u64 *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                string_add_u64(builder, *value);
        } else {
                string_add(builder, NULL_STR);
        }
}

static void val_float(jak_carbon_printer *self, struct jak_string *builder, const float *value)
{
        JAK_UNUSED(self);
        if (JAK_LIKELY(value != NULL)) {
                string_add_float(builder, *value);
        } else {
                string_add(builder, NULL_STR);
        }
}

static void val_string(jak_carbon_printer *self, struct jak_string *builder, const char *value, jak_u64 strlen)
{
        JAK_UNUSED(self);
        string_add_char(builder, '"');
        string_add_nchar(builder, value, strlen);
        string_add_char(builder, '"');
}

#define code_of(x, data_len)      (x + data_len + 2)
#define data_of(x)      (x)

static void print_binary(jak_carbon_printer *self, struct jak_string *builder, const jak_carbon_binary *binary)
{
        /* base64 code will be written into the extra's buffer after a null-terminated copy of the binary data */
        struct jak_json_compact_extra *extra = (struct jak_json_compact_extra *) self->extra;
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
        JAK_zero_memory(extra->buffer, extra->buffer_size);
        /* copy binary data into buffer, and leave one (zero'd) byte free; null-termination is required by libb64 */
        memcpy(data_of(extra->buffer), binary->blob, binary->blob_len);

        string_add(builder, "{ ");
        string_add(builder, "\"type\": \"");
        string_add_nchar(builder, binary->mime_type, binary->mime_type_strlen);
        string_add(builder, "\", \"encoding\": \"base64\", \"binary-string\": \"");

        base64_encodestate state;
        base64_init_encodestate(&state);

        jak_u64 code_len = base64_encode_block(data_of(extra->buffer), binary->blob_len + 2,
                                               code_of(extra->buffer, binary->blob_len), &state);
        base64_encode_blockend(code_of(extra->buffer, binary->blob_len), &state);
        string_add_nchar(builder, code_of(extra->buffer, binary->blob_len), code_len);


        string_add(builder, "\" }");
}

static void val_binary(jak_carbon_printer *self, struct jak_string *builder, const jak_carbon_binary *binary)
{
        print_binary(self, builder, binary);
}

static void comma(jak_carbon_printer *self, struct jak_string *builder)
{
        JAK_UNUSED(self);
        string_add(builder, ", ");
}

static void print_key(struct jak_string *builder, const char *key_name, jak_u64 key_len)
{
        string_add_char(builder, '"');
        string_add_nchar(builder, key_name, key_len);
        string_add(builder, "\": ");
}

static void prop_null(jak_carbon_printer *self, struct jak_string *builder,
                      const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add(builder, "null");
}

static void prop_true(jak_carbon_printer *self, struct jak_string *builder,
                      const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add(builder, "true");
}

static void prop_false(jak_carbon_printer *self, struct jak_string *builder,
                       const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add(builder, "false");
}

static void prop_signed(jak_carbon_printer *self, struct jak_string *builder,
                        const char *key_name, jak_u64 key_len, const jak_i64 *value)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add_i64(builder, *value);
}

static void prop_unsigned(jak_carbon_printer *self, struct jak_string *builder,
                          const char *key_name, jak_u64 key_len, const jak_u64 *value)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add_u64(builder, *value);
}

static void prop_float(jak_carbon_printer *self, struct jak_string *builder,
                       const char *key_name, jak_u64 key_len, const float *value)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add_float(builder, *value);
}

static void prop_string(jak_carbon_printer *self, struct jak_string *builder,
                        const char *key_name, jak_u64 key_len, const char *value, jak_u64 strlen)
{
        JAK_UNUSED(self);
        print_key(builder, key_name, key_len);
        string_add_char(builder, '"');
        string_add_nchar(builder, value, strlen);
        string_add_char(builder, '"');
}

static void prop_binary(jak_carbon_printer *self, struct jak_string *builder,
                        const char *key_name, jak_u64 key_len, const jak_carbon_binary *binary)
{
        print_key(builder, key_name, key_len);
        print_binary(self, builder, binary);
}

static void array_prop_name(jak_carbon_printer *self, struct jak_string *builder,
                            const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self)
        print_key(builder, key_name, key_len);
}

static void obj_prop_name(jak_carbon_printer *self, struct jak_string *builder,
                          const char *key_name, jak_u64 key_len)
{
        JAK_UNUSED(self)
        print_key(builder, key_name, key_len);
}

// ---------------------------------------------------------------------------------------------------------------------

bool json_compact_printer_create(jak_carbon_printer *printer)
{
        JAK_ERROR_IF_NULL(printer);
        printer->drop = drop;

        printer->record_begin = nop;
        printer->record_end = nop;

        printer->meta_begin = nop;
        printer->meta_data = meta_data_nop;
        printer->meta_end = nop;

        printer->doc_begin = nop;
        printer->doc_end = nop;

        printer->empty_record = empty_record;

        printer->unit_array_begin = nop;
        printer->unit_array_end = nop;

        printer->array_begin = array_begin;
        printer->array_end = array_end;

        printer->const_null = const_null;
        printer->const_true = const_true;
        printer->const_false = const_false;

        printer->val_signed = val_signed;
        printer->val_unsigned = val_unsigned;
        printer->val_float = val_float;
        printer->val_string = val_string;
        printer->val_binary = val_binary;

        printer->comma = comma;

        printer->prop_null = prop_null;
        printer->prop_true = prop_true;
        printer->prop_false = prop_false;
        printer->prop_signed = prop_signed;
        printer->prop_unsigned = prop_unsigned;
        printer->prop_float = prop_float;
        printer->prop_string = prop_string;
        printer->prop_binary = prop_binary;
        printer->array_prop_name = array_prop_name;
        printer->column_prop_name = array_prop_name;
        printer->obj_prop_name = obj_prop_name;
        printer->obj_begin = obj_begin;
        printer->obj_end = obj_end;

        printer->extra = JAK_MALLOC(sizeof(struct jak_json_compact_extra));
        struct jak_json_compact_extra *extra = (struct jak_json_compact_extra *) printer->extra;
        *extra = (struct jak_json_compact_extra) {
                .buffer_size = INIT_BUFFER_LEN,
                .buffer = JAK_MALLOC(INIT_BUFFER_LEN)
        };
        JAK_zero_memory(extra->buffer, extra->buffer_size);

        return true;
}
