// file: alloc.h

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

#ifndef NG5_ALLOC
#define NG5_ALLOC

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Allocates <code>num</code> elements of size <code>sizeof(type)</code> using the allocator <code>alloc</code> and
 * creates a new stack variable <code>type *name</code>.
 */
#define ALLOCATOR_MALLOC(type, name, num, alloc)                  \
    type *name = AllocatorMalloc(alloc, num *sizeof(type))

/**
 * Invokes a free operation in <code>alloc</code> allocator to free up memory assigned to pointer <code>name</code>
 */
#define ALLOCATOR_FREE(name, alloc)                              \
    AllocatorFree(alloc, name)

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Abstraction over memory allocation. Use 'allocator_default' for the standard c-lib allocator
 */
typedef struct Allocator Allocator;

typedef struct Allocator
{
    /**
     *  Implementation-specific data (private fields etc.)
     *  This pointer may point to NULL.
     */
    void *extra;

    /**
     *  Implementation to call memory allocation.
     */
    void *(*malloc)(Allocator *self, size_t size);

    /**
     *  Implementation to call memory re-allocation.
     */
    void *(*realloc)(Allocator *self, void *ptr, size_t size);

    /**
     *  Implementation to call freeing up memory.
     *  Depending on the strategy, freeing up memory might be lazy.
     */
    void (*free)(Allocator *self, void *ptr);

    /**
     *  Perform a deep copy of this allocator including implementation-specific data stored in 'extra'
     *
     * @param dst non-null target in which 'self' should be cloned
     * @param self non-null source which should be clones in 'dst'
     */
    void (*clone)(Allocator *dst, const Allocator *self);

} Allocator;

/**
 * Returns standard c-lib allocator (malloc, realloc, free)
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
int AllocatorCreateDefault(Allocator *alloc);

/**
 * Creates a new allocator 'dst' with default constructor (in case of 'this' is null), or as copy of
 * 'this' (in case 'this' is non-null)
 * @param dst non-null destination in which the allocator should be stored
 * @param self possibly null-pointer to an allocator implementation
 * @return a value unequal to STATUS_OK in case the operation is not successful
 */
int AllocatorThisOrDefault(Allocator *dst, const Allocator *self);

/**
 * Invokes memory allocation of 'size' bytes using the allocator 'alloc'.
 *
 * If allocation fails, the system may panic.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param size number of bytes requested
 * @return non-null pointer to memory allocated with 'alloc'
 */
void *AllocatorMalloc(Allocator *alloc, size_t size);

/**
 * Invokes memory re-allocation for pointer 'ptr' (that is managed by 'alloc') to size 'size' in bytes.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @param size new number of bytes for 'ptr'
 * @return non-null pointer that points to reallocated memory for 'ptr'
 */
void *AllocatorRealloc(Allocator *alloc, void *ptr, size_t size);

/**
 * Invokes memory freeing for pointer 'ptr' (that is managed by 'alloc').
 * Depending on the implementation, this operation might be lazy.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @return STATUS_OK if success, STATUS_NULLPTR if <code>alloc</code> or <code>ptr</ptr> is <b>NULL</b>
 */
int AllocatorFree(Allocator *alloc, void *ptr);

/**
 * Performs a deep copy of the allocator 'src' into the allocator 'dst'.
 *
 * @param dst non-null pointer to allocator implementation (of same implementation as src)
 * @param src non-null pointer to allocator implementation (of same implementation as dst)
 * @return STATUS_OK in case of success, otherwise a value unequal to STATUS_OK describing the error
 */
int AllocatorClone(Allocator *dst, const Allocator *src);

NG5_END_DECL

#endif