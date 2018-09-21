// file: strhash.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#ifndef NG5_STRHASH
#define NG5_STRHASH

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "alloc.h"
#include "vector.h"
#include "hash.h"

NG5_BEGIN_DECL

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E   F O R W A R D I N G
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct StringHashTable StringHashTable;

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

enum StringHashTableTag
{
    STRINGHASHTABLE_MEM
};

typedef struct StringHashCounters
{
    size_t numBucketSearchMiss;
    size_t numBucketSearchHit;
    size_t numBucketCacheSearchMiss;
    size_t numBucketCacheSearchHit;
} StringHashCounters;

typedef struct StringHashTable
{

    /**
     * Implementation-specific values
     */
    void *extra;

    /**
     * Implementation tag
     */
    enum StringHashTableTag tag;

    /*
     * Statistics to lookup misses and hits
     *
     * <b>Note</b>: Implementation must maintain counters by itself
     */
    struct StringHashCounters counters;

    /**
    *  Memory allocator that is used to get memory for user data
    */
    Allocator allocator;

    /**
     *  Frees resources bound to <code>self</code> via the allocator specified by the constructor
     */
    int (*drop)(StringHashTable *self);

    /**
     * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key.
     */
    int (*putSafeBulk)(StringHashTable *self, char *const *keys, const StringId *values, size_t numPairs);

    /**
     * Put <code>num_pair</code> objects into this map maybe without checking for updates.
     */
    int (*putFastBulk)(StringHashTable *self, char *const *keys, const StringId *values, size_t numPairs);

    /**
     * Same as 'put_safe_bulk' but specialized for a single element
     */
    int (*putSafeExact)(StringHashTable *self, const char *key, StringId value);

    /**
     * Same as 'put_fast_bulk' but specialized for a single element
     */
    int (*putFastExact)(StringHashTable *self, const char *key, StringId value);

    /**
     * Get the values associated with <code>keys</code> in this map (if any).
     */
    int (*getSafeBulk)(StringHashTable *self, StringId **out, bool **foundMask, size_t *numNotFound,
                         char *const *keys, size_t numKeys);

    /**
     * The same as 'get_safe_bulk' but optimized for a single element
     */
    int (*getSafeExact)(StringHashTable *self, StringId *out, bool *foundMask, const char *key);

    /**
     * Get the values associated with <code>keys</code> in this map. All keys <u>must</u> exist.
     */
    int (*getFast)(StringHashTable *self, StringId **out, char *const *keys, size_t numKeys);

    /**
     * Updates keys associated with <code>values</code> in this map. All values <u>must</u> exist, and the
     * mapping between keys and values must be bidirectional.
     */
    int (*updateKeyFast)(StringHashTable *self, const StringId *values, char *const *keys, size_t numKeys);

    /**
     * Removes the objects with the gives keys from this map
     */
    int (*remove)(StringHashTable *self, char *const *keys, size_t numKeys);

    /**
     * Frees up allocated memory for <code>ptr</code> via the allocator in <code>map</code> that was specified
     * by the call to <code>string_id_map_create</code>
     */
    int (*free)(StringHashTable *self, void *ptr);

} StringHashTable;

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
inline static int StringHashTableDrop(StringHashTable *map)
{
    CHECK_NON_NULL(map);
    assert(map->drop);

    return map->drop(map);
}

