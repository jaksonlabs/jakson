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
 * Internally, the str_hash is organized by paritions where each partition is assigned exclusively to
 * one thread. Each such parition contains of as a vector of length 'num_buckets' which contains of elements of a bucket
 * type. A bucket type is an fixed-size array of entries, each containing a key and a value. In case of no collisions,
 * this array contains of exactly one element. In case of collisions, colliding keys are stored in this array of entries
 * in the same bucket. To speedup lookups, this entry vector may be additionally sorted and a (specialized) binary
 * search is invoked to find the bucket entry associated to a particular key (if any). Other lookup strategies includes
 * both single and multi-threaded forward scans. Which strategy to use in which case is decided by the
 * str_hash itself, however. To satisfy user-specific memory limitations, some per-bucket elements may
 * be swapped out to a didicated swap space.
 *
 * The underlying hashing function is the Jenkins hash function.
 */

#ifndef STRHASH_H
#define STRHASH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/stdx/alloc.h>
#include <jakson/std/vector.h>
#include <jakson/std/hash.h>
#include <jakson/types.h>

BEGIN_DECL

/**
 * Enables or disabled packing of entries inside a bucket. By default, packing is disabled.
 * To turn on packing, set 'CONFIG_PACK_BUCKETS' symbol
 */
#ifdef CONFIG_PACK_BUCKETS
#define
#define BUCKET_PACKING __attribute__((__packed__))
#else
#define BUCKET_PACKING
#endif

/**
 * Number of elements stored in the per-bucket cache (Tier 2, see below)
 */
#ifndef CONFIG_BUCKET_CACHE_SIZE
#define CONFIG_BUCKET_CACHE_SIZE  16
#endif

/**
 * Maximum number of elements stored per bucket (Tier 3, see below)
 */
#ifndef CONFIG_BUCKET_CAPACITY
#define CONFIG_BUCKET_CAPACITY  1024
#endif

typedef enum str_hash_tag {
        MEMORY_RESIDENT
} str_hash_tag_e;

typedef struct str_hash_counters {
        size_t num_bucket_search_miss;
        size_t num_bucket_search_hit;
        size_t num_bucket_cache_search_miss;
        size_t num_bucket_cache_search_hit;
} str_hash_counters;

typedef struct str_hash {
        /**
         * Implementation-specific values
         */
        void *extra;

        /**
         * Implementation tag
         */
        str_hash_tag_e tag;

        /**
         * Statistics to lookup misses and hits
         *
         * <b>Note</b>: Implementation must maintain counters by itself
         */
        str_hash_counters counters;

        /**
        *  Memory allocator that is used to get memory for user data
        */
        allocator allocator;

        /**
         *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
         */
        int (*drop)(str_hash *self);

        /**
         * Put <code>num_pair</code> objects into this str_hash maybe updating old objects with the same key.
         */
        int (*put_bulk_safe)(str_hash *self, char *const *keys, const archive_field_sid_t *values, size_t npairs);

        /**
         * Put <code>num_pair</code> objects into this str_hash maybe without checking for updates.
         */
        int (*put_bulk_fast)(str_hash *self, char *const *keys, const archive_field_sid_t *values, size_t npairs);

        /**
         * Same as 'put_safe_bulk' but specialized for a single element
         */
        int (*put_exact_safe)(str_hash *self, const char *key, archive_field_sid_t value);

        /**
         * Same as 'put_fast_bulk' but specialized for a single element
         */
        int (*put_exact_fast)(str_hash *self, const char *key, archive_field_sid_t value);

        /**
         * Get the values associated with <code>keys</code> in this str_hash (if any).
         */
        int (*get_bulk_safe)(str_hash *self, archive_field_sid_t **out, bool **found_mask, size_t *nnot_found, char *const *keys, size_t nkeys);

        /**
         * The same as 'get_safe_bulk' but optimized for a single element
         */
        int (*get_exact_safe)(str_hash *self, archive_field_sid_t *out, bool *found_mask, const char *key);

        /**
         * Get the values associated with <code>keys</code> in this str_hash. All keys <u>must</u> exist.
         */
        int (*get_fast)(str_hash *self, archive_field_sid_t **out, char *const *keys, size_t nkeys);

        /**
         * Updates keys associated with <code>values</code> in this str_hash. All values <u>must</u> exist, and the
         * mapping between keys and values must be bidirectional.
         */
        int (*update_key_fast)(str_hash *self, const archive_field_sid_t *values, char *const *keys, size_t nkeys);

        /**
         * Removes the objects with the gives keys from this str_hash
         */
        int (*remove)(str_hash *self, char *const *keys, size_t nkeys);

        /**
         * Frees up allocated memory for <code>ptr</code> via the allocator in <code>str_hash</code> that was specified
         * by the call to <code>string_id_map_create</code>
         */
        int (*free)(str_hash *self, void *ptr);

        /**
         *  Error information
         */
        err err;
} str_hash;

