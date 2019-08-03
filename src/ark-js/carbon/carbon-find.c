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

#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-find.h>
#include "carbon-find.h"

static void result_from_array(struct carbon_find *find, struct carbon_array_it *it);
static inline bool result_from_column(struct carbon_find *find, u32 requested_idx, struct carbon_column_it *it);

ARK_EXPORT(bool) carbon_find_open(struct carbon_find *out, const char *dot_path, struct carbon *doc)
{
        error_if_null(out)
        error_if_null(dot_path)
        error_if_null(doc)
        struct carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        bool status = carbon_find_create(out, &path, doc);
        carbon_dot_path_drop(&path);
        return status;
}

ARK_EXPORT(bool) carbon_find_close(struct carbon_find *find)
{
        error_if_null(find)
        if (carbon_find_has_result(find)) {
                enum carbon_field_type type;
                carbon_find_result_type(&type, find);
                switch (type) {
                case CARBON_FIELD_TYPE_OBJECT:
                        error_print(ARK_ERR_NOTIMPLEMENTED)
                        break;
                case CARBON_FIELD_TYPE_ARRAY:
                        carbon_array_it_drop(find->value.array_it);
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8:
                case CARBON_FIELD_TYPE_COLUMN_U16:
                case CARBON_FIELD_TYPE_COLUMN_U32:
                case CARBON_FIELD_TYPE_COLUMN_U64:
                case CARBON_FIELD_TYPE_COLUMN_I8:
                case CARBON_FIELD_TYPE_COLUMN_I16:
                case CARBON_FIELD_TYPE_COLUMN_I32:
                case CARBON_FIELD_TYPE_COLUMN_I64:
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        break;
                default:
                        break;
                }
        }
        return carbon_find_drop(find);
}

ARK_EXPORT(bool) carbon_find_create(struct carbon_find *find, struct carbon_dot_path *path, struct carbon *doc)
{
        error_if_null(find)
        error_if_null(path)
        error_if_null(doc)

        error_init(&find->err);
        find->doc = doc;

        ark_check_success(carbon_path_evaluator_begin(&find->path_evaluater, path, doc));
        if (carbon_path_evaluator_has_result(&find->path_evaluater)) {
                switch (find->path_evaluater.result.container_type) {
                case CARBON_ARRAY:
                        result_from_array(find, &find->path_evaluater.result.containers.array.it);
                        break;
                case CARBON_COLUMN:
                        result_from_column(find, find->path_evaluater.result.containers.column.elem_pos,
                                &find->path_evaluater.result.containers.column.it);
                        break;
                default:
                        error(&path->err, ARK_ERR_INTERNALERR);
                        return false;
                }
                return true;
        } else {
                return false;
        }
}

ARK_EXPORT(bool) carbon_find_has_result(struct carbon_find *find)
{
        error_if_null(find)
        return carbon_path_evaluator_has_result(&find->path_evaluater);
}

ARK_EXPORT(bool) carbon_find_result_type(enum carbon_field_type *type, struct carbon_find *find)
{
        error_if_null(type)
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        *type = find->type;
        return true;
}

ARK_EXPORT(struct carbon_array_it *) carbon_find_result_array(struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_ARRAY, &find->err, ARK_ERR_TYPEMISMATCH)
        return find->value.array_it;
}

ARK_EXPORT(struct carbon_column_it *) carbon_find_result_column(struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_COLUMN_U8 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_U16 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_U32 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_U64 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_I8 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_I16 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_I32 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_I64 &&
                find->type != CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                find->type != CARBON_FIELD_TYPE_COLUMN_BOOLEAN, &find->err, ARK_ERR_TYPEMISMATCH)
        return find->value.column_it;
}

ARK_EXPORT(bool) carbon_find_result_boolean(bool *out, struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_boolean(find->type), &find->err, ARK_ERR_TYPEMISMATCH)
        *out = find->value.boolean;
        return true;
}

