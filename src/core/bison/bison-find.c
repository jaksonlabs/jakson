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

#include <core/bison/bison-find.h>
#include "core/bison/bison-find.h"

static inline bool traverse_array(struct bison_find *find, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_array_it *it);
static inline bool traverse_column(struct bison_find *find, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_column_it *it);
static bool find_by_path(struct bison_find *find, struct bison_dot_path *path, struct bison *doc);

NG5_EXPORT(bool) bison_find_create(struct bison_find *find, struct bison_dot_path *path, struct bison *doc)
{
        error_if_null(find)
        error_if_null(path)
        error_if_null(doc)

        error_init(&find->err);
        find->has_result = false;
        find->cleanup_needed = false;
        find->doc = doc;

        if (!bison_dot_path_is_empty(path)) {
                find->cleanup_needed = true;
                find->has_result = find_by_path(find, path, doc);
        }

        return find->has_result;
}

NG5_EXPORT(bool) bison_find_has_result(struct bison_find *find)
{
        error_if_null(find)
        return find->has_result;
}

NG5_EXPORT(bool) bison_find_result_type(enum bison_field_type *type, struct bison_find *find)
{
        error_if_null(type)
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        *type = find->type;
        return true;
}

NG5_EXPORT(struct bison_array_it *) bison_find_result_array(struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_ARRAY, &find->err, NG5_ERR_TYPEMISMATCH)
        return find->value.array_it;
}

NG5_EXPORT(struct bison_column_it *) bison_find_result_column(struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_COLUMN, &find->err, NG5_ERR_TYPEMISMATCH)
        return find->value.column_it;
}

