/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <ark-js/carbon/carbon-path.h>
#include <ark-js/carbon/carbon-find.h>
#include <ark-js/carbon/carbon-revise.h>

static inline enum carbon_path_status traverse_column(struct carbon_path_evaluator *state,
        const struct carbon_dot_path *path, u32 current_path_pos, struct carbon_column_it *it);

static inline enum carbon_path_status traverse_array(struct carbon_path_evaluator *state,
        const struct carbon_dot_path *path, u32 current_path_pos, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_path_evaluator_begin(struct carbon_path_evaluator *eval, struct carbon_dot_path *path,
        struct carbon *doc)
{
        error_if_null(eval)
        error_if_null(path)
        error_if_null(doc)

        eval->doc = doc;
        ark_check_success(error_init(&eval->err));
        ark_check_success(carbon_iterator_open(&eval->root_it, eval->doc));
        eval->status = traverse_array(eval, path, 0, &eval->root_it);
        return true;
}

ARK_EXPORT(bool) carbon_path_evaluator_begin_mutable(struct carbon_path_evaluator *eval, const struct carbon_dot_path *path,
        struct carbon_revise *context)
{
        error_if_null(eval)
        error_if_null(path)
        error_if_null(context)

        eval->doc = context->revised_doc;
        ark_check_success(error_init(&eval->err));
        ark_check_success(carbon_revise_iterator_open(&eval->root_it, context));
        eval->status = traverse_array(eval, path, 0, &eval->root_it);
        return true;
}

ARK_EXPORT(bool) carbon_path_evaluator_status(enum carbon_path_status *status, struct carbon_path_evaluator *state)
{
        error_if_null(status)
        error_if_null(state)
        *status = state->status;
        return true;
}

ARK_EXPORT(bool) carbon_path_evaluator_has_result(struct carbon_path_evaluator *state)
{
        error_if_null(state)
        return state->status == carbon_PATH_RESOLVED;
}

ARK_EXPORT(bool) carbon_path_evaluator_end(struct carbon_path_evaluator *state)
{
        error_if_null(state)
        carbon_iterator_close(&state->root_it);
        return true;
}

ARK_EXPORT(bool) carbon_path_exists(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        bool result = carbon_find_open(&find, path, doc);
        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_array(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_array(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_column(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_column(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_object(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_object(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_container(struct carbon *doc, const char *path)
{
        return (carbon_path_is_array(doc, path) || carbon_path_is_column(doc, path) || carbon_path_is_object(doc, path));
}

ARK_EXPORT(bool) carbon_path_is_null(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_null(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_number(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_number(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_boolean(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_boolean(field_type);
        }

        carbon_find_close(&find);
        return result;
}

ARK_EXPORT(bool) carbon_path_is_string(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        enum carbon_field_type field_type;
        bool result = false;

        if (carbon_find_open(&find, path, doc)) {
                carbon_find_result_type(&field_type, &find);
                result = carbon_field_type_is_string(field_type);
        }

        carbon_find_close(&find);
        return result;
}

static inline enum carbon_path_status traverse_array(struct carbon_path_evaluator *state,
        const struct carbon_dot_path *path, u32 current_path_pos, struct carbon_array_it *it)
{
        assert(state);
        assert(path);
        assert(it);
        assert(current_path_pos < path->path_len);

        enum carbon_field_type elem_type;
        enum carbon_dot_node_type node_type;
        u32 path_length;
        bool status;
        u32 requested_array_idx;
        u32 current_array_idx = 0;

        carbon_dot_path_type_at(&node_type, current_path_pos, path);
        status = carbon_array_it_next(it);

        carbon_dot_path_len(&path_length, path);

        if (!status) {
                /* empty document */
                return carbon_PATH_EMPTY_DOC;
        } else {
                switch (node_type) {
                case DOT_NODE_ARRAY_IDX:
                        carbon_dot_path_idx_at(&requested_array_idx, current_path_pos, path);
                        while (current_array_idx < requested_array_idx && carbon_array_it_next(it))
                        { current_array_idx++; }
                        assert(current_array_idx <= requested_array_idx);
                        if (current_array_idx != requested_array_idx) {
                                /* root array has too less elements to reach the requested index */
                                return carbon_PATH_NOSUCHINDEX;
                        } else {
                                /* requested index is reached; depending on the subsequent path, lookup may stops */
                                carbon_array_it_field_type(&elem_type, it);
                                u32 next_path_pos = current_path_pos + 1;
                                if (next_path_pos < path_length) {
                                        /* path must be further evaluated in the next step, which requires a container
                                         * type (for traversability) */
                                        enum carbon_dot_node_type next_node_type;
                                        carbon_dot_path_type_at(&next_node_type, next_path_pos, path);
                                        if (!carbon_field_type_is_traversable(elem_type)) {
                                                /* the array element is not a container; path evaluation stops here */
                                                return carbon_PATH_NOTTRAVERSABLE;
                                        } else {
                                                /* array element is traversable */
                                                switch (next_node_type) {
                                                case DOT_NODE_ARRAY_IDX:
                                                        /* next node in path is an array index which requires that
                                                         * the current array element is an array or column */
                                                        if (elem_type != CARBON_FIELD_TYPE_ARRAY &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_U8 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_U16 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_U32 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_U64 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_I8 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_I16 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_I32 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_I64 &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                                                                elem_type != CARBON_FIELD_TYPE_COLUMN_BOOLEAN) {
                                                                return carbon_PATH_NOCONTAINER;
                                                        } else {
                                                                if (elem_type == CARBON_FIELD_TYPE_ARRAY) {
                                                                        struct carbon_array_it *sub_it = carbon_array_it_array_value(it);
                                                                        status = traverse_array(state, path, next_path_pos, sub_it);
                                                                        return status;
                                                                } else {
                                                                        assert(elem_type == CARBON_FIELD_TYPE_COLUMN_U8 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_U16 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_U32 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_U64 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_I8 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_I16 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_I32 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_I64 ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                                                                                elem_type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                                                                        struct carbon_column_it *sub_it = carbon_array_it_column_value(it);
                                                                        return traverse_column(state, path, next_path_pos, sub_it);
                                                                }
                                                        }
                                                case DOT_NODE_KEY_NAME:
                                                        /* next node in path is a key name which requires that
                                                         * the current array element is of type object */
                                                        if (elem_type != CARBON_FIELD_TYPE_OBJECT) {
                                                                return carbon_PATH_NOTANOBJECT;
                                                        } else {
                                                                error_print_and_die_if(true, ARK_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                                                return carbon_PATH_INTERNAL;
                                                        }
                                                default:
                                                error_print(ARK_ERR_INTERNALERR);
                                                        return carbon_PATH_INTERNAL;
                                                }
                                        }
                                } else {
                                        /* path end is reached */
                                        state->result.container_type = CARBON_ARRAY;
                                        state->result.containers.array.it = it;
                                        return carbon_PATH_RESOLVED;
                                }
                        }
                case DOT_NODE_KEY_NAME:
                        /* first array element exists, which must be of type object */
                        carbon_array_it_field_type(&elem_type, it);
                        if (elem_type != CARBON_FIELD_TYPE_OBJECT) {
                                /* first array element is not of type object and a key lookup cannot
                                 * be executed, consequentially */
                                return carbon_PATH_NOTANOBJECT;
                        } else {
                                error_print_and_die_if(true, ARK_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                return carbon_PATH_INTERNAL;
                        }
                        break;
                default:
                error(&((struct carbon_dot_path *)path)->err, ARK_ERR_INTERNALERR);
                        return  carbon_PATH_INTERNAL;
                }
        }
}

static inline enum carbon_path_status traverse_column(struct carbon_path_evaluator *state,
        const struct carbon_dot_path *path, u32 current_path_pos, struct carbon_column_it *it)
{
        u32 total_path_len;
        u32 requested_idx;
        u32 nun_values_contained;
        enum carbon_dot_node_type node_type;
        enum carbon_field_type column_type;
        carbon_dot_path_len(&total_path_len, path);
        if (current_path_pos + 1 != total_path_len) {
                /* a column cannot contain further containers; since the current path node is not
                 * the last one, traversal cannot be continued */
                return carbon_PATH_NONESTING;
        } else {
                carbon_dot_path_type_at(&node_type, current_path_pos, path);
                assert(node_type == DOT_NODE_ARRAY_IDX);
                carbon_dot_path_idx_at(&requested_idx, current_path_pos, path);
                carbon_column_it_values_info(&column_type, &nun_values_contained, it);
                if (requested_idx >= nun_values_contained) {
                        /* requested index does not exists in this column */
                        return carbon_PATH_NOSUCHINDEX;
                } else {
                        state->result.container_type = CARBON_COLUMN;
                        state->result.containers.column.it = it;
                        state->result.containers.column.elem_pos = requested_idx;
                        return carbon_PATH_RESOLVED;
                }
        }
}