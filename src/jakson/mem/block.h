/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef MEMBLOCK_H
#define MEMBLOCK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>

BEGIN_DECL

bool memblock_create(memblock **block, size_t size);
bool memblock_drop(memblock *block);

bool memblock_from_file(memblock **block, FILE *file, size_t nbytes);
bool memblock_from_raw_data(memblock **block, const void *data, size_t nbytes);

bool memblock_get_error(err *out, memblock *block);

bool memblock_zero_out(memblock *block);
bool memblock_size(offset_t *size, const memblock *block);
offset_t memblock_last_used_byte(const memblock *block);
bool memblock_write_to_file(FILE *file, const memblock *block);
const char *memblock_raw_data(const memblock *block);
bool memblock_resize(memblock *block, size_t size);
bool memblock_write(memblock *block, offset_t position, const char *data, offset_t nbytes);
bool memblock_cpy(memblock **dst, memblock *src);
bool memblock_shrink(memblock *block);
bool memblock_move_right(memblock *block, offset_t where, size_t nbytes);
bool memblock_move_left(memblock *block, offset_t where, size_t nbytes);
bool memblock_move_ex(memblock *block, offset_t where, size_t nbytes, bool zero_out);
void *memblock_move_contents_and_drop(memblock *block);
bool memfile_update_last_byte(memblock *block, size_t where);

END_DECL

#endif
