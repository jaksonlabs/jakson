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

#include "core/bison/bison-dot.h"
#include "core/bison/bison-find.h"

static void result_from_array(struct bison_find *find, struct bison_array_it *it);
static inline bool result_from_column(struct bison_find *find, u32 requested_idx, struct bison_column_it *it);

NG5_EXPORT(bool) bison_find_create(struct bison_find *find, struct bison_dot_path *path, struct bison *doc)
{
        error_if_null(find)
        error_if_null(path)
        error_if_null(doc)

        error_init(&find->err);
        find->doc = doc;

        ng5_check_success(bison_path_evaluator_begin(&find->path_evaluater, path, doc));
        if (bison_path_evaluator_has_result(&find->path_evaluater)) {
                switch (find->path_evaluater.result.container_type) {
                case BISON_ARRAY:
                        result_from_array(find, find->path_evaluater.result.containers.array.it);
                        break;
                case BISON_COLUMN:
                        result_from_column(find, find->path_evaluater.result.containers.column.elem_pos,
                                find->path_evaluater.result.containers.column.it);
                        break;
                default:
                        error(&path->err, NG5_ERR_INTERNALERR);
                        return false;
                }
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_find_has_result(struct bison_find *find)
{
        error_if_null(find)
        return find->path_evaluater.status == BISON_PATH_RESOLVED;
}

NG5_EXPORT(bool) bison_find_result_type(enum bison_field_type *type, struct bison_find *find)
{
        error_if_null(type)
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        *type = find->type;
        return true;
}

NG5_EXPORT(struct bison_array_it *) bison_find_result_array(struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_ARRAY, &find->err, NG5_ERR_TYPEMISMATCH)
        return find->value.array_it;
}

NG5_EXPORT(struct bison_column_it *) bison_find_result_column(struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_COLUMN, &find->err, NG5_ERR_TYPEMISMATCH)
        return find->value.column_it;
}

NG5_EXPORT(bool) bison_find_result_boolean(bool *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_boolean(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.boolean;
        return true;
}

NG5_EXPORT(bool) bison_find_result_unsigned(u64 *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_unsigned_integer(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.unsigned_number;
        return true;
}

NG5_EXPORT(bool) bison_find_result_signed(i64 *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_signed_integer(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.signed_number;
        return true;
}

NG5_EXPORT(bool) bison_find_result_float(float *out, struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_floating_number(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        *out = find->value.float_number;
        return true;
}

NG5_EXPORT(const char *) bison_find_result_string(u64 *str_len, struct bison_find *find)
{
        error_if_null(find)
        error_if_null(str_len)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(find->type != BISON_FIELD_TYPE_STRING, &find->err, NG5_ERR_TYPEMISMATCH)
        *str_len = find->value.string.len;
        return find->value.string.base;
}

NG5_EXPORT(struct bison_binary *) bison_find_result_binary(struct bison_find *find)
{
        error_if_null(find)
        error_if(!bison_path_evaluator_has_result(&find->path_evaluater), &find->err, NG5_ERR_ILLEGALSTATE)
        error_if(!bison_field_type_is_binary(find->type), &find->err, NG5_ERR_TYPEMISMATCH)
        return &find->value.binary;
}

NG5_EXPORT(bool) bison_find_drop(struct bison_find *find)
{
        error_if_null(find)
        bison_path_evaluator_end(&find->path_evaluater);
        return true;
}

static void result_from_array(struct bison_find *find, struct bison_array_it *it)
{
        find->type = it->it_field_type;
        switch (find->type) {
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

static inline bool result_from_column(struct bison_find *find, u32 requested_idx, struct bison_column_it *it)
{
        u32 num_contained_values;
        bison_column_it_values_info(&find->type, &num_contained_values, it);
        assert(requested_idx < num_contained_values);

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