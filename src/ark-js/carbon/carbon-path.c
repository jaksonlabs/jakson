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
#include "carbon-path.h"

static inline enum carbon_path_status traverse_column(struct carbon_path_evaluator *state,
                                                      const struct carbon_dot_path *path, u32 current_path_pos,
                                                      struct carbon_column_it *it);

static inline enum carbon_path_status traverse_array(struct carbon_path_evaluator *state,
                                                     const struct carbon_dot_path *path, u32 current_path_pos,
                                                     struct carbon_array_it *it, bool is_record);

bool carbon_path_evaluator_begin(struct carbon_path_evaluator *eval, struct carbon_dot_path *path,
                                 struct carbon *doc)
{
        error_if_null(eval)
        error_if_null(path)
        error_if_null(doc)

        ark_zero_memory(eval, sizeof(struct carbon_path_evaluator));
        eval->doc = doc;
        ark_check_success(error_init(&eval->err));
        ark_check_success(carbon_iterator_open(&eval->root_it, eval->doc));
        eval->status = traverse_array(eval, path, 0, &eval->root_it, true);
        ark_check_success(carbon_iterator_close(&eval->root_it));
        return true;
}

bool carbon_path_evaluator_begin_mutable(struct carbon_path_evaluator *eval, const struct carbon_dot_path *path,
                                         struct carbon_revise *context)
{
        error_if_null(eval)
        error_if_null(path)
        error_if_null(context)

        eval->doc = context->revised_doc;
        ark_check_success(error_init(&eval->err));
        ark_check_success(carbon_revise_iterator_open(&eval->root_it, context));
        eval->status = traverse_array(eval, path, 0, &eval->root_it, true);
        ark_check_success(carbon_iterator_close(&eval->root_it));
        return true;
}

bool carbon_path_evaluator_status(enum carbon_path_status *status, struct carbon_path_evaluator *state)
{
        error_if_null(status)
        error_if_null(state)
        *status = state->status;
        return true;
}

bool carbon_path_evaluator_has_result(struct carbon_path_evaluator *state)
{
        error_if_null(state)
        return state->status == CARBON_PATH_RESOLVED;
}

bool carbon_path_evaluator_end(struct carbon_path_evaluator *state)
{
        error_if_null(state)
        switch (state->result.container_type) {
                case CARBON_OBJECT:
                        carbon_object_it_drop(&state->result.containers.object.it);
                        break;
                case CARBON_ARRAY:
                        carbon_array_it_drop(&state->result.containers.array.it);
                        break;
                case CARBON_COLUMN:
                        break;
                default: error_print(ARK_ERR_NOTIMPLEMENTED);
        }
        return true;
}

bool carbon_path_exists(struct carbon *doc, const char *path)
{
        struct carbon_find find;
        bool result = carbon_find_open(&find, path, doc);
        carbon_find_close(&find);
        return result;
}

bool carbon_path_is_array(struct carbon *doc, const char *path)
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

bool carbon_path_is_column(struct carbon *doc, const char *path)
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

bool carbon_path_is_object(struct carbon *doc, const char *path)
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

bool carbon_path_is_container(struct carbon *doc, const char *path)
{
        return (carbon_path_is_array(doc, path) || carbon_path_is_column(doc, path) ||
                carbon_path_is_object(doc, path));
}

bool carbon_path_is_null(struct carbon *doc, const char *path)
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

bool carbon_path_is_number(struct carbon *doc, const char *path)
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

bool carbon_path_is_boolean(struct carbon *doc, const char *path)
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

bool carbon_path_is_string(struct carbon *doc, const char *path)
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

static inline enum carbon_path_status traverse_object(struct carbon_path_evaluator *state,
                                                     const struct carbon_dot_path *path, u32 current_path_pos,
                                                     struct carbon_object_it *it)
{
        enum carbon_dot_node_type node_type;
        u32 path_length;
        bool status;

        carbon_dot_path_type_at(&node_type, current_path_pos, path);
        assert(node_type == DOT_NODE_KEY_NAME);

        status = carbon_object_it_next(it);
        carbon_dot_path_len(&path_length, path);
        const char *needle = carbon_dot_path_key_at(current_path_pos, path);
        u64 needle_len = strlen(needle);
        u32 next_path_pos = current_path_pos + 1;

