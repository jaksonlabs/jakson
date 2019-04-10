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

#ifndef CARBON_BITMAP_H
#define CARBON_BITMAP_H

#include "shared/common.h"
#include "std/vec.h"

CARBON_BEGIN_DECL

typedef struct carbon_bitmap
{
    vec_t ofType(u64) data;
    u16 num_bits;
} carbon_bitmap_t;

CARBON_EXPORT(bool)
carbon_bitmap_create(carbon_bitmap_t *bitmap, u16 num_bits);

CARBON_EXPORT(bool)
carbon_bitmap_cpy(carbon_bitmap_t *dst, const carbon_bitmap_t *src);

CARBON_EXPORT(bool)
carbon_bitmap_drop(carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(size_t)
carbon_bitmap_nbits(const carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(bool)
carbon_bitmap_clear(carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(bool)
carbon_bitmap_set(carbon_bitmap_t *carbon_bitmap_t, u16 bit_position, bool on);

CARBON_EXPORT(bool)
carbon_bitmap_get(carbon_bitmap_t *carbon_bitmap_t, u16 bit_position);

CARBON_EXPORT(bool)
carbon_bitmap_lshift(carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(bool)
carbon_bitmap_print(FILE *file, const carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(bool)
carbon_bitmap_blocks(u32 **blocks, u32 *num_blocks, const carbon_bitmap_t *carbon_bitmap_t);

CARBON_EXPORT(void)
carbon_bitmap_print_bits(FILE *file, u32 n);

CARBON_EXPORT(void)
carbon_bitmap_print_bits_in_char(FILE *file, char n);

CARBON_END_DECL

#endif