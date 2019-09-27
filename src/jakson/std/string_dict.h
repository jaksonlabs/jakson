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

#ifndef STRDIC_H
#define STRDIC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/stdx/alloc.h>
#include <jakson/types.h>
#include <jakson/std/hash.h>
#include <jakson/std/vector.h>

BEGIN_DECL

FORWARD_STRUCT_DECL(StringDictionary)
FORWARD_STRUCT_DECL(Vector)

typedef enum str_dict_tag_e {
        SYNC, ASYNC
} str_dict_tag_e;

/**
 * Thread-safe string pool implementation
 */
typedef struct string_dict {
        /**
         * Implementation-specific fields
         */
        void *extra;

        /**
         * Tag determining the current implementation
         */
        str_dict_tag_e tag;

        /**
         * Memory allocator that is used to get memory for user data
         */
        allocator alloc;

        /**
         * Frees up implementation-specific resources.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*drop)(string_dict *self);

        /**
         * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*insert)(string_dict *self, archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads);

        /**
         * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
         * all string identifiers in <code>strings</code> are valid.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*remove)(string_dict *self, archive_field_sid_t *strings, size_t nstrings);

        /**
         * Get the string ids associated with <code>keys</code> in this async_map_exec (if any).
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*locate_safe)(string_dict *self, archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, char *const *keys, size_t num_keys);

        /**
         * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*locate_fast)(string_dict *self, archive_field_sid_t **out, char *const *keys, size_t num_keys);

        /**
         * Extracts strings given their string identifier. All <code>ids</code> must be known.
         *
         * Note: Implementation must ensure thread-safeness
         */
        char **(*extract)(string_dict *self, const archive_field_sid_t *ids, size_t num_ids);

        /**
         * Frees up memory allocated inside a function call via the allocator given in the constructor
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*free)(string_dict *self, void *ptr);

        /**
         * Reset internal statistic counters
         */
        bool (*resetCounters)(string_dict *self);

        /**
         * Get internal statistic counters
         */
        bool (*counters)(string_dict *self, str_hash_counters *counters);

        /**
         * Returns number of distinct strings stored in the dictionary
         */
        bool (*num_distinct)(string_dict *self, size_t *num);

        /**
         * Returns all contained (unique) strings and their mapped (unique) ids
         */
        bool (*get_contents)(string_dict *self, vector ofType (char *) *strings, vector ofType(archive_field_sid_t) *string_ids);
} string_dict;

/**
 *
 * @param dic
 * @return
 */
static BUILT_IN(bool) string_dict_drop(string_dict *dic)
{
        ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->drop);
        return dic->drop(dic);
}

static BUILT_IN(bool)
string_dict_insert(string_dict *dic, archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads)
{
        ERROR_IF_NULL(dic);
        ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->insert);
        return dic->insert(dic, out, strings, nstrings, nthreads);
}

static BUILT_IN(bool)  string_dict_reset_counters(string_dict *dic)
{
        ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->resetCounters);
        return dic->resetCounters(dic);
}

static BUILT_IN(bool)  string_dict_get_counters(str_hash_counters *counters, string_dict *dic)
{
        ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->counters);
        return dic->counters(dic, counters);
}

static BUILT_IN(bool)  string_dict_remove(string_dict *dic, archive_field_sid_t *strings, size_t num_strings)
{
        ERROR_IF_NULL(dic);
        ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->remove);
        return dic->remove(dic, strings, num_strings);
}

static BUILT_IN(bool) string_dict_locate_safe(archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, string_dict *dic, char *const *keys, size_t num_keys)
{
        ERROR_IF_NULL(out);
        ERROR_IF_NULL(found_mask);
        ERROR_IF_NULL(num_not_found);
        ERROR_IF_NULL(dic);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_safe);
        return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

static BUILT_IN(bool) string_dict_locate_fast(archive_field_sid_t **out, string_dict *dic, char *const *keys, size_t nkeys)
{
        ERROR_IF_NULL(out);
        ERROR_IF_NULL(dic);
        ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_fast);
        return dic->locate_fast(dic, out, keys, nkeys);
}

static BUILT_IN(char **)string_dict_extract(string_dict *dic, const archive_field_sid_t *ids, size_t nids)
{
        return dic->extract(dic, ids, nids);
}

static BUILT_IN(bool) string_dict_free(string_dict *dic, void *ptr)
{
        ERROR_IF_NULL(dic);
        if (ptr) {
                JAK_ASSERT(dic->free);
                return dic->free(dic, ptr);
        } else {
                return true;
        }
}

static BUILT_IN(bool) string_dict_num_distinct(size_t *num, string_dict *dic)
{
        ERROR_IF_NULL(num);
        ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->num_distinct);
        return dic->num_distinct(dic, num);
}

static BUILT_IN(bool) string_dict_get_contents(vector ofType (char *) *strings, vector ofType(archive_field_sid_t) *string_ids, string_dict *dic)
{
        ERROR_IF_NULL(strings)
        ERROR_IF_NULL(string_ids)
        ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->get_contents);
        return dic->get_contents(dic, strings, string_ids);
}

END_DECL

#endif
