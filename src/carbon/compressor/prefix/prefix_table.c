#include <malloc.h>
#include <string.h>

#include <carbon/compressor/prefix/prefix_table.h>
#include <carbon/compressor/prefix/prefix_read_only_tree.h>
#include <carbon/compressor/prefix/prefix_node_priority_queue.h>

carbon_prefix_table* carbon_prefix_table_create() {
    carbon_prefix_table * table = malloc(sizeof(carbon_prefix_table));
    table->capacity = 256;
    table->table = malloc(sizeof(char *) * table->capacity);
    table->table[ 0 ] = malloc(3);
    table->table[ 0 ][0] = 0;
    table->table[ 0 ][1] = 0;
    table->table[ 0 ][2] = 0;
    table->next_entry = 1;

    return table;
}

size_t carbon_prefix_table_length(
    carbon_prefix_table * table
) {
    return table->next_entry == 0 ? 256 * 256 : table->next_entry;
}


void carbon_prefix_table_free(
    carbon_prefix_table * table
)
{
    for (size_t i = 0; i < carbon_prefix_table_length(table);++i) {
        free(table->table[i]);
    }

    free(table->table);
    free(table);
}


uint16_t carbon_prefix_table_add(
    carbon_prefix_table * table,
    char *prefix
) {
    if(table->next_entry == table->capacity) {
        table->capacity = (size_t) (2 * table->capacity);
        table->table = realloc(table->table, sizeof(char *) * table->capacity);
    }

    size_t length = strlen(prefix + 2);
    table->table[table->next_entry] = malloc(length + 3);
    table->table[table->next_entry][length + 2] = 0;
    memcpy(table->table[table->next_entry], prefix, length + 2);

    // Yeah, we could rely on overflowing the uint8_t... but it's not good practice
    if(table->next_entry == 256 * 256 - 1) {
        return table->next_entry = 0;
    } else {
        return table->next_entry++;
    }
}

void carbon_prefix_table_resolve(
    carbon_prefix_table * table,
    char *encoded, char **buffer,
    size_t *buffer_size, size_t *position
) {

    if(!encoded[ 0 ] && !encoded[ 1 ]) {
        *position = 0;
    } else {
        carbon_prefix_table_resolve(table, table->table[(uint8_t) encoded[0] * 256 + (uint8_t) encoded[1]], buffer, buffer_size, position);
    }

    size_t length = strlen(encoded + 2);
    if( *buffer_size < *position + length + 2 ) {
        *buffer = realloc(*buffer, *position + length + 2);
        *buffer_size = *position + length + 2;
    }

    strncpy(*buffer + *position, encoded + 2, length);
    *position += length;
    (*buffer)[*position] = 0;
}

carbon_prefix_ro_tree * carbon_prefix_table_to_encoder_tree(
    carbon_prefix_table * table
) {
    carbon_prefix_ro_tree * tree = carbon_prefix_ro_tree_create(2);

    char * resolve_buffer = malloc(10);
    size_t resolve_buffer_len = 10;
    size_t prefix_length = 0;

    for(size_t i = 0; i < carbon_prefix_table_length(table) ; ++i) {
        carbon_prefix_table_resolve(table, table->table[i], &resolve_buffer, &resolve_buffer_len, &prefix_length);
        carbon_prefix_ro_tree_add_prefix(tree, resolve_buffer, prefix_length, i);
    }

    free(resolve_buffer);
    return tree;
}

void carbon_prefix_tree_encode_all_with_queue(
    carbon_prefix_tree_node * node,
    carbon_prefix_table * prefix_table
) {
    carbon_prefix_tree_node_priority_queue * queue =
            carbon_prefix_node_priority_queue_create(64);

    {
        carbon_prefix_tree_child_list * child_item = node->children;

        while(child_item) {
            char * empty_prefix = malloc(4);
            empty_prefix[0] = 0;
            empty_prefix[1] = 0;
            empty_prefix[2] = 0;
            carbon_prefix_tree_node_priority_queue_push(
                queue, carbon_prefix_tree_node_with_prefix_create(child_item->child, empty_prefix, 2, 4)
            );

            child_item = child_item->next;
        }
    }

    carbon_prefix_tree_node_with_prefix * node_with_prefix;
    while( ( node_with_prefix = carbon_prefix_tree_node_priority_queue_pop( queue ) ) ) {

        // Extend the prefix
        if( node_with_prefix->prefix_length + 3 >= node_with_prefix->prefix_capacity ) {
            node_with_prefix->prefix = realloc(node_with_prefix->prefix, 2 * node_with_prefix->prefix_capacity);
            node_with_prefix->prefix_capacity = 2 * node_with_prefix->prefix_capacity;
        }

        node_with_prefix->prefix[node_with_prefix->prefix_length] = node_with_prefix->node->value;

        // No children...
        if(!node_with_prefix->node->children) {
            node_with_prefix->prefix[++(node_with_prefix->prefix_length)] = 0;

            uint16_t prefix_id = carbon_prefix_table_add(prefix_table, node_with_prefix->prefix);
            free(node_with_prefix->prefix);
            carbon_prefix_tree_node_with_prefix_free(node_with_prefix);

            if( !prefix_id ) break;
            continue;
        }

        // Exactly one child
        if(!node_with_prefix->node->children->next) {

            carbon_prefix_tree_node_priority_queue_push(
                queue, carbon_prefix_tree_node_with_prefix_create(
                            node_with_prefix->node->children->child,
                            node_with_prefix->prefix,
                            node_with_prefix->prefix_length + 1,
                            node_with_prefix->prefix_capacity
                )
            );

            carbon_prefix_tree_node_with_prefix_free(node_with_prefix);
            continue;
        }

        char * prefix;
        size_t prefix_length = 0;

        // Very simple cost function if adding the current node is useful
        if( (node_with_prefix->prefix_length + 1) > 3 ) {
            node_with_prefix->prefix[node_with_prefix->prefix_length + 1] = 0;
            uint16_t prefix_id = carbon_prefix_table_add(prefix_table, node_with_prefix->prefix);

            if( !prefix_id ) {
                free(node_with_prefix->prefix);
                carbon_prefix_tree_node_with_prefix_free(node_with_prefix);
                break;
            }

            prefix = node_with_prefix->prefix;
            prefix[0] = (char)(prefix_id >> 8);
            prefix[1] = (char)(prefix_id & 0xFF);
            prefix[2] = 0;
            prefix_length = 2;
        } else {
            prefix = node_with_prefix->prefix;
            prefix_length = node_with_prefix->prefix_length + 1;
        }

        carbon_prefix_tree_child_list * child_item = node_with_prefix->node->children;
        while(child_item) {
            char * prefix_copy = malloc(node_with_prefix->prefix_capacity);
            memcpy(prefix_copy, prefix, prefix_length);

            carbon_prefix_tree_node_priority_queue_push(
                queue, carbon_prefix_tree_node_with_prefix_create(
                    child_item->child, prefix_copy, prefix_length, node_with_prefix->prefix_capacity
                )
            );

            child_item = child_item->next;
        }

        free(node_with_prefix->prefix);
        carbon_prefix_tree_node_with_prefix_free(node_with_prefix);
    }

    while( ( node_with_prefix = carbon_prefix_tree_node_priority_queue_pop( queue ) ) ) {
        free(node_with_prefix->prefix);
        carbon_prefix_tree_node_with_prefix_free(node_with_prefix);
    }

    carbon_prefix_node_priority_queue_free(queue);
}
