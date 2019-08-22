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

#ifndef JAK_ARCHIVE_ITER_H
#define JAK_ARCHIVE_ITER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

enum jak_prop_iter_state {
        JAK_PROP_ITER_INIT,
        JAK_PROP_ITER_NULLS,
        JAK_PROP_ITER_BOOLS,
        JAK_PROP_ITER_INT8S,
        JAK_PROP_ITER_INT16S,
        JAK_PROP_ITER_INT32S,
        JAK_PROP_ITER_INT64S,
        JAK_PROP_ITER_UINT8S,
        JAK_PROP_ITER_UINT16S,
        JAK_PROP_ITER_UINT32S,
        JAK_PROP_ITER_UINT64S,
        JAK_PROP_ITER_FLOATS,
        JAK_PROP_ITER_STRINGS,
        JAK_PROP_ITER_OBJECTS,
        JAK_PROP_ITER_NULL_ARRAYS,
        JAK_PROP_ITER_BOOL_ARRAYS,
        JAK_PROP_ITER_INT8_ARRAYS,
        JAK_PROP_ITER_INT16_ARRAYS,
        JAK_PROP_ITER_INT32_ARRAYS,
        JAK_PROP_ITER_INT64_ARRAYS,
        JAK_PROP_ITER_UINT8_ARRAYS,
        JAK_PROP_ITER_UINT16_ARRAYS,
        JAK_PROP_ITER_UINT32_ARRAYS,
        JAK_PROP_ITER_UINT64_ARRAYS,
        JAK_PROP_ITER_FLOAT_ARRAYS,
        JAK_PROP_ITER_STRING_ARRAYS,
        JAK_PROP_ITER_OBJECT_ARRAYS,
        JAK_PROP_ITER_DONE
};

struct jak_archive_object {
        jak_global_id_t object_id;                  /* unique object id */
        jak_offset_t offset;                        /* this objects header offset */
        struct jak_archive_prop_offs prop_offsets;  /* per-property type offset in the record table byte stream */
        jak_offset_t next_obj_off;                  /* offset to next object in list, or NULL if no such exists */
        struct jak_memfile memfile;
        struct jak_error err;
};

enum jak_prop_iter_mode {
        JAK_PROP_ITER_MODE_OBJECT,
        JAK_PROP_ITER_MODE_COLLECTION,
};

struct jak_object_iter_state {
        jak_offset_t prop_type_off_data;            /* offset of type-dependent data in memfile */
        struct jak_fixed_prop prop_group_header;    /* type, num props and keys */
        jak_offset_t current_prop_group_off;
        jak_offset_t prop_data_off;
        const jak_archive_field_sid_t *keys;                /* current property key in this iteration */
        enum jak_archive_field_type type;                   /* property basic value type (e.g., int8, or object) */
        bool is_array;                          /* flag indicating that property is an array type */
};

struct jak_collection_iter_state {
        jak_offset_t collection_start_off;
        jak_u32 num_column_groups;
        jak_u32 current_column_group_idx;
        const jak_archive_field_sid_t *column_group_keys;
        const jak_offset_t *column_group_offsets;

        struct {
                jak_u32 num_columns;
                jak_u32 num_objects;
                const jak_global_id_t *object_ids;
                const jak_offset_t *column_offs;
                struct {
                        jak_u32 idx;
                        jak_archive_field_sid_t name;
                        enum jak_archive_field_type type;
                        jak_u32 num_elem;
                        const jak_offset_t *elem_offsets;
                        const jak_u32 *elem_positions;
                        struct {
                                jak_u32 idx;
                                jak_u32 array_length;
                                const void *array_base;
                        } current_entry;
                } current_column;
        } current_column_group;
};

struct jak_archive_value_vector {
        struct jak_prop_iter *prop_iter;            /* pointer to property iterator that created this iterator */
        struct jak_memfile record_table_memfile;    /* iterator-local read-only memfile on archive record table */
        enum jak_archive_field_type prop_type;              /* property basic value type (e.g., int8, or object) */
        bool is_array;                          /* flag indicating whether value type is an array or not */
        jak_offset_t data_off;                      /* offset in memfile where type-dependent data begins */
        jak_u32 value_max_idx;                      /* maximum index of a value callable by 'at' functions */
        struct jak_error err;                         /* error information */
        jak_global_id_t object_id;                  /* current object id */
        const jak_archive_field_sid_t *keys;
        union {
                struct {
                        const jak_offset_t *offsets;
                        struct jak_archive_object object;
                } object;
                struct {
                        union {
                                const jak_archive_field_i8_t *int8s;
                                const jak_archive_field_i16_t *int16s;
                                const jak_archive_field_i32_t *int32s;
                                const jak_archive_field_i64_t *int64s;
                                const jak_archive_field_u8_t *uint8s;
                                const jak_archive_field_u16_t *uint16s;
                                const jak_archive_field_u32_t *uint32s;
                                const jak_archive_field_u64_t *uint64s;
                                const jak_archive_field_number_t *numbers;
                                const jak_archive_field_sid_t *strings;
                                const jak_archive_field_boolean_t *booleans;
                        } values;
                } basic;
                struct {
                        union {
                                const jak_u32 *array_lengths;
                                const jak_u32 *num_nulls_contained;
                        } meta;