/**
 * Resets statistics counters
 *
 * @param map a non-null pointer to the map
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTableResetCounters(StringHashTable *map)
{
    CHECK_NON_NULL(map);
    memset(&map->counters, 0, sizeof(struct StringHashCounters));
    return STATUS_OK;
}

/**
 * Returns statistics counters
 * @param out non-null pointer to destination counter
 * @param map non-null pointer to the map
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTableCounters(StringHashCounters *out, const StringHashTable *map)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(out);
    *out = map->counters;
    return STATUS_OK;
}

/**
 * Put <code>num_pair</code> objects into this map maybe updating old objects with the same key. If it is
 * guaranteed that the key is not yet inserted into this table, use <code>string_hashtable_put_blind</code>
 * instead.
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null constant pointer to a list of at least <code>numPairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>numPairs</code> length of 64bit values
 * @param numPairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTablePutSafe(StringHashTable *map, char *const *keys, const StringId *values,
                                         size_t numPairs)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    CHECK_NON_NULL(values);
    assert(map->putSafeBulk);

    return map->putSafeBulk(map, keys, values, numPairs);
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
 * @param keys a non-null constant pointer to a list of at least <code>numPairs</code> length of constant strings
 * @param values a non-null constant pointer to a list of at least <code>numPairs</code> length of 64bit values
 * @param numPairs the number of pairs that are read via <code>keys</code> and <code>values</code>
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int StringHashTablePutFastBulk(StringHashTable *map, char *const *keys, const StringId *values,
                                             size_t numPairs)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    CHECK_NON_NULL(values);
    assert(map->putFastBulk);

    return map->putFastBulk(map, keys, values, numPairs);
}

/**
 * Same as 'string_lookup_put_bulk' but specialized for a single pair
 */
inline static int StringHashTablePutExact(StringHashTable *map, const char *key, StringId value)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(key);
    assert(map->putSafeExact);

    return map->putSafeExact(map, key, value);
}

/**
 * Same as 'string_lookup_put_fast_bulk' but specialized for a single pair
 */
inline static int StringHashTablePutFastExact(StringHashTable *map, const char *key, StringId value)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(key);

    assert(map->putFastExact);

    return map->putFastExact(map, key, value);
}

/**
 * Get the values associated with <code>keys</code> in this map (if any). In case one <code>key</code> does not
 * exists, the function will return this information via the parameters <code>foundMask</code> and
 * <code>numNotFound</code>. However, in case it is guaranteed that all keys exist, consider to use
 * <code>string_id_map_get_blind</code> instead. *
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate enough memory to store the
 *            result. There are <code>numKeys</code> elements returned, but not all of them are guaranteed to
 *            contain a particular value. That an entry does not contain a particular value happens if the
 *            associated key is not stored in this map. Whether or not one particular entry is a valid value,
 *            can be determined by the caller via the <code>foundMask</code>.
 *            <b>Important</b> <code>out</code> must be freed manually by calling <code>string_id_map_free</code>.
 * @param foundMask A non-null pointer to an unallocated memory address. The map will allocate enough memory to store
 *            the result. There are <code>numKeys</code> boolean values returned. This mask is used to determine
 *            if the i-th key has a mapping in this map. If this is the case, the i-th entry in <code>foundMask</code>
 *            is <b>true</b> and the i-th entry in <code>out</code> holds the value. Otherwise, in case the i-th
 *            value in <code>foundMask</code> is <b>false</b>, there is no value stored to the i-th key in
 *            <code>keys</code>, and reading <code>out</code> for the i-th position is undefined.
 * @param numNotFound A non-null pointer to a value that will store the number of keys in <code>keys</code> for
 *                      which no value is stored in this map.
 * @param num_out A non-null pointer to an unsigned integer that will contain the number of values return by the
 *                call to this function.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>numKeys</code> strings
 * @param numKeys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTableGetSafeBulk(StringId **out, bool **foundMask, size_t *numNotFound,
                                             StringHashTable *map,
                                             char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(foundMask);
    CHECK_NON_NULL(numNotFound);
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    assert(map->getSafeBulk);

    int result = map->getSafeBulk(map, out, foundMask, numNotFound, keys, numKeys);

    assert (out != NULL);
    assert (foundMask != NULL);

    return result;
}

inline static int StringHashTableGetSafeExact(StringId *out, bool *found, StringHashTable *map, const char *key)
{
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(found);
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(key);
    assert(map->getSafeExact);

    int result = map->getSafeExact(map, out, found, key);

    assert (out != NULL);
    assert (found != NULL);

    return result;
}

/**
 * Get the values associated with <code>keys</code> in this map. In case one <code>key</code> does not
 * exists, the behavior is undefined.
 *
 * However, if it cannot be guaranteed that all keys are known, use
 * <code>string_id_map_get_test</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate <code>numKeys</code>
 *            times <code>sizeof(StringId)</code> bytes memory to store the result. There are <code>numKeys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>numKeys</code> strings
 * @param numKeys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTableGetFast(StringId **out, StringHashTable *map,
                                         char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    assert(map->getFast);

    return map->getFast(map, out, keys, numKeys);
}

/**
 * Update keys for a given list of values. It must be guaranteed that the mapping between a key and its value is
 * bidirectional, and that all values exists.
 *
 * If you want to update a value given its key, use <code>string_hashtable_put_test</code> or
 * <code>string_hashtable_put_blind</code> instead.
 *
 * @param out A non-null pointer to an unallocated memory address. The map will allocate <code>numKeys</code>
 *            times <code>sizeof(StringId)</code> bytes memory to store the result. There are <code>numKeys</code>
 *            elements returned, and all of them are guaranteed to contain a particular value.
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>numKeys</code> strings
 * @param numKeys the number of keys
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indicating the error.
 */
