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
#include "std/string_builder.h"

NG5_EXPORT(bool) string_builder_create(struct string_builder *builder)
{
        return string_builder_create_ex(builder, 1024);
}

NG5_EXPORT(bool) string_builder_create_ex(struct string_builder *builder, size_t capacity)
{
        error_if_null(builder)
        error_if_null(capacity)
        error_init(&builder->err);
        builder->cap = capacity;
        builder->end = 0;
        builder->data = malloc(capacity);
        error_if_and_return(!builder->data, &builder->err, NG5_ERR_MALLOCERR, false);
        ng5_zero_memory(builder->data, builder->cap);
        return true;
}

NG5_EXPORT(bool) string_builder_append(struct string_builder *builder, const char *str)
{
        error_if_null(builder)
        error_if_null(str)
        size_t len = strlen(str);

        /* resize if needed */
        if (unlikely(builder->end + len >= builder->cap)) {
                size_t new_cap = (builder->end + len) * 1.7f;
                builder->data = realloc(builder->data, new_cap);
                error_if_and_return(!builder->data, &builder->err, NG5_ERR_REALLOCERR, false);
                ng5_zero_memory(builder->data + builder->cap, (new_cap - builder->cap));
                builder->cap = new_cap;
        }

        /* append string */
        memcpy(builder->data + builder->end, str, len);
        builder->end += len;

        return true;
}

NG5_EXPORT(bool) string_builder_append_char(struct string_builder *builder, char c)
{
        error_if_null(builder)
        char buffer[2];
        sprintf(buffer, "%c", c);
        return string_builder_append(builder, buffer);
        return true;
}

NG5_EXPORT(bool) string_builder_append_u64(struct string_builder *builder, u64 value)
{
        char buffer[21];
        sprintf(buffer, "%" PRIu64, value);
        return string_builder_append(builder, buffer);
}

NG5_EXPORT(bool) string_builder_clear(struct string_builder *builder)
{
        error_if_null(builder)
        ng5_zero_memory(builder->data, builder->cap);
        builder->end = 0;
        return true;
}

NG5_EXPORT(size_t) string_builder_length(struct string_builder *builder)
{
        error_if_null(builder)
        return builder->end;
}

NG5_EXPORT(bool) string_builder_drop(struct string_builder *builder)
{
        error_if_null(builder)
        free (builder->data);
        return true;
}

NG5_EXPORT(const char *) string_builder_cstr(struct string_builder *builder)
{
        return builder->data;
}