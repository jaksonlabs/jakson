#include <stdlib.h>
#include <status.h>
#include <stdio.h>

#include <ng5/allocator.h>

static void *default_malloc(struct allocator *self, size_t size);
static void *default_realloc(struct allocator *self, void *ptr, size_t size);
static void default_free(struct allocator *self, void *ptr);
static void default_clone(struct allocator *dst, const struct allocator *self);

int allocator_default(struct allocator *alloc)
{
    if (alloc) {
        alloc->extra       = NULL;
        alloc->malloc      = default_malloc;
        alloc->realloc     = default_realloc;
        alloc->free        = default_free;
        alloc->gc          = NULL;
        alloc->clone       = default_clone;
        alloc->drop        = NULL;
        return STATUS_OK;
    } else {
        return STATUS_NULLPTR;
    }
}

int allocator_this_or_default(struct allocator *dst, const struct allocator *this)
{
    if (!this) {
        return allocator_default(dst);
    } else {
        return allocator_clone(dst, this);
    }
}

void *allocator_malloc(struct allocator *alloc, size_t size)
{
    assert(alloc);
    return alloc->malloc(alloc, size);
}

void *allocator_realloc(struct allocator *alloc, void *ptr, size_t size)
{
    return alloc->realloc(alloc, ptr, size);
}

int allocator_free(struct allocator *alloc, void *ptr)
{
    check_non_null(alloc);
    check_non_null(ptr);
    alloc->free(alloc, ptr);
    return STATUS_OK;
}

void allocator_gc(struct allocator *alloc)
{
    if (alloc->gc) {
        alloc->gc(alloc);
    }
}

int allocator_clone(struct allocator *dst, const struct allocator *src)
{
    check_non_null(dst && src)
    src->clone(dst, src);
    return STATUS_OK;
}

int allocator_drop(struct allocator *alloc)
{
    check_non_null(alloc)
    if (alloc->drop) {
        alloc->drop(alloc);
    }
    return STATUS_OK;
}

static void *default_malloc(struct allocator *self, size_t size)
{
    unused(self);
    void *result;

    if ((result = malloc(size)) == NULL) {
        panic("malloc failed");
    } else {
        return result;
    }
}

static void *default_realloc(struct allocator *self, void *ptr, size_t size)
{
    unused(self);
    void *result;

    if ((result = realloc(ptr, size)) == NULL) {
        panic("malloc failed");
    } else {
        return result;
    }
}

static void default_free(struct allocator *self, void *ptr)
{
    unused(self);
    return free(ptr);
}

static void default_clone(struct allocator *dst, const struct allocator *self)
{
    *dst = *self;
}
