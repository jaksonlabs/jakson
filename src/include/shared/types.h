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

#ifndef NG5_TYPES_H
#define NG5_TYPES_H

#include "shared/common.h"

NG5_BEGIN_DECL

typedef struct types carbon_t; /* forwarded from 'types.h' */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct carbon_doc_obj carbon_doc_obj_t;
typedef u64              carbon_string_id_t;
typedef char                  carbon_null_t;
typedef i8                carbon_boolean_t;
typedef i8                carbon_i8;
typedef i16               carbon_i16;
typedef i32               carbon_i32;
typedef i64               carbon_i64;
typedef u8               carbon_u8;
typedef u16              carbon_u16;
typedef u32              carbon_u32;
typedef u64              carbon_u64;
typedef float                 carbon_number_t;
typedef const char *          carbon_cstring_t;

#define NG5_NULL_ENCODED_STRING            0
#define NG5_NULL_BOOLEAN                   INT8_MAX
#define NG5_NULL_INT8                      INT8_MAX
#define NG5_NULL_INT16                     INT16_MAX
#define NG5_NULL_INT32                     INT32_MAX
#define NG5_NULL_INT64                     INT64_MAX
#define NG5_NULL_UINT8                     UINT8_MAX
#define NG5_NULL_UINT16                    UINT16_MAX
#define NG5_NULL_UINT32                    UINT32_MAX
#define NG5_NULL_UINT64                    UINT64_MAX
#define NG5_NULL_FLOAT                     NAN
#define NG5_NULL_OBJECT_MODEL(objectModel) (objectModel->entries.num_elems == 0)

#define NG5_IS_NULL_STRING(str)   (str == NG5_NULL_ENCODED_STRING)
#define NG5_IS_NULL_BOOLEAN(val)  (val == NG5_NULL_BOOLEAN)
#define NG5_IS_NULL_INT8(val)     (val == NG5_NULL_INT8)
#define NG5_IS_NULL_INT16(val)    (val == NG5_NULL_INT16)
#define NG5_IS_NULL_INT32(val)    (val == NG5_NULL_INT32)
#define NG5_IS_NULL_INT64(val)    (val == NG5_NULL_INT64)
#define NG5_IS_NULL_UINT8(val)    (val == NG5_NULL_UINT8)
#define NG5_IS_NULL_UINT16(val)   (val == NG5_NULL_UINT16)
#define NG5_IS_NULL_UINT32(val)   (val == NG5_NULL_UINT32)
#define NG5_IS_NULL_UINT64(val)   (val == NG5_NULL_UINT64)
#define NG5_IS_NULL_NUMBER(val)   (val == NG5_NULL_FLOAT)

#define NG5_LIMITS_INT8_MAX                (NG5_NULL_INT8 - 1)
#define NG5_LIMITS_INT16_MAX               (NG5_NULL_INT16 - 1)
#define NG5_LIMITS_INT32_MAX               (NG5_NULL_INT32 - 1)
#define NG5_LIMITS_INT64_MAX               (NG5_NULL_INT64 - 1)
#define NG5_LIMITS_UINT8_MAX               (NG5_NULL_UINT8 - 1)
#define NG5_LIMITS_UINT16_MAX              (NG5_NULL_UINT16 - 1)
#define NG5_LIMITS_UINT32_MAX              (NG5_NULL_UINT32 - 1)
#define NG5_LIMITS_UINT64_MAX              (NG5_NULL_UINT64 - 1)

#define NG5_LIMITS_INT8_MIN                INT8_MIN
#define NG5_LIMITS_INT16_MIN               INT16_MIN
#define NG5_LIMITS_INT32_MIN               INT32_MIN
#define NG5_LIMITS_INT64_MIN               INT64_MIN
#define NG5_LIMITS_UINT8_MIN               0
#define NG5_LIMITS_UINT16_MIN              0
#define NG5_LIMITS_UINT32_MIN              0
#define NG5_LIMITS_UINT64_MIN              0

#define NG5_NULL_TEXT "null"

#define NG5_BOOLEAN_FALSE 0
#define NG5_BOOLEAN_TRUE  1

typedef enum carbon_field_type
{
    field_null          = 0,
    field_bool          = 1,
    field_int8          = 2,
    field_int16         = 3,
    field_int32         = 4,
    field_int64         = 5,
    field_uint8         = 6,
    field_uint16        = 7,
    field_uint32        = 8,
    field_uint64        = 9,
    field_float         = 10,
    field_string        = 11,
    field_object        = 12
} field_e;

#define GET_TYPE_SIZE(value_type)                                                                                       \
({                                                                                                                     \
    size_t value_size;                                                                                                 \
    switch (value_type) {                                                                                               \
        case field_null:                                                                                   \
            value_size = sizeof(u16);                                                                             \
            break;                                                                                                     \
        case field_bool:                                                                                   \
            value_size = sizeof(carbon_boolean_t);                                                                        \
            break;                                                                                                     \
        case field_int8:                                                                                   \
            value_size = sizeof(carbon_i8);                                                                        \
            break;                                                                                                     \
        case field_int16:                                                                                  \
            value_size = sizeof(carbon_i16);                                                                       \
            break;                                                                                                     \
        case field_int32:                                                                                  \
            value_size = sizeof(carbon_i32);                                                                       \
            break;                                                                                                     \
        case field_int64:                                                                                  \
            value_size = sizeof(carbon_i64);                                                                       \
            break;                                                                                                     \
        case field_uint8:                                                                                  \
            value_size = sizeof(carbon_u8);                                                                       \
            break;                                                                                                     \
        case field_uint16:                                                                                 \
            value_size = sizeof(carbon_u16);                                                                      \
            break;                                                                                                     \
        case field_uint32:                                                                                 \
            value_size = sizeof(carbon_u32);                                                                       \
            break;                                                                                                     \
        case field_uint64:                                                                                 \
            value_size = sizeof(carbon_u64);                                                                       \
            break;                                                                                                     \
        case field_float:                                                                                  \
            value_size = sizeof(carbon_number_t);                                                                       \
            break;                                                                                                     \
        case field_string:                                                                                 \
            value_size = sizeof(carbon_string_id_t);                                                                   \
            break;                                                                                                     \
        case field_object:                                                                                 \
            value_size = sizeof(columndoc_obj_t);                                                               \
            break;                                                                                                     \
        default:                                                                                                       \
        carbon_print_error_and_die(NG5_ERR_NOTYPE);                                                                 \
    }                                                                                                                  \
    value_size;                                                                                                        \
})

typedef bool (*carbon_predicate_string_t)(carbon_string_id_t id, const void *capture, carbon_t *context);

NG5_END_DECL

#endif