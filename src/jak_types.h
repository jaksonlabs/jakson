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

#ifndef JAK_TYPES_H
#define JAK_TYPES_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef uint8_t jak_u8;
typedef uint16_t jak_u16;
typedef uint32_t jak_u32;
typedef uint64_t jak_u64;
typedef int8_t jak_i8;
typedef int16_t jak_i16;
typedef int32_t jak_i32;
typedef int64_t jak_i64;
typedef float jak_float;

#define JAK_U8_NULL         UINT8_MAX
#define JAK_U16_NULL        UINT16_MAX
#define JAK_U32_NULL        UINT32_MAX
#define JAK_U64_NULL        UINT64_MAX
#define JAK_I8_NULL         INT8_MIN
#define JAK_I16_NULL        INT16_MIN
#define JAK_I32_NULL        INT32_MIN
#define JAK_I64_NULL        INT64_MIN
#define JAK_FLOAT_NULL      NAN

#define JAK_CARBON_U8_MIN    UINT8_MIN
#define JAK_CARBON_U16_MIN   UINT16_MIN
#define JAK_CARBON_U32_MIN   UINT32_MIN
#define JAK_CARBON_U64_MIN   UINT64_MIN
#define JAK_CARBON_I8_MIN    (INT8_MIN + 1)
#define JAK_CARBON_I16_MIN   (INT16_MIN + 1)
#define JAK_CARBON_I32_MIN   (INT32_MIN + 1)
#define JAK_CARBON_I64_MIN   (INT64_MIN + 1)
#define JAK_CARBON_U8_MAX    (JAK_U8_NULL - 1)
#define JAK_CARBON_U16_MAX   (JAK_U16_NULL - 1)
#define JAK_CARBON_U32_MAX   (JAK_U32_NULL - 1)
#define JAK_CARBON_U64_MAX   (JAK_U64_NULL - 1)
#define JAK_CARBON_I8_MAX    INT8_MAX
#define JAK_CARBON_I16_MAX   INT16_MAX
#define JAK_CARBON_I32_MAX   INT32_MAX
#define JAK_CARBON_I64_MAX   INT64_MAX

#define JAK_CARBON_BOOLEAN_COLUMN_FALSE     0
#define JAK_CARBON_BOOLEAN_COLUMN_TRUE      1
#define JAK_CARBON_BOOLEAN_COLUMN_NULL      2

#define JAK_IS_NULL_BOOLEAN(x)      (x == JAK_CARBON_BOOLEAN_COLUMN_NULL)
#define JAK_IS_NULL_U8(x)           (x == JAK_U8_NULL)
#define JAK_IS_NULL_U16(x)          (x == JAK_U16_NULL)
#define JAK_IS_NULL_U32(x)          (x == JAK_U32_NULL)
#define JAK_IS_NULL_U64(x)          (x == JAK_U64_NULL)
#define JAK_IS_NULL_I8(x)           (x == JAK_I8_NULL)
#define JAK_IS_NULL_I16(x)          (x == JAK_I16_NULL)
#define JAK_IS_NULL_I32(x)          (x == JAK_I32_NULL)
#define JAK_IS_NULL_I64(x)          (x == JAK_I64_NULL)
#define JAK_IS_NULL_FLOAT(x)        (isnan(x))

typedef jak_u64 jak_archive_field_sid_t;  /* string identifier, resolvable by a string dictionary */
typedef char field_null_t;
typedef jak_i8 jak_archive_field_boolean_t;
typedef jak_i8 jak_archive_field_i8_t;
typedef jak_i16 jak_archive_field_i16_t;
typedef jak_i32 jak_archive_field_i32_t;
typedef jak_i64 jak_archive_field_i64_t;
typedef jak_u8 jak_archive_field_u8_t;
typedef jak_u16 jak_archive_field_u16_t;
typedef jak_u32 jak_archive_field_u32_t;
typedef jak_u64 jak_archive_field_u64_t;
typedef float jak_archive_field_number_t;
typedef const char *field_jak_string_t;

#define JAK_NULL_ENCODED_STRING            0
#define JAK_NULL_BOOLEAN                   INT8_MAX
#define JAK_NULL_INT8                      INT8_MAX
#define JAK_NULL_INT16                     INT16_MAX
#define JAK_NULL_INT32                     INT32_MAX
#define JAK_NULL_INT64                     INT64_MAX
#define JAK_NULL_UINT8                     UINT8_MAX
#define JAK_NULL_UINT16                    UINT16_MAX
#define JAK_NULL_UINT32                    UINT32_MAX
#define JAK_NULL_UINT64                    UINT64_MAX
#define JAK_NULL_FLOAT                     NAN
#define JAK_NULL_OBJECT_MODEL(objectModel) (objectModel->entries.num_elems == 0)

