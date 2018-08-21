#include <stdx/slot_vector.h>

int slot_vector_create(struct slot_vector *vector, const struct allocator *alloc, size_t elem_size, size_t cap_elems)
{
    check_non_null(vector)
    check_non_null(alloc)
    check_success(vector_create(&vector->content, alloc, elem_size, cap_elems));
    check_success(vector_create(&vector->freelist, alloc, sizeof(slot_vector_slot_t), cap_elems));
    return STATUS_OK;
}

int slot_vector_set_growfactor(struct slot_vector *vec, float factor)
{
    check_non_null(vec)
    check_success(vector_set_growfactor(&vec->content, factor));
    check_success(vector_set_growfactor(&vec->freelist, factor));
    return STATUS_OK;
}

int slot_vector_drop(struct slot_vector *vec)
{
    check_non_null(vec)
    check_success(vector_drop(&vec->content));
    check_success(vector_drop(&vec->freelist));
    return STATUS_OK;
}

int slot_vector_is_empty(struct slot_vector *vec)
{
    check_non_null(vec)
    return vector_is_empty(&vec->content);
}

int slot_vector_insert(struct slot_vector *vec, optional struct vector of_type(slot_vector_slot_t) *ids,
        const void *data, size_t num_elems)
{
    check_non_null(vec)
    check_non_null(ids)
    check_non_null(data)

    unused(data); // TODO: ???

    if (ids) {
        vector_create(ids, &vec->content.allocator, sizeof(slot_vector_slot_t), num_elems);
    }

    /* check and handle whether the content vector must be resized */
    size_t target = vector_len(&vec->content) + num_elems;
    if (target > vector_cap(&vec->content)) {
        assert(vector_cap(&vec->freelist) == vector_cap(&vec->content));
        size_t required = target - vector_cap(&vec->content);
        size_t has      = 0;
        size_t add      = 0;
        while (has < required) {
            vector_grow(&add, &vec->freelist);
            vector_grow(NULL, &vec->content);
            has += add;
        }
        assert(vector_cap(&vec->freelist) == vector_cap(&vec->content));
        vector_enlarge_size(&vec->content);
        slot_vector_slot_t next_slot = vector_len(&vec->content);
        while (has--) {
            vector_push(&vec->freelist, &next_slot, 1);
            next_slot++;
        }
    }

    /* perform insert */
    assert(vector_len(&vec->freelist) > 0);
    slot_vector_slot_t slot = *(slot_vector_slot_t *) vector_pop(&vec->freelist);
    vector_at(&vec->content, slot);
    if (ids) {
        vector_push(ids, &slot, 1);
    }

    return STATUS_OK;
}

const void *slot_vector_at(struct slot_vector *vec, slot_vector_slot_t slot)
{
    if (!vec || slot >= vector_len(&vec->content)) {
        return NULL;
    } else {
        return vec->content.base + slot * vec->content.elem_size;
    }
}

int slot_vector_remove(struct slot_vector *vec, slot_vector_slot_t slot)
{
    check_non_null(vec)
    if (slot >= vector_len(&vec->content)) {
        return STATUS_ILLEGALARG;
    } else {
        vector_push(&vec->freelist, &slot, 1);
        return STATUS_OK;
    }
}

size_t slot_vector_len(const struct slot_vector *vec)
{
    check_non_null(vec);
    return vector_len(&vec->content);
}

size_t slot_vector_cap(const struct slot_vector *vec)
{
    check_non_null(vec);
    return vector_len(&vec->freelist);
}
