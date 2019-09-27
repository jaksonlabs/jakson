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

#ifndef ARCHIVE_ITER_H
#define ARCHIVE_ITER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/archive.h>

BEGIN_DECL

typedef enum prop_iter_state {
        PROP_ITER_INIT,
        PROP_ITER_NULLS,
        PROP_ITER_BOOLS,
        PROP_ITER_INT8S,
        PROP_ITER_INT16S,
        PROP_ITER_INT32S,
        PROP_ITER_INT64S,
        PROP_ITER_UINT8S,
        PROP_ITER_UINT16S,
        PROP_ITER_UINT32S,
        PROP_ITER_UINT64S,
        PROP_ITER_FLOATS,
        PROP_ITER_STRINGS,
        PROP_ITER_OBJECTS,
        PROP_ITER_NULL_ARRAYS,
        PROP_ITER_BOOL_ARRAYS,
        PROP_ITER_INT8_ARRAYS,
        PROP_ITER_INT16_ARRAYS,
        PROP_ITER_INT32_ARRAYS,
        PROP_ITER_INT64_ARRAYS,
        PROP_ITER_UINT8_ARRAYS,
        PROP_ITER_UINT16_ARRAYS,
        PROP_ITER_UINT32_ARRAYS,
        PROP_ITER_UINT64_ARRAYS,
        PROP_ITER_FLOAT_ARRAYS,
        PROP_ITER_STRING_ARRAYS,
        PROP_ITER_OBJECT_ARRAYS,
        PROP_ITER_DONE
} prop_iter_state_e;

typedef struct archive_object {
        unique_id_t object_id;                  /** unique object id */
        offset_t offset;                        /** this objects header offset */
        archive_prop_offs prop_offsets;  /** per-property type offset in the record table byte stream */
        offset_t next_obj_off;                  /** offset to next object in list, or NULL if no such exists */
        memfile memfile;
        err err;
} archive_object;

typedef enum prop_iter_mode {
        PROP_ITER_MODE_OBJECT,
        PROP_ITER_MODE_COLLECTION,
} prop_iter_mode_e;

typedef struct object_iter_state {
        fixed_prop prop_group_header;    /** type, num props and keys */
        offset_t current_prop_group_off;
        offset_t prop_data_off;
        const archive_field_sid_t *keys;                /** current property key in this iteration */
        enum archive_field_type type;                   /** property basic value type (e.g., int8, or object) */
        bool is_array;                          /** flag indicating that property is an array type */
} object_iter_state;

typedef struct collection_iter_state {
        offset_t collection_start_off;
        u32 num_column_groups;
        u32 current_column_group_idx;
        const archive_field_sid_t *column_group_keys;
        const offset_t *column_group_offsets;

        struct {
                u32 num_columns;
                u32 num_objects;
                const unique_id_t *object_ids;
                const offset_t *column_offs;
                struct {
                        u32 idx;
                        archive_field_sid_t name;
                        enum archive_field_type type;
                        u32 num_elem;
                        const offset_t *elem_offsets;
                        const u32 *elem_positions;
                        struct {
                                u32 idx;
                                u32 array_length;
                                const void *array_base;
                        } current_entry;
                } current_column;
        } current_column_group;
} collection_iter_state;

typedef struct archive_value_vector {
        prop_iter *prop_iter;            /** pointer to property iterator that created this iterator */
        memfile record_table_memfile;    /** iterator-local read-only mem on archive record table */
        enum archive_field_type prop_type;              /** property basic value type (e.g., int8, or object) */
        bool is_array;                          /** flag indicating whether value type is an array or not */
        offset_t data_off;                      /** offset in mem where type-dependent data begins */
        u32 value_max_idx;                      /** maximum index of a value callable by 'at' functions */
        err err;                         /** ERROR information */
        unique_id_t object_id;                  /** current object id */
        const archive_field_sid_t *keys;
        union {
                struct {
                        const offset_t *offsets;
                        archive_object object;
                } object;
                struct {
                        union {
                                const archive_field_i8_t *int8s;
                                const archive_field_i16_t *int16s;
                                const archive_field_i32_t *int32s;
                                const archive_field_i64_t *int64s;
                                const archive_field_u8_t *uint8s;
                                const archive_field_u16_t *uint16s;
                                const archive_field_u32_t *uint32s;
                                const archive_field_u64_t *uint64s;
                                const archive_field_number_t *numbers;
                                const archive_field_sid_t *strings;
                                const archive_field_boolean_t *booleans;
                        } values;
                } basic;
                struct {
                        union {
                                const u32 *array_lengths;
                                const u32 *num_nulls_contained;
                        } meta;

                        union {
                                const archive_field_i8_t *int8s_base;
                                const archive_field_i16_t *int16s_base;
                                const archive_field_i32_t *int32s_base;
                                const archive_field_i64_t *int64s_base;
                                const archive_field_u8_t *uint8s_base;
                                const archive_field_u16_t *uint16s_base;
                                const archive_field_u32_t *uint32s_base;
                                const archive_field_u64_t *uint64s_base;
                                const archive_field_number_t *numbers_base;
                                const archive_field_sid_t *strings_base;
                                const archive_field_boolean_t *booleans_base;
                        } values;
                } arrays;
        } data;
} archive_value_vector;

