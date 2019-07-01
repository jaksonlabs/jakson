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

#include "libs/libb64/libb64.h"

#include "core/bison/bison-printers.h"

#define INIT_BUFFER_LEN 1024

#define NULL_STR "null"

struct json_formatter_extra
{
        bool intent;
        bool strict;
        char *buffer;
        size_t buffer_size;
};

inline static bool json_formatter_has_intention(struct bison_printer *printer);
inline static bool json_formatter_is_strict(struct bison_printer *printer);

static void json_formatter_drop(struct bison_printer *self);

static void json_formatter_bison_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_header_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_header_contents(struct bison_printer *self, struct string_builder *builder, object_id_t oid, u64 rev);
static void json_formatter_bison_header_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_bison_payload_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_payload_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_bison_array_begin(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_array_end(struct bison_printer *self, struct string_builder *builder);

static void json_formatter_bison_null(struct bison_printer *self, struct string_builder *builder);
static void json_formatter_bison_true(struct bison_printer *self, bool is_null, struct string_builder *builder);
static void json_formatter_bison_false(struct bison_printer *self, bool is_null, struct string_builder *builder);

static void json_formatter_bison_signed(struct bison_printer *self, struct string_builder *builder, const i64 *value);
static void json_formatter_bison_unsigned(struct bison_printer *self, struct string_builder *builder, const u64 *value);
static void json_formatter_bison_float(struct bison_printer *self, struct string_builder *builder, const float *value);

static void json_formatter_bison_string(struct bison_printer *self, struct string_builder *builder, const char *value, u64 strlen);
static void json_formatter_bison_binary(struct bison_printer *self, struct string_builder *builder, const struct bison_binary *binary);

static void json_formatter_bison_comma(struct bison_printer *self, struct string_builder *builder);

NG5_EXPORT(bool) bison_json_formatter_create(struct bison_printer *printer)
{
        error_if_null(printer);
        printer->drop = json_formatter_drop;

        printer->print_bison_begin = json_formatter_bison_begin;
        printer->print_bison_end = json_formatter_bison_end;

        printer->print_bison_header_begin = json_formatter_header_begin;
        printer->print_bison_header_contents = json_formatter_bison_header_contents;
        printer->print_bison_header_end = json_formatter_bison_header_end;

        printer->print_bison_payload_begin = json_formatter_bison_payload_begin;
        printer->print_bison_payload_end = json_formatter_bison_payload_end;

        printer->print_bison_array_begin = json_formatter_bison_array_begin;
        printer->print_bison_array_end = json_formatter_bison_array_end;

        printer->print_bison_null = json_formatter_bison_null;
        printer->print_bison_true = json_formatter_bison_true;
        printer->print_bison_false = json_formatter_bison_false;

        printer->print_bison_signed = json_formatter_bison_signed;
        printer->print_bison_unsigned = json_formatter_bison_unsigned;
        printer->print_bison_float = json_formatter_bison_float;
        printer->print_bison_string = json_formatter_bison_string;
        printer->print_bison_binary = json_formatter_bison_binary;

        printer->print_bison_comma = json_formatter_bison_comma;

        printer->extra = malloc(sizeof(struct json_formatter_extra));
        struct json_formatter_extra *extra = (struct json_formatter_extra *) printer->extra;
        *extra = (struct json_formatter_extra) {
                .intent = false,
                .strict = true,
                .buffer_size = INIT_BUFFER_LEN,
                .buffer = malloc(INIT_BUFFER_LEN)
        };
        ng5_zero_memory(extra->buffer, extra->buffer_size);

        return true;
}

NG5_EXPORT(bool) bison_json_formatter_set_intent(struct bison_printer *printer, bool enable)
{
        error_if_null(printer);
        ((struct json_formatter_extra *) printer->extra)->intent = enable;
        return true;
}

NG5_EXPORT(bool) bison_json_formatter_set_strict(struct bison_printer *printer, bool enable)
{
        error_if_null(printer);
        ((struct json_formatter_extra *) printer->extra)->strict = enable;
        return true;
}

static void json_formatter_drop(struct bison_printer *self)
{
        struct json_formatter_extra *extra = (struct json_formatter_extra *) self->extra;
        free(extra->buffer);
        free (self->extra);
}

ng5_func_unused
inline static bool json_formatter_has_intention(struct bison_printer *printer)
{
        return ((struct json_formatter_extra *) printer->extra)->intent;
}

inline static bool json_formatter_is_strict(struct bison_printer *printer)
{
        return ((struct json_formatter_extra *) printer->extra)->strict;
}

static void json_formatter_bison_begin(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "{");
}

static void json_formatter_bison_end(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "}");
}

static void json_formatter_header_begin(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"meta\": {");
        } else {
                string_builder_append(builder, "meta: {");
        }
}

