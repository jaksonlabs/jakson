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

#ifndef THREAD_POOL_STATISTICS_H
#define THREAD_POOL_STATISTICS_H

#include <jakson/stdinc.h>

BEGIN_DECL

#include <jakson/utils/time.h>
#include <stdio.h>

typedef struct thread_pool_stats {
        struct timespec creation_time;
        unsigned int task_enqueued_count;
        unsigned int task_complete_count;
        long long complete_time;
        long long wait_time;
        long long avg_complete_time;
        long long avg_wait_time;
} thread_pool_stats;

typedef struct thread_stats {
        struct timespec creation_time;
        long long idle_time;
        long long busy_time;
        size_t task_count;
} thread_stats;

typedef struct task_stats {
        struct timespec enqueue_time;
        struct timespec execution_time;
        struct timespec complete_time;
} task_stats;

static inline long long __get_time_diff(struct timespec *begin, struct timespec *end)
{
        return (end->tv_sec - begin->tv_sec) * 1000000000L + (end->tv_nsec - begin->tv_nsec); /// 1000000000.0;
}

END_DECL

#endif