/**
 * Frees resources bound to <code>str_hash</code> via the allocator specified by the call to <code>string_id_map_create</code>.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indiciating the ERROR.
 */
inline static int str_hash_drop(str_hash *str_hash)
{
        ERROR_IF_NULL(str_hash);
        JAK_ASSERT(str_hash->drop);
        return str_hash->drop(str_hash);
}

/**
 * Resets statistics counters
 *
 * @param str_hash a non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static bool str_hash_reset_counters(str_hash *str_hash)
{
        ERROR_IF_NULL(str_hash);
        memset(&str_hash->counters, 0, sizeof(str_hash_counters));
        return true;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param str_hash non-null pointer to the str_hash
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static int str_hash_get_counters(str_hash_counters *out, const str_hash *str_hash)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(out);
        *out = str_hash->counters;
        return true;
}

/**
 * Put <code>num_pair</code> objects into this str_hash maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>string_hashtable_put_blind</code>
 * instead.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static int str_hash_put_safe(str_hash *str_hash, char *const *keys, const archive_field_sid_t *values, size_t npairs)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        ERROR_IF_NULL(values);
        JAK_ASSERT(str_hash->put_bulk_safe);
        return str_hash->put_bulk_safe(str_hash, keys, values, npairs);
}

/**
 * Put <code>num_pair</code> objects into this str_hash, ingoring whether the key exists or not. This function is
 * useful for insert operations of pairs where it is guaranteed that the keys are not yet inserted into this hashtable.
 * In case this guarantee is broken, the behavior is undefined. Depending on the implementation, this specialized
 * <code>put</code> function may have a better performance.
 *
 * If a check for existence is required, use <code>string_hashtable_put_test</code>
 * instead.
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indiciating the ERROR.
 */
inline static int str_hash_put_bulk_fast(str_hash *str_hash, char *const *keys, const archive_field_sid_t *values, size_t npairs)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        ERROR_IF_NULL(values);
        JAK_ASSERT(str_hash->put_bulk_fast);
        return str_hash->put_bulk_fast(str_hash, keys, values, npairs);
}

/**
 * Same as 'string_lookup_put_bulk' but specialized for a single pair
 */
inline static int str_hash_put_exact(str_hash *str_hash, const char *key, archive_field_sid_t value)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->put_exact_safe);
        return str_hash->put_exact_safe(str_hash, key, value);
}

/**
 * Same as 'string_lookup_put_fast_bulk' but specialized for a single pair
 */
inline static int str_hash_put_exact_fast(str_hash *str_hash, const char *key, archive_field_sid_t value)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->put_exact_fast);
        return str_hash->put_exact_fast(str_hash, key, value);
}

