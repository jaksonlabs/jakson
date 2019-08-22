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

#include <jak_hash.h>
#include <jak_hash_set.h>

#define HASHCODE_OF(size, x) JAK_HASH_BERNSTEIN(size, x)
#define FIX_MAP_AUTO_REHASH_LOADFACTOR 0.9f

bool jak_hashset_create(jak_hashset *map, jak_error *err, size_t key_size, size_t capacity)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(key_size)

        int err_code = JAK_ERR_INITFAILED;

        map->size = 0;

        JAK_SUCCESS_OR_JUMP(jak_vector_create(&map->key_data, NULL, key_size, capacity), error_handling);
        JAK_SUCCESS_OR_JUMP(jak_vector_create(&map->table, NULL, sizeof(jak_hashset_bucket), capacity),
                            cleanup_key_data_and_error);
        JAK_SUCCESS_OR_JUMP(jak_vector_enlarge_size_to_capacity(&map->table), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_vector_zero_memory(&map->table), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_spinlock_init(&map->lock), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_error_init(&map->err), cleanup_key_value_table_and_error);

        return true;

        cleanup_key_value_table_and_error:
        if (!jak_vector_drop(&map->table)) {
                err_code = JAK_ERR_DROPFAILED;
        }
        cleanup_key_data_and_error:
        if (!jak_vector_drop(&map->key_data)) {
                err_code = JAK_ERR_DROPFAILED;
        }
        error_handling:
        JAK_ERROR(err, err_code);
        return false;
}

bool jak_hashset_drop(jak_hashset *map)
{
        JAK_ERROR_IF_NULL(map)

        bool status = true;

        status &= jak_vector_drop(&map->table);
        status &= jak_vector_drop(&map->key_data);

        if (!status) {
                JAK_ERROR(&map->err, JAK_ERR_DROPFAILED);
        }

        return status;
}

jak_vector *jak_hashset_keys(jak_hashset *map)
{
        if (map) {
                jak_vector *result = JAK_MALLOC(sizeof(jak_vector));
                jak_vector_create(result, NULL, map->key_data.elem_size, map->key_data.num_elems);
                for (jak_u32 i = 0; i < map->table.num_elems; i++) {
                        jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, i, jak_hashset_bucket);
                        if (bucket->in_use_flag) {
                                const void *data = jak_vector_at(&map->key_data, bucket->key_idx);
                                jak_vector_push(result, data, 1);
                        }
                }
                return result;
        } else {
                return NULL;
        }
}