inline static int StringHashTableUpdateFast(StringHashTable *map, const StringId *values,
                                            char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    assert(map->updateKeyFast);

    return map->updateKeyFast(map, values, keys, numKeys);
}

/**
 * Removes the objects with the gives keys from this map
 *
 * @param map a non-null pointer to the map
 * @param keys a non-null pointer to a list of at least <code>numKeys</code> strings
 * @param numKeys the number of keys
 * @return
 */
inline static int StringHashTableRemove(StringHashTable *map, char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(map);
    CHECK_NON_NULL(keys);
    assert(map->remove);

    return map->remove(map, keys, numKeys);
}

/**
 * Frees up allocated memory for <code>values</code> via the allocator in <code>map</code> that was specified
 * by the call to <code>string_id_map_create</code>
 *
 * @param values A non-null pointer (potentially resulting from a call to <code>string_id_map_get</code>)
 * @return <code>STATUS_OK</code> in case of success, otherwise a value indiciating the error.
 */
inline static int StringHashTableUnalloc(void *ptr, StringHashTable *map)
{
    CHECK_NON_NULL(ptr);
    CHECK_NON_NULL(map);
    assert(map->free);

    return map->free(map, ptr);
}

/**
 * Resets the counter <code>counters</code> by setting all members to zero.
 *
 * @param counters non-null pointer to counter object
 * @return STATUS_OK if everything went normal, otherwise an value indicating the error
 */
inline static int StringHashTableCountersInit(StringHashCounters *counters)
{
    CHECK_NON_NULL(counters);
    memset(counters, 0, sizeof(StringHashCounters));
    return STATUS_OK;
}

/**
 * Adds members of both input parameters and stores the result in <code>dstLhs</code>.
 *
 * @param dstLhs non-null pointer to counter (will contain the result)
 * @param rhs non-null pointer to counter
 * @return STATUS_OK if everything went normal, otherwise an value indicating the error
 */
inline static int StringHashTableCountersAdd(StringHashCounters *dstLhs, const StringHashCounters *rhs)
{
    CHECK_NON_NULL(dstLhs);
    CHECK_NON_NULL(rhs);
    dstLhs->numBucketSearchMiss += rhs->numBucketSearchMiss;
    dstLhs->numBucketSearchHit += rhs->numBucketSearchHit;
    dstLhs->numBucketCacheSearchHit += rhs->numBucketCacheSearchHit;
    dstLhs->numBucketCacheSearchMiss += rhs->numBucketCacheSearchMiss;
    return STATUS_OK;
}

NG5_END_DECL

#endif