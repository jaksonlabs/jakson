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

#include <jakson/carbon/dot.h>
#include <jakson/carbon/find.h>

static void result_from_array(carbon_find *find, carbon_array_it *it);

static void result_from_object(carbon_find *find, carbon_object_it *it);

static inline bool
result_from_column(carbon_find *find, u32 requested_idx, carbon_column_it *it);

bool carbon_find_begin(carbon_find *out, const char *dot_path, carbon *doc)
{
        ERROR_IF_NULL(out)
        ERROR_IF_NULL(dot_path)
        ERROR_IF_NULL(doc)
        carbon_dot_path path;
        carbon_dot_path_from_string(&path, dot_path);
        carbon_find_create(out, &path, doc);
        carbon_dot_path_drop(&path);
        return true;
}

fn_result carbon_find_end(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        if (FN_BOOL(carbon_find_has_result(find))) {
                carbon_field_type_e type;
                carbon_find_result_type(&type, find);
                switch (type) {
                        case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                                carbon_object_it_drop(find->value.object_it);
                                break;
                        case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                                carbon_array_it_drop(find->value.array_it);
                                break;
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                                break;
                        default:
                                break;
                }
                return carbon_find_drop(find);
        }
        return FN_OK();
}

fn_result carbon_find_create(carbon_find *find, carbon_dot_path *path, carbon *doc)
{
        FN_FAIL_IF_NULL(find, path, doc)

        ZERO_MEMORY(find, sizeof(carbon_find));
        error_init(&find->err);
        find->doc = doc;

        FN_FAIL_FORWARD_IF_NOT_OK(carbon_path_evaluator_begin(&find->path_evaluater, path, doc));
        if (FN_BOOL(carbon_path_evaluator_has_result(&find->path_evaluater))) {
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
                        default:
                                return FN_FAIL(ERR_INTERNALERR, "unknown container type");
                }
        }
        return FN_OK();
}

fn_result ofType(bool) carbon_find_has_result(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        return carbon_path_evaluator_has_result(&find->path_evaluater);
}

