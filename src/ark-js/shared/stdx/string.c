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

#include <inttypes.h>
#include <ark-js/shared/stdx/string.h>

bool string_create(struct string *builder)
{
        return string_create_ex(builder, 1024);
}

bool string_create_ex(struct string *builder, size_t capacity)
{
        error_if_null(builder)
        error_if_null(capacity)
        error_init(&builder->err);
        builder->cap = capacity;
        builder->end = 0;
        builder->data = ark_malloc(capacity);
        error_if_and_return(!builder->data, &builder->err, ARK_ERR_MALLOCERR, false);
        ark_zero_memory(builder->data, builder->cap);
        return true;
}

bool string_add(struct string *builder, const char *str)
{
        error_if_null(builder)
        error_if_null(str)
        u64 len = strlen(str);
        return string_add_nchar(builder, str, len);
}

bool string_add_nchar(struct string *builder, const char *str, u64 strlen)
{
        error_if_null(builder)
        error_if_null(str)

        /* resize if needed */
        if (unlikely(builder->end + strlen >= builder->cap)) {
                size_t new_cap = (builder->end + strlen) * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                error_if_and_return(!builder->data, &builder->err, ARK_ERR_REALLOCERR, false);
                ark_zero_memory(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }

        /* append string */
        memcpy(builder->data + builder->end, str, strlen);
        builder->end += strlen;

        return true;
}

bool string_add_char(struct string *builder, char c)
{
        error_if_null(builder)
        char buffer[2];
        sprintf(buffer, "%c", c);
        return string_add(builder, buffer);
        return true;
}

bool string_add_u8(struct string *builder, u8 value)
{
        char buffer[21];
        ark_zero_memory(buffer, ARK_ARRAY_LENGTH(buffer));
        sprintf(buffer, "%u", value);
        return string_add(builder, buffer);
}

bool string_add_u16(struct string *builder, u16 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return string_add(builder, buffer);
}

bool string_add_u32(struct string *builder, u32 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return string_add(builder, buffer);
}

bool string_add_u64(struct string *builder, u64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIu64, value);
        return string_add(builder, buffer);
}

bool string_add_i8(struct string *builder, i8 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_add(builder, buffer);
}

bool string_add_i16(struct string *builder, i16 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_add(builder, buffer);
}

bool string_add_i32(struct string *builder, i32 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_add(builder, buffer);
}

bool string_add_i64(struct string *builder, i64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIi64, value);
        return string_add(builder, buffer);
}

bool string_add_u64_as_hex(struct string *builder, u64 value)
{
        char buffer[17];
        sprintf(buffer, "%016"PRIx64, value);
        return string_add(builder, buffer);
}

bool string_add_u64_as_hex_0x_prefix_compact(struct string *builder, u64 value)
{
        char buffer[17];
        sprintf(buffer, "0x%"PRIx64, value);
        return string_add(builder, buffer);
}

bool string_add_float(struct string *builder, float value)
{
        char buffer[2046];
        sprintf(buffer, "%0.2f", value);
        return string_add(builder, buffer);
}

bool string_clear(struct string *builder)
{
        error_if_null(builder)
        ark_zero_memory(builder->data, builder->cap);
        builder->end = 0;
        return true;
}

bool string_ensure_capacity(struct string *builder, u64 cap)
{
        error_if_null(builder)
        /* resize if needed */
        if (unlikely(cap > builder->cap)) {
                size_t new_cap = cap * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                error_if_and_return(!builder->data, &builder->err, ARK_ERR_REALLOCERR, false);
                ark_zero_memory(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }
        return true;
}

size_t string_len(struct string *builder)
{
        error_if_null(builder)
        return builder->end;
}

bool string_drop(struct string *builder)
{
        error_if_null(builder)
        free(builder->data);
        return true;
}

bool string_print(struct string *builder)
{
        return string_fprint(stdout, builder);
}

bool string_fprint(FILE *file, struct string *builder)
{
        error_if_null(file)
        error_if_null(builder)
        fprintf(file, "%s\n", string_cstr(builder));
        return true;
}

const char *string_cstr(struct string *builder)
{
        return builder->data;
}