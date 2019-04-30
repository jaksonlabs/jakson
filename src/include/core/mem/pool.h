/**
 * Copyright 2019 Marcus Pinnecke
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

#ifndef NG5_POOL_H
#define NG5_POOL_H

#include "shared/common.h"
#include "core/mem/pools/none.h"
#include "std/vec.h"
#include "std/histogram.h"
#include "shared/error.h"

NG5_BEGIN_DECL

#define TO_C_PTR(pool_ptr)      (((void *)pool_ptr) + sizeof(struct pool_ptr_header))
#define FROM_C_PTR(c_ptr)       ((struct pool_ptr *) ((void *) c_ptr - sizeof(struct pool_ptr_header)))

enum pool_options
{
        MEM_UNPOOLED   = 0,       /* no special treatment; just delegate to clib `malloc`, `realloc`, and `free` */
        MEM_POOLED     = 1 << 0,  /* if disabled, just use 'malloc' from clib */
        MEM_GC_SNYC    = 1 << 1,  /* if pooling enabled, run GC sync if triggered by caller */
        MEM_GC_ASYNC   = 1 << 2,  /* if pooling enabled, run GC async if triggered by caller */
        MEM_PRESSURE   = 1 << 3,  /* if pooling enabled, run GC automatically as long as mem pressure is high */
        MEM_LINEAR     = 1 << 4,  /* use one single freelist holding 'free'd blocks for reuse */
        MEM_CHUNKED    = 1 << 5,  /* use fixed k single freelist holding 'free'd blocks for reuse for k different sizes */
        MEM_BALANCED   = 1 << 6,  /* use variable k single freelist holding 'free'd blocks for reuse for k different sizes */
        MEM_FIRST_FIT  = 1 << 7,  /* use strategy that selects first block in freelist that is larger than requested one */
        MEM_BEST_FIT   = 1 << 8,  /* use strategy that selects least block in freelist that is larger than requested one */
        MEM_RANDOM_FIT = 1 << 9,  /* use strategy that selects a block in freelist larger than request randomly with k tries */
        MEM_CRACKED    = 1 << 10, /* build an auto organizing index above the freelist */
        MEM_PARALLEL   = 1 << 11, /* use multiple threads for searching */
        MEM_USESIMD    = 1 << 12, /* use SIMD acceleration for lookups */
        MEM_AUTO_DEDUP = 1 << 13  /* maps equal memory blocks (based on memcmp) to same pointer */
};

enum pool_impl_tag
{
        POOL_IMPL_NONE
};

static struct pool_register_entry
{
        struct {
                u16 pooled     : 1;
                u16 gc_sync    : 1;
                u16 gc_async   : 1;
                u16 pressure   : 1;
                u16 linear     : 1;
                u16 chunked    : 1;
                u16 balanced   : 1;
                u16 first_fit  : 1;
                u16 best_fit   : 1;
                u16 random_fit : 1;
                u16 cracked    : 1;
                u16 parallel   : 1;
                u16 simd       : 1;
                u16 dedup      : 1;
        } ops;

        void (*_create)(struct pool_strategy *dst);
        void (*_drop)(struct pool_strategy *dst);
} pool_register[] = {
        {
                .ops.pooled     = false,
                .ops.gc_sync    = false,
                .ops.gc_async   = false,
                .ops.pressure   = false,
                .ops.linear     = false,
                .ops.chunked    = false,
                .ops.balanced   = false,
                .ops.first_fit  = false,
                .ops.best_fit   = false,
                .ops.random_fit = false,
                .ops.cracked    = false,
                .ops.parallel   = false,
                .ops.simd       = false,
                .ops.dedup      = false,
                ._create = pool_strategy_none_create,
                ._drop = NULL
        }
};

struct pool_counters
{
        u32 num_alloc_calls;            /* num of calls to `alloc` that may lead to a system call */
        u32 num_realloc_calls;          /* num of calls to `realloc` that may lead to a system call */
        u32 num_free_calls;             /* num of calls to `free` that may lead to a system call */

