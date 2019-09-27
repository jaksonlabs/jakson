/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef CARBON_STRING_H
#define CARBON_STRING_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/mem/file.h>

BEGIN_DECL

bool carbon_string_write(memfile *file, const char *string);
bool carbon_string_nchar_write(memfile *file, const char *string, u64 str_len);
bool carbon_string_nomarker_write(memfile *file, const char *string);
bool carbon_string_nomarker_nchar_write(memfile *file, const char *string, u64 str_len);
bool carbon_string_nomarker_remove(memfile *file);
bool carbon_string_remove(memfile *file);
bool carbon_string_update(memfile *file, const char *string);
bool carbon_string_update_wnchar(memfile *file, const char *string, size_t str_len);
bool carbon_string_skip(memfile *file);
bool carbon_string_nomarker_skip(memfile *file);
const char *carbon_string_read(u64 *len, memfile *file);
const char *carbon_string_nomarker_read(u64 *len, memfile *file);

END_DECL

#endif
