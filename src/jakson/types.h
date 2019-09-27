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

#ifndef TYPES_H
#define TYPES_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>

BEGIN_DECL

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define U8_NULL         UINT8_MAX
#define U16_NULL        UINT16_MAX
#define U32_NULL        UINT32_MAX
#define U64_NULL        UINT64_MAX
#define I8_NULL         INT8_MIN
#define I16_NULL        INT16_MIN
#define I32_NULL        INT32_MIN
#define I64_NULL        INT64_MIN
#define FLOAT_NULL      NAN

#define CARBON_U8_MIN    UINT8_MIN
#define CARBON_U16_MIN   UINT16_MIN
#define CARBON_U32_MIN   UINT32_MIN
#define CARBON_U64_MIN   UINT64_MIN
#define CARBON_I8_MIN    (INT8_MIN + 1)
#define CARBON_I16_MIN   (INT16_MIN + 1)
#define CARBON_I32_MIN   (INT32_MIN + 1)
#define CARBON_I64_MIN   (INT64_MIN + 1)
#define CARBON_U8_MAX    (U8_NULL - 1)
#define CARBON_U16_MAX   (U16_NULL - 1)
#define CARBON_U32_MAX   (U32_NULL - 1)
#define CARBON_U64_MAX   (U64_NULL - 1)
#define CARBON_I8_MAX    INT8_MAX
#define CARBON_I16_MAX   INT16_MAX
#define CARBON_I32_MAX   INT32_MAX
#define CARBON_I64_MAX   INT64_MAX

#define CARBON_BOOLEAN_COLUMN_FALSE     0
#define CARBON_BOOLEAN_COLUMN_TRUE      1
#define CARBON_BOOLEAN_COLUMN_NULL      2

#define IS_NULL_BOOLEAN(x)      (x == CARBON_BOOLEAN_COLUMN_NULL)
#define IS_NULL_U8(x)           (x == U8_NULL)
#define IS_NULL_U16(x)          (x == U16_NULL)
#define IS_NULL_U32(x)          (x == U32_NULL)
#define IS_NULL_U64(x)          (x == U64_NULL)
#define IS_NULL_I8(x)           (x == I8_NULL)
#define IS_NULL_I16(x)          (x == I16_NULL)
#define IS_NULL_I32(x)          (x == I32_NULL)
#define IS_NULL_I64(x)          (x == I64_NULL)
#define IS_NULL_FLOAT(x)        (isnan(x))

typedef u64 archive_field_sid_t;  /** string_buffer identifier, resolvable by a string_buffer dictionary */
typedef char field_null_t;
typedef i8 archive_field_boolean_t;
typedef i8 archive_field_i8_t;
typedef i16 archive_field_i16_t;
typedef i32 archive_field_i32_t;
typedef i64 archive_field_i64_t;
typedef u8 archive_field_u8_t;
typedef u16 archive_field_u16_t;
typedef u32 archive_field_u32_t;
typedef u64 archive_field_u64_t;
typedef float archive_field_number_t;
typedef const char *field_string_t;

#define NULL_ENCODED_STRING            0
#define NULL_BOOLEAN                   INT8_MAX
#define NULL_INT8                      INT8_MAX
#define NULL_INT16                     INT16_MAX
#define NULL_INT32                     INT32_MAX
#define NULL_INT64                     INT64_MAX
#define NULL_UINT8                     UINT8_MAX
#define NULL_UINT16                    UINT16_MAX
#define NULL_UINT32                    UINT32_MAX
#define NULL_UINT64                    UINT64_MAX
#define NULL_FLOAT                     NAN
#define NULL_OBJECT_MODEL(objectModel) (objectModel->entries.num_elems == 0)

