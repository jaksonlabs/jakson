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

#ifndef COLUMNDOC_H
#define COLUMNDOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>
#include <jakson/std/bloom.h>
#include <jakson/std/string_dict.h>
#include <jakson/archive/doc.h>

BEGIN_DECL

/**
 * Transformation of an JSON-like array of objects to a columnar representation of key values.
 *
 * The assumption is that an array of objects is LIKELY to have elements of the same (well yet unknown) schema. The
 * following structure captures a particular key (with its data type) found in one or more elements inside this array,
 * and stores the value assigned to this key along with with the object position in the array in which this mapping
 * occurs. Note that a keys name might have different data types inside objects embedded in arrays, and a key is not
 * guaranteed to occur in all objects embedded in the array.
 */
typedef struct column_doc_column {
        /** Key name */
        archive_field_sid_t key_name;
        /** Particular key type */
        archive_field_e type;
        /** Positions of objects in the parent array that has this particular key name with this particular value type */
        vector ofType(u32) array_positions;
        /** Values stored in objects assigned to this key-type mapping. The i-th element in `values` (which hold the
         * i-th value) is associated to the i-th element in `arrayPosition` which holds the position of the object inside
         * the array from which this pair was taken. */
        vector ofType(Vector ofType( < T >)) values;
} column_doc_column;

typedef struct column_doc_group {
        /** Key name */
        archive_field_sid_t key;
        /** Key columns as a decomposition of objects stored in that JSON-like array */
        vector ofType(column_doc_column) columns;
} column_doc_group;

