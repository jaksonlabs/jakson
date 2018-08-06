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

#ifndef NG5_SIMPLE_SCAN_SINGLETHREADED
#define NG5_SIMPLE_SCAN_SINGLETHREADED

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <common.h>
#include <stdx/string_id_map.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Setups a new <code>struct string_id_map</code> which is a specialized hash table mapping a string (c-strings)
 * to an id (64bit values). This data structure is <b>thread-safe</b>.
 *
 * <b>Notes regarding the ownership</b>: Putting string keys into this map follows a hollow-copy strategy, i.e.,
 * pointers to strings used for keys are <u>copied</u> into the map. Whatever is behind these pointers is
 * <u>not owned</u> by this map. Especially, memory is not freed for these elements when the map is dropped. Moreover,
 * the caller must ensure not to corrupt whatever is behind these pointers during the lifetime of the map.
 *
 * @param map a non-null pointer to the new map
 * @param alloc the allocator that should be used for memory management. If this parameter is set to NULL, the
 *              default clib allocator will be used
 * @param num_buckets The number of buckets used in the map. The bucket list is resized, if conditions
 *                    'bucket_count_min' and 'entry_overflow_min' are satisfied (see below)
 * @param cap_buckets The reserved number of elements inside a bucket that are handled for collision resolving
 * @param bucket_grow_factor A value greater 1 that is used to determine the next reserved amount of memory if the
 *                           entry list inside a single bucket has an not enough space to store additionally elements
 * @return <code>STATUS_OK</code> is call is successful, or another status value indicating the error.
 */
int string_id_map_create_scan_single_threaded(struct string_id_map *map, const struct allocator *alloc, size_t num_buckets,
        size_t cap_buckets, float bucket_grow_factor);

#endif
