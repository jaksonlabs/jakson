/**
 * Copyright 2018 Marcus Pinnecke
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

// ---------------------------------------------------------------------------------------------------------------------
//
//  S U M M A R Y
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * SUMMARY
 *
 * A specialized hash table that uses strings as keys, and 64bit values. The specialization is to avoid some
 * indirection cost to compare keys, e.g., by calling a function pointer to compare two objects like clib suggest it,
 * and to ensure that the value is embedded in continuous memory rather than a pointer to another distant memory block.
 *
 * Internally, the carbon_parallel_map_exec is organized by paritions where each partition is assigned exclusively to
 * one thread. Each such parition contains of as a vector of length 'num_buckets' which contains of elements of a bucket
 * type. A bucket type is an fixed-size array of entries, each containing a key and a value. In case of no collisions,
 * this array contains of exactly one element. In case of collisions, colliding keys are stored in this array of entries
 * in the same bucket. To speedup lookups, this entry vector may be additionally sorted and a (specialized) binary
 * search is invoked to find the bucket entry associated to a particular key (if any). Other lookup strategies includes
 * both single and multi-threaded forward scans. Which strategy to use in which case is decided by the
 * carbon_parallel_map_exec itself, however. To satisfy user-specific memory limitations, some per-bucket elements may
 * be swapped out to a didicated swap space.
 *
 * The underlying hashing function is the Jenkins hash function.
 */

#ifndef CARBON_STRHASH_H
#define CARBON_STRHASH_H

#include <assert.h>

#include "shared/common.h"
#include "core/alloc/alloc.h"
#include "std/vec.h"

#include "hash/hash.h"
#include "shared/types.h"

CARBON_BEGIN_DECL

/**
 * Enables or disabled packing of entries inside a bucket. By default, packing is disabled.
 * To turn on packing, set 'CARBON_CONFIG_PACK_BUCKETS' symbol
 */
#ifdef CARBON_CONFIG_PACK_BUCKETS
#define
#define CARBON_BUCKET_PACKING __attribute__((__packed__))
#else
#define CARBON_BUCKET_PACKING
#endif

/**
 * Number of elements stored in the per-bucket cache (Tier 2, see below)
 */
#ifndef CARBON_CONFIG_BUCKET_CACHE_SIZE
#define CARBON_CONFIG_BUCKET_CACHE_SIZE  16
#endif

/**
 * Maximum number of elements stored per bucket (Tier 3, see below)
 */
#ifndef CARBON_CONFIG_BUCKET_CAPACITY
#define CARBON_CONFIG_BUCKET_CAPACITY  1024
#endif

typedef struct carbon_strhash carbon_strhash_t;

typedef enum
{
    CARBON_STRHASH_INMEMORY
} carbon_strhash_tag_e;

typedef struct carbon_string_hash_counters
{
    size_t num_bucket_search_miss;
    size_t num_bucket_search_hit;
    size_t num_bucket_cache_search_miss;
    size_t num_bucket_cache_search_hit;
} carbon_string_hash_counters_t;

typedef struct carbon_strhash
{
    /**
     * Implementation-specific values
     */
    void *extra;

    /**
     * Implementation tag
     */
    carbon_strhash_tag_e tag;

    /**
     * Statistics to lookup misses and hits
     *
     * <b>Note</b>: Implementation must maintain counters by itself
     */
    carbon_string_hash_counters_t counters;

    /**
    *  Memory allocator that is used to get memory for user data
    */
    struct allocator allocator;

    /**
     *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
     */
    int (*drop)(carbon_strhash_t *self);

    /**
     * Put <code>num_pair</code> objects into this carbon_parallel_map_exec maybe updating old objects with the same key.
     */
    int (*put_bulk_safe)(carbon_strhash_t *self, char *const *keys, const carbon_string_id_t *values, size_t npairs);

    /**
     * Put <code>num_pair</code> objects into this carbon_parallel_map_exec maybe without checking for updates.
     */
    int (*put_bulk_fast)(carbon_strhash_t *self, char *const *keys, const carbon_string_id_t *values, size_t npairs);

    /**
     * Same as 'put_safe_bulk' but specialized for a single element
     */
    int (*put_exact_safe)(carbon_strhash_t *self, const char *key, carbon_string_id_t value);

    /**
     * Same as 'put_fast_bulk' but specialized for a single element
     */
    int (*put_exact_fast)(carbon_strhash_t *self, const char *key, carbon_string_id_t value);

    /**
     * Get the values associated with <code>keys</code> in this carbon_parallel_map_exec (if any).
     */
    int (*get_bulk_safe)(carbon_strhash_t *self, carbon_string_id_t **out, bool **found_mask, size_t *nnot_found,
                         char *const *keys, size_t nkeys);

    /**
     * The same as 'get_safe_bulk' but optimized for a single element
     */
    int (*get_exact_safe)(carbon_strhash_t *self, carbon_string_id_t *out, bool *found_mask, const char *key);

    /**
     * Get the values associated with <code>keys</code> in this carbon_parallel_map_exec. All keys <u>must</u> exist.
     */
    int (*get_fast)(carbon_strhash_t *self, carbon_string_id_t **out, char *const *keys, size_t nkeys);

    /**
     * Updates keys associated with <code>values</code> in this carbon_parallel_map_exec. All values <u>must</u> exist, and the
     * mapping between keys and values must be bidirectional.
     */
    int (*update_key_fast)(carbon_strhash_t *self, const carbon_string_id_t *values, char *const *keys, size_t nkeys);

    /**
     * Removes the objects with the gives keys from this carbon_parallel_map_exec
     */
    int (*remove)(carbon_strhash_t *self, char *const *keys, size_t nkeys);

    /**
     * Frees up allocated memory for <code>ptr</code> via the allocator in <code>carbon_parallel_map_exec</code> that was specified
     * by the call to <code>string_id_map_create</code>
     */
    int (*free)(carbon_strhash_t *self, void *ptr);

    /**
     *  Error information
     */
    struct err err;

} carbon_strhash_t;

