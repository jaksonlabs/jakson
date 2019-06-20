/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "core/bison/bison-path.h"

static inline enum bison_path_status traverse_column(struct bison_path_evaluator *state, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_column_it *it);

static inline enum bison_path_status traverse_array(struct bison_path_evaluator *state, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_array_it *it);


NG5_EXPORT(bool) bison_path_evaluator_begin(struct bison_path_evaluator *eval, struct bison_dot_path *path,
        struct bison *doc)
{
        error_if_null(eval)
        error_if_null(path)
        error_if_null(doc)

        eval->doc = doc;
        ng5_check_success(error_init(&eval->err));
        ng5_check_success(bison_iterator_open(&eval->root_it, eval->doc));
        eval->status = traverse_array(eval, path, 0, &eval->root_it);
        return true;

}

NG5_EXPORT(bool) bison_path_evaluator_status(enum bison_path_status *status, struct bison_path_evaluator *state)
{
        error_if_null(status)
        error_if_null(state)
        *status = state->status;
        return true;
}

NG5_EXPORT(bool) bison_path_evaluator_has_result(struct bison_path_evaluator *state)
{
        error_if_null(state)
        return state->status == BISON_PATH_RESOLVED;
}

NG5_EXPORT(bool) bison_path_evaluator_end(struct bison_path_evaluator *state)
{
        error_if_null(state)
        bison_iterator_close(&state->root_it);
        return true;
}

static inline enum bison_path_status traverse_array(struct bison_path_evaluator *state, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_array_it *it)
{
        assert(state);
        assert(path);
        assert(it);
        assert(current_path_pos < path->path_len);

        enum bison_field_type elem_type;
        enum bison_dot_node_type node_type;
        u32 path_length;
        bool status;
        u32 requested_array_idx;
        u32 current_array_idx = 0;

        bison_dot_path_type_at(&node_type, current_path_pos, path);
        status = bison_array_it_next(it);

        bison_dot_path_len(&path_length, path);

        if (!status) {
                /* empty document */
                return BISON_PATH_EMPTY_DOC;
        } else {
                switch (node_type) {
                case DOT_NODE_ARRAY_IDX:
                        bison_dot_path_idx_at(&requested_array_idx, current_path_pos, path);
                        while (current_array_idx < requested_array_idx && bison_array_it_next(it))
                        { current_array_idx++; }
                        assert(current_array_idx <= requested_array_idx);
                        if (current_array_idx != requested_array_idx) {
                                /* root array has too less elements to reach the requested index */
                                return BISON_PATH_NOSUCHINDEX;
                        } else {
                                /* requested index is reached; depending on the subsequent path, lookup may stops */
                                bison_array_it_field_type(&elem_type, it);
                                u32 next_path_pos = current_path_pos + 1;
                                if (next_path_pos < path_length) {
                                        /* path must be further evaluated in the next step, which requires a container
                                         * type (for traversability) */
                                        enum bison_dot_node_type next_node_type;
                                        bison_dot_path_type_at(&next_node_type, next_path_pos, path);
                                        if (!bison_field_type_is_traversable(elem_type)) {
                                                /* the array element is not a container; path evaluation stops here */
                                                return BISON_PATH_NOTTRAVERSABLE;
                                        } else {
                                                /* array element is traversable */
                                                switch (next_node_type) {
                                                case DOT_NODE_ARRAY_IDX:
                                                        /* next node in path is an array index which requires that
                                                         * the current array element is an array or column */
                                                        if (elem_type != BISON_FIELD_TYPE_ARRAY &&
                                                                elem_type != BISON_FIELD_TYPE_COLUMN) {
                                                                return BISON_PATH_NOCONTAINER;
                                                        } else {
                                                                if (elem_type == BISON_FIELD_TYPE_ARRAY) {
                                                                        struct bison_array_it *sub_it = bison_array_it_array_value(it);
                                                                        status = traverse_array(state, path, next_path_pos, sub_it);
                                                                        return status;
                                                                } else {
                                                                        assert(elem_type == BISON_FIELD_TYPE_COLUMN);
                                                                        struct bison_column_it *sub_it = bison_array_it_column_value(it);
                                                                        return traverse_column(state, path, next_path_pos, sub_it);
                                                                }
                                                        }
                                                case DOT_NODE_KEY_NAME:
                                                        /* next node in path is a key name which requires that
                                                         * the current array element is of type object */
                                                        if (elem_type != BISON_FIELD_TYPE_OBJECT) {
                                                                return BISON_PATH_NOTANOBJECT;
                                                        } else {
                                                                error_print_and_die_if(true, NG5_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                                                return BISON_PATH_INTERNAL;
                                                        }
                                                default:
                                                error_print(NG5_ERR_INTERNALERR);
                                                        return BISON_PATH_INTERNAL;
                                                }
                                        }
                                } else {
                                        /* path end is reached */
                                        state->result.container_type = BISON_ARRAY;
                                        state->result.containers.array.it = it;
                                        return BISON_PATH_RESOLVED;
                                }
                        }
                case DOT_NODE_KEY_NAME:
                        /* first array element exists, which must be of type object */
                        bison_array_it_field_type(&elem_type, it);
                        if (elem_type != BISON_FIELD_TYPE_OBJECT) {
                                /* first array element is not of type object and a key lookup cannot
                                 * be executed, consequentially */
                                return BISON_PATH_NOTANOBJECT;
                        } else {
                                error_print_and_die_if(true, NG5_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                return BISON_PATH_INTERNAL;
                        }
                        break;
                default:
                error(&path->err, NG5_ERR_INTERNALERR);
                        return  BISON_PATH_INTERNAL;
                }
        }
}

static inline enum bison_path_status traverse_column(struct bison_path_evaluator *state, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_column_it *it)
{
        u32 total_path_len;
        u32 requested_idx;
        u32 nun_values_contained;
        enum bison_dot_node_type node_type;
        enum bison_field_type column_type;
        bison_dot_path_len(&total_path_len, path);
        if (current_path_pos + 1 != total_path_len) {
                /* a column cannot contain further containers; since the current path node is not
                 * the last one, traversal cannot be continued */
                return BISON_PATH_NONESTING;
        } else {
                bison_dot_path_type_at(&node_type, current_path_pos, path);
                assert(node_type == DOT_NODE_ARRAY_IDX);
                bison_dot_path_idx_at(&requested_idx, current_path_pos, path);
                bison_column_it_values_info(&column_type, &nun_values_contained, it);
                if (requested_idx >= nun_values_contained) {
                        /* requested index does not exists in this column */
                        return BISON_PATH_NOSUCHINDEX;
                } else {
                        state->result.container_type = BISON_COLUMN;
                        state->result.containers.column.it = it;
                        state->result.containers.column.elem_pos = requested_idx;
                        return BISON_PATH_RESOLVED;
                }
        }
}