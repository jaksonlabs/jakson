// file: string_hashtable.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <http://www.gnu.org/licenses/>.
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
 * Internally, the map is organized by paritions where each partition is assigned exclusively to one thread. Each
 * such parition contains of as a vector of length 'num_buckets' which contains of elements of a bucket type.
 * A bucket type is an fixed-size array of entries, each containing a key and a value. In case of no collisions, this
 * array contains of exactly one element. In case of collisions, colliding keys are stored in this array of entries in
 * the same bucket. To speedup lookups, this entry vector may be additionally sorted and a (specialized) binary search
 * is invoked to find the bucket entry associated to a particular key (if any). Other lookup strategies includes
 * both single and multi-threaded forward scans. Which strategy to use in which case is decided by the map itself,
 * however. To satisfy user-specific memory limitations, some per-bucket elements may be swapped out to a didicated
 * swap space.
 *
 * The underlying hashing function is the Jenkins hash function.
 */

#ifndef _NG5_STRING_HASHTABLE
#define _NG5_STRING_HASHTABLE

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <ng5_common.h>
#include <stdx/ng5_allocator.h>
#include <stdx/ng5_vector.h>
#include <stdx/ng5_unit.h>
#include "ng5_hash.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  S T A T I C   C O N F I G U R A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Enables or disabled packing of entries inside a bucket. By default, packing is disabled.
 * To turn on packing, set 'NG5_CONFIG_PACK_BUCKETS' symbol
 */
#ifdef NG5_CONFIG_PACK_BUCKETS
#define
#define NG5_BUCKET_PACKING __attribute__((__packed__))
#else
#define NG5_BUCKET_PACKING
#endif

/**
 * Number of elements stored in the per-bucket cache (Tier 2, see below)
 */
#ifndef NG5_CONFIG_BUCKET_CACHE_SIZE
#define NG5_CONFIG_BUCKET_CACHE_SIZE  16
#endif

/**
 * Maximum number of elements stored per bucket (Tier 3, see below)
 */
#ifndef NG5_CONFIG_BUCKET_CAPACITY
#define NG5_CONFIG_BUCKET_CAPACITY  1024
#endif

enum string_lookup_impl { STRING_ID_MAP_SIMPLE };

struct string_map_counters {
  size_t num_bucket_search_miss;
  size_t num_bucket_search_hit;
  size_t num_bucket_cache_search_miss;
  size_t num_bucket_cache_search_hit;
};

struct string_map {

  /**
   * Implementation-specific values
   */
  void *                     extra;

  /**
   * Implementation tag
   */
  enum string_lookup_impl    tag;

  /*
   * Statistics to lookup misses and hits
   *
   * <b>Note</b>: Implementation must maintain counters by itself
   */
  struct string_map_counters counters;

  /**
  *  Memory allocator that is used to get memory for user data
  */
  ng5_allocator_t               allocator;

  /**
   *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
   */
  int (*drop)(struct string_map *self);

  /**
   * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key.
   */
  int (*put_safe)(struct string_map *self, char *const *keys, const string_id_t *values, size_t num_pairs);

  /**
   * Put <code>num_pair</code> objects into this map maybe without checking for updates.
   */
  int (*put_fast)(struct string_map *self, char *const *keys, const string_id_t *values, size_t num_pairs);

  /**
   * Get the values associated with <code>keys</code> in this map (if any).
   */
  int (*get_safe)(struct string_map *self, string_id_t **out, bool **found_mask, size_t *num_not_found,
          char *const *keys, size_t num_keys);

  /**
   * Get the values associated with <code>keys</code> in this map. All keys <u>must</u> exist.
   */
  int (*get_fast)(struct string_map *self, string_id_t **out, char *const *keys, size_t num_keys);

  /**
   * Updates keys associated with <code>values</code> in this map. All values <u>must</u> exist, and the
   * mapping between keys and values must be bidirectional.
   */
  int (*update_key_fast)(struct string_map *self, const string_id_t *values, char *const *keys, size_t num_keys);

  /**
   * Removes the objects with the gives keys from this map
   */
  int (*remove)(struct string_map *self, char *const *keys, size_t num_keys);

  /**
   * Frees up allocated memory for <code>ptr</code> via the allocator in <code>map</code> that was specified
   * by the call to <code>string_id_map_create</code>
   */
  int (*free)(struct string_map *self, void *ptr);

};

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   D E L E G A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Frees resources bound to <code>map</code> via the allocator specified by the call to <code>string_id_map_create</code>.
 *
 * @param map a non-null pointer to the map
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int string_lookup_drop(struct string_map* map)
{
    check_non_null(map);
    assert(map->drop);

    return map->drop(map);
}

