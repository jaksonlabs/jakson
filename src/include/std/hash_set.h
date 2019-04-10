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

#ifndef NG5_HASHTEST_H
#define NG5_HASHTEST_H

#include "shared/common.h"
#include "std/vec.h"
#include "core/async/spin.h"

NG5_BEGIN_DECL

typedef struct
{
    bool     in_use_flag;  /* flag indicating if bucket is in use */
    i32  displacement; /* difference between intended position during insert, and actual position in table */
    u64 key_idx;      /* position of key element in owning carbon_hashset_t structure */
} carbon_hashset_bucket_t;

/**
 * Hashset implementation specialized for key of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: u64.
 *
 * This hashset is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct
{
    struct vector key_data;
    struct vector ofType(carbon_hashset_bucket_t) table;
    struct spinlock lock;
    u32 size;
    struct err err;
} carbon_hashset_t;

NG5_DEFINE_GET_ERROR_FUNCTION(hashset, carbon_hashset_t, set);

NG5_EXPORT(bool)
carbon_hashset_create(carbon_hashset_t *map, struct err *err, size_t key_size, size_t capacity);

NG5_EXPORT(carbon_hashset_t *)
carbon_hashset_cpy(carbon_hashset_t *src);

NG5_EXPORT(bool)
carbon_hashset_drop(carbon_hashset_t *map);

NG5_EXPORT(struct vector *)
carbon_hashset_keys(carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_clear(carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_avg_displace(float *displace, const carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_lock(carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_unlock(carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_insert_or_update(carbon_hashset_t *map, const void *keys, uint_fast32_t num_pairs);

NG5_EXPORT(bool)
carbon_hashset_remove_if_contained(carbon_hashset_t *map, const void *keys, size_t num_pairs);

NG5_EXPORT(bool)
carbon_hashset_contains_key(carbon_hashset_t *map, const void *key);

NG5_EXPORT(bool)
carbon_hashset_get_fload_factor(float *factor, carbon_hashset_t *map);

NG5_EXPORT(bool)
carbon_hashset_rehash(carbon_hashset_t *map);

NG5_END_DECL

#endif