                        union {
                                const jak_archive_field_i8_t *int8s_base;
                                const jak_archive_field_i16_t *int16s_base;
                                const jak_archive_field_i32_t *int32s_base;
                                const jak_archive_field_i64_t *int64s_base;
                                const jak_archive_field_u8_t *uint8s_base;
                                const jak_archive_field_u16_t *uint16s_base;
                                const jak_archive_field_u32_t *uint32s_base;
                                const jak_archive_field_u64_t *uint64s_base;
                                const jak_archive_field_number_t *numbers_base;
                                const jak_archive_field_sid_t *strings_base;
                                const jak_archive_field_boolean_t *booleans_base;
                        } values;
                } arrays;
        } data;
};

struct jak_prop_iter {
        struct jak_archive_object object;                 /* current object */
        struct jak_memfile record_table_memfile;          /* iterator-local read-only memfile on archive record table */

        jak_u16 mask;                                     /* user-defined mask which properties to include */
        enum jak_prop_iter_mode mode;                     /* determines whether to iterating over object or collection */
        struct jak_error err;                               /* error information */
        enum jak_prop_iter_state prop_cursor;             /* current property type in iteration */

        struct jak_object_iter_state mode_object;
        struct jak_collection_iter_state mode_collection;

};

struct jak_independent_iter_state {
        struct jak_memfile record_table_memfile;           /* iterator-local read-only memfile on archive record table */
        struct jak_collection_iter_state state;            /* iterator-local state */
        struct jak_error err;                                /* error information */
};

struct jak_column_object_iter {
        struct jak_memfile memfile;
        struct jak_collection_iter_state entry_state;
        struct jak_archive_object obj;
        jak_offset_t next_obj_off;
        struct jak_error err;
};

#define JAK_ARCHIVE_ITER_MASK_PRIMITIVES             (1 << 1)
#define JAK_ARCHIVE_ITER_MASK_ARRAYS                 (1 << 2)

#define JAK_ARCHIVE_ITER_MASK_INT8                   (1 << 3)
#define JAK_ARCHIVE_ITER_MASK_INT16                  (1 << 4)
#define JAK_ARCHIVE_ITER_MASK_INT32                  (1 << 5)
#define JAK_ARCHIVE_ITER_MASK_INT64                  (1 << 6)
#define JAK_ARCHIVE_ITER_MASK_UINT8                  (1 << 7)
#define JAK_ARCHIVE_ITER_MASK_UINT16                 (1 << 8)
#define JAK_ARCHIVE_ITER_MASK_UINT32                 (1 << 9)
#define JAK_ARCHIVE_ITER_MASK_UINT64                 (1 << 10)
#define JAK_ARCHIVE_ITER_MASK_NUMBER                 (1 << 11)
#define JAK_ARCHIVE_ITER_MASK_STRING                 (1 << 12)
#define JAK_ARCHIVE_ITER_MASK_BOOLEAN                (1 << 13)
#define JAK_ARCHIVE_ITER_MASK_NULL                   (1 << 14)
#define JAK_ARCHIVE_ITER_MASK_OBJECT                 (1 << 15)

#define JAK_ARCHIVE_ITER_MASK_INTEGER               JAK_ARCHIVE_ITER_MASK_INT8       |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT16      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT32      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INT64      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT8      |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT16     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT32     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_UINT64

#define JAK_ARCHIVE_ITER_MASK_ANY                   JAK_ARCHIVE_ITER_MASK_PRIMITIVES |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_ARRAYS     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_INTEGER    |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_NUMBER     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_STRING     |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_BOOLEAN    |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_NULL       |                                 \
                                                    JAK_ARCHIVE_ITER_MASK_OBJECT

JAK_DEFINE_GET_ERROR_FUNCTION(archive_value_vector, struct jak_archive_value_vector, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_prop_iter, struct jak_prop_iter, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_collection_iter, struct jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_group_iter, struct jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_iter, struct jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_entry_iter, struct jak_independent_iter_state, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_column_entry_object_iter, struct jak_column_object_iter, iter)

JAK_DEFINE_GET_ERROR_FUNCTION(archive_object, struct jak_archive_object, obj)

bool jak_archive_prop_iter_from_archive(struct jak_prop_iter *iter, struct jak_error *err, jak_u16 mask,
                                        struct jak_archive *archive);

bool jak_archive_prop_iter_from_object(struct jak_prop_iter *iter, jak_u16 mask, struct jak_error *err,
                                       const struct jak_archive_object *obj);

bool jak_archive_value_vector_from_prop_iter(struct jak_archive_value_vector *value, struct jak_error *err,
                                             struct jak_prop_iter *prop_iter);

