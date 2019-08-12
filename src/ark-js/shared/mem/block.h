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

#ifndef ARK_MEMBLOCK_H
#define ARK_MEMBLOCK_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>

ARK_BEGIN_DECL

struct memblock;

bool memblock_create(struct memblock **block, size_t size);

bool memblock_zero_out(struct memblock *block);

bool memblock_from_file(struct memblock **block, FILE *file, size_t nbytes);

bool memblock_drop(struct memblock *block);

bool memblock_get_error(struct err *out, struct memblock *block);

bool memblock_size(offset_t *size, const struct memblock *block);

offset_t memblock_last_used_byte(const struct memblock *block);

bool memblock_write_to_file(FILE *file, const struct memblock *block);

const char *memblock_raw_data(const struct memblock *block);

bool memblock_resize(struct memblock *block, size_t size);

bool memblock_write(struct memblock *block, offset_t position, const char *data, offset_t nbytes);

bool memblock_cpy(struct memblock **dst, struct memblock *src);

bool memblock_shrink(struct memblock *block);

bool memblock_move_right(struct memblock *block, offset_t where, size_t nbytes);

bool memblock_move_left(struct memblock *block, offset_t where, size_t nbytes);

bool memblock_move_ex(struct memblock *block, offset_t where, size_t nbytes, bool zero_out);

void *memblock_move_contents_and_drop(struct memblock *block);

bool memfile_update_last_byte(struct memblock *block, size_t where);

ARK_END_DECL

#endif
