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

int slot_vector_create(struct slot_vector *vector, const Allocator *alloc, size_t elem_size, size_t cap_elems)
{
    CHECK_NON_NULL(vector)
    CHECK_NON_NULL(alloc)
    CHECK_SUCCESS(VectorCreate(&vector->content, alloc, elem_size, cap_elems));
    CHECK_SUCCESS(VectorCreate(&vector->freelist, alloc, sizeof(slot_vector_slot_t), cap_elems));
    return STATUS_OK;
}

int slot_vector_set_growfactor(struct slot_vector *vec, float factor)
{
    CHECK_NON_NULL(vec)
    CHECK_SUCCESS(ng5_vector_set_growfactor(&vec->content, factor));
    CHECK_SUCCESS(ng5_vector_set_growfactor(&vec->freelist, factor));
    return STATUS_OK;
}

int slot_vector_drop(struct slot_vector *vec)
{
    CHECK_NON_NULL(vec)
    CHECK_SUCCESS(VectorDrop(&vec->content));
    CHECK_SUCCESS(VectorDrop(&vec->freelist));
    return STATUS_OK;
}

int slot_vector_is_empty(struct slot_vector *vec)
{
    CHECK_NON_NULL(vec)
    return ng5_vector_is_empty(&vec->content);
}

int slot_vector_insert(struct slot_vector *vec, optional Vector ofType(slot_vector_slot_t) *ids,
                       const void *data, size_t num_elems)
{
    CHECK_NON_NULL(vec)
    CHECK_NON_NULL(ids)
    CHECK_NON_NULL(data)

    UNUSED(data); // TODO: ???

    if (ids) {
        VectorCreate(ids, vec->content.allocator, sizeof(slot_vector_slot_t), num_elems);
    }

    /* check and handle whether the content ng5_vector must be resized */
    size_t target = ng5_vector_len(&vec->content) + num_elems;
    if (target > VectorCapacity(&vec->content)) {
        assert(VectorCapacity(&vec->freelist) == VectorCapacity(&vec->content));
        size_t required = target - VectorCapacity(&vec->content);
        size_t has = 0;
        size_t add = 0;
        while (has < required) {
            ng5_vector_grow(&add, &vec->freelist);
            ng5_vector_grow(NULL, &vec->content);
            has += add;
        }
        assert(VectorCapacity(&vec->freelist) == VectorCapacity(&vec->content));
        ng5_vector_enlarge_size(&vec->content);
        slot_vector_slot_t next_slot = ng5_vector_len(&vec->content);
        while (has--) {
            VectorPush(&vec->freelist, &next_slot, 1);
            next_slot++;
        }
    }

    /* perform insert */
    assert(ng5_vector_len(&vec->freelist) > 0);
    slot_vector_slot_t slot = *(slot_vector_slot_t *) ng5_vector_pop(&vec->freelist);
    ng5_vector_at(&vec->content, slot);
    if (ids) {
        VectorPush(ids, &slot, 1);
    }

    return STATUS_OK;
}

const void *slot_vector_at(struct slot_vector *vec, slot_vector_slot_t slot)
{
    if (!vec || slot >= ng5_vector_len(&vec->content)) {
        return NULL;
    }
    else {
        return vec->content.base + slot * vec->content.elem_size;
    }
}

int slot_vector_remove(struct slot_vector *vec, slot_vector_slot_t slot)
{
    CHECK_NON_NULL(vec)
    if (slot >= ng5_vector_len(&vec->content)) {
        return STATUS_ILLEGALARG;
    }
    else {
        VectorPush(&vec->freelist, &slot, 1);
        return STATUS_OK;
    }
}

size_t slot_vector_len(const struct slot_vector *vec)
{
    CHECK_NON_NULL(vec);
    return ng5_vector_len(&vec->content);
}

size_t slot_vector_cap(const struct slot_vector *vec)
{
    CHECK_NON_NULL(vec);
    return ng5_vector_len(&vec->freelist);
}
