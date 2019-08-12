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

#ifndef ARK_STRING_BUILDER_H
#define ARK_STRING_BUILDER_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>

ARK_BEGIN_DECL

struct string {
    char *data;
    size_t cap;
    size_t end;
    struct err err;
};

ARK_DEFINE_GET_ERROR_FUNCTION(string, struct string, builder);

bool string_create(struct string *builder);

bool string_create_ex(struct string *builder, size_t capacity);

bool string_add(struct string *builder, const char *str);

bool string_add_nchar(struct string *builder, const char *str, u64 strlen);

bool string_add_char(struct string *builder, char c);

bool string_add_u8(struct string *builder, u8 value);

bool string_add_u16(struct string *builder, u16 value);

bool string_add_u32(struct string *builder, u32 value);

bool string_add_u64(struct string *builder, u64 value);

bool string_add_i8(struct string *builder, i8 value);

bool string_add_i16(struct string *builder, i16 value);

bool string_add_i32(struct string *builder, i32 value);

bool string_add_i64(struct string *builder, i64 value);

bool string_add_float(struct string *builder, float value);

bool string_clear(struct string *builder);

size_t string_len(struct string *builder);

bool string_drop(struct string *builder);

bool string_print(struct string *builder);

bool string_fprint(FILE *file, struct string *builder);

const char *string_cstr(struct string *builder);

ARK_END_DECL

#endif
