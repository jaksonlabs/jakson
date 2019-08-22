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

#ifndef JAK_CARBON_FIELD_H
#define JAK_CARBON_FIELD_H

enum carbon_field_type {
        /* constants */
        CARBON_FIELD_TYPE_NULL = JAK_CARBON_MARKER_NULL, /* null */
        CARBON_FIELD_TYPE_TRUE = JAK_CARBON_MARKER_TRUE, /* true */
        CARBON_FIELD_TYPE_FALSE = JAK_CARBON_MARKER_FALSE, /* false */

        /* containers */
        CARBON_FIELD_TYPE_OBJECT = JAK_CARBON_MARKER_OBJECT_BEGIN, /* object */
        CARBON_FIELD_TYPE_ARRAY = JAK_CARBON_MARKER_ARRAY_BEGIN, /* variable-type array of elements of varying type */
        CARBON_FIELD_TYPE_COLUMN_U8 = JAK_CARBON_MARKER_COLUMN_U8, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_U16 = JAK_CARBON_MARKER_COLUMN_U16, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_U32 = JAK_CARBON_MARKER_COLUMN_U32, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_U64 = JAK_CARBON_MARKER_COLUMN_U64, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_I8 = JAK_CARBON_MARKER_COLUMN_I8, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_I16 = JAK_CARBON_MARKER_COLUMN_I16, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_I32 = JAK_CARBON_MARKER_COLUMN_I32, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_I64 = JAK_CARBON_MARKER_COLUMN_I64, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_FLOAT = JAK_CARBON_MARKER_COLUMN_FLOAT, /* fixed-type array of elements of particular type */
        CARBON_FIELD_TYPE_COLUMN_BOOLEAN = JAK_CARBON_MARKER_COLUMN_BOOLEAN, /* fixed-type array of elements of particular type */

        /* character strings */
        CARBON_FIELD_TYPE_STRING = JAK_CARBON_MARKER_STRING, /* UTF-8 string */

        /* numbers */
        CARBON_FIELD_TYPE_NUMBER_U8 = JAK_CARBON_MARKER_U8, /* 8bit unsigned integer */
        CARBON_FIELD_TYPE_NUMBER_U16 = JAK_CARBON_MARKER_U16, /* 16bit unsigned integer */
        CARBON_FIELD_TYPE_NUMBER_U32 = JAK_CARBON_MARKER_U32, /* 32bit unsigned integer */
        CARBON_FIELD_TYPE_NUMBER_U64 = JAK_CARBON_MARKER_U64, /* 64bit unsigned integer */
        CARBON_FIELD_TYPE_NUMBER_I8 = JAK_CARBON_MARKER_I8, /* 8bit signed integer */
        CARBON_FIELD_TYPE_NUMBER_I16 = JAK_CARBON_MARKER_I16, /* 16bit signed integer */
        CARBON_FIELD_TYPE_NUMBER_I32 = JAK_CARBON_MARKER_I32, /* 32bit signed integer */
        CARBON_FIELD_TYPE_NUMBER_I64 = JAK_CARBON_MARKER_I64, /* 64bit signed integer */
        CARBON_FIELD_TYPE_NUMBER_FLOAT = JAK_CARBON_MARKER_FLOAT, /* 32bit float */

        /* binary data */
        CARBON_FIELD_TYPE_BINARY = JAK_CARBON_MARKER_BINARY, /* arbitrary binary object with known mime type */
        CARBON_FIELD_TYPE_BINARY_CUSTOM = JAK_CARBON_MARKER_CUSTOM_BINARY, /* arbitrary binary object with unknown mime type*/
};

enum carbon_column_type {
    CARBON_COLUMN_TYPE_U8,
    CARBON_COLUMN_TYPE_U16,
    CARBON_COLUMN_TYPE_U32,
    CARBON_COLUMN_TYPE_U64,
    CARBON_COLUMN_TYPE_I8,
    CARBON_COLUMN_TYPE_I16,
    CARBON_COLUMN_TYPE_I32,
    CARBON_COLUMN_TYPE_I64,
    CARBON_COLUMN_TYPE_FLOAT,
    CARBON_COLUMN_TYPE_BOOLEAN
};

