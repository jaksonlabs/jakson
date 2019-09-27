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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#ifndef JAK_CARBON_COMMIT_HASH_H
#define JAK_CARBON_COMMIT_HASH_H

#include <jakson/jak_stdinc.h>
#include <jakson/jak_error.h>
#include <jakson/memfile/jak_memfile.h>
#include <jakson/std/jak_string.h>

JAK_BEGIN_DECL

bool jak_carbon_commit_hash_create(jak_memfile *file);
bool jak_carbon_commit_hash_skip(jak_memfile *file);
bool jak_carbon_commit_hash_read(jak_u64 *commit_hash, jak_memfile *file);
bool jak_carbon_commit_hash_peek(jak_u64 *commit_hash, jak_memfile *file);
bool jak_carbon_commit_hash_update(jak_memfile *file, const char *base, jak_u64 len);
bool jak_carbon_commit_hash_compute(jak_u64 *commit_hash, const void *base, jak_u64 len);
const char *jak_carbon_commit_hash_to_str(jak_string *dst, jak_u64 commit_hash);
bool jak_carbon_commit_hash_append_to_str(jak_string *dst, jak_u64 commit_hash);
jak_u64 jak_carbon_commit_hash_from_str(const char *commit_str, jak_error *err);

JAK_END_DECL

#endif
