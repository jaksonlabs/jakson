// file: result_handle.h

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

#ifndef NG5_RESULT_HANDLE
#define NG5_RESULT_HANDLE

#include <ng5_common.h>
#include <stdx/ng5_slot_vector.h>
#include <stdx/ng5_vector.h>

enum result_handle_type {
    RESULT_HANDLE_TYPE_STORE_STRING_ID
};

struct result_handle
{
    struct storage_engine          *context;
    const struct ng5_vector of_type(T) *result;
    slot_vector_slot_t              id;
    enum result_handle_type         type;
};

int result_handle_create(struct result_handle *handle, struct storage_engine *context, slot_vector_slot_t id,
        const struct ng5_vector of_type(T) *result, enum result_handle_type type);

const void *result_handle_read(size_t *num_elements, const struct result_handle *handle);

int result_handle_drop(struct result_handle *handle);

enum result_handle_type result_handle_type(struct result_handle *handle);

#endif