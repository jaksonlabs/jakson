// file: bitset.c

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "bitmap.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define NUM_BITS(x)             (sizeof(x) * 8)
#define SET_BIT(n)              ( ((uint64_t) 1) << (n) )
#define FIELD_SET(x, mask)      ( x |=  (mask) )
#define FIELD_CLEAR(x, mask)    ( x &= ~(mask) )

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M E P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int BitmapCreate(Bitmap *bitmap, uint16_t numBits)
{
    CHECK_NON_NULL(bitmap);

    Allocator alloc;
    AllocatorCreateDefault(&alloc);
    VectorCreate(&bitmap->data, &alloc, sizeof(uint64_t), ceil(numBits / (double) NUM_BITS(uint64_t)));
    size_t cap = VectorCapacity(&bitmap->data);
    uint64_t zero = 0;
    VectorRepreatedPush(&bitmap->data, &zero, cap);
    bitmap->numBits = numBits;

    return STATUS_OK;
}

int BitmapDrop(Bitmap *bitset)
{
    return VectorDrop(&bitset->data);
}

size_t BitmapNumBits(const Bitmap *bitset)
{
    CHECK_NON_NULL(bitset);
    return bitset->numBits;
}

int BitmapClear(Bitmap *bitset)
{
    CHECK_NON_NULL(bitset);
    void *data = (void *) VectorData(&bitset->data);
    memset(data, 0, sizeof(uint64_t) * VectorCapacity(&bitset->data));
    return STATUS_OK;
}

int BitmapSet(Bitmap *bitset, uint16_t bitPosition, bool on)
{
    CHECK_NON_NULL(bitset)
    size_t blockPos = floor(bitPosition / (double) NUM_BITS(uint64_t));
    size_t blockBit = bitPosition % NUM_BITS(uint64_t);
    uint64_t block = *VECTOR_GET(&bitset->data, blockPos, uint64_t);
    uint64_t mask = SET_BIT(blockBit);
    if (on) {
        FIELD_SET(block, mask);
    }
    else {
        FIELD_CLEAR(block, mask);
    }
    VectorSet(&bitset->data, blockPos, &block);
    return STATUS_OK;
}

bool BitmapGet(Bitmap *bitset, uint16_t bitPosition)
{
    CHECK_NON_NULL(bitset)
    size_t blockPos = floor(bitPosition / (double) NUM_BITS(uint64_t));
    size_t blockBit = bitPosition % NUM_BITS(uint64_t);
    uint64_t block = *VECTOR_GET(&bitset->data, blockPos, uint64_t);
    uint64_t mask = SET_BIT(blockBit);
    return ((mask & block) >> bitPosition) == true;
}