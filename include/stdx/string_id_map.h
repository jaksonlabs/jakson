// file: string_id_map.h

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

#ifndef _NG5_STRING_ID_MAP
#define _NG5_STRING_ID_MAP

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <common.h>
#include <ng5/allocator.h>
#include <stdx/vector.h>
#include <stdx/unit.h>
#include "hash.h"

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

enum string_id_map_impl { STRING_ID_MAP_SIMPLE, STRING_ID_MAP_PARALLEL };

struct string_id_map {

  /**
   * Implementation-specific values
   */
  void *                  extra;

  /**
   * Implementation tag
   */
  enum string_id_map_impl tag;

  /**
  *  Memory allocator that is used to get memory for user data
  */
  struct allocator        allocator;

  /**
   *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
   */
  int (*drop)(struct string_id_map *self);

  /**
   * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key.
   */
  int (*put)(struct string_id_map *self, char *const *keys, const uint64_t *values, size_t num_pairs);

  /**
   * Get the values associated with <code>keys</code> in this map (if any).
   */
  int (*get)(struct string_id_map *self, uint64_t **out, bool **found_mask, size_t *num_not_found,
          char *const *keys, size_t num_keys);

  /**
   * Removes the objects with the gives keys from this map
   */
  int (*remove)(struct string_id_map *self, char *const *keys, size_t num_keys);

  /**
   * Frees up allocated memory for <code>ptr</code> via the allocator in <code>map</code> that was specified
   * by the call to <code>string_id_map_create</code>
   */
  int (*free)(struct string_id_map *self, void *ptr);
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Setups a new <code>struct string_id_map</code> which is a specialized hash table mapping a string (c-strings)
 * to an id (64bit values). This data structure is <b>thread-safe</b>.
 *
 * <b>Notes regarding the ownership</b>: Putting string keys into this map follows a hollow-copy strategy, i.e.,
 * pointers to strings used for keys are <u>copied</u> into the map. Whatever is behind these pointers is
 * <u>not owned</u> by this map. Especially, memory is not freed for these elements when the map is dropped. Moreover,
 * the caller must ensure not to corrupt whatever is behind these pointers during the lifetime of the map.
 *
 * @param map a non-null pointer to the new map
 * @param alloc the allocator that should be used for memory management. If this parameter is set to NULL, the
 *              default clib allocator will be used
 * @param num_buckets The number of buckets used in the map. The bucket list is resized, if conditions
 *                    'bucket_count_min' and 'entry_overflow_min' are satisfied (see below)
 * @param cap_buckets The reserved number of elements inside a bucket that are handled for collision resolving
 * @param bucket_grow_factor A value greater 1 that is used to determine the next reserved amount of memory if the
 *                           entry list inside a single bucket has an not enough space to store additionally elements
 * @return <code>STATUS_OK</code> is call is successful, or another status value indicating the error.
 */
int string_id_map_create_simple(struct string_id_map *map, const struct allocator *alloc, size_t num_buckets,
                                size_t cap_buckets, float bucket_grow_factor);

/**
 * Setups a new <code>struct string_id_map</code> which is a specialized hash table mapping a string (c-strings)
 * to an id (64bit values). This data structure is <b>thread-safe</b>.
 *
 * <b>Notes regarding the ownership</b>: Putting string keys into this map follows a hollow-copy strategy, i.e.,
 * pointers to strings used for keys are <u>copied</u> into the map. Whatever is behind these pointers is
 * <u>not owned</u> by this map. Especially, memory is not freed for these elements when the map is dropped. Moreover,
 * the caller must ensure not to corrupt whatever is behind these pointers during the lifetime of the map.
 *
 * @param map a non-null pointer to the new map
 * @param alloc the allocator that should be used for memory management. If this parameter is set to NULL, the
 *              default clib allocator will be used
 * @param num_buckets The number of buckets used in the map. The bucket list is resized, if conditions
 *                    'bucket_count_min' and 'entry_overflow_min' are satisfied (see below)
 * @param cap_buckets The reserved number of elements inside a bucket that are handled for collision resolving
 * @param map_grow_factor A value greater 1 that is used to determine the next reserved amount of memory if a rehashing
 *                        is invoked due to massive overflow
 * @param bucket_grow_factor A value greater 1 that is used to determine the next reserved amount of memory if the
 *                           entry list inside a single bucket has an not enough space to store additionally elements
 * @param max_memory_size The maximum size in byte this map is allowed to occupy during the runtime. The minimum size is
 *                        the memory footprint of a single bucket map managing having one parition with one entry in each
 *                        cache level of that single bucket. In case <code>max_memory_size</code> is less than this size,
 *                        <code>max_memory_size</code> is ignored.
 * @return <code>STATUS_OK</code> is call is successful, or another status value indicating the error.
 */
int string_id_map_create_parallel(struct string_id_map *map, const struct allocator *alloc, size_t num_buckets,
                                  size_t cap_buckets, float map_grow_factor, float bucket_grow_factor,
                                  size_t max_memory_size, const char *swap_space_dir);

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
inline static int string_id_map_drop(struct string_id_map *map)
{
    check_non_null(map);
    assert(map->drop);

    return map->drop(map);
}

/**
 * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key.
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null constant pointer to a list of at least <code>num_pairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>num_pairs</code> length of 64bit values
 * @param num_pairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int string_id_map_put(struct string_id_map *map, char *const*keys, const uint64_t *values,
                                    size_t num_pairs)
{
    check_non_null(map);
    check_non_null(keys);
    check_non_null(values);
    assert(map->put);

    return map->put(map, keys, values, num_pairs);
}

/**
 * Get the values associated with <code>keys</code> in this map (if any).
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
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int string_id_map_get(uint64_t **out, bool **found_mask, size_t *num_not_found, struct string_id_map *map,
                                    char *const *keys, size_t num_keys)
{
    check_non_null(out);
    check_non_null(found_mask);
    check_non_null(num_not_found);
    check_non_null(map);
    check_non_null(keys);
    assert(map->get);

    return map->get(map, out, found_mask, num_not_found, keys, num_keys);
}

/**
 * Removes the objects with the gives keys from this map
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>num_keys</code> strings
 * @param num_keys the number of keys
 * @return
 */
inline static int string_id_map_remove(struct string_id_map *map, char *const *keys, size_t num_keys)
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
inline static int string_id_map_free(void *ptr, struct string_id_map *map)
{
    check_non_null(ptr);
    check_non_null(map);
    assert(map->free);

    return map->free(map, ptr);
}

#endif