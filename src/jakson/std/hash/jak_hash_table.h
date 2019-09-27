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

#ifndef JAK_HAHSTABLE_H
#define JAK_HAHSTABLE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/jak_vector.h>
#include <jakson/std/jak_spinlock.h>

JAK_BEGIN_DECL

typedef struct jak_hashtable_bucket {
        bool in_use_flag;  /* flag indicating if bucket is in use */
        jak_i32 displacement; /* difference between intended position during insert, and actual position in table */
        jak_u32 num_probs;    /* number of probe calls to this bucket */
        jak_u64 data_idx;      /* position of key element in owning jak_hashtable structure */
} jak_hashtable_bucket;

/**
 * Hash table implementation specialized for key and value types of fixed-length size, and where comparision
 * for equals is byte-compare. With this, calling a (type-dependent) compare function becomes obsolete.
 *
 * Example: mapping of jak_u64 to jak_u32.
 *
 * This hash table is optimized to reduce access time to elements. Internally, a robin-hood hashing technique is used.
 *
 * Note: this implementation does not support string or pointer types. The structure is thread-safe by a spinlock
 * lock implementation.
 */
typedef struct jak_hashtable {
        jak_vector key_data;
        jak_vector value_data;
        jak_vector ofType(jak_hashtable_bucket) table;
        jak_spinlock lock;
        jak_u32 size;
        jak_error err;
} jak_hashtable;

JAK_DEFINE_GET_ERROR_FUNCTION(jak_hashtable, jak_hashtable, table);

bool jak_hashtable_create(jak_hashtable *map, jak_error *err, size_t key_size, size_t value_size, size_t capacity);
jak_hashtable *jak_hashtable_cpy(jak_hashtable *src);
bool jak_hashtable_drop(jak_hashtable *map);

bool jak_hashtable_clear(jak_hashtable *map);
bool jak_hashtable_avg_displace(float *displace, const jak_hashtable *map);
bool jak_hashtable_lock(jak_hashtable *map);
bool jak_hashtable_unlock(jak_hashtable *map);
bool jak_hashtable_insert_or_update(jak_hashtable *map, const void *keys, const void *values, uint_fast32_t num_pairs);
bool jak_hashtable_serialize(FILE *file, jak_hashtable *table);
bool jak_hashtable_deserialize(jak_hashtable *table, jak_error *err, FILE *file);
bool jak_hashtable_remove_if_contained(jak_hashtable *map, const void *keys, size_t num_pairs);
const void *jak_hashtable_get_value(jak_hashtable *map, const void *key);
bool jak_hashtable_get_load_factor(float *factor, jak_hashtable *map);
bool jak_hashtable_rehash(jak_hashtable *map);

JAK_END_DECL

#endif