        if (!status) {
                /* empty document */
                return CARBON_PATH_EMPTY_DOC;
        } else {
                u64 key_len;
                do {
                        const char *key_name = carbon_object_it_prop_name(&key_len, it);
                        if (key_len == needle_len && strncmp(key_name, needle, needle_len) == 0) {
                                if (next_path_pos == path_length) {
                                        state->result.container_type = CARBON_OBJECT;
                                        carbon_object_it_clone(&state->result.containers.object.it, it);
                                        return CARBON_PATH_RESOLVED;
                                } else {
                                        /* path end not reached, traverse further if possible */
                                        assert(next_path_pos < path_length);

                                        enum carbon_field_type prop_type;
                                        carbon_object_it_prop_type(&prop_type, it);

                                        if(!carbon_field_type_is_traversable(prop_type)) {
                                                return CARBON_PATH_NOTTRAVERSABLE;
                                        } else {
                                                assert(prop_type == CARBON_FIELD_TYPE_OBJECT ||
                                                               prop_type == CARBON_FIELD_TYPE_ARRAY ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_U8 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_U16 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_U32 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_U64 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_I8 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_I16 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_I32 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_I64 ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                                                               prop_type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                                                switch (prop_type) {
                                                        case CARBON_FIELD_TYPE_OBJECT: {
                                                                struct carbon_object_it *sub_it = carbon_object_it_object_value(
                                                                        it);
                                                                enum carbon_path_status ret = traverse_object(state,
                                                                                         path,
                                                                                         next_path_pos,
                                                                                         sub_it);
                                                                carbon_object_it_drop(sub_it);
                                                                return ret;
                                                        }
                                                        case CARBON_FIELD_TYPE_ARRAY: {
                                                                struct carbon_array_it *sub_it = carbon_object_it_array_value(
                                                                        it);
                                                                enum carbon_path_status ret = traverse_array(state,
                                                                                        path,
                                                                                        next_path_pos,
                                                                                        sub_it, false);
                                                                carbon_array_it_drop(sub_it);
                                                                return ret;
                                                        }
                                                        case CARBON_FIELD_TYPE_COLUMN_U8:
                                                        case CARBON_FIELD_TYPE_COLUMN_U16:
                                                        case CARBON_FIELD_TYPE_COLUMN_U32:
                                                        case CARBON_FIELD_TYPE_COLUMN_U64:
                                                        case CARBON_FIELD_TYPE_COLUMN_I8:
                                                        case CARBON_FIELD_TYPE_COLUMN_I16:
                                                        case CARBON_FIELD_TYPE_COLUMN_I32:
                                                        case CARBON_FIELD_TYPE_COLUMN_I64:
                                                        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                                                        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                                                struct carbon_column_it *sub_it = carbon_object_it_column_value(
                                                                        it);
                                                                return traverse_column(state,
                                                                                       path,
                                                                                       next_path_pos,
                                                                                       sub_it);
                                                        }
                                                        default:
                                                        error(&it->err, ARK_ERR_UNSUPPORTEDTYPE)
                                                                return CARBON_PATH_INTERNAL;
                                                }
                                        }
                                }
                        }
                } while (carbon_object_it_next(it));
        }

        return CARBON_PATH_NOSUCHKEY;
}

static inline enum carbon_path_status traverse_array(struct carbon_path_evaluator *state,
                                                     const struct carbon_dot_path *path, u32 current_path_pos,
                                                     struct carbon_array_it *it, bool is_record)
{
        assert(state);
        assert(path);
        assert(it);
        assert(current_path_pos < path->path_len);

        enum carbon_field_type elem_type;
        enum carbon_dot_node_type node_type;
        u32 path_length;
        enum carbon_path_status status;
        u32 requested_array_idx;
        u32 current_array_idx = 0;
        bool is_unit_array = carbon_array_it_is_unit(it);

        carbon_dot_path_type_at(&node_type, current_path_pos, path);

        carbon_dot_path_len(&path_length, path);

