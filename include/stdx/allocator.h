// file: allocator.h

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

#ifndef _NG5_ALLOCATOR
#define _NG5_ALLOCATOR

#include <common.h>

/**
 * Abstraction over memory allocation. Use 'allocator_default' for the standard c-lib allocator
 */
struct allocator {
    /**
     *  Implementation-specific data (private fields etc.)
     *  This pointer may point to NULL.
     */
    void             *extra;

    /**
     *  Implementation to call memory allocation.
     */
    void             *(*malloc)(struct allocator *self, size_t size);

    /**
     *  Implementation to call memory re-allocation.
     */
    void             *(*realloc)(struct allocator *self, void *ptr, size_t size);

    /**
     *  Implementation to call freeing up memory.
     *  Depending on the strategy, freeing up memory might be lazy.
     */
    void              (*free)(struct allocator *self, void *ptr);

    /**
     *  Implementation to call garbage collection (if implemented).
     *  Setting this function pointer to NULL indicates that garbage collection is not supported.
     */
    void              (*gc)(struct allocator *self);

    /**
     *  Perform a deep copy of this allocator including implementation-specific data stored in 'extra'
     *
     * @param dst non-null target in which 'self' should be cloned
     * @param self non-null source which should be clones in 'dst'
     */
    void               (*clone)(struct allocator *dst, const struct allocator *self);

    /**
     *  Deletes this allocator by freeing up implementation-specific data that might be stored in 'extra'.
     *  Setting this function pointer to NULL indicates that no futher cleanup is required
     *  (i.e., extra must not be freed)
     *
     * @param self non-null ponter to this allocator
     */
    void              (*drop)(struct allocator *self);
};

/**
 * Returns standard c-lib allocator (malloc, realloc, free)
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
int allocator_default(struct allocator *alloc);

/**
 * Creates a new allocator 'dst' with default constructor (in case of 'this' is null), or as copy of
 * 'this' (in case 'this' is non-null)
 * @param dst non-null destination in which the allocator should be stored
 * @param this possibly null-pointer to an allocator implementation
 * @return a value unequal to STATUS_OK in case the operation is not successful
 */
int allocator_this_or_default(struct allocator *dst, const struct allocator *this);

/**
 * Invokes memory allocation of 'size' bytes using the allocator 'alloc'.
 *
 * If allocation fails, the system may panic.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param size number of bytes requested
 * @return non-null pointer to memory allocated with 'alloc'
 */
void *allocator_malloc(struct allocator *alloc, size_t size);

/**
 * Invokes memory re-allocation for pointer 'ptr' (that is managed by 'alloc') to size 'size' in bytes.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @param size new number of bytes for 'ptr'
 * @return non-null pointer that points to reallocated memory for 'ptr'
 */
void *allocator_realloc(struct allocator *alloc, void *ptr, size_t size);

/**
 * Invokes memory freeing for pointer 'ptr' (that is managed by 'alloc').
 * Depending on the implementation, this operation might be lazy.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @return STATUS_OK if success, STATUS_NULLPTR if <code>alloc</code> or <code>ptr</ptr> is <b>NULL</b>
 */
int  allocator_free(struct allocator *alloc, void *ptr);

/**
 * Invokes a garbage collection for memory manages with 'alloc' (if supported).
 * Depending on the implementation, a call to this function may also frees up memory previously
 * marked as to-be-freed via a call to 'allocator_realloc' (if the allocator implements a lazy strategy)
 *
 * @param alloc non-null pointer to allocator implementation
 */
void  allocator_gc(struct allocator *alloc);

/**
 * Performs a deep copy of the allocator 'src' into the allocator 'dst'.
 *
 * @param dst non-null pointer to allocator implementation (of same implementation as src)
 * @param src non-null pointer to allocator implementation (of same implementation as dst)
 * @return STATUS_OK in case of success, otherwise a value unequal to STATUS_OK describing the error
 */
int allocator_clone(struct allocator *dst, const struct allocator *src);

/**
 *  Deletes this allocator by freeing up implementation-specific data.
 *
 * @param alloc non-null pointer to allocator implementation
 * @return STATUS_OK in case of success, otherwise a value unequal to STATUS_OK describing the error
 */
int allocator_drop(struct allocator *alloc);

#endif