ARK_EXPORT(bool) carbon_find_result_unsigned(u64 *out, struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_unsigned_integer(find->type), &find->err, ARK_ERR_TYPEMISMATCH)
        *out = find->value.unsigned_number;
        return true;
}

ARK_EXPORT(bool) carbon_find_result_signed(i64 *out, struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_signed_integer(find->type), &find->err, ARK_ERR_TYPEMISMATCH)
        *out = find->value.signed_number;
        return true;
}

ARK_EXPORT(bool) carbon_find_result_float(float *out, struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_floating_number(find->type), &find->err, ARK_ERR_TYPEMISMATCH)
        *out = find->value.float_number;
        return true;
}

ARK_EXPORT(const char *) carbon_find_result_string(u64 *str_len, struct carbon_find *find)
{
        error_if_null(find)
        error_if_null(str_len)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_STRING, &find->err, ARK_ERR_TYPEMISMATCH)
        *str_len = find->value.string.len;
        return find->value.string.base;
}

ARK_EXPORT(struct carbon_binary *) carbon_find_result_binary(struct carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, ARK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_binary(find->type), &find->err, ARK_ERR_TYPEMISMATCH)
        return &find->value.binary;
}

ARK_EXPORT(bool) carbon_find_drop(struct carbon_find *find)
{
        error_if_null(find)
        carbon_path_evaluator_end(&find->path_evaluater);
        return true;
}

static void result_from_array(struct carbon_find *find, struct carbon_array_it *it)
{
        find->type = it->field_access.it_field_type;
        switch (find->type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                        /* no value to be stored */
                        break;
                case CARBON_FIELD_TYPE_ARRAY:
                        find->value.array_it = carbon_array_it_array_value(it);
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8:
                case CARBON_FIELD_TYPE_COLUMN_U16:
                case CARBON_FIELD_TYPE_COLUMN_U32:
                case CARBON_FIELD_TYPE_COLUMN_U64:
                case CARBON_FIELD_TYPE_COLUMN_I8:
                case CARBON_FIELD_TYPE_COLUMN_I16:
                case CARBON_FIELD_TYPE_COLUMN_I32:
                case CARBON_FIELD_TYPE_COLUMN_I64:
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        find->value.column_it = carbon_array_it_column_value(it);
                        break;
                case CARBON_FIELD_TYPE_OBJECT:
                        error_print_and_die_if(true, ARK_ERR_NOTIMPLEMENTED); /* TODO: implement this for objects */
                        break;
                case CARBON_FIELD_TYPE_STRING:
                        find->value.string.base = carbon_array_it_string_value(&find->value.string.len, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_U64:
                        carbon_array_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                        carbon_array_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        carbon_array_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case CARBON_FIELD_TYPE_BINARY:
                case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        carbon_array_it_binary_value(&find->value.binary, it);
                        break;
                default:
                        error(&find->err, ARK_ERR_INTERNALERR);
                        break;
        }
}

static inline bool result_from_column(struct carbon_find *find, u32 requested_idx, struct carbon_column_it *it)
{
        u32 num_contained_values;
        carbon_column_it_values_info(&find->type, &num_contained_values, it);
        assert(requested_idx < num_contained_values);

        switch(find->type) {
        case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                find->value.boolean = carbon_column_it_boolean_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_U8:
                find->value.unsigned_number = carbon_column_it_u8_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_U16:
                find->value.unsigned_number = carbon_column_it_u16_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_U32:
                find->value.unsigned_number = carbon_column_it_u32_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_U64:
                find->value.unsigned_number = carbon_column_it_u64_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_I8:
                find->value.signed_number = carbon_column_it_i8_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_I16:
                find->value.signed_number = carbon_column_it_i16_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_I32:
                find->value.signed_number = carbon_column_it_i32_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_I64:
                find->value.signed_number = carbon_column_it_i64_values(NULL, it)[requested_idx];
                break;
        case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                find->value.float_number = carbon_column_it_float_values(NULL, it)[requested_idx];
                break;
        default:
        error(&it->err, ARK_ERR_UNSUPPORTEDTYPE)
                return false;
        }
        return true;
}