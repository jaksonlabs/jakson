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
#include <jak_string.h>

bool jak_string_create(jak_string *builder)
{
        return jak_string_create_ex(builder, 1024);
}

bool jak_string_create_ex(jak_string *builder, size_t capacity)
{
        JAK_ERROR_IF_NULL(builder)
        JAK_ERROR_IF_NULL(capacity)
        jak_error_init(&builder->err);
        builder->cap = capacity;
        builder->end = 0;
        builder->data = JAK_MALLOC(capacity);
        JAK_ERROR_IF_AND_RETURN(!builder->data, &builder->err, JAK_ERR_MALLOCERR, false);
        JAK_ZERO_MEMORY(builder->data, builder->cap);
        return true;
}

bool jak_string_add(jak_string *builder, const char *str)
{
        JAK_ERROR_IF_NULL(builder)
        JAK_ERROR_IF_NULL(str)
        jak_u64 len = strlen(str);
        return jak_string_add_nchar(builder, str, len);
}

bool jak_string_add_nchar(jak_string *builder, const char *str, jak_u64 strlen)
{
        JAK_ERROR_IF_NULL(builder)
        JAK_ERROR_IF_NULL(str)

        /* resize if needed */
        if (JAK_UNLIKELY(builder->end + strlen >= builder->cap)) {
                size_t new_cap = (builder->end + strlen) * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                JAK_ERROR_IF_AND_RETURN(!builder->data, &builder->err, JAK_ERR_REALLOCERR, false);
                JAK_ZERO_MEMORY(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }

        /* append string */
        memcpy(builder->data + builder->end, str, strlen);
        builder->end += strlen;

        return true;
}

bool jak_string_add_char(jak_string *builder, char c)
{
        JAK_ERROR_IF_NULL(builder)
        char buffer[2];
        sprintf(buffer, "%c", c);
        return jak_string_add(builder, buffer);
        return true;
}

bool jak_string_add_u8(jak_string *builder, jak_u8 value)
{
        char buffer[21];
        JAK_ZERO_MEMORY(buffer, JAK_ARRAY_LENGTH(buffer));
        sprintf(buffer, "%u", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_u16(jak_string *builder, jak_u16 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_u32(jak_string *builder, jak_u32 value)
{
        char buffer[21];
        sprintf(buffer, "%u", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_u64(jak_string *builder, jak_u64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIu64, value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_i8(jak_string *builder, jak_i8 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_i16(jak_string *builder, jak_i16 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_i32(jak_string *builder, jak_i32 value)
{
        char buffer[21];
        sprintf(buffer, "%d", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_i64(jak_string *builder, jak_i64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIi64, value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_u64_as_hex(jak_string *builder, jak_u64 value)
{
        char buffer[17];
        sprintf(buffer, "%016"PRIx64, value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_u64_as_hex_0x_prefix_compact(jak_string *builder, jak_u64 value)
{
        char buffer[17];
        sprintf(buffer, "0x%"PRIx64, value);
        return jak_string_add(builder, buffer);
}

bool jak_string_add_float(jak_string *builder, float value)
{
        char buffer[2046];
        sprintf(buffer, "%0.2f", value);
        return jak_string_add(builder, buffer);
}

bool jak_string_clear(jak_string *builder)
{
        JAK_ERROR_IF_NULL(builder)
        JAK_ZERO_MEMORY(builder->data, builder->cap);
        builder->end = 0;
        return true;
}

bool jak_string_ensure_capacity(jak_string *builder, jak_u64 cap)
{
        JAK_ERROR_IF_NULL(builder)
        /* resize if needed */
        if (JAK_UNLIKELY(cap > builder->cap)) {
                size_t new_cap = cap * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                JAK_ERROR_IF_AND_RETURN(!builder->data, &builder->err, JAK_ERR_REALLOCERR, false);
                JAK_ZERO_MEMORY(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }
        return true;
}

size_t jak_string_len(jak_string *builder)
{
        JAK_ERROR_IF_NULL(builder)
        return builder->end;
}

bool jak_string_drop(jak_string *builder)
{
        JAK_ERROR_IF_NULL(builder)
        free(builder->data);
        return true;
}

bool jak_string_print(jak_string *builder)
{
        return jak_string_fprint(stdout, builder);
}

bool jak_string_fprint(FILE *file, jak_string *builder)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(builder)
        fprintf(file, "%s\n", jak_string_cstr(builder));
        return true;
}

const char *jak_string_cstr(jak_string *builder)
{
        return builder->data;
}