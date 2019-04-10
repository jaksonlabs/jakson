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

#ifndef NG5_MEMBLOCK_H
#define NG5_MEMBLOCK_H

#include "shared/common.h"
#include "shared/error.h"

NG5_BEGIN_DECL

typedef struct carbon_memblock carbon_memblock_t;

NG5_EXPORT(bool)
carbon_memblock_create(carbon_memblock_t **block, size_t size);

NG5_EXPORT(bool)
carbon_memblock_from_file(carbon_memblock_t **block, FILE *file, size_t nbytes);

NG5_EXPORT(bool)
carbon_memblock_drop(carbon_memblock_t *block);

NG5_EXPORT(bool)
carbon_memblock_get_error(struct err *out, carbon_memblock_t *block);

NG5_EXPORT(bool)
carbon_memblock_size(offset_t *size, const carbon_memblock_t *block);

NG5_EXPORT(bool)
carbon_memblock_write_to_file(FILE *file, const carbon_memblock_t *block);

NG5_EXPORT(const carbon_byte_t *)
carbon_memblock_raw_data(const carbon_memblock_t *block);

NG5_EXPORT(bool)
carbon_memblock_resize(carbon_memblock_t *block, size_t size);

NG5_EXPORT(bool)
carbon_memblock_write(carbon_memblock_t *block, offset_t position, const carbon_byte_t *data, offset_t nbytes);

NG5_EXPORT(bool)
carbon_memblock_cpy(carbon_memblock_t **dst, carbon_memblock_t *src);

NG5_EXPORT(bool)
carbon_memblock_shrink(carbon_memblock_t *block);

NG5_EXPORT(void *)
carbon_memblock_move_contents_and_drop(carbon_memblock_t *block);

NG5_END_DECL

#endif