typedef struct column_doc_obj {
        /** Parent document meta doc */
        column_doc *parent;
        /** Key in parent document meta doc that maps to this one, or "/" if this is the top-level meta doc */
        archive_field_sid_t parent_key;
        /** Index inside the array of this doc in its parents property, or 0 if this is not an array type or top-level */
        size_t index;

        /** Inverted index of keys mapping to primitive boolean types (sorted by key) */
        vector ofType(archive_field_sid_t) bool_prop_keys;
        /** Inverted index of keys mapping to primitive int8 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int8_prop_keys;
        /** Inverted index of keys mapping to primitive int16 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int16_prop_keys;
        /** Inverted index of keys mapping to primitive int32 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int32_prop_keys;
        /** Inverted index of keys mapping to primitive int64 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int64_prop_keys;
        /** Inverted index of keys mapping to primitive uint8 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint8_prop_keys;
        /** Inverted index of keys mapping to primitive uint16 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint16_prop_keys;
        /** Inverted index of keys mapping to primitive uint32 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uin32_prop_keys;
        /** Inverted index of keys mapping to primitive uint64 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint64_prop_keys;
        /** Inverted index of keys mapping to primitive string types (sorted by key) */
        vector ofType(archive_field_sid_t) string_prop_keys;
        /** Inverted index of keys mapping to primitive real types (sorted by key) */
        vector ofType(archive_field_sid_t) float_prop_keys;
        /** Inverted index of keys mapping to primitive null values (sorted by key) */
        vector ofType(archive_field_sid_t) null_prop_keys;
        /** Inverted index of keys mapping to exactly one nested object value (sorted by key) */
        vector ofType(archive_field_sid_t) obj_prop_keys;

        /** Inverted index of keys mapping to array of boolean types (sorted by key)*/
        vector ofType(archive_field_sid_t) bool_array_prop_keys;
        /** Inverted index of keys mapping to array of int8 number types (sorted by key)*/
        vector ofType(archive_field_sid_t) int8_array_prop_keys;
        /** Inverted index of keys mapping to array of int16 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int16_array_prop_keys;
        /** Inverted index of keys mapping to array of int32 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int32_array_prop_keys;
        /** Inverted index of keys mapping to array of int64 number types (sorted by key) */
        vector ofType(archive_field_sid_t) int64_array_prop_keys;
        /** Inverted index of keys mapping to array of uint8 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint8_array_prop_keys;
        /** Inverted index of keys mapping to array of uint16 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint16_array_prop_keys;
        /** Inverted index of keys mapping to array of uint32 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint32_array_prop_keys;
        /** Inverted index of keys mapping to array of uint64 number types (sorted by key) */
        vector ofType(archive_field_sid_t) uint64_array_prop_keys;
        /** Inverted index of keys mapping array of string types (sorted by key) */
        vector ofType(archive_field_sid_t) string_array_prop_keys;
        /** Inverted index of keys mapping array of real types (sorted by key) */
        vector ofType(archive_field_sid_t) float_array_prop_keys;
        /** Inverted index of keys mapping array of null value (sorted by key)s */
        vector ofType(archive_field_sid_t) null_array_prop_keys;

        /** Primitive boolean values associated to keys stored above (sorted by key) */
        vector ofType(FIELD_BOOLEANean_t) bool_prop_vals;
        /** Primitive int8 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_i8_t) int8_prop_vals;
        /** Primitive int16 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_i16_t) int16_prop_vals;
        /** Primitive int32 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_i32_t) int32_prop_vals;
        /** Primitive int64 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_i64_t) int64_prop_vals;
        /** Primitive uint8 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_u8_t) uint8_prop_vals;
        /** Primitive uint16 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_u16_t) uint16_prop_vals;
        /** Primitive uint32 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_u32_t) uint32_prop_vals;
        /** Primitive uint64 number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_u64_t) uint64_prop_vals;
        /** Primitive real number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_number_t) float_prop_vals;
        /** Primitive string number values associated to keys stored above (sorted by key) */
        vector ofType(archive_field_sid_t) string_prop_vals;

        /** Array of boolean values associated to keys stored above (sorted by key) */
        vector ofType(Vector) bool_array_prop_vals;
        /** Array of int8 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) int8_array_prop_vals;
        /** Array of int16 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) int16_array_prop_vals;
        /** Array of int32 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) int32_array_prop_vals;
        /** Array of int64 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) int64_array_prop_vals;
        /** Array of uint8 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) uint8_array_prop_vals;
        /** Array of uint16 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) uint16_array_prop_vals;
        /** Array of uint32 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) uint32_array_prop_vals;
        /** Array of uint64 number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) ui64_array_prop_vals;
        /** Array of real number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) float_array_prop_vals;
        /** Array of string number values associated to keys stored above (sorted by key) */
        vector ofType(Vector) string_array_prop_vals;
        /** Array of null values associated to keys stored above (sorted by key). The number represents the
         * multiplicity of nulls for the associated key. */
        vector ofType(u16) null_array_prop_vals;
        /** Primitive objects associated to keys stored above (sorted by key) */
        vector ofType(column_doc_obj) obj_prop_vals;

        /** Index of primitive boolean values associated to keys stored above (sorted by value) */
        vector ofType(u32) bool_val_idxs;
        /** Index of primitive int8 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) int8_val_idxs;
        /** Index of primitive int16 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) int16_val_idxs;
        /** Index of primitive int32 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) int32_val_idxs;
        /** Index of primitive int64 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) int64_val_idxs;
        /** Index of primitive uint8 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) uint8_val_idxs;
        /** Index of primitive uint16 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) uint16_val_idxs;
        /** Index of primitive uint32 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) uint32_val_idxs;
        /** Index of primitive uint64 number values associated to keys stored above (sorted by value) */
        vector ofType(u32) uint64_val_idxs;
        /** Index of primitive real number values associated to keys stored above (sorted by value) */
        vector ofType(u32) float_val_idxs;
        /** Index of primitive string number values associated to keys stored above (sorted by value) */
        vector ofType(u32) string_val_idxs;

        /** Index of array of boolean values associated to keys stored above (sorted by value) */
        vector ofType(Vector) bool_array_idxs;
        /** Index of array of int8 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) int8_array_idxs;
        /** Index of array of int16 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) int16_array_idxs;
        /** Index of array of int32 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) int32_array_idxs;
        /** Index of array of int64 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) int64_array_idxs;
        /** Index of array of uint8 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) uint8_array_idxs;
        /** Index of array of uint16 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) uint16_array_idxs;
        /** Index of array of uint32 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) uint32_array_idxs;
        /** Index of array of uint64 number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) uint64_array_idxs;
        /** Index of array of real number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) float_array_idxs;
        /** Index of array of string number values associated to keys stored above (sorted by value) */
        vector ofType(Vector) string_array_idxs;

        /** Array of objects associated to keys stored above (sorted by key) */
        vector ofType(column_doc_group) obj_array_props;
} column_doc_obj;

typedef struct column_doc {
        const doc *doc;
        string_dict *dic;
        column_doc_obj columndoc;
        const doc_bulk *bulk;
        bool read_optimized;
        err err;
} column_doc;

bool columndoc_create(column_doc *columndoc, err *err, const doc *doc,  const doc_bulk *bulk, const doc_entries *entries, string_dict *dic);
bool columndoc_drop(column_doc *doc);

bool columndoc_free(column_doc *doc);

bool columndoc_print(FILE *file, column_doc *doc);

END_DECL

#endif