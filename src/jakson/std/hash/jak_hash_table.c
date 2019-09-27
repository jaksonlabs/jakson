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

#include <jakson/std/jak_hash.h>
#include <jakson/std/hash/jak_hash_table.h>

#define _JAK_HASH_TABLE_HASHCODE_OF(size, x) JAK_HASH_BERNSTEIN(size, x)
#define FIX_MAP_AUTO_REHASH_LOADFACTOR 0.9f

bool jak_hashtable_create(jak_hashtable *map, jak_error *err, size_t key_size, size_t value_size,
                      size_t capacity)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(key_size)
        JAK_ERROR_IF_NULL(value_size)

        int err_code = JAK_ERR_INITFAILED;

        map->size = 0;

        JAK_SUCCESS_OR_JUMP(jak_vector_create(&map->key_data, NULL, key_size, capacity), error_handling);
        JAK_SUCCESS_OR_JUMP(jak_vector_create(&map->value_data, NULL, value_size, capacity), cleanup_key_data_and_error);
        JAK_SUCCESS_OR_JUMP(jak_vector_create(&map->table, NULL, sizeof(jak_hashtable_bucket), capacity),
                            cleanup_value_key_data_and_error);
        JAK_SUCCESS_OR_JUMP(jak_vector_enlarge_size_to_capacity(&map->table), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_vector_zero_memory(&map->table), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_spinlock_init(&map->lock), cleanup_key_value_table_and_error);
        JAK_SUCCESS_OR_JUMP(jak_error_init(&map->err), cleanup_key_value_table_and_error);

        return true;

        cleanup_key_value_table_and_error:
        if (!jak_vector_drop(&map->table)) {
                err_code = JAK_ERR_DROPFAILED;
        }
        cleanup_value_key_data_and_error:
        if (!jak_vector_drop(&map->value_data)) {
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

bool jak_hashtable_drop(jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(map)

        bool status = true;

        status &= jak_vector_drop(&map->table);
        status &= jak_vector_drop(&map->value_data);
        status &= jak_vector_drop(&map->key_data);

        if (!status) {
                JAK_ERROR(&map->err, JAK_ERR_DROPFAILED);
        }

        return status;
}

jak_hashtable *jak_hashtable_cpy(jak_hashtable *src)
{
        if (src) {
                jak_hashtable *cpy = JAK_MALLOC(sizeof(jak_hashtable));

                jak_hashtable_lock(src);

                jak_hashtable_create(cpy,
                                 &src->err,
                                 src->key_data.elem_size,
                                 src->value_data.elem_size,
                                 src->table.cap_elems);

                JAK_ASSERT(src->key_data.cap_elems == src->value_data.cap_elems
                           && src->value_data.cap_elems == src->table.cap_elems);
                JAK_ASSERT((src->key_data.num_elems == src->value_data.num_elems)
                           && src->value_data.num_elems <= src->table.num_elems);

                jak_vector_cpy_to(&cpy->key_data, &src->key_data);
                jak_vector_cpy_to(&cpy->value_data, &src->value_data);
                jak_vector_cpy_to(&cpy->table, &src->table);
                cpy->size = src->size;
                jak_error_cpy(&cpy->err, &src->err);

                JAK_ASSERT(cpy->key_data.cap_elems == src->value_data.cap_elems
                           && src->value_data.cap_elems == cpy->table.cap_elems);
                JAK_ASSERT((cpy->key_data.num_elems == src->value_data.num_elems)
                           && src->value_data.num_elems <= cpy->table.num_elems);

                jak_hashtable_unlock(src);
                return cpy;
        } else {
                JAK_ERROR(&src->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_hashtable_clear(jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ASSERT(map->key_data.cap_elems == map->value_data.cap_elems
                   && map->value_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT((map->key_data.num_elems == map->value_data.num_elems)
                   && map->value_data.num_elems <= map->table.num_elems);

        jak_hashtable_lock(map);

        bool status = jak_vector_clear(&map->key_data) && jak_vector_clear(&map->value_data) && jak_vector_zero_memory(&map->table);

        map->size = 0;

        JAK_ASSERT(map->key_data.cap_elems == map->value_data.cap_elems
                   && map->value_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT((map->key_data.num_elems == map->value_data.num_elems)
                   && map->value_data.num_elems <= map->table.num_elems);

        if (!status) {
                JAK_ERROR(&map->err, JAK_ERR_OPPFAILED);
        }

        jak_hashtable_unlock(map);

        return status;
}

bool jak_hashtable_avg_displace(float *displace, const jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(displace);
        JAK_ERROR_IF_NULL(map);

        size_t sum_dis = 0;
        for (size_t i = 0; i < map->table.num_elems; i++) {
                jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, i, jak_hashtable_bucket);
                sum_dis += abs(bucket->displacement);
        }
        *displace = (sum_dis / (float) map->table.num_elems);

        return true;
}

bool jak_hashtable_lock(jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(map)
        //jak_spinlock_acquire(&map->lock);
        return true;
}

bool jak_hashtable_unlock(jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(map)
        //jak_spinlock_release(&map->lock);
        return true;
}

static inline const void *_jak_hash_table_get_bucket_key(const jak_hashtable_bucket *bucket, const jak_hashtable *map)
{
        return map->key_data.base + bucket->data_idx * map->key_data.elem_size;
}

static inline const void *get_bucket_value(const jak_hashtable_bucket *bucket, const jak_hashtable *map)
{
        return map->value_data.base + bucket->data_idx * map->value_data.elem_size;
}

static void _jak_hash_table_insert(jak_hashtable_bucket *bucket, jak_hashtable *map, const void *key, const void *value,
                   jak_i32 displacement)
{
        JAK_ASSERT(map->key_data.num_elems == map->value_data.num_elems);
        jak_u64 idx = map->key_data.num_elems;
        void *key_datum = JAK_VECTOR_NEW_AND_GET(&map->key_data, void *);
        void *value_datum = JAK_VECTOR_NEW_AND_GET(&map->value_data, void *);
        memcpy(key_datum, key, map->key_data.elem_size);
        memcpy(value_datum, value, map->value_data.elem_size);
        bucket->data_idx = idx;
        bucket->in_use_flag = true;
        bucket->displacement = displacement;
        map->size++;
}

static inline uint_fast32_t _jak_hash_table_insert_or_update(jak_hashtable *map, const jak_u32 *bucket_idxs, const void *keys,
                                             const void *values, uint_fast32_t num_pairs)
{
        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                const void *value = values + i * map->key_data.elem_size;
                jak_u32 intended_bucket_idx = bucket_idxs[i];

                jak_u32 bucket_idx = intended_bucket_idx;

                jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, bucket_idx, jak_hashtable_bucket);
                if (bucket->in_use_flag && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) != 0) {
                        bool fitting_bucket_found = false;
                        jak_u32 displace_idx;
                        for (displace_idx = bucket_idx + 1; displace_idx < map->table.num_elems; displace_idx++) {
                                jak_hashtable_bucket
                                        *bucket = JAK_VECTOR_GET(&map->table, displace_idx, jak_hashtable_bucket);
                                fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag
                                                                                &&
                                                                                memcmp(_jak_hash_table_get_bucket_key(bucket, map), key,
                                                                                       map->key_data.elem_size) == 0);
                                if (fitting_bucket_found) {
                                        break;
                                } else {
                                        jak_i32 displacement = displace_idx - bucket_idx;
                                        const void *swap_key = _jak_hash_table_get_bucket_key(bucket, map);
                                        const void *swap_value = get_bucket_value(bucket, map);

                                        if (bucket->displacement < displacement) {
                                                _jak_hash_table_insert(bucket, map, key, value, displacement);
                                                _jak_hash_table_insert_or_update(map, &displace_idx, swap_key, swap_value, 1);
                                                goto next_round;
                                        }
                                }
                        }
                        if (!fitting_bucket_found) {
                                for (displace_idx = 0; displace_idx < bucket_idx - 1; displace_idx++) {
                                        const jak_hashtable_bucket
                                                *bucket = JAK_VECTOR_GET(&map->table, displace_idx, jak_hashtable_bucket);
                                        fitting_bucket_found = !bucket->in_use_flag || (bucket->in_use_flag
                                                                                        && memcmp(_jak_hash_table_get_bucket_key(bucket,
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
                        bucket = JAK_VECTOR_GET(&map->table, bucket_idx, jak_hashtable_bucket);
                }

                bool is_update =
                        bucket->in_use_flag && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                if (is_update) {
                        void *bucket_value = (void *) get_bucket_value(bucket, map);
                        memcpy(bucket_value, value, map->value_data.elem_size);
                } else {
                        jak_i32 displacement = intended_bucket_idx - bucket_idx;
                        _jak_hash_table_insert(bucket, map, key, value, displacement);
                }

                next_round:
                if (map->size >= FIX_MAP_AUTO_REHASH_LOADFACTOR * map->table.cap_elems) {
                        return i + 1; /* tell the caller that pair i was inserted, but it successors not */
                }

        }
        return 0;
}

bool jak_hashtable_insert_or_update(jak_hashtable *map, const void *keys, const void *values,
                                uint_fast32_t num_pairs)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(keys)
        JAK_ERROR_IF_NULL(values)

        JAK_ASSERT(map->key_data.cap_elems == map->value_data.cap_elems
                   && map->value_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT((map->key_data.num_elems == map->value_data.num_elems)
                   && map->value_data.num_elems <= map->table.num_elems);

        jak_hashtable_lock(map);

        jak_u32 *bucket_idxs = JAK_MALLOC(num_pairs * sizeof(jak_u32));
        if (!bucket_idxs) {
                JAK_ERROR(&map->err, JAK_ERR_MALLOCERR);
                return false;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                hash32_t hash = _JAK_HASH_TABLE_HASHCODE_OF(map->key_data.elem_size, key);
                bucket_idxs[i] = hash % map->table.num_elems;
        }

        uint_fast32_t cont_idx = 0;
        do {
                cont_idx = _jak_hash_table_insert_or_update(map,
                                            bucket_idxs + cont_idx,
                                            keys + cont_idx * map->key_data.elem_size,
                                            values + cont_idx * map->value_data.elem_size,
                                            num_pairs - cont_idx);
                if (cont_idx != 0) {
                        /* rehashing is required, and [status, num_pairs) are left to be inserted */
                        if (!jak_hashtable_rehash(map)) {
                                jak_hashtable_unlock(map);
                                return false;
                        }
                }
        } while (cont_idx != 0);

        free(bucket_idxs);
        jak_hashtable_unlock(map);

        return true;
}

typedef struct jak_hashtable_header {
        char marker;
        jak_offset_t key_data_off;
        jak_offset_t value_data_off;
        jak_offset_t table_off;
        jak_u32 size;
} jak_hashtable_header;

bool jak_hashtable_serialize(FILE *file, jak_hashtable *table)
{
        jak_offset_t header_pos = ftell(file);
        fseek(file, sizeof(jak_hashtable_header), SEEK_CUR);

        jak_offset_t key_data_off = ftell(file);
        if (!jak_vector_serialize(file, &table->key_data)) {
                goto error_handling;
        }

        jak_offset_t value_data_off = ftell(file);
        if (!jak_vector_serialize(file, &table->value_data)) {
                goto error_handling;
        }

        jak_offset_t table_off = ftell(file);
        if (!jak_vector_serialize(file, &table->table)) {
                goto error_handling;
        }

        jak_offset_t end = ftell(file);

        fseek(file, header_pos, SEEK_SET);
        jak_hashtable_header header = {.marker = JAK_MARKER_SYMBOL_HASHTABLE_HEADER, .size = table
                ->size, .key_data_off = key_data_off, .value_data_off = value_data_off, .table_off = table_off};
        int nwrite = fwrite(&header, sizeof(jak_hashtable_header), 1, file);
        JAK_ERROR_IF(nwrite != 1, &table->err, JAK_ERR_FWRITE_FAILED);
        fseek(file, end, SEEK_SET);
        return true;

        error_handling:
        fseek(file, header_pos, SEEK_SET);
        return false;
}

bool jak_hashtable_deserialize(jak_hashtable *table, jak_error *err, FILE *file)
{
        JAK_ERROR_IF_NULL(table)
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(file)

        int err_code = JAK_ERR_NOERR;

        jak_hashtable_header header;
        jak_offset_t start = ftell(file);
        int nread = fread(&header, sizeof(jak_hashtable_header), 1, file);
        if (nread != 1) {
                err_code = JAK_ERR_FREAD_FAILED;
                goto error_handling;
        }
        if (header.marker != JAK_MARKER_SYMBOL_HASHTABLE_HEADER) {
                err_code = JAK_ERR_CORRUPTED;
                goto error_handling;
        }

        fseek(file, header.key_data_off, SEEK_SET);
        if (!jak_vector_deserialize(&table->key_data, err, file)) {
                err_code = err->code;
                goto error_handling;
        }

        fseek(file, header.value_data_off, SEEK_SET);
        if (!jak_vector_deserialize(&table->value_data, err, file)) {
                err_code = err->code;
                goto error_handling;
        }

        fseek(file, header.table_off, SEEK_SET);
        if (!jak_vector_deserialize(&table->table, err, file)) {
                err_code = err->code;
                goto error_handling;
        }

        jak_spinlock_init(&table->lock);
        jak_error_init(&table->err);
        return true;

        error_handling:
        fseek(file, start, SEEK_SET);
        JAK_ERROR(err, err_code);
        return false;
}

bool jak_hashtable_remove_if_contained(jak_hashtable *map, const void *keys, size_t num_pairs)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(keys)

        jak_hashtable_lock(map);

        jak_u32 *bucket_idxs = JAK_MALLOC(num_pairs * sizeof(jak_u32));
        if (!bucket_idxs) {
                JAK_ERROR(&map->err, JAK_ERR_MALLOCERR);
                jak_hashtable_unlock(map);
                return false;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                bucket_idxs[i] = _JAK_HASH_TABLE_HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
        }

        for (uint_fast32_t i = 0; i < num_pairs; i++) {
                const void *key = keys + i * map->key_data.elem_size;
                jak_u32 bucket_idx = bucket_idxs[i];
                jak_u32 actual_idx = bucket_idx;
                bool bucket_found = false;

                for (jak_u32 k = bucket_idx; !bucket_found && k < map->table.num_elems; k++) {
                        const jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashtable_bucket);
                        bucket_found = bucket->in_use_flag
                                       && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                        actual_idx = k;
                }
                for (jak_u32 k = 0; !bucket_found && k < bucket_idx; k++) {
                        const jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashtable_bucket);
                        bucket_found = bucket->in_use_flag
                                       && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                        actual_idx = k;
                }

                if (bucket_found) {
                        jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, actual_idx, jak_hashtable_bucket);
                        bucket->in_use_flag = false;
                        bucket->data_idx = 0;
                        bucket->num_probs = 0;
                }
        }

        free(bucket_idxs);

        jak_hashtable_unlock(map);

        return true;
}

const void *jak_hashtable_get_value(jak_hashtable *map, const void *key)
{
        JAK_ERROR_IF_NULL(map)
        JAK_ERROR_IF_NULL(key)

        const void *result = NULL;

        jak_hashtable_lock(map);

        jak_u32 bucket_idx = _JAK_HASH_TABLE_HASHCODE_OF(map->key_data.elem_size, key) % map->table.num_elems;
        jak_u32 actual_idx = bucket_idx;
        bool bucket_found = false;

        for (jak_u32 k = bucket_idx; !bucket_found && k < map->table.num_elems; k++) {
                const jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashtable_bucket);
                bucket_found =
                        bucket->in_use_flag && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                actual_idx = k;
        }
        for (jak_u32 k = 0; !bucket_found && k < bucket_idx; k++) {
                const jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, k, jak_hashtable_bucket);
                bucket_found =
                        bucket->in_use_flag && memcmp(_jak_hash_table_get_bucket_key(bucket, map), key, map->key_data.elem_size) == 0;
                actual_idx = k;
        }

        if (bucket_found) {
                jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&map->table, actual_idx, jak_hashtable_bucket);
                result = get_bucket_value(bucket, map);
        }

        jak_hashtable_unlock(map);

        return result;
}

