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

#include <jakson/stdinc.h>
#include <jakson/carbon/markers.h>
#include <jakson/carbon/containers.h>
#include <jakson/fn_result.h>

BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  abstract type class (base or derived)
// ---------------------------------------------------------------------------------------------------------------------

/** derivation type of any abstract type */
typedef enum carbon_abstract {
        /** Does not need further treatment to guarantee properties (unsorted, and not duplicate-free) */
        CARBON_ABSTRACT_BASE,
        /** particular abstract type with further properties (such as uniqueness of contained elements), enabling the
        * application to check certain promises and guarantees */
        CARBON_ABSTRACT_DERIVED,
} carbon_abstract_e;

/** Reads the abstract type from the memory file without moving the memory file cursors. This function translates
 * from a particular derived container (e.g., CARBON_UNSORTED_MULTISET_COL_U8, or CARBON_SORTED_MULTIMAP)
 * to its abstract type (e.g., CARBON_ABSTRACT_BASE resp. CARBON_ABSTRACT_DERIVED) */
fn_result carbon_abstract_type(carbon_abstract_e *type, memfile *memfile);

/** Calls carbon_abstract_type and returns true in case of an abstract base type for a particular
 * derived container marker that is read from the current position of the mem without moving
 * the memory files cursor.
 *
 * In case of success, a boolean value is returned, indicating whether the particular container marker is an
 * abstract base type, or not.
 *
 * In case of any failure (such as the read maker does not belong to any known derived container), the function
 * returns an err. */
fn_result ofType(bool) carbon_abstract_is_base(memfile *memfile);

/** Calls carbon_abstract_is_base and negates its result */
fn_result ofType(bool) carbon_abstract_is_derived(memfile *memfile);

// ---------------------------------------------------------------------------------------------------------------------
//  abstract type (multiset, set, sorted or unsorted)
// ---------------------------------------------------------------------------------------------------------------------

/** class of an abstract type independent of a particular abstract derived container */
typedef enum carbon_abstract_type_class {
        /** abstract base types */
        CARBON_TYPE_UNSORTED_MULTISET,     /** element type: values, distinct elements: no, sorted: no */
        CARBON_TYPE_UNSORTED_MULTIMAP,     /** element type: pairs, distinct elements: no, sorted: no */

        /** derived abstract types */
        CARBON_TYPE_SORTED_MULTISET,       /** element type: values, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_SET,          /** element type: values, distinct elements: yes, sorted: no */
        CARBON_TYPE_SORTED_SET,            /** element type: values, distinct elements: yes, sorted: yes */
        CARBON_TYPE_SORTED_MAP,            /** element type: pairs, distinct elements: yes, sorted: yes */
        CARBON_TYPE_SORTED_MULTIMAP,       /** element type: pairs, distinct elements: no, sorted: yes */
        CARBON_TYPE_UNSORTED_MAP           /** element type: pairs, distinct elements: yes, sorted: no */
} carbon_abstract_type_class_e;

/** Returns the abstract type class for a particular abstract derived container marker that is read from
 * the current position in the memory file without moving the memory files cursor. */
fn_result carbon_abstract_get_class(carbon_abstract_type_class_e *type, memfile *memfile);

/** Returns true if the abstract type class is of multiset (i.e., if the class is CARBON_TYPE_UNSORTED_MULTISET, or
 * CARBON_TYPE_SORTED_MULTISET. */
fn_result ofType(bool) carbon_abstract_is_multiset(carbon_abstract_type_class_e type);

/** Returns true if the abstract type class is of set (i.e., if the class is CARBON_TYPE_UNSORTED_SET, or
 * CARBON_TYPE_SORTED_SET. */
fn_result ofType(bool) carbon_abstract_is_set(carbon_abstract_type_class_e type);

/** Returns true if the abstract type class is of multimap (i.e., if the class is CARBON_TYPE_UNSORTED_MULTIMAP, or
 * CARBON_TYPE_SORTED_MULTIMAP. */
