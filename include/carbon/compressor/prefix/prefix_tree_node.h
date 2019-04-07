#ifndef CARBON_PREFIX_TREE_NODE_H
#define CARBON_PREFIX_TREE_NODE_H

#include <glob.h>

typedef struct carbon_prefix_tree_child_list_t {
    struct carbon_prefix_tree_node_t * child;
    struct carbon_prefix_tree_child_list_t * next;
} carbon_prefix_tree_child_list;

typedef struct carbon_prefix_tree_node_t {
    char value;
    size_t count;

    carbon_prefix_tree_child_list *children;
} carbon_prefix_tree_node;

carbon_prefix_tree_node * carbon_prefix_tree_node_create(char value);

void carbon_prefix_tree_node_free(carbon_prefix_tree_node ** node);

carbon_prefix_tree_node * carbon_prefix_tree_node_add_child(
        carbon_prefix_tree_node * parent,
        carbon_prefix_tree_child_list *last_child_of_parent,
        char value
);

void carbon_prefix_tree_node_remove_child(
        carbon_prefix_tree_node * parent,
        carbon_prefix_tree_node * child
);

carbon_prefix_tree_node * carbon_prefix_tree_node_child(
        carbon_prefix_tree_node *node,
        carbon_prefix_tree_child_list **last_item,
        char value
);

size_t carbon_prefix_tree_child_list_length(carbon_prefix_tree_child_list * list);

carbon_prefix_tree_node ** carbon_prefix_tree_child_list_to_array(
        carbon_prefix_tree_child_list * list,
        size_t * length
);

size_t carbon_prefix_tree_node_sum(carbon_prefix_tree_node * node);

void carbon_prefix_tree_node_print(carbon_prefix_tree_node *node, int level);

void carbon_prefix_tree_node_add_string(
    carbon_prefix_tree_node * tree,
    char const *string,
    size_t max_new_nodes
);

carbon_prefix_tree_node* carbon_prefix_tree_node_prune(
    carbon_prefix_tree_node * parent,
    size_t min_support
);

void carbon_prefix_tree_calculate_savings(
    carbon_prefix_tree_node * node,
    size_t costs_for_split
);

#endif //CARBON_PREFIX_TREE_NODE_H
