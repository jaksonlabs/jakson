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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>

#ifndef JAK_CARBON_JAK_FIELD_H
#define JAK_CARBON_JAK_FIELD_H

typedef enum jak_carbon_field_type {
        /* constants */
        JAK_CARBON_FIELD_TYPE_NULL = JAK_CARBON_MARKER_NULL, /* null */
        JAK_CARBON_FIELD_TYPE_TRUE = JAK_CARBON_MARKER_TRUE, /* true */
        JAK_CARBON_FIELD_TYPE_FALSE = JAK_CARBON_MARKER_FALSE, /* false */

        /* containers */
        JAK_CARBON_FIELD_TYPE_OBJECT = JAK_CARBON_MARKER_OBJECT_BEGIN, /* object */
        JAK_CARBON_FIELD_TYPE_ARRAY = JAK_CARBON_MARKER_ARRAY_BEGIN, /* variable-type array of elements of varying type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U8 = JAK_CARBON_MARKER_COLUMN_U8, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U16 = JAK_CARBON_MARKER_COLUMN_U16, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U32 = JAK_CARBON_MARKER_COLUMN_U32, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_U64 = JAK_CARBON_MARKER_COLUMN_U64, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I8 = JAK_CARBON_MARKER_COLUMN_I8, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I16 = JAK_CARBON_MARKER_COLUMN_I16, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I32 = JAK_CARBON_MARKER_COLUMN_I32, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_I64 = JAK_CARBON_MARKER_COLUMN_I64, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT = JAK_CARBON_MARKER_COLUMN_FLOAT, /* fixed-type array of elements of particular type */
        JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN = JAK_CARBON_MARKER_COLUMN_BOOLEAN, /* fixed-type array of elements of particular type */

        /* character strings */
        JAK_CARBON_FIELD_TYPE_STRING = JAK_CARBON_MARKER_STRING, /* UTF-8 string */

        /* numbers */
        JAK_CARBON_FIELD_TYPE_NUMBER_U8 = JAK_CARBON_MARKER_U8, /* 8bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U16 = JAK_CARBON_MARKER_U16, /* 16bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U32 = JAK_CARBON_MARKER_U32, /* 32bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_U64 = JAK_CARBON_MARKER_U64, /* 64bit unsigned integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I8 = JAK_CARBON_MARKER_I8, /* 8bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I16 = JAK_CARBON_MARKER_I16, /* 16bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I32 = JAK_CARBON_MARKER_I32, /* 32bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_I64 = JAK_CARBON_MARKER_I64, /* 64bit signed integer */
        JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT = JAK_CARBON_MARKER_FLOAT, /* 32bit float */

        /* binary data */
        JAK_CARBON_FIELD_TYPE_BINARY = JAK_CARBON_MARKER_BINARY, /* arbitrary binary object with known mime type */
        JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM = JAK_CARBON_MARKER_CUSTOM_BINARY, /* arbitrary binary object with unknown mime type*/
} jak_carbon_field_type_e;

typedef enum jak_carbon_column_type {
        JAK_CARBON_COLUMN_TYPE_U8,
        JAK_CARBON_COLUMN_TYPE_U16,
        JAK_CARBON_COLUMN_TYPE_U32,
        JAK_CARBON_COLUMN_TYPE_U64,
        JAK_CARBON_COLUMN_TYPE_I8,
        JAK_CARBON_COLUMN_TYPE_I16,
        JAK_CARBON_COLUMN_TYPE_I32,
        JAK_CARBON_COLUMN_TYPE_I64,
        JAK_CARBON_COLUMN_TYPE_FLOAT,
        JAK_CARBON_COLUMN_TYPE_BOOLEAN
} jak_carbon_column_type_e;

typedef enum jak_carbon_field_class {
        JAK_CARBON_FIELD_CLASS_CONSTANT,
        JAK_CARBON_FIELD_CLASS_NUMBER,
        JAK_CARBON_FIELD_CLASS_CHARACTER_STRING,
        JAK_CARBON_FIELD_CLASS_BINARY_STRING,
        JAK_CARBON_FIELD_CLASS_CONTAINER
} jak_carbon_field_class_e;

typedef enum jak_carbon_constant {
        JAK_CARBON_CONSTANT_TRUE,
        JAK_CARBON_CONSTANT_FALSE,
        JAK_CARBON_CONSTANT_NULL
} jak_carbon_constant_e;

#define JAK_CARBON_FIELD_TYPE_NULL_STR "null"
#define JAK_CARBON_FIELD_TYPE_TRUE_STR "boolean-true"
#define JAK_CARBON_FIELD_TYPE_FALSE_STR "boolean-false"
#define JAK_CARBON_FIELD_TYPE_OBJECT_STR "object"
#define JAK_CARBON_FIELD_TYPE_ARRAY_STR "array"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U8_STR "column-jak_u8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U16_STR "column-jak_u16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U32_STR "column-jak_u32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U64_STR "column-jak_u64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I8_STR "column-jak_i8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I16_STR "column-jak_i16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I32_STR "column-jak_i32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I64_STR "column-jak_i64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT_STR "column-float"
#define JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN_STR "column-boolean"
#define JAK_CARBON_FIELD_TYPE_STRING_STR "string"
#define JAK_CARBON_FIELD_TYPE_BINARY_STR "binary"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U8_STR "number-jak_u8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U16_STR "number-jak_u16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U32_STR "number-jak_u32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U64_STR "number-jak_u64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I8_STR "number-jak_i8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I16_STR "number-jak_i16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I32_STR "number-jak_i32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I64_STR "number-jak_i64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT_STR "number-float"

JAK_BEGIN_DECL

const char *jak_carbon_field_type_str(jak_error *err, jak_carbon_field_type_e type);

bool jak_carbon_field_type_is_traversable(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_signed(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_unsigned(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_floating(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_number(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_integer(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_binary(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_boolean(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_array(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_column(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_object(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_null(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_string(jak_carbon_field_type_e type);
bool jak_carbon_field_type_is_constant(jak_carbon_field_type_e type);

jak_carbon_field_class_e jak_carbon_field_type_get_class(jak_carbon_field_type_e type, jak_error *err);

bool jak_carbon_field_skip(jak_memfile *file);
bool jak_carbon_field_skip_object(jak_memfile *file);
bool jak_carbon_field_skip_array(jak_memfile *file);
bool jak_carbon_field_skip_column(jak_memfile *file);
bool jak_carbon_field_skip_binary(jak_memfile *file);
bool jak_carbon_field_skip_custom_binary(jak_memfile *file);
bool jak_carbon_field_skip_string(jak_memfile *file);
bool jak_carbon_field_skip_float(jak_memfile *file);
bool jak_carbon_field_skip_boolean(jak_memfile *file);
bool jak_carbon_field_skip_null(jak_memfile *file);
bool jak_carbon_field_skip_8(jak_memfile *file);
bool jak_carbon_field_skip_16(jak_memfile *file);
bool jak_carbon_field_skip_32(jak_memfile *file);
bool jak_carbon_field_skip_64(jak_memfile *file);

jak_carbon_field_type_e jak_carbon_field_type_for_column(jak_carbon_column_type_e type);
jak_carbon_field_type_e jak_carbon_field_type_column_entry_to_regular_type(jak_carbon_field_type_e type, bool is_null, bool is_true);

JAK_END_DECL

#endif
