#include <stdlib.h>
#include <ng5_status.h>
#include <stdio.h>

#include <stdx/ng5_allocator.h>

static void *default_malloc(ng5_allocator_t *self, size_t size);
static void *default_realloc(ng5_allocator_t *self, void *ptr, size_t size);
static void default_free(ng5_allocator_t *self, void *ptr);
static void default_clone(ng5_allocator_t *dst, const ng5_allocator_t *self);

int allocator_default(ng5_allocator_t *alloc)
{
    if (alloc) {
        alloc->extra        = NULL;
        alloc->malloc       = default_malloc;
        alloc->realloc      = default_realloc;
        alloc->free         = default_free;
        alloc->gc           = NULL;
        alloc->clone        = default_clone;
        alloc->drop         = NULL;
        return STATUS_OK;
    } else {
        return STATUS_NULLPTR;
    }
}

int allocator_this_or_default(ng5_allocator_t *dst, const ng5_allocator_t *this)
{
    if (!this) {
        return allocator_default(dst);
    } else {
        return allocator_clone(dst, this);
    }
}

void *allocator_malloc(ng5_allocator_t *alloc, size_t size)
{
    assert(alloc);
    return alloc->malloc(alloc, size);
}

void *allocator_realloc(ng5_allocator_t *alloc, void *ptr, size_t size)
{
    return alloc->realloc(alloc, ptr, size);
}

int allocator_free(ng5_allocator_t *alloc, void *ptr)
{
    check_non_null(alloc);
    check_non_null(ptr);
    alloc->free(alloc, ptr);
    return STATUS_OK;
}

void allocator_gc(ng5_allocator_t *alloc)
{
    if (alloc->gc) {
        alloc->gc(alloc);
    }
}

int allocator_clone(ng5_allocator_t *dst, const ng5_allocator_t *src)
{
    check_non_null(dst && src)
    src->clone(dst, src);
    return STATUS_OK;
}

int allocator_drop(ng5_allocator_t *alloc)
{
    check_non_null(alloc)
    if (alloc->drop) {
        alloc->drop(alloc);
    }
    return STATUS_OK;
}

static void *default_malloc(ng5_allocator_t *self, size_t size)
{
    unused(self);
    void *result;

    if ((result = malloc(size)) == NULL) {
        panic("malloc failed");
    } else {
        return result;
    }
}

static void *default_realloc(ng5_allocator_t *self, void *ptr, size_t size)
{
    unused(self);
    void *result;

    if ((result = realloc(ptr, size)) == NULL) {
        panic("malloc failed");
    } else {
        return result;
    }
}

static void default_free(ng5_allocator_t *self, void *ptr)
{
    unused(self);
    return free(ptr);
}

static void default_clone(ng5_allocator_t *dst, const ng5_allocator_t *self)
{
    *dst = *self;
}
