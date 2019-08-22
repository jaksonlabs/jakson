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
 * Internally, the jak_async_map_exec is organized by paritions where each partition is assigned exclusively to
 * one thread. Each such parition contains of as a vector of length 'num_buckets' which contains of elements of a bucket
 * type. A bucket type is an fixed-size array of entries, each containing a key and a value. In case of no collisions,
 * this array contains of exactly one element. In case of collisions, colliding keys are stored in this array of entries
 * in the same bucket. To speedup lookups, this entry vector may be additionally sorted and a (specialized) binary
 * search is invoked to find the bucket entry associated to a particular key (if any). Other lookup strategies includes
 * both single and multi-threaded forward scans. Which strategy to use in which case is decided by the
 * jak_async_map_exec itself, however. To satisfy user-specific memory limitations, some per-bucket elements may
 * be swapped out to a didicated swap space.
 *
 * The underlying hashing function is the Jenkins hash function.
 */

#ifndef JAK_STRHASH_H
#define JAK_STRHASH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_vector.h>
#include <jak_hash.h>
#include <jak_types.h>

JAK_BEGIN_DECL

/**
 * Enables or disabled packing of entries inside a bucket. By default, packing is disabled.
 * To turn on packing, set 'JAK_CONFIG_PACK_BUCKETS' symbol
 */
#ifdef JAK_CONFIG_PACK_BUCKETS
#define
#define JAK_BUCKET_PACKING __attribute__((__packed__))
#else
#define JAK_BUCKET_PACKING
#endif

/**
 * Number of elements stored in the per-bucket cache (Tier 2, see below)
 */
#ifndef JAK_CONFIG_BUCKET_CACHE_SIZE
#define JAK_CONFIG_BUCKET_CACHE_SIZE  16
#endif

/**
 * Maximum number of elements stored per bucket (Tier 3, see below)
 */
#ifndef JAK_CONFIG_BUCKET_CAPACITY
#define JAK_CONFIG_BUCKET_CAPACITY  1024
#endif

struct jak_str_hash;

enum strhash_tag {
        MEMORY_RESIDENT
};

struct jak_str_hash_counters {
        size_t num_bucket_search_miss;
        size_t num_bucket_search_hit;
        size_t num_bucket_cache_search_miss;
        size_t num_bucket_cache_search_hit;
};

struct jak_str_hash {
        /**
         * Implementation-specific values
         */
        void *extra;

        /**
         * Implementation tag
         */
        enum strhash_tag tag;

        /**
         * Statistics to lookup misses and hits
         *
         * <b>Note</b>: Implementation must maintain counters by itself
         */
        struct jak_str_hash_counters counters;

        /**
        *  Memory allocator that is used to get memory for user data
        */
        jak_allocator allocator;

        /**
         *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
         */
        int (*drop)(struct jak_str_hash *self);

        /**
         * Put <code>num_pair</code> objects into this jak_async_map_exec maybe updating old objects with the same key.
         */
        int (*put_bulk_safe)(struct jak_str_hash *self, char *const *keys, const jak_archive_field_sid_t *values,
                             size_t npairs);

        /**
         * Put <code>num_pair</code> objects into this jak_async_map_exec maybe without checking for updates.
         */
        int (*put_bulk_fast)(struct jak_str_hash *self, char *const *keys, const jak_archive_field_sid_t *values,
                             size_t npairs);

        /**
         * Same as 'put_safe_bulk' but specialized for a single element
         */
        int (*put_exact_safe)(struct jak_str_hash *self, const char *key, jak_archive_field_sid_t value);

        /**
         * Same as 'put_fast_bulk' but specialized for a single element
         */
        int (*put_exact_fast)(struct jak_str_hash *self, const char *key, jak_archive_field_sid_t value);

        /**
         * Get the values associated with <code>keys</code> in this jak_async_map_exec (if any).
         */
        int (*get_bulk_safe)(struct jak_str_hash *self, jak_archive_field_sid_t **out, bool **found_mask,
                             size_t *nnot_found,
                             char *const *keys, size_t nkeys);

        /**
         * The same as 'get_safe_bulk' but optimized for a single element
         */
        int
        (*get_exact_safe)(struct jak_str_hash *self, jak_archive_field_sid_t *out, bool *found_mask, const char *key);

