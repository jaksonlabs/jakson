// file: slab.h

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

#ifndef _NG5_SLOT_MAP
#define _NG5_SLOT_MAP

#include <stdx/vector.h>
#include <stdx/time.h>

typedef struct slot_map_bucket {
    const struct slot_map     *context;
    const void                *data_ptr;
    const timestamp_t          version;
} slot_handle_t;

struct slot_map {
    struct allocator     alloc;
    struct vector        data;
    struct vector        bucket_free_list;
    struct vector        buckets;
};

enum status slot_map_create(struct slot_map *map, const struct allocator *alloc, size_t elem_size, size_t capacity);

enum status slot_map_drop(struct slot_map *map);

enum status slot_map_insert(slot_handle_t *handles, const void *data, size_t num_data);

enum status slot_map_update(slot_handle_t *handles, const void *data, size_t num_objects);

enum status slot_map_remove(slot_handle_t *handles, size_t num_handles);

enum status slot_handle_drop(slot_handle_t *handles, size_t num_handles);

#endif
