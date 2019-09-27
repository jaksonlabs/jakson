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

#ifndef JAK_PRIORITY_QUEUE_H
#define JAK_PRIORITY_QUEUE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>

JAK_BEGIN_DECL

typedef struct jak_priority_queue_element_info {
        size_t priority;
        void *element;
} jak_priority_queue_element_info;

typedef struct jak_priority_queue {
        jak_priority_queue_element_info *data;
        size_t num_elements;
        size_t capacity;
        pthread_mutex_t mutex;
} jak_priority_queue;

void jak_priority_queue_init(jak_priority_queue *queue);
void jak_priority_queue_free(jak_priority_queue *queue);
void jak_priority_queue_push(jak_priority_queue *queue, void *data, size_t priority);
void *jak_priority_queue_pop(jak_priority_queue *queue);
int jak_priority_queue_is_empty(jak_priority_queue *queue);

JAK_END_DECL

#endif