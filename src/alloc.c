// file: alloc.c

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "alloc.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static void *invokeMalloc(Allocator *self, size_t size);
static void *invokeRealloc(Allocator *self, void *ptr, size_t size);
static void invokeFree(Allocator *self, void *ptr);
static void invokeClone(Allocator *dst, const Allocator *self);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int AllocatorCreateDefault(Allocator *alloc)
{
    if (alloc) {
        alloc->extra = NULL;
        alloc->malloc = invokeMalloc;
        alloc->realloc = invokeRealloc;
        alloc->free = invokeFree;
        alloc->clone = invokeClone;
        return STATUS_OK;
    }
    else {
        return STATUS_NULLPTR;
    }
}

int AllocatorThisOrDefault(Allocator *dst, const Allocator *self)
{
    if (!self) {
        return AllocatorCreateDefault(dst);
    }
    else {
        return AllocatorClone(dst, self);
    }
}

void *AllocatorMalloc(Allocator *alloc, size_t size)
{
    assert(alloc);
    return alloc->malloc(alloc, size);
}

void *AllocatorRealloc(Allocator *alloc, void *ptr, size_t size)
{
    return alloc->realloc(alloc, ptr, size);
}

int AllocatorFree(Allocator *alloc, void *ptr)
{
    CHECK_NON_NULL(alloc);
    CHECK_NON_NULL(ptr);
    alloc->free(alloc, ptr);
    return STATUS_OK;
}

int AllocatorClone(Allocator *dst, const Allocator *src)
{
    CHECK_NON_NULL(dst && src)
    src->clone(dst, src);
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static void *invokeMalloc(Allocator *self, size_t size)
{
    UNUSED(self);
    void *result;

    if ((result = malloc(size)) == NULL) {
        PANIC("malloc failed");
    }
    else {
        return result;
    }
}

static void *invokeRealloc(Allocator *self, void *ptr, size_t size)
{
    UNUSED(self);
    void *result;

    if ((result = realloc(ptr, size)) == NULL) {
        PANIC("realloc failed");
    }
    else {
        return result;
    }
}

static void invokeFree(Allocator *self, void *ptr)
{
    UNUSED(self);
    return free(ptr);
}

static void invokeClone(Allocator *dst, const Allocator *self)
{
    *dst = *self;
}
