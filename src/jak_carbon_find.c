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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_carbon_dot.h>
#include <jak_carbon_find.h>
#include "jak_carbon_find.h"

static void result_from_array(jak_carbon_find *find, jak_carbon_array_it *it);

static void result_from_object(jak_carbon_find *find, jak_carbon_object_it *it);

static inline bool
result_from_column(jak_carbon_find *find, jak_u32 requested_idx, jak_carbon_column_it *it);

bool jak_carbon_find_open(jak_carbon_find *out, const char *dot_path, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(dot_path)
        JAK_ERROR_IF_NULL(doc)
        jak_carbon_dot_path path;
        jak_carbon_dot_path_from_string(&path, dot_path);
        jak_carbon_find_create(out, &path, doc);
        jak_carbon_dot_path_drop(&path);
        return true;
}

bool jak_carbon_find_close(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        if (jak_carbon_find_has_result(find)) {
                jak_carbon_field_type_e type;
                jak_carbon_find_result_type(&type, find);
                switch (type) {
                        case JAK_CARBON_FIELD_TYPE_OBJECT:
                                jak_carbon_object_it_drop(find->value.object_it);
                                break;
                        case JAK_CARBON_FIELD_TYPE_ARRAY:
                                jak_carbon_array_it_drop(find->value.array_it);
                                break;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                                break;
                        default:
                                break;
                }
                return jak_carbon_find_drop(find);
        }
        return true;
}

bool jak_carbon_find_create(jak_carbon_find *find, jak_carbon_dot_path *path, jak_carbon *doc)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF_NULL(path)
        JAK_ERROR_IF_NULL(doc)

        JAK_ZERO_MEMORY(find, sizeof(jak_carbon_find));
        jak_error_init(&find->err);
        find->doc = doc;

        JAK_CHECK_SUCCESS(jak_carbon_path_evaluator_begin(&find->path_evaluater, path, doc));
        if (jak_carbon_path_evaluator_has_result(&find->path_evaluater)) {
                switch (find->path_evaluater.result.container_type) {
                        case JAK_CARBON_ARRAY:
                                result_from_array(find, &find->path_evaluater.result.containers.array.it);
                                break;
                        case JAK_CARBON_COLUMN:
                                result_from_column(find, find->path_evaluater.result.containers.column.elem_pos,
                                                   &find->path_evaluater.result.containers.column.it);
                                break;
                        case JAK_CARBON_OBJECT:
                                result_from_object(find, &find->path_evaluater.result.containers.object.it);
                                break;
                        default: JAK_ERROR(&path->err, JAK_ERR_INTERNALERR);
                                return false;
                }
        }
        return true;
}

bool jak_carbon_find_has_result(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        return jak_carbon_path_evaluator_has_result(&find->path_evaluater);
}

