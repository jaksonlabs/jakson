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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_bitmap.h>

bool jak_bitmap_create(jak_bitmap *bitmap, jak_u16 num_bits)
{
        JAK_ERROR_IF_NULL(bitmap);

        jak_allocator alloc;
        jak_alloc_create_std(&alloc);
        vec_create(&bitmap->data, &alloc, sizeof(jak_u32), ceil(num_bits / (double) JAK_bit_num_of(jak_u32)));
        size_t cap = vec_capacity(&bitmap->data);
        jak_u32 zero = 0;
        vec_repeated_push(&bitmap->data, &zero, cap);
        bitmap->num_bits = num_bits;

        return true;
}

bool jak_bitmap_cpy(jak_bitmap *dst, const jak_bitmap *src)
{
        dst->num_bits = src->num_bits;
        return vec_cpy(&dst->data, &src->data);
}

bool jak_bitmap_drop(jak_bitmap *bitset)
{
        return vec_drop(&bitset->data);
}

size_t jak_bitmap_nbits(const jak_bitmap *bitset)
{
        JAK_ERROR_IF_NULL(bitset);
        return bitset->num_bits;
}

bool jak_bitmap_clear(jak_bitmap *bitset)
{
        JAK_ERROR_IF_NULL(bitset);
        void *data = (void *) vec_data(&bitset->data);
        memset(data, 0, sizeof(jak_u32) * vec_capacity(&bitset->data));
        return true;
}

bool jak_bitmap_set(jak_bitmap *bitset, jak_u16 bit_position, bool on)
{
        JAK_ERROR_IF_NULL(bitset)
        size_t block_pos = floor(bit_position / (double) JAK_bit_num_of(jak_u32));
        size_t block_bit = bit_position % JAK_bit_num_of(jak_u32);
        jak_u32 block = *vec_get(&bitset->data, block_pos, jak_u32);
        jak_u32 mask = JAK_set_bit(block_bit);
        if (on) {
                JAK_set_bits(block, mask);
        } else {
                JAK_unset_bits(block, mask);
        }
        vec_set(&bitset->data, block_pos, &block);
        return true;
}

bool jak_bitmap_get(jak_bitmap *bitset, jak_u16 bit_position)
{
        JAK_ERROR_IF_NULL(bitset)
        size_t block_pos = floor(bit_position / (double) JAK_bit_num_of(jak_u32));
        size_t block_bit = bit_position % JAK_bit_num_of(jak_u32);
        jak_u32 block = *vec_get(&bitset->data, block_pos, jak_u32);
        jak_u32 mask = JAK_set_bit(block_bit);
        return ((mask & block) >> bit_position) == true;
}

bool jak_bitmap_lshift(jak_bitmap *map)
{
        JAK_ERROR_IF_NULL(map)
        for (int i = map->num_bits - 1; i >= 0; i--) {
                bool f = i > 0 ? jak_bitmap_get(map, i - 1) : false;
                jak_bitmap_set(map, i, f);
        }
        return true;
}

void jak_bitmap_print_bits(FILE *file, jak_u32 n)
{
        for (int i = 31; i >= 0; i--) {
                jak_u32 mask = 1 << i;
                jak_u32 k = n & mask;
                fprintf(file, "%s", k == 0 ? "0" : "1");
        }
}

void jak_bitmap_print_bits_in_char(FILE *file, char n)
{
        fprintf(file, "0b");
        for (int i = 7; i >= 0; i--) {
                char mask = 1 << i;
                char k = n & mask;
                fprintf(file, "%s", k == 0 ? "0" : "1");
        }
}

bool jak_bitmap_blocks(jak_u32 **blocks, jak_u32 *num_blocks, const jak_bitmap *map)
{
        JAK_ERROR_IF_NULL(blocks)
        JAK_ERROR_IF_NULL(num_blocks)
        JAK_ERROR_IF_NULL(map)

        jak_u32 *result = JAK_MALLOC(map->data.num_elems * sizeof(jak_u32));
        jak_i32 k = 0;
        for (jak_i32 i = map->data.num_elems - 1; i >= 0; i--) {
                result[k++] = *vec_get(&map->data, i, jak_u32);
        }
        *blocks = result;
        *num_blocks = map->data.num_elems;

        return true;
}

bool jak_bitmap_print(FILE *file, const jak_bitmap *map)
{
        JAK_ERROR_IF_NULL(map)

        jak_u32 *blocks, num_blocks;

        jak_bitmap_blocks(&blocks, &num_blocks, map);

        for (jak_u32 i = 0; i < num_blocks; i++) {
                fprintf(file, " %"PRIu32 " |", blocks[i]);
        }

        free(blocks);

        for (jak_i32 i = map->data.num_elems - 1; i >= 0; i--) {
                jak_u32 block = *vec_get(&map->data, i, jak_u32);
                jak_bitmap_print_bits(stdout, block);
                fprintf(file, " |");
        }

        fprintf(file, "\n");
        return true;
}