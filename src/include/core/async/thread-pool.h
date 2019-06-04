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

struct thread_task {
        void *args;
        pthread_attr_t *attr;
        task_routine routine;
        size_t group_id;
        size_t priority;
        struct task_stats statistics;
};

struct thread_info;

struct task_state {
        atomic_int task_count; // remaining tasks in this group
        unsigned generation;
};

struct task_handle {
        size_t index;
        unsigned generation;
};

struct thread_pool {
        char *name;
        pthread_t *pool;
        struct priority_queue waiting_tasks;
        struct task_state *task_group_states;
        size_t task_state_capacity; // number of tasks that can be tracked
        size_t size;
        size_t capacity;
        struct thread_info **thread_infos;
        struct thread_task **thread_tasks;
        struct thread_pool_stats *statistics;
        int enable_monitoring;
};

struct thread_info {
        char name[12];
        struct thread_pool *pool;
        size_t id;
        atomic_int status;
        struct thread_stats *statistics;
};

NG5_EXPORT(struct thread_pool *) thread_pool_create(size_t num_threads, int enable_monitoring);

NG5_EXPORT(struct thread_pool *) thread_pool_create_named(size_t num_threads, const char *name, int enable_monitoring);

// Releases all resources hold by the threadpool. 
// Currently working threads may finish but tasks left in the queue will be discarded.
NG5_EXPORT(void) thread_pool_free(struct thread_pool *pool);

NG5_EXPORT(void) thread_pool_set_name(struct thread_pool *pool, const char *name);

// Sets the number of active threads to num_threads.
// Currently working threads are terminated after there task is completed.
NG5_EXPORT(bool) thread_pool_resize(struct thread_pool *pool, size_t num_threads);

// Add multiple tasks to be executed. Their progress is tracked by a single handle.
// hndl can be a nullptr.
NG5_EXPORT(bool) thread_pool_enqueue_tasks(struct thread_task *task, struct thread_pool *pool, size_t num_tasks, struct task_handle *hndl);
NG5_EXPORT(bool) thread_pool_enqueue_task(struct thread_task *task, struct thread_pool *pool, struct task_handle *hndl);

// Add multiple tasks to be executed. Waits until all passed tasks are finished. 
// The main thread also participates in task execution
NG5_EXPORT(bool) thread_pool_enqueue_tasks_wait(struct thread_task *task, struct thread_pool *pool, size_t num_tasks);

// Waits until the tasks referenced by hndl are completed.
NG5_EXPORT(bool) thread_pool_wait_for_task(struct thread_pool *pool, struct task_handle *hndl);

// Waits until all tasks currently in the queue are executed.
// The main thread also participates in task execution.
NG5_EXPORT(bool) thread_pool_wait_for_all(struct thread_pool *pool);

NG5_EXPORT(void *) __thread_main(void *args);
NG5_EXPORT(struct thread_task *) __get_next_task(struct thread_pool *pool);

NG5_EXPORT(bool) __create_thread(struct thread_info *thread_info, pthread_t *pp);

NG5_EXPORT(void) __sig_seg(int sig);

NG5_END_DECL

#endif
