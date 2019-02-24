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

#ifndef CARBON_TYPES_H
#define CARBON_TYPES_H

#include "carbon-common.h"

CARBON_BEGIN_DECL

typedef struct carbon carbon_t; /* forwarded from 'carbon.h' */

typedef struct carbon_doc_obj carbon_doc_obj_t;
typedef uint64_t              carbon_string_id_t;
typedef char                  carbon_null_t;
typedef int8_t                carbon_boolean_t;
typedef int8_t                carbon_int8_t;
typedef int16_t               carbon_int16_t;
typedef int32_t               carbon_int32_t;
typedef int64_t               carbon_int64_t;
typedef uint8_t               carbon_uint8_t;
typedef uint16_t              carbon_uint16_t;
typedef uint32_t              carbon_uint32_t;
typedef uint64_t              carbon_uint64_t;
typedef float                 carbon_number_t;
typedef const char *          carbon_cstring_t;

#define CARBON_NULL_ENCODED_STRING            0
#define CARBON_NULL_BOOLEAN                   INT8_MAX
#define CARBON_NULL_INT8                      INT8_MAX
#define CARBON_NULL_INT16                     INT16_MAX
#define CARBON_NULL_INT32                     INT32_MAX
#define CARBON_NULL_INT64                     INT64_MAX
#define CARBON_NULL_UINT8                     UINT8_MAX
#define CARBON_NULL_UINT16                    UINT16_MAX
#define CARBON_NULL_UINT32                    UINT32_MAX
#define CARBON_NULL_UINT64                    UINT64_MAX
#define CARBON_NULL_FLOAT                     NAN
#define CARBON_NULL_OBJECT_MODEL(objectModel) (objectModel->entries.num_elems == 0)

#define CARBON_IS_NULL_STRING(str)   (str == CARBON_NULL_ENCODED_STRING)
#define CARBON_IS_NULL_BOOLEAN(val)  (val == CARBON_NULL_BOOLEAN)
#define CARBON_IS_NULL_INT8(val)     (val == CARBON_NULL_INT8)
#define CARBON_IS_NULL_INT16(val)    (val == CARBON_NULL_INT16)
#define CARBON_IS_NULL_INT32(val)    (val == CARBON_NULL_INT32)
#define CARBON_IS_NULL_INT64(val)    (val == CARBON_NULL_INT64)
#define CARBON_IS_NULL_UINT8(val)    (val == CARBON_NULL_UINT8)
#define CARBON_IS_NULL_UINT16(val)   (val == CARBON_NULL_UINT16)
#define CARBON_IS_NULL_UINT32(val)   (val == CARBON_NULL_UINT32)
#define CARBON_IS_NULL_UINT64(val)   (val == CARBON_NULL_UINT64)
#define CARBON_IS_NULL_NUMBER(val)   (val == CARBON_NULL_FLOAT)

#define CARBON_LIMITS_INT8_MAX                (CARBON_NULL_INT8 - 1)
#define CARBON_LIMITS_INT16_MAX               (CARBON_NULL_INT16 - 1)
#define CARBON_LIMITS_INT32_MAX               (CARBON_NULL_INT32 - 1)
#define CARBON_LIMITS_INT64_MAX               (CARBON_NULL_INT64 - 1)
#define CARBON_LIMITS_UINT8_MAX               (CARBON_NULL_UINT8 - 1)
#define CARBON_LIMITS_UINT16_MAX              (CARBON_NULL_UINT16 - 1)
#define CARBON_LIMITS_UINT32_MAX              (CARBON_NULL_UINT32 - 1)
#define CARBON_LIMITS_UINT64_MAX              (CARBON_NULL_UINT64 - 1)

#define CARBON_LIMITS_INT8_MIN                INT8_MIN
#define CARBON_LIMITS_INT16_MIN               INT16_MIN
#define CARBON_LIMITS_INT32_MIN               INT32_MIN
#define CARBON_LIMITS_INT64_MIN               INT64_MIN
#define CARBON_LIMITS_UINT8_MIN               0
#define CARBON_LIMITS_UINT16_MIN              0
#define CARBON_LIMITS_UINT32_MIN              0
#define CARBON_LIMITS_UINT64_MIN              0

#define CARBON_NULL_TEXT "null"

#define CARBON_BOOLEAN_FALSE 0
#define CARBON_BOOLEAN_TRUE  1

typedef enum carbon_field_type
{
    carbon_field_type_null          = 0,
    carbon_field_type_bool          = 1,
    carbon_field_type_int8          = 2,
    carbon_field_type_int16         = 3,
    carbon_field_type_int32         = 4,
    carbon_field_type_int64         = 5,
    carbon_field_type_uint8         = 6,
    carbon_field_type_uint16        = 7,
    carbon_field_type_uint32        = 8,
    carbon_field_type_uint64        = 9,
    carbon_field_type_float         = 10,
    carbon_field_type_string        = 11,
    carbon_field_type_object        = 12
} carbon_field_type_e;

#define GET_TYPE_SIZE(value_type)                                                                                       \
({                                                                                                                     \
    size_t value_size;                                                                                                 \
    switch (value_type) {                                                                                               \
        case carbon_field_type_null:                                                                                   \
            value_size = sizeof(uint16_t);                                                                             \
            break;                                                                                                     \
        case carbon_field_type_bool:                                                                                   \
            value_size = sizeof(carbon_boolean_t);                                                                        \
            break;                                                                                                     \
        case carbon_field_type_int8:                                                                                   \
            value_size = sizeof(carbon_int8_t);                                                                        \
            break;                                                                                                     \
        case carbon_field_type_int16:                                                                                  \
            value_size = sizeof(carbon_int16_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_int32:                                                                                  \
            value_size = sizeof(carbon_int32_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_int64:                                                                                  \
            value_size = sizeof(carbon_int64_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_uint8:                                                                                  \
            value_size = sizeof(carbon_uint8_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_uint16:                                                                                 \
            value_size = sizeof(carbon_uint16_t);                                                                      \
            break;                                                                                                     \
        case carbon_field_type_uint32:                                                                                 \
            value_size = sizeof(carbon_uint32_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_uint64:                                                                                 \
            value_size = sizeof(carbon_uint64_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_float:                                                                                  \
            value_size = sizeof(carbon_number_t);                                                                       \
            break;                                                                                                     \
        case carbon_field_type_string:                                                                                 \
            value_size = sizeof(carbon_string_id_t);                                                                   \
            break;                                                                                                     \
        case carbon_field_type_object:                                                                                 \
            value_size = sizeof(carbon_columndoc_obj_t);                                                               \
            break;                                                                                                     \
        default:                                                                                                       \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NOTYPE);                                                                 \
    }                                                                                                                  \
    value_size;                                                                                                        \
})

typedef bool (*carbon_predicate_string_t)(carbon_string_id_t id, const void *capture, carbon_t *context);

CARBON_END_DECL

#endif