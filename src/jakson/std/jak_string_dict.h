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

#include <jakson/stdinc.h>
#include <jakson/stdx/jak_alloc.h>
#include <jakson/types.h>
#include <jakson/std/jak_hash.h>
#include <jakson/std/jak_vector.h>

JAK_BEGIN_DECL

JAK_FORWARD_STRUCT_DECL(StringDictionary)
JAK_FORWARD_STRUCT_DECL(Vector)

typedef enum jak_str_dict_tag_e {
        JAK_SYNC, JAK_ASYNC
} jak_str_dict_tag_e;

/**
 * Thread-safe string pool implementation
 */
typedef struct jak_string_dict {
        /**
         * Implementation-specific fields
         */
        void *extra;

        /**
         * Tag determining the current implementation
         */
        jak_str_dict_tag_e tag;

        /**
         * Memory allocator that is used to get memory for user data
         */
        jak_allocator alloc;

        /**
         * Frees up implementation-specific resources.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*drop)(jak_string_dict *self);

        /**
         * Inserts a particular number of strings into this dictionary and returns associated string identifiers.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*insert)(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads);

        /**
         * Removes a particular number of strings from this dictionary by their ids. The caller must ensure that
         * all string identifiers in <code>strings</code> are valid.
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*remove)(jak_string_dict *self, jak_archive_field_sid_t *strings, size_t nstrings);

        /**
         * Get the string ids associated with <code>keys</code> in this jak_async_map_exec (if any).
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*locate_safe)(jak_string_dict *self, jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, char *const *keys, size_t num_keys);

        /**
         * Get the string ids associated with <code>keys</code> in this dic. All keys <u>must</u> exist.
         *
         * Note: Implementation must ensure thread-safeness
        */
        bool (*locate_fast)(jak_string_dict *self, jak_archive_field_sid_t **out, char *const *keys, size_t num_keys);

        /**
         * Extracts strings given their string identifier. All <code>ids</code> must be known.
         *
         * Note: Implementation must ensure thread-safeness
         */
        char **(*extract)(jak_string_dict *self, const jak_archive_field_sid_t *ids, size_t num_ids);

        /**
         * Frees up memory allocated inside a function call via the allocator given in the constructor
         *
         * Note: Implementation must ensure thread-safeness
         */
        bool (*free)(jak_string_dict *self, void *ptr);

        /**
         * Reset internal statistic counters
         */
        bool (*resetCounters)(jak_string_dict *self);

        /**
         * Get internal statistic counters
         */
        bool (*counters)(jak_string_dict *self, jak_str_hash_counters *counters);

        /**
         * Returns number of distinct strings stored in the dictionary
         */
        bool (*num_distinct)(jak_string_dict *self, size_t *num);

        /**
         * Returns all contained (unique) strings and their mapped (unique) ids
         */
        bool (*get_contents)(jak_string_dict *self, jak_vector ofType (char *) *strings, jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids);
} jak_string_dict;

/**
 *
 * @param dic
 * @return
 */
static JAK_BUILT_IN(bool) jak_string_dict_drop(jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->drop);
        return dic->drop(dic);
}

static JAK_BUILT_IN(bool)
jak_string_dict_insert(jak_string_dict *dic, jak_archive_field_sid_t **out, char *const *strings, size_t nstrings, size_t nthreads)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->insert);
        return dic->insert(dic, out, strings, nstrings, nthreads);
}

static JAK_BUILT_IN(bool)  jak_string_dict_reset_counters(jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->resetCounters);
        return dic->resetCounters(dic);
}

static JAK_BUILT_IN(bool)  jak_string_dict_get_counters(jak_str_hash_counters *counters, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->counters);
        return dic->counters(dic, counters);
}

static JAK_BUILT_IN(bool)  jak_string_dict_remove(jak_string_dict *dic, jak_archive_field_sid_t *strings, size_t num_strings)
{
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(strings);
        JAK_ASSERT(dic->remove);
        return dic->remove(dic, strings, num_strings);
}

static JAK_BUILT_IN(bool) jak_string_dict_locate_safe(jak_archive_field_sid_t **out, bool **found_mask, size_t *num_not_found, jak_string_dict *dic, char *const *keys, size_t num_keys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(found_mask);
        JAK_ERROR_IF_NULL(num_not_found);
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_safe);
        return dic->locate_safe(dic, out, found_mask, num_not_found, keys, num_keys);
}

static JAK_BUILT_IN(bool) jak_string_dict_locate_fast(jak_archive_field_sid_t **out, jak_string_dict *dic, char *const *keys, size_t nkeys)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(dic);
        JAK_ERROR_IF_NULL(keys);
        JAK_ASSERT(dic->locate_fast);
        return dic->locate_fast(dic, out, keys, nkeys);
}

static JAK_BUILT_IN(char **)jak_string_dict_extract(jak_string_dict *dic, const jak_archive_field_sid_t *ids, size_t nids)
{
        return dic->extract(dic, ids, nids);
}

static JAK_BUILT_IN(bool) jak_string_dict_free(jak_string_dict *dic, void *ptr)
{
        JAK_ERROR_IF_NULL(dic);
        if (ptr) {
                JAK_ASSERT(dic->free);
                return dic->free(dic, ptr);
        } else {
                return true;
        }
}

static JAK_BUILT_IN(bool) jak_string_dict_num_distinct(size_t *num, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(num);
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->num_distinct);
        return dic->num_distinct(dic, num);
}

static JAK_BUILT_IN(bool) jak_string_dict_get_contents(jak_vector ofType (char *) *strings, jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids, jak_string_dict *dic)
{
        JAK_ERROR_IF_NULL(strings)
        JAK_ERROR_IF_NULL(jak_string_ids)
        JAK_ERROR_IF_NULL(dic);
        JAK_ASSERT(dic->get_contents);
        return dic->get_contents(dic, strings, jak_string_ids);
}

JAK_END_DECL

#endif
