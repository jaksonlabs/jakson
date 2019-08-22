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

#ifndef JAK_STRDIC_H
#define JAK_STRDIC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <assert.h>

#include <jak_stdinc.h>
#include <jak_alloc.h>
#include <jak_types.h>
#include <jak_hash.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

JAK_FORWARD_STRUCT_DECL(StringDictionary)
JAK_FORWARD_STRUCT_DECL(Vector)

struct jak_str_hash_counters;

enum jak_str_dict_tag {
    SYNC, ASYNC
};

/**
 * Thread-safe string pool implementation
 */
struct jak_string_dict {
    /**
     * Implementation-specific fields
     */
    void *extra;

    /**
     * Tag determining the current implementation
     */
    enum jak_str_dict_tag tag;

    /**
     * Memory allocator that is used to get memory for user data
     */
    struct jak_allocator alloc;

    /**
     * Frees up implementation-specific resources.
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*drop)(struct jak_string_dict *self);

    /**
     * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
     *
     * Note: Implementation must ensure thread-safeness
    */
    bool (*insert)(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads);

    /**
     * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
     * all string identifiers in <code>strings</code> are valid.
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*remove)(struct jak_string_dict *self, jak_archive_field_sid_t *strings, size_t nstrings);

    /**
     * Get the string ids associated with <code>keys</code> in this parallel_map_exec (if any).
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*locate_safe)(struct jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found,
                        char *const *keys, size_t num_keys);

    /**
     * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
     *
     * Note: Implementation must ensure thread-safeness
    */
    bool (*locate_fast)(struct jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys);

    /**
     * Extracts strings given their string identifier. All <code>ids</code> must be known.
     *
     * Note: Implementation must ensure thread-safeness
     */
    char **(*extract)(struct jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids);

    /**
     * Frees up memory allocated inside a function call via the allocator given in the constructor
     *
     * Note: Implementation must ensure thread-safeness
     */
    bool (*free)(struct jak_string_dict *self, void *ptr);

    /**
     * Reset internal statistic counters
     */
    bool (*resetCounters)(struct jak_string_dict *self);

    /**
     * Get internal statistic counters
     */
    bool (*counters)(struct jak_string_dict *self, struct jak_str_hash_counters *counters);

    /**
     * Returns number of distinct strings stored in the dictionary
     */
    bool (*num_distinct)(struct jak_string_dict *self, size_t *num);

    /**
     * Returns all contained (unique) strings and their mapped (unique) ids
     */
    bool (*get_contents)(struct jak_string_dict *self, struct vector ofType (char *) *strings,
                         struct vector ofType(jak_archive_field_sid_t) *string_ids);
};

/**
 *
 * @param dic
 * @return
 */
JAK_FUNC_UNUSED
static bool strdic_drop(struct jak_string_dict *dic)
{
        error_if_null(dic);
        assert(dic->drop);
        return dic->drop(dic);
}

JAK_FUNC_UNUSED
static bool
strdic_insert(struct jak_string_dict *dic, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads)
{
        error_if_null(dic);
        error_if_null(strings);
        assert(dic->insert);
        return dic->insert(dic, out, strings, nstrings, nthreads);
}

JAK_FUNC_UNUSED
static bool strdic_reset_counters(struct jak_string_dict *dic)
{
        error_if_null(dic);
        assert(dic->resetCounters);
        return dic->resetCounters(dic);
}

JAK_FUNC_UNUSED
static bool strdic_get_counters(struct jak_str_hash_counters *counters, struct jak_string_dict *dic)
{
        error_if_null(dic);
        assert(dic->counters);
        return dic->counters(dic, counters);
}

JAK_FUNC_UNUSED
static bool strdic_remove(struct jak_string_dict *dic, jak_archive_field_sid_t *strings, size_t num_strings)
{
        error_if_null(dic);
        error_if_null(strings);
        assert(dic->remove);
        return dic->remove(dic, strings, num_strings);
}

JAK_FUNC_UNUSED
static bool strdic_locate_safe(jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, struct jak_string_dict *dic,
                               char *const *keys, size_t num_keys)
{
        error_if_null(out);
        error_if_null(found_mask);
        error_if_null(num_not_found);
        error_if_null(dic);
        error_if_null(keys);
        assert(dic->locate_safe);
        return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

JAK_FUNC_UNUSED
static bool strdic_locate_fast(jak_archive_field_sid_t **out, struct jak_string_dict *dic, char *const *keys, size_t nkeys)
{
        error_if_null(out);
        error_if_null(dic);
        error_if_null(keys);
        assert(dic->locate_fast);
        return dic->locate_fast(dic, out, keys, nkeys);
}

JAK_FUNC_UNUSED
static char **strdic_extract(struct jak_string_dict *dic, const jak_archive_field_sid_t *ids, size_t nids)
{
        return dic->extract(dic, ids, nids);
}

JAK_FUNC_UNUSED
static bool strdic_free(struct jak_string_dict *dic, void *ptr)
{
        error_if_null(dic);
        if (ptr) {
                assert(dic->free);
                return dic->free(dic, ptr);
        } else {
                return true;
        }
}

JAK_FUNC_UNUSED
static bool strdic_num_distinct(size_t *num, struct jak_string_dict *dic)
{
        error_if_null(num);
        error_if_null(dic);
        assert(dic->num_distinct);
        return dic->num_distinct(dic, num);
}

JAK_FUNC_UNUSED
static bool strdic_get_contents(struct vector ofType (char *) *strings, struct vector ofType(jak_archive_field_sid_t) *string_ids,
                                struct jak_string_dict *dic)
{
        error_if_null(strings)
        error_if_null(string_ids)
        error_if_null(dic);
        assert(dic->get_contents);
        return dic->get_contents(dic, strings, string_ids);
}

JAK_END_DECL

#endif
