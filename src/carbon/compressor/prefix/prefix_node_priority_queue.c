#include <stdlib.h>
#include <carbon/compressor/prefix/prefix_node_priority_queue.h>

void util_max_heapify(
        carbon_prefix_tree_node_with_prefix **heap_array,
        size_t loc, size_t count
) {
    size_t left, right, largest;
    left = 2*(loc) + 1;
    right = left + 1;
    largest = loc;


    if (left <= count && heap_array[left]->node->count > heap_array[largest]->node->count) {
        largest = left;
    }
    if (right <= count && heap_array[right]->node->count > heap_array[largest]->node->count) {
        largest = right;
    }

    if(largest != loc) {
        carbon_prefix_tree_node_with_prefix * temp = heap_array[loc];
        heap_array[loc] = heap_array[largest];
        heap_array[largest] = temp;
        util_max_heapify(heap_array, largest, count);
    }
}


carbon_prefix_tree_node_priority_queue * carbon_prefix_node_priority_queue_create(size_t initial_size) {
    carbon_prefix_tree_node_priority_queue * queue
            = malloc(sizeof(carbon_prefix_tree_node_priority_queue));

    queue->count = 0;
    queue->size = initial_size;
    queue->min_size = initial_size;
    queue->heap_array = malloc(sizeof(carbon_prefix_tree_node_with_prefix *) * queue->size);
    return queue;
}


void carbon_prefix_node_priority_queue_free(
        carbon_prefix_tree_node_priority_queue *queue
)
{
    free(queue->heap_array);
    free(queue);
}


void carbon_prefix_tree_node_priority_queue_push(
        carbon_prefix_tree_node_priority_queue *queue,
        carbon_prefix_tree_node_with_prefix *element
) {
    size_t index, parent;

    if(queue->count >= queue->size)
    {
        queue->size *= 2;
        queue->heap_array = realloc(queue->heap_array, sizeof(carbon_prefix_tree_node_with_prefix *) * queue->size);
    }

    index = queue->count++;

    for(;index; index = parent)
    {
        parent = (index - 1) / 2;
        if (queue->heap_array[parent]->node->count >= element->node->count)
            break;

        queue->heap_array[index] = queue->heap_array[parent];
    }

    queue->heap_array[index] = element;
}

carbon_prefix_tree_node_with_prefix *carbon_prefix_tree_node_priority_queue_pop(
        carbon_prefix_tree_node_priority_queue *queue
) {
    if( queue->count == 0 )
        return 0;

    carbon_prefix_tree_node_with_prefix * removed;
    carbon_prefix_tree_node_with_prefix * temp = queue->heap_array[--queue->count];

    if ((queue->count <= (queue->size / 2)) && (queue->size > queue->min_size))
    {
        queue->size /= 2;
        queue->heap_array = realloc(queue->heap_array, sizeof(carbon_prefix_tree_node_with_prefix *) * queue->size);
    }

    removed = queue->heap_array[0];
    queue->heap_array[0] = temp;
    util_max_heapify(queue->heap_array, 0, queue->count);
    return removed;
}

carbon_prefix_tree_node_with_prefix * carbon_prefix_tree_node_with_prefix_create(
        carbon_prefix_tree_node *node,
        char *prefix,
        size_t prefix_length,
        size_t prefix_capacity
) {
    carbon_prefix_tree_node_with_prefix *  pair =
            malloc(sizeof(carbon_prefix_tree_node_with_prefix));

    pair->node = node;
    pair->prefix = prefix;
    pair->prefix_length = prefix_length;
    pair->prefix_capacity = prefix_capacity;

    return pair;
}

void carbon_prefix_tree_node_with_prefix_free(
        carbon_prefix_tree_node_with_prefix *node_with_prefix
)
{
    free(node_with_prefix);
}
