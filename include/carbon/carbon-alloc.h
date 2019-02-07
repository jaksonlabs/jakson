/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CARBON_ALLOC_H
#define CARBON_ALLOC_H

#include "carbon-common.h"
#include "carbon-error.h"

CARBON_BEGIN_DECL

/**
 * Allocates <code>num</code> elements of size <code>sizeof(type)</code> using the allocator <code>alloc</code> and
 * creates a new stack variable <code>type *name</code>.
 */
#define CARBON_MALLOC(type, name, num, alloc)                                                                          \
    type *name = carbon_malloc(alloc, num *sizeof(type))

/**
 * Invokes a free operation in <code>alloc</code> allocator to free up memory assigned to pointer <code>name</code>
 */
#define CARBON_FREE(name, alloc)                                                                                       \
    carbon_free(alloc, name)

typedef struct carbon_alloc
{
    /**
     *  Implementation-specific data (private fields etc.)
     *  This pointer may point to NULL.
     */
    void *extra;

    /**
     *  Error information
     */
    carbon_err_t err;

    /**
     *  Implementation to call memory allocation.
     */
    void *(*malloc)(struct carbon_alloc *self, size_t size);

    /**
     *  Implementation to call memory re-allocation.
     */
    void *(*realloc)(struct carbon_alloc *self, void *ptr, size_t size);

    /**
     *  Implementation to call freeing up memory.
     *  Depending on the strategy, freeing up memory might be lazy.
     */
    void (*free)(struct carbon_alloc *self, void *ptr);

    /**
     *  Perform a deep copy of this allocator including implementation-specific data stored in 'extra'
     *
     * @param dst non-null target in which 'self' should be cloned
     * @param self non-null source which should be clones in 'dst'
     */
    void (*clone)(struct carbon_alloc *dst, const struct carbon_alloc *self);
} carbon_alloc_t;

/**
 * Returns standard c-lib allocator (malloc, realloc, free)
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
CARBON_EXPORT (bool)
carbon_alloc_create_std(carbon_alloc_t *alloc);

/**
 * Creates a new allocator 'dst' with default constructor (in case of 'this' is null), or as copy of
 * 'this' (in case 'this' is non-null)
 * @param dst non-null destination in which the allocator should be stored
 * @param self possibly null-pointer to an allocator implementation
 * @return a value unequal to STATUS_OK in case the operation is not successful
 */
CARBON_EXPORT (bool)
carbon_alloc_this_or_std(carbon_alloc_t *dst, const carbon_alloc_t *self);

/**
 * Performs a deep copy of the allocator 'src' into the allocator 'dst'.
 *
 * @param dst non-null pointer to allocator implementation (of same implementation as src)
 * @param src non-null pointer to allocator implementation (of same implementation as dst)
 * @return STATUS_OK in case of success, otherwise a value unequal to STATUS_OK describing the error
 */
CARBON_EXPORT (bool)
carbon_alloc_clone(carbon_alloc_t *dst, const carbon_alloc_t *src);

/**
 * Invokes memory allocation of 'size' bytes using the allocator 'alloc'.
 *
 * If allocation fails, the system may panic.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param size number of bytes requested
 * @return non-null pointer to memory allocated with 'alloc'
 */
CARBON_EXPORT (void *)
carbon_malloc(carbon_alloc_t *alloc, size_t size);

/**
 * Invokes memory re-allocation for pointer 'ptr' (that is managed by 'alloc') to size 'size' in bytes.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @param size new number of bytes for 'ptr'
 * @return non-null pointer that points to reallocated memory for 'ptr'
 */
CARBON_EXPORT (void *)
carbon_realloc(carbon_alloc_t *alloc, void *ptr, size_t size);

/**
 * Invokes memory freeing for pointer 'ptr' (that is managed by 'alloc').
 * Depending on the implementation, this operation might be lazy.
 *
 * @param alloc non-null pointer to allocator implementation
 * @param ptr non-null pointer manged by 'alloc'
 * @return STATUS_OK if success, STATUS_NULLPTR if <code>alloc</code> or <code>ptr</ptr> is <b>NULL</b>
 */
CARBON_EXPORT (bool)
carbon_free(carbon_alloc_t *alloc, void *ptr);

CARBON_END_DECL

#endif
