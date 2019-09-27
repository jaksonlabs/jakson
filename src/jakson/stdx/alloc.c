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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdx/alloc.h>

static void *_alloc_invoke_malloc(allocator *self, size_t size);

static void *_alloc_invoke_realloc(allocator *self, void *ptr, size_t size);

static void _alloc_invoke_free(allocator *self, void *ptr);

static void _alloc_invoke_clone(allocator *dst, const allocator *self);

bool alloc_create_std(allocator *alloc)
{
        if (alloc) {
                alloc->extra = NULL;
                alloc->malloc = _alloc_invoke_malloc;
                alloc->realloc = _alloc_invoke_realloc;
                alloc->free = _alloc_invoke_free;
                alloc->clone = _alloc_invoke_clone;
                error_init(&alloc->err);
                return true;
        } else {
                return false;
        }
}

bool alloc_this_or_std(allocator *dst, const allocator *self)
{
        if (!self) {
                return alloc_create_std(dst);
        } else {
                return alloc_clone(dst, self);
        }
}

void *alloc_malloc(allocator *alloc, size_t size)
{
        JAK_ASSERT(alloc);
        return alloc->malloc(alloc, size);
}

void *alloc_realloc(allocator *alloc, void *ptr, size_t size)
{
        return alloc->realloc(alloc, ptr, size);
}

bool alloc_free(allocator *alloc, void *ptr)
{
        ERROR_IF_NULL(alloc);
        ERROR_IF_NULL(ptr);
        alloc->free(alloc, ptr);
        return true;
}

bool alloc_clone(allocator *dst, const allocator *src)
{
        ERROR_IF_NULL(dst && src)
        src->clone(dst, src);
        return true;
}

static void *_alloc_invoke_malloc(allocator *self, size_t size)
{
        UNUSED(self);
        void *result;

        errno = 0;
        if ((result = malloc(size)) == NULL) {
                ERROR_PRINT_AND_DIE(ERR_MALLOCERR)
        } else {
                return result;
        }
}

static void *_alloc_invoke_realloc(allocator *self, void *ptr, size_t size)
{
        UNUSED(self);
        void *result;

        if ((result = realloc(ptr, size)) == NULL) {
                ERROR_PRINT(ERR_MALLOCERR)
                return ptr;
        } else {
                return result;
        }
}

static void _alloc_invoke_free(allocator *self, void *ptr)
{
        UNUSED(self);
        return free(ptr);
}

static void _alloc_invoke_clone(allocator *dst, const allocator *self)
{
        *dst = *self;
}
