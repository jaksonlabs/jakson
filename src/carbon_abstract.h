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

#ifndef CARBON_ABSTRACT
#define CARBON_ABSTRACT

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_carbon.h>
#include <jak_fn_result.h>

JAK_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  abstract type class (base or derived)
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_abstract {
        /* Does not need further treatment to guarantee properties (unsorted, and not duplicate-free) */
        CARBON_ABSTRACT_BASE,
        /* particular abstract type with further properties (such as uniqueness of contained elements), enabling the
        * application to check certain promises and guarantees */
        CARBON_ABSTRACT_DERIVED,
} carbon_abstract_e;

fn_result carbon_abstract_type(carbon_abstract_e *type, jak_memfile *memfile);
fn_result ofType(bool) carbon_abstract_is_base(jak_memfile *memfile);
fn_result ofType(bool) carbon_abstract_is_derived(jak_memfile *memfile);

// ---------------------------------------------------------------------------------------------------------------------
//  abstract type (multiset, set, sorted or unsorted)
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_abstract_type_class {
        /* abstract base types */
        CARBON_TYPE_UNSORTED_MULTISET,     /* element type: values, distinct elements: no, sorted: no */
        CARBON_TYPE_UNSORTED_MULTIMAP,     /* element type: pairs, distinct elements: no, sorted: no */

        /* derived abstract types */
        CARBON_TYPE_SORTED_MULTISET,       /* element type: values, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_SET,          /* element type: values, distinct elements: yes, sorted: no */
        CARBON_TYPE_SORTED_SET,            /* element type: values, distinct elements: yes, sorted: yes */
        CARBON_TYPE_SORTED_MAP,            /* element type: pairs, distinct elements: yes, sorted: yes */
        CARBON_TYPE_SORTED_MULTIMAP,       /* element type: pairs, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_MAP           /* element type: pairs, distinct elements: yes, sorted: no */
} carbon_abstract_type_class_e;

fn_result carbon_abstract_get_class(carbon_abstract_type_class_e *type, jak_memfile *memfile);
fn_result ofType(bool) carbon_abstract_is_multiset(carbon_abstract_type_class_e type);
fn_result ofType(bool) carbon_abstract_is_set(carbon_abstract_type_class_e type);
fn_result ofType(bool) carbon_abstract_is_multimap(carbon_abstract_type_class_e type);
fn_result ofType(bool) carbon_abstract_is_map(carbon_abstract_type_class_e type);
fn_result ofType(bool) carbon_abstract_is_sorted(carbon_abstract_type_class_e type);
fn_result ofType(bool) carbon_abstract_is_distinct(carbon_abstract_type_class_e type);

// ---------------------------------------------------------------------------------------------------------------------
//  derived type (actual abstract type and which container is used)
// ---------------------------------------------------------------------------------------------------------------------