fn_result ofType(bool) carbon_abstract_is_multimap(carbon_abstract_type_class_e type);

/** Returns true if the abstract type class is of map (i.e., if the class is CARBON_TYPE_SORTED_MAP, or
 * CARBON_TYPE_UNSORTED_MAP. */
fn_result ofType(bool) carbon_abstract_is_map(carbon_abstract_type_class_e type);

/** Returns true if the abstract type class is sorted (i.e., if the class is CARBON_TYPE_SORTED_MULTISET,
 * CARBON_TYPE_SORTED_SET, CARBON_TYPE_SORTED_MAP, or CARBON_TYPE_SORTED_MULTIMAP */
fn_result ofType(bool) carbon_abstract_is_sorted(carbon_abstract_type_class_e type);

/** Returns true if the abstract type class does not contain duplicate entries (i.e., if the class is
 * CARBON_TYPE_UNSORTED_SET, CARBON_TYPE_SORTED_SET, CARBON_TYPE_SORTED_MAP, or CARBON_TYPE_UNSORTED_MAP) */
fn_result ofType(bool) carbon_abstract_is_distinct(carbon_abstract_type_class_e type);

// ---------------------------------------------------------------------------------------------------------------------
//  derived type (actual abstract type and which container is used)
// ---------------------------------------------------------------------------------------------------------------------