typedef struct prop_iter {
        archive_object object;                 /** current object */
        memfile record_table_memfile;          /** iterator-local read-only mem on archive record table */
        u16 mask;                                     /** user-defined mask which properties to include */
        prop_iter_mode_e mode;                     /** determines whether to iterating over object or collection */
        err err;                               /** ERROR information */
        prop_iter_state_e prop_cursor;             /** current property type in iteration */
        object_iter_state mode_object;
        collection_iter_state mode_collection;
} prop_iter;

typedef struct independent_iter_state {
        memfile record_table_memfile;           /** iterator-local read-only mem on archive record table */
        collection_iter_state state;            /** iterator-local state */
        err err;                                /** ERROR information */
} independent_iter_state;

typedef struct column_object_iter {
        memfile memfile;
        collection_iter_state entry_state;
        archive_object obj;
        offset_t next_obj_off;
        err err;
} column_object_iter;

#define ARCHIVE_ITER_MASK_PRIMITIVES             (1 << 1)
#define ARCHIVE_ITER_MASK_ARRAYS                 (1 << 2)

#define ARCHIVE_ITER_MASK_INT8                   (1 << 3)
#define ARCHIVE_ITER_MASK_INT16                  (1 << 4)
#define ARCHIVE_ITER_MASK_INT32                  (1 << 5)
#define ARCHIVE_ITER_MASK_INT64                  (1 << 6)
#define ARCHIVE_ITER_MASK_UINT8                  (1 << 7)
#define ARCHIVE_ITER_MASK_UINT16                 (1 << 8)
#define ARCHIVE_ITER_MASK_UINT32                 (1 << 9)
#define ARCHIVE_ITER_MASK_UINT64                 (1 << 10)
#define ARCHIVE_ITER_MASK_NUMBER                 (1 << 11)
#define ARCHIVE_ITER_MASK_STRING                 (1 << 12)
#define ARCHIVE_ITER_MASK_BOOLEAN                (1 << 13)
#define ARCHIVE_ITER_MASK_NULL                   (1 << 14)
#define ARCHIVE_ITER_MASK_OBJECT                 (1 << 15)

#define ARCHIVE_ITER_MASK_INTEGER               ARCHIVE_ITER_MASK_INT8       |                                 \
                                                    ARCHIVE_ITER_MASK_INT16      |                                 \
                                                    ARCHIVE_ITER_MASK_INT32      |                                 \
                                                    ARCHIVE_ITER_MASK_INT64      |                                 \
                                                    ARCHIVE_ITER_MASK_UINT8      |                                 \
                                                    ARCHIVE_ITER_MASK_UINT16     |                                 \
                                                    ARCHIVE_ITER_MASK_UINT32     |                                 \
                                                    ARCHIVE_ITER_MASK_UINT64

#define ARCHIVE_ITER_MASK_ANY                   ARCHIVE_ITER_MASK_PRIMITIVES |                                 \
                                                    ARCHIVE_ITER_MASK_ARRAYS     |                                 \
                                                    ARCHIVE_ITER_MASK_INTEGER    |                                 \
                                                    ARCHIVE_ITER_MASK_NUMBER     |                                 \
                                                    ARCHIVE_ITER_MASK_STRING     |                                 \
                                                    ARCHIVE_ITER_MASK_BOOLEAN    |                                 \
                                                    ARCHIVE_ITER_MASK_NULL       |                                 \
                                                    ARCHIVE_ITER_MASK_OBJECT

