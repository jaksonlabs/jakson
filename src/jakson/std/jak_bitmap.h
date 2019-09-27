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

#include <jakson/stdinc.h>
#include <jakson/std/jak_vector.h>

JAK_BEGIN_DECL

typedef struct jak_bitmap {
        jak_vector ofType(jak_u64) data;
        jak_u16 num_bits;
} jak_bitmap;

bool jak_bitmap_create(jak_bitmap *bitmap, jak_u16 num_bits);
bool jak_bitmap_cpy(jak_bitmap *dst, const jak_bitmap *src);
bool jak_bitmap_drop(jak_bitmap *map);
size_t jak_bitmap_nbits(const jak_bitmap *map);
bool jak_bitmap_clear(jak_bitmap *map);
bool jak_bitmap_set(jak_bitmap *map, jak_u16 bit_position, bool on);
bool jak_bitmap_get(jak_bitmap *map, jak_u16 bit_position);
bool jak_bitmap_lshift(jak_bitmap *map);
bool jak_bitmap_print(FILE *file, const jak_bitmap *map);
bool jak_bitmap_blocks(jak_u32 **blocks, jak_u32 *num_blocks, const jak_bitmap *map);
void jak_bitmap_print_bits(FILE *file, jak_u32 n);
void jak_bitmap_print_bits_in_char(FILE *file, char n);

JAK_END_DECL

#endif