#define JAK_IS_NULL_STRING(str)   (str == JAK_NULL_ENCODED_STRING)
#define JAK_IS_NULL_BOOL(val)     (val == JAK_NULL_BOOLEAN)
#define JAK_IS_NULL_INT8(val)     (val == JAK_NULL_INT8)
#define JAK_IS_NULL_INT16(val)    (val == JAK_NULL_INT16)
#define JAK_IS_NULL_INT32(val)    (val == JAK_NULL_INT32)
#define JAK_IS_NULL_INT64(val)    (val == JAK_NULL_INT64)
#define JAK_IS_NULL_UINT8(val)    (val == JAK_NULL_UINT8)
#define JAK_IS_NULL_UINT16(val)   (val == JAK_NULL_UINT16)
#define JAK_IS_NULL_UINT32(val)   (val == JAK_NULL_UINT32)
#define JAK_IS_NULL_UINT64(val)   (val == JAK_NULL_UINT64)
#define JAK_IS_NULL_NUMBER(val)   (val == JAK_NULL_FLOAT)

#define JAK_LIMITS_INT8_MAX                (JAK_NULL_INT8 - 1)
#define JAK_LIMITS_INT16_MAX               (JAK_NULL_INT16 - 1)
#define JAK_LIMITS_INT32_MAX               (JAK_NULL_INT32 - 1)
#define JAK_LIMITS_INT64_MAX               (JAK_NULL_INT64 - 1)
#define JAK_LIMITS_UINT8_MAX               (JAK_NULL_UINT8 - 1)
#define JAK_LIMITS_UINT16_MAX              (JAK_NULL_UINT16 - 1)
#define JAK_LIMITS_UINT32_MAX              (JAK_NULL_UINT32 - 1)
#define JAK_LIMITS_UINT64_MAX              (JAK_NULL_UINT64 - 1)

#define JAK_LIMITS_INT8_MIN                INT8_MIN
#define JAK_LIMITS_INT16_MIN               INT16_MIN
#define JAK_LIMITS_INT32_MIN               INT32_MIN
#define JAK_LIMITS_INT64_MIN               INT64_MIN
#define JAK_LIMITS_UINT8_MIN               0
#define JAK_LIMITS_UINT16_MIN              0
#define JAK_LIMITS_UINT32_MIN              0
#define JAK_LIMITS_UINT64_MIN              0

#define JAK_NULL_TEXT "null"

#define JAK_BOOLEAN_FALSE 0
#define JAK_BOOLEAN_TRUE  1

#define GET_TYPE_SIZE(value_type)                                                                                      \
({                                                                                                                     \
    size_t value_size;                                                                                                 \
    switch (value_type) {                                                                                              \
        case JAK_FIELD_NULL:                                                                                           \
            value_size = sizeof(jak_u16);                                                                              \
            break;                                                                                                     \
        case JAK_FIELD_BOOLEAN:                                                                                        \
            value_size = sizeof(jak_archive_field_boolean_t);                                                          \
            break;                                                                                                     \
        case JAK_FIELD_INT8:                                                                                           \
            value_size = sizeof(jak_archive_field_i8_t);                                                               \
            break;                                                                                                     \
        case JAK_FIELD_INT16:                                                                                          \
            value_size = sizeof(jak_archive_field_i16_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_INT32:                                                                                          \
            value_size = sizeof(jak_archive_field_i32_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_INT64:                                                                                          \
            value_size = sizeof(jak_archive_field_i64_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT8:                                                                                          \
            value_size = sizeof(jak_archive_field_u8_t);                                                               \
            break;                                                                                                     \
        case JAK_FIELD_UINT16:                                                                                         \
            value_size = sizeof(jak_archive_field_u16_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT32:                                                                                         \
            value_size = sizeof(jak_archive_field_u32_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_UINT64:                                                                                         \
            value_size = sizeof(jak_archive_field_u64_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_FLOAT:                                                                                          \
            value_size = sizeof(jak_archive_field_number_t);                                                           \
            break;                                                                                                     \
        case JAK_FIELD_STRING:                                                                                         \
            value_size = sizeof(jak_archive_field_sid_t);                                                              \
            break;                                                                                                     \
        case JAK_FIELD_OBJECT:                                                                                         \
            value_size = sizeof(jak_column_doc_obj);                                                                   \
            break;                                                                                                     \
        default:                                                                                                       \
        JAK_ERROR_PRINT_AND_DIE(JAK_ERR_NOTYPE);                                                                       \
    }                                                                                                                  \
    value_size;                                                                                                        \
})

JAK_END_DECL

#endif