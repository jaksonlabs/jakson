/**
 * Copyright 2019 Marcus Pinnecke
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

#include <carbon_abstract.h>
#include <jak_memfile.h>

fn_result carbon_abstract_type(carbon_abstract_e *type, jak_memfile *memfile)
{
        FN_FAIL_IF_NULL(memfile)
        carbon_derived_e derived;
        if (JAK_LIKELY(FN_IS_OK(carbon_abstract_get_derived_type(&derived, memfile)))) {
                switch (derived) {
                        case CARBON_UNSORTED_MULTIMAP:
                        case CARBON_UNSORTED_MULTISET_ARRAY:
                        case CARBON_UNSORTED_MULTISET_COL_U8:
                        case CARBON_UNSORTED_MULTISET_COL_U16:
                        case CARBON_UNSORTED_MULTISET_COL_U32:
                        case CARBON_UNSORTED_MULTISET_COL_U64:
                        case CARBON_UNSORTED_MULTISET_COL_I8:
                        case CARBON_UNSORTED_MULTISET_COL_I16:
                        case CARBON_UNSORTED_MULTISET_COL_I32:
                        case CARBON_UNSORTED_MULTISET_COL_I64:
                        case CARBON_UNSORTED_MULTISET_COL_FLOAT:
                        case CARBON_UNSORTED_MULTISET_COL_BOOLEAN:
                                JAK_OPTIONAL_SET(type, CARBON_ABSTRACT_BASE);
                                goto return_true;
                        case CARBON_SORTED_MULTIMAP:
                        case CARBON_UNSORTED_MAP:
                        case CARBON_SORTED_MAP:
                        case CARBON_SORTED_MULTISET_ARRAY:
                        case CARBON_UNSORTED_SET_ARRAY:
                        case CARBON_SORTED_SET_ARRAY:
                        case CARBON_SORTED_MULTISET_COL_U8:
                        case CARBON_UNSORTED_SET_COL_U8:
                        case CARBON_SORTED_SET_COL_U8:
                        case CARBON_SORTED_MULTISET_COL_U16:
                        case CARBON_UNSORTED_SET_COL_U16:
                        case CARBON_SORTED_SET_COL_U16:
                        case CARBON_SORTED_MULTISET_COL_U32:
                        case CARBON_UNSORTED_SET_COL_U32:
                        case CARBON_SORTED_SET_COL_U32:
                        case CARBON_SORTED_MULTISET_COL_U64:
                        case CARBON_UNSORTED_SET_COL_U64:
                        case CARBON_SORTED_SET_COL_U64:
                        case CARBON_SORTED_MULTISET_COL_I8:
                        case CARBON_UNSORTED_SET_COL_I8:
                        case CARBON_SORTED_SET_COL_I8:
                        case CARBON_SORTED_MULTISET_COL_I16:
                        case CARBON_UNSORTED_SET_COL_I16:
                        case CARBON_SORTED_SET_COL_I16:
                        case CARBON_SORTED_MULTISET_COL_I32:
                        case CARBON_UNSORTED_SET_COL_I32:
                        case CARBON_SORTED_SET_COL_I32:
                        case CARBON_SORTED_MULTISET_COL_I64:
                        case CARBON_UNSORTED_SET_COL_I64:
                        case CARBON_SORTED_SET_COL_I64:
                        case CARBON_SORTED_MULTISET_COL_FLOAT:
                        case CARBON_UNSORTED_SET_COL_FLOAT:
                        case CARBON_SORTED_SET_COL_FLOAT:
                        case CARBON_SORTED_MULTISET_COL_BOOLEAN:
                        case CARBON_UNSORTED_SET_COL_BOOLEAN:
                        case CARBON_SORTED_SET_COL_BOOLEAN:
                                JAK_OPTIONAL_SET(type, CARBON_ABSTRACT_DERIVED);
                                goto return_true;
                        default:
                                return FN_FAIL(JAK_ERR_MARKERMAPPING, "unknown abstract type marker detected");
                }
return_true:
                return FN_OK();
        } else {
                return FN_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_base(jak_memfile *memfile)
{
        carbon_abstract_e type;
        if (JAK_LIKELY(FN_IS_OK(carbon_abstract_type(&type, memfile)))) {
                return FN_OK_BOOL(type == CARBON_ABSTRACT_BASE);
        } else {
                return FN_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_derived(jak_memfile *memfile)
{
        fn_result ofType(bool) ret = carbon_abstract_is_base(memfile);
        bool result = FN_BOOL(ret);
        return FN_OK_BOOL(!result);
}

fn_result carbon_abstract_get_class(carbon_abstract_type_class_e *type, jak_memfile *memfile)
{
        FN_FAIL_IF_NULL(type, memfile)
        carbon_derived_e derived;
        if (JAK_LIKELY(FN_IS_OK(carbon_abstract_get_derived_type(&derived, memfile)))) {
                switch (derived) {
                        case CARBON_SORTED_MAP:
                                *type = CARBON_TYPE_SORTED_MAP;
                                goto return_true;
                        case CARBON_SORTED_MULTIMAP:
                                *type = CARBON_TYPE_SORTED_MULTIMAP;
                                goto return_true;
                        case CARBON_SORTED_MULTISET_ARRAY:
                        case CARBON_SORTED_MULTISET_COL_BOOLEAN:
                        case CARBON_SORTED_MULTISET_COL_FLOAT:
                        case CARBON_SORTED_MULTISET_COL_I16:
                        case CARBON_SORTED_MULTISET_COL_I32:
                        case CARBON_SORTED_MULTISET_COL_I64:
                        case CARBON_SORTED_MULTISET_COL_I8:
                        case CARBON_SORTED_MULTISET_COL_U16:
                        case CARBON_SORTED_MULTISET_COL_U32:
                        case CARBON_SORTED_MULTISET_COL_U64:
                        case CARBON_SORTED_MULTISET_COL_U8:
                                *type = CARBON_TYPE_SORTED_MULTISET;
                                goto return_true;
                        case CARBON_SORTED_SET_ARRAY:
                        case CARBON_SORTED_SET_COL_BOOLEAN:
                        case CARBON_SORTED_SET_COL_FLOAT:
                        case CARBON_SORTED_SET_COL_I16:
                        case CARBON_SORTED_SET_COL_I32:
                        case CARBON_SORTED_SET_COL_I64:
                        case CARBON_SORTED_SET_COL_I8:
                        case CARBON_SORTED_SET_COL_U16:
                        case CARBON_SORTED_SET_COL_U32:
                        case CARBON_SORTED_SET_COL_U64:
                        case CARBON_SORTED_SET_COL_U8:
                                *type = CARBON_TYPE_SORTED_SET;
                                goto return_true;
                        case CARBON_UNSORTED_MAP:
                                *type = CARBON_TYPE_UNSORTED_MAP;
                                goto return_true;
                        case CARBON_UNSORTED_MULTIMAP:
                                *type = CARBON_TYPE_UNSORTED_MULTIMAP;
                                goto return_true;
                        case CARBON_UNSORTED_MULTISET_ARRAY:
                        case CARBON_UNSORTED_MULTISET_COL_BOOLEAN:
                        case CARBON_UNSORTED_MULTISET_COL_FLOAT:
                        case CARBON_UNSORTED_MULTISET_COL_I16:
                        case CARBON_UNSORTED_MULTISET_COL_I32:
                        case CARBON_UNSORTED_MULTISET_COL_I64:
                        case CARBON_UNSORTED_MULTISET_COL_I8:
                        case CARBON_UNSORTED_MULTISET_COL_U16:
                        case CARBON_UNSORTED_MULTISET_COL_U32:
                        case CARBON_UNSORTED_MULTISET_COL_U64:
                        case CARBON_UNSORTED_MULTISET_COL_U8:
                                *type = CARBON_TYPE_UNSORTED_MULTISET;
                                goto return_true;
                        case CARBON_UNSORTED_SET_ARRAY:
                        case CARBON_UNSORTED_SET_COL_BOOLEAN:
                        case CARBON_UNSORTED_SET_COL_FLOAT:
                        case CARBON_UNSORTED_SET_COL_I16:
                        case CARBON_UNSORTED_SET_COL_I32:
                        case CARBON_UNSORTED_SET_COL_I64:
                        case CARBON_UNSORTED_SET_COL_I8:
                        case CARBON_UNSORTED_SET_COL_U16:
                        case CARBON_UNSORTED_SET_COL_U32:
                        case CARBON_UNSORTED_SET_COL_U64:
                        case CARBON_UNSORTED_SET_COL_U8:
                                *type = CARBON_TYPE_UNSORTED_SET;
                                goto return_true;
                        default:
                                return FN_FAIL(JAK_ERR_MARKERMAPPING, "unknown marker detected");
                }
                return_true:
                return FN_OK();
        } else {
                return FN_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_multiset(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_MULTISET:
                case CARBON_TYPE_SORTED_MULTISET:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_set(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_SET:
                case CARBON_TYPE_SORTED_SET:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_multimap(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_MULTIMAP:
                case CARBON_TYPE_SORTED_MULTIMAP:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_map(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_UNSORTED_MAP:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_sorted(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_SORTED_MULTISET:
                case CARBON_TYPE_SORTED_SET:
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_SORTED_MULTIMAP:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_distinct(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_SET:
                case CARBON_TYPE_SORTED_SET:
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_UNSORTED_MAP:
                        return FN_OK_TRUE();
                default:
                        return FN_OK_FALSE();
        }
}

fn_result carbon_abstract_class_to_list_derivable(carbon_list_derivable_e *out, carbon_abstract_type_class_e in)
{
        FN_FAIL_IF_NULL(out)
        switch (in) {
                case CARBON_TYPE_UNSORTED_MULTISET:
                        *out = CARBON_LIST_UNSORTED_MULTISET;
                        break;
                case CARBON_TYPE_SORTED_MULTISET:
                        *out = CARBON_LIST_SORTED_MULTISET;
                        break;
                case CARBON_TYPE_UNSORTED_SET:
                        *out = CARBON_LIST_UNSORTED_SET;
                        break;
                case CARBON_TYPE_SORTED_SET:
                        *out = CARBON_LIST_SORTED_SET;
                        break;
                default:
                        return FN_FAIL(JAK_ERR_TYPEMISMATCH, "abstract class type does not encode a list type");
        }
        return FN_OK();
}

fn_result carbon_abstract_write_base_type(jak_memfile *memfile, jak_carbon_container_sub_type_e type)
{
        FN_FAIL_IF_NULL(memfile)
        jak_memfile_write(memfile, &type, sizeof(jak_u8));
        return FN_OK();
}

fn_result carbon_abstract_write_derived_type(jak_memfile *memfile, carbon_derived_e type)
{
        FN_FAIL_IF_NULL(type, memfile)
        jak_memfile_write(memfile, &type, sizeof(jak_u8));
        return FN_OK();
}

fn_result carbon_abstract_get_container_subtype(jak_carbon_container_sub_type_e *type, jak_memfile *memfile)
{
        FN_FAIL_IF_NULL(type, memfile)
        jak_u8 marker = jak_memfile_peek_byte(memfile);
        switch (marker) {
                /* abstract types for object containers */
                case CARBON_UNSORTED_MULTIMAP:
                case CARBON_SORTED_MULTIMAP:
                case CARBON_UNSORTED_MAP:
                case CARBON_SORTED_MAP:
                        *type = CARBON_CONTAINER_OBJECT;
                        return FN_OK();
                /* abstract types for array containers */
                case CARBON_UNSORTED_MULTISET_ARRAY:
                case CARBON_SORTED_MULTISET_ARRAY:
                case CARBON_UNSORTED_SET_ARRAY:
                case CARBON_SORTED_SET_ARRAY:
                        *type = CARBON_CONTAINER_ARRAY;
                        return FN_OK();
                /* abstract types for column-u8 containers */
                case CARBON_UNSORTED_MULTISET_COL_U8:
                case CARBON_SORTED_MULTISET_COL_U8:
                case CARBON_UNSORTED_SET_COL_U8:
                case CARBON_SORTED_SET_COL_U8:
                        *type = CARBON_CONTAINER_COLUMN_U8;
                        return FN_OK();
                /* abstract types for column-u16 containers */
                case CARBON_UNSORTED_MULTISET_COL_U16:
                case CARBON_SORTED_MULTISET_COL_U16:
                case CARBON_UNSORTED_SET_COL_U16:
                case CARBON_SORTED_SET_COL_U16:
                        *type = CARBON_CONTAINER_COLUMN_U16;
                        return FN_OK();
                /* abstract types for column-u32 containers */
                case CARBON_UNSORTED_MULTISET_COL_U32:
                case CARBON_SORTED_MULTISET_COL_U32:
                case CARBON_UNSORTED_SET_COL_U32:
                case CARBON_SORTED_SET_COL_U32:
                        *type = CARBON_CONTAINER_COLUMN_U32;
                        return FN_OK();
                /* abstract types for column-u64 containers */
                case CARBON_UNSORTED_MULTISET_COL_U64:
                case CARBON_SORTED_MULTISET_COL_U64:
                case CARBON_UNSORTED_SET_COL_U64:
                case CARBON_SORTED_SET_COL_U64:
                        *type = CARBON_CONTAINER_COLUMN_U64;
                        return FN_OK();
                /* abstract types for column-i8 containers */
                case CARBON_UNSORTED_MULTISET_COL_I8:
                case CARBON_SORTED_MULTISET_COL_I8:
                case CARBON_UNSORTED_SET_COL_I8:
                case CARBON_SORTED_SET_COL_I8:
                        *type = CARBON_CONTAINER_COLUMN_I8;
                        return FN_OK();
                /* abstract types for column-i16 containers */
                case CARBON_UNSORTED_MULTISET_COL_I16:
                case CARBON_SORTED_MULTISET_COL_I16:
                case CARBON_UNSORTED_SET_COL_I16:
                case CARBON_SORTED_SET_COL_I16:
                        *type = CARBON_CONTAINER_COLUMN_I16;
                        return FN_OK();
                /* abstract types for column-i32 containers */
                case CARBON_UNSORTED_MULTISET_COL_I32:
                case CARBON_SORTED_MULTISET_COL_I32:
                case CARBON_UNSORTED_SET_COL_I32:
                case CARBON_SORTED_SET_COL_I32:
                        *type = CARBON_CONTAINER_COLUMN_I32;
                        return FN_OK();
                /* abstract types for column-i64 containers */
                case CARBON_UNSORTED_MULTISET_COL_I64:
                case CARBON_SORTED_MULTISET_COL_I64:
                case CARBON_UNSORTED_SET_COL_I64:
                case CARBON_SORTED_SET_COL_I64:
                        *type = CARBON_CONTAINER_COLUMN_I64;
                        return FN_OK();
                /* abstract types for column-float containers */
                case CARBON_UNSORTED_MULTISET_COL_FLOAT:
                case CARBON_SORTED_MULTISET_COL_FLOAT:
                case CARBON_UNSORTED_SET_COL_FLOAT:
                case CARBON_SORTED_SET_COL_FLOAT:
                        *type = CARBON_CONTAINER_COLUMN_FLOAT;
                        return FN_OK();
                /* abstract types for column-boolean containers */
                case CARBON_UNSORTED_MULTISET_COL_BOOLEAN:
                case CARBON_SORTED_MULTISET_COL_BOOLEAN:
                case CARBON_UNSORTED_SET_COL_BOOLEAN:
                case CARBON_SORTED_SET_COL_BOOLEAN:
                        *type = CARBON_CONTAINER_COLUMN_BOOLEAN;
                        return FN_OK();
                default:
                        return FN_FAIL(JAK_ERR_MARKERMAPPING, "unknown marker encoding an abstract type");
        }
        return FN_OK();
}

