/**
 * Copyright 2019 Marcus Pinnecke
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

#include <carbon/carbon-hash.h>
#include <carbon/carbon-hashtable.h>

#define HASHCODE_OF(size, x) CARBON_HASH_BERNSTEIN(size, x)
#define FIX_MAP_AUTO_REHASH_LOADFACTOR 0.9f

CARBON_EXPORT(bool)
carbon_hashtable_create(carbon_hashtable_t *map, carbon_err_t *err, size_t key_size, size_t value_size, size_t capacity)
{
    CARBON_NON_NULL_OR_ERROR(map)
    CARBON_NON_NULL_OR_ERROR(key_size)
    CARBON_NON_NULL_OR_ERROR(value_size)

    int err_code = CARBON_ERR_INITFAILED;

    map->size = 0;

    CARBON_SUCCESS_OR_JUMP(carbon_vec_create(&map->key_data, NULL, key_size, capacity),
                           error_handling);
    CARBON_SUCCESS_OR_JUMP(carbon_vec_create(&map->value_data, NULL, value_size, capacity),
                           cleanup_key_data_and_error);
    CARBON_SUCCESS_OR_JUMP(carbon_vec_create(&map->table, NULL, sizeof(carbon_hashtable_bucket_t), capacity),
                           cleanup_value_key_data_and_error);
    CARBON_SUCCESS_OR_JUMP(carbon_vec_enlarge_size_to_capacity(&map->table),
                           cleanup_key_value_table_and_error);
    CARBON_SUCCESS_OR_JUMP(carbon_vec_zero_memory(&map->table),
                           cleanup_key_value_table_and_error);
    CARBON_SUCCESS_OR_JUMP(carbon_spinlock_init(&map->lock),
                           cleanup_key_value_table_and_error);
    CARBON_SUCCESS_OR_JUMP(carbon_error_init(&map->err),
                           cleanup_key_value_table_and_error);

    return true;

cleanup_key_value_table_and_error:
    if (!carbon_vec_drop(&map->table)) {
        err_code = CARBON_ERR_DROPFAILED;
    }
cleanup_value_key_data_and_error:
    if (!carbon_vec_drop(&map->value_data)) {
        err_code = CARBON_ERR_DROPFAILED;
    }
cleanup_key_data_and_error:
    if (!carbon_vec_drop(&map->key_data)) {
        err_code = CARBON_ERR_DROPFAILED;
    }
error_handling:
    CARBON_ERROR(err, err_code);
    return false;
}

CARBON_EXPORT(bool)
carbon_hashtable_drop(carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(map)

    bool status = true;

    status &= carbon_vec_drop(&map->table);
    status &= carbon_vec_drop(&map->value_data);
    status &= carbon_vec_drop(&map->key_data);

    if (!status) {
        CARBON_ERROR(&map->err, CARBON_ERR_DROPFAILED);
    }

    return status;
}

CARBON_EXPORT(carbon_hashtable_t *)
carbon_hashtable_cpy(carbon_hashtable_t *src)
{
    if(src)
    {
        carbon_hashtable_t *cpy = malloc(sizeof(carbon_hashtable_t));

        carbon_hashtable_lock(src);

        carbon_hashtable_create(cpy, &src->err, src->key_data.elem_size, src->value_data.elem_size,
                                src->table.cap_elems);

        assert(src->key_data.cap_elems == src->value_data.cap_elems && src->value_data.cap_elems == src->table.cap_elems);
        assert((src->key_data.num_elems == src->value_data.num_elems) && src->value_data.num_elems <= src->table.num_elems);

        carbon_vec_cpy_to(&cpy->key_data, &src->key_data);
        carbon_vec_cpy_to(&cpy->value_data, &src->value_data);
        carbon_vec_cpy_to(&cpy->table, &src->table);
        cpy->size = src->size;
        carbon_error_cpy(&cpy->err, &src->err);

        assert(cpy->key_data.cap_elems == src->value_data.cap_elems && src->value_data.cap_elems == cpy->table.cap_elems);
        assert((cpy->key_data.num_elems == src->value_data.num_elems) && src->value_data.num_elems <= cpy->table.num_elems);

        carbon_hashtable_unlock(src);
        return cpy;
    } else
    {
        CARBON_ERROR(&src->err, CARBON_ERR_NULLPTR);
        return NULL;
    }
}

CARBON_EXPORT(bool)
carbon_hashtable_clear(carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(map)
    assert(map->key_data.cap_elems == map->value_data.cap_elems && map->value_data.cap_elems == map->table.cap_elems);
    assert((map->key_data.num_elems == map->value_data.num_elems) && map->value_data.num_elems <= map->table.num_elems);

    carbon_hashtable_lock(map);

    bool     status   = carbon_vec_clear(&map->key_data) &&
                        carbon_vec_clear(&map->value_data) &&
                        carbon_vec_zero_memory(&map->table);

    map->size = 0;

    assert(map->key_data.cap_elems == map->value_data.cap_elems && map->value_data.cap_elems == map->table.cap_elems);
    assert((map->key_data.num_elems == map->value_data.num_elems) && map->value_data.num_elems <= map->table.num_elems);

    if (!status) {
        CARBON_ERROR(&map->err, CARBON_ERR_OPPFAILED);
    }

    carbon_hashtable_unlock(map);

    return status;
}

CARBON_EXPORT(bool)
carbon_hashtable_avg_displace(float *displace, const carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(displace);
    CARBON_NON_NULL_OR_ERROR(map);

    size_t sum_dis = 0;
    for (size_t i = 0; i < map->table.num_elems; i++) {
        carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, i, carbon_hashtable_bucket_t);
        sum_dis += abs(bucket->displacement);
    }
    *displace = (sum_dis / (float) map->table.num_elems);

    return true;
}

CARBON_EXPORT(bool)
carbon_hashtable_lock(carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(map)
    carbon_spinlock_acquire(&map->lock);
    return true;
}

CARBON_EXPORT(bool)
carbon_hashtable_unlock(carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(map)
    carbon_spinlock_release(&map->lock);
    return true;
}

static inline const void *
get_bucket_key(const carbon_hashtable_bucket_t *bucket, const carbon_hashtable_t *map)
{
    return map->key_data.base + bucket->data_idx * map->key_data.elem_size;
}

static inline const void *
get_bucket_value(const carbon_hashtable_bucket_t *bucket, const carbon_hashtable_t *map)
{
    return map->value_data.base + bucket->data_idx * map->value_data.elem_size;
}

static void
insert(carbon_hashtable_bucket_t *bucket, carbon_hashtable_t *map, const void *key, const void *value, int32_t displacement)
{
    assert(map->key_data.num_elems == map->value_data.num_elems);
    uint64_t idx = map->key_data.num_elems;
    void *key_datum = VECTOR_NEW_AND_GET(&map->key_data, void *);
    void *value_datum = VECTOR_NEW_AND_GET(&map->value_data, void *);
    memcpy(key_datum, key, map->key_data.elem_size);
    memcpy(value_datum, value, map->value_data.elem_size);
    bucket->data_idx = idx;
    bucket->in_use_flag = true;
    bucket->displacement = displacement;
    map->size++;
}

static inline uint_fast32_t
insert_or_update(carbon_hashtable_t *map, const uint32_t *bucket_idxs, const void *keys, const void *values,
                 uint_fast32_t num_pairs)
{
    for (uint_fast32_t i = 0; i < num_pairs; i++)
    {
        const void *key = keys + i * map->key_data.elem_size;
        const void *value = values + i * map->key_data.elem_size;
        uint32_t intended_bucket_idx = bucket_idxs[i];

        uint32_t bucket_idx = intended_bucket_idx;

        carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, bucket_idx, carbon_hashtable_bucket_t);
        if (bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) != 0) {
            bool fitting_bucket_found = false;
            uint32_t displace_idx;
            for (displace_idx = bucket_idx + 1; displace_idx < map->table.num_elems; displace_idx++)
            {
                carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, displace_idx, carbon_hashtable_bucket_t);
                fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0);
                if (fitting_bucket_found) {
                    break;
                } else {
                    int32_t displacement = displace_idx - bucket_idx;
                    const void *swap_key = get_bucket_key(bucket, map);
                    const void *swap_value = get_bucket_value(bucket, map);

                    if (bucket->displacement < displacement) {
                        insert(bucket, map, key, value, displacement);
                        insert_or_update(map, &displace_idx, swap_key, swap_value, 1);
                        goto next_round;
                    }
                }
            }
            if (!fitting_bucket_found) {
                for (displace_idx = 0; displace_idx < bucket_idx - 1; displace_idx++)
                {
                    const carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, displace_idx, carbon_hashtable_bucket_t);
                    fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0);
                    if (fitting_bucket_found) {
                        break;
                    }
                }
            }

            assert(fitting_bucket_found == true);
            bucket_idx = displace_idx;
            bucket = CARBON_VECTOR_GET(&map->table, bucket_idx, carbon_hashtable_bucket_t);
        }

        bool is_update = bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
        if (is_update) {
            void *bucket_value = (void *) get_bucket_value(bucket, map);
            memcpy(bucket_value, value, map->value_data.elem_size);
        } else {
            int32_t displacement = intended_bucket_idx - bucket_idx;
            insert(bucket, map, key, value, displacement);
        }

        next_round:
        if (map->size >= FIX_MAP_AUTO_REHASH_LOADFACTOR * map->table.cap_elems)
        {
            return i + 1; /* tell the caller that pair i was inserted, but it successors not */
        }

    }
    return 0;
}

