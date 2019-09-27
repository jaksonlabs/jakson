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
#include <jakson/std/string.h>

#include <jakson/utils/priority_queue.h>

void priority_queue_init(priority_queue *queue)
{
        queue->num_elements = 0;
        pthread_mutex_init(&queue->mutex, NULL);
        queue->capacity = 0;
        queue->data = NULL;
}

void priority_queue_free(priority_queue *queue)
{
        free(queue->data);
}

void priority_queue_resize(priority_queue *queue, size_t size)
{
        priority_queue_element_info *new_data = MALLOC(sizeof(priority_queue_element_info) * size);

        memcpy(new_data, queue->data, sizeof(priority_queue_element_info) * queue->num_elements);
        free(queue->data);

        queue->data = new_data;
}

inline static void swap(priority_queue_element_info *el1, priority_queue_element_info *el2)
{
        priority_queue_element_info tmp = *el1;
        *el1 = *el2;
        *el2 = tmp;
}

static void down_heap(priority_queue_element_info *heap, size_t size)
{
        size_t cur = 0;
        size_t id = 1;
        for (size_t next = 3;; next = (next << 1) + 1) {
                size_t min = 0xffffffff;
                size_t minId = 0;
                while (id != next && id < size) {
                        if (heap[id].priority < min) {
                                min = heap[id].priority;
                                minId = id;
                        }
                        ++id;
                }
                if (heap[cur].priority <= heap[minId].priority) { break; }
                swap(&heap[cur], &heap[minId]);
                cur = minId;
                if (id == size) { break; }
        }
}

static void up_heap(priority_queue_element_info *heap, size_t size)
{
        if (size == 1) { return; }
        size_t log = 31 - __builtin_clz(size);

        size_t begin = 1 << (log - 1);
        size_t cur = size - 1;

        for (;;) {
                size_t end = (begin << 1) + 1;
                if (end > size) { end = size; }
                size_t max = heap[begin].priority;
                size_t id = begin;
                for (size_t i = begin + 1; i < end; ++i) {
                        if (heap[i].priority >= max) {
                                max = heap[i].priority;
                                id = i;
                        }
                }
                if (heap[id].priority <= heap[cur].priority) { break; }
                else {
                        swap(&heap[cur], &heap[id]);
                        cur = id;
                }
                if (!begin) { break; }
                begin >>= 1;
        }
}

void priority_queue_push(priority_queue *queue, void *data, size_t priority)
{
        pthread_mutex_lock(&queue->mutex);

        if (queue->num_elements >= queue->capacity) {
                priority_queue_resize(queue, ((queue->num_elements + 1) * 3) / 2);
        }

        queue->data[queue->num_elements].element = data;
        queue->data[queue->num_elements].priority = priority;
        ++queue->num_elements;
        up_heap(queue->data, queue->num_elements);

        pthread_mutex_unlock(&queue->mutex);
}

void *priority_queue_pop(priority_queue *queue)
{
        pthread_mutex_lock(&queue->mutex);

        void *ptr = NULL;

        if (queue->num_elements) {
                --queue->num_elements;

                ptr = queue->data[0].element;
                queue->data[0] = queue->data[queue->num_elements];
                down_heap(queue->data, queue->num_elements + 1);
        }

        pthread_mutex_unlock(&queue->mutex);
        return ptr;
}

int priority_queue_is_empty(priority_queue *queue)
{
        pthread_mutex_lock(&queue->mutex);

        int result = !queue->num_elements;

        pthread_mutex_unlock(&queue->mutex);
        return result;
}