        if (!carbon_array_it_next(it)) {
                /* empty document */
                return CARBON_PATH_EMPTY_DOC;
        } else {
                switch (node_type) {
                        case DOT_NODE_ARRAY_IDX:
                                carbon_dot_path_idx_at(&requested_array_idx, current_path_pos, path);
                                while (current_array_idx < requested_array_idx &&
                                       carbon_array_it_next(it)) { current_array_idx++; }
                                assert(current_array_idx <= requested_array_idx);
                                if (current_array_idx != requested_array_idx) {
                                        /* root array has too less elements to reach the requested index */
                                        return CARBON_PATH_NOSUCHINDEX;
                                } else {
                                        /* requested index is reached; depending on the subsequent path, lookup may stops */
                                        carbon_array_it_field_type(&elem_type, it);
                                        u32 next_path_pos = current_path_pos + 1;
                                        if (is_unit_array && is_record && carbon_field_type_is_column(elem_type)) {
                                                struct carbon_column_it *sub_it = carbon_array_it_column_value(
                                                        it);
                                                return traverse_column(state,
                                                                       path,
                                                                       next_path_pos,
                                                                       sub_it);
                                        } else {
                                                if (next_path_pos < path_length) {
                                                        /* path must be further evaluated in the next step, which requires a container
                                                         * type (for traversability) */
                                                        enum carbon_dot_node_type next_node_type;
                                                        carbon_dot_path_type_at(&next_node_type, next_path_pos, path);
                                                        if (!carbon_field_type_is_traversable(elem_type)) {
                                                                /* the array element is not a container; path evaluation stops here */
                                                                return CARBON_PATH_NOTTRAVERSABLE;
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
                                                                                    elem_type !=
                                                                                    CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                                                                                    elem_type !=
                                                                                    CARBON_FIELD_TYPE_COLUMN_BOOLEAN) {
                                                                                        return CARBON_PATH_NOCONTAINER;
                                                                                } else {
                                                                                        if (elem_type ==
                                                                                            CARBON_FIELD_TYPE_ARRAY) {
                                                                                                struct carbon_array_it *sub_it = carbon_array_it_array_value(
                                                                                                        it);
                                                                                                status = traverse_array(state,
                                                                                                                        path,
                                                                                                                        next_path_pos,
                                                                                                                        sub_it, false);
                                                                                                carbon_array_it_drop(sub_it);
                                                                                                return status;
                                                                                        } else {
                                                                                                assert(elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_U8 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_U16 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_U32 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_U64 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_I8 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_I16 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_I32 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_I64 ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                                                                                                       elem_type ==
                                                                                                       CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                                                                                                struct carbon_column_it *sub_it = carbon_array_it_column_value(
                                                                                                        it);
                                                                                                return traverse_column(state,
                                                                                                                       path,
                                                                                                                       next_path_pos,
                                                                                                                       sub_it);
                                                                                        }
                                                                                }
                                                                        case DOT_NODE_KEY_NAME:
                                                                                /* next node in path is a key name which requires that
                                                                                 * the current array element is of type object */
                                                                                if (elem_type != CARBON_FIELD_TYPE_OBJECT) {
                                                                                        return CARBON_PATH_NOTANOBJECT;
                                                                                } else {
                                                                                        struct carbon_object_it *sub_it = carbon_array_it_object_value(
                                                                                                it);
                                                                                        status = traverse_object(state,
                                                                                                                 path,
                                                                                                                 next_path_pos,
                                                                                                                 sub_it);
                                                                                        carbon_object_it_drop(sub_it);
                                                                                        return status;
                                                                                }
                                                                        default: error_print(ARK_ERR_INTERNALERR);
                                                                                return CARBON_PATH_INTERNAL;
                                                                }
                                                        }
                                                } else {
                                                        /* path end is reached */
                                                        state->result.container_type = CARBON_ARRAY;
                                                        carbon_array_it_clone(&state->result.containers.array.it, it);
                                                        return CARBON_PATH_RESOLVED;
                                                }
                                        }
                                }
                        case DOT_NODE_KEY_NAME:
                                /* first array element exists, which must be of type object */
                                carbon_array_it_field_type(&elem_type, it);
                                if (elem_type != CARBON_FIELD_TYPE_OBJECT) {
                                        /* first array element is not of type object and a key lookup cannot
                                         * be executed, consequentially */
                                        return CARBON_PATH_NOTANOBJECT;
                                } else {
                                        /* next node in path is a key name which requires that
                                                                         * the current array element is of type object */
                                        if (elem_type != CARBON_FIELD_TYPE_OBJECT) {
                                                return CARBON_PATH_NOTANOBJECT;
                                        } else {
                                                if (is_unit_array && is_record) {
                                                        struct carbon_object_it *sub_it = carbon_array_it_object_value(
                                                                it);
                                                        status = traverse_object(state,
                                                                                 path,
                                                                                 current_path_pos,
                                                                                 sub_it);
                                                        carbon_object_it_drop(sub_it);
                                                        return status;
                                                } else {
                                                        return CARBON_PATH_NOSUCHKEY;
                                                }
                                        }
                                }
                                break;
                        default: error(&((struct carbon_dot_path *) path)->err, ARK_ERR_INTERNALERR);
                                return CARBON_PATH_INTERNAL;
                }
        }
}

static inline enum carbon_path_status traverse_column(struct carbon_path_evaluator *state,
                                                      const struct carbon_dot_path *path, u32 current_path_pos,
                                                      struct carbon_column_it *it)
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
                return CARBON_PATH_NONESTING;
        } else {
                carbon_dot_path_type_at(&node_type, current_path_pos, path);
                assert(node_type == DOT_NODE_ARRAY_IDX);
                carbon_dot_path_idx_at(&requested_idx, current_path_pos, path);
                carbon_column_it_values_info(&column_type, &nun_values_contained, it);
                if (requested_idx >= nun_values_contained) {
                        /* requested index does not exists in this column */
                        return CARBON_PATH_NOSUCHINDEX;
                } else {
                        state->result.container_type = CARBON_COLUMN;
                        carbon_column_it_clone(&state->result.containers.column.it, it);
                        state->result.containers.column.elem_pos = requested_idx;
                        return CARBON_PATH_RESOLVED;
                }
        }
}