CARBON_EXPORT(bool)
carbon_hashtable_insert_or_update(carbon_hashtable_t *map, const void *keys, const void *values, uint_fast32_t num_pairs)
{
    CARBON_NON_NULL_OR_ERROR(map)
    CARBON_NON_NULL_OR_ERROR(keys)
    CARBON_NON_NULL_OR_ERROR(values)

    assert(map->key_data.cap_elems == map->value_data.cap_elems && map->value_data.cap_elems == map->table.cap_elems);
    assert((map->key_data.num_elems == map->value_data.num_elems) && map->value_data.num_elems <= map->table.num_elems);

    carbon_hashtable_lock(map);

    uint32_t *bucket_idxs = malloc(num_pairs * sizeof(uint32_t));
    if (!bucket_idxs)
    {
        CARBON_ERROR(&map->err, CARBON_ERR_MALLOCERR);
        return false;
    }

    for (uint_fast32_t i = 0; i < num_pairs; i++)
    {
        const void *key = keys + i * map->key_data.elem_size;
        carbon_hash32_t hash = HASHCODE_OF(map->key_data.elem_size, key);
        bucket_idxs[i] = hash % map->table.num_elems;
    }

    uint_fast32_t cont_idx = 0;
    do {
        cont_idx = insert_or_update(map,
                                    bucket_idxs + cont_idx,
                                    keys + cont_idx * map->key_data.elem_size,
                                    values + cont_idx * map->value_data.elem_size,
                                    num_pairs - cont_idx);
        if (cont_idx != 0) {
            /* rehashing is required, and [status, num_pairs) are left to be inserted */
            if (!carbon_hashtable_rehash(map)) {
                carbon_hashtable_unlock(map);
                return false;
            }
        }
    } while (cont_idx != 0);

    free(bucket_idxs);
    carbon_hashtable_unlock(map);

    return true;
}

