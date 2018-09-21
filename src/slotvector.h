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

typedef size_t SlotVectorSlot;

typedef struct SlotVector
{
    Vector ofType(T) content;
    Vector ofType(slot_vector_slot_t) freeList;
} SlotVector;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int SlotVectorCreate(SlotVector *vector, const Allocator *alloc, size_t elemSize, size_t capElems);

int SlotVectorSetGrowFactor(SlotVector *vec, float factor);

int SlotVectorDrop(SlotVector *vec);

int SlotVectorIsEmpty(SlotVector *vec);

int SlotVectorInsert(SlotVector *vec, optional Vector ofType(SlotVectorSlot) *ids,
                     const void *data, size_t numElems);

const void *SlotVectorAt(SlotVector *vec, SlotVectorSlot slot);

int SlotVectorRemove(SlotVector *vec, SlotVectorSlot slot);

size_t SlotVectorLength(const SlotVector *vec);

size_t SlotVectorCapacity(const SlotVector *vec);

NG5_END_DECL

#endif