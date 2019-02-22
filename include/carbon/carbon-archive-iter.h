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

#ifndef CARBON_ARCHIVE_ITER_H
#define CARBON_ARCHIVE_ITER_H

#include "carbon-common.h"
#include "carbon-error.h"
#include "carbon-archive.h"

CARBON_BEGIN_DECL

typedef enum
{
    CARBON_PROP_ITER_STATE_INIT,

    CARBON_PROP_ITER_STATE_NULLS,
    CARBON_PROP_ITER_STATE_BOOLS,
    CARBON_PROP_ITER_STATE_INT8S,
    CARBON_PROP_ITER_STATE_INT16S,
    CARBON_PROP_ITER_STATE_INT32S,
    CARBON_PROP_ITER_STATE_INT64S,
    CARBON_PROP_ITER_STATE_UINT8S,
    CARBON_PROP_ITER_STATE_UINT16S,
    CARBON_PROP_ITER_STATE_UINT32S,
    CARBON_PROP_ITER_STATE_UINT64S,
    CARBON_PROP_ITER_STATE_FLOATS,
    CARBON_PROP_ITER_STATE_STRINGS,
    CARBON_PROP_ITER_STATE_OBJECTS,
    CARBON_PROP_ITER_STATE_NULL_ARRAYS,
    CARBON_PROP_ITER_STATE_BOOL_ARRAYS,
    CARBON_PROP_ITER_STATE_INT8_ARRAYS,
    CARBON_PROP_ITER_STATE_INT16_ARRAYS,
    CARBON_PROP_ITER_STATE_INT32_ARRAYS,
    CARBON_PROP_ITER_STATE_INT64_ARRAYS,
    CARBON_PROP_ITER_STATE_UINT8_ARRAYS,
    CARBON_PROP_ITER_STATE_UINT16_ARRAYS,
    CARBON_PROP_ITER_STATE_UINT32_ARRAYS,
    CARBON_PROP_ITER_STATE_UINT64_ARRAYS,
    CARBON_PROP_ITER_STATE_FLOAT_ARRAYS,
    CARBON_PROP_ITER_STATE_STRING_ARRAYS,
    CARBON_PROP_ITER_STATE_OBJECT_ARRAYS,

    CARBON_PROP_ITER_STATE_DONE
} carbon_prop_iter_state_e;


typedef struct
{
    carbon_object_id_t            object_id;     /* unique object id */
    carbon_off_t                  offset;        /* this objects header offset */
    carbon_archive_prop_offs_t    prop_offsets;  /* per-property type offset in the record table byte stream */
    carbon_off_t                  next_obj_off;  /* offset to next object in list, or NULL if no such exists */
} carbon_archive_object_t;
//
//typedef struct
//{
//    size_t                         ngroups;
//    const carbon_string_id_t      *keys;
//    const carbon_off_t            *groups_offsets;
//    carbon_archive_object_t       *context;
//    carbon_err_t                   err;
//} carbon_archive_table_t;
//
//typedef struct
//{
//    size_t                         ncolumns;
//    const carbon_off_t            *column_offsets;
//    carbon_archive_object_t       *context;
//    carbon_err_t                   err;
//} carbon_column_group_t;
//
//typedef struct
//{
//    size_t                         nelems;
//    carbon_field_type_e            type;
//    const carbon_off_t            *entry_offsets;
//    const uint32_t                *position_list;
//    carbon_archive_object_t       *context;
//    carbon_err_t                   err;
//} carbon_column_t;
//
//typedef struct
//{
//    carbon_off_t                   data_offset;
//    uint32_t                       nentries;
//    carbon_field_type_e            type;
//    carbon_archive_object_t       *context;
//    const void                    *data;
//} carbon_field_t;
//
//typedef struct
//{
//    carbon_field_t *field;
//    carbon_memblock_t *mem_block;
//    uint32_t current_idx;
//    uint32_t max_idx;
//    carbon_archive_object_t obj;
//} carbon_object_cursor_t;

typedef struct carbon_archive_prop_iter
{
    //carbon_archive_t          *archive;                 /* pointer to archive that is iterated */
    carbon_archive_object_t    object;                  /* current object */
    carbon_memfile_t           record_table_memfile;    /* iterator-local read-only memfile on archive record table */

    uint16_t                   mask;                    /* user-defined mask which properties to include */
    carbon_prop_iter_state_e   prop_cursor;             /* current property type in iteration */

    uint32_t                   prop_pos_current;        /* current position */
    carbon_off_t               prop_type_off_current;   /* current offset in memfile to start of prop group */
    carbon_off_t               prop_type_off_data;      /* offset of type-dependent data in memfile */
    carbon_fixed_prop_t        prop_header;             /* type, num props and keys */

    carbon_string_id_t         key;                     /* current property key in this iteration */
    carbon_basic_type_e              type;                    /* property basic value type (e.g., int8, or object) */
    bool                       is_array;                /* flag indicating that property is an array type */
    carbon_err_t               err;                     /* error information */
} carbon_archive_prop_iter_t;

typedef struct carbon_archive_value
{
    carbon_archive_prop_iter_t *prop_iter;               /* pointer to property iterator that created this iterator */
    carbon_memfile_t            record_table_memfile;    /* iterator-local read-only memfile on archive record table */
    carbon_basic_type_e         prop_type;               /* property basic value type (e.g., int8, or object) */
    bool                        is_array;                /* flag indicating whether value type is an array or not */
    carbon_off_t                data_off;                /* offset in memfile where type-dependend data begins */
    uint32_t                    value_idx;               /* the index of this value in the group (also index of key) */
    uint32_t                    value_idx_max;           /* max index of this value in the group (also index of key) */

    carbon_err_t                err;                     /* error information */

    union {
        union {

        } array;

        union {
            carbon_int8_t           int8;
            carbon_int16_t          int16;
            carbon_int32_t          int32;
            carbon_int64_t          int64;
            carbon_uint8_t          uint8;
            carbon_uint16_t         uint16;
            carbon_uint32_t         uint32;
            carbon_uint64_t         uint64;
            carbon_float_t          number;
            carbon_string_id_t      string;
            carbon_bool_t           boolean;
            carbon_archive_object_t object;
        } basic;

    } data;


} carbon_archive_value_t;