jak_hashset *jak_hashset_cpy(jak_hashset *src)
{
        if (src) {
                jak_hashset *cpy = JAK_MALLOC(sizeof(jak_hashset));

                jak_hashset_lock(src);

                jak_hashset_create(cpy, &src->err, src->key_data.elem_size, src->table.cap_elems);

                JAK_ASSERT(src->key_data.cap_elems == src->table.cap_elems);
                JAK_ASSERT(src->key_data.num_elems <= src->table.num_elems);

                jak_vector_cpy_to(&cpy->key_data, &src->key_data);
                jak_vector_cpy_to(&cpy->table, &src->table);
                cpy->size = src->size;
                jak_error_cpy(&cpy->err, &src->err);

                JAK_ASSERT(cpy->key_data.cap_elems == cpy->table.cap_elems);
                JAK_ASSERT(cpy->key_data.num_elems <= cpy->table.num_elems);

                jak_hashset_unlock(src);
                return cpy;
        } else {
                JAK_ERROR(&src->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_hashset_clear(jak_hashset *map)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ASSERT(map->key_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT(map->key_data.num_elems <= map->table.num_elems);

        jak_hashset_lock(map);

        bool status = jak_vector_clear(&map->key_data) && jak_vector_zero_memory(&map->table);

        map->size = 0;

        JAK_ASSERT(map->key_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT(map->key_data.num_elems <= map->table.num_elems);

        if (!status) {
                JAK_ERROR(&map->err, JAK_ERR_OPPFAILED);
        }

        jak_hashset_unlock(map);

        return status;
}

bool jak_hashset_avg_displace(float *displace, const jak_hashset *map)
{
        JAK_ERROR_IF_NULL(displace);
        JAK_ERROR_IF_NULL(map);

        size_t sum_dis = 0;
        for (size_t i = 0; i < map->table.num_elems; i++) {
                jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, i, jak_hashset_bucket);
                sum_dis += abs(bucket->displacement);
        }
        *displace = (sum_dis / (float) map->table.num_elems);

        return true;
}

bool jak_hashset_lock(jak_hashset *map)
{
        JAK_ERROR_IF_NULL(map)
        jak_spinlock_acquire(&map->lock);
        return true;
}

bool jak_hashset_unlock(jak_hashset *map)
{
        JAK_ERROR_IF_NULL(map)
        jak_spinlock_release(&map->lock);
        return true;
}

static inline const void *get_bucket_key(const jak_hashset_bucket *bucket, const jak_hashset *map)
{
        return map->key_data.base + bucket->key_idx * map->key_data.elem_size;
}

static void insert(jak_hashset_bucket *bucket, jak_hashset *map, const void *key, jak_i32 displacement)
{
        jak_u64 idx = map->key_data.num_elems;
        void *key_datum = JAK_VECTOR_NEW_AND_GET(&map->key_data, void *);
        memcpy(key_datum, key, map->key_data.elem_size);
        bucket->key_idx = idx;
        bucket->in_use_flag = true;
        bucket->displacement = displacement;
        map->size++;
}

static inline uint_fast32_t insert_or_update(jak_hashset *map, const jak_u32 *bucket_idxs, const void *keys,
                                             uint_fast32_t num_pairs)
{
        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                jak_u32 intended_bucket_idx = bucket_idxs[i];

                jak_u32 bucket_idx = intended_bucket_idx;

                jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, bucket_idx, jak_hashset_bucket);
                if (bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) != 0) {
                        bool fitting_bucket_found = false;
                        jak_u32 displace_idx;
                        for (displace_idx = bucket_idx + 1; displace_idx < map->table.num_elems; displace_idx++) {
                                jak_hashset_bucket
                                        *bucket = JAK_VECTOR_GET(&map->table, displace_idx, jak_hashset_bucket);
                                fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag
                                                                                &&
                                                                                memcmp(get_bucket_key(bucket, map), key,
                                                                                       map->key_data.elem_size) == 0);
                                if (fitting_bucket_found) {
                                        break;
                                } else {
                                        jak_i32 displacement = displace_idx - bucket_idx;
                                        const void *swap_key = get_bucket_key(bucket, map);

                                        if (bucket->displacement < displacement) {
                                                insert(bucket, map, key, displacement);
                                                insert_or_update(map, &displace_idx, swap_key, 1);
                                                goto next_round;
                                        }
                                }
                        }
                        if (!fitting_bucket_found) {
                                for (displace_idx = 0; displace_idx < bucket_idx - 1; displace_idx++) {
                                        const jak_hashset_bucket
                                                *bucket = JAK_VECTOR_GET(&map->table, displace_idx, jak_hashset_bucket);
                                        fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag
                                                                                        && memcmp(get_bucket_key(bucket,
                                                                                                                 map),
                                                                                                  key,
                                                                                                  map->key_data.elem_size)
                                                                                           == 0);
                                        if (fitting_bucket_found) {
                                                break;
                                        }
                                }
                        }

                        JAK_ASSERT(fitting_bucket_found == true);
                        bucket_idx = displace_idx;
                        bucket = JAK_VECTOR_GET(&map->table, bucket_idx, jak_hashset_bucket);
                }

                bool is_update =
                        bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                if (!is_update) {
                        jak_i32 displacement = intended_bucket_idx - bucket_idx;
                        insert(bucket, map, key, displacement);
                }

                next_round:
                if (map->size >= FIX_MAP_AUTO_REHASH_LOADFACTOR * map->table.cap_elems) {
                        return i + 1; /* tell the caller that pair i was inserted, but it successors not */
                }

        }
        return 0;
}