/**
 * Get the values associated with <code>keys</code> in this str_hash (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>found_mask</code> and
 * <code>num_not_found</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate enough memory to store the
 *            result. There are <code>num_keys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this str_hash. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>found_mask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>string_id_map_free</code>.
 * @param found_mask A non-null pointer to an unallocated memory address. The str_hash will allocate enough memory to store
 *            the result. There are <code>num_keys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this str_hash. If this is the case, the i-th entry in <code>found_mask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>found_mask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param num_not_found A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this str_hash.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static int str_hash_get_bulk_safe(archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, str_hash *str_hash, char *const *keys, size_t nkeys)
{
        ERROR_IF_NULL(out);
        ERROR_IF_NULL(found_mask);
        ERROR_IF_NULL(num_not_found);
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->get_bulk_safe);
        int result = str_hash->get_bulk_safe(str_hash, out, found_mask, num_not_found, keys, nkeys);
        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found_mask != NULL);
        return result;
}

inline static int str_hash_get_bulk_safe_exact(archive_field_sid_t *out, bool *found, str_hash *str_hash, const char *key)
{
        ERROR_IF_NULL(out);
        ERROR_IF_NULL(found);
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(key);
        JAK_ASSERT(str_hash->get_exact_safe);
        int result = str_hash->get_exact_safe(str_hash, out, found, key);
        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found != NULL);
        return result;
}

/**
 * Get the values associated with <code>keys</code> in this str_hash. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate <code>num_keys</code>
 *            times <code>sizeof(archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static int str_hash_get_bulk_fast(archive_field_sid_t **out, str_hash *str_hash, char *const *keys, size_t nkeys)
{
        ERROR_IF_NULL(out);
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->get_fast);
        return str_hash->get_fast(str_hash, out, keys, nkeys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>string_hashtable_put_test</code> or
 * <code>string_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The str_hash will allocate <code>num_keys</code>
 *            times <code>sizeof(archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the ERROR.
 */
inline static int str_hash_update_fast(str_hash *str_hash, const archive_field_sid_t *values, char *const *keys, size_t nkeys)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->update_key_fast);
        return str_hash->update_key_fast(str_hash, values, keys, nkeys);
}

/**
 * Removes the objects with the gives keys from this str_hash
 *
 * @param str_hash a non-null pointer to the str_hash
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int str_hash_remove(str_hash *str_hash, char *const *keys, size_t nkeys)
{
        ERROR_IF_NULL(str_hash);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(str_hash->remove);
        return str_hash->remove(str_hash, keys, nkeys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>str_hash</code> that was specified
 * by the call to <code>string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>string_id_map_get</code>)
 * @return <code>true</code> in case of success, otherwise a value indiciating the ERROR.
 */
inline static int str_hash_free(void *ptr, str_hash *str_hash)
{
        ERROR_IF_NULL(ptr);
        ERROR_IF_NULL(str_hash);
        JAK_ASSERT(str_hash->free);
        return str_hash->free(str_hash, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return true if everything went normal, otherwise an value indicating the ERROR
 */
inline static int str_hash_counters_init(str_hash_counters *counters)
{
        ERROR_IF_NULL(counters);
        memset(counters, 0, sizeof(str_hash_counters));
        return true;
}

/**
 * Adds members of both input parameters and stores the result in <code>dstLhs</code>.
 *
 * @param dstLhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return true if everything went normal, otherwise an value indicating the ERROR
 */
inline static int str_hash_counters_add(str_hash_counters *dst_lhs, const str_hash_counters *rhs)
{
        ERROR_IF_NULL(dst_lhs);
        ERROR_IF_NULL(rhs);
        dst_lhs->num_bucket_search_miss += rhs->num_bucket_search_miss;
        dst_lhs->num_bucket_search_hit += rhs->num_bucket_search_hit;
        dst_lhs->num_bucket_cache_search_hit += rhs->num_bucket_cache_search_hit;
        dst_lhs->num_bucket_cache_search_miss += rhs->num_bucket_cache_search_miss;
        return true;
}

END_DECL

#endif
