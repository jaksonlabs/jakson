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

static void result_from_array(struct jak_carbon_find *find, struct jak_carbon_array_it *it);

static void result_from_object(struct jak_carbon_find *find, struct jak_carbon_object_it *it);

static inline bool result_from_column(struct jak_carbon_find *find, u32 requested_idx, struct jak_carbon_column_it *it);

bool carbon_find_open(struct jak_carbon_find *out, const char *dot_path, struct jak_carbon *doc)
{
        error_if_null(out)
        error_if_null(dot_path)
        error_if_null(doc)
        struct jak_carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        carbon_find_create(out, &path, doc);
        carbon_dot_path_drop(&path);
        return true;
}

bool carbon_find_close(struct jak_carbon_find *find)
{
        error_if_null(find)
        if (carbon_find_has_result(find)) {
                enum carbon_field_type type;
                carbon_find_result_type(&type, find);
                switch (type) {
                        case CARBON_FIELD_TYPE_OBJECT:
                                carbon_object_it_drop(find->value.object_it);
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
                return carbon_find_drop(find);
        }
        return true;
}

bool carbon_find_create(struct jak_carbon_find *find, struct jak_carbon_dot_path *path, struct jak_carbon *doc)
{
        error_if_null(find)
        error_if_null(path)
        error_if_null(doc)

        JAK_zero_memory(find, sizeof(struct jak_carbon_find));
        error_init(&find->err);
        find->doc = doc;

        JAK_check_success(carbon_path_evaluator_begin(&find->path_evaluater, path, doc));
        if (carbon_path_evaluator_has_result(&find->path_evaluater)) {
                switch (find->path_evaluater.result.container_type) {
                        case CARBON_ARRAY:
                                result_from_array(find, &find->path_evaluater.result.containers.array.it);
                                break;
                        case CARBON_COLUMN:
                                result_from_column(find, find->path_evaluater.result.containers.column.elem_pos,
                                                   &find->path_evaluater.result.containers.column.it);
                                break;
                        case CARBON_OBJECT:
                                result_from_object(find, &find->path_evaluater.result.containers.object.it);
                                break;
                        default: error(&path->err, JAK_ERR_INTERNALERR);
                                return false;
                }
        }
        return true;
}

bool carbon_find_has_result(struct jak_carbon_find *find)
{
        error_if_null(find)
        return carbon_path_evaluator_has_result(&find->path_evaluater);
}

const char *carbon_find_result_to_str(struct jak_string *dst_str, enum carbon_printer_impl print_type, struct jak_carbon_find *find)
{
        error_if_null(dst_str)
        error_if_null(find)

        string_clear(dst_str);

        struct printer printer;
        carbon_printer_by_type(&printer, print_type);

        if (carbon_find_has_result(find)) {
                enum carbon_field_type result_type;
                carbon_find_result_type(&result_type, find);
                switch (result_type) {
                        case CARBON_FIELD_TYPE_NULL:
                                carbon_printer_null(&printer, dst_str);
                                break;
                        case CARBON_FIELD_TYPE_TRUE:
                                carbon_printer_true(&printer, false, dst_str);
                                break;
                        case CARBON_FIELD_TYPE_FALSE:
                                carbon_printer_false(&printer, false, dst_str);
                                break;
                        case CARBON_FIELD_TYPE_OBJECT: {
                                struct jak_carbon_object_it *sub_it = carbon_find_result_object(find);
                                carbon_printer_print_object(sub_it, &printer, dst_str);
                        } break;
                        case CARBON_FIELD_TYPE_ARRAY: {
                                struct jak_carbon_array_it *sub_it = carbon_find_result_array(find);
                                carbon_printer_print_array(sub_it, &printer, dst_str, false);
                        } break;
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
                                struct jak_carbon_column_it *sub_it = carbon_find_result_column(find);
                                carbon_printer_print_column(sub_it, &printer, dst_str);
                        } break;
                        case CARBON_FIELD_TYPE_STRING: {
                                u64 str_len = 0;
                                const char *str = carbon_find_result_string(&str_len, find);
                                carbon_printer_string(&printer, dst_str, str, str_len);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_U8: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u8_or_null(&printer, dst_str, (u8) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_U16: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u16_or_null(&printer, dst_str, (u16) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_U32: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u32_or_null(&printer, dst_str, (u32) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_U64: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u64_or_null(&printer, dst_str, (u64) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_I8: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i8_or_null(&printer, dst_str, (i8) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_I16: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i16_or_null(&printer, dst_str, (i16) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_I32: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i32_or_null(&printer, dst_str, (i32) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_I64: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i64_or_null(&printer, dst_str, (i64) val);
                        } break;
                        case CARBON_FIELD_TYPE_NUMBER_FLOAT: {
                                float val = 0;
                                carbon_find_result_float(&val, find);
                                carbon_printer_float(&printer, dst_str, &val);
                        } break;
                        case CARBON_FIELD_TYPE_BINARY:
                        case CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                                const struct jak_carbon_binary *val = carbon_find_result_binary(find);
                                carbon_printer_binary(&printer, dst_str, val);
                        } break;
                        default:
                                error(&find->err, JAK_ERR_INTERNALERR)
                                return NULL;
                }

        } else {
                string_add(dst_str, JAK_CARBON_NIL_STR);
        }
        carbon_printer_drop(&printer);

        return string_cstr(dst_str);
}

const char *carbon_find_result_to_json_compact(struct jak_string *dst_str, struct jak_carbon_find *find)
{
        return carbon_find_result_to_str(dst_str, JSON_COMPACT, find);
}

char *carbon_find_result_to_json_compact_dup(struct jak_carbon_find *find)
{
        struct jak_string str;
        string_create(&str);
        char *ret = strdup(carbon_find_result_to_json_compact(&str, find));
        string_drop(&str);
        return ret;
}

bool carbon_find_result_type(enum carbon_field_type *type, struct jak_carbon_find *find)
{
        error_if_null(type)
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        *type = find->type;
        return true;
}

struct jak_carbon_array_it *carbon_find_result_array(struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_ARRAY, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.array_it;
}

struct jak_carbon_object_it *carbon_find_result_object(struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_OBJECT, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.object_it;
}

struct jak_carbon_column_it *carbon_find_result_column(struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_COLUMN_U8 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_U16 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_U32 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_U64 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_I8 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_I16 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_I32 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_I64 &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                 find->type != CARBON_FIELD_TYPE_COLUMN_BOOLEAN, &find->err, JAK_ERR_TYPEMISMATCH)
        return find->value.column_it;
}

bool carbon_find_result_boolean(bool *out, struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_boolean(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.boolean;
        return true;
}

bool carbon_find_result_unsigned(u64 *out, struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_unsigned_integer(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.unsigned_number;
        return true;
}

bool carbon_find_result_signed(i64 *out, struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_signed_integer(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.signed_number;
        return true;
}

bool carbon_find_result_float(float *out, struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_floating_number(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        *out = find->value.float_number;
        return true;
}

const char *carbon_find_result_string(u64 *str_len, struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if_null(str_len)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(find->type != CARBON_FIELD_TYPE_STRING, &find->err, JAK_ERR_TYPEMISMATCH)
        *str_len = find->value.string.len;
        return find->value.string.base;
}

struct jak_carbon_binary *carbon_find_result_binary(struct jak_carbon_find *find)
{
        error_if_null(find)
        error_if(!carbon_path_evaluator_has_result(&find->path_evaluater), &find->err, JAK_ERR_ILLEGALSTATE)
        error_if(!carbon_field_type_is_binary(find->type), &find->err, JAK_ERR_TYPEMISMATCH)
        return &find->value.binary;
}

bool carbon_find_drop(struct jak_carbon_find *find)
{
        error_if_null(find)
        carbon_path_evaluator_end(&find->path_evaluater);
        return true;
}

static void result_from_array(struct jak_carbon_find *find, struct jak_carbon_array_it *it)
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
                        find->value.object_it = carbon_array_it_object_value(it);
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
                default: error(&find->err, JAK_ERR_INTERNALERR);
                        break;
        }
}

static void result_from_object(struct jak_carbon_find *find, struct jak_carbon_object_it *it)
{
        carbon_object_it_prop_type(&find->type, it);
        switch (find->type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                        /* no value to be stored */
                        break;
                case CARBON_FIELD_TYPE_ARRAY:
                        find->value.array_it = carbon_object_it_array_value(it);
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
                        find->value.column_it = carbon_object_it_column_value(it);
                        break;
                case CARBON_FIELD_TYPE_OBJECT:
                        find->value.object_it = carbon_object_it_object_value(it);
                        break;
                case CARBON_FIELD_TYPE_STRING:
                        find->value.string.base = carbon_object_it_string_value(&find->value.string.len, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_U64:
                        carbon_object_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                        carbon_object_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        carbon_object_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case CARBON_FIELD_TYPE_BINARY:
                case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        carbon_object_it_binary_value(&find->value.binary, it);
                        break;
                default: error(&find->err, JAK_ERR_INTERNALERR);
                        break;
        }
}

static inline bool result_from_column(struct jak_carbon_find *find, u32 requested_idx, struct jak_carbon_column_it *it)
{
        u32 num_contained_values;
        carbon_column_it_values_info(&find->type, &num_contained_values, it);
        assert(requested_idx < num_contained_values);

        switch (find->type) {
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        u8 field_value = carbon_column_it_boolean_values(NULL, it)[requested_idx];
                        if (is_null_boolean(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else if (field_value == JAK_CARBON_BOOLEAN_COLUMN_TRUE) {
                                find->type = CARBON_FIELD_TYPE_TRUE;
                        } else if (field_value == JAK_CARBON_BOOLEAN_COLUMN_FALSE) {
                                find->type = CARBON_FIELD_TYPE_FALSE;
                        } else {
                                error(&it->err, JAK_ERR_INTERNALERR);
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_U8: {
                        u8 field_value = carbon_column_it_u8_values(NULL, it)[requested_idx];
                        if (is_null_u8(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_U8;
                                find->value.unsigned_number = carbon_column_it_u8_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_U16: {
                        u16 field_value = carbon_column_it_u16_values(NULL, it)[requested_idx];
                        if (is_null_u16(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_U16;
                                find->value.unsigned_number = carbon_column_it_u16_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_U32: {
                        u32 field_value = carbon_column_it_u32_values(NULL, it)[requested_idx];
                        if (is_null_u32(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_U32;
                                find->value.unsigned_number = carbon_column_it_u32_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_U64: {
                        u64 field_value = carbon_column_it_u64_values(NULL, it)[requested_idx];
                        if (is_null_u64(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_U64;
                                find->value.unsigned_number = carbon_column_it_u64_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_I8: {
                        i8 field_value = carbon_column_it_i8_values(NULL, it)[requested_idx];
                        if (is_null_i8(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_I8;
                                find->value.signed_number = carbon_column_it_i8_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_I16: {
                        i16 field_value = carbon_column_it_i16_values(NULL, it)[requested_idx];
                        if (is_null_i16(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_I16;
                                find->value.signed_number = carbon_column_it_i16_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_I32: {
                        i32 field_value = carbon_column_it_i32_values(NULL, it)[requested_idx];
                        if (is_null_i32(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_I32;
                                find->value.signed_number = carbon_column_it_i32_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_I64: {
                        i64 field_value = carbon_column_it_i64_values(NULL, it)[requested_idx];
                        if (is_null_i64(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_I64;
                                find->value.signed_number = carbon_column_it_i64_values(NULL, it)[requested_idx];
                        }
                } break;
                case CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                        float field_value = carbon_column_it_float_values(NULL, it)[requested_idx];
                        if (is_null_float(field_value)) {
                                find->type = CARBON_FIELD_TYPE_NULL;
                        } else {
                                find->type = CARBON_FIELD_TYPE_NUMBER_FLOAT;
                                find->value.float_number = carbon_column_it_float_values(NULL, it)[requested_idx];
                        }
                } break;
                default: error(&it->err, JAK_ERR_UNSUPPORTEDTYPE)
                        return false;
        }
        return true;
}