bool jak_hashtable_get_load_factor(float *factor, jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(factor)
        JAK_ERROR_IF_NULL(map)

        jak_hashtable_lock(map);

        *factor = map->size / (float) map->table.num_elems;

        jak_hashtable_unlock(map);

        return true;
}

bool jak_hashtable_rehash(jak_hashtable *map)
{
        JAK_ERROR_IF_NULL(map)

        jak_hashtable_lock(map);

        jak_hashtable *cpy = jak_hashtable_cpy(map);
        jak_hashtable_clear(map);

        size_t new_cap = (cpy->key_data.cap_elems + 1) * 1.7f;

        jak_vector_grow_to(&map->key_data, new_cap);
        jak_vector_grow_to(&map->value_data, new_cap);
        jak_vector_grow_to(&map->table, new_cap);
        jak_vector_enlarge_size_to_capacity(&map->table);
        jak_vector_zero_memory(&map->table);

        JAK_ASSERT(map->key_data.cap_elems == map->value_data.cap_elems
                   && map->value_data.cap_elems == map->table.cap_elems);
        JAK_ASSERT((map->key_data.num_elems == map->value_data.num_elems)
                   && map->value_data.num_elems <= map->table.num_elems);

        for (size_t i = 0; i < cpy->table.num_elems; i++) {
                jak_hashtable_bucket *bucket = JAK_VECTOR_GET(&cpy->table, i, jak_hashtable_bucket);
                if (bucket->in_use_flag) {
                        const void *old_key = _jak_hash_table_get_bucket_key(bucket, cpy);
                        const void *old_value = get_bucket_value(bucket, cpy);
                        if (!jak_hashtable_insert_or_update(map, old_key, old_value, 1)) {
                                JAK_ERROR(&map->err, JAK_ERR_REHASH_NOROLLBACK)
                                jak_hashtable_unlock(map);
                                return false;
                        }
                }
        }

        jak_hashtable_unlock(map);
        return true;
}