static fn_result ofType(bool) __carbon_abstract_is_instanceof(jak_memfile *memfile, jak_carbon_container_sub_type_e T)
{
        jak_carbon_container_sub_type_e type;
        if (JAK_LIKELY(FN_IS_OK(carbon_abstract_get_container_subtype(&type, memfile)))) {
                return FN_OK_BOOL(type == T);
        } else {
                return FN_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_instanceof_object(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_OBJECT);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_array(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_ARRAY);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_u8(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_U8);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_u16(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_U16);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_u32(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_U32);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_u64(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_U64);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_i8(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_I8);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_i16(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_I16);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_i32(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_I32);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_i64(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_I64);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_float(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_FLOAT);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column_boolean(jak_memfile *memfile)
{
        return __carbon_abstract_is_instanceof(memfile, CARBON_CONTAINER_COLUMN_BOOLEAN);
}

fn_result ofType(bool) carbon_abstract_is_instanceof_column(jak_memfile *memfile)
{
       if (FN_IS_TRUE(carbon_abstract_is_instanceof_column_u8(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_u16(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_u32(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_u64(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_i8(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_i16(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_i32(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_i64(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_float(memfile)) ||
                FN_IS_TRUE(carbon_abstract_is_instanceof_column_boolean(memfile))) {
                return FN_OK_TRUE();
        } else {
                return FN_OK_FALSE();
        }
}

fn_result ofType(bool) carbon_abstract_is_instanceof_list(jak_memfile *memfile)
{
        if (FN_IS_TRUE(carbon_abstract_is_instanceof_array(memfile)) ||
            FN_IS_TRUE(carbon_abstract_is_instanceof_column(memfile))) {
                return FN_OK_TRUE();
        } else {
                return FN_OK_FALSE();
        }
}

fn_result carbon_abstract_derive_list_to(carbon_derived_e *concrete, jak_carbon_list_container_e is,
                                         carbon_list_derivable_e should)
{
        FN_FAIL_IF_NULL(concrete)
        switch (is) {
                case CARBON_LIST_CONTAINER_ARRAY:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_ARRAY;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_ARRAY;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_ARRAY;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_ARRAY;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_U8:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_U8;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_U8;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_U8;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_U8;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_U16:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_U16;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_U16;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_U16;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_U16;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_U32:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_U32;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_U32;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_U32;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_U32;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_U64:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_U64;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_U64;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_U64;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_U64;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_I8:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_I8;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_I8;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_I8;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_I8;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_I16:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_I16;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_I16;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_I16;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_I16;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_I32:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_I32;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_I32;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_I32;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_I32;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_I64:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_I64;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_I64;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_I64;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_I64;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_BOOLEAN:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_BOOLEAN;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_BOOLEAN;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_BOOLEAN;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_BOOLEAN;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                case CARBON_LIST_CONTAINER_COLUMN_FLOAT:
                        switch (should) {
                                case CARBON_LIST_UNSORTED_MULTISET:
                                        *concrete = CARBON_UNSORTED_MULTISET_COL_FLOAT;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_MULTISET:
                                        *concrete = CARBON_SORTED_MULTISET_COL_FLOAT;
                                        return FN_OK();
                                case CARBON_LIST_UNSORTED_SET:
                                        *concrete = CARBON_UNSORTED_SET_COL_FLOAT;
                                        return FN_OK();
                                case CARBON_LIST_SORTED_SET:
                                        *concrete = CARBON_SORTED_SET_COL_FLOAT;
                                        return FN_OK();
                                default:
                                        goto error_case;
                        }
                default:
                        goto error_case;
        }
error_case:
        return FN_FAIL(JAK_ERR_INTERNALERR, "unknown list container type");
}

fn_result carbon_abstract_derive_map_to(carbon_derived_e *concrete, carbon_map_derivable_e should)
{
        FN_FAIL_IF_NULL(concrete)
        switch (should) {
                case CARBON_MAP_UNSORTED_MULTIMAP:
                        *concrete = CARBON_UNSORTED_MULTIMAP;
                        return FN_OK();
                case CARBON_MAP_SORTED_MULTIMAP:
                        *concrete = CARBON_SORTED_MULTIMAP;
                        return FN_OK();
                case CARBON_MAP_UNSORTED_MAP:
                        *concrete = CARBON_UNSORTED_MAP;
                        return FN_OK();
                case CARBON_MAP_SORTED_MAP:
                        *concrete = CARBON_SORTED_MAP;
                        return FN_OK();
                default:
                        return FN_FAIL(JAK_ERR_INTERNALERR, "unknown list container type");
        }
}

fn_result carbon_abstract_get_derived_type(carbon_derived_e *type, jak_memfile *memfile)
{
        FN_FAIL_IF_NULL(type, memfile)
        jak_u8 c = jak_memfile_peek_byte(memfile);
        if (!(c == CARBON_MUNSORTED_MULTIMAP || c == CARBON_MSORTED_MULTIMAP || c == CARBON_MUNSORTED_MAP ||
                       c == CARBON_MSORTED_MAP || c == CARBON_MUNSORTED_MULTISET_ARR ||
                       c == CARBON_MSORTED_MULTISET_ARR || c == CARBON_MUNSORTED_SET_ARR ||
                       c == CARBON_MSORTED_SET_ARR || c == CARBON_MUNSORTED_MULTISET_U8 ||
                       c == CARBON_MSORTED_MULTISET_U8 || c == CARBON_MUNSORTED_SET_U8 ||
                       c == CARBON_MSORTED_SET_U8 || c == CARBON_MUNSORTED_MULTISET_U16 ||
                       c == CARBON_MSORTED_MULTISET_U16 || c == CARBON_MUNSORTED_SET_U16 ||
                       c == CARBON_MSORTED_SET_U16 || c == CARBON_MUNSORTED_MULTISET_U32 ||
                       c == CARBON_MSORTED_MULTISET_U32 || c == CARBON_MUNSORTED_SET_U32 ||
                       c == CARBON_MSORTED_SET_U32 || c == CARBON_MUNSORTED_MULTISET_U64 ||
                       c == CARBON_MSORTED_MULTISET_U64 || c == CARBON_MUNSORTED_SET_U64 ||
                       c == CARBON_MSORTED_SET_U64 || c == CARBON_MUNSORTED_MULTISET_I8 ||
                       c == CARBON_MSORTED_MULTISET_I8 || c == CARBON_MUNSORTED_SET_I8 ||
                       c == CARBON_MSORTED_SET_I8 || c == CARBON_MUNSORTED_MULTISET_I16 ||
                       c == CARBON_MSORTED_MULTISET_I16 || c == CARBON_MUNSORTED_SET_I16 ||
                       c == CARBON_MSORTED_SET_I16 || c == CARBON_MUNSORTED_MULTISET_I32 ||
                       c == CARBON_MSORTED_MULTISET_I32 || c == CARBON_MUNSORTED_SET_I32 ||
                       c == CARBON_MSORTED_SET_I32 || c == CARBON_MUNSORTED_MULTISET_I64 ||
                       c == CARBON_MSORTED_MULTISET_I64 || c == CARBON_MUNSORTED_SET_I64 ||
                       c == CARBON_MSORTED_SET_I64 || c == CARBON_MUNSORTED_MULTISET_FLOAT ||
                       c == CARBON_MSORTED_MULTISET_FLOAT || c == CARBON_MUNSORTED_SET_FLOAT ||
                       c == CARBON_MSORTED_SET_FLOAT || c == CARBON_MUNSORTED_MULTISET_BOOLEAN ||
                       c == CARBON_MSORTED_MULTISET_BOOLEAN || c == CARBON_MUNSORTED_SET_BOOLEAN ||
                       c == CARBON_MSORTED_SET_BOOLEAN)) {
                return FN_FAIL(JAK_ERR_MARKERMAPPING, "unknown marker for abstract derived type");
        }
        *type = c;
        return FN_OK();
}