/**
 * Resets statistics counters
 *
 * @param map a non-null pointer to the map
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_reset_counters(struct string_map* map)
{
    check_non_null(map);
    memset(&map->counters, 0, sizeof(struct string_map_counters));
    return STATUS_OK;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param map non-null pointer to the map
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_counters(struct string_map_counters *out, const struct string_map *map)
{
    check_non_null(map);
    check_non_null(out);
    *out = map->counters;
    return STATUS_OK;
}

/**
 * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>string_hashtable_put_blind</code>
 * instead.
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_put_safe(struct string_map* map, char* const* keys, const string_id_t* values,
        size_t num_pairs)
{
    check_non_null(map);
    check_non_null(keys);
    check_non_null(values);
    assert(map->put_safe);

    return map->put_safe(map, keys, values, num_pairs);
}

/**
 * Put <code>num_pair</code> objects into this map, ingoring whether the key exists or not. This function is
 * useful for insert operations of pairs where it is guaranteed that the keys are not yet inserted into this hashtable.
 * In case this guarantee is broken, the behavior is undefined. Depending on the implementation, this specialized
 * <code>put</code> function may have a better performance.
 *
 * If a check for existence is required, use <code>string_hashtable_put_test</code>
 * instead.
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int string_lookup_put_fast(struct string_map* map, char* const* keys, const string_id_t* values,
        size_t num_pairs)
{
    check_non_null(map);
    check_non_null(keys);
    check_non_null(values);
    assert(map->put_fast);

    return map->put_fast(map, keys, values, num_pairs);
}

/**
 * Get the values associated with <code>keys</code> in this map (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>found_mask</code> and
 * <code>num_not_found</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate enough memory to store the
 *            result. There are <code>num_keys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this map. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>found_mask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>string_id_map_free</code>.
 * @param found_mask A non-null pointer to an unallocated memory address. The map will allocate enough memory to store
 *            the result. There are <code>num_keys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this map. If this is the case, the i-th entry in <code>found_mask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>found_mask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param num_not_found A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this map.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_get_safe(string_id_t** out, bool** found_mask, size_t* num_not_found,
        struct string_map* map,
        char* const* keys, size_t num_keys)
{
    check_non_null(out);
    check_non_null(found_mask);
    check_non_null(num_not_found);
    check_non_null(map);
    check_non_null(keys);
    assert(map->get_safe);

    int result = map->get_safe(map, out, found_mask, num_not_found, keys, num_keys);

    assert (out != NULL);
    assert (found_mask != NULL);

    return result;
}

/**
 * Get the values associated with <code>keys</code> in this map. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate <code>num_keys</code>
 *            times <code>sizeof(string_id_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_get_fast(string_id_t** out, struct string_map* map,
        char* const* keys, size_t num_keys)
{
    check_non_null(out);
    check_non_null(map);
    check_non_null(keys);
    assert(map->get_fast);

    return map->get_fast(map, out, keys, num_keys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>string_hashtable_put_test</code> or
 * <code>string_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate <code>num_keys</code>
 *            times <code>sizeof(string_id_t)</code> bytes memory to store the result. There are <code>num_keys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int string_lookup_update_fast(struct string_map* map, const string_id_t* values,
        char* const* keys, size_t num_keys)
{
    check_non_null(map);
    check_non_null(keys);
    assert(map->update_key_fast);

    return map->update_key_fast(map, values, keys, num_keys);
}

/**
 * Removes the objects with the gives keys from this map
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int string_lookup_remove(struct string_map* map, char* const* keys, size_t num_keys)
{
    check_non_null(map);
    check_non_null(keys);
    assert(map->remove);

    return map->remove(map, keys, num_keys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>map</code> that was specified
 * by the call to <code>string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>string_id_map_get</code>)
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int string_lookup_free(void* ptr, struct string_map* map)
{
    check_non_null(ptr);
    check_non_null(map);
    assert(map->free);

    return map->free(map, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return STATUS_OK if everything went normal, otherwise an value indicating the error
 */
inline static int string_map_counters_init(struct string_map_counters *counters) {
    check_non_null(counters);
    memset(counters, 0, sizeof(struct string_map_counters));
    return STATUS_OK;
}

/**
 * Adds members of both input parameters and stores the result in <code>dst_lhs</code>.
 *
 * @param dst_lhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return STATUS_OK if everything went normal, otherwise an value indicating the error
 */
inline static int string_map_counters_add(struct string_map_counters *dst_lhs, const struct string_map_counters *rhs)
{
    check_non_null(dst_lhs);
    check_non_null(rhs);
    dst_lhs->num_bucket_search_miss       += rhs->num_bucket_search_miss;
    dst_lhs->num_bucket_search_hit        += rhs->num_bucket_search_hit;
    dst_lhs->num_bucket_cache_search_hit  += rhs->num_bucket_cache_search_hit;
    dst_lhs->num_bucket_cache_search_miss += rhs->num_bucket_cache_search_miss;
    return STATUS_OK;
}

#endif