bool jak_archive_prop_iter_next(enum jak_prop_iter_mode *type, struct jak_archive_value_vector *value_vector,
                                struct jak_independent_iter_state *collection_iter, struct jak_prop_iter *prop_iter);

const jak_archive_field_sid_t *
jak_archive_collection_iter_get_keys(jak_u32 *num_keys, struct jak_independent_iter_state *iter);

bool jak_archive_collection_next_column_group(struct jak_independent_iter_state *group_iter,
                                              struct jak_independent_iter_state *iter);

const jak_global_id_t *
jak_archive_column_group_get_object_ids(jak_u32 *num_objects, struct jak_independent_iter_state *iter);

bool jak_archive_column_group_next_column(struct jak_independent_iter_state *column_iter,
                                          struct jak_independent_iter_state *iter);

bool jak_archive_column_get_name(jak_archive_field_sid_t *name, enum jak_archive_field_type *type,
                                 struct jak_independent_iter_state *column_iter);

const jak_u32 *
jak_archive_column_get_entry_positions(jak_u32 *num_entry, struct jak_independent_iter_state *column_iter);

bool
jak_archive_column_next_entry(struct jak_independent_iter_state *entry_iter, struct jak_independent_iter_state *iter);

bool jak_archive_column_entry_get_type(enum jak_archive_field_type *type, struct jak_independent_iter_state *entry);

#define JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name)                                            \
const built_in_type *                                                                                      \
jak_archive_column_entry_get_##name(jak_u32 *array_length, struct jak_independent_iter_state *entry);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i8_t, int8s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i16_t, int16s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i32_t, int32s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i64_t, int64s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u8_t, uint8s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u16_t, uint16s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, uint32s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u64_t, uint64s);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_sid_t, strings);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_number_t, numbers);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_boolean_t, booleans);

JAK_DEFINE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, nulls);

bool
jak_archive_column_entry_get_objects(struct jak_column_object_iter *iter, struct jak_independent_iter_state *entry);

const struct jak_archive_object *jak_archive_column_entry_object_iter_next_object(struct jak_column_object_iter *iter);

bool jak_archive_object_get_object_id(jak_global_id_t *id, const struct jak_archive_object *object);

bool jak_archive_object_get_prop_iter(struct jak_prop_iter *iter, const struct jak_archive_object *object);

bool jak_archive_value_vector_get_object_id(jak_global_id_t *id, const struct jak_archive_value_vector *iter);

const jak_archive_field_sid_t *
jak_jak_archive_value_vector_get_keys(jak_u32 *num_keys, struct jak_archive_value_vector *iter);

bool jak_jak_archive_value_vector_get_basic_type(enum jak_archive_field_type *type,
                                                 const struct jak_archive_value_vector *value);

bool jak_archive_value_vector_is_array_type(bool *is_array, const struct jak_archive_value_vector *value);

bool jak_jak_archive_value_vector_get_length(jak_u32 *length, const struct jak_archive_value_vector *value);

bool jak_archive_value_vector_is_of_objects(bool *is_object, struct jak_archive_value_vector *value);

bool jak_jak_archive_value_vector_get_object_at(struct jak_archive_object *object, jak_u32 idx,
                                                struct jak_archive_value_vector *value);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name)                                                            \
bool                                                                                                       \
jak_archive_value_vector_is_##name(bool *type_match, struct jak_archive_value_vector *value);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(name, built_in_type)                                            \
const built_in_type *                                                                                      \
jak_archive_value_vector_get_##name(jak_u32 *num_values, struct jak_archive_value_vector *value);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, jak_archive_field_i8_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, jak_archive_field_i16_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, jak_archive_field_i32_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, jak_archive_field_i64_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, jak_archive_field_u8_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, jak_archive_field_u16_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, jak_archive_field_u32_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, jak_archive_field_u64_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, jak_archive_field_sid_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, jak_archive_field_number_t)

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, jak_archive_field_boolean_t)

const jak_archive_field_u32_t *jak_archive_value_vector_get_null_arrays(jak_u32 *num_values,
                                                                        struct jak_archive_value_vector *value);

#define JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type)                                         \
const built_in_type *                                                                                      \
jak_archive_value_vector_get_##name##_arrays_at(jak_u32 *array_length, jak_u32 idx,                                                \
                                               struct jak_archive_value_vector *value);                                    \


JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, jak_archive_field_i8_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, jak_archive_field_i16_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, jak_archive_field_i32_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, jak_archive_field_i64_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, jak_archive_field_u8_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, jak_archive_field_u16_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, jak_archive_field_u32_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, jak_archive_field_u64_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, jak_archive_field_sid_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, jak_archive_field_number_t);

JAK_DEFINE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, jak_archive_field_boolean_t);

void jak_archive_int_reset_carbon_object_mem_file(struct jak_archive_object *object);

JAK_END_DECL

#endif
