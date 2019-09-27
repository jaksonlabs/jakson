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
#include <jakson/std/string.h>
#include <ctype.h>

bool string_buffer_create(string_buffer *builder)
{
        return string_buffer_create_ex(builder, 1024);
}

bool string_buffer_create_ex(string_buffer *builder, size_t capacity)
{
        ERROR_IF_NULL(builder)
        ERROR_IF_NULL(capacity)
        error_init(&builder->err);
        builder->cap = capacity;
        builder->end = 0;
        builder->data = MALLOC(capacity);
        ERROR_IF_AND_RETURN(!builder->data, &builder->err, ERR_MALLOCERR, false);
        ZERO_MEMORY(builder->data, builder->cap);
        return true;
}

bool string_buffer_add(string_buffer *builder, const char *str)
{
        ERROR_IF_NULL(builder)
        ERROR_IF_NULL(str)
        u64 len = strlen(str);
        return string_buffer_add_nchar(builder, str, len);
}

bool string_buffer_add_nchar(string_buffer *builder, const char *str, u64 strlen)
{
        ERROR_IF_NULL(builder)
        ERROR_IF_NULL(str)

        /** resize if needed */
        if (UNLIKELY(builder->end + strlen >= builder->cap)) {
                size_t new_cap = (builder->end + strlen) * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                ERROR_IF_AND_RETURN(!builder->data, &builder->err, ERR_REALLOCERR, false);
                ZERO_MEMORY(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }

        /** append string_buffer */
        memcpy(builder->data + builder->end, str, strlen);
        builder->end += strlen;

        return true;
}

bool string_buffer_add_char(string_buffer *builder, char c)
{
        ERROR_IF_NULL(builder)
        char buffer[2];
        sprintf(buffer, "%c", c);
        return string_buffer_add(builder, buffer);
        return true;
}

bool string_buffer_add_u8(string_buffer *builder, u8 value)
{
        char buffer[21];
        ZERO_MEMORY(buffer, ARRAY_LENGTH(buffer));
        sprintf(buffer, "%u", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_u16(string_buffer *builder, u16 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_u32(string_buffer *builder, u32 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_u64(string_buffer *builder, u64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIu64, value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_i8(string_buffer *builder, i8 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_i16(string_buffer *builder, i16 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_i32(string_buffer *builder, i32 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_i64(string_buffer *builder, i64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIi64, value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_u64_as_hex(string_buffer *builder, u64 value)
{
        char buffer[17];
        sprintf(buffer, "%016"PRIx64, value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_u64_as_hex_0x_prefix_compact(string_buffer *builder, u64 value)
{
        char buffer[17];
        sprintf(buffer, "0x%"PRIx64, value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_add_float(string_buffer *builder, float value)
{
        char buffer[2046];
        sprintf(buffer, "%0.2f", value);
        return string_buffer_add(builder, buffer);
}

bool string_buffer_clear(string_buffer *builder)
{
        ERROR_IF_NULL(builder)
        ZERO_MEMORY(builder->data, builder->cap);
        builder->end = 0;
        return true;
}

bool string_buffer_ensure_capacity(string_buffer *builder, u64 cap)
{
        ERROR_IF_NULL(builder)
        /** resize if needed */
        if (UNLIKELY(cap > builder->cap)) {
                size_t new_cap = cap * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                ERROR_IF_AND_RETURN(!builder->data, &builder->err, ERR_REALLOCERR, false);
                ZERO_MEMORY(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }
        return true;
}

size_t string_len(string_buffer *builder)
{
        ERROR_IF_NULL(builder)
        return builder->end;
}

bool string_buffer_trim(string_buffer *builder)
{
        ERROR_IF_NULL(builder)
        if (builder->end > 0) {
                char *string = builder->data;
                int len = strlen(string);

                while (isspace(string[len - 1])) {
                        string[--len] = '\0';
                }
                while (*string && isspace(*string)) {
                        ++string;
                        --len;
                }

                builder->end = len + 1;
                memmove(builder->data, string, builder->end);
        }
        return true;
}

bool string_buffer_is_empty(string_buffer *builder)
{
        return builder ? (builder->end == 0 || (builder->end == 1 && builder->data[0] == '\0')) : true;
}

bool string_buffer_drop(string_buffer *builder)
{
        ERROR_IF_NULL(builder)
        free(builder->data);
        return true;
}

bool string_buffer_print(string_buffer *builder)
{
        return string_buffer_fprint(stdout, builder);
}

bool string_buffer_fprint(FILE *file, string_buffer *builder)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(builder)
        fprintf(file, "%s\n", string_cstr(builder));
        return true;
}

const char *string_cstr(string_buffer *builder)
{
        return builder->data;
}