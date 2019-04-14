#ifndef CARBON_PRIORITY_QUEUE_H
#define CARBON_PRIORITY_QUEUE_H

#include <carbon/carbon-common.h>

typedef void * carbon_priority_queue_any_t;

typedef struct carbon_priority_queue_elem {
    carbon_priority_queue_any_t entry;
    size_t priority;
} carbon_priority_queue_elem_t;

typedef struct carbon_priority_queue {
    size_t size;
    size_t count;
    size_t min_size;

    carbon_priority_queue_elem_t **heap_array;
} carbon_priority_queue_t;

carbon_priority_queue_t * carbon_priority_queue_create(
        size_t initial_size
);

void carbon_priority_queue_free(
        carbon_priority_queue_t * queue
);

void carbon_priority_queue_push(
        carbon_priority_queue_t *queue,
        carbon_priority_queue_any_t element,
        size_t priority
);

carbon_priority_queue_any_t * carbon_priority_queue_pop(
        carbon_priority_queue_t *queue
);

#endif //CARBON_PRIORITY_QUEUE_H
