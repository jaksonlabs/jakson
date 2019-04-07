#ifndef CARBON_PREFIX_TABLE_H
#define CARBON_PREFIX_TABLE_H

#include <glob.h>
#include <stdint.h>

struct carbon_prefix_ro_tree_t;
typedef struct carbon_prefix_ro_tree_t carbon_prefix_ro_tree;

struct carbon_prefix_tree_node_t;
typedef struct carbon_prefix_tree_node_t carbon_prefix_tree_node;

typedef struct carbon_prefix_table_t {
    char ** table;
    uint16_t next_entry;
    size_t   capacity;
} carbon_prefix_table;

carbon_prefix_table * carbon_prefix_table_create();
size_t carbon_prefix_table_length(carbon_prefix_table * table);
void carbon_prefix_table_free(carbon_prefix_table * table);

uint16_t carbon_prefix_table_add(
    carbon_prefix_table * table,
    char * prefix
);

void carbon_prefix_table_resolve(
    carbon_prefix_table * table,
    char *encoded, char ** buffer,
    size_t *buffer_size, size_t * position
);

carbon_prefix_ro_tree * carbon_prefix_table_to_encoder_tree(carbon_prefix_table * table);

void carbon_prefix_tree_encode_all_with_queue(
    carbon_prefix_tree_node * node,
    carbon_prefix_table * prefix_table
);

#endif //CARBON_PREFIX_TABLE_H
