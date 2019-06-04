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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "core/async/thread-pool.h"

static inline void __execute_task();

NG5_EXPORT(thread_pool_t *) thread_pool_create(size_t num_threads, int enable_monitoring)
{
        thread_pool_t *pool = malloc(sizeof(thread_pool_t));

        pool->name = NULL;
        pool->size = num_threads;
        pool->capacity = num_threads * 2;
        priority_queue_init(&pool->waiting_tasks);

        pool->thread_tasks = calloc(num_threads, sizeof(thread_task_t *));
        pool->thread_infos = calloc(sizeof(__thread_information_t *) * pool->capacity, 1);
        pool->task_state_capacity = MAX_NUM_TASKS;
        pool->task_group_states = calloc(pool->task_state_capacity, sizeof(__task_state_t));
        pool->enable_monitoring = enable_monitoring;

        pthread_t *threads = malloc(sizeof(pthread_t) * pool->capacity);
        pool->pool = threads;

        if (enable_monitoring) {
                pool->statistics = calloc(1, sizeof(thread_pool_stats));
        }

        for (size_t i = 0; i < pool->capacity; i++) {
                // one block per thread to reduce risk of two threads sharing the same cache line
                __thread_information_t *thread_info = malloc(sizeof(__thread_information_t));
                pool->thread_infos[i] = thread_info;
                thread_info->pool = pool;
                thread_info->id = i;
                thread_info->status = thread_status_empty;
                sprintf(thread_info->name, "worker-%zu", i); // "worker%I64d" lead to segfault on linux

                if (enable_monitoring) {
                        thread_info->statistics = calloc(1, sizeof(thread_stats));
                }

        }
        for (size_t i = 0; i < num_threads; ++i) {
                __create_thread(pool->thread_infos[i], &pool->pool[i]);
        }

        return pool;
}

NG5_EXPORT(thread_pool_t *) thread_pool_create_named(size_t num_threads, const char *name, int enable_monitoring)
{
        thread_pool_t *pool = thread_pool_create(num_threads, enable_monitoring);

        if (name) {
                thread_pool_set_name(pool, name);
        }

        return pool;
}

NG5_EXPORT(void) thread_pool_free(thread_pool_t *pool)
{
        // Update all status
        for (size_t i = 0; i < pool->size; ++i) {
                pool->thread_infos[i]->status = thread_status_will_terminate;
        }
        // wait for threads to finish
        for (size_t i = 0; i < pool->size; ++i) {
                pthread_join(pool->pool[i], NULL);
                if (pool->enable_monitoring) {
                        free(pool->thread_infos[i]->statistics);
                }

                free(pool->thread_infos[i]);
        }
        for (size_t i = pool->size; i < pool->capacity; ++i) {
                if (pool->enable_monitoring) {
                        free(pool->thread_infos[i]->statistics);
                }

                free(pool->thread_infos[i]);
        }

        priority_queue_free(&pool->waiting_tasks);
        free(pool->pool);
        free(pool->thread_tasks);
        free(pool->thread_infos);
        free(pool->task_group_states);
        if (pool->name) {
                free(pool->name);
        }
        if (pool->enable_monitoring) {
                free(pool->statistics);
        }
        free(pool);
}

NG5_EXPORT(void) thread_pool_set_name(thread_pool_t *pool, const char *name)
{
        if (pool->name) {
                free(pool->name);
        }

        size_t s = strlen(name);
        char *str = malloc(s + 1);
        strcpy(str, name);
        pool->name = str;
}

NG5_EXPORT(bool) thread_pool_resize(thread_pool_t *pool, size_t num_threads)
{
        if (num_threads > pool->size) {
                if (num_threads > pool->capacity) {
                        return false;
                }

                for (size_t i = pool->size; i < num_threads; ++i) {
                        //try to revive thread
                        int will_terminate = thread_status_will_terminate;
                        if (atomic_compare_exchange_strong(&pool->thread_infos[i]->status,
                                &will_terminate,
                                thread_status_idle)) { }
                        else {
                                // create a new
                                if (pool->enable_monitoring && !pool->thread_infos[i]->statistics) {
                                        pool->thread_infos[i]->statistics = calloc(1, sizeof(thread_stats));
                                }
                                __create_thread(pool->thread_infos[i], &pool->pool[i]);
                        }
                }
        } else if (num_threads < pool->size) {
                for (size_t i = num_threads; i < pool->size; ++i) {
                        // mark threads for termination
                        pool->thread_infos[i]->status = thread_status_will_terminate;
                }
        }

        pool->size = num_threads;

        return true;
}

NG5_EXPORT(bool) thread_pool_enqueue_tasks(thread_task_t *tasks, thread_pool_t *pool, size_t num_tasks, task_handle_t *hndl)
{
        // find unused slot
        size_t ind = 0;

        for (; pool->task_group_states[ind].task_count; ind = (ind + 8) % MAX_NUM_TASKS)
                { }

        // increment generation first to always be identifiable as finished
        ++pool->task_group_states[ind].generation;
        pool->task_group_states[ind].task_count = num_tasks;

        for (size_t i = 0; i < num_tasks; i++) {

                if (pool->enable_monitoring) {
                        //tasks[i].statistics = calloc(1, sizeof(task_stats));
                        clock_gettime(CLOCK_MONOTONIC, &tasks[i].statistics.enqueue_time);
                        pool->statistics->task_enqueued_count++;
                }

                tasks[i].group_id = ind;
                priority_queue_push(&pool->waiting_tasks, &tasks[i], tasks[i].priority);
        }

        if (hndl) {
                hndl->index = ind;
                hndl->generation = pool->task_group_states[ind].generation;
        }

        return true;
}

