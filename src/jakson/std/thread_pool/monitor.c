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

#include <stdio.h>

#include <jakson/stdinc.h>
#include <jakson/std/thread_pool.h>

thread_pool_stats thread_pool_get_stats(thread_pool *pool)
{
        // In case no tasks care completed, no averages can be calculated
        if (pool->statistics->task_complete_count) {
                pool->statistics->avg_complete_time =
                        pool->statistics->complete_time / pool->statistics->task_complete_count;
                pool->statistics->avg_wait_time = pool->statistics->wait_time / pool->statistics->task_complete_count;
        }
        return *pool->statistics;
}

thread_stats thread_pool_get_thread_stats(thread_pool *pool, size_t id)
{
        thread_stats *thread_stats = pool->thread_infos[id]->statistics;

        // busy_time = running_time - idle_time
        struct timespec current;
        clock_gettime(CLOCK_MONOTONIC, &current);
        thread_stats->busy_time = __get_time_diff(&thread_stats->creation_time, &current) - thread_stats->idle_time;
        return *thread_stats;
}

double thread_pool_get_time_working(thread_pool *pool)
{
        struct timespec end;

        clock_gettime(CLOCK_MONOTONIC, &end);
        double avg = 0.f;
        for (size_t i = 0; i < pool->size; ++i) {
                struct timespec begin = pool->thread_infos[i]->statistics->creation_time;
                double t = __get_time_diff(&begin, &end);
                avg += t / (t + pool->thread_infos[i]->statistics->idle_time);
        }
        return avg / pool->size;
}