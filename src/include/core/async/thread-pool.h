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

#ifndef NG5_THREAD_POOL_H
#define NG5_THREAD_POOL_H

#include "shared/common.h"
#include "std/priority_queue.h"

#include "thread-pool-status.h"
#include "thread-pool-stats.h"

NG5_BEGIN_DECL

#ifndef noop
#define noop (void)0
#endif //noop

#include <stdlib.h>
#include <pthread.h>

// Required due to bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
// stdatomic.h must no be included in GTest
#ifndef CPP_TEST

#include <stdatomic.h>

#endif

#define MAX_NUM_TASKS 4097

typedef void (*task_routine)(void *routine);

typedef struct thread_task_t {
        void *args;
        pthread_attr_t *attr;
        task_routine routine;
        size_t group_id;
        size_t priority;
        task_stats statistics;
} thread_task_t;

struct __thread_information_t;

typedef struct __task_state_t {
        atomic_int task_count; // remaining tasks in this group
        unsigned generation;
} __task_state_t;

typedef struct task_handle_t {
        size_t index;
        unsigned generation;
} task_handle_t;

typedef struct thread_pool_t {
        char *name;
        pthread_t *pool;
        struct priority_queue waiting_tasks;
        __task_state_t *task_group_states;
        size_t task_state_capacity; // number of tasks that can be tracked
        size_t size;
        size_t capacity;
        struct __thread_information_t **thread_infos;
        thread_task_t **thread_tasks;
        thread_pool_stats *statistics;
        int enable_monitoring;
} thread_pool_t;

typedef struct __thread_information_t {
        char name[12];
        thread_pool_t *pool;
        size_t id;
        atomic_int status;
        thread_stats *statistics;
} __thread_information_t;

NG5_EXPORT(thread_pool_t *) thread_pool_create(size_t num_threads, int enable_monitoring);

NG5_EXPORT(thread_pool_t *) thread_pool_create_named(size_t num_threads, const char *name, int enable_monitoring);

// Releases all resources hold by the threadpool. 
// Currently working threads may finish but tasks left in the queue will be discarded.
NG5_EXPORT(void) thread_pool_free(thread_pool_t *pool);

NG5_EXPORT(void) thread_pool_set_name(thread_pool_t *pool, const char *name);

// Sets the number of active threads to num_threads.
// Currently working threads are terminated after there task is completed.
NG5_EXPORT(bool) thread_pool_resize(thread_pool_t *pool, size_t num_threads);

// Add multiple tasks to be executed. Their progress is tracked by a single handle.
// hndl can be a nullptr.
NG5_EXPORT(bool) thread_pool_enqueue_tasks(thread_task_t *task, thread_pool_t *pool, size_t num_tasks, task_handle_t *hndl);
NG5_EXPORT(bool) thread_pool_enqueue_task(thread_task_t *task, thread_pool_t *pool, task_handle_t *hndl);

// Add multiple tasks to be executed. Waits until all passed tasks are finished. 
// The main thread also participates in task execution
NG5_EXPORT(bool) thread_pool_enqueue_tasks_wait(thread_task_t *task, thread_pool_t *pool, size_t num_tasks);

// Waits until the tasks referenced by hndl are completed.
NG5_EXPORT(bool) thread_pool_wait_for_task(thread_pool_t *pool, task_handle_t *hndl);

// Waits until all tasks currently in the queue are executed.
// The main thread also participates in task execution.
NG5_EXPORT(bool) thread_pool_wait_for_all(thread_pool_t *pool);

NG5_EXPORT(void *) __thread_main(void *args);
NG5_EXPORT(thread_task_t *) __get_next_task(thread_pool_t *pool);

NG5_EXPORT(bool) __create_thread(__thread_information_t *thread_info, pthread_t *pp);

NG5_EXPORT(void) __sig_seg(int sig);

NG5_END_DECL

#endif
