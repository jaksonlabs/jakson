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

#ifndef JAK_MEMBLOCK_H
#define JAK_MEMBLOCK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>

JAK_BEGIN_DECL

bool jak_memblock_create(jak_memblock **block, size_t size);
bool jak_memblock_drop(jak_memblock *block);

bool jak_memblock_from_file(jak_memblock **block, FILE *file, size_t nbytes);
bool jak_memblock_from_raw_data(jak_memblock **block, const void *data, size_t nbytes);

bool jak_memblock_get_error(jak_error *out, jak_memblock *block);

bool jak_memblock_zero_out(jak_memblock *block);
bool jak_memblock_size(jak_offset_t *size, const jak_memblock *block);
jak_offset_t jak_memblock_last_used_byte(const jak_memblock *block);
bool jak_memblock_write_to_file(FILE *file, const jak_memblock *block);
const char *jak_memblock_raw_data(const jak_memblock *block);
bool jak_memblock_resize(jak_memblock *block, size_t size);
bool jak_memblock_write(jak_memblock *block, jak_offset_t position, const char *data, jak_offset_t nbytes);
bool jak_memblock_cpy(jak_memblock **dst, jak_memblock *src);
bool jak_memblock_shrink(jak_memblock *block);
bool jak_memblock_move_right(jak_memblock *block, jak_offset_t where, size_t nbytes);
bool jak_memblock_move_left(jak_memblock *block, jak_offset_t where, size_t nbytes);
bool jak_memblock_move_ex(jak_memblock *block, jak_offset_t where, size_t nbytes, bool zero_out);
void *jak_memblock_move_contents_and_drop(jak_memblock *block);
bool jak_memfile_update_last_byte(jak_memblock *block, size_t where);

JAK_END_DECL

#endif