NG5_EXPORT(bool) bison_find_result_boolean(bool *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_boolean(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.boolean;
        return true;
}

NG5_EXPORT(bool) bison_find_result_unsigned(u64 *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_unsigned_integer(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.unsigned_number;
        return true;
}

NG5_EXPORT(bool) bison_find_result_signed(i64 *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_signed_integer(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.signed_number;
        return true;
}

NG5_EXPORT(bool) bison_find_result_float(float *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_floating_number(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.float_number;
        return true;
}

NG5_EXPORT(const char *) bison_find_result_string(u64 *str_len, struct bison_find *find)
{
        error_if_null(find)
        error_if_null(str_len)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_STRING, &find->err, NG5_ERR_TYPEMISMATCH)
        *str_len = find->value.string.len;
        return find->value.string.base;
}

NG5_EXPORT(struct bison_binary *) bison_find_result_binary(struct bison_find *find)
{
        error_if_null(find)
        error_if(!find->has_result, &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_binary(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        return &find->value.binary;
}

NG5_EXPORT(bool) bison_find_drop(struct bison_find *find)
{
        error_if_null(find)
        if (find->cleanup_needed) {
                bison_iterator_close(&find->root_it);
        }

        return true;
}

static void access_field_value(struct bison_find *find, struct bison_array_it *it, enum bison_field_type elem_type)
{
        assert ((it->it_field_type == elem_type) && (elem_type == find->type));
        switch (elem_type) {
                case BISON_FIELD_TYPE_NULL:
                case BISON_FIELD_TYPE_TRUE:
                case BISON_FIELD_TYPE_FALSE:
                        /* no value to be stored */
                        break;
                case BISON_FIELD_TYPE_ARRAY:
                        find->value.array_it = bison_array_it_array_value(it);
                        break;
                case BISON_FIELD_TYPE_COLUMN:
                        find->value.column_it = bison_array_it_column_value(it);
                        break;
                case BISON_FIELD_TYPE_OBJECT:
                        error_print_and_die_if(true, NG5_ERR_NOTIMPLEMENTED); /* TODO: implement this for objects */
                        break;
                case BISON_FIELD_TYPE_STRING:
                        find->value.string.base = bison_array_it_string_value(&find->value.string.len, it);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8:
                case BISON_FIELD_TYPE_NUMBER_U16:
                case BISON_FIELD_TYPE_NUMBER_U32:
                case BISON_FIELD_TYPE_NUMBER_U64:
                        bison_array_it_unsigned_value(&find->number_is_nulled, &find->value.unsigned_number, it);
                        break;
                case BISON_FIELD_TYPE_NUMBER_I8:
                case BISON_FIELD_TYPE_NUMBER_I16:
                case BISON_FIELD_TYPE_NUMBER_I32:
                case BISON_FIELD_TYPE_NUMBER_I64:
                        bison_array_it_signed_value(&find->number_is_nulled, &find->value.signed_number, it);
                        break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT:
                        bison_array_it_float_value(&find->number_is_nulled, &find->value.float_number, it);
                        break;
                case BISON_FIELD_TYPE_BINARY:
                case BISON_FIELD_TYPE_BINARY_CUSTOM:
                        bison_array_it_binary_value(&find->value.binary, it);
                        break;
                default:
                        error(&find->err, NG5_ERR_INTERNALERR);
                        break;
        }
}

static inline bool traverse_array(struct bison_find *find, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_array_it *it)
{
        assert(find);
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
                return false;
        } else {
                switch (node_type) {
                case DOT_NODE_ARRAY_IDX:
                        bison_dot_path_idx_at(&requested_array_idx, current_path_pos, path);
                        while (current_array_idx < requested_array_idx && bison_array_it_next(it))
                                { current_array_idx++; }
                        assert(current_array_idx <= requested_array_idx);
                        if (current_array_idx != requested_array_idx) {
                                /* root array has too less elements to reach the requested index */
                                return false;
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
                                                return false;
                                        } else {
                                                /* array element is traversable */
                                                switch (next_node_type) {
                                                case DOT_NODE_ARRAY_IDX:
                                                        /* next node in path is an array index which requires that
                                                         * the current array element is an array or column */
                                                        if (elem_type != BISON_FIELD_TYPE_ARRAY &&
                                                                elem_type != BISON_FIELD_TYPE_COLUMN) {
                                                                return false;
                                                        } else {
                                                                if (elem_type == BISON_FIELD_TYPE_ARRAY) {
                                                                        struct bison_array_it *sub_it = bison_array_it_array_value(it);
                                                                        status = traverse_array(find, path, next_path_pos, sub_it);
                                                                        bison_int_array_it_auto_close(sub_it);
                                                                        return status;
                                                                } else {
                                                                        assert(elem_type == BISON_FIELD_TYPE_COLUMN);
                                                                        struct bison_column_it *sub_it = bison_array_it_column_value(it);
                                                                        return traverse_column(find, path, next_path_pos, sub_it);
                                                                }
                                                        }
                                                case DOT_NODE_KEY_NAME:
                                                        /* next node in path is a key name which requires that
                                                         * the current array element is of type object */
                                                        if (elem_type != BISON_FIELD_TYPE_OBJECT) {
                                                                return false;
                                                        } else {
                                                                error_print_and_die_if(true, NG5_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                                                return false;
                                                        }
                                                default:
                                                error(&find->err, NG5_ERR_INTERNALERR);
                                                        return false;
                                                }
                                        }
                                } else {
                                        /* path end is reached */
                                        find->type = elem_type;
                                        access_field_value(find, it, elem_type);
                                        return true;
                                }
                        }
                case DOT_NODE_KEY_NAME:
                        /* first array element exists, which must be of type object */
                        bison_array_it_field_type(&elem_type, it);
                        if (elem_type != BISON_FIELD_TYPE_OBJECT) {
                                /* first array element is not of type object and a key lookup cannot
                                 * be executed, consequentially */
                                return false;
                        } else {
                                error_print_and_die_if(true, NG5_ERR_NOTIMPLEMENTED) /* TODO: implement for objects */
                                return false;
                        }
                        break;
                default:
                error(&path->err, NG5_ERR_INTERNALERR);
                        return  false;
                }
        }
}

static inline bool traverse_column(struct bison_find *find, struct bison_dot_path *path, u32 current_path_pos,
        struct bison_column_it *it)
{
        u32 total_path_len;
        u32 requested_idx;
        u32 nun_values_contained;
        enum bison_dot_node_type node_type;
        enum bison_field_type column_type;
        bison_dot_path_len(&total_path_len, path);
        if (current_path_pos + 1 != total_path_len) {
                /* a column contained cannot contain further containers; since the current path node is not
                 * the last one, traversal cannot be continued */
                return false;
        } else {
                bison_dot_path_type_at(&node_type, current_path_pos, path);
                assert(node_type == DOT_NODE_ARRAY_IDX);
                bison_dot_path_idx_at(&requested_idx, current_path_pos, path);
                bison_column_it_values(&column_type, &nun_values_contained, it);
                if (requested_idx >= nun_values_contained) {
                        /* requested index does not exists in this column */
                        return false;
                } else {
                        find->type = column_type;
                        switch(find->type) {
                                case BISON_FIELD_TYPE_NULL:
                                        /* nothing to do */
                                        break;
                                case BISON_FIELD_TYPE_TRUE:
                                case BISON_FIELD_TYPE_FALSE:
                                        find->value.boolean = bison_column_it_boolean_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_U8:
                                        find->value.unsigned_number = bison_column_it_u8_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_U16:
                                        find->value.unsigned_number = bison_column_it_u16_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_U32:
                                        find->value.unsigned_number = bison_column_it_u32_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_U64:
                                        find->value.unsigned_number = bison_column_it_u64_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_I8:
                                        find->value.signed_number = bison_column_it_i8_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_I16:
                                        find->value.signed_number = bison_column_it_i16_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_I32:
                                        find->value.signed_number = bison_column_it_i32_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_I64:
                                        find->value.signed_number = bison_column_it_i64_values(NULL, it)[requested_idx];
                                        break;
                                case BISON_FIELD_TYPE_NUMBER_FLOAT:
                                        find->value.float_number = bison_column_it_float_values(NULL, it)[requested_idx];
                                        break;
                                default:
                                        error(&it->err, NG5_ERR_UNSUPPORTEDTYPE)
                                        return false;
                        }
                        return true;
                }
        }
}

static bool find_by_path(struct bison_find *find, struct bison_dot_path *path, struct bison *doc)
{
        bison_iterator_open(&find->root_it, doc);
        return traverse_array(find, path, 0, &find->root_it);
}