enum carbon_field_class {
    CARBON_FIELD_CLASS_CONSTANT,
    CARBON_FIELD_CLASS_NUMBER,
    CARBON_FIELD_CLASS_CHARACTER_STRING,
    CARBON_FIELD_CLASS_BINARY_STRING,
    CARBON_FIELD_CLASS_CONTAINER
};

enum carbon_constant {
    CARBON_CONSTANT_TRUE,
    CARBON_CONSTANT_FALSE,
    CARBON_CONSTANT_NULL
};

#define JAK_CARBON_FIELD_TYPE_NULL_STR "null"
#define JAK_CARBON_FIELD_TYPE_TRUE_STR "boolean-true"
#define JAK_CARBON_FIELD_TYPE_FALSE_STR "boolean-false"
#define JAK_CARBON_FIELD_TYPE_OBJECT_STR "object"
#define JAK_CARBON_FIELD_TYPE_ARRAY_STR "array"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U8_STR "column-u8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U16_STR "column-u16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U32_STR "column-u32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_U64_STR "column-u64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I8_STR "column-i8"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I16_STR "column-i16"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I32_STR "column-i32"
#define JAK_CARBON_FIELD_TYPE_COLUMN_I64_STR "column-i64"
#define JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT_STR "column-float"
#define JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN_STR "column-boolean"
#define JAK_CARBON_FIELD_TYPE_STRING_STR "string"
#define JAK_CARBON_FIELD_TYPE_BINARY_STR "binary"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U8_STR "number-u8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U16_STR "number-u16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U32_STR "number-u32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_U64_STR "number-u64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I8_STR "number-i8"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I16_STR "number-i16"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I32_STR "number-i32"
#define JAK_CARBON_FIELD_TYPE_NUMBER_I64_STR "number-i64"
#define JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT_STR "number-float"

JAK_BEGIN_DECL

const char *carbon_field_type_str(struct err *err, enum carbon_field_type type);

bool carbon_field_type_is_traversable(enum carbon_field_type type);

bool carbon_field_type_is_signed_integer(enum carbon_field_type type);

bool carbon_field_type_is_unsigned_integer(enum carbon_field_type type);

bool carbon_field_type_is_floating_number(enum carbon_field_type type);

bool carbon_field_type_is_number(enum carbon_field_type type);

bool carbon_field_type_is_integer(enum carbon_field_type type);

bool carbon_field_type_is_binary(enum carbon_field_type type);

bool carbon_field_type_is_boolean(enum carbon_field_type type);

bool carbon_field_type_is_array(enum carbon_field_type type);

bool carbon_field_type_is_column(enum carbon_field_type type);

bool carbon_field_type_is_object(enum carbon_field_type type);

bool carbon_field_type_is_null(enum carbon_field_type type);

bool carbon_field_type_is_string(enum carbon_field_type type);

enum carbon_field_class carbon_field_type_get_class(enum carbon_field_type type, struct err *err);

bool carbon_field_type_is_constant(enum carbon_field_type type);

bool carbon_field_skip(struct memfile *file);

bool carbon_field_skip_object(struct memfile *file);

bool carbon_field_skip_array(struct memfile *file);

bool carbon_field_skip_column(struct memfile *file);

bool carbon_field_skip_binary(struct memfile *file);

bool carbon_field_skip_custom_binary(struct memfile *file);

bool carbon_field_skip_string(struct memfile *file);

bool carbon_field_skip_float(struct memfile *file);

bool carbon_field_skip_boolean(struct memfile *file);

bool carbon_field_skip_null(struct memfile *file);

bool carbon_field_skip_8(struct memfile *file);

bool carbon_field_skip_16(struct memfile *file);

bool carbon_field_skip_32(struct memfile *file);

bool carbon_field_skip_64(struct memfile *file);

enum carbon_field_type carbon_field_type_for_column(enum carbon_column_type type);

enum carbon_field_type carbon_field_type_column_entry_to_regular_type(enum carbon_field_type type, bool is_null, bool is_true);

JAK_END_DECL

#endif
