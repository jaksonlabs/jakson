// file: simple_bsearch.h

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

#ifndef NG5_SIMPLE_SCAN4_CACHE
#define NG5_SIMPLE_SCAN4_CACHE


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <ng5_common.h>
#include <stdx/ng5_string_map.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Branch-free scan + swapping + caching
 */
int string_hashtable_create_scan4_cache(struct string_map* map, const ng5_allocator_t* alloc, size_t num_buckets,
        size_t cap_buckets, float bucket_grow_factor);

#endif
