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

#ifndef BITMAP_H
#define BITMAP_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>

BEGIN_DECL

typedef struct bitmap {
        vector ofType(u64) data;
        u16 num_bits;
} bitmap;

bool bitmap_create(bitmap *bitmap, u16 num_bits);
bool bitmap_cpy(bitmap *dst, const bitmap *src);
bool bitmap_drop(bitmap *map);
size_t bitmap_nbits(const bitmap *map);
bool bitmap_clear(bitmap *map);
bool bitmap_set(bitmap *map, u16 bit_position, bool on);
bool bitmap_get(bitmap *map, u16 bit_position);
bool bitmap_lshift(bitmap *map);
bool bitmap_print(FILE *file, const bitmap *map);
bool bitmap_blocks(u32 **blocks, u32 *num_blocks, const bitmap *map);
void bitmap_print_bits(FILE *file, u32 n);
void bitmap_print_bits_in_char(FILE *file, char n);

END_DECL

#endif