/** particular abstract derived type container (with marker) */
typedef enum carbon_derived {
        /** abstract types for object containers */
        CARBON_UNSORTED_MULTIMAP = CARBON_MUNSORTED_MULTIMAP,
        CARBON_SORTED_MULTIMAP = CARBON_MSORTED_MULTIMAP,
        CARBON_UNSORTED_MAP = CARBON_MUNSORTED_MAP,
        CARBON_SORTED_MAP = CARBON_MSORTED_MAP,

        /** abstract types for array containers */
        CARBON_UNSORTED_MULTISET_ARRAY = CARBON_MUNSORTED_MULTISET_ARR,
        CARBON_SORTED_MULTISET_ARRAY = CARBON_MSORTED_MULTISET_ARR,
        CARBON_UNSORTED_SET_ARRAY = CARBON_MUNSORTED_SET_ARR,
        CARBON_SORTED_SET_ARRAY = CARBON_MSORTED_SET_ARR,

        /** abstract types for column-u8 containers */
        CARBON_UNSORTED_MULTISET_COL_U8 = CARBON_MUNSORTED_MULTISET_U8,
        CARBON_SORTED_MULTISET_COL_U8 = CARBON_MSORTED_MULTISET_U8,
        CARBON_UNSORTED_SET_COL_U8 = CARBON_MUNSORTED_SET_U8,
        CARBON_SORTED_SET_COL_U8 = CARBON_MSORTED_SET_U8,

        /** abstract types for column-u16 containers */
        CARBON_UNSORTED_MULTISET_COL_U16 = CARBON_MUNSORTED_MULTISET_U16,
        CARBON_SORTED_MULTISET_COL_U16 = CARBON_MSORTED_MULTISET_U16,
        CARBON_UNSORTED_SET_COL_U16 = CARBON_MUNSORTED_SET_U16,
        CARBON_SORTED_SET_COL_U16 = CARBON_MSORTED_SET_U16,

        /** abstract types for column-u32 containers */
        CARBON_UNSORTED_MULTISET_COL_U32 = CARBON_MUNSORTED_MULTISET_U32,
        CARBON_SORTED_MULTISET_COL_U32 = CARBON_MSORTED_MULTISET_U32,
        CARBON_UNSORTED_SET_COL_U32 = CARBON_MUNSORTED_SET_U32,
        CARBON_SORTED_SET_COL_U32 = CARBON_MSORTED_SET_U32,

        /** abstract types for column-u64 containers */
        CARBON_UNSORTED_MULTISET_COL_U64 = CARBON_MUNSORTED_MULTISET_U64,
        CARBON_SORTED_MULTISET_COL_U64 = CARBON_MSORTED_MULTISET_U64,
        CARBON_UNSORTED_SET_COL_U64 = CARBON_MUNSORTED_SET_U64,
        CARBON_SORTED_SET_COL_U64 = CARBON_MSORTED_SET_U64,

        /** abstract types for column-i8 containers */
        CARBON_UNSORTED_MULTISET_COL_I8 = CARBON_MUNSORTED_MULTISET_I8,
        CARBON_SORTED_MULTISET_COL_I8 = CARBON_MSORTED_MULTISET_I8,
        CARBON_UNSORTED_SET_COL_I8 = CARBON_MUNSORTED_SET_I8,
        CARBON_SORTED_SET_COL_I8 = CARBON_MSORTED_SET_I8,

        /** abstract types for column-i16 containers */
        CARBON_UNSORTED_MULTISET_COL_I16 = CARBON_MUNSORTED_MULTISET_I16,
        CARBON_SORTED_MULTISET_COL_I16 = CARBON_MSORTED_MULTISET_I16,
        CARBON_UNSORTED_SET_COL_I16 = CARBON_MUNSORTED_SET_I16,
        CARBON_SORTED_SET_COL_I16 = CARBON_MSORTED_SET_I16,

        /** abstract types for column-i32 containers */
        CARBON_UNSORTED_MULTISET_COL_I32 = CARBON_MUNSORTED_MULTISET_I32,
        CARBON_SORTED_MULTISET_COL_I32 = CARBON_MSORTED_MULTISET_I32,
        CARBON_UNSORTED_SET_COL_I32 = CARBON_MUNSORTED_SET_I32,
        CARBON_SORTED_SET_COL_I32 = CARBON_MSORTED_SET_I32,

        /** abstract types for column-i64 containers */
        CARBON_UNSORTED_MULTISET_COL_I64 = CARBON_MUNSORTED_MULTISET_I64,
        CARBON_SORTED_MULTISET_COL_I64 = CARBON_MSORTED_MULTISET_I64,
        CARBON_UNSORTED_SET_COL_I64 = CARBON_MUNSORTED_SET_I64,
        CARBON_SORTED_SET_COL_I64 = CARBON_MSORTED_SET_I64,

        /** abstract types for column-float containers */
        CARBON_UNSORTED_MULTISET_COL_FLOAT = CARBON_MUNSORTED_MULTISET_FLOAT,
        CARBON_SORTED_MULTISET_COL_FLOAT = CARBON_MSORTED_MULTISET_FLOAT,
        CARBON_UNSORTED_SET_COL_FLOAT = CARBON_MUNSORTED_SET_FLOAT,
        CARBON_SORTED_SET_COL_FLOAT = CARBON_MSORTED_SET_FLOAT,

        /** abstract types for column-boolean containers */
        CARBON_UNSORTED_MULTISET_COL_BOOLEAN = CARBON_MUNSORTED_MULTISET_BOOLEAN,
        CARBON_SORTED_MULTISET_COL_BOOLEAN = CARBON_MSORTED_MULTISET_BOOLEAN,
        CARBON_UNSORTED_SET_COL_BOOLEAN = CARBON_MUNSORTED_SET_BOOLEAN,
        CARBON_SORTED_SET_COL_BOOLEAN = CARBON_MSORTED_SET_BOOLEAN
} carbon_derived_e;

/** derivable types for a list container (column or array) */
typedef enum carbon_list_derivable
{
        /** the container type that implements the list */
        CARBON_LIST_UNSORTED_MULTISET,
        /** mark list as sorted */
        CARBON_LIST_SORTED_MULTISET,
        /** mark list as non-distinct */
        CARBON_LIST_UNSORTED_SET,
        /** mark list as sorted and distinct */
        CARBON_LIST_SORTED_SET
} carbon_list_derivable_e;

