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

#ifndef NG5_MEMPOOL_H
#define NG5_MEMPOOL_H

#include <shared/error.h>
#include <std/vec.h>

struct mempool
{
        struct err        err;
        struct vector     in_used;
        struct vector     freelist_qe_8B;
};

NG5_EXPORT(bool) mempool_create(struct mempool *pool);
NG5_EXPORT(bool) mempool_drop(struct mempool *pool);
NG5_EXPORT(void *) mempool_alloc(struct mempool *pool, u64 nbytes);
NG5_EXPORT(void *) mempool_alloc_array(u32 how_many, u64 nbytes);
NG5_EXPORT(bool) mempool_free(struct mempool *pool, void *ptr);
NG5_EXPORT(bool) mempool_free_all(struct mempool *pool);
NG5_EXPORT(void *) mempool_realloc(struct mempool *pool, void *ptr, u64 nbytes);
NG5_EXPORT(bool) mempool_gc(struct mempool *pool);


#endif
