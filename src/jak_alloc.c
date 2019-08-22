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

#include <errno.h>

#include <jak_alloc.h>

static void *invoke_malloc(struct jak_allocator *self, size_t size);

static void *invoke_realloc(struct jak_allocator *self, void *ptr, size_t size);

static void invoke_free(struct jak_allocator *self, void *ptr);

static void invoke_clone(struct jak_allocator *dst, const struct jak_allocator *self);

bool jak_alloc_create_std(struct jak_allocator *alloc)
{
        if (alloc) {
                alloc->extra = NULL;
                alloc->malloc = invoke_malloc;
                alloc->realloc = invoke_realloc;
                alloc->free = invoke_free;
                alloc->clone = invoke_clone;
                error_init(&alloc->err);
                return true;
        } else {
                return false;
        }
}

bool jak_alloc_this_or_std(struct jak_allocator *dst, const struct jak_allocator *self)
{
        if (!self) {
                return jak_alloc_create_std(dst);
        } else {
                return jak_alloc_clone(dst, self);
        }
}

void *jak_alloc_malloc(struct jak_allocator *alloc, size_t size)
{
        JAK_ASSERT(alloc);
        return alloc->malloc(alloc, size);
}

void *jak_alloc_realloc(struct jak_allocator *alloc, void *ptr, size_t size)
{
        return alloc->realloc(alloc, ptr, size);
}

bool jak_alloc_free(struct jak_allocator *alloc, void *ptr)
{
        JAK_ERROR_IF_NULL(alloc);
        JAK_ERROR_IF_NULL(ptr);
        alloc->free(alloc, ptr);
        return true;
}

bool jak_alloc_clone(struct jak_allocator *dst, const struct jak_allocator *src)
{
        JAK_ERROR_IF_NULL(dst && src)
        src->clone(dst, src);
        return true;
}

static void *invoke_malloc(struct jak_allocator *self, size_t size)
{
        JAK_UNUSED(self);
        void *result;

        errno = 0;
        if ((result = malloc(size)) == NULL) {
                print_error_and_die(JAK_ERR_MALLOCERR)
        } else {
                return result;
        }
}

static void *invoke_realloc(struct jak_allocator *self, void *ptr, size_t size)
{
        JAK_UNUSED(self);
        void *result;

        if ((result = realloc(ptr, size)) == NULL) {
                error_print(JAK_ERR_MALLOCERR)
                return ptr;
        } else {
                return result;
        }
}

static void invoke_free(struct jak_allocator *self, void *ptr)
{
        JAK_UNUSED(self);
        return free(ptr);
}

static void invoke_clone(struct jak_allocator *dst, const struct jak_allocator *self)
{
        *dst = *self;
}
