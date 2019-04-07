#ifndef CARBON_PREFIX_READ_ONLY_TREE_H
#define CARBON_PREFIX_READ_ONLY_TREE_H

#include <stdint.h>

#include "prefix_tree_node.h"
#include "prefix_table.h"

struct carbon_prefix_ro_tree_t;

typedef struct carbon_prefix_ro_tree_child_t {
    char * value;
    size_t value_length;
    struct carbon_prefix_ro_tree_t * child;
} carbon_prefix_ro_tree_child;

typedef struct carbon_prefix_ro_tree_t {
    uint16_t prefix;

    size_t num_children;
    size_t max_children;
    carbon_prefix_ro_tree_child * children;
} carbon_prefix_ro_tree;

carbon_prefix_ro_tree * carbon_prefix_ro_tree_create(size_t max_children);

void carbon_prefix_ro_tree_free(carbon_prefix_ro_tree * tree);

carbon_prefix_ro_tree * carbon_prefix_ro_tree_child_add(
        carbon_prefix_ro_tree * parent, char * value
);

void carbon_prefix_ro_tree_add_prefix(
    carbon_prefix_ro_tree * tree,
    char * resolved_prefix,
    size_t prefix_length,
    uint16_t prefix_id
);

uint16_t carbon_prefix_ro_tree_max_prefix(
    carbon_prefix_ro_tree * tree,
    char * string,
    size_t * prefix_length
);

#endif
