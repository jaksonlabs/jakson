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

#include <math.h>
#include <lzma.h>
#include "bitmap/bitmap.h"

CARBON_EXPORT(bool)
carbon_bitmap_create(carbon_bitmap_t *bitmap, u16 num_bits)
{
    CARBON_NON_NULL_OR_ERROR(bitmap);

    struct allocator alloc;
    carbon_alloc_create_std(&alloc);
    carbon_vec_create(&bitmap->data, &alloc, sizeof(u32), ceil(num_bits / (double) CARBON_NUM_BITS(u32)));
    size_t cap = carbon_vec_capacity(&bitmap->data);
    u32 zero = 0;
    carbon_vec_repeated_push(&bitmap->data, &zero, cap);
    bitmap->num_bits = num_bits;

    return true;
}

CARBON_EXPORT(bool)
carbon_bitmap_cpy(carbon_bitmap_t *dst, const carbon_bitmap_t *src)
{
    dst->num_bits = src->num_bits;
    return carbon_vec_cpy(&dst->data, &src->data);
}

CARBON_EXPORT(bool)
carbon_bitmap_drop(carbon_bitmap_t *bitset)
{
    return carbon_vec_drop(&bitset->data);
}

size_t carbon_bitmap_nbits(const carbon_bitmap_t *bitset)
{
    CARBON_NON_NULL_OR_ERROR(bitset);
    return bitset->num_bits;
}

CARBON_EXPORT(bool)
carbon_bitmap_clear(carbon_bitmap_t *bitset)
{
    CARBON_NON_NULL_OR_ERROR(bitset);
    void *data = (void *) carbon_vec_data(&bitset->data);
    memset(data, 0, sizeof(u32) * carbon_vec_capacity(&bitset->data));
    return true;
}

CARBON_EXPORT(bool)
carbon_bitmap_set(carbon_bitmap_t *bitset, u16 bit_position, bool on)
{
    CARBON_NON_NULL_OR_ERROR(bitset)
    size_t block_pos = floor(bit_position / (double) CARBON_NUM_BITS(u32));
    size_t block_bit = bit_position % CARBON_NUM_BITS(u32);
    u32 block = *vec_get(&bitset->data, block_pos, u32);
    u32 mask = CARBON_SET_BIT(block_bit);
    if (on) {
        CARBON_FIELD_SET(block, mask);
    }
    else {
        CARBON_FIELD_CLEAR(block, mask);
    }
    carbon_vec_set(&bitset->data, block_pos, &block);
    return true;
}

bool carbon_bitmap_get(carbon_bitmap_t *bitset, u16 bit_position)
{
    CARBON_NON_NULL_OR_ERROR(bitset)
    size_t block_pos = floor(bit_position / (double) CARBON_NUM_BITS(u32));
    size_t block_bit = bit_position % CARBON_NUM_BITS(u32);
    u32 block = *vec_get(&bitset->data, block_pos, u32);
    u32 mask = CARBON_SET_BIT(block_bit);
    return ((mask & block) >> bit_position) == true;
}

CARBON_EXPORT(bool)
carbon_bitmap_lshift(carbon_bitmap_t *carbon_bitmap_t)
{
    CARBON_NON_NULL_OR_ERROR(carbon_bitmap_t)
    for (int i = carbon_bitmap_t->num_bits - 1; i >= 0; i--) {
        bool f = i > 0 ? carbon_bitmap_get(carbon_bitmap_t, i - 1) : false;
        carbon_bitmap_set(carbon_bitmap_t, i, f);
    }
    return true;
}

void carbon_bitmap_print_bits(FILE *file, u32 n)
{
    for (int i = 31; i >= 0; i--) {
        u32 mask = 1 << i;
        u32 k = n & mask;
        fprintf(file, "%s", k == 0 ? "0" : "1");
    }
}

void carbon_bitmap_print_bits_in_char(FILE *file, char n)
{
    fprintf(file, "0b");
    for (int i = 7; i >= 0; i--) {
        char mask = 1 << i;
        char k = n & mask;
        fprintf(file, "%s", k == 0 ? "0" : "1");
    }
}

bool carbon_bitmap_blocks(u32 **blocks, u32 *num_blocks, const carbon_bitmap_t *carbon_bitmap_t)
{
    CARBON_NON_NULL_OR_ERROR(blocks)
    CARBON_NON_NULL_OR_ERROR(num_blocks)
    CARBON_NON_NULL_OR_ERROR(carbon_bitmap_t)

    u32 *result = malloc(carbon_bitmap_t->data.num_elems * sizeof(u32));
    i32 k = 0;
    for (i32 i = carbon_bitmap_t->data.num_elems - 1; i >= 0; i--) {
        result[k++] = *vec_get(&carbon_bitmap_t->data, i, u32);
    }
    *blocks = result;
    *num_blocks = carbon_bitmap_t->data.num_elems;

    return true;
}

bool carbon_bitmap_print(FILE *file, const carbon_bitmap_t *carbon_bitmap_t)
{
    CARBON_NON_NULL_OR_ERROR(carbon_bitmap_t)

    u32 *blocks, num_blocks;

    carbon_bitmap_blocks(&blocks, &num_blocks, carbon_bitmap_t);

    for (u32 i = 0; i < num_blocks; i++) {
        fprintf(file, " %"PRIu32 " |", blocks[i]);
    }

    free(blocks);

    for (i32 i = carbon_bitmap_t->data.num_elems - 1; i >= 0; i--) {
        u32 block = *vec_get(&carbon_bitmap_t->data, i, u32);
        carbon_bitmap_print_bits(stdout, block);
        fprintf(file, " |");
    }

    fprintf(file, "\n");
    return true;
}