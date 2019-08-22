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

#ifndef JAK_HASHTEST_H
#define JAK_HASHTEST_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_spinlock.h>

JAK_BEGIN_DECL

struct hashset_bucket {
    bool in_use_flag;  /* flag indicating if bucket is in use */
    jak_i32 displacement; /* difference between intended position during insert, and actual position in table */
    jak_u64 key_idx;      /* position of key element in owning struct hashset structure */
};

/**
 * Hashset implementation specialized for key of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: jak_u64.
 *
 * This hashset is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
struct hashset {
    struct jak_vector key_data;
    struct jak_vector ofType(struct hashset_bucket) table;
    struct spinlock lock;
    jak_u32 size;
    struct jak_error err;
};

JAK_DEFINE_GET_ERROR_FUNCTION(hashset, struct hashset, set);

bool hashset_create(struct hashset *map, struct jak_error *err, size_t key_size, size_t capacity);

struct hashset *hashset_cpy(struct hashset *src);

bool hashset_drop(struct hashset *map);

struct jak_vector *hashset_keys(struct hashset *map);

bool hashset_clear(struct hashset *map);

bool hashset_avg_displace(float *displace, const struct hashset *map);

bool hashset_lock(struct hashset *map);

bool hashset_unlock(struct hashset *map);

bool hashset_insert_or_update(struct hashset *map, const void *keys, uint_fast32_t num_pairs);

bool hashset_remove_if_contained(struct hashset *map, const void *keys, size_t num_pairs);

bool hashset_contains_key(struct hashset *map, const void *key);

bool hashset_get_fload_factor(float *factor, struct hashset *map);

bool hashset_rehash(struct hashset *map);

JAK_END_DECL

#endif
