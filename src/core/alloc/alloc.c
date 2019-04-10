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

#include <errno.h>
#include <assert.h>

#include "core/alloc/alloc.h"

static void *
invoke_malloc(struct allocator *self, size_t size);

static void *
invoke_realloc(struct allocator *self, void *ptr, size_t size);

static void
invoke_free(struct allocator *self, void *ptr);

static void
invoke_clone(struct allocator *dst, const struct allocator *self);


CARBON_EXPORT (bool)
carbon_alloc_create_std(struct allocator *alloc)
{
    if (alloc) {
        alloc->extra = NULL;
        alloc->malloc = invoke_malloc;
        alloc->realloc = invoke_realloc;
        alloc->free = invoke_free;
        alloc->clone = invoke_clone;
        carbon_error_init(&alloc->err);
        return true;
    }
    else {
        return false;
    }
}

CARBON_EXPORT (bool)
carbon_alloc_this_or_std(struct allocator *dst, const struct allocator *self)
{
    if (!self) {
        return carbon_alloc_create_std(dst);
    }
    else {
        return carbon_alloc_clone(dst, self);
    }
}

CARBON_EXPORT (void *)
carbon_malloc(struct allocator *alloc, size_t size)
{
    assert(alloc);
    return alloc->malloc(alloc, size);
}

CARBON_EXPORT (void *)
carbon_realloc(struct allocator *alloc, void *ptr, size_t size)
{
    return alloc->realloc(alloc, ptr, size);
}

CARBON_EXPORT (bool)
carbon_free(struct allocator *alloc, void *ptr)
{
    CARBON_NON_NULL_OR_ERROR(alloc);
    CARBON_NON_NULL_OR_ERROR(ptr);
    alloc->free(alloc, ptr);
    return true;
}

CARBON_EXPORT (bool)
carbon_alloc_clone(struct allocator *dst, const struct allocator *src)
{
    CARBON_NON_NULL_OR_ERROR(dst && src)
    src->clone(dst, src);
    return true;
}

static void *
invoke_malloc(struct allocator *self, size_t size)
{
    CARBON_UNUSED(self);
    void *result;

    errno = 0;
    if ((result = malloc(size)) == NULL) {
        carbon_print_error_and_die(CARBON_ERR_MALLOCERR)
    }
    else {
        return result;
    }
}

static void *
invoke_realloc(struct allocator *self, void *ptr, size_t size)
{
    CARBON_UNUSED(self);
    void *result;

    if ((result = realloc(ptr, size)) == NULL) {
        CARBON_PRINT_ERROR(CARBON_ERR_MALLOCERR)
        return ptr;
    }
    else {
        return result;
    }
}

static void
invoke_free(struct allocator *self, void *ptr)
{
    CARBON_UNUSED(self);
    return free(ptr);
}

static void
invoke_clone(struct allocator *dst, const struct allocator *self)
{
    *dst = *self;
}
