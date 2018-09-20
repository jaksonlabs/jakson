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

int VectorCreate(Vector *out, const Allocator *alloc, size_t elem_size, size_t cap_elems)
{
    CHECK_NON_NULL(out)
    out->allocator = malloc(sizeof(Allocator));
    AllocatorThisOrDefault(out->allocator, alloc);
    out->base = AllocatorMalloc(out->allocator, cap_elems * elem_size);
    out->num_elems = 0;
    out->cap_elems = cap_elems;
    out->elem_size = elem_size;
    out->grow_factor = 1.7f;
    return STATUS_OK;
}

int ng5_vector_advise(Vector *vec, int madvise_advice)
{
    CHECK_NON_NULL(vec);
    UNUSED(vec);
    UNUSED(madvise_advice);
    madvise(vec->base, vec->cap_elems * vec->elem_size, madvise_advice);
    return STATUS_OK;
}

int ng5_vector_set_growfactor(Vector *vec, float factor)
{
    CHECK_NON_NULL(vec);
    CHECK_LARGER_ONE(factor);
    vec->grow_factor = factor;
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

int ng5_vector_is_empty(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->num_elems == 0 ? STATUS_TRUE : STATUS_FALSE;
}

int VectorPush(Vector *vec, const void *data, size_t num_elems)
{
    CHECK_NON_NULL(vec && data)
    size_t next_num = vec->num_elems + num_elems;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
    vec->num_elems += num_elems;
    return STATUS_OK;
}

// TODO: remove
//void *ng5_vector_push_and_get(Vector* vec, const void* data, size_t num_elems)
//{
//    size_t pos    = ng5_vector_len(vec);
//    int    status = VectorPush(vec, data, num_elems);
//    return status == STATUS_OK ? (void *) ng5_vector_at(vec, pos) : NULL;
//}

int VectorRepreatedPush(Vector *vec, const void *data, size_t how_many)
{
    CHECK_NON_NULL(vec && data)
    size_t next_num = vec->num_elems + how_many;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    for (size_t i = 0; i < how_many; i++) {
        memcpy(vec->base + (vec->num_elems + i) * vec->elem_size, data, vec->elem_size);
    }

    vec->num_elems += how_many;
    return STATUS_OK;
}

const void *ng5_vector_pop(Vector *vec)
{
    void *result;
    if (BRANCH_LIKELY((result = (vec ? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elem_size : NULL) : NULL))
                   != NULL)) {
        vec->num_elems--;
    }
    return result;
}

int ng5_vector_grow(size_t *num_new_slots, Vector *vec)
{
    CHECK_NON_NULL(vec)
    size_t free_slots_before = vec->cap_elems - vec->num_elems;

    vec->cap_elems = (vec->cap_elems * vec->grow_factor) + 1;
    vec->base = AllocatorRealloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    size_t free_slots_after = vec->cap_elems - vec->num_elems;
    if (BRANCH_LIKELY(num_new_slots != NULL)) {
        *num_new_slots = free_slots_after - free_slots_before;
    }
    return STATUS_OK;
}

size_t ng5_vector_len(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->num_elems;
}

const void *ng5_vector_at(const Vector *vec, size_t pos)
{
    return (vec && pos < vec->num_elems) ? vec->base + pos * vec->elem_size : NULL;
}

size_t VectorCapacity(const Vector *vec)
{
    CHECK_NON_NULL(vec)
    return vec->cap_elems;
}

int ng5_vector_enlarge_size(Vector *vec)
{
    CHECK_NON_NULL(vec);
    vec->num_elems = vec->cap_elems;
    return STATUS_OK;
}

int ng5_vector_set(Vector *vec, size_t pos, const void *data)
{
    CHECK_NON_NULL(vec)
    assert(pos < vec->num_elems);
    memcpy(vec->base + pos * vec->elem_size, data, vec->elem_size);
    return STATUS_OK;
}

const void *ng5_vector_data(const Vector *vec)
{
    return vec ? vec->base : NULL;
}