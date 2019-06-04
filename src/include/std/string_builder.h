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

#ifndef NG5_STRING_BUILDER_H
#define NG5_STRING_BUILDER_H

#include "shared/common.h"
#include "shared/error.h"

NG5_BEGIN_DECL

struct string_builder
{
        char *data;
        size_t cap;
        size_t end;
        struct err err;
};

NG5_DEFINE_GET_ERROR_FUNCTION(string_builder, struct string_builder, builder);

NG5_EXPORT(bool) string_builder_create(struct string_builder *builder);

NG5_EXPORT(bool) string_builder_create_ex(struct string_builder *builder, size_t capacity);

NG5_EXPORT(bool) string_builder_append(struct string_builder *builder, const char *str);

NG5_EXPORT(bool) string_builder_append_u64(struct string_builder *builder, u64 value);

NG5_EXPORT(bool) string_builder_clear(struct string_builder *builder);

NG5_EXPORT(size_t) string_builder_length(struct string_builder *builder);

NG5_EXPORT(bool) string_builder_drop(struct string_builder *builder);

NG5_EXPORT(const char *) string_builder_cstr(struct string_builder *builder);

NG5_END_DECL

#endif