NG5_EXPORT(bool) thread_pool_enqueue_task(thread_task_t *task, thread_pool_t *pool, task_handle_t *hndl)
{
        return thread_pool_enqueue_tasks(task, pool, 1, hndl);
}

NG5_EXPORT(bool) thread_pool_enqueue_tasks_wait(thread_task_t *tasks, thread_pool_t *pool, size_t num_tasks)
{
        // Pass all tasks except the last one to the queue
        task_handle_t hndl;
        thread_pool_enqueue_tasks(tasks, pool, num_tasks - 1, &hndl);

        // Execute the last tasks in the calling thread
        thread_task_t *main_task = &tasks[num_tasks - 1];

        if (pool->enable_monitoring) {
                pool->statistics->task_enqueued_count++;
                //main_task->statistics = calloc(1, sizeof(task_stats));

                // No waiting if the calling thread executes the task
                clock_gettime(CLOCK_MONOTONIC, &main_task->statistics.enqueue_time);
                main_task->statistics.execution_time = main_task->statistics.enqueue_time;

                (*main_task->routine)(main_task->args);
                clock_gettime(CLOCK_MONOTONIC, &main_task->statistics.complete_time);
                pool->statistics->task_complete_count++;
        } else {
                (*main_task->routine)(main_task->args);
        }

        return thread_pool_wait_for_task(pool, &hndl);
}

NG5_EXPORT(bool) thread_pool_wait_for_task(thread_pool_t *pool, task_handle_t *hndl)
{
        volatile unsigned *gen = &pool->task_group_states[hndl->index].generation;
        while (*gen == hndl->generation && pool->task_group_states[hndl->index].task_count) { }
        return true;
}

NG5_EXPORT(bool) thread_pool_wait_for_all(thread_pool_t *pool)
{
        thread_task_t *next_task;
        while ((next_task = __get_next_task(pool))) {

                if (pool->enable_monitoring) {
                        clock_gettime(CLOCK_MONOTONIC, &next_task->statistics.execution_time);
                        __execute_task(pool, next_task);
                        clock_gettime(CLOCK_MONOTONIC, &next_task->statistics.complete_time);
                        pool->statistics->task_complete_count++;

                        // Just add the time, calculate the average at evaluation time
                        pool->statistics->wait_time += __get_time_diff(&next_task->statistics.enqueue_time,
                                &next_task->statistics.execution_time);
                        pool->statistics->complete_time += __get_time_diff(&next_task->statistics.execution_time,
                                &next_task->statistics.complete_time);
                } else {
                        __execute_task(pool, next_task);
                }

        }
        for (;;) {
                size_t sum = 0;
                for (size_t i = 0; i < pool->task_state_capacity; ++i) {
                        sum += pool->task_group_states[i].task_count;
                }
                if (!sum) { return true; }
        }
        return false;
}

NG5_EXPORT(void *) __thread_main(void *args)
{
        __thread_information_t *thread_info = (__thread_information_t *) args;

        // Fill statistics if available
        struct timespec begin;
        if (thread_info->pool->enable_monitoring) {
                clock_gettime(CLOCK_MONOTONIC, &thread_info->statistics->creation_time);
                begin = thread_info->statistics->creation_time;
        }

        while (1) {
                thread_task_t *next_task = __get_next_task(thread_info->pool);
                // the task has to be executed since it has been taken out of the queue
                if (next_task) {

                        // Fill statistics if available
                        if (thread_info->pool->enable_monitoring) {
                                // measure time outside to prevent incorrect times while in execution
                                struct timespec end;
                                clock_gettime(CLOCK_MONOTONIC, &end);
                                next_task->statistics.execution_time = end;
                                thread_info->statistics->idle_time += __get_time_diff(&begin, &end);

                                __execute_task(thread_info->pool, next_task);
                                clock_gettime(CLOCK_MONOTONIC, &begin);
                                next_task->statistics.complete_time = begin;
                                thread_info->statistics->task_count++;
                                thread_info->pool->statistics->task_complete_count++;

                                // Just add the time, calculate the average at evaluation time
                                thread_info->pool->statistics->wait_time +=
                                        __get_time_diff(&next_task->statistics.enqueue_time,
                                                &next_task->statistics.execution_time);
                                thread_info->pool->statistics->complete_time +=
                                        __get_time_diff(&next_task->statistics.execution_time,
                                                &next_task->statistics.complete_time);

                        } else {
                                __execute_task(thread_info->pool, next_task);
                        }

                }

                // Check if this thread has to terminate, set the status and leave the loop
                int will_terminate = thread_status_will_terminate;
                if (atomic_compare_exchange_strong(&thread_info->status, &will_terminate, thread_status_killed)) {
                        break;
                }


//    if(!next_task)
//      nanosleep(&waiting_time_start, &waiting_time_end);
        }

        thread_info->status = thread_status_finished;

        // Be sure to free the passed thread_information since no other reference exists
        // free(thread_info);

        return (void *) 0;
}

NG5_EXPORT(thread_task_t *) __get_next_task(thread_pool_t *pool)
{
        thread_task_t *next_task = priority_queue_pop(&pool->waiting_tasks);
        return next_task;
}

NG5_EXPORT(bool) __create_thread(__thread_information_t *thread_info, pthread_t *pp)
{
        thread_info->status = thread_status_created;
        pthread_create(pp, NULL, &__thread_main, thread_info);

        return true;
}

void *faulty;

size_t num;

void __execute_task(thread_pool_t *pool, thread_task_t *task)
{
        faulty = task->routine;
        num++;
        (*task->routine)(task->args);
        --pool->task_group_states[task->group_id].task_count;
}

NG5_EXPORT(void) __sig_seg(int sig)
{
        if (sig != SIGSEGV) {
                return;
        } else {
                printf("%p \n", faulty);
                printf("occured after: %li", num);
                exit(1);
        }
}