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

#ifndef HASHTEST_H
#define HASHTEST_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>
#include <jakson/std/spinlock.h>

BEGIN_DECL

typedef struct hashset_bucket {
        bool in_use_flag;  /** flag indicating if bucket is in use */
        i32 displacement; /** difference between intended position during insert, and actual position in table */
        u64 key_idx;      /** position of key element in owning hashset structure */
} hashset_bucket;

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
typedef struct hashset {
        vector key_data;
        vector ofType(hashset_bucket) table;
        spinlock lock;
        u32 size;
        err err;
} hashset;

bool hashset_create(hashset *map, err *err, size_t key_size, size_t capacity);
hashset *hashset_cpy(hashset *src);
bool hashset_drop(hashset *map);

vector *hashset_keys(hashset *map);
bool hashset_clear(hashset *map);
bool hashset_avg_displace(float *displace, const hashset *map);
bool hashset_lock(hashset *map);
bool hashset_unlock(hashset *map);
bool hashset_insert_or_update(hashset *map, const void *keys, uint_fast32_t num_pairs);
bool hashset_remove_if_contained(hashset *map, const void *keys, size_t num_pairs);
bool hashset_contains_key(hashset *map, const void *key);
bool hashset_get_load_factor(float *factor, hashset *map);
bool hashset_rehash(hashset *map);

END_DECL

#endif
