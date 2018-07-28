// file: hash_table.h

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

#ifndef _NG5_VECTOR
#define _NG5_VECTOR

#include <common.h>
#include <ng5/allocator.h>

struct hash_table
{
  /**
  *  Memory allocator that is used to get memory for user data
  */
  struct allocator    allocator;

};

int hash_table_create(struct hash_table *out, const struct allocator *alloc, size_t key_size, size_t elem_size,
                      size_t num_buckets, size_t cap_buckets);

int hash_table_drop(struct hash_table *out);

int hash_table_put(struct hash_table *table, const char *keys, const char *values, size_t num_pairs);

int hash_table_get(struct hash_table *table, const char *keys)


#endif