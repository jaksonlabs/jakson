// file: strdic.h

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

#ifndef NG5_STRDIC
#define NG5_STRDIC

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "hash.h"
#include "alloc.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E   F O R W A R D I N G
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct StringDictionary StringDictionary;
typedef struct StringHashCounters StringHashCounters;

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

enum StrDicTag
{
    STRDIC_SYNC, STRDIC_ASYNC
};

/**
 * Thread-safe string pool implementation
 */
typedef struct StringDictionary
{
    /**
     * Implementation-specific fields
     */
    void *extra;

    /**
     * Tag determining the current implementation
     */
    enum StrDicTag tag;

    /**
     * Memory allocator that is used to get memory for user data
     */
    Allocator alloc;

    /**
     * Frees up implementation-specific resources.
     *
     * Note: Implementation must ensure thread-safeness
     */
    int (*drop)(StringDictionary *self);

    /**
     * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
     *
     * Note: Implementation must ensure thread-safeness
    */
    int (*insert)(StringDictionary *self, StringId **out, char *const *strings,
                  size_t numStrings, size_t numThreads);

    /**
     * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
     * all string identifiers in <code>strings</code> are valid.
     *
     * Note: Implementation must ensure thread-safeness
     */
    int (*remove)(StringDictionary *self, StringId *strings, size_t numStrings);

    /**
     * Get the string ids associated with <code>keys</code> in this map (if any).
     *
     * Note: Implementation must ensure thread-safeness
     */
    int (*locateSafe)(StringDictionary *self, StringId **out, bool **foundMask,
                       size_t *numNotFound, char *const *keys, size_t numKeys);

    /**
     * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
     *
     * Note: Implementation must ensure thread-safeness
    */
    int (*locateFast)(StringDictionary *self, StringId **out, char *const *keys,
                       size_t numKeys);

    /**
     * Extracts strings given their string identifier. All <code>ids</code> must be known.
     *
     * Note: Implementation must ensure thread-safeness
     */
    char **(*extract)(StringDictionary *self, const StringId *ids, size_t numIds);

    /**
     * Frees up memory allocated inside a function call via the allocator given in the constructor
     *
     * Note: Implementation must ensure thread-safeness
     */
    int (*free)(StringDictionary *self, void *ptr);

    /**
     * Reset internal statistic counters
     */
    int (*resetCounters)(StringDictionary *self);

    /**
     * Get internal statistic counters
     */
    int (*counters)(StringDictionary *self, StringHashCounters *counters);

    /**
     * Returns number of distinct strings stored in the dictionary
     */
    int (*numDistinct)(StringDictionary *self, size_t *num);
} StringDictionary;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 *
 * @param dic
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryDrop(StringDictionary *dic)
{
    CHECK_NON_NULL(dic);
    assert(dic->drop);
    return dic->drop(dic);
}

/**
 *
 * @param dic
 * @param out
 * @param num_out
 * @param strings
 * @param numStrings
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryInsert(StringDictionary *dic, StringId **out, char *const *strings, size_t numStrings,
                                  size_t numThreads)
{
    CHECK_NON_NULL(dic);
    CHECK_NON_NULL(strings);
    assert(dic->insert);
    return dic->insert(dic, out, strings, numStrings, numThreads);
}

NG5_LIB_FUNCTION
static int StringDictionaryResetCounters(StringDictionary *dic)
{
    CHECK_NON_NULL(dic);
    assert(dic->resetCounters);
    return dic->resetCounters(dic);
}

NG5_LIB_FUNCTION
static int StringDictionaryGetCounters(StringHashCounters *counters, StringDictionary *dic)
{
    CHECK_NON_NULL(dic);
    assert(dic->counters);
    return dic->counters(dic, counters);
}

/**
 *
 * @param dic
 * @param strings
 * @param numStrings
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryRemove(StringDictionary *dic, StringId *strings, size_t numStrings)
{
    CHECK_NON_NULL(dic);
    CHECK_NON_NULL(strings);
    assert(dic->remove);
    return dic->remove(dic, strings, numStrings);
}

/**
 *
 * @param out
 * @param foundMask
 * @param numNotFound
 * @param dic
 * @param keys
 * @param numKeys
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryLocateSafe(StringId **out, bool **foundMask, size_t *numNotFound,
                                      StringDictionary *dic, char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(foundMask);
    CHECK_NON_NULL(numNotFound);
    CHECK_NON_NULL(dic);
    CHECK_NON_NULL(keys);
    assert(dic->locateSafe);
    return dic->locateSafe(dic, out, foundMask, numNotFound, keys, numKeys);
}

/**
 *
 * @param out
 * @param dic
 * @param keys
 * @param numKeys
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryLocateFast(StringId **out, StringDictionary *dic, char *const *keys, size_t numKeys)
{
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(dic);
    CHECK_NON_NULL(keys);
    assert(dic->locateFast);
    return dic->locateFast(dic, out, keys, numKeys);
}

/**
 *
 * @param strings
 * @param num_out
 * @param dic
 * @param ids
 * @param numIds
 * @return
 */
NG5_LIB_FUNCTION
static char **StringDictionaryExtract(StringDictionary *dic, const StringId *ids, size_t numIds)
{
    assert(dic->extract);
    return dic->extract(dic, ids, numIds);
}

/**
 *
 * @param dic
 * @param ptr
 * @return
 */
NG5_LIB_FUNCTION
static int StringDictionaryFree(StringDictionary *dic, void *ptr)
{
    CHECK_NON_NULL(dic);
    CHECK_NON_NULL(ptr);
    assert(dic->free);
    return dic->free(dic, ptr);
}

NG5_LIB_FUNCTION
static int StringDictionaryNumDistinct(size_t *num, StringDictionary *dic)
{
    CHECK_NON_NULL(num);
    CHECK_NON_NULL(dic);
    assert(dic->numDistinct);
    return dic->numDistinct(dic, num);
}

NG5_END_DECL

#endif
