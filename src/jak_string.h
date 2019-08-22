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

#ifndef JAK_STRING_BUILDER_H
#define JAK_STRING_BUILDER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

struct jak_string {
        char *data;
        size_t cap;
        size_t end;
        struct jak_error err;
};

JAK_DEFINE_GET_ERROR_FUNCTION(string, struct jak_string, builder);

bool string_create(struct jak_string *builder);

bool string_create_ex(struct jak_string *builder, size_t capacity);

bool string_add(struct jak_string *builder, const char *str);

bool string_add_nchar(struct jak_string *builder, const char *str, jak_u64 strlen);

bool string_add_char(struct jak_string *builder, char c);

bool string_add_u8(struct jak_string *builder, jak_u8 value);

bool string_add_u16(struct jak_string *builder, jak_u16 value);

bool string_add_u32(struct jak_string *builder, jak_u32 value);

bool string_add_u64(struct jak_string *builder, jak_u64 value);

bool string_add_i8(struct jak_string *builder, jak_i8 value);

bool string_add_i16(struct jak_string *builder, jak_i16 value);

bool string_add_i32(struct jak_string *builder, jak_i32 value);

bool string_add_i64(struct jak_string *builder, jak_i64 value);

bool string_add_u64_as_hex(struct jak_string *builder, jak_u64 value);

bool string_add_u64_as_hex_0x_prefix_compact(struct jak_string *builder, jak_u64 value);

bool string_add_float(struct jak_string *builder, float value);

bool string_clear(struct jak_string *builder);

bool string_ensure_capacity(struct jak_string *builder, jak_u64 cap);

size_t string_len(struct jak_string *builder);

bool string_drop(struct jak_string *builder);

bool string_print(struct jak_string *builder);

bool string_fprint(FILE *file, struct jak_string *builder);

const char *string_cstr(struct jak_string *builder);

JAK_END_DECL

#endif
