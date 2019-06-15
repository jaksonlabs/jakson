#include <stdlib.h>
#include <stdio.h>

#include <carbon/compressor/prefix/prefix_tree_node.h>

carbon_prefix_tree_node *carbon_prefix_tree_node_create(char value) {
    carbon_prefix_tree_node * new_node = malloc(sizeof(carbon_prefix_tree_node));
    new_node->value = value;
    new_node->count = 0;
    new_node->children = 0;

    return new_node;
}

void carbon_prefix_tree_node_free(carbon_prefix_tree_node **node) {
    carbon_prefix_tree_child_list * child_list_item = (*node)->children;

    while( child_list_item ) {
        carbon_prefix_tree_child_list * next = child_list_item->next;
        carbon_prefix_tree_node_free(&child_list_item->child);

        free(child_list_item);
        child_list_item = next;
    }

    free(*node);
}

carbon_prefix_tree_node *carbon_prefix_tree_node_add_child(
        carbon_prefix_tree_node *parent,
        carbon_prefix_tree_child_list *last_child_of_parent,
        char value
) {
    carbon_prefix_tree_child_list * list_item = malloc(sizeof(carbon_prefix_tree_child_list));

    list_item->next = 0;
    list_item->child = carbon_prefix_tree_node_create( value );

    if( last_child_of_parent )
        last_child_of_parent->next = list_item;
    else
        parent->children = list_item;

    return list_item->child;
}

void carbon_prefix_tree_node_remove_child(
        carbon_prefix_tree_node *parent,
        carbon_prefix_tree_node *child
) {
    carbon_prefix_tree_child_list * previous_item = 0;
    carbon_prefix_tree_child_list * current_item = parent->children;

    while( current_item ) {
        if( current_item->child == child ) {
            if( previous_item ) {
                previous_item->next = current_item->next;
            } else {
                parent->children = current_item->next;
            }

            free(current_item);
            carbon_prefix_tree_node_free(&child);
            return;
        }

        previous_item = current_item;
        current_item = current_item->next;
    }
}

carbon_prefix_tree_node *carbon_prefix_tree_node_child(
        carbon_prefix_tree_node *node,
        carbon_prefix_tree_child_list **last_item,
        char value
) {
    carbon_prefix_tree_child_list * item = node->children;
    *last_item = 0;

    while( item ) {
        if( item->child->value == value )
            return item->child;

        *last_item = item;
        item = item->next;
    }

    return 0;
}

size_t carbon_prefix_tree_child_list_length(carbon_prefix_tree_child_list *list) {
    size_t child_count = 0;
    while( list ) {
        ++child_count;
        list = list->next;
    }

    return child_count;
}

carbon_prefix_tree_node ** carbon_prefix_tree_child_list_to_array(
        carbon_prefix_tree_child_list *list,
        size_t *length
) {
    *length = carbon_prefix_tree_child_list_length(list);

    carbon_prefix_tree_node ** child_array = malloc(sizeof(carbon_prefix_tree_node *) * *length);
    for(size_t i = 0; i < *length; ++i, list = list->next) {
        child_array[ i ] = list->child;
    }

    return child_array;
}

void carbon_prefix_tree_node_print(
        carbon_prefix_tree_node *node,
        int level
) {
    for(int i =0; i < level; ++i)
        putchar(' ');
    printf("%c: %zu\n", node->value, node->count);

    carbon_prefix_tree_child_list * item = node->children;
    while( item ) {
        carbon_prefix_tree_node_print(item->child, level + 1);
        item = item->next;
    }
}

size_t carbon_prefix_tree_node_sum(carbon_prefix_tree_node *node) {
    size_t sum = (node->value ? node->count : 0);

    carbon_prefix_tree_child_list * item = node->children;
    while( item ) {
        sum += carbon_prefix_tree_node_sum(item->child);
        item = item->next;
    }

    return sum;
}


void carbon_prefix_tree_node_add_string(
    carbon_prefix_tree_node * tree,
    char const *string,
    size_t max_new_nodes
) {
    carbon_prefix_tree_child_list * last_sibling = 0;
    carbon_prefix_tree_node * current = tree;

    while(*string && max_new_nodes) {
        carbon_prefix_tree_node * child = carbon_prefix_tree_node_child(current, &last_sibling, *string);

        if(!child) {
            child = carbon_prefix_tree_node_add_child(current, last_sibling, *string);
            --max_new_nodes;
        }

        child->count++;
        string++;
        current = child;
    }
}

carbon_prefix_tree_node* carbon_prefix_tree_node_prune(
    carbon_prefix_tree_node * parent,
    size_t min_support
) {
    carbon_prefix_tree_child_list * previous_item = 0;
    carbon_prefix_tree_child_list * current_item = parent->children;

    while( current_item ) {
        if( current_item->child->count < min_support ) {
            if( previous_item ) {
                previous_item->next = current_item->next;
            } else {
                parent->children = current_item->next;
            }

            carbon_prefix_tree_child_list * tmp = current_item->next;
            carbon_prefix_tree_node_free(&current_item->child);
            free(current_item);
            current_item = tmp;
        } else {
            carbon_prefix_tree_node_prune(current_item->child, min_support);

            previous_item = current_item;
            current_item = current_item->next;
        }
    }

    return 0;
}

void carbon_prefix_tree_calculate_savings(
    carbon_prefix_tree_node * node,
    size_t costs_for_split
) {
    // No children...
    if(!node->children) {
        --node->count;
        return;
    }

    // Exactly one child
    if(!node->children->next) {
        carbon_prefix_tree_calculate_savings(node->children->child, costs_for_split);
        node->count += node->children->child->count - 1;
        return;
    }

    size_t savings = 0;
    size_t splits  = 0;
    size_t max_saving = 0;

    carbon_prefix_tree_node       * max_child = 0;
    carbon_prefix_tree_child_list * child_item = node->children;
    while( child_item ) {
        carbon_prefix_tree_calculate_savings(child_item->child, costs_for_split);

        size_t saving = child_item->child->count;
        if( saving > costs_for_split ) {
            if( saving > max_saving ) {
                max_saving = saving;
                max_child  = child_item->child;
            }

            savings += saving - costs_for_split;
            ++splits;

            child_item = child_item->next;
        } else {
            carbon_prefix_tree_child_list * tmp = child_item->next;
            carbon_prefix_tree_node_remove_child(node, child_item->child);

            child_item = tmp;
        }

    }

    if( savings <= max_saving )  {
        node->count += max_saving;

        child_item = node->children;
        while( child_item ) {
            if( child_item->child != max_child ) {
                carbon_prefix_tree_child_list * tmp = child_item->next;
                carbon_prefix_tree_node_remove_child(node, child_item->child);
                child_item = tmp;
            } else {
                child_item = child_item->next;
            }
        }
    }
}