bool jak_hashset_insert_or_update(jak_hashset *map, const void *keys, uint_fast32_t num_pairs)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(keys)

        JAK_ASSERT(map->key_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT(map->key_data.num_elems <= map->table.num_elems);

        jak_hashset_lock(map);

        jak_u32 *bucket_idxs = JAK_MALLOC(num_pairs * sizeof(jak_u32));
        if (!bucket_idxs) {
                JAK_ERROR(&map->err, JAK_ERR_MALLOCERR);
                return false;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                hash32_t hash = HASHCODE_OF(map->key_data.elem_size, key);
                bucket_idxs[i] = hash % map->table.num_elems;
        }

        uint_fast32_t cont_idx = 0;
        do {
                cont_idx = insert_or_update(map,
                                            bucket_idxs + cont_idx,
                                            keys + cont_idx * map->key_data.elem_size,
                                            num_pairs - cont_idx);
                if (cont_idx != 0) {
                        /* rehashing is required, and [status, num_pairs) are left to be inserted */
                        if (!jak_hashset_rehash(map)) {
                                jak_hashset_unlock(map);
                                return false;
                        }
                }
        } while (cont_idx != 0);

        free(bucket_idxs);
        jak_hashset_unlock(map);

        return true;
}

bool jak_hashset_remove_if_contained(jak_hashset *map, const void *keys, size_t num_pairs)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(keys)

        jak_hashset_lock(map);

        jak_u32 *bucket_idxs = JAK_MALLOC(num_pairs * sizeof(jak_u32));
        if (!bucket_idxs) {
                JAK_ERROR(&map->err, JAK_ERR_MALLOCERR);
                jak_hashset_unlock(map);
                return false;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                bucket_idxs[i] = HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                jak_u32 bucket_idx = bucket_idxs[i];
                jak_u32 actual_idx = bucket_idx;
                bool bucket_found = false;

                for (jak_u32 k = bucket_idx; !bucket_found && k < map->table.num_elems; k++) {
                        const jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashset_bucket);
                        bucket_found = bucket->in_use_flag
                                       && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                        actual_idx = k;
                }
                for (jak_u32 k = 0; !bucket_found && k < bucket_idx; k++) {
                        const jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashset_bucket);
                        bucket_found = bucket->in_use_flag
                                       && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                        actual_idx = k;
                }

                if (bucket_found) {
                        jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, actual_idx, jak_hashset_bucket);
                        bucket->in_use_flag = false;
                        bucket->key_idx = 0;
                }
        }

        free(bucket_idxs);

        jak_hashset_unlock(map);

        return true;
}

bool jak_hashset_contains_key(jak_hashset *map, const void *key)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(key)

        bool result = false;

        jak_hashset_lock(map);

        jak_u32 bucket_idx = HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
        bool bucket_found = false;

        for (jak_u32 k = bucket_idx; !bucket_found && k < map->table.num_elems; k++) {
                const jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashset_bucket);
                bucket_found =
                        bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
        }
        for (jak_u32 k = 0; !bucket_found && k < bucket_idx; k++) {
                const jak_hashset_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashset_bucket);
                bucket_found =
                        bucket->in_use_flag && memcmp(get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
        }

        result = bucket_found;
        jak_hashset_unlock(map);

        return result;
}

bool jak_hashset_get_load_factor(float *factor, jak_hashset *map)
{
        JAK_ERROR_IF_NULL(factor)
        JAK_ERROR_IF_NULL(map)

        jak_hashset_lock(map);

        *factor = map->size / (float) map->table.num_elems;

        jak_hashset_unlock(map);

        return true;
}

bool jak_hashset_rehash(jak_hashset *map)
{
        JAK_ERROR_IF_NULL(map)

        jak_hashset_lock(map);

        jak_hashset *cpy = jak_hashset_cpy(map);
        jak_hashset_clear(map);

        size_t new_cap = (cpy->key_data.cap_elems + 1) * 1.7f;

        jak_vector_grow_to(&map->key_data, new_cap);
        jak_vector_grow_to(&map->table, new_cap);
        jak_vector_enlarge_size_to_capacity(&map->table);
        jak_vector_zero_memory(&map->table);

        JAK_ASSERT(map->key_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT(map->key_data.num_elems <= map->table.num_elems);

        for (size_t i = 0; i < cpy->table.num_elems; i++) {
                jak_hashset_bucket *bucket = JAK_VECTOR_GET(&cpy->table, i, jak_hashset_bucket);
                if (bucket->in_use_flag) {
                        const void *old_key = get_bucket_key(bucket, cpy);
                        if (!jak_hashset_insert_or_update(map, old_key, 1)) {
                                JAK_ERROR(&map->err, JAK_ERR_REHASH_NOROLLBACK)
                                jak_hashset_unlock(map);
                                return false;
                        }
                }
        }

        jak_hashset_unlock(map);
        return true;
}