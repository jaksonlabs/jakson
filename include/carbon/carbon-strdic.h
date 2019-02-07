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

#ifndef CARBON_STRDIC_H
#define CARBON_STRDIC_H

#include "carbon-common.h"
#include "carbon-alloc.h"
#include "carbon-types.h"
#include "carbon-hash.h"

CARBON_BEGIN_DECL

CARBON_FORWARD_STRUCT_DECL(StringDictionary)
CARBON_FORWARD_STRUCT_DECL(Vector)

typedef struct carbon_string_hash_counters carbon_string_hash_counters_t;

typedef enum
{
    CARBON_STRDIC_TYPE_SYNC, CARBON_STRDIC_TYPE_ASYNC
} carbon_strdic_type_e;

/**
 * Thread-safe string pool implementation
 */
typedef struct carbon_strdic carbon_strdic_t;

typedef struct carbon_strdic
{
    /**
     * Implementation-specific fields
     */
    void *extra;

    /**
     * Tag determining the current implementation
     */
    carbon_strdic_type_e tag;

    /**
     * Memory allocator that is used to get memory for user data
     */
    carbon_alloc_t alloc;

    /**
     * Frees up implementation-specific resources.
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*drop)(carbon_strdic_t *self);

    /**
     * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
     *
     * Note: Implementation must ensure thread-safeness
    */
    bool (*insert)(carbon_strdic_t *self, carbon_string_id_t **out, char *const *strings,
                  size_t nstrings, size_t nthreads);

    /**
     * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
     * all string identifiers in <code>strings</code> are valid.
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*remove)(carbon_strdic_t *self, carbon_string_id_t *strings, size_t nstrings);

    /**
     * Get the string ids associated with <code>keys</code> in this map (if any).
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*locate_safe)(carbon_strdic_t *self, carbon_string_id_t **out, bool **found_mask,
                       size_t *num_not_found, char *const *keys, size_t num_keys);

    /**
     * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
     *
     * Note: Implementation must ensure thread-safeness
    */
    bool (*locate_fast)(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                       size_t num_keys);

    /**
     * Extracts strings given their string identifier. All <code>ids</code> must be known.
     *
     * Note: Implementation must ensure thread-safeness
     */
    char **(*extract)(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t num_ids);

    /**
     * Frees up memory allocated inside a function call via the allocator given in the constructor
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*free)(carbon_strdic_t *self, void *ptr);

    /**
     * Reset internal statistic counters
     */
    bool (*resetCounters)(carbon_strdic_t *self);

    /**
     * Get internal statistic counters
     */
    bool (*counters)(carbon_strdic_t *self, carbon_string_hash_counters_t *counters);

    /**
     * Returns number of distinct strings stored in the dictionary
     */
    bool (*num_distinct)(carbon_strdic_t *self, size_t *num);

    /**
     * Returns all contained (unique) strings and their mapped (unique) ids
     */
    bool (*get_contents)(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                       carbon_vec_t ofType(carbon_string_id_t) * string_ids);
} carbon_strdic_t;

/**
 *
 * @param dic
 * @return
 */
CARBON_FUNC_UNUSED
static bool
carbon_strdic_drop(carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    assert(dic->drop);
    return dic->drop(dic);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_insert(carbon_strdic_t *dic, carbon_string_id_t **out, char *const *strings, size_t nstrings,
                     size_t nthreads)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    CARBON_NON_NULL_OR_ERROR(strings);
    assert(dic->insert);
    return dic->insert(dic, out, strings, nstrings, nthreads);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_reset_counters(carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    assert(dic->resetCounters);
    return dic->resetCounters(dic);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_get_counters(carbon_string_hash_counters_t *counters, carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    assert(dic->counters);
    return dic->counters(dic, counters);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_remove(carbon_strdic_t *dic, carbon_string_id_t *strings, size_t numStrings)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    CARBON_NON_NULL_OR_ERROR(strings);
    assert(dic->remove);
    return dic->remove(dic, strings, numStrings);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_locate_safe(carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                          carbon_strdic_t *dic, char *const *keys, size_t num_keys)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(found_mask);
    CARBON_NON_NULL_OR_ERROR(num_not_found);
    CARBON_NON_NULL_OR_ERROR(dic);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(dic->locate_safe);
    return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_locate_fast(carbon_string_id_t **out, carbon_strdic_t *dic, char *const *keys, size_t nkeys)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(dic);
    CARBON_NON_NULL_OR_ERROR(keys);
    assert(dic->locate_fast);
    return dic->locate_fast(dic, out, keys, nkeys);
}

CARBON_FUNC_UNUSED
static char **
carbon_strdic_extract(carbon_strdic_t *dic, const carbon_string_id_t *ids, size_t nids)
{
    CARBON_NON_NULL_OR_ERROR(dic->extract);
    return dic->extract(dic, ids, nids);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_free(carbon_strdic_t *dic, void *ptr)
{
    CARBON_NON_NULL_OR_ERROR(dic);
    if (ptr) {
        assert(dic->free);
        return dic->free(dic, ptr);
    } else {
        return true;
    }
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_num_distinct(size_t *num, carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(num);
    CARBON_NON_NULL_OR_ERROR(dic);
    assert(dic->num_distinct);
    return dic->num_distinct(dic, num);
}

CARBON_FUNC_UNUSED
static bool
carbon_strdic_get_contents(carbon_vec_t ofType (char *) *strings,
                           carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts,
                           carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(strings)
    CARBON_NON_NULL_OR_ERROR(carbon_string_id_ts)
    CARBON_NON_NULL_OR_ERROR(dic);
    assert(dic->get_contents);
    return dic->get_contents(dic, strings, carbon_string_id_ts);
}

CARBON_END_DECL

#endif
