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

#include "carbon/carbon-bitmap.h"

CARBON_EXPORT(bool)
carbon_bitmap_create(carbon_bitmap_t *bitmap, uint16_t num_bits)
{
    CARBON_NON_NULL_OR_ERROR(bitmap);

    carbon_alloc_t alloc;
    carbon_alloc_create_std(&alloc);
    carbon_vec_create(&bitmap->data, &alloc, sizeof(uint32_t), ceil(num_bits / (double) CARBON_NUM_BITS(uint32_t)));
    size_t cap = VectorCapacity(&bitmap->data);
    uint32_t zero = 0;
    carbon_vec_repeated_push(&bitmap->data, &zero, cap);
    bitmap->num_bits = num_bits;

    return true;
}

CARBON_EXPORT(bool)
carbon_bitmap_cpy(carbon_bitmap_t *dst, const carbon_bitmap_t *src)
{
    dst->num_bits = src->num_bits;
    return VectorCpy(&dst->data, &src->data);
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
    memset(data, 0, sizeof(uint32_t) * VectorCapacity(&bitset->data));
    return true;
}

CARBON_EXPORT(bool)
carbon_bitmap_set(carbon_bitmap_t *bitset, uint16_t bit_position, bool on)
{
    CARBON_NON_NULL_OR_ERROR(bitset)
    size_t blockPos = floor(bit_position / (double) CARBON_NUM_BITS(uint32_t));
    size_t blockBit = bit_position % CARBON_NUM_BITS(uint32_t);
    uint32_t block = *VECTOR_GET(&bitset->data, blockPos, uint32_t);
    uint32_t mask = CARBON_SET_BIT(blockBit);
    if (on) {
        CARBON_FIELD_SET(block, mask);
    }
    else {
        CARBON_FIELD_CLEAR(block, mask);
    }
    VectorSet(&bitset->data, blockPos, &block);
    return true;
}

bool carbon_bitmap_get(carbon_bitmap_t *bitset, uint16_t bitPosition)
{
    CARBON_NON_NULL_OR_ERROR(bitset)
    size_t blockPos = floor(bitPosition / (double) CARBON_NUM_BITS(uint32_t));
    size_t blockBit = bitPosition % CARBON_NUM_BITS(uint32_t);
    uint32_t block = *VECTOR_GET(&bitset->data, blockPos, uint32_t);
    uint32_t mask = CARBON_SET_BIT(blockBit);
    return ((mask & block) >> bitPosition) == true;
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

void carbon_bitmap_print_bits(FILE *file, uint32_t n)
{
    for (int i = 31; i >= 0; i--) {
        uint32_t mask = 1 << i;
        uint32_t k = n & mask;
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

bool carbon_bitmap_blocks(uint32_t **blocks, uint32_t *numBlocks, const carbon_bitmap_t *carbon_bitmap_t)
{
    CARBON_NON_NULL_OR_ERROR(blocks)
    CARBON_NON_NULL_OR_ERROR(numBlocks)
    CARBON_NON_NULL_OR_ERROR(carbon_bitmap_t)

    uint32_t *result = malloc(carbon_bitmap_t->data.numElems * sizeof(uint32_t));
    int32_t k = 0;
    for (int32_t i = carbon_bitmap_t->data.numElems - 1; i >= 0; i--) {
        result[k++] = *VECTOR_GET(&carbon_bitmap_t->data, i, uint32_t);
    }
    *blocks = result;
    *numBlocks = carbon_bitmap_t->data.numElems;

    return true;
}

bool carbon_bitmap_print(FILE *file, const carbon_bitmap_t *carbon_bitmap_t)
{
    CARBON_NON_NULL_OR_ERROR(carbon_bitmap_t)

    uint32_t *blocks, numBlocks;

    carbon_bitmap_blocks(&blocks, &numBlocks, carbon_bitmap_t);

    for (uint32_t i = 0; i < numBlocks; i++) {
        fprintf(file, " %"PRIu32 " |", blocks[i]);
    }

    free(blocks);

    for (int32_t i = carbon_bitmap_t->data.numElems - 1; i >= 0; i--) {
        uint32_t block = *VECTOR_GET(&carbon_bitmap_t->data, i, uint32_t);
        carbon_bitmap_print_bits(stdout, block);
        fprintf(file, " |");
    }

    fprintf(file, "\n");
    return true;
}