fn_result ofType(const char *)
carbon_find_result_to_str(string_buffer *dst_str, carbon_printer_impl_e print_type, carbon_find *find)
{
        FN_FAIL_IF_NULL(dst_str, find)

        string_buffer_clear(dst_str);

        carbon_printer printer;
        carbon_printer_by_type(&printer, print_type);

        if (FN_BOOL(carbon_find_has_result(find))) {
                carbon_field_type_e result_type;
                carbon_find_result_type(&result_type, find);
                switch (result_type) {
                        case CARBON_FIELD_NULL:
                                carbon_printer_null(&printer, dst_str);
                                break;
                        case CARBON_FIELD_TRUE:
                                carbon_printer_true(&printer, false, dst_str);
                                break;
                        case CARBON_FIELD_FALSE:
                                carbon_printer_false(&printer, false, dst_str);
                                break;
                        case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                        case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                                carbon_object_it *sub_it = FN_PTR(carbon_object_it, carbon_find_result_object(find));
                                carbon_printer_print_object(sub_it, &printer, dst_str);
                        }
                                break;
                        case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                                carbon_array_it *sub_it = FN_PTR(carbon_array_it, carbon_find_result_array(find));
                                carbon_printer_print_array(sub_it, &printer, dst_str, false);
                        }
                                break;
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                                carbon_column_it *sub_it = FN_PTR(carbon_column_it, carbon_find_result_column(find));
                                carbon_printer_print_column(sub_it, &printer, dst_str);
                        }
                                break;
                        case CARBON_FIELD_STRING: {
                                u64 str_len = 0;
                                const char *str = FN_PTR(const char, carbon_find_result_string(&str_len, find));
                                carbon_printer_string(&printer, dst_str, str, str_len);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_U8: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u8_or_null(&printer, dst_str, (u8) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_U16: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u16_or_null(&printer, dst_str, (u16) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_U32: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u32_or_null(&printer, dst_str, (u32) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_U64: {
                                u64 val = 0;
                                carbon_find_result_unsigned(&val, find);
                                carbon_printer_u64_or_null(&printer, dst_str, (u64) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I8: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i8_or_null(&printer, dst_str, (i8) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I16: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i16_or_null(&printer, dst_str, (i16) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I32: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i32_or_null(&printer, dst_str, (i32) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_I64: {
                                i64 val = 0;
                                carbon_find_result_signed(&val, find);
                                carbon_printer_i64_or_null(&printer, dst_str, (i64) val);
                        }
                                break;
                        case CARBON_FIELD_NUMBER_FLOAT: {
                                float val = 0;
                                carbon_find_result_float(&val, find);
                                carbon_printer_float(&printer, dst_str, &val);
                        }
                                break;
                        case CARBON_FIELD_BINARY:
                        case CARBON_FIELD_BINARY_CUSTOM: {
                                const carbon_binary *val = FN_PTR(const carbon_binary, carbon_find_result_binary(find));
                                carbon_printer_binary(&printer, dst_str, val);
                        }
                                break;
                        default:
                                return FN_FAIL(ERR_INTERNALERR, "unknown field type");
                }

        } else {
                string_buffer_add(dst_str, CARBON_NIL_STR);
        }
        carbon_printer_drop(&printer);

        return FN_OK_PTR(string_cstr(dst_str));
}

fn_result ofType(const char *) carbon_find_result_to_json_compact(string_buffer *dst_str, carbon_find *find)
{
        return carbon_find_result_to_str(dst_str, JSON_COMPACT, find);
}

fn_result ofType(char *) carbon_find_result_to_json_compact_dup(carbon_find *find)
{
        string_buffer str;
        string_buffer_create(&str);
        char *ret = strdup(FN_PTR(const char, carbon_find_result_to_json_compact(&str, find)));
        string_buffer_drop(&str);
        return FN_OK_PTR(ret);
}

fn_result carbon_find_result_type(carbon_field_type_e *type, carbon_find *find)
{
        FN_FAIL_IF_NULL(type, find)
        FN_FAIL_FORWARD_IF_NOT_OK(carbon_path_evaluator_has_result(&find->path_evaluater));
        *type = find->type;
        return FN_OK();
}

fn_result carbon_find_update_array_type(carbon_find *find, carbon_list_derivable_e derivation)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_array_or_subtype(type)) {
                memfile mod;
                carbon_array_it *it = FN_PTR(carbon_array_it, carbon_find_result_array(find));
                memfile_clone(&mod, &it->memfile);
                memfile_seek_from_here(&mod, -sizeof(u8));
                carbon_derived_e derive_marker;
                carbon_abstract_derive_list_to(&derive_marker, CARBON_LIST_CONTAINER_ARRAY, derivation);
                carbon_abstract_write_derived_type(&mod, derive_marker);

                return FN_OK();

        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: array type update must be invoked on array or sub type");
        }
}

fn_result ofType(bool) carbon_find_array_is_multiset(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_array_or_subtype(type)) {
                carbon_array_it *it = FN_PTR(carbon_array_it, carbon_find_result_array(find));
                return carbon_array_it_is_multiset(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: array type query must be invoked on array or sub type");
        }
}

fn_result ofType(bool) carbon_find_array_is_sorted(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_array_or_subtype(type)) {
                carbon_array_it *it = FN_PTR(carbon_array_it, carbon_find_result_array(find));
                return carbon_array_it_is_sorted(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: array type query must be invoked on array or sub type");
        }
}

fn_result carbon_find_update_column_type(carbon_find *find, carbon_list_derivable_e derivation)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_column_or_subtype(type)) {
                carbon_column_it *it = FN_PTR(carbon_column_it, carbon_find_result_column(find));
                memfile_save_position(&it->memfile);
                memfile_seek(&it->memfile, it->column_start_offset);

                carbon_derived_e derive_marker;
                carbon_list_container_e list_container;
                carbon_list_container_type_by_column_type(&list_container, it->type);
                carbon_abstract_derive_list_to(&derive_marker, list_container, derivation);
                carbon_abstract_write_derived_type(&it->memfile, derive_marker);

                memfile_restore_position(&it->memfile);

                return FN_OK();

        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: column type update must be invoked on column or sub type");
        }
}

fn_result ofType(bool) carbon_find_column_is_multiset(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_column_or_subtype(type)) {
                carbon_column_it *it = FN_PTR(carbon_column_it, carbon_find_result_column(find));
                return carbon_column_it_is_multiset(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: column query must be invoked on column or sub type");
        }
}

fn_result ofType(bool) carbon_find_column_is_sorted(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_column_or_subtype(type)) {
                carbon_column_it *it = FN_PTR(carbon_column_it, carbon_find_result_column(find));
                return carbon_column_it_is_sorted(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: column query must be invoked on column or sub type");
        }
}

fn_result carbon_find_update_object_type(carbon_find *find, carbon_map_derivable_e derivation)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_object_or_subtype(type)) {
                carbon_object_it *it = FN_PTR(carbon_object_it, carbon_find_result_object(find));
                memfile_save_position(&it->memfile);
                memfile_seek(&it->memfile, it->object_start_off);

                carbon_derived_e derive_marker;
                carbon_abstract_derive_map_to(&derive_marker, derivation);
                carbon_abstract_write_derived_type(&it->memfile, derive_marker);

                memfile_restore_position(&it->memfile);

                return FN_OK();

        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: object type update must be invoked on object or sub type");
        }
}