static void json_formatter_bison_header_contents(struct bison_printer *self, struct string_builder *builder, object_id_t oid, u64 rev)
{
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"_id\": ");
                string_builder_append_u64(builder, oid);
                string_builder_append(builder, ", \"_rev\": ");
                string_builder_append_u64(builder, rev);
        } else {
                string_builder_append(builder, "_id: ");
                string_builder_append_u64(builder, oid);
                string_builder_append(builder, ", _rev: ");
                string_builder_append_u64(builder, rev);
        }
}

static void json_formatter_bison_header_end(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "}, ");
}

static void json_formatter_bison_payload_begin(struct bison_printer *self, struct string_builder *builder)
{
        if (json_formatter_is_strict(self)) {
                string_builder_append(builder, "\"doc\": ");
        } else {
                string_builder_append(builder, "doc: ");
        }
}

static void json_formatter_bison_payload_end(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        unused(builder);
}

static void json_formatter_bison_array_begin(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "[");
}

static void json_formatter_bison_array_end(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "]");
}

static void json_formatter_bison_null(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, "null");
}

static void json_formatter_bison_true(struct bison_printer *self, bool is_null, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, is_null ? "null" : "true");
}

static void json_formatter_bison_false(struct bison_printer *self, bool is_null, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, is_null ? "null" : "false");
}

static void json_formatter_bison_signed(struct bison_printer *self, struct string_builder *builder, const i64 *value)
{
        unused(self);
        if (likely(value != NULL)) {
                string_builder_append_i64(builder, *value);
        } else {
                string_builder_append(builder, NULL_STR);
        }

}

static void json_formatter_bison_unsigned(struct bison_printer *self, struct string_builder *builder, const u64 *value)
{
        unused(self);
        if (likely(value != NULL)) {
                string_builder_append_u64(builder, *value);
        } else {
                string_builder_append(builder, NULL_STR);
        }
}

static void json_formatter_bison_float(struct bison_printer *self, struct string_builder *builder, const float *value)
{
        unused(self);
        if (likely(value != NULL)) {
                string_builder_append_float(builder, *value);
        } else {
                string_builder_append(builder, NULL_STR);
        }
}

static void json_formatter_bison_string(struct bison_printer *self, struct string_builder *builder, const char *value, u64 strlen)
{
        unused(self);
        string_builder_append_char(builder, '"');
        string_builder_append_nchar(builder, value, strlen);
        string_builder_append_char(builder, '"');
}

#define code_of(x, data_len)      (x + data_len + 2)
#define data_of(x)      (x)

static void json_formatter_bison_binary(struct bison_printer *self, struct string_builder *builder, const struct bison_binary *binary)
{
        unused(self);

        /* base64 code will be written into the extra's buffer after a null-terminated copy of the binary data */
        struct json_formatter_extra *extra = (struct json_formatter_extra *) self->extra;
        /* buffer of at least 2x data length for base64 code + 1x data length to hold the null-terminated value */
        size_t required_buff_size = 3 * (binary->blob_len + 1);
        /* increase buffer capacity if needed */
        if (extra->buffer_size < required_buff_size) {
                extra->buffer_size = required_buff_size * 1.7f;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                error_print_if(!extra->buffer, NG5_ERR_REALLOCERR);
        }
        /* decrease buffer capacity if needed */
        if (extra->buffer_size * 0.3f > required_buff_size) {
                extra->buffer_size = required_buff_size;
                extra->buffer = realloc(extra->buffer, extra->buffer_size);
                error_print_if(!extra->buffer, NG5_ERR_REALLOCERR);
        }

        assert(extra->buffer_size >= required_buff_size);
        ng5_zero_memory(extra->buffer, extra->buffer_size);
        /* copy binary data into buffer, and leave one (zero'd) byte free; null-termination is required by libb64 */
        memcpy(data_of(extra->buffer), binary->blob, binary->blob_len);

        string_builder_append(builder, "{ ");
        string_builder_append(builder, "\"type\": \"");
        string_builder_append_nchar(builder, binary->mime_type, binary->mime_type_strlen);
        string_builder_append(builder, "\", \"encoding\": \"base64\", \"binary-string\": \"");

        base64_encodestate state;
        base64_init_encodestate(&state);

        u64 code_len = base64_encode_block(data_of(extra->buffer), binary->blob_len + 2,
                code_of(extra->buffer, binary->blob_len), &state);
        base64_encode_blockend(code_of(extra->buffer, binary->blob_len), &state);
        string_builder_append_nchar(builder, code_of(extra->buffer, binary->blob_len), code_len);


        string_builder_append(builder, "\" }");
}

static void json_formatter_bison_comma(struct bison_printer *self, struct string_builder *builder)
{
        unused(self);
        string_builder_append(builder, ", ");
}