typedef enum carbon_derived {
        /* abstract types for object containers */
        CARBON_UNSORTED_MULTIMAP = CARBON_MUNSORTED_MULTIMAP,
        CARBON_SORTED_MULTIMAP = CARBON_MSORTED_MULTIMAP,
        CARBON_UNSORTED_MAP = CARBON_MUNSORTED_MAP,
        CARBON_SORTED_MAP = CARBON_MSORTED_MAP,

        /* abstract types for array containers */
        CARBON_UNSORTED_MULTISET_ARRAY = CARBON_MUNSORTED_MULTISET_ARR,
        CARBON_SORTED_MULTISET_ARRAY = CARBON_MSORTED_MULTISET_ARR,
        CARBON_UNSORTED_SET_ARRAY = CARBON_MUNSORTED_SET_ARR,
        CARBON_SORTED_SET_ARRAY = CARBON_MSORTED_SET_ARR,

        /* abstract types for column-u8 containers */
        CARBON_UNSORTED_MULTISET_COL_U8 = CARBON_MUNSORTED_MULTISET_U8,
        CARBON_SORTED_MULTISET_COL_U8 = CARBON_MSORTED_MULTISET_U8,
        CARBON_UNSORTED_SET_COL_U8 = CARBON_MUNSORTED_SET_U8,
        CARBON_SORTED_SET_COL_U8 = CARBON_MSORTED_SET_U8,

        /* abstract types for column-u16 containers */
        CARBON_UNSORTED_MULTISET_COL_U16 = CARBON_MUNSORTED_MULTISET_U16,
        CARBON_SORTED_MULTISET_COL_U16 = CARBON_MSORTED_MULTISET_U16,
        CARBON_UNSORTED_SET_COL_U16 = CARBON_MUNSORTED_SET_U16,
        CARBON_SORTED_SET_COL_U16 = CARBON_MSORTED_SET_U16,

        /* abstract types for column-u32 containers */
        CARBON_UNSORTED_MULTISET_COL_U32 = CARBON_MUNSORTED_MULTISET_U32,
        CARBON_SORTED_MULTISET_COL_U32 = CARBON_MSORTED_MULTISET_U32,
        CARBON_UNSORTED_SET_COL_U32 = CARBON_MUNSORTED_SET_U32,
        CARBON_SORTED_SET_COL_U32 = CARBON_MSORTED_SET_U32,

        /* abstract types for column-u64 containers */
        CARBON_UNSORTED_MULTISET_COL_U64 = CARBON_MUNSORTED_MULTISET_U64,
        CARBON_SORTED_MULTISET_COL_U64 = CARBON_MSORTED_MULTISET_U64,
        CARBON_UNSORTED_SET_COL_U64 = CARBON_MUNSORTED_SET_U64,
        CARBON_SORTED_SET_COL_U64 = CARBON_MSORTED_SET_U64,

        /* abstract types for column-i8 containers */
        CARBON_UNSORTED_MULTISET_COL_I8 = CARBON_MUNSORTED_MULTISET_I8,
        CARBON_SORTED_MULTISET_COL_I8 = CARBON_MSORTED_MULTISET_I8,
        CARBON_UNSORTED_SET_COL_I8 = CARBON_MUNSORTED_SET_I8,
        CARBON_SORTED_SET_COL_I8 = CARBON_MSORTED_SET_I8,

        /* abstract types for column-i16 containers */
        CARBON_UNSORTED_MULTISET_COL_I16 = CARBON_MUNSORTED_MULTISET_I16,
        CARBON_SORTED_MULTISET_COL_I16 = CARBON_MSORTED_MULTISET_I16,
        CARBON_UNSORTED_SET_COL_I16 = CARBON_MUNSORTED_SET_I16,
        CARBON_SORTED_SET_COL_I16 = CARBON_MSORTED_SET_I16,

        /* abstract types for column-i32 containers */
        CARBON_UNSORTED_MULTISET_COL_I32 = CARBON_MUNSORTED_MULTISET_I32,
        CARBON_SORTED_MULTISET_COL_I32 = CARBON_MSORTED_MULTISET_I32,
        CARBON_UNSORTED_SET_COL_I32 = CARBON_MUNSORTED_SET_I32,
        CARBON_SORTED_SET_COL_I32 = CARBON_MSORTED_SET_I32,

        /* abstract types for column-i64 containers */
        CARBON_UNSORTED_MULTISET_COL_I64 = CARBON_MUNSORTED_MULTISET_I64,
        CARBON_SORTED_MULTISET_COL_I64 = CARBON_MSORTED_MULTISET_I64,
        CARBON_UNSORTED_SET_COL_I64 = CARBON_MUNSORTED_SET_I64,
        CARBON_SORTED_SET_COL_I64 = CARBON_MSORTED_SET_I64,

        /* abstract types for column-float containers */
        CARBON_UNSORTED_MULTISET_COL_FLOAT = CARBON_MUNSORTED_MULTISET_FLOAT,
        CARBON_SORTED_MULTISET_COL_FLOAT = CARBON_MSORTED_MULTISET_FLOAT,
        CARBON_UNSORTED_SET_COL_FLOAT = CARBON_MUNSORTED_SET_FLOAT,
        CARBON_SORTED_SET_COL_FLOAT = CARBON_MSORTED_SET_FLOAT,

        /* abstract types for column-boolean containers */
        CARBON_UNSORTED_MULTISET_COL_BOOLEAN = CARBON_MUNSORTED_MULTISET_BOOLEAN,
        CARBON_SORTED_MULTISET_COL_BOOLEAN = CARBON_MSORTED_MULTISET_BOOLEAN,
        CARBON_UNSORTED_SET_COL_BOOLEAN = CARBON_MUNSORTED_SET_BOOLEAN,
        CARBON_SORTED_SET_COL_BOOLEAN = CARBON_MSORTED_SET_BOOLEAN
} carbon_derived_e;

fn_result carbon_abstract_get_derived_type(carbon_derived_e *type, jak_memfile *memfile);

JAK_END_DECL

#endif