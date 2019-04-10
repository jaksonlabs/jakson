/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef CARBON_COLUMNDOC_H
#define CARBON_COLUMNDOC_H

#include "shared/common.h"
#include "stdx/vec.h"
#include "bloom/bloom.h"
#include "strdic/strdic.h"
#include "doc.h"

CARBON_BEGIN_DECL

/**
 * Transformation of an JSON-like array of objects to a columnar representation of key values.
 *
 * The assumption is that an array of objects is likely to have elements of the same (well yet unknown) schema. The
 * following structure captures a particular key (with its data type) found in one or more elements inside this array,
 * and stores the value assigned to this key along with with the object position in the array in which this mapping
 * occurs. Note that a keys name might have different data types inside objects embedded in arrays, and a key is not
 * guaranteed to occur in all objects embedded in the array.
 */
typedef struct carbon_columndoc_column
{
    /** Key name */
    carbon_string_id_t key_name;
    /** Particular key type */
    field_e type;
    /** Positions of objects in the parent array that has this particular key name with this particular value type */
    vec_t ofType(u32) array_positions;
    /** Values stored in objects assigned to this key-type mapping. The i-th element in `values` (which hold the
     * i-th value) is associated to the i-th element in `arrayPosition` which holds the position of the object inside
     * the array from which this pair was taken. */
    vec_t ofType(Vector ofType(<T>)) values;
} carbon_columndoc_column_t;

typedef struct carbon_columndoc_columngroup
{
    /** Key name */
    carbon_string_id_t key;
    /** Key columns as a decomposition of objects stored in that JSON-like array */
    vec_t ofType(carbon_columndoc_column_t) columns;

} carbon_columndoc_columngroup_t;

