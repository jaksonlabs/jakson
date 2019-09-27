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

#ifndef HAHSTABLE_H
#define HAHSTABLE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>
#include <jakson/std/spinlock.h>

BEGIN_DECL

typedef struct hashtable_bucket {
        bool in_use_flag;  /** flag indicating if bucket is in use */
        i32 displacement; /** difference between intended position during insert, and actual position in table */
        u32 num_probs;    /** number of probe calls to this bucket */
        u64 data_idx;      /** position of key element in owning hashtable structure */
} hashtable_bucket;

/**
 * Hash table implementation specialized for key and value types of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: mapping of u64 to u32.
 *
 * This hash table is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct hashtable {
        vector key_data;
        vector value_data;
        vector ofType(hashtable_bucket) table;
        spinlock lock;
        u32 size;
        err err;
} hashtable;

bool hashtable_create(hashtable *map, err *err, size_t key_size, size_t value_size, size_t capacity);
hashtable *hashtable_cpy(hashtable *src);
bool hashtable_drop(hashtable *map);

bool hashtable_clear(hashtable *map);
bool hashtable_avg_displace(float *displace, const hashtable *map);
bool hashtable_lock(hashtable *map);
bool hashtable_unlock(hashtable *map);
bool hashtable_insert_or_update(hashtable *map, const void *keys, const void *values, uint_fast32_t num_pairs);
bool hashtable_serialize(FILE *file, hashtable *table);
bool hashtable_deserialize(hashtable *table, err *err, FILE *file);
bool hashtable_remove_if_contained(hashtable *map, const void *keys, size_t num_pairs);
const void *hashtable_get_value(hashtable *map, const void *key);
bool hashtable_get_load_factor(float *factor, hashtable *map);
bool hashtable_rehash(hashtable *map);

END_DECL

#endif
