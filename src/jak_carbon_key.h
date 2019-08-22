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

#ifndef JAK_JAK_CARBON_KEY_H
#define JAK_JAK_CARBON_KEY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

bool carbon_key_create(struct jak_memfile *file, jak_carbon_key_e type, struct jak_error *err);

bool carbon_key_skip(jak_carbon_key_e *out, struct jak_memfile *file);

bool carbon_key_read_type(jak_carbon_key_e *out, struct jak_memfile *file);

bool carbon_key_write_unsigned(struct jak_memfile *file, jak_u64 key);

bool carbon_key_write_signed(struct jak_memfile *file, jak_i64 key);

bool carbon_key_write_string(struct jak_memfile *file, const char *key);

bool carbon_key_update_string(struct jak_memfile *file, const char *key);

bool carbon_key_update_string_wnchar(struct jak_memfile *file, const char *key, size_t length);

const void *carbon_key_read(jak_u64 *len, jak_carbon_key_e *out, struct jak_memfile *file);

const char *carbon_key_type_str(jak_carbon_key_e type);

JAK_END_DECL

#endif