bool archive_prop_iter_from_archive(prop_iter *iter, err *err, u16 mask, archive *archive);
bool archive_prop_iter_from_object(prop_iter *iter, u16 mask, err *err, const archive_object *obj);
bool archive_value_vector_from_prop_iter(archive_value_vector *value, err *err, prop_iter *prop_iter);
bool archive_prop_iter_next(prop_iter_mode_e *type, archive_value_vector *value_vector, independent_iter_state *collection_iter, prop_iter *prop_iter);
const archive_field_sid_t *archive_collection_iter_get_keys(u32 *num_keys, independent_iter_state *iter);
bool archive_collection_next_column_group(independent_iter_state *group_iter, independent_iter_state *iter);
const unique_id_t *archive_column_group_get_object_ids(u32 *num_objects, independent_iter_state *iter);
bool archive_column_group_next_column(independent_iter_state *column_iter, independent_iter_state *iter);
bool archive_column_get_name(archive_field_sid_t *name, enum archive_field_type *type, independent_iter_state *column_iter);
const u32 * archive_column_get_entry_positions(u32 *num_entry, independent_iter_state *column_iter);
bool archive_column_next_entry(independent_iter_state *entry_iter, independent_iter_state *iter);
bool archive_column_entry_get_type(enum archive_field_type *type, independent_iter_state *entry);

#define DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name)                                            \
const built_in_type *                                                                                      \
archive_column_entry_get_##name(u32 *array_length, independent_iter_state *entry);

DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i8_t, int8s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i16_t, int16s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i32_t, int32s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i64_t, int64s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u8_t, uint8s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u16_t, uint16s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u32_t, uint32s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u64_t, uint64s);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_sid_t, strings);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_number_t, numbers);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_boolean_t, booleans);
DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u32_t, nulls);

bool archive_column_entry_get_objects(column_object_iter *iter, independent_iter_state *entry);
const archive_object *archive_column_entry_object_iter_next_object(column_object_iter *iter);
bool archive_object_get_object_id(unique_id_t *id, const archive_object *object);
bool archive_object_get_prop_iter(prop_iter *iter, const archive_object *object);
bool archive_value_vector_get_object_id(unique_id_t *id, const archive_value_vector *iter);
const archive_field_sid_t *archive_value_vector_get_keys(u32 *num_keys, archive_value_vector *iter);
bool archive_value_vector_get_basic_type(enum archive_field_type *type, const archive_value_vector *value);
bool archive_value_vector_is_array_type(bool *is_array, const archive_value_vector *value);
bool archive_value_vector_get_length(u32 *length, const archive_value_vector *value);
bool archive_value_vector_is_of_objects(bool *is_object, archive_value_vector *value);
bool archive_value_vector_get_object_at(archive_object *object, u32 idx, archive_value_vector *value);

#define DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name)                                                            \
bool                                                                                                       \
archive_value_vector_is_##name(bool *type_match, archive_value_vector *value);

DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean);
DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null);

#define DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(name, built_in_type)                                            \
const built_in_type *                                                                                      \
archive_value_vector_get_##name(u32 *num_values, archive_value_vector *value);

DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, archive_field_i8_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, archive_field_i16_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, archive_field_i32_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, archive_field_i64_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, archive_field_u8_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, archive_field_u16_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, archive_field_u32_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, archive_field_u64_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, archive_field_sid_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, archive_field_number_t)
DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, archive_field_boolean_t)

const archive_field_u32_t *archive_value_vector_get_null_arrays(u32 *num_values, archive_value_vector *value);

#define DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type)                                         \
const built_in_type *                                                                                      \
archive_value_vector_get_##name##_arrays_at(u32 *array_length, u32 idx,                                                \
                                               archive_value_vector *value);                                    \


DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, archive_field_i8_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, archive_field_i16_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, archive_field_i32_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, archive_field_i64_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, archive_field_u8_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, archive_field_u16_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, archive_field_u32_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, archive_field_u64_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, archive_field_sid_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, archive_field_number_t);
DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, archive_field_boolean_t);

void archive_int_reset_carbon_object_mem_file(archive_object *object);

END_DECL

#endif