fn_result ofType(bool) carbon_find_object_is_multimap(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_object_or_subtype(type)) {
                carbon_object_it *it = FN_PTR(carbon_object_it, carbon_find_result_object(find));
                return carbon_object_it_is_multimap(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: object query must be invoked on object or sub type");
        }
}

fn_result ofType(bool) carbon_find_object_is_sorted(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_object_or_subtype(type)) {
                carbon_object_it *it = FN_PTR(carbon_object_it, carbon_find_result_object(find));
                return carbon_object_it_is_sorted(it);
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "find: object query must be invoked on object or sub type");
        }
}

fn_result ofType(bool) carbon_find_multimap(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_object_or_subtype(type)) {
                return carbon_find_object_is_multimap(find);
        } else {
                return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_find_multiset(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_array_or_subtype(type)) {
                return carbon_find_array_is_multiset(find);
        } else if (carbon_field_type_is_column_or_subtype(type)) {
                return carbon_find_column_is_multiset(find);
        } else {
                return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_find_sorted(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_field_type_e type;
        carbon_find_result_type(&type, find);
        if (carbon_field_type_is_array_or_subtype(type)) {
                return carbon_find_array_is_sorted(find);
        } else if (carbon_field_type_is_column_or_subtype(type)) {
                return carbon_find_column_is_sorted(find);
        } else {
                return carbon_find_object_is_sorted(find);
        }
}


fn_result __check_path_evaluator_has_result(carbon_find *find)
{
        assert(find);
        if (UNLIKELY(!FN_IS_TRUE(carbon_path_evaluator_has_result(&find->path_evaluater)))) {
                return FN_FAIL(ERR_ILLEGALSTATE, "no path evaluation result available");
        } else {
                return FN_OK();
        }
}

fn_result ofType(carbon_array_it *) carbon_find_result_array(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_array_or_subtype(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "container must be array or sub type");
        }

        return FN_OK_PTR(find->value.array_it);
}

fn_result ofType(carbon_object_it *) carbon_find_result_object(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_object_or_subtype(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "container must be object or sub type");
        }

        return FN_OK_PTR(find->value.object_it);
}

fn_result ofType(carbon_column_it *) carbon_find_result_column(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_column_or_subtype(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "container must be column or sub type");
        }

        return FN_OK_PTR(find->value.column_it);
}

fn_result ofType(bool) carbon_find_result_boolean(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_boolean(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of boolean type");
        }

        return FN_OK_BOOL(find->value.boolean);
}

