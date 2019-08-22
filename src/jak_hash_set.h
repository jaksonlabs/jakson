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

typedef struct jak_hashset_bucket {
        bool in_use_flag;  /* flag indicating if bucket is in use */
        jak_i32 displacement; /* difference between intended position during insert, and actual position in table */
        jak_u64 key_idx;      /* position of key element in owning jak_hashset structure */
} jak_hashset_bucket;

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
typedef struct jak_hashset {
        struct jak_vector key_data;
        struct jak_vector ofType(jak_hashset_bucket) table;
        jak_spinlock lock;
        jak_u32 size;
        jak_error err;
} jak_hashset;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_hashset, jak_hashset, set);

bool jak_hashset_create(jak_hashset *map, jak_error *err, size_t key_size, size_t capacity);
jak_hashset *jak_hashset_cpy(jak_hashset *src);
bool jak_hashset_drop(jak_hashset *map);

struct jak_vector *jak_hashset_keys(jak_hashset *map);
bool jak_hashset_clear(jak_hashset *map);
bool jak_hashset_avg_displace(float *displace, const jak_hashset *map);
bool jak_hashset_lock(jak_hashset *map);
bool jak_hashset_unlock(jak_hashset *map);
bool jak_hashset_insert_or_update(jak_hashset *map, const void *keys, uint_fast32_t num_pairs);
bool jak_hashset_remove_if_contained(jak_hashset *map, const void *keys, size_t num_pairs);
bool jak_hashset_contains_key(jak_hashset *map, const void *key);
bool jak_hashset_get_load_factor(float *factor, jak_hashset *map);
bool jak_hashset_rehash(jak_hashset *map);

JAK_END_DECL

#endif
