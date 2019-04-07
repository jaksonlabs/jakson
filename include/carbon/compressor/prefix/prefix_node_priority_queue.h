#ifndef CARBON_PREFIX_NODE_PRIORITY_QUEUE_H
#define CARBON_PREFIX_NODE_PRIORITY_QUEUE_H

#include "prefix_tree_node.h"

typedef struct carbon_prefix_tree_node_with_prefix_t {
    struct carbon_prefix_tree_node_t * node;
    char * prefix;
    size_t prefix_length;
    size_t prefix_capacity;
} carbon_prefix_tree_node_with_prefix;

typedef struct carbon_prefix_tree_node_priority_queue_t {
    size_t size;
    size_t count;
    size_t min_size;
    carbon_prefix_tree_node_with_prefix **heap_array;
} carbon_prefix_tree_node_priority_queue;

carbon_prefix_tree_node_with_prefix * carbon_prefix_tree_node_with_prefix_create(
        struct carbon_prefix_tree_node_t * node,
        char * prefix,
        size_t prefix_length,
        size_t prefix_capacity
);

void carbon_prefix_tree_node_with_prefix_free(
        carbon_prefix_tree_node_with_prefix * node_with_prefix
);

carbon_prefix_tree_node_priority_queue * carbon_prefix_node_priority_queue_create(size_t initial_size);

void carbon_prefix_node_priority_queue_free(
        carbon_prefix_tree_node_priority_queue * queue
);

void carbon_prefix_tree_node_priority_queue_push(
        carbon_prefix_tree_node_priority_queue *queue,
        carbon_prefix_tree_node_with_prefix * element
);

carbon_prefix_tree_node_with_prefix * carbon_prefix_tree_node_priority_queue_pop(
        carbon_prefix_tree_node_priority_queue *queue
);

#endif //CARBON_PREFIX_NODE_PRIORITY_QUEUE_H
