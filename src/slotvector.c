// file: slotvector.c

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

#include "slotvector.h"

int SlotVectorCreate(SlotVector *vector, const Allocator *alloc, size_t elemSize, size_t capElems)
{
    CHECK_NON_NULL(vector)
    CHECK_NON_NULL(alloc)
    CHECK_SUCCESS(VectorCreate(&vector->content, alloc, elemSize, capElems));
    CHECK_SUCCESS(VectorCreate(&vector->freeList, alloc, sizeof(SlotVectorSlot), capElems));
    return STATUS_OK;
}

int SlotVectorSetGrowFactor(SlotVector *vec, float factor)
{
    CHECK_NON_NULL(vec)
    CHECK_SUCCESS(VectorSetGrowFactor(&vec->content, factor));
    CHECK_SUCCESS(VectorSetGrowFactor(&vec->freeList, factor));
    return STATUS_OK;
}

int SlotVectorDrop(SlotVector *vec)
{
    CHECK_NON_NULL(vec)
    CHECK_SUCCESS(VectorDrop(&vec->content));
    CHECK_SUCCESS(VectorDrop(&vec->freeList));
    return STATUS_OK;
}

int SlotVectorIsEmpty(SlotVector *vec)
{
    CHECK_NON_NULL(vec)
    return VectorIsEmpty(&vec->content);
}

int SlotVectorInsert(SlotVector *vec, optional Vector ofType(slot_vector_slot_t) *ids,
                     const void *data, size_t numElems)
{
    CHECK_NON_NULL(vec)
    CHECK_NON_NULL(ids)
    CHECK_NON_NULL(data)

    UNUSED(data); // TODO: ???

    if (ids) {
        VectorCreate(ids, vec->content.allocator, sizeof(SlotVectorSlot), numElems);
    }

    /* check and handle whether the content ng5_vector must be resized */
    size_t target = VectorLength(&vec->content) + numElems;
    if (target > VectorCapacity(&vec->content)) {
        assert(VectorCapacity(&vec->freeList) == VectorCapacity(&vec->content));
        size_t required = target - VectorCapacity(&vec->content);
        size_t has = 0;
        size_t add = 0;
        while (has < required) {
            VectorGrow(&add, &vec->freeList);
            VectorGrow(NULL, &vec->content);
            has += add;
        }
        assert(VectorCapacity(&vec->freeList) == VectorCapacity(&vec->content));
        VectorEnlargeSizeToCapacity(&vec->content);
        SlotVectorSlot nextSlot = VectorLength(&vec->content);
        while (has--) {
            VectorPush(&vec->freeList, &nextSlot, 1);
            nextSlot++;
        }
    }

    /* perform insert */
    assert(VectorLength(&vec->freeList) > 0);
    SlotVectorSlot slot = *(SlotVectorSlot *) VectorPop(&vec->freeList);
    VectorAt(&vec->content, slot);
    if (ids) {
        VectorPush(ids, &slot, 1);
    }

    return STATUS_OK;
}

const void *SlotVectorAt(SlotVector *vec, SlotVectorSlot slot)
{
    if (!vec || slot >= VectorLength(&vec->content)) {
        return NULL;
    }
    else {
        return vec->content.base + slot * vec->content.elemSize;
    }
}

int SlotVectorRemove(SlotVector *vec, SlotVectorSlot slot)
{
    CHECK_NON_NULL(vec)
    if (slot >= VectorLength(&vec->content)) {
        return STATUS_ILLEGALARG;
    }
    else {
        VectorPush(&vec->freeList, &slot, 1);
        return STATUS_OK;
    }
}

size_t SlotVectorLength(const SlotVector *vec)
{
    CHECK_NON_NULL(vec);
    return VectorLength(&vec->content);
}

size_t SlotVectorCapacity(const SlotVector *vec)
{
    CHECK_NON_NULL(vec);
    return VectorLength(&vec->freeList);
}
