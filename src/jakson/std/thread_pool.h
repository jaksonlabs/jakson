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

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/utils/priority_queue.h>
#include <jakson/std/thread_pool/status.h>
#include <jakson/std/thread_pool/stats.h>

BEGIN_DECL

#ifndef NOOP
#define NOOP (void)0
#endif

#include <stdlib.h>
#include <pthread.h>

// Required due to bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
// stdatomic.h must no be included in GTest
#ifndef CPP_TEST

#include <stdatomic.h>

#endif

#define THREAD_POOL_MAX_TASKS 4097

typedef void (*task_routine)(void *routine);

typedef struct thread_task {
        void *args;
        pthread_attr_t *attr;
        task_routine routine;
        size_t group_id;
        size_t priority;
        task_stats statistics;
} thread_task;

typedef struct task_state {
        atomic_int task_count; // remaining tasks in this group
        unsigned generation;
} task_state;

typedef struct task_handle {
        size_t index;
        unsigned generation;
} task_handle;

typedef struct thread_pool {
        char *name;
        pthread_t *pool;
        priority_queue waiting_tasks;
        task_state *task_group_states;
        size_t task_state_capacity; // number of tasks that can be tracked
        size_t size;
        size_t capacity;
        thread_info **thread_infos;
        thread_task **thread_tasks;
        thread_pool_stats *statistics;
        int enable_monitoring;
} thread_pool;

typedef struct thread_info {
        char name[12];
        thread_pool *pool;
        size_t id;
        atomic_int status;
        thread_stats *statistics;
} thread_info;

thread_pool *thread_pool_create(size_t num_threads, int enable_monitoring);
thread_pool *thread_pool_create_named(size_t num_threads, const char *name, int enable_monitoring);

// Releases all resources hold by the threadpool. 
// Currently working threads may finish but tasks left in the queue will be discarded.
void thread_pool_free(thread_pool *pool);
void thread_pool_set_name(thread_pool *pool, const char *name);

// Sets the number of active threads to num_threads.
// Currently working threads are terminated after there task is completed.
bool thread_pool_resize(thread_pool *pool, size_t num_threads);

// Add multiple tasks to be executed. Their progress is tracked by a single handle.
// hndl can be a nullptr.
bool thread_pool_enqueue_tasks(thread_task *task, thread_pool *pool, size_t num_tasks, task_handle *hndl);

bool thread_pool_enqueue_task(thread_task *task, thread_pool *pool, task_handle *hndl);

// Add multiple tasks to be executed. Waits until all passed tasks are finished. 
// The main thread also participates in task execution
bool thread_pool_enqueue_tasks_wait(thread_task *task, thread_pool *pool, size_t num_tasks);

// Waits until the tasks referenced by hndl are completed.
bool thread_pool_wait_for_task(thread_pool *pool, task_handle *hndl);

// Waits until all tasks currently in the queue are executed.
// The main thread also participates in task execution.
bool thread_pool_wait_for_all(thread_pool *pool);

void *__thread_main(void *args);
thread_task *__get_next_task(thread_pool *pool);
bool __create_thread(thread_info *thread_info, pthread_t *pp);
void __sig_seg(int sig);

END_DECL

#endif
