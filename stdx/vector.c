#include <stdx/vector.h>

int vector_create(struct vector *out, const struct allocator *alloc, size_t elem_size, size_t cap_elems)
{
    check_non_null(out && alloc)
    allocator_clone(&out->allocator, alloc);
    out->base = allocator_malloc(&out->allocator, elem_size);
    out->num_elems = 0;
    out->cap_elems = cap_elems;
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

int vector_push(struct vector *vec, const char *data, size_t num_elems)
{
    check_non_null(vec && data)
    size_t next_num = vec->num_elems + num_elems;
    if (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * 1.7f;
        vec->base = allocator_realloc(&vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
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