        /**
         * Get the values associated with <code>keys</code> in this jak_async_map_exec. All keys <u>must</u> exist.
         */
        int (*get_fast)(struct jak_str_hash *self, jak_archive_field_sid_t **out, char *const *keys, size_t nkeys);

        /**
         * Updates keys associated with <code>values</code> in this jak_async_map_exec. All values <u>must</u> exist, and the
         * mapping between keys and values must be bidirectional.
         */
        int (*update_key_fast)(struct jak_str_hash *self, const jak_archive_field_sid_t *values, char *const *keys,
                               size_t nkeys);

        /**
         * Removes the objects with the gives keys from this jak_async_map_exec
         */
        int (*remove)(struct jak_str_hash *self, char *const *keys, size_t nkeys);

        /**
         * Frees up allocated memory for <code>ptr</code> via the allocator in <code>jak_async_map_exec</code> that was specified
         * by the call to <code>string_id_map_create</code>
         */
        int (*free)(struct jak_str_hash *self, void *ptr);

        /**
         *  Error information
         */
        struct jak_error err;

};

JAK_DEFINE_GET_ERROR_FUNCTION(strhash, struct jak_str_hash, table);

/**
 * Frees resources bound to <code>jak_async_map_exec</code> via the allocator specified by the call to <code>string_id_map_create</code>.
 *
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int strhash_drop(struct jak_str_hash *jak_async_map_exec)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ASSERT(jak_async_map_exec->drop);

        return jak_async_map_exec->drop(jak_async_map_exec);
}

/**
 * Resets statistics counters
 *
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static bool strhash_reset_counters(struct jak_str_hash *jak_async_map_exec)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        memset(&jak_async_map_exec->counters, 0, sizeof(struct jak_str_hash_counters));
        return true;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param jak_async_map_exec non-null pointer to the jak_async_map_exec
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int strhash_get_counters(struct jak_str_hash_counters *out, const struct jak_str_hash *jak_async_map_exec)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(out);
        *out = jak_async_map_exec->counters;
        return true;
}

/**
 * Put <code>num_pair</code> objects into this jak_async_map_exec maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>string_hashtable_put_blind</code>
 * instead.
 *
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
strhash_put_safe(struct jak_str_hash *jak_async_map_exec, char *const *keys, const jak_archive_field_sid_t *values,
                 size_t npairs)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ERROR_IF_NULL(values);
        JAK_ASSERT(jak_async_map_exec->put_bulk_safe);

        return jak_async_map_exec->put_bulk_safe(jak_async_map_exec, keys, values, npairs);
}

/**
 * Put <code>num_pair</code> objects into this jak_async_map_exec, ingoring whether the key exists or not. This function is
 * useful for insert operations of pairs where it is guaranteed that the keys are not yet inserted into this hashtable.
 * In case this guarantee is broken, the behavior is undefined. Depending on the implementation, this specialized
 * <code>put</code> function may have a better performance.
 *
 * If a check for existence is required, use <code>string_hashtable_put_test</code>
 * instead.
 *
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int
strhash_put_bulk_fast(struct jak_str_hash *jak_async_map_exec, char *const *keys, const jak_archive_field_sid_t *values,
                      size_t npairs)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ERROR_IF_NULL(values);
        JAK_ASSERT(jak_async_map_exec->put_bulk_fast);

        return jak_async_map_exec->put_bulk_fast(jak_async_map_exec, keys, values, npairs);
}

/**
 * Same as 'string_lookup_put_bulk' but specialized for a single pair
 */
inline static int
strhash_put_exact(struct jak_str_hash *jak_async_map_exec, const char *key, jak_archive_field_sid_t value)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(key);
        JAK_ASSERT(jak_async_map_exec->put_exact_safe);

        return jak_async_map_exec->put_exact_safe(jak_async_map_exec, key, value);
}

/**
 * Same as 'string_lookup_put_fast_bulk' but specialized for a single pair
 */
inline static int
strhash_put_exact_fast(struct jak_str_hash *jak_async_map_exec, const char *key, jak_archive_field_sid_t value)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(key);

        JAK_ASSERT(jak_async_map_exec->put_exact_fast);

        return jak_async_map_exec->put_exact_fast(jak_async_map_exec, key, value);
}

