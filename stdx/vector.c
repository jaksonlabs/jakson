#include <stdx/vector.h>
#include <sys/mman.h>

int vector_create(struct vector *out, const struct allocator *alloc, size_t elem_size, size_t cap_elems)
{
    check_non_null(out)
    allocator_this_or_default(&out->allocator, alloc);
    out->base = allocator_malloc(&out->allocator, cap_elems * elem_size);
    out->num_elems   = 0;
    out->cap_elems   = cap_elems;
    out->elem_size   = elem_size;
    out->grow_factor = 1.7f;
    return STATUS_OK;
}

int vector_madvise(struct vector *vec, int madvise_advice)
{
    check_non_null(vec);
    madvise(vec->base, vec->cap_elems * vec->elem_size, madvise_advice);
    return STATUS_OK;
}

int vector_set_growfactor(struct vector *vec, float factor)
{
    check_non_null(vec);
    check_larger_one(factor);
    vec->grow_factor = factor;
    return STATUS_OK;
}

int vector_drop(struct vector *vec)
{
    check_non_null(vec)
    allocator_free(&vec->allocator, vec->base);
    allocator_drop(&vec->allocator);
    vec->base = NULL;
    return STATUS_OK;
}

int vector_is_empty(struct vector *vec)
{
    check_non_null(vec)
    return vec->num_elems == 0 ? STATUS_TRUE : STATUS_FALSE;
}

int vector_push(struct vector *vec, const void *data, size_t num_elems)
{
    check_non_null(vec && data)
    size_t next_num = vec->num_elems + num_elems;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = allocator_realloc(&vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
    vec->num_elems += num_elems;
    return STATUS_OK;
}

int vector_repreat_push(struct vector *vec, const void *data, size_t how_many)
{
    check_non_null(vec && data)
    size_t next_num = vec->num_elems + how_many;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = allocator_realloc(&vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    for (size_t i = 0; i < how_many; i++) {
        memcpy(vec->base + (vec->num_elems + i) * vec->elem_size, data, vec->elem_size);
    }

    vec->num_elems += how_many;
    return STATUS_OK;
}

const void *vector_pop(struct vector *vec)
{
    void *result;
    if (likely((result = (vec? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elem_size : NULL) : NULL)) != NULL)) {
        vec->num_elems--;
    }
    return result;
}

int vector_grow(size_t *num_new_slots, struct vector *vec)
{
    check_non_null(vec)
    size_t free_slots_before = vec->cap_elems - vec->num_elems;
    vec->cap_elems          *= vec->grow_factor;
    vec->base = allocator_realloc(&vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    size_t free_slots_after  = vec->cap_elems - vec->num_elems;
    if (likely(num_new_slots != NULL)) {
        *num_new_slots = free_slots_after - free_slots_before;
    }
    return STATUS_OK;
}

size_t vector_len(const struct vector *vec)
{
    check_non_null(vec)
    return vec->num_elems;
}

size_t vector_cap(const struct vector *vec)
{
    check_non_null(vec)
    return vec->cap_elems;
}

const void *vector_data(struct vector *vec)
{
    return vec ? vec->base : NULL;
}