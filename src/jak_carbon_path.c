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

#include <jak_carbon_path.h>
#include <jak_carbon_find.h>
#include <jak_carbon_revise.h>
#include "jak_carbon_path.h"

static inline jak_carbon_path_status_e traverse_column(jak_carbon_path_evaluator *state,
                                                      const jak_carbon_dot_path *path, jak_u32 current_path_pos,
                                                      jak_carbon_column_it *it);

static inline jak_carbon_path_status_e traverse_array(jak_carbon_path_evaluator *state,
                                                     const jak_carbon_dot_path *path, jak_u32 current_path_pos,
                                                     jak_carbon_array_it *it, bool is_record);

bool jak_carbon_path_evaluator_begin(jak_carbon_path_evaluator *eval, jak_carbon_dot_path *path,
                                 jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(eval)
        JAK_ERROR_IF_NULL(path)
        JAK_ERROR_IF_NULL(doc)

        JAK_zero_memory(eval, sizeof(jak_carbon_path_evaluator));
        eval->doc = doc;
        JAK_check_success(error_init(&eval->err));
        JAK_check_success(jak_carbon_iterator_open(&eval->root_it, eval->doc));
        eval->status = traverse_array(eval, path, 0, &eval->root_it, true);
        JAK_check_success(jak_carbon_iterator_close(&eval->root_it));
        return true;
}

bool jak_carbon_path_evaluator_begin_mutable(jak_carbon_path_evaluator *eval, const jak_carbon_dot_path *path,
                                         struct jak_carbon_revise *context)
{
        JAK_ERROR_IF_NULL(eval)
        JAK_ERROR_IF_NULL(path)
        JAK_ERROR_IF_NULL(context)

        eval->doc = context->revised_doc;
        JAK_check_success(error_init(&eval->err));
        JAK_check_success(carbon_revise_iterator_open(&eval->root_it, context));
        eval->status = traverse_array(eval, path, 0, &eval->root_it, true);
        JAK_check_success(jak_carbon_iterator_close(&eval->root_it));
        return true;
}

bool jak_carbon_path_evaluator_status(jak_carbon_path_status_e *status, jak_carbon_path_evaluator *state)
{
        JAK_ERROR_IF_NULL(status)
        JAK_ERROR_IF_NULL(state)
        *status = state->status;
        return true;
}

bool jak_carbon_path_evaluator_has_result(jak_carbon_path_evaluator *state)
{
        JAK_ERROR_IF_NULL(state)
        return state->status == JAK_CARBON_PATH_RESOLVED;
}

bool jak_carbon_path_evaluator_end(jak_carbon_path_evaluator *state)
{
        JAK_ERROR_IF_NULL(state)
        switch (state->result.container_type) {
                case JAK_CARBON_OBJECT:
                        jak_carbon_object_it_drop(&state->result.containers.object.it);
                        break;
                case JAK_CARBON_ARRAY:
                        jak_carbon_array_it_drop(&state->result.containers.array.it);
                        break;
                case JAK_CARBON_COLUMN:
                        break;
                default: error_print(JAK_ERR_NOTIMPLEMENTED);
        }
        return true;
}

