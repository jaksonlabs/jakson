#include <carbon/compressor/prefix/prefix_read_only_tree.h>

#include <stdlib.h>
#include <string.h>

carbon_prefix_ro_tree * carbon_prefix_ro_tree_create(size_t max_children) {
    carbon_prefix_ro_tree * ro_node =
            malloc(sizeof(carbon_prefix_ro_tree));

    ro_node->prefix = 0;
    ro_node->num_children = 0;
    ro_node->max_children = max_children;
    ro_node->children = malloc(max_children * sizeof(carbon_prefix_ro_tree_child));

    return ro_node;
}

carbon_prefix_ro_tree * carbon_prefix_ro_tree_child_add(
    carbon_prefix_ro_tree * parent,
    char * value
) {

    if( parent->max_children == parent->num_children ) {
        parent->max_children += 1;
        parent->children = realloc(parent->children, sizeof(carbon_prefix_ro_tree_child) * parent->max_children);
    }

    size_t value_len = strlen(value);

    parent->children[parent->num_children].value_length = value_len;

    if(value_len >= sizeof(char *)) {
        parent->children[parent->num_children].value = malloc(value_len + 1);
        strncpy(parent->children[parent->num_children].value, value, value_len);
        parent->children[parent->num_children].value[value_len] = 0;
    } else {
        strncpy((char *)&parent->children[parent->num_children].value, value, value_len);
        ((char *)(&parent->children[parent->num_children].value))[value_len] = 0;
    }

    return parent->children[parent->num_children++].child = carbon_prefix_ro_tree_create(1);
}

void carbon_prefix_ro_tree_add_prefix(
    carbon_prefix_ro_tree *tree,
    char * resolved_prefix, size_t prefix_length, uint16_t prefix_id
) {
    carbon_prefix_ro_tree * current_node = tree;

    size_t   input_length = prefix_length;
    size_t   num_chars_processed = 0;
    char * c = resolved_prefix;
    while(*c) {
        size_t idx = 0;
        for(;idx < current_node->num_children; ++idx) {
            carbon_prefix_ro_tree_child * child = &current_node->children[idx];
            char * child_prefix = child->value_length < sizeof(char *) ? (char *)&child->value : child->value;
            if(child->value_length <= input_length - num_chars_processed && strncmp(child_prefix, c, child->value_length) == 0) {
                break;
            }
        }

        if(idx < current_node->num_children) {
            num_chars_processed += current_node->children[idx].value_length;
            c = resolved_prefix + num_chars_processed;
            current_node = current_node->children[idx].child;
        } else {
            current_node = carbon_prefix_ro_tree_child_add(current_node, c);
            break;
        }

    }

    current_node->prefix = prefix_id;
}

uint16_t carbon_prefix_ro_tree_max_prefix(
    carbon_prefix_ro_tree *tree,
    char *string,
    size_t *prefix_length
) {
    *prefix_length = 0;

    uint16_t max_prefix = 0;
    size_t   input_length = strlen(string);
    size_t   num_chars_processed = 0;
    char * c = string;

    carbon_prefix_ro_tree * current_node = tree;

    while(*c) {
        size_t idx = 0;
        for(;idx < current_node->num_children; ++idx) {
            carbon_prefix_ro_tree_child * child = &current_node->children[idx];
            char * child_prefix = child->value_length < sizeof(char *) ? (char *)&child->value : child->value;
            if(child->value_length <= input_length - num_chars_processed && strncmp(child_prefix, c, child->value_length) == 0) {
                break;
            }
        }

        if(idx < current_node->num_children) {
            num_chars_processed += current_node->children[idx].value_length;
            current_node = current_node->children[idx].child;
            max_prefix = current_node->prefix;
            *prefix_length = num_chars_processed;
            c = string + num_chars_processed;
        } else {
            return max_prefix;
        }

    }

    return max_prefix;
}

void carbon_prefix_ro_tree_free(
    carbon_prefix_ro_tree *tree
) {
    for(size_t i = 0; i < tree->num_children; ++i) {
        if(tree->children[i].value_length >= sizeof(char *))
            free(tree->children[i].value);
        carbon_prefix_ro_tree_free(tree->children[i].child);
    }

    free(tree->children);
    free(tree);
}