const char *
jak_carbon_find_result_to_str(jak_string *dst_str, jak_carbon_printer_impl_e print_type, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(dst_str)
        JAK_ERROR_IF_NULL(find)

        jak_string_clear(dst_str);

        jak_carbon_printer printer;
        jak_carbon_printer_by_type(&printer, print_type);

        if (jak_carbon_find_has_result(find)) {
                jak_carbon_field_type_e result_type;
                jak_carbon_find_result_type(&result_type, find);
                switch (result_type) {
                        case JAK_CARBON_FIELD_TYPE_NULL:
                                jak_carbon_printer_null(&printer, dst_str);
                                break;
                        case JAK_CARBON_FIELD_TYPE_TRUE:
                                jak_carbon_printer_true(&printer, false, dst_str);
                                break;
                        case JAK_CARBON_FIELD_TYPE_FALSE:
                                jak_carbon_printer_false(&printer, false, dst_str);
                                break;
                        case JAK_CARBON_FIELD_TYPE_OBJECT: {
                                jak_carbon_object_it *sub_it = jak_carbon_find_result_object(find);
                                jak_carbon_printer_print_object(sub_it, &printer, dst_str);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_ARRAY: {
                                jak_carbon_array_it *sub_it = jak_carbon_find_result_array(find);
                                jak_carbon_printer_print_array(sub_it, &printer, dst_str, false);
                        }
                                break;
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
                                jak_carbon_column_it *sub_it = jak_carbon_find_result_column(find);
                                jak_carbon_printer_print_column(sub_it, &printer, dst_str);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_STRING: {
                                jak_u64 str_len = 0;
                                const char *str = jak_carbon_find_result_string(&str_len, find);
                                jak_carbon_printer_string(&printer, dst_str, str, str_len);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_U8: {
                                jak_u64 val = 0;
                                jak_carbon_find_result_unsigned(&val, find);
                                jak_carbon_printer_u8_or_null(&printer, dst_str, (jak_u8) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_U16: {
                                jak_u64 val = 0;
                                jak_carbon_find_result_unsigned(&val, find);
                                jak_carbon_printer_u16_or_null(&printer, dst_str, (jak_u16) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_U32: {
                                jak_u64 val = 0;
                                jak_carbon_find_result_unsigned(&val, find);
                                jak_carbon_printer_u32_or_null(&printer, dst_str, (jak_u32) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_U64: {
                                jak_u64 val = 0;
                                jak_carbon_find_result_unsigned(&val, find);
                                jak_carbon_printer_u64_or_null(&printer, dst_str, (jak_u64) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_I8: {
                                jak_i64 val = 0;
                                jak_carbon_find_result_signed(&val, find);
                                jak_carbon_printer_i8_or_null(&printer, dst_str, (jak_i8) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_I16: {
                                jak_i64 val = 0;
                                jak_carbon_find_result_signed(&val, find);
                                jak_carbon_printer_i16_or_null(&printer, dst_str, (jak_i16) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_I32: {
                                jak_i64 val = 0;
                                jak_carbon_find_result_signed(&val, find);
                                jak_carbon_printer_i32_or_null(&printer, dst_str, (jak_i32) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_I64: {
                                jak_i64 val = 0;
                                jak_carbon_find_result_signed(&val, find);
                                jak_carbon_printer_i64_or_null(&printer, dst_str, (jak_i64) val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT: {
                                float val = 0;
                                jak_carbon_find_result_float(&val, find);
                                jak_carbon_printer_float(&printer, dst_str, &val);
                        }
                                break;
                        case JAK_CARBON_FIELD_TYPE_BINARY:
                        case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                                const jak_carbon_binary *val = jak_carbon_find_result_binary(find);
                                jak_carbon_printer_binary(&printer, dst_str, val);
                        }
                                break;
                        default: JAK_ERROR(&find->err, JAK_ERR_INTERNALERR)
                                return NULL;
                }

        } else {
                jak_string_add(dst_str, JAK_CARBON_NIL_STR);
        }
        jak_carbon_printer_drop(&printer);

        return jak_string_cstr(dst_str);
}

const char *jak_carbon_find_result_to_json_compact(jak_string *dst_str, jak_carbon_find *find)
{
        return jak_carbon_find_result_to_str(dst_str, JAK_JSON_COMPACT, find);
}

char *jak_carbon_find_result_to_json_compact_dup(jak_carbon_find *find)
{
        jak_string str;
        jak_string_create(&str);
        char *ret = strdup(jak_carbon_find_result_to_json_compact(&str, find));
        jak_string_drop(&str);
        return ret;
}

bool jak_carbon_find_result_type(jak_carbon_field_type_e *type, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(type)
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        *type = find->type;
        return true;
}

jak_carbon_array_it *jak_carbon_find_result_array(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(find->type != JAK_CARBON_FIELD_TYPE_ARRAY, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.array_it;
}

jak_carbon_object_it *jak_carbon_find_result_object(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(find->type != JAK_CARBON_FIELD_TYPE_OBJECT, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.object_it;
}

jak_carbon_column_it *jak_carbon_find_result_column(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(find->type != JAK_CARBON_FIELD_TYPE_COLUMN_U8 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_U16 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_U32 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_U64 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_I8 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_I16 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_I32 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_I64 &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                 find->type != JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.column_it;
}

bool jak_carbon_find_result_boolean(bool *out, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(!jak_carbon_field_type_is_boolean(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.boolean;
        return true;
}

bool jak_carbon_find_result_unsigned(jak_u64 *out, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(!jak_carbon_field_type_is_unsigned(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.unsigned_number;
        return true;
}

bool jak_carbon_find_result_signed(jak_i64 *out, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(!jak_carbon_field_type_is_signed(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.signed_number;
        return true;
}

bool jak_carbon_find_result_float(float *out, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(!jak_carbon_field_type_is_floating(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.float_number;
        return true;
}

const char *jak_carbon_find_result_string(jak_u64 *str_len, jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF_NULL(str_len)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(find->type != JAK_CARBON_FIELD_TYPE_STRING, &find->err, JAK_ERR_TYPEMISMATCH)
        *str_len = find->value.string.len;
        return find->value.string.base;
}

jak_carbon_binary *jak_carbon_find_result_binary(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        JAK_ERROR_IF(!jak_carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        JAK_ERROR_IF(!jak_carbon_field_type_is_binary(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        return &find->value.binary;
}

bool jak_carbon_find_drop(jak_carbon_find *find)
{
        JAK_ERROR_IF_NULL(find)
        jak_carbon_path_evaluator_end(&find->path_evaluater);
        return true;
}

static void result_from_array(jak_carbon_find *find, jak_carbon_array_it *it)
{
        find->type = it->field_access.it_field_type;
        switch (find->type) {
                case JAK_CARBON_FIELD_TYPE_NULL:
                case JAK_CARBON_FIELD_TYPE_TRUE:
                case JAK_CARBON_FIELD_TYPE_FALSE:
                        /* no value to be stored */
                        break;
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                        find->value.array_it = jak_carbon_array_it_array_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        find->value.column_it = jak_carbon_array_it_column_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                        find->value.object_it = jak_carbon_array_it_object_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_STRING:
                        find->value.string.base = jak_carbon_array_it_jak_string_value(&find->value.string.len, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U64:
                        jak_carbon_array_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I64:
                        jak_carbon_array_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        jak_carbon_array_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_BINARY:
                case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        jak_carbon_array_it_binary_value(&find->value.binary, it);
                        break;
                default: JAK_ERROR(&find->err, JAK_ERR_INTERNALERR);
                        break;
        }
}

static void result_from_object(jak_carbon_find *find, jak_carbon_object_it *it)
{
        jak_carbon_object_it_prop_type(&find->type, it);
        switch (find->type) {
                case JAK_CARBON_FIELD_TYPE_NULL:
                case JAK_CARBON_FIELD_TYPE_TRUE:
                case JAK_CARBON_FIELD_TYPE_FALSE:
                        /* no value to be stored */
                        break;
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                        find->value.array_it = jak_carbon_object_it_array_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        find->value.column_it = jak_carbon_object_it_column_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                        find->value.object_it = jak_carbon_object_it_object_value(it);
                        break;
                case JAK_CARBON_FIELD_TYPE_STRING:
                        find->value.string.base = jak_carbon_object_it_jak_string_value(&find->value.string.len, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U64:
                        jak_carbon_object_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I64:
                        jak_carbon_object_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        jak_carbon_object_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case JAK_CARBON_FIELD_TYPE_BINARY:
                case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        jak_carbon_object_it_binary_value(&find->value.binary, it);
                        break;
                default: JAK_ERROR(&find->err, JAK_ERR_INTERNALERR);
                        break;
        }
}

static inline bool
result_from_column(jak_carbon_find *find, jak_u32 requested_idx, jak_carbon_column_it *it)
{
        jak_u32 num_contained_values;
        jak_carbon_column_it_values_info(&find->type, &num_contained_values, it);
        JAK_ASSERT(requested_idx < num_contained_values);

        switch (find->type) {
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        jak_u8 field_value = jak_carbon_column_it_boolean_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_BOOLEAN(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else if (field_value == JAK_CARBON_BOOLEAN_COLUMN_TRUE) {
                                find->type = JAK_CARBON_FIELD_TYPE_TRUE;
                        } else if (field_value == JAK_CARBON_BOOLEAN_COLUMN_FALSE) {
                                find->type = JAK_CARBON_FIELD_TYPE_FALSE;
                        } else {
                                JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8: {
                        jak_u8 field_value = jak_carbon_column_it_u8_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_U8(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_U8;
                                find->value.unsigned_number = jak_carbon_column_it_u8_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16: {
                        jak_u16 field_value = jak_carbon_column_it_u16_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_U16(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_U16;
                                find->value.unsigned_number = jak_carbon_column_it_u16_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32: {
                        jak_u32 field_value = jak_carbon_column_it_u32_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_U32(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_U32;
                                find->value.unsigned_number = jak_carbon_column_it_u32_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64: {
                        jak_u64 field_value = jak_carbon_column_it_u64_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_U64(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_U64;
                                find->value.unsigned_number = jak_carbon_column_it_u64_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8: {
                        jak_i8 field_value = jak_carbon_column_it_i8_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_I8(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_I8;
                                find->value.signed_number = jak_carbon_column_it_i8_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16: {
                        jak_i16 field_value = jak_carbon_column_it_i16_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_I16(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_I16;
                                find->value.signed_number = jak_carbon_column_it_i16_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32: {
                        jak_i32 field_value = jak_carbon_column_it_i32_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_I32(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_I32;
                                find->value.signed_number = jak_carbon_column_it_i32_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64: {
                        jak_i64 field_value = jak_carbon_column_it_i64_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_I64(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_I64;
                                find->value.signed_number = jak_carbon_column_it_i64_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                        float field_value = jak_carbon_column_it_float_values(NULL, it)[requested_idx];
                        if (JAK_IS_NULL_FLOAT(field_value)) {
                                find->type = JAK_CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT;
                                find->value.float_number = jak_carbon_column_it_float_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                default: JAK_ERROR(&it->err, JAK_ERR_UNSUPPORTEDTYPE)
                        return false;
        }
        return true;
}