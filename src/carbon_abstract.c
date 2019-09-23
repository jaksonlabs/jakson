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

fn_result carbon_abstract_type(carbon_abstract_e *type, jak_memfile *memfile)
{
        JAK_NONULL_OR_FAIL(memfile)
        carbon_derived_e derived;
        if (JAK_LIKELY(JAK_IS_OK(carbon_abstract_get_derived_type(&derived, memfile)))) {
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
                                return JAK_OK_BOOL(false);
                }
return_true:
                return JAK_OK_BOOL(true);
        } else {
                return JAK_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_base(jak_memfile *memfile)
{
        carbon_abstract_e type;
        if (JAK_LIKELY(JAK_IS_OK(carbon_abstract_type(&type, memfile)))) {
                return JAK_OK_BOOL(type == CARBON_ABSTRACT_BASE);
        } else {
                return JAK_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_derived(jak_memfile *memfile)
{
        fn_result ofType(bool) ret = carbon_abstract_is_base(memfile);
        bool result = JAK_BOOL(ret);
        return JAK_OK_BOOL(!result);
}

fn_result carbon_abstract_get_class(carbon_abstract_type_class_e *type, jak_memfile *memfile)
{
        JAK_NONULL_OR_FAIL(type, memfile)
        carbon_derived_e derived;
        if (JAK_LIKELY(JAK_IS_OK(carbon_abstract_get_derived_type(&derived, memfile)))) {
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
                                return JAK_OK_BOOL(false);
                }
                return_true:
                return JAK_OK_BOOL(true);
        } else {
                return JAK_FAIL_FORWARD();
        }
}

fn_result ofType(bool) carbon_abstract_is_multiset(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_MULTISET:
                case CARBON_TYPE_SORTED_MULTISET:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result ofType(bool) carbon_abstract_is_set(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_SET:
                case CARBON_TYPE_SORTED_SET:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result ofType(bool) carbon_abstract_is_multimap(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_MULTIMAP:
                case CARBON_TYPE_SORTED_MULTIMAP:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result ofType(bool) carbon_abstract_is_map(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_UNSORTED_MAP:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result ofType(bool) carbon_abstract_is_sorted(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_SORTED_MULTISET:
                case CARBON_TYPE_SORTED_SET:
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_SORTED_MULTIMAP:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result ofType(bool) carbon_abstract_is_distinct(carbon_abstract_type_class_e type)
{
        switch (type) {
                case CARBON_TYPE_UNSORTED_SET:
                case CARBON_TYPE_SORTED_SET:
                case CARBON_TYPE_SORTED_MAP:
                case CARBON_TYPE_UNSORTED_MAP:
                        return JAK_OK_BOOL(true);
                default:
                        return JAK_OK_BOOL(false);
        }
}

fn_result carbon_abstract_get_derived_type(carbon_derived_e *type, jak_memfile *memfile)
{
        JAK_NONULL_OR_FAIL(type, memfile)
        jak_u8 c = jak_memfile_peek_byte(memfile);
        assert(c == CARBON_MUNSORTED_MULTIMAP || c == CARBON_MSORTED_MULTIMAP || c == CARBON_MUNSORTED_MAP ||
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
                       c == CARBON_MSORTED_SET_BOOLEAN);
        return JAK_OK();
}