        u32 num_gc_calls;               /* number of calls to `gc` (garbage collection) calls */

        u32 num_managed_alloc_calls;    /* num of calls to pool_alloc that do not call `alloc` (e.g., by caching) */
        u32 num_managed_realloc_calls;  /* num of calls to pool_realloc that do not call `realloc` (e.g., by caching) */
        u32 num_free_realloc_calls;     /* num of calls to pool_free that do not call `free` (e.g., by caching) */

        u32 impl_mem_footprint;         /* total memory requirements (in B) for implementation at time of calling */

        u32 num_bytes_allocd;           /* num of bytes currently allocated in total */
        u32 num_bytes_reallocd;         /* num of bytes currently reallocated in total */
        u32 num_bytes_freed;            /* num of bytes currently free'd in total */

        u32 num_bytes_alloc_cache;      /* bytes used to organize managed alloc calls (e.g., cache size) */
        u32 num_bytes_realloc_cache;    /* bytes used to organize managed realloc calls (e.g., cache size) */
        u32 num_bytes_free_cache;       /* bytes used to organize managed free calls (e.g., cache size) */

        u32 num_bytes_alloc_blocked;    /* portion (in bytes) of num_bytes_alloc_cache that cannot be used */
        u32 num_bytes_realloc_blocked;  /* portion (in bytes) of num_bytes_realloc_cache that cannot be used */
        u32 num_bytes_free_blocked;     /* portion (in bytes) of num_bytes_free_cache that cannot be used */
};

struct pool; /* forwarded */

struct pool_strategy
{
        const char *impl_name;

        enum pool_impl_tag tag;
        struct pool_counters counters;

        void *extra;
        struct pool *context;

        void *(*_alloc)(struct pool_strategy *self, u64 nbytes);
        void *(*_realloc)(struct pool_strategy *self, void *ptr, u64 nbytes);
        bool (*_free)(struct pool_strategy *self, void *ptr);
        bool (*_gc)(struct pool_strategy *self);
        bool (*_update_counters)(struct pool_strategy *self);
        bool (*_reset_counters)(struct pool_strategy *self);
};

struct pool
{
        struct err                        err;
        struct vector ofType(pool_ptr)    in_used;
        struct vector ofType(u32)         in_used_pos_freelist;
        struct spinlock                   lock;
        struct pool_strategy              strategy;
};

struct pool_ptr_header {
        struct {
                u64 is_free     : 1;
                u64 bytes_used  : 32;
                u64 bytes_total : 31;
        } meta;
        u32 list_position;
};


struct pool_ptr
{
        struct pool_ptr_header header;
        void *ptr;
};



NG5_DEFINE_GET_ERROR_FUNCTION(pool, struct pool, p);

NG5_EXPORT(bool) pool_create(struct pool *pool, enum pool_options options);
NG5_EXPORT(bool) pool_create_by_name(struct pool *pool, const char *name);
NG5_EXPORT(bool) pool_drop(struct pool *pool);
NG5_EXPORT(bool) pool_reset_counters(struct pool *pool);
NG5_EXPORT(bool) pool_get_counters(struct pool_counters *counters, struct pool *pool);
NG5_EXPORT(void *) pool_alloc(struct pool *pool, u64 nbytes);
NG5_EXPORT(void *) pool_alloc_array(struct pool *pool, u32 how_many, u64 nbytes);
NG5_EXPORT(void *) pool_realloc(struct pool *pool, void *ptr, u64 nbytes);
NG5_EXPORT(const char*) pool_impl_name(struct pool *pool);
NG5_EXPORT(bool) pool_gc(struct pool *pool);
NG5_EXPORT(bool) pool_free(struct pool *pool, void *ptr);
NG5_EXPORT(bool) pool_free_all(struct pool *pool);

NG5_EXPORT(struct pool_ptr *) pool_internal_register(struct pool *pool, void *ptr, u32 bytes_used, u32 bytes_total);

NG5_EXPORT(void) pool_internal_unregister(struct pool *pool, struct pool_ptr *ptr);

NG5_END_DECL

#endif
