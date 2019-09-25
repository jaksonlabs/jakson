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

#ifndef JAK_STRING_H
#define JAK_STRING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

typedef struct jak_string {
        char *data;
        size_t cap;
        size_t end;
        jak_error err;
} jak_string;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_string, jak_string, builder);

bool jak_string_create(jak_string *builder);
bool jak_string_create_ex(jak_string *builder, size_t capacity);
bool jak_string_drop(jak_string *builder);

bool jak_string_add(jak_string *builder, const char *str);
bool jak_string_add_nchar(jak_string *builder, const char *str, jak_u64 strlen);
bool jak_string_add_char(jak_string *builder, char c);
bool jak_string_add_u8(jak_string *builder, jak_u8 value);
bool jak_string_add_u16(jak_string *builder, jak_u16 value);
bool jak_string_add_u32(jak_string *builder, jak_u32 value);
bool jak_string_add_u64(jak_string *builder, jak_u64 value);
bool jak_string_add_i8(jak_string *builder, jak_i8 value);
bool jak_string_add_i16(jak_string *builder, jak_i16 value);
bool jak_string_add_i32(jak_string *builder, jak_i32 value);
bool jak_string_add_i64(jak_string *builder, jak_i64 value);
bool jak_string_add_u64_as_hex(jak_string *builder, jak_u64 value);
bool jak_string_add_u64_as_hex_0x_prefix_compact(jak_string *builder, jak_u64 value);
bool jak_string_add_float(jak_string *builder, float value);
bool jak_string_clear(jak_string *builder);
bool jak_string_ensure_capacity(jak_string *builder, jak_u64 cap);
size_t jak_string_len(jak_string *builder);
bool jak_string_trim(jak_string *builder);
bool jak_string_is_empty(jak_string *builder);

const char *jak_string_cstr(jak_string *builder);

bool jak_string_print(jak_string *builder);
bool jak_string_fprint(FILE *file, jak_string *builder);

JAK_END_DECL

#endif
