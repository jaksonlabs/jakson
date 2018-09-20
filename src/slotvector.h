// file: slotvector.h

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

#ifndef NG5_SLOTVECTOR
#define NG5_SLOTVECTOR

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "vector.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef size_t slot_vector_slot_t;

struct slot_vector
{
    Vector ofType(T) content;
    Vector ofType(slot_vector_slot_t) freelist;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int slot_vector_create(struct slot_vector *vector, const Allocator *alloc, size_t elem_size, size_t cap_elems);

int slot_vector_set_growfactor(struct slot_vector *vec, float factor);

int slot_vector_drop(struct slot_vector *vec);

int slot_vector_is_empty(struct slot_vector *vec);

int slot_vector_insert(struct slot_vector *vec, optional Vector ofType(slot_vector_slot_t) *ids,
                       const void *data, size_t num_elems);

const void *slot_vector_at(struct slot_vector *vec, slot_vector_slot_t slot);

int slot_vector_remove(struct slot_vector *vec, slot_vector_slot_t slot);

size_t slot_vector_len(const struct slot_vector *vec);

size_t slot_vector_cap(const struct slot_vector *vec);

NG5_END_DECL

#endif