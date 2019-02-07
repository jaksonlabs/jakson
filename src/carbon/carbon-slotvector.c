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

#include "carbon/carbon-slotvector.h"

bool SlotVectorCreate(SlotVector *vector, const carbon_alloc_t *alloc, size_t elemSize, size_t capElems)
{
    CARBON_NON_NULL_OR_ERROR(vector)
    CARBON_NON_NULL_OR_ERROR(alloc)
    CARBON_CHECK_SUCCESS(carbon_vec_create(&vector->content, alloc, elemSize, capElems));
    CARBON_CHECK_SUCCESS(carbon_vec_create(&vector->freeList, alloc, sizeof(SlotVectorSlot), capElems));
    carbon_error_init(&vector->err);
    return true;
}

CARBON_EXPORT(bool)
SlotVectorGetError(carbon_err_t *err, const SlotVector *vector)
{
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(vector)
    carbon_error_cpy(err, &vector->err);
    return true;
}

bool SlotVectorSetGrowFactor(SlotVector *vec, float factor)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    CARBON_CHECK_SUCCESS(VectorSetGrowFactor(&vec->content, factor));
    CARBON_CHECK_SUCCESS(VectorSetGrowFactor(&vec->freeList, factor));
    return true;
}

bool SlotVectorDrop(SlotVector *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    CARBON_CHECK_SUCCESS(VectorDrop(&vec->content));
    CARBON_CHECK_SUCCESS(VectorDrop(&vec->freeList));
    return true;
}

bool SlotVectorIsEmpty(SlotVector *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return carbon_vec_is_empty(&vec->content);
}

bool SlotVectorInsert(SlotVector *vec, CARBON_NULLABLE carbon_vec_t ofType(slot_vector_slot_t) *ids,
                     const void *data, size_t numElems)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    CARBON_NON_NULL_OR_ERROR(ids)
    CARBON_NON_NULL_OR_ERROR(data)

    CARBON_UNUSED(data); // TODO: ???

    if (ids) {
        carbon_vec_create(ids, vec->content.allocator, sizeof(SlotVectorSlot), numElems);
    }

    /** check and handle whether the content CARBON_vector must be resized */
    size_t target = carbon_vec_length(&vec->content) + numElems;
    if (target > VectorCapacity(&vec->content)) {
        assert(VectorCapacity(&vec->freeList) == VectorCapacity(&vec->content));
        size_t required = target - VectorCapacity(&vec->content);
        size_t has = 0;
        size_t add = 0;
        while (has < required) {
            carbon_vec_grow(&add, &vec->freeList);
            carbon_vec_grow(NULL, &vec->content);
            has += add;
        }
        assert(VectorCapacity(&vec->freeList) == VectorCapacity(&vec->content));
        VectorEnlargeSizeToCapacity(&vec->content);
        SlotVectorSlot nextSlot = carbon_vec_length(&vec->content);
        while (has--) {
            carbon_vec_push(&vec->freeList, &nextSlot, 1);
            nextSlot++;
        }
    }

    /** perform insert */
    assert(carbon_vec_length(&vec->freeList) > 0);
    SlotVectorSlot slot = *(SlotVectorSlot *) carbon_vec_pop(&vec->freeList);
    VectorAt(&vec->content, slot);
    if (ids) {
        carbon_vec_push(ids, &slot, 1);
    }

    return true;
}

const void *SlotVectorAt(SlotVector *vec, SlotVectorSlot slot)
{
    if (!vec || slot >= carbon_vec_length(&vec->content)) {
        return NULL;
    }
    else {
        return vec->content.base + slot * vec->content.elemSize;
    }
}

bool SlotVectorRemove(SlotVector *vec, SlotVectorSlot slot)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    if (slot >= carbon_vec_length(&vec->content)) {
        CARBON_ERROR(&vec->err, CARBON_ERR_ILLEGALARG);
        return false;
    }
    else {
        carbon_vec_push(&vec->freeList, &slot, 1);
        return true;
    }
}

size_t SlotVectorLength(const SlotVector *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    return carbon_vec_length(&vec->content);
}

size_t SlotVectorCapacity(const SlotVector *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    return carbon_vec_length(&vec->freeList);
}
