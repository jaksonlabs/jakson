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

#ifndef STRING_H
#define STRING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>

BEGIN_DECL

typedef struct string_buffer {
        char *data;
        size_t cap;
        size_t end;
        err err;
} string_buffer;

bool string_buffer_create(string_buffer *builder);
bool string_buffer_create_ex(string_buffer *builder, size_t capacity);
bool string_buffer_drop(string_buffer *builder);

bool string_buffer_add(string_buffer *builder, const char *str);
bool string_buffer_add_nchar(string_buffer *builder, const char *str, u64 strlen);
bool string_buffer_add_char(string_buffer *builder, char c);
bool string_buffer_add_u8(string_buffer *builder, u8 value);
bool string_buffer_add_u16(string_buffer *builder, u16 value);
bool string_buffer_add_u32(string_buffer *builder, u32 value);
bool string_buffer_add_u64(string_buffer *builder, u64 value);
bool string_buffer_add_i8(string_buffer *builder, i8 value);
bool string_buffer_add_i16(string_buffer *builder, i16 value);
bool string_buffer_add_i32(string_buffer *builder, i32 value);
bool string_buffer_add_i64(string_buffer *builder, i64 value);
bool string_buffer_add_u64_as_hex(string_buffer *builder, u64 value);
bool string_buffer_add_u64_as_hex_0x_prefix_compact(string_buffer *builder, u64 value);
bool string_buffer_add_float(string_buffer *builder, float value);
bool string_buffer_clear(string_buffer *builder);
bool string_buffer_ensure_capacity(string_buffer *builder, u64 cap);
size_t string_len(string_buffer *builder);
bool string_buffer_trim(string_buffer *builder);
bool string_buffer_is_empty(string_buffer *builder);

const char *string_cstr(string_buffer *builder);

bool string_buffer_print(string_buffer *builder);
bool string_buffer_fprint(FILE *file, string_buffer *builder);

END_DECL

#endif
