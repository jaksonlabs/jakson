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

bool bitmap_create(struct jak_bitmap *bitmap, jak_u16 num_bits)
{
        error_if_null(bitmap);

        struct jak_allocator alloc;
        jak_alloc_create_std(&alloc);
        vec_create(&bitmap->data, &alloc, sizeof(jak_u32), ceil(num_bits / (double) JAK_bit_num_of(jak_u32)));
        size_t cap = vec_capacity(&bitmap->data);
        jak_u32 zero = 0;
        vec_repeated_push(&bitmap->data, &zero, cap);
        bitmap->num_bits = num_bits;

        return true;
}

bool bitmap_cpy(struct jak_bitmap *dst, const struct jak_bitmap *src)
{
        dst->num_bits = src->num_bits;
        return vec_cpy(&dst->data, &src->data);
}

bool bitmap_drop(struct jak_bitmap *bitset)
{
        return vec_drop(&bitset->data);
}

size_t bitmap_nbits(const struct jak_bitmap *bitset)
{
        error_if_null(bitset);
        return bitset->num_bits;
}

bool bitmap_clear(struct jak_bitmap *bitset)
{
        error_if_null(bitset);
        void *data = (void *) vec_data(&bitset->data);
        memset(data, 0, sizeof(jak_u32) * vec_capacity(&bitset->data));
        return true;
}

bool bitmap_set(struct jak_bitmap *bitset, jak_u16 bit_position, bool on)
{
        error_if_null(bitset)
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

bool bitmap_get(struct jak_bitmap *bitset, jak_u16 bit_position)
{
        error_if_null(bitset)
        size_t block_pos = floor(bit_position / (double) JAK_bit_num_of(jak_u32));
        size_t block_bit = bit_position % JAK_bit_num_of(jak_u32);
        jak_u32 block = *vec_get(&bitset->data, block_pos, jak_u32);
        jak_u32 mask = JAK_set_bit(block_bit);
        return ((mask & block) >> bit_position) == true;
}

bool bitmap_lshift(struct jak_bitmap *map)
{
        error_if_null(map)
        for (int i = map->num_bits - 1; i >= 0; i--) {
                bool f = i > 0 ? bitmap_get(map, i - 1) : false;
                bitmap_set(map, i, f);
        }
        return true;
}

void bitmap_print_bits(FILE *file, jak_u32 n)
{
        for (int i = 31; i >= 0; i--) {
                jak_u32 mask = 1 << i;
                jak_u32 k = n & mask;
                fprintf(file, "%s", k == 0 ? "0" : "1");
        }
}

void bitmap_print_bits_in_char(FILE *file, char n)
{
        fprintf(file, "0b");
        for (int i = 7; i >= 0; i--) {
                char mask = 1 << i;
                char k = n & mask;
                fprintf(file, "%s", k == 0 ? "0" : "1");
        }
}

bool bitmap_blocks(jak_u32 **blocks, jak_u32 *num_blocks, const struct jak_bitmap *map)
{
        error_if_null(blocks)
        error_if_null(num_blocks)
        error_if_null(map)

        jak_u32 *result = JAK_MALLOC(map->data.num_elems * sizeof(jak_u32));
        jak_i32 k = 0;
        for (jak_i32 i = map->data.num_elems - 1; i >= 0; i--) {
                result[k++] = *vec_get(&map->data, i, jak_u32);
        }
        *blocks = result;
        *num_blocks = map->data.num_elems;

        return true;
}

bool bitmap_print(FILE *file, const struct jak_bitmap *map)
{
        error_if_null(map)

        jak_u32 *blocks, num_blocks;

        bitmap_blocks(&blocks, &num_blocks, map);

        for (jak_u32 i = 0; i < num_blocks; i++) {
                fprintf(file, " %"PRIu32 " |", blocks[i]);
        }

        free(blocks);

        for (jak_i32 i = map->data.num_elems - 1; i >= 0; i--) {
                jak_u32 block = *vec_get(&map->data, i, jak_u32);
                bitmap_print_bits(stdout, block);
                fprintf(file, " |");
        }

        fprintf(file, "\n");
        return true;
}