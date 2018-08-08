// file: strings.h

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

#ifndef _NG5_STRING_DIC
#define _NG5_STRING_DIC

#include <common.h>
#include <stdx/hash.h>
#include "allocator.h"

enum string_dic_tag { STRING_DIC_NAIVE };

/**
 * Thread-safe string pool implementation
 */
struct string_dic
{
    /**
     * Implementation-specific fields
     */
    void                *extra;

    /**
     * Tag determining the current implementation
     */
    enum string_dic_tag tag;

    /**
     * Memory allocator that is used to get memory for user data
     */
    struct allocator    alloc;

    /**
     * Frees up implementation-specific resources.
     *
     * Note: Implementation must ensure thread-safeness
     */
    int                 (*drop)(struct string_dic *self);

    /**
     * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
     *
     * Note: Implementation must ensure thread-safeness
    */
    int                  (*insert)(struct string_dic *self, string_id_t **out, char * const*strings, size_t num_strings);

    /**
     * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
     * all string identifiers in <code>strings</code> are valid.
     *
     * Note: Implementation must ensure thread-safeness
     */
    int                  (*remove)(struct string_dic *self, string_id_t *strings, size_t num_strings);

    /**
     * Get the string ids associated with <code>keys</code> in this map (if any).
     *
     * Note: Implementation must ensure thread-safeness
     */
    int                  (*locate_safe)(struct string_dic *self, string_id_t **out, bool **found_mask,
                                        size_t *num_not_found, char *const *keys, size_t num_keys);

    /**
     * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
     *
     * Note: Implementation must ensure thread-safeness
    */
    int                  (*locate_fast)(struct string_dic *self, string_id_t **out, char *const *keys,
                                         size_t num_keys);

    /**
     * Extracts strings given their string identifier. All <code>ids</code> must be known.
     *
     * Note: Implementation must ensure thread-safeness
     */
    char **             (*extract)(struct string_dic *self, const string_id_t *ids, size_t num_ids);

    /**
     * Frees up memory allocated inside a function call via the allocator given in the constructor
     *
     * Note: Implementation must ensure thread-safeness
     */
    int                  (*free)(struct string_dic *self, void *ptr);
};

/**
 *
 * @param dic
 * @return
 */
unused_fn
static int string_dic_drop(struct string_dic* dic)
{
    check_non_null(dic);
    assert(dic->drop);
    return dic->drop(dic);
}

/**
 *
 * @param dic
 * @param out
 * @param num_out
 * @param strings
 * @param num_strings
 * @return
 */
unused_fn
static int string_dic_insert(struct string_dic* dic, string_id_t** out, char* const* strings, size_t num_strings)
{
    check_non_null(dic);
    check_non_null(strings);
    assert(dic->insert);
    return dic->insert(dic, out, strings, num_strings);
}

/**
 *
 * @param dic
 * @param strings
 * @param num_strings
 * @return
 */
unused_fn
static int string_dic_remove(struct string_dic* dic, string_id_t* strings, size_t num_strings)
{
    check_non_null(dic);
    check_non_null(strings);
    assert(dic->remove);
    return dic->remove(dic, strings, num_strings);
}

/**
 *
 * @param out
 * @param found_mask
 * @param num_not_found
 * @param dic
 * @param keys
 * @param num_keys
 * @return
 */
unused_fn
static int string_dic_locate_safe(string_id_t** out, bool** found_mask, size_t* num_not_found,
        struct string_dic* dic, char* const* keys, size_t num_keys)
{
    check_non_null(out);
    check_non_null(found_mask);
    check_non_null(num_not_found);
    check_non_null(dic);
    check_non_null(keys);
    assert(dic->locate_safe);
    return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

/**
 *
 * @param out
 * @param dic
 * @param keys
 * @param num_keys
 * @return
 */
unused_fn
static int string_dic_locate_fast(string_id_t** out, struct string_dic* dic, char* const* keys, size_t num_keys)
{
    check_non_null(out);
    check_non_null(dic);
    check_non_null(keys);
    assert(dic->locate_fast);
    return dic->locate_fast(dic, out, keys, num_keys);
}

/**
 *
 * @param strings
 * @param num_out
 * @param dic
 * @param ids
 * @param num_ids
 * @return
 */
unused_fn
static char**string_dic_extract(struct string_dic* dic, const string_id_t* ids, size_t num_ids)
{
    check_non_null(strings);
    check_non_null(num_out);
    check_non_null(dic);
    check_non_null(ids);
    assert(dic->extract);
    return dic->extract(dic, ids, num_ids);
}

/**
 *
 * @param dic
 * @param ptr
 * @return
 */
unused_fn
static int string_dic_free(struct string_dic* dic, void* ptr)
{
    check_non_null(dic);
    check_non_null(ptr);
    assert(dic->free);
    return dic->free(dic, ptr);
}

#endif
