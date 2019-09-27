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

#ifndef CARBON_KEY_H
#define CARBON_KEY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/carbon.h>
#include <jakson/mem/file.h>

BEGIN_DECL

bool carbon_key_create(memfile *file, carbon_key_e type, err *err);
bool carbon_key_skip(carbon_key_e *out, memfile *file);
bool carbon_key_read_type(carbon_key_e *out, memfile *file);
bool carbon_key_write_unsigned(memfile *file, u64 key);
bool carbon_key_write_signed(memfile *file, i64 key);
bool carbon_key_write_string(memfile *file, const char *key);
bool carbon_key_update_string(memfile *file, const char *key);
bool carbon_key_update_string_wnchar(memfile *file, const char *key, size_t length);
const void *carbon_key_read(u64 *len, carbon_key_e *out, memfile *file);
const char *carbon_key_type_str(carbon_key_e type);

END_DECL

#endif