CARBON_EXPORT(bool)
carbon_hashtable_remove_if_contained(carbon_hashtable_t *map, const void *keys, size_t num_pairs)
{
    CARBON_NON_NULL_OR_ERROR(map)
    CARBON_NON_NULL_OR_ERROR(keys)

    carbon_hashtable_lock(map);

    uint32_t *bucket_idxs = malloc(num_pairs * sizeof(uint32_t));
    if (!bucket_idxs)
    {
        CARBON_ERROR(&map->err, CARBON_ERR_MALLOCERR);
        carbon_hashtable_unlock(map);
        return false;
    }

    for (uint_fast32_t i = 0; i < num_pairs; i++)
    {
        const void *key = keys + i * map->key_data.elem_size;
        bucket_idxs[i] = HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
    }

    for (uint_fast32_t i = 0; i < num_pairs; i++)
    {
        const void *key = keys + i * map->key_data.elem_size;
        uint32_t bucket_idx = bucket_idxs[i];
        uint32_t actual_idx = bucket_idx;
        bool bucket_found = false;

        for (uint32_t k = bucket_idx; !bucket_found && k < map->table.num_elems; k++)
        {
            const carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, k, carbon_hashtable_bucket_t);
            bucket_found = bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
            actual_idx = k;
        }
        for (uint32_t k = 0; !bucket_found && k < bucket_idx; k++)
        {
            const carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, k, carbon_hashtable_bucket_t);
            bucket_found = bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
            actual_idx = k;
        }

        if (bucket_found) {
            carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, actual_idx, carbon_hashtable_bucket_t);
            bucket->in_use_flag = false;
            bucket->data_idx = 0;
            bucket->num_probs = 0;
        }
    }

    free(bucket_idxs);

    carbon_hashtable_unlock(map);

    return true;
}

