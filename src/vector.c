// file: vector.c

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

#include <sys/mman.h>

#include "vector.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int VectorCreate(Vector *out, const Allocator *alloc, size_t elemSize, size_t capElems)
{
    CHECK_NON_NULL(out)
    out->allocator = malloc(sizeof(Allocator));
    AllocatorThisOrDefault(out->allocator, alloc);
    out->base = AllocatorMalloc(out->allocator, capElems * elemSize);
    out->numElems = 0;
    out->capElems = capElems;
    out->elemSize = elemSize;
    out->growFactor = 1.7f;
    return STATUS_OK;
}

int VectorMemoryAdvice(Vector *vec, int madviseAdvice)
{
    CHECK_NON_NULL(vec);
    UNUSED(vec);
    UNUSED(madviseAdvice);
    madvise(vec->base, vec->capElems * vec->elemSize, madviseAdvice);
    return STATUS_OK;
}

int VectorSetGrowFactor(Vector *vec, float factor)
{
    CHECK_NON_NULL(vec);
    CHECK_LARGER_ONE(factor);
    vec->growFactor = factor;
    return STATUS_OK;
}

int VectorDrop(Vector *vec)
{
    CHECK_NON_NULL(vec)
    AllocatorFree(vec->allocator, vec->base);
    free(vec->allocator);
    vec->base = NULL;
    return STATUS_OK;
}

int VectorIsEmpty(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->numElems == 0 ? STATUS_TRUE : STATUS_FALSE;
}

int VectorPush(Vector *vec, const void *data, size_t numElems)
{
    CHECK_NON_NULL(vec && data)
    size_t nextNum = vec->numElems + numElems;
    while (nextNum > vec->capElems) {
        size_t more = nextNum - vec->capElems;
        vec->capElems = (vec->capElems + more) * vec->growFactor;
        vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    }
    memcpy(vec->base + vec->numElems * vec->elemSize, data, numElems * vec->elemSize);
    vec->numElems += numElems;
    return STATUS_OK;
}

int VectorRepreatedPush(Vector *vec, const void *data, size_t howOften)
{
    CHECK_NON_NULL(vec && data)
    size_t nextNum = vec->numElems + howOften;
    while (nextNum > vec->capElems) {
        size_t more = nextNum - vec->capElems;
        vec->capElems = (vec->capElems + more) * vec->growFactor;
        vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    }
    for (size_t i = 0; i < howOften; i++) {
        memcpy(vec->base + (vec->numElems + i) * vec->elemSize, data, vec->elemSize);
    }

    vec->numElems += howOften;
    return STATUS_OK;
}

const void *VectorPop(Vector *vec)
{
    void *result;
    if (BRANCH_LIKELY((result = (vec ? (vec->numElems > 0 ? vec->base + (vec->numElems - 1) * vec->elemSize : NULL) : NULL))
                   != NULL)) {
        vec->numElems--;
    }
    return result;
}

int VectorGrow(size_t *numNewSlots, Vector *vec)
{
    CHECK_NON_NULL(vec)
    size_t freeSlotsBefore = vec->capElems - vec->numElems;

    vec->capElems = (vec->capElems * vec->growFactor) + 1;
    vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    size_t freeSlotsAfter = vec->capElems - vec->numElems;
    if (BRANCH_LIKELY(numNewSlots != NULL)) {
        *numNewSlots = freeSlotsAfter - freeSlotsBefore;
    }
    return STATUS_OK;
}

size_t VectorLength(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->numElems;
}

const void *VectorAt(const Vector *vec, size_t pos)
{
    return (vec && pos < vec->numElems) ? vec->base + pos * vec->elemSize : NULL;
}

size_t VectorCapacity(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->capElems;
}

int VectorEnlargeSizeToCapacity(Vector *vec)
{
    CHECK_NON_NULL(vec);
    vec->numElems = vec->capElems;
    return STATUS_OK;
}

int VectorSet(Vector *vec, size_t pos, const void *data)
{
    CHECK_NON_NULL(vec)
    assert(pos < vec->numElems);
    memcpy(vec->base + pos * vec->elemSize, data, vec->elemSize);
    return STATUS_OK;
}

const void *VectorData(const Vector *vec)
{
    return vec ? vec->base : NULL;
}
