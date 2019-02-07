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

#ifndef CARBON_SLOTVECTOR_H
#define CARBON_SLOTVECTOR_H

#include "carbon-common.h"
#include "carbon-vector.h"

CARBON_BEGIN_DECL

typedef size_t SlotVectorSlot;

typedef struct SlotVector
{
    carbon_vec_t ofType(T) content;
    carbon_vec_t ofType(slot_vector_slot_t) freeList;
    carbon_err_t err;
} SlotVector;

CARBON_EXPORT(bool)
SlotVectorCreate(SlotVector *vector, const carbon_alloc_t *alloc, size_t elemSize, size_t capElems);

CARBON_EXPORT(bool)
SlotVectorGetError(carbon_err_t *err, const SlotVector *vector);

CARBON_EXPORT(bool)
SlotVectorSetGrowFactor(SlotVector *vec, float factor);

CARBON_EXPORT(bool)
SlotVectorDrop(SlotVector *vec);

CARBON_EXPORT(bool)
SlotVectorIsEmpty(SlotVector *vec);

CARBON_EXPORT(bool)
SlotVectorInsert(SlotVector *vec, CARBON_NULLABLE carbon_vec_t ofType(SlotVectorSlot) *ids,
                     const void *data, size_t numElems);

CARBON_EXPORT(const void *)
SlotVectorAt(SlotVector *vec, SlotVectorSlot slot);

CARBON_EXPORT(bool)
SlotVectorRemove(SlotVector *vec, SlotVectorSlot slot);

CARBON_EXPORT(size_t)
SlotVectorLength(const SlotVector *vec);

CARBON_EXPORT(size_t)
SlotVectorCapacity(const SlotVector *vec);

CARBON_END_DECL

#endif