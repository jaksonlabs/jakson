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

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/carbon/bison/bison.h>

#ifndef BISON_FIELD_H
#define BISON_FIELD_H

enum bison_field_type
{
        /* constants */
        BISON_FIELD_TYPE_NULL = BISON_MARKER_NULL, /* null */
        BISON_FIELD_TYPE_TRUE = BISON_MARKER_TRUE, /* true */
        BISON_FIELD_TYPE_FALSE = BISON_MARKER_FALSE, /* false */

        /* containers */
        BISON_FIELD_TYPE_OBJECT = BISON_MARKER_OBJECT_BEGIN, /* object */
        BISON_FIELD_TYPE_ARRAY = BISON_MARKER_ARRAY_BEGIN, /* variable-type array of elements of varying type */
        BISON_FIELD_TYPE_COLUMN_U8 = BISON_MARKER_COLUMN_U8, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_U16 = BISON_MARKER_COLUMN_U16, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_U32 = BISON_MARKER_COLUMN_U32, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_U64 = BISON_MARKER_COLUMN_U64, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_I8 = BISON_MARKER_COLUMN_I8, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_I16 = BISON_MARKER_COLUMN_I16, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_I32 = BISON_MARKER_COLUMN_I32, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_I64 = BISON_MARKER_COLUMN_I64, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_FLOAT = BISON_MARKER_COLUMN_FLOAT, /* fixed-type array of elements of particular type */
        BISON_FIELD_TYPE_COLUMN_BOOLEAN = BISON_MARKER_COLUMN_BOOLEAN, /* fixed-type array of elements of particular type */

        /* character strings */
        BISON_FIELD_TYPE_STRING = BISON_MARKER_STRING, /* UTF-8 string */

        /* numbers */
        BISON_FIELD_TYPE_NUMBER_U8 = BISON_MARKER_U8, /* 8bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U16 = BISON_MARKER_U16, /* 16bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U32 = BISON_MARKER_U32, /* 32bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U64 = BISON_MARKER_U64, /* 64bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_I8 = BISON_MARKER_I8, /* 8bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I16 = BISON_MARKER_I16, /* 16bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I32 = BISON_MARKER_I32, /* 32bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I64 = BISON_MARKER_I64, /* 64bit signed integer */
        BISON_FIELD_TYPE_NUMBER_FLOAT = BISON_MARKER_FLOAT, /* 32bit float */

        /* binary data */
        BISON_FIELD_TYPE_BINARY = BISON_MARKER_BINARY, /* arbitrary binary object with known mime type */
        BISON_FIELD_TYPE_BINARY_CUSTOM = BISON_MARKER_CUSTOM_BINARY, /* arbitrary binary object with unknown mime type*/
};

enum bison_column_type
{
        BISON_COLUMN_TYPE_U8,
        BISON_COLUMN_TYPE_U16,
        BISON_COLUMN_TYPE_U32,
        BISON_COLUMN_TYPE_U64,
        BISON_COLUMN_TYPE_I8,
        BISON_COLUMN_TYPE_I16,
        BISON_COLUMN_TYPE_I32,
        BISON_COLUMN_TYPE_I64,
        BISON_COLUMN_TYPE_FLOAT,
        BISON_COLUMN_TYPE_BOOLEAN
};

enum bison_field_class
{
        BISON_FIELD_CLASS_CONSTANT,
        BISON_FIELD_CLASS_NUMBER,
        BISON_FIELD_CLASS_CHARACTER_STRING,
        BISON_FIELD_CLASS_BINARY_STRING,
        BISON_FIELD_CLASS_CONTAINER
};

enum bison_constant
{
        BISON_CONSTANT_TRUE,
        BISON_CONSTANT_FALSE,
        BISON_CONSTANT_NULL
};

#define BISON_FIELD_TYPE_NULL_STR "null"
#define BISON_FIELD_TYPE_TRUE_STR "boolean (true)"
#define BISON_FIELD_TYPE_FALSE_STR "boolean (false)"
#define BISON_FIELD_TYPE_OBJECT_STR "object"
#define BISON_FIELD_TYPE_ARRAY_STR "array"
#define BISON_FIELD_TYPE_COLUMN_U8_STR "column-u8"
#define BISON_FIELD_TYPE_COLUMN_U16_STR "column-u16"
#define BISON_FIELD_TYPE_COLUMN_U32_STR "column-u32"
#define BISON_FIELD_TYPE_COLUMN_U64_STR "column-u64"
#define BISON_FIELD_TYPE_COLUMN_I8_STR "column-i8"
#define BISON_FIELD_TYPE_COLUMN_I16_STR "column-i16"
#define BISON_FIELD_TYPE_COLUMN_I32_STR "column-i32"
#define BISON_FIELD_TYPE_COLUMN_I64_STR "column-i64"
#define BISON_FIELD_TYPE_COLUMN_FLOAT_STR "column-float"
#define BISON_FIELD_TYPE_COLUMN_BOOLEAN_STR "column-boolean"
#define BISON_FIELD_TYPE_STRING_STR "string"
#define BISON_FIELD_TYPE_BINARY_STR "binary"
#define BISON_FIELD_TYPE_NUMBER_U8_STR "number (u8)"
#define BISON_FIELD_TYPE_NUMBER_U16_STR "number (u16)"
#define BISON_FIELD_TYPE_NUMBER_U32_STR "number (u32)"
#define BISON_FIELD_TYPE_NUMBER_U64_STR "number (u64)"
#define BISON_FIELD_TYPE_NUMBER_I8_STR "number (i8)"
#define BISON_FIELD_TYPE_NUMBER_I16_STR "number (i16)"
#define BISON_FIELD_TYPE_NUMBER_I32_STR "number (i32)"
#define BISON_FIELD_TYPE_NUMBER_I64_STR "number (i64)"
#define BISON_FIELD_TYPE_NUMBER_FLOAT_STR "number (float)"

NG5_BEGIN_DECL

NG5_EXPORT(const char *) bison_field_type_str(struct err *err, enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_traversable(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_signed_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_unsigned_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_floating_number(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_number(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_integer(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_binary(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_boolean(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_array(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_column(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_object(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_null(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_type_is_string(enum bison_field_type type);

NG5_EXPORT(enum bison_field_class) bison_field_type_get_class(enum bison_field_type type, struct err *err);

NG5_EXPORT(bool) bison_field_type_is_constant(enum bison_field_type type);

NG5_EXPORT(bool) bison_field_skip(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_object(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_array(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_column(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_binary(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_custom_binary(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_string(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_float(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_boolean(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_null(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_8(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_16(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_32(struct memfile *file);

NG5_EXPORT(bool) bison_field_skip_64(struct memfile *file);

NG5_EXPORT(enum bison_field_type) bison_field_type_for_column(enum bison_column_type type);

NG5_END_DECL

#endif