/**
 * Get the values associated with <code>keys</code> in this jak_async_map_exec (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>found_mask</code> and
 * <code>num_not_found</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The jak_async_map_exec will allocate enough memory to store the
 *            result. There are <code>num_keys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this jak_async_map_exec. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>found_mask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>string_id_map_free</code>.
 * @param found_mask A non-null pointer to an unallocated memory address. The jak_async_map_exec will allocate enough memory to store
 *            the result. There are <code>num_keys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this jak_async_map_exec. If this is the case, the i-th entry in <code>found_mask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>found_mask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param num_not_found A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this jak_async_map_exec.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int strhash_get_bulk_safe(jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                                        struct jak_str_hash *jak_async_map_exec, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found_mask);
        JAK_ERROR_IF_NULL(num_not_found);
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(jak_async_map_exec->get_bulk_safe);

        int result = jak_async_map_exec->get_bulk_safe(jak_async_map_exec, out, found_mask, num_not_found, keys, nkeys);

        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found_mask != NULL);

        return result;
}

inline static int
strhash_get_bulk_safe_exact(jak_archive_field_sid_t *out, bool *found, struct jak_str_hash *jak_async_map_exec,
                            const char *key)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found);
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(key);
        JAK_ASSERT(jak_async_map_exec->get_exact_safe);

        int result = jak_async_map_exec->get_exact_safe(jak_async_map_exec, out, found, key);

        JAK_ASSERT (out != NULL);
        JAK_ASSERT (found != NULL);

        return result;
}

/**
 * Get the values associated with <code>keys</code> in this jak_async_map_exec. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The jak_async_map_exec will allocate <code>num_keys</code>
 *            times <code>sizeof(jak_archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
strhash_get_bulk_fast(jak_archive_field_sid_t **out, struct jak_str_hash *jak_async_map_exec, char *const *keys,
                      size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(jak_async_map_exec->get_fast);

        return jak_async_map_exec->get_fast(jak_async_map_exec, out, keys, nkeys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>string_hashtable_put_test</code> or
 * <code>string_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The jak_async_map_exec will allocate <code>num_keys</code>
 *            times <code>sizeof(jak_archive_field_sid_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>true</code> in case of success, otherwise a value indicating the error.
 */
inline static int
strhash_update_fast(struct jak_str_hash *jak_async_map_exec, const jak_archive_field_sid_t *values, char *const *keys,
                    size_t nkeys)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(jak_async_map_exec->update_key_fast);

        return jak_async_map_exec->update_key_fast(jak_async_map_exec, values, keys, nkeys);
}

/**
 * Removes the objects with the gives keys from this jak_async_map_exec
 *
 * @param jak_async_map_exec a non-null pointer to the jak_async_map_exec
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int strhash_remove(struct jak_str_hash *jak_async_map_exec, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(jak_async_map_exec->remove);

        return jak_async_map_exec->remove(jak_async_map_exec, keys, nkeys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>jak_async_map_exec</code> that was specified
 * by the call to <code>string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>string_id_map_get</code>)
 * @return <code>true</code> in case of success, otherwise a value indiciating the error.
 */
inline static int strhash_free(void *ptr, struct jak_str_hash *jak_async_map_exec)
{
        JAK_ERROR_IF_NULL(ptr);
        JAK_ERROR_IF_NULL(jak_async_map_exec);
        JAK_ASSERT(jak_async_map_exec->free);

        return jak_async_map_exec->free(jak_async_map_exec, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return true if everything went normal, otherwise an value indicating the error
 */
inline static int strhash_counters_init(struct jak_str_hash_counters *counters)
{
        JAK_ERROR_IF_NULL(counters);
        memset(counters, 0, sizeof(struct jak_str_hash_counters));
        return true;
}

/**
 * Adds members of both input parameters and stores the result in <code>dstLhs</code>.
 *
 * @param dstLhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return true if everything went normal, otherwise an value indicating the error
 */
inline static int strhash_counters_add(struct jak_str_hash_counters *dst_lhs, const struct jak_str_hash_counters *rhs)
{
        JAK_ERROR_IF_NULL(dst_lhs);
        JAK_ERROR_IF_NULL(rhs);
        dst_lhs->num_bucket_search_miss += rhs->num_bucket_search_miss;
        dst_lhs->num_bucket_search_hit += rhs->num_bucket_search_hit;
        dst_lhs->num_bucket_cache_search_hit += rhs->num_bucket_cache_search_hit;
        dst_lhs->num_bucket_cache_search_miss += rhs->num_bucket_cache_search_miss;
        return true;
}

JAK_END_DECL

#endif