bool jak_carbon_path_exists(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        bool result = jak_carbon_find_open(&find, path, doc);
        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_array(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_array(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_column(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_column(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_object(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_object(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_container(jak_carbon *doc, const char *path)
{
        return (jak_carbon_path_is_array(doc, path) || jak_carbon_path_is_column(doc, path) ||
                jak_carbon_path_is_object(doc, path));
}

bool jak_carbon_path_is_null(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_null(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_number(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_number(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_boolean(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_boolean(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

bool jak_carbon_path_is_string(jak_carbon *doc, const char *path)
{
        jak_carbon_find find;
        jak_carbon_field_type_e field_type;
        bool result = false;

        if (jak_carbon_find_open(&find, path, doc)) {
                jak_carbon_find_result_type(&field_type, &find);
                result = jak_carbon_field_type_is_string(field_type);
        }

        jak_carbon_find_close(&find);
        return result;
}

static inline jak_carbon_path_status_e traverse_object(jak_carbon_path_evaluator *state,
                                                      const jak_carbon_dot_path *path, jak_u32 current_path_pos,
                                                      jak_carbon_object_it *it)
{
        carbon_dot_node_e node_type;
        jak_u32 path_length;
        bool status;

        jak_carbon_dot_path_type_at(&node_type, current_path_pos, path);
        JAK_ASSERT(node_type == JAK_DOT_NODE_KEY_NAME);

        status = jak_carbon_object_it_next(it);
        jak_carbon_dot_path_len(&path_length, path);
        const char *needle = jak_carbon_dot_path_key_at(current_path_pos, path);
        jak_u64 needle_len = strlen(needle);
        jak_u32 next_path_pos = current_path_pos + 1;

        if (!status) {
                /* empty document */
                return JAK_CARBON_PATH_EMPTY_DOC;
        } else {
                jak_u64 key_len;
                do {
                        const char *key_name = jak_carbon_object_it_prop_name(&key_len, it);
                        if (key_len == needle_len && strncmp(key_name, needle, needle_len) == 0) {
                                if (next_path_pos == path_length) {
                                        state->result.container_type = JAK_CARBON_OBJECT;
                                        jak_carbon_object_it_clone(&state->result.containers.object.it, it);
                                        return JAK_CARBON_PATH_RESOLVED;
                                } else {
                                        /* path end not reached, traverse further if possible */
                                        JAK_ASSERT(next_path_pos < path_length);

                                        jak_carbon_field_type_e prop_type;
                                        jak_carbon_object_it_prop_type(&prop_type, it);

                                        if (!jak_carbon_field_type_is_traversable(prop_type)) {
                                                return JAK_CARBON_PATH_NOTTRAVERSABLE;
                                        } else {
                                                JAK_ASSERT(prop_type == JAK_CARBON_FIELD_TYPE_OBJECT ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_ARRAY ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                                                           prop_type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                                                switch (prop_type) {
                                                        case JAK_CARBON_FIELD_TYPE_OBJECT: {
                                                                jak_carbon_object_it *sub_it = jak_carbon_object_it_object_value(
                                                                        it);
                                                                jak_carbon_path_status_e ret = traverse_object(state,
                                                                                                              path,
                                                                                                              next_path_pos,
                                                                                                              sub_it);
                                                                jak_carbon_object_it_drop(sub_it);
                                                                return ret;
                                                        }
                                                        case JAK_CARBON_FIELD_TYPE_ARRAY: {
                                                                jak_carbon_array_it *sub_it = jak_carbon_object_it_array_value(
                                                                        it);
                                                                jak_carbon_path_status_e ret = traverse_array(state,
                                                                                                             path,
                                                                                                             next_path_pos,
                                                                                                             sub_it,
                                                                                                             false);
                                                                jak_carbon_array_it_drop(sub_it);
                                                                return ret;
                                                        }
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                                                        case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                                                jak_carbon_column_it *sub_it = jak_carbon_object_it_column_value(
                                                                        it);
                                                                return traverse_column(state,
                                                                                       path,
                                                                                       next_path_pos,
                                                                                       sub_it);
                                                        }
                                                        default: error(&it->err, JAK_ERR_UNSUPPORTEDTYPE)
                                                                return JAK_CARBON_PATH_INTERNAL;
                                                }
                                        }
                                }
                        }
                } while (jak_carbon_object_it_next(it));
        }

        return JAK_CARBON_PATH_NOSUCHKEY;
}

static inline jak_carbon_path_status_e traverse_array(jak_carbon_path_evaluator *state,
                                                     const jak_carbon_dot_path *path, jak_u32 current_path_pos,
                                                     jak_carbon_array_it *it, bool is_record)
{
        JAK_ASSERT(state);
        JAK_ASSERT(path);
        JAK_ASSERT(it);
        JAK_ASSERT(current_path_pos < path->path_len);

        jak_carbon_field_type_e elem_type;
        carbon_dot_node_e node_type;
        jak_u32 path_length;
        jak_carbon_path_status_e status;
        jak_u32 requested_array_idx;
        jak_u32 current_array_idx = 0;
        bool is_unit_array = jak_carbon_array_it_is_unit(it);

        jak_carbon_dot_path_type_at(&node_type, current_path_pos, path);

        jak_carbon_dot_path_len(&path_length, path);

        if (!jak_carbon_array_it_next(it)) {
                /* empty document */
                return JAK_CARBON_PATH_EMPTY_DOC;
        } else {
                switch (node_type) {
                        case JAK_DOT_NODE_ARRAY_IDX:
                                jak_carbon_dot_path_idx_at(&requested_array_idx, current_path_pos, path);
                                while (current_array_idx < requested_array_idx &&
                                       jak_carbon_array_it_next(it)) { current_array_idx++; }
                                JAK_ASSERT(current_array_idx <= requested_array_idx);
                                if (current_array_idx != requested_array_idx) {
                                        /* root array has too less elements to reach the requested index */
                                        return JAK_CARBON_PATH_NOSUCHINDEX;
                                } else {
                                        /* requested index is reached; depending on the subsequent path, lookup may stops */
                                        jak_carbon_array_it_field_type(&elem_type, it);
                                        jak_u32 next_path_pos = current_path_pos + 1;
                                        if (is_unit_array && is_record && jak_carbon_field_type_is_column(elem_type)) {
                                                jak_carbon_column_it *sub_it = jak_carbon_array_it_column_value(
                                                        it);
                                                return traverse_column(state,
                                                                       path,
                                                                       next_path_pos,
                                                                       sub_it);
                                        } else {
                                                if (next_path_pos < path_length) {
                                                        /* path must be further evaluated in the next step, which requires a container
                                                         * type (for traversability) */
                                                        carbon_dot_node_e next_node_type;
                                                        jak_carbon_dot_path_type_at(&next_node_type, next_path_pos, path);
                                                        if (!jak_carbon_field_type_is_traversable(elem_type)) {
                                                                /* the array element is not a container; path evaluation stops here */
                                                                return JAK_CARBON_PATH_NOTTRAVERSABLE;
                                                        } else {
                                                                /* array element is traversable */
                                                                switch (next_node_type) {
                                                                        case JAK_DOT_NODE_ARRAY_IDX:
                                                                                /* next node in path is an array index which requires that
                                                                                 * the current array element is an array or column */
                                                                                if (elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_ARRAY &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_U8 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_U16 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_U32 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_U64 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_I8 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_I16 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_I32 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_I64 &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                                                                                    elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN) {
                                                                                        return JAK_CARBON_PATH_NOCONTAINER;
                                                                                } else {
                                                                                        if (elem_type ==
                                                                                            JAK_CARBON_FIELD_TYPE_ARRAY) {
                                                                                                jak_carbon_array_it *sub_it = jak_carbon_array_it_array_value(
                                                                                                        it);
                                                                                                status = traverse_array(
                                                                                                        state,
                                                                                                        path,
                                                                                                        next_path_pos,
                                                                                                        sub_it, false);
                                                                                                jak_carbon_array_it_drop(
                                                                                                        sub_it);
                                                                                                return status;
                                                                                        } else {
                                                                                                JAK_ASSERT(elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                                                                                                           elem_type ==
                                                                                                           JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                                                                                                jak_carbon_column_it *sub_it = jak_carbon_array_it_column_value(
                                                                                                        it);
                                                                                                return traverse_column(
                                                                                                        state,
                                                                                                        path,
                                                                                                        next_path_pos,
                                                                                                        sub_it);
                                                                                        }
                                                                                }
                                                                        case JAK_DOT_NODE_KEY_NAME:
                                                                                /* next node in path is a key name which requires that
                                                                                 * the current array element is of type object */
                                                                                if (elem_type !=
                                                                                    JAK_CARBON_FIELD_TYPE_OBJECT) {
                                                                                        return JAK_CARBON_PATH_NOTANOBJECT;
                                                                                } else {
                                                                                        jak_carbon_object_it *sub_it = jak_carbon_array_it_object_value(
                                                                                                it);
                                                                                        status = traverse_object(state,
                                                                                                                 path,
                                                                                                                 next_path_pos,
                                                                                                                 sub_it);
                                                                                        jak_carbon_object_it_drop(sub_it);
                                                                                        return status;
                                                                                }
                                                                        default: error_print(JAK_ERR_INTERNALERR);
                                                                                return JAK_CARBON_PATH_INTERNAL;
                                                                }
                                                        }
                                                } else {
                                                        /* path end is reached */
                                                        state->result.container_type = JAK_CARBON_ARRAY;
                                                        jak_carbon_array_it_clone(&state->result.containers.array.it, it);
                                                        return JAK_CARBON_PATH_RESOLVED;
                                                }
                                        }
                                }
                        case JAK_DOT_NODE_KEY_NAME:
                                /* first array element exists, which must be of type object */
                                jak_carbon_array_it_field_type(&elem_type, it);
                                if (elem_type != JAK_CARBON_FIELD_TYPE_OBJECT) {
                                        /* first array element is not of type object and a key lookup cannot
                                         * be executed, consequentially */
                                        return JAK_CARBON_PATH_NOTANOBJECT;
                                } else {
                                        /* next node in path is a key name which requires that
                                                                         * the current array element is of type object */
                                        if (elem_type != JAK_CARBON_FIELD_TYPE_OBJECT) {
                                                return JAK_CARBON_PATH_NOTANOBJECT;
                                        } else {
                                                if (is_unit_array && is_record) {
                                                        jak_carbon_object_it *sub_it = jak_carbon_array_it_object_value(
                                                                it);
                                                        status = traverse_object(state,
                                                                                 path,
                                                                                 current_path_pos,
                                                                                 sub_it);
                                                        jak_carbon_object_it_drop(sub_it);
                                                        return status;
                                                } else {
                                                        return JAK_CARBON_PATH_NOSUCHKEY;
                                                }
                                        }
                                }
                                break;
                        default: error(&((jak_carbon_dot_path *) path)->err, JAK_ERR_INTERNALERR);
                                return JAK_CARBON_PATH_INTERNAL;
                }
        }
}

static inline jak_carbon_path_status_e traverse_column(jak_carbon_path_evaluator *state,
                                                      const jak_carbon_dot_path *path, jak_u32 current_path_pos,
                                                      jak_carbon_column_it *it)
{
        jak_u32 total_path_len;
        jak_u32 requested_idx;
        jak_u32 nun_values_contained;
        carbon_dot_node_e node_type;
        jak_carbon_field_type_e column_type;
        jak_carbon_dot_path_len(&total_path_len, path);
        if (current_path_pos + 1 != total_path_len) {
                /* a column cannot contain further containers; since the current path node is not
                 * the last one, traversal cannot be continued */
                return JAK_CARBON_PATH_NONESTING;
        } else {
                jak_carbon_dot_path_type_at(&node_type, current_path_pos, path);
                JAK_ASSERT(node_type == JAK_DOT_NODE_ARRAY_IDX);
                jak_carbon_dot_path_idx_at(&requested_idx, current_path_pos, path);
                jak_carbon_column_it_values_info(&column_type, &nun_values_contained, it);
                if (requested_idx >= nun_values_contained) {
                        /* requested index does not exists in this column */
                        return JAK_CARBON_PATH_NOSUCHINDEX;
                } else {
                        state->result.container_type = JAK_CARBON_COLUMN;
                        jak_carbon_column_it_clone(&state->result.containers.column.it, it);
                        state->result.containers.column.elem_pos = requested_idx;
                        return JAK_CARBON_PATH_RESOLVED;
                }
        }
}