CARBON_DEFINE_GET_ERROR_FUNCTION(carbon_strhash_t, carbon_strhash_t, table);

/**
 * Frees resources bound to <code>carbon_parallel_map_exec</code> via the allocator specified by the call to <code>string_id_map_create</code>.
 *
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int
carbon_strhash_drop(carbon_strhash_t *carbon_parallel_map_exec)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    assert(carbon_parallel_map_exec->drop);

    return carbon_parallel_map_exec->drop(carbon_parallel_map_exec);
}

/**
 * Resets statistics counters
 *
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static bool
carbon_strhash_reset_counters(carbon_strhash_t *carbon_parallel_map_exec)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    memset(&carbon_parallel_map_exec->counters, 0, sizeof(carbon_string_hash_counters_t));
    return true;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param carbon_parallel_map_exec non-null pointer to the carbon_parallel_map_exec
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
carbon_strhash_get_counters(carbon_string_hash_counters_t *out, const carbon_strhash_t *carbon_parallel_map_exec)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(out);
    *out = carbon_parallel_map_exec->counters;
    return true;
}

/**
 * Put <code>num_pair</code> objects into this carbon_parallel_map_exec maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>string_hashtable_put_blind</code>
 * instead.
 *
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
carbon_strhash_put_safe(carbon_strhash_t *carbon_parallel_map_exec, char *const *keys, const carbon_string_id_t *values,
                        size_t npairs)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    CARBON_NON_NULL_OR_ERROR(values);
    assert(carbon_parallel_map_exec->put_bulk_safe);

    return carbon_parallel_map_exec->put_bulk_safe(carbon_parallel_map_exec, keys, values, npairs);
}

/**
 * Put <code>num_pair</code> objects into this carbon_parallel_map_exec, ingoring whether the key exists or not. This function is
 * useful for insert operations of pairs where it is guaranteed that the keys are not yet inserted into this hashtable.
 * In case this guarantee is broken, the behavior is undefined. Depending on the implementation, this specialized
 * <code>put</code> function may have a better performance.
 *
 * If a check for existence is required, use <code>string_hashtable_put_test</code>
 * instead.
 *
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int
carbon_strhash_put_bulk_fast(carbon_strhash_t *carbon_parallel_map_exec, char *const *keys, const carbon_string_id_t *values,
                             size_t npairs)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    CARBON_NON_NULL_OR_ERROR(values);
    assert(carbon_parallel_map_exec->put_bulk_fast);

    return carbon_parallel_map_exec->put_bulk_fast(carbon_parallel_map_exec, keys, values, npairs);
}

/**
 * Same as 'string_lookup_put_bulk' but specialized for a single pair
 */
inline static int
carbon_strhash_put_exact(carbon_strhash_t *carbon_parallel_map_exec, const char *key, carbon_string_id_t value)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(key);
    assert(carbon_parallel_map_exec->put_exact_safe);

    return carbon_parallel_map_exec->put_exact_safe(carbon_parallel_map_exec, key, value);
}

/**
 * Same as 'string_lookup_put_fast_bulk' but specialized for a single pair
 */
inline static int
carbon_strhash_put_exact_fast(carbon_strhash_t *carbon_parallel_map_exec, const char *key, carbon_string_id_t value)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(key);

    assert(carbon_parallel_map_exec->put_exact_fast);

    return carbon_parallel_map_exec->put_exact_fast(carbon_parallel_map_exec, key, value);
}