/** derivable types for a map container (object) */
typedef enum carbon_map_derivable
{
        /** the container type that implements the map */
        CARBON_MAP_UNSORTED_MULTIMAP = CARBON_MUNSORTED_MULTIMAP,
        /** mark map as sorted */
        CARBON_MAP_SORTED_MULTIMAP = CARBON_MSORTED_MULTIMAP,
        /** mark map as non-distinct */
        CARBON_MAP_UNSORTED_MAP = CARBON_MUNSORTED_MAP,
        /** mark map as sorted and non-distinct */
        CARBON_MAP_SORTED_MAP = CARBON_MSORTED_MAP
} carbon_map_derivable_e;

/** Converts an abstract type class to a list derivable type. In case the abstract type class does not define
 * a list type, the function fails */
fn_result carbon_abstract_class_to_list_derivable(carbon_list_derivable_e *out, carbon_abstract_type_class_e in);

/** Converts a list derivable type to an abstract type class. In case of error, the function fails. */
fn_result carbon_abstract_list_derivable_to_class(carbon_abstract_type_class_e *out, carbon_list_derivable_e in);

fn_result carbon_abstract_map_derivable_to_class(carbon_abstract_type_class_e *out, carbon_map_derivable_e in);

/** Writes the marker for a particular base type to the actual position in the memory file, and steps
 * the memory file cursor one byte towards the end. */
fn_result carbon_abstract_write_base_type(memfile *memfile, carbon_container_sub_type_e type);

/** Writes the marker for the particular derived abstract type to the actual position in the memory file, and
 * steps the memory file cursor one byte towards the end. */
fn_result carbon_abstract_write_derived_type(memfile *memfile, carbon_derived_e type);

/** Peeks a byte from the memory file and returns the encoded container sub type. This is either an object
 * container, an array container, or and particular column container. In case a derived type is found, the
 * actual container type that implements that derived type is returned. For instance, if '[1]' is read,
 * a column-u8 container type is returned, and if [SOH] is read (which is CARBON_MSORTED_MULTISET_U8),
 * a column-u8 container type is returned, too. */
fn_result carbon_abstract_get_container_subtype(carbon_container_sub_type_e *type, memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an object container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_object(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an array container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_array(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-u8 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_u8(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-u16 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_u16(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-u32 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_u32(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-u64 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_u64(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-i8 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_i8(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-i16 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_i16(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-i32 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_i32(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-i64 container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_i64(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-float container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_float(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets an column-boolean container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column_boolean(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets a type of column container (u8, u16,...) or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_column(memfile *memfile);

/** Peeks a byte as marker from the memory file without moving the memory file cursor and returns true if this
 * marker sets a type of column container or array container or a derived type of that container type. */
fn_result ofType(bool) carbon_abstract_is_instanceof_list(memfile *memfile);

/** Returns the concrete derived type <code>concrete</code> (e.g., CARBON_SORTED_SET_COL_BOOLEAN) for a
 * given list type <code>is</code> (e.g., CARBON_LIST_CONTAINER_COLUMN_BOOLEAN) when deriving that
 * list type to a particular abstract type <code>should</code> (e.g., CARBON_SORTED_SET) */
fn_result carbon_abstract_derive_list_to(carbon_derived_e *concrete, carbon_list_container_e is,
                                         carbon_list_derivable_e should);

/** Returns the concrete derived type <code>concrete</code> (e.g., CARBON_MAP_SORTED_MULTIMAP) for a
 * given map when deriving that map type to a particular abstract type <code>should</code>
 * (e.g., CARBON_SORTED_MULTIMAP) */
fn_result carbon_abstract_derive_map_to(carbon_derived_e *concrete, carbon_map_derivable_e should);

/** Reads a marker from the memory file, and returns the particular abstract derived container (including
 * the marker) without moving the memory files cursor. In case of an failure (e.g., the read marker is not known),
 * the function returns an err. */
fn_result carbon_abstract_get_derived_type(carbon_derived_e *type, memfile *memfile);

END_DECL

#endif