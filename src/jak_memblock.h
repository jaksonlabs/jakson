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

#include <jak_stdinc.h>
#include <jak_error.h>

JAK_BEGIN_DECL

struct jak_memblock;

bool memblock_create(struct jak_memblock **block, size_t size);

bool memblock_zero_out(struct jak_memblock *block);

bool memblock_from_file(struct jak_memblock **block, FILE *file, size_t nbytes);

bool memblock_drop(struct jak_memblock *block);

bool memblock_get_error(struct jak_error *out, struct jak_memblock *block);

bool memblock_size(jak_offset_t *size, const struct jak_memblock *block);

jak_offset_t memblock_last_used_byte(const struct jak_memblock *block);

bool memblock_write_to_file(FILE *file, const struct jak_memblock *block);

const char *memblock_raw_data(const struct jak_memblock *block);

bool memblock_resize(struct jak_memblock *block, size_t size);

bool memblock_write(struct jak_memblock *block, jak_offset_t position, const char *data, jak_offset_t nbytes);

bool memblock_cpy(struct jak_memblock **dst, struct jak_memblock *src);

bool memblock_shrink(struct jak_memblock *block);

bool memblock_move_right(struct jak_memblock *block, jak_offset_t where, size_t nbytes);

bool memblock_move_left(struct jak_memblock *block, jak_offset_t where, size_t nbytes);

bool memblock_move_ex(struct jak_memblock *block, jak_offset_t where, size_t nbytes, bool zero_out);

void *memblock_move_contents_and_drop(struct jak_memblock *block);

bool memfile_update_last_byte(struct jak_memblock *block, size_t where);

JAK_END_DECL

#endif
