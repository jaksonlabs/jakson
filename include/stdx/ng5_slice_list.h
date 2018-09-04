// file: slice_list.h

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

#ifndef NG5_SLICE_LIST
#define NG5_SLICE_LIST

#include <ng5_common.h>
#include <stdx/ng5_vector.h>
#include "ng5_bitset.h"
#include "ng5_spinlock.h"

typedef struct ng5_slice_t ng5_slice_t;

typedef uint32_t (*ng5_slice_find_func_t)(ng5_slice_t *slice, const void *neelde);

typedef struct ng5_slice_t
{
    ng5_slice_t            *next;
    ng5_slice_find_func_t   find;

    spinlock_t              spinlock;

    void                   *base;
    uint32_t               *indices;
    ng5_bitset_t            in_use_flags;
} ng5_slice_t;

typedef struct ng5_slice_list_t
{
    size_t                  slice_cap;
    size_t                  slice_elem_size;
    ng5_allocator_t         alloc;
    ng5_slice_t            *head,
                           *tail,
                           *appender;
    ng5_bitset_t            slice_freelist;
} ng5_slice_list_t;

typedef struct ng5_slice_handle_t
{
    ng5_slice_t            *container;
    const void             *element;
} ng5_slice_handle_t;

int ng5_slice_list_create(ng5_slice_t *list, ng5_allocator_t alloc, size_t elem_size, size_t slice_cap);

int ng5_slice_list_drop(ng5_slice_t *list);

ng5_slice_handle_t *ng5_slice_list_lookup(ng5_slice_t *list, const void *needles, size_t nneedles);

int ng5_slice_list_insert(ng5_slice_t *list, const void *elems, size_t neleme);

#endif