/**
 * Copyright 2019 Marcus Pinnecke, Robert Jendersie, Johannes Wuensche, Johann Wagner, and Marten Wallewein-Eising
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without ion, including without limitation the
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

#ifndef ARK_THREAD_POOL_MONITORING_H
#define ARK_THREAD_POOL_MONITORING_H

#include <ark-js/shared/common.h>

ARK_BEGIN_DECL

#include "thread-pool.h"

// Returns the average fraction of time the active threads have been working.
double thread_pool_get_time_working(struct thread_pool *pool);

// Fill all stats of the passed thread pool instance
struct thread_pool_stats thread_pool_get_stats(struct thread_pool *pool);

// Fill all stats of the thread matching the given id in the thread pool
struct thread_stats thread_pool_get_thread_stats(struct thread_pool *pool, size_t id);

ARK_END_DECL

#endif