#define CARBON_ARCHIVE_ITER_MASK_PRIMITIVES             (1 << 1)
#define CARBON_ARCHIVE_ITER_MASK_ARRAYS                 (1 << 2)

#define CARBON_ARCHIVE_ITER_MASK_INT8                   (1 << 3)
#define CARBON_ARCHIVE_ITER_MASK_INT16                  (1 << 4)
#define CARBON_ARCHIVE_ITER_MASK_INT32                  (1 << 5)
#define CARBON_ARCHIVE_ITER_MASK_INT64                  (1 << 6)
#define CARBON_ARCHIVE_ITER_MASK_UINT8                  (1 << 7)
#define CARBON_ARCHIVE_ITER_MASK_UINT16                 (1 << 8)
#define CARBON_ARCHIVE_ITER_MASK_UINT32                 (1 << 9)
#define CARBON_ARCHIVE_ITER_MASK_UINT64                 (1 << 10)
#define CARBON_ARCHIVE_ITER_MASK_NUMBER                 (1 << 11)
#define CARBON_ARCHIVE_ITER_MASK_STRING                 (1 << 12)
#define CARBON_ARCHIVE_ITER_MASK_BOOLEAN                (1 << 13)
#define CARBON_ARCHIVE_ITER_MASK_NULL                   (1 << 14)
#define CARBON_ARCHIVE_ITER_MASK_OBJECT                 (1 << 15)

#define CARBON_ARCHIVE_ITER_MASK_INTEGER                CARBON_ARCHIVE_ITER_MASK_INT8       |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_INT16      |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_INT32      |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_INT64      |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_UINT8      |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_UINT16     |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_UINT32     |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_UINT64

#define CARBON_ARCHIVE_ITER_MASK_ANY                    CARBON_ARCHIVE_ITER_MASK_PRIMITIVES |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_ARRAYS     |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_INTEGER    |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_NUMBER     |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_STRING     |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_BOOLEAN    |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_NULL       |                          \
                                                    CARBON_ARCHIVE_ITER_MASK_OBJECT


CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_archive(carbon_archive_prop_iter_t *iter,
                                  carbon_err_t *err,
                                  uint16_t mask,
                                  carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_object(carbon_archive_prop_iter_t *iter,
                                 uint16_t mask,
                                 carbon_archive_object_t *obj,
                                 carbon_archive_value_t *value);

CARBON_EXPORT(bool)
carbon_archive_prop_iter_next(carbon_string_id_t *key, carbon_archive_value_t *value,
                          carbon_archive_prop_iter_t *iter);

CARBON_EXPORT(bool)
carbon_archive_prop_iter_get_object_id(carbon_object_id_t *id, carbon_archive_prop_iter_t *iter);



CARBON_EXPORT(bool)
carbon_archive_value_from_prop_iter(carbon_archive_value_t *value,
                                    carbon_err_t *err,
                                    carbon_archive_prop_iter_t *prop_iter);

CARBON_EXPORT(bool)
carbon_archive_value_get_basic_type(carbon_basic_type_e *type, const carbon_archive_value_t *value);

CARBON_EXPORT(bool)
carbon_archive_value_is_array_type(bool *is_array, const carbon_archive_value_t *value);


CARBON_EXPORT(bool)
carbon_archive_value_is_object(bool *is_object, carbon_archive_value_t *value);

CARBON_EXPORT(bool)
carbon_archive_value_get_object(carbon_archive_object_t *object, carbon_archive_value_t *value);


CARBON_EXPORT(bool)
carbon_archive_value_is_null(bool *is_null, carbon_archive_value_t *value);


#define DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(type, name)                                                                 \
    CARBON_EXPORT(bool)                                                                                                \
    carbon_archive_value_is_##name (bool *is_##name, carbon_archive_value_t *value);                                   \
                                                                                                                       \
    CARBON_EXPORT(bool)                                                                                                \
    carbon_archive_value_get_##name (type *name, carbon_archive_value_t *value);                                       \

DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_bool_t, boolean)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int8_t, int8)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int16_t, int16)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int32_t, int32)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int64_t, int64)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint8_t, uint8)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint16_t, uint16)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint32_t, uint32)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint64_t, uint64)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_float_t, number)
DECLARE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_string_id_t, string)

//CARBON_EXPORT(bool)
//carbon_archive_value_is_boolean(bool *is_boolean, carbon_archive_value_t *value);

//CARBON_EXPORT(bool)
//carbon_archive_value_get_boolean(bool *boolean, carbon_archive_value_t *value);

//CARBON_EXPORT(bool)
//carbon_archive_value_iter_position(uint32_t *pos, carbon_archive_value_iter_t *iter);
//
//CARBON_EXPORT(const void *)
//carbon_archive_value_iter_values(uint32_t *num_values, carbon_archive_value_iter_t *iter);
//
//CARBON_EXPORT(bool)
//carbon_archive_value_iter_next_object(carbon_archive_prop_iter_t *iter, carbon_archive_value_iter_t *value_iter);
//
//CARBON_EXPORT(bool)
//carbon_archive_value_iter_close_and_drop(carbon_archive_value_iter_t *iter);








void
carbon_int_reset_cabin_object_mem_file(carbon_archive_object_t *object);




CARBON_END_DECL

#endif
