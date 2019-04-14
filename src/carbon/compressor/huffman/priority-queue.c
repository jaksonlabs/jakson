#include <stdlib.h>
#include <carbon/compressor/huffman/priority-queue.h>

void this_priority_queue_max_heapify(
        carbon_priority_queue_t *queue,
        size_t loc, size_t count
) {
    size_t left, right, largest;
    left = 2*(loc) + 1;
    right = left + 1;
    largest = loc;


    if( left <= count && queue->heap_array[left]->priority > queue->heap_array[largest]->priority ) {
        largest = left;
    }
    if( right <= count && queue->heap_array[right]->priority > queue->heap_array[largest]->priority ) {
        largest = right;
    }

    if(largest != loc) {
        carbon_priority_queue_elem_t * temp = queue->heap_array[loc];
        queue->heap_array[loc] = queue->heap_array[largest];
        queue->heap_array[largest] = temp;
        this_priority_queue_max_heapify(queue, largest, count);
    }
}


carbon_priority_queue_t * carbon_priority_queue_create(
        size_t initial_size
) {
    carbon_priority_queue_t * queue = malloc(sizeof(carbon_priority_queue_t));

    queue->count = 0;
    queue->size = initial_size;
    queue->min_size = initial_size;
    queue->heap_array = malloc(sizeof(carbon_priority_queue_elem_t *) * queue->size);
    return queue;
}


void carbon_priority_queue_free(
        carbon_priority_queue_t *queue
)
{
    free(queue->heap_array);
    free(queue);
}


void carbon_priority_queue_push(
        carbon_priority_queue_t *queue,
        carbon_priority_queue_any_t element,
        size_t priority
) {
    size_t index, parent;

    if(queue->count >= queue->size)
    {
        queue->size *= 2;
        queue->heap_array = realloc(queue->heap_array, sizeof(carbon_priority_queue_elem_t *) * queue->size);
    }

    index = queue->count++;

    for(;index; index = parent)
    {
        parent = (index - 1) / 2;
        if (queue->heap_array[parent]->priority >= priority)
            break;

        queue->heap_array[index] = queue->heap_array[parent];
    }

    queue->heap_array[index] = malloc(sizeof(carbon_priority_queue_elem_t));
    queue->heap_array[index]->entry = element;
    queue->heap_array[index]->priority = priority;
}

carbon_priority_queue_any_t *carbon_priority_queue_pop(
        carbon_priority_queue_t *queue
) {
    if( queue->count == 0 )
        return 0;

    carbon_priority_queue_elem_t * removed;
    carbon_priority_queue_elem_t * temp = queue->heap_array[--queue->count];

    if ((queue->count <= (queue->size / 2)) && (queue->size > queue->min_size))
    {
        queue->size /= 2;
        queue->heap_array = realloc(queue->heap_array, sizeof(carbon_priority_queue_elem_t *) * queue->size);
    }

    removed = queue->heap_array[0];
    queue->heap_array[0] = temp;
    this_priority_queue_max_heapify(queue, 0, queue->count);

    carbon_priority_queue_any_t entry = removed->entry;
    free(removed);
    return entry;
}
