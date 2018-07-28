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

struct string_pool
{
    void                *extra;

    struct allocator     alloc;

    enum status          (*create)(struct string_pool *self);

    enum status          (*drop)(struct string_pool *self);

    enum status          (*insert)(struct string *out, size_t *num_out,
                                   struct string_pool *self, const char **strings, size_t num_strings);

    enum status          (*remove)(struct string_pool *self, struct string *strings, size_t num_strings);

    enum status          (*find_by_string)(struct string *out, size_t *num_out, struct string_pool *self,
                                           const char **strings, size_t num_strings);

    enum status          (*find_by_id)(struct string *out, size_t *num_out, struct string_pool *self,
                                       const char **strings, size_t num_strings);

    enum status          (*resolve)(char **out, size_t *num_out, struct string_pool *self, struct string *strings,
                                    size_t num_strings);
};

struct string
{
    struct string_pool *context;
    string_id_t         id;
    size_t              len;
    hash_t              hash;
};


#endif