CARBON_EXPORT(const void *)
carbon_hashtable_get_value(carbon_hashtable_t *map, const void *key)
{
    CARBON_NON_NULL_OR_ERROR(map)
    CARBON_NON_NULL_OR_ERROR(key)

    const void *result = NULL;

    carbon_hashtable_lock(map);

    uint32_t bucket_idx = HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
    uint32_t actual_idx = bucket_idx;
    bool bucket_found = false;

    for (uint32_t k = bucket_idx; !bucket_found && k < map->table.num_elems; k++)
    {
        const carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, k, carbon_hashtable_bucket_t);
        bucket_found = bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
        actual_idx = k;
    }
    for (uint32_t k = 0; !bucket_found && k < bucket_idx; k++)
    {
        const carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, k, carbon_hashtable_bucket_t);
        bucket_found = bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
        actual_idx = k;
    }

    if (bucket_found) {
        carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&map->table, actual_idx, carbon_hashtable_bucket_t);
        result = get_bucket_value(bucket, map);
    }

    carbon_hashtable_unlock(map);

    return result;
}

CARBON_EXPORT(bool)
carbon_hashtable_get_fload_factor(float *factor, carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(factor)
    CARBON_NON_NULL_OR_ERROR(map)

    carbon_hashtable_lock(map);

    *factor = map->size / (float) map->table.num_elems;

    carbon_hashtable_unlock(map);

    return true;
}

CARBON_EXPORT(bool)
carbon_hashtable_rehash(carbon_hashtable_t *map)
{
    CARBON_NON_NULL_OR_ERROR(map)

    carbon_hashtable_lock(map);

    carbon_hashtable_t *cpy = carbon_hashtable_cpy(map);
    carbon_hashtable_clear(map);

    size_t new_cap = (cpy->key_data.cap_elems + 1) * 1.7f;

    carbon_vec_grow_to(&map->key_data, new_cap);
    carbon_vec_grow_to(&map->value_data, new_cap);
    carbon_vec_grow_to(&map->table, new_cap);
    carbon_vec_enlarge_size_to_capacity(&map->table);
    carbon_vec_zero_memory(&map->table);

    assert(map->key_data.cap_elems == map->value_data.cap_elems && map->value_data.cap_elems == map->table.cap_elems);
    assert((map->key_data.num_elems == map->value_data.num_elems) && map->value_data.num_elems <= map->table.num_elems);

    for (size_t i = 0; i < cpy->table.num_elems; i++) {
        carbon_hashtable_bucket_t *bucket = CARBON_VECTOR_GET(&cpy->table, i, carbon_hashtable_bucket_t);
        if (bucket->in_use_flag) {
            const void *old_key = get_bucket_key(bucket, cpy);
            const void *old_value = get_bucket_value(bucket, cpy);
            if (!carbon_hashtable_insert_or_update(map, old_key, old_value, 1)) {
                CARBON_ERROR(&map->err, CARBON_ERR_REHASH_NOROLLBACK)
                carbon_hashtable_unlock(map);
                return false;
            }
        }
    }

    carbon_hashtable_unlock(map);
    return true;
}