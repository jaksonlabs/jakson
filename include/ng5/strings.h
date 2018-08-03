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

#ifndef _NG5_STRINGS
#define _NG5_STRINGS

#include <common.h>
#include <stdx/hash.h>
#include "allocator.h"

typedef size_t string_id_t;

struct string;

enum string_pool_tag { SP_NAIVE };

/**
 * Thread-safe string pool implementation
 */
struct string_pool
{
    void                *extra;

    enum string_pool_tag tag;

    struct allocator     alloc;

    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                 (*drop)(struct string_pool *self);

    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                  (*insert)(struct string_pool *self, struct string *out, size_t *num_out,
                                   const char **strings, size_t num_strings);

    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                  (*remove)(struct string_pool *self, struct string *strings, size_t num_strings);

    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                  (*find_by_string)(struct string_pool *self, struct string *out, size_t *num_out,
                                           const char **strings, size_t num_strings);
    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                  (*find_by_id)(struct string_pool *self, const char **strings, size_t *num_out,
                                       const string_id_t *ids, size_t num_ids);

    /**
     *
     *
     * Note: Implementation must ensure thread-safeness
     *
     * @param self
     * @return
    */
    int                  (*resolve)(struct string_pool *self, char **out, size_t *num_out, struct string *strings,
                                    size_t num_strings);
};

struct string
{
    struct string_pool *context;
    string_id_t         id;
    size_t              len;
    hash_t              hash;
};

int string_pool_create_naive(struct string_pool *pool, size_t capacity, const struct allocator *alloc);

int string_pool_drop(struct string_pool *pool);

int string_pool_insert(struct string *out, size_t *num_out, struct string_pool *pool,
                       const char **strings, size_t num_strings);

int string_pool_remove(struct string_pool *pool, struct string *strings, size_t num_strings);

int string_pool_find_by_string(struct string *out, size_t *num_out, struct string_pool *pool,
                               const char **strings, size_t num_strings);

int string_pool_find_by_id(const char **strings, size_t *num_out, struct string_pool *pool,
                           const string_id_t *ids, size_t num_ids);


#endif
