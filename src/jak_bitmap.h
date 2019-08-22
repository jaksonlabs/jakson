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

#ifndef JAK_BITMAP_H
#define JAK_BITMAP_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

struct jak_bitmap {
    struct vector ofType(jak_u64) data;
    jak_u16 num_bits;
};

bool bitmap_create(struct jak_bitmap *bitmap, jak_u16 num_bits);

bool bitmap_cpy(struct jak_bitmap *dst, const struct jak_bitmap *src);

bool bitmap_drop(struct jak_bitmap *map);

size_t bitmap_nbits(const struct jak_bitmap *map);

bool bitmap_clear(struct jak_bitmap *map);

bool bitmap_set(struct jak_bitmap *map, jak_u16 bit_position, bool on);

bool bitmap_get(struct jak_bitmap *map, jak_u16 bit_position);

bool bitmap_lshift(struct jak_bitmap *map);

bool bitmap_print(FILE *file, const struct jak_bitmap *map);

bool bitmap_blocks(jak_u32 **blocks, jak_u32 *num_blocks, const struct jak_bitmap *map);

void bitmap_print_bits(FILE *file, jak_u32 n);

void bitmap_print_bits_in_char(FILE *file, char n);

JAK_END_DECL

#endif