#define IS_NULL_STRING(str)   (str == NULL_ENCODED_STRING)
#define IS_NULL_BOOL(val)     (val == NULL_BOOLEAN)
#define IS_NULL_INT8(val)     (val == NULL_INT8)
#define IS_NULL_INT16(val)    (val == NULL_INT16)
#define IS_NULL_INT32(val)    (val == NULL_INT32)
#define IS_NULL_INT64(val)    (val == NULL_INT64)
#define IS_NULL_UINT8(val)    (val == NULL_UINT8)
#define IS_NULL_UINT16(val)   (val == NULL_UINT16)
#define IS_NULL_UINT32(val)   (val == NULL_UINT32)
#define IS_NULL_UINT64(val)   (val == NULL_UINT64)
#define IS_NULL_NUMBER(val)   (val == NULL_FLOAT)

#define LIMITS_INT8_MAX                (NULL_INT8 - 1)
#define LIMITS_INT16_MAX               (NULL_INT16 - 1)
#define LIMITS_INT32_MAX               (NULL_INT32 - 1)
#define LIMITS_INT64_MAX               (NULL_INT64 - 1)
#define LIMITS_UINT8_MAX               (NULL_UINT8 - 1)
#define LIMITS_UINT16_MAX              (NULL_UINT16 - 1)
#define LIMITS_UINT32_MAX              (NULL_UINT32 - 1)
#define LIMITS_UINT64_MAX              (NULL_UINT64 - 1)

#define LIMITS_INT8_MIN                INT8_MIN
#define LIMITS_INT16_MIN               INT16_MIN
#define LIMITS_INT32_MIN               INT32_MIN
#define LIMITS_INT64_MIN               INT64_MIN
#define LIMITS_UINT8_MIN               0
#define LIMITS_UINT16_MIN              0
#define LIMITS_UINT32_MIN              0
#define LIMITS_UINT64_MIN              0

#define NULL_TEXT "null"

#define BOOLEAN_FALSE 0
#define BOOLEAN_TRUE  1

#define GET_TYPE_SIZE(value_type)                                                                                      \
({                                                                                                                     \
    size_t value_size;                                                                                                 \
    switch (value_type) {                                                                                              \
        case FIELD_NULL:                                                                                           \
            value_size = sizeof(u16);                                                                              \
            break;                                                                                                     \
        case FIELD_BOOLEAN:                                                                                        \
            value_size = sizeof(archive_field_boolean_t);                                                          \
            break;                                                                                                     \
        case FIELD_INT8:                                                                                           \
            value_size = sizeof(archive_field_i8_t);                                                               \
            break;                                                                                                     \
        case FIELD_INT16:                                                                                          \
            value_size = sizeof(archive_field_i16_t);                                                              \
            break;                                                                                                     \
        case FIELD_INT32:                                                                                          \
            value_size = sizeof(archive_field_i32_t);                                                              \
            break;                                                                                                     \
        case FIELD_INT64:                                                                                          \
            value_size = sizeof(archive_field_i64_t);                                                              \
            break;                                                                                                     \
        case FIELD_UINT8:                                                                                          \
            value_size = sizeof(archive_field_u8_t);                                                               \
            break;                                                                                                     \
        case FIELD_UINT16:                                                                                         \
            value_size = sizeof(archive_field_u16_t);                                                              \
            break;                                                                                                     \
        case FIELD_UINT32:                                                                                         \
            value_size = sizeof(archive_field_u32_t);                                                              \
            break;                                                                                                     \
        case FIELD_UINT64:                                                                                         \
            value_size = sizeof(archive_field_u64_t);                                                              \
            break;                                                                                                     \
        case FIELD_FLOAT:                                                                                          \
            value_size = sizeof(archive_field_number_t);                                                           \
            break;                                                                                                     \
        case FIELD_STRING:                                                                                         \
            value_size = sizeof(archive_field_sid_t);                                                              \
            break;                                                                                                     \
        case FIELD_OBJECT:                                                                                         \
            value_size = sizeof(column_doc_obj);                                                                   \
            break;                                                                                                     \
        default:                                                                                                       \
        ERROR_PRINT_AND_DIE(ERR_NOTYPE);                                                                       \
    }                                                                                                                  \
    value_size;                                                                                                        \
})

END_DECL

#endif