fn_result carbon_find_result_unsigned(u64 *out, carbon_find *find)
{
        FN_FAIL_IF_NULL(out, find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_unsigned(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of unsigned type");
        }

        *out = find->value.unsigned_number;
        return FN_OK();
}

fn_result carbon_find_result_signed(i64 *out, carbon_find *find)
{
        FN_FAIL_IF_NULL(out, find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_signed(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of signed type");
        }

        *out = find->value.signed_number;
        return FN_OK();
}

fn_result carbon_find_result_float(float *out, carbon_find *find)
{
        FN_FAIL_IF_NULL(out, find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_floating(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of float type");
        }

        *out = find->value.float_number;
        return FN_OK();
}

fn_result ofType(const char *) carbon_find_result_string(u64 *str_len, carbon_find *find)
{
        FN_FAIL_IF_NULL(str_len, find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_string(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of string type");
        }
        *str_len = find->value.string.len;
        return FN_OK_PTR(find->value.string.base);
}

fn_result ofType(carbon_binary *) carbon_find_result_binary(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        FN_FAIL_FORWARD_IF_NOT_OK(__check_path_evaluator_has_result(find));

        if (UNLIKELY(!carbon_field_type_is_binary(find->type))) {
                return FN_FAIL(ERR_TYPEMISMATCH, "result value must be of binary type");
        }

        return FN_OK_PTR(&find->value.binary);
}

fn_result carbon_find_drop(carbon_find *find)
{
        FN_FAIL_IF_NULL(find)
        carbon_path_evaluator_end(&find->path_evaluater);
        return FN_OK();
}

static void result_from_array(carbon_find *find, carbon_array_it *it)
{
        find->type = it->field_access.it_field_type;
        switch (find->type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /** no value to be stored */
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        find->value.array_it = carbon_array_it_array_value(it);
                        find->value.array_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        find->value.column_it = carbon_array_it_column_value(it);
                        find->value.column_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        find->value.object_it = carbon_array_it_object_value(it);
                        find->value.object_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_STRING:
                        find->value.string.base = carbon_array_it_string_value(&find->value.string.len, it);
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                        carbon_array_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                        carbon_array_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        carbon_array_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        carbon_array_it_binary_value(&find->value.binary, it);
                        break;
                default: ERROR(&find->err, ERR_INTERNALERR);
                        break;
        }
}

static void result_from_object(carbon_find *find, carbon_object_it *it)
{
        carbon_object_it_prop_type(&find->type, it);
        switch (find->type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /** no value to be stored */
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        find->value.array_it = carbon_object_it_array_value(it);
                        find->value.array_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        find->value.column_it = carbon_object_it_column_value(it);
                        find->value.column_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        find->value.object_it = carbon_object_it_object_value(it);
                        find->value.object_it->memfile.mode = find->doc->memfile.mode;
                        break;
                case CARBON_FIELD_STRING:
                        find->value.string.base = carbon_object_it_string_value(&find->value.string.len, it);
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                        carbon_object_it_unsigned_value(&find->value_is_nulled, &find->value.unsigned_number, it);
                        break;
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                        carbon_object_it_signed_value(&find->value_is_nulled, &find->value.signed_number, it);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        carbon_object_it_float_value(&find->value_is_nulled, &find->value.float_number, it);
                        break;
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        carbon_object_it_binary_value(&find->value.binary, it);
                        break;
                default: ERROR(&find->err, ERR_INTERNALERR);
                        break;
        }
}

static inline bool
result_from_column(carbon_find *find, u32 requested_idx, carbon_column_it *it)
{
        u32 num_contained_values;
        carbon_column_it_values_info(&find->type, &num_contained_values, it);
        JAK_ASSERT(requested_idx < num_contained_values);

        switch (find->type) {
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        u8 field_value = carbon_column_it_boolean_values(NULL, it)[requested_idx];
                        if (IS_NULL_BOOLEAN(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else if (field_value == CARBON_BOOLEAN_COLUMN_TRUE) {
                                find->type = CARBON_FIELD_TRUE;
                        } else if (field_value == CARBON_BOOLEAN_COLUMN_FALSE) {
                                find->type = CARBON_FIELD_FALSE;
                        } else {
                                ERROR(&it->err, ERR_INTERNALERR);
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET: {
                        u8 field_value = carbon_column_it_u8_values(NULL, it)[requested_idx];
                        if (IS_NULL_U8(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_U8;
                                find->value.unsigned_number = carbon_column_it_u8_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET: {
                        u16 field_value = carbon_column_it_u16_values(NULL, it)[requested_idx];
                        if (IS_NULL_U16(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_U16;
                                find->value.unsigned_number = carbon_column_it_u16_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET: {
                        u32 field_value = carbon_column_it_u32_values(NULL, it)[requested_idx];
                        if (IS_NULL_U32(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_U32;
                                find->value.unsigned_number = carbon_column_it_u32_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET: {
                        u64 field_value = carbon_column_it_u64_values(NULL, it)[requested_idx];
                        if (IS_NULL_U64(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_U64;
                                find->value.unsigned_number = carbon_column_it_u64_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET: {
                        i8 field_value = carbon_column_it_i8_values(NULL, it)[requested_idx];
                        if (IS_NULL_I8(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_I8;
                                find->value.signed_number = carbon_column_it_i8_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET: {
                        i16 field_value = carbon_column_it_i16_values(NULL, it)[requested_idx];
                        if (IS_NULL_I16(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_I16;
                                find->value.signed_number = carbon_column_it_i16_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET: {
                        i32 field_value = carbon_column_it_i32_values(NULL, it)[requested_idx];
                        if (IS_NULL_I32(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_I32;
                                find->value.signed_number = carbon_column_it_i32_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                        i64 field_value = carbon_column_it_i64_values(NULL, it)[requested_idx];
                        if (IS_NULL_I64(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_I64;
                                find->value.signed_number = carbon_column_it_i64_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET: {
                        float field_value = carbon_column_it_float_values(NULL, it)[requested_idx];
                        if (IS_NULL_FLOAT(field_value)) {
                                find->type = CARBON_FIELD_NULL;
                        } else {
                                find->type = CARBON_FIELD_NUMBER_FLOAT;
                                find->value.float_number = carbon_column_it_float_values(NULL, it)[requested_idx];
                        }
                }
                        break;
                default: ERROR(&it->err, ERR_UNSUPPORTEDTYPE)
                        return false;
        }
        return true;
}