typedef struct carbon_columndoc_obj
{
    /** Parent document meta doc */
    carbon_columndoc_t *parent;
    /** Key in parent document meta doc that maps to this one, or "/" if this is the top-level meta doc */
    carbon_string_id_t parent_key;
    /** Index inside the array of this doc in its parents property, or 0 if this is not an array type or top-level */
    size_t index;

    /** Inverted index of keys mapping to primitive boolean types (sorted by key) */
    vec_t ofType(carbon_string_id_t) bool_prop_keys;
    /** Inverted index of keys mapping to primitive int8 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int8_prop_keys;
    /** Inverted index of keys mapping to primitive int16 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int16_prop_keys;
    /** Inverted index of keys mapping to primitive int32 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int32_prop_keys;
    /** Inverted index of keys mapping to primitive int64 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int64_prop_keys;
    /** Inverted index of keys mapping to primitive uint8 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint8_prop_keys;
    /** Inverted index of keys mapping to primitive uint16 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint16_prop_keys;
    /** Inverted index of keys mapping to primitive uint32 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uin32_prop_keys;
    /** Inverted index of keys mapping to primitive uint64 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint64_prop_keys;
    /** Inverted index of keys mapping to primitive string types (sorted by key) */
    vec_t ofType(carbon_string_id_t) string_prop_keys;
    /** Inverted index of keys mapping to primitive real types (sorted by key) */
    vec_t ofType(carbon_string_id_t) float_prop_keys;
    /** Inverted index of keys mapping to primitive null values (sorted by key) */
    vec_t ofType(carbon_string_id_t) null_prop_keys;
    /** Inverted index of keys mapping to exactly one nested object value (sorted by key) */
    vec_t ofType(carbon_string_id_t) obj_prop_keys;

    /** Inverted index of keys mapping to array of boolean types (sorted by key)*/
    vec_t ofType(carbon_string_id_t) bool_array_prop_keys;
    /** Inverted index of keys mapping to array of int8 number types (sorted by key)*/
    vec_t ofType(carbon_string_id_t) int8_array_prop_keys;
    /** Inverted index of keys mapping to array of int16 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int16_array_prop_keys;
    /** Inverted index of keys mapping to array of int32 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int32_array_prop_keys;
    /** Inverted index of keys mapping to array of int64 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) int64_array_prop_keys;
    /** Inverted index of keys mapping to array of uint8 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint8_array_prop_keys;
    /** Inverted index of keys mapping to array of uint16 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint16_array_prop_keys;
    /** Inverted index of keys mapping to array of uint32 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint32_array_prop_keys;
    /** Inverted index of keys mapping to array of uint64 number types (sorted by key) */
    vec_t ofType(carbon_string_id_t) uint64_array_prop_keys;
    /** Inverted index of keys mapping array of string types (sorted by key) */
    vec_t ofType(carbon_string_id_t) string_array_prop_keys;
    /** Inverted index of keys mapping array of real types (sorted by key) */
    vec_t ofType(carbon_string_id_t) float_array_prop_keys;
    /** Inverted index of keys mapping array of null value (sorted by key)s */
    vec_t ofType(carbon_string_id_t) null_array_prop_keys;

    /** Primitive boolean values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_boolean_t) bool_prop_vals;
    /** Primitive int8 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_i8) int8_prop_vals;
    /** Primitive int16 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_i16) int16_prop_vals;
    /** Primitive int32 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_i32) int32_prop_vals;
    /** Primitive int64 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_i64) int64_prop_vals;
    /** Primitive uint8 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_u8) uint8_prop_vals;
    /** Primitive uint16 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_u16) uint16_prop_vals;
    /** Primitive uint32 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_u32) uint32_prop_vals;
    /** Primitive uint64 number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_u64) uint64_prop_vals;
    /** Primitive real number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_number_t) float_prop_vals;
    /** Primitive string number values associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_string_id_t) string_prop_vals;

    /** Array of boolean values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) bool_array_prop_vals;
    /** Array of int8 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) int8_array_prop_vals;
    /** Array of int16 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) int16_array_prop_vals;
    /** Array of int32 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) int32_array_prop_vals;
    /** Array of int64 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) int64_array_prop_vals;
    /** Array of uint8 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) uint8_array_prop_vals;
    /** Array of uint16 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) uint16_array_prop_vals;
    /** Array of uint32 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) uint32_array_prop_vals;
    /** Array of uint64 number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) ui64_array_prop_vals;
    /** Array of real number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) float_array_prop_vals;
    /** Array of string number values associated to keys stored above (sorted by key) */
    vec_t ofType(Vector) string_array_prop_vals;
    /** Array of null values associated to keys stored above (sorted by key). The number represents the
     * multiplicity of nulls for the associated key. */
    vec_t ofType(u16) null_array_prop_vals;
    /** Primitive objects associated to keys stored above (sorted by key) */
    vec_t ofType(columndoc_obj_t) obj_prop_vals;

    /** Index of primitive boolean values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) bool_val_idxs;
    /** Index of primitive int8 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) int8_val_idxs;
    /** Index of primitive int16 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) int16_val_idxs;
    /** Index of primitive int32 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) int32_val_idxs;
    /** Index of primitive int64 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) int64_val_idxs;
    /** Index of primitive uint8 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) uint8_val_idxs;
    /** Index of primitive uint16 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) uint16_val_idxs;
    /** Index of primitive uint32 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) uint32_val_idxs;
    /** Index of primitive uint64 number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) uint64_val_idxs;
    /** Index of primitive real number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) float_val_idxs;
    /** Index of primitive string number values associated to keys stored above (sorted by value) */
    vec_t ofType(u32) string_val_idxs;

    /** Index of array of boolean values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) bool_array_idxs;
    /** Index of array of int8 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) int8_array_idxs;
    /** Index of array of int16 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) int16_array_idxs;
    /** Index of array of int32 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) int32_array_idxs;
    /** Index of array of int64 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) int64_array_idxs;
    /** Index of array of uint8 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) uint8_array_idxs;
    /** Index of array of uint16 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) uint16_array_idxs;
    /** Index of array of uint32 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) uint32_array_idxs;
    /** Index of array of uint64 number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) uint64_array_idxs;
    /** Index of array of real number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) float_array_idxs;
    /** Index of array of string number values associated to keys stored above (sorted by value) */
    vec_t ofType(Vector) string_array_idxs;

    /** Array of objects associated to keys stored above (sorted by key) */
    vec_t ofType(carbon_columndoc_columngroup_t) obj_array_props;

} columndoc_obj_t;

typedef struct carbon_columndoc
{
    const carbon_doc_t *doc;
    carbon_strdic_t *dic;
    columndoc_obj_t columndoc;
    const carbon_doc_bulk_t *bulk;
    bool read_optimized;
    struct err err;
} carbon_columndoc_t;

CARBON_EXPORT(bool)
carbon_columndoc_create(carbon_columndoc_t *columndoc, struct err *err, const carbon_doc_t *doc,
                        const carbon_doc_bulk_t *bulk, const carbon_doc_entries_t *entries,
                        carbon_strdic_t *dic);

CARBON_DEFINE_GET_ERROR_FUNCTION(columndoc, carbon_columndoc_t, doc)

CARBON_EXPORT(bool)
carbon_columndoc_free(carbon_columndoc_t *doc);

CARBON_EXPORT(bool)
carbon_columndoc_print(FILE *file, carbon_columndoc_t *doc);

CARBON_EXPORT(bool)
carbon_columndoc_drop(carbon_columndoc_t *doc);

CARBON_END_DECL

#endif