/**
 * Get the values associated with <code>keys</code> in this carbon_parallel_map_exec (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>found_mask</code> and
 * <code>num_not_found</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The carbon_parallel_map_exec will allocate enough memory to store the
 *            result. There are <code>num_keys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this carbon_parallel_map_exec. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>found_mask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>string_id_map_free</code>.
 * @param found_mask A non-null pointer to an unallocated memory address. The carbon_parallel_map_exec will allocate enough memory to store
 *            the result. There are <code>num_keys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this carbon_parallel_map_exec. If this is the case, the i-th entry in <code>found_mask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>found_mask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param num_not_found A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this carbon_parallel_map_exec.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
carbon_strhash_get_bulk_safe(carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                             carbon_strhash_t *carbon_parallel_map_exec,
                             char *const *keys, size_t nkeys)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(found_mask);
    CARBON_NON_NULL_OR_ERROR(num_not_found);
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(carbon_parallel_map_exec->get_bulk_safe);

    int result = carbon_parallel_map_exec->get_bulk_safe(carbon_parallel_map_exec, out, found_mask, num_not_found, keys, nkeys);

    assert (out != NULL);
    assert (found_mask != NULL);

    return result;
}

inline static int
carbon_strhash_get_bulk_safe_exact(carbon_string_id_t *out, bool *found, carbon_strhash_t *carbon_parallel_map_exec, const char *key)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(found);
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(key);
    assert(carbon_parallel_map_exec->get_exact_safe);

    int result = carbon_parallel_map_exec->get_exact_safe(carbon_parallel_map_exec, out, found, key);

    assert (out != NULL);
    assert (found != NULL);

    return result;
}

/**
 * Get the values associated with <code>keys</code> in this carbon_parallel_map_exec. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The carbon_parallel_map_exec will allocate <code>num_keys</code>
 *            times <code>sizeof(carbon_string_id_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
carbon_strhash_get_bulk_fast(carbon_string_id_t **out, carbon_strhash_t *carbon_parallel_map_exec,
                             char *const *keys, size_t nkeys)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(carbon_parallel_map_exec->get_fast);

    return carbon_parallel_map_exec->get_fast(carbon_parallel_map_exec, out, keys, nkeys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>string_hashtable_put_test</code> or
 * <code>string_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The carbon_parallel_map_exec will allocate <code>num_keys</code>
 *            times <code>sizeof(carbon_string_id_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
carbon_strhash_update_fast(carbon_strhash_t *carbon_parallel_map_exec, const carbon_string_id_t *values,
                           char *const *keys, size_t nkeys)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(carbon_parallel_map_exec->update_key_fast);

    return carbon_parallel_map_exec->update_key_fast(carbon_parallel_map_exec, values, keys, nkeys);
}

/**
 * Removes the objects with the gives keys from this carbon_parallel_map_exec
 *
 * @param carbon_parallel_map_exec a non-null pointer to the carbon_parallel_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int
carbon_strhash_remove(carbon_strhash_t *carbon_parallel_map_exec, char *const *keys, size_t nkeys)
{
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(carbon_parallel_map_exec->remove);

    return carbon_parallel_map_exec->remove(carbon_parallel_map_exec, keys, nkeys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>carbon_parallel_map_exec</code> that was specified
 * by the call to <code>string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>string_id_map_get</code>)
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int
carbon_strhash_free(void *ptr, carbon_strhash_t *carbon_parallel_map_exec)
{
    CARBON_NON_NULL_OR_ERROR(ptr);
    CARBON_NON_NULL_OR_ERROR(carbon_parallel_map_exec);
    assert(carbon_parallel_map_exec->free);

    return carbon_parallel_map_exec->free(carbon_parallel_map_exec, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return true if everything went normal, otherwise an value indicating the error
 */
inline static int
carbon_strhash_counters_init(carbon_string_hash_counters_t *counters)
{
    CARBON_NON_NULL_OR_ERROR(counters);
    memset(counters, 0, sizeof(carbon_string_hash_counters_t));
    return true;
}

/**
 * Adds members of both input parameters and stores the result in <code>dstLhs</code>.
 *
 * @param dstLhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return true if everything went normal, otherwise an value indicating the error
 */
inline static int
carbon_strhash_counters_add(carbon_string_hash_counters_t *dst_lhs, const carbon_string_hash_counters_t *rhs)
{
    CARBON_NON_NULL_OR_ERROR(dst_lhs);
    CARBON_NON_NULL_OR_ERROR(rhs);
    dst_lhs->num_bucket_search_miss += rhs->num_bucket_search_miss;
    dst_lhs->num_bucket_search_hit += rhs->num_bucket_search_hit;
    dst_lhs->num_bucket_cache_search_hit += rhs->num_bucket_cache_search_hit;
    dst_lhs->num_bucket_cache_search_miss += rhs->num_bucket_cache_search_miss;
    return true;
}

CARBON_END_DECL

#endif
