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
#include <ark-js/carbon/carbon.h>

#ifndef carbon_FIELD_H
#define carbon_FIELD_H

enum carbon_field_type
{
        /* constants */
        carbon_FIELD_TYPE_NULL = carbon_MARKER_NULL, /* null */
        carbon_FIELD_TYPE_TRUE = carbon_MARKER_TRUE, /* true */
        carbon_FIELD_TYPE_FALSE = carbon_MARKER_FALSE, /* false */

        /* containers */
        carbon_FIELD_TYPE_OBJECT = carbon_MARKER_OBJECT_BEGIN, /* object */
        carbon_FIELD_TYPE_ARRAY = carbon_MARKER_ARRAY_BEGIN, /* variable-type array of elements of varying type */
        carbon_FIELD_TYPE_COLUMN_U8 = carbon_MARKER_COLUMN_U8, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_U16 = carbon_MARKER_COLUMN_U16, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_U32 = carbon_MARKER_COLUMN_U32, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_U64 = carbon_MARKER_COLUMN_U64, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_I8 = carbon_MARKER_COLUMN_I8, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_I16 = carbon_MARKER_COLUMN_I16, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_I32 = carbon_MARKER_COLUMN_I32, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_I64 = carbon_MARKER_COLUMN_I64, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_FLOAT = carbon_MARKER_COLUMN_FLOAT, /* fixed-type array of elements of particular type */
        carbon_FIELD_TYPE_COLUMN_BOOLEAN = carbon_MARKER_COLUMN_BOOLEAN, /* fixed-type array of elements of particular type */

        /* character strings */
        carbon_FIELD_TYPE_STRING = carbon_MARKER_STRING, /* UTF-8 string */

        /* numbers */
        carbon_FIELD_TYPE_NUMBER_U8 = carbon_MARKER_U8, /* 8bit unsigned integer */
        carbon_FIELD_TYPE_NUMBER_U16 = carbon_MARKER_U16, /* 16bit unsigned integer */
        carbon_FIELD_TYPE_NUMBER_U32 = carbon_MARKER_U32, /* 32bit unsigned integer */
        carbon_FIELD_TYPE_NUMBER_U64 = carbon_MARKER_U64, /* 64bit unsigned integer */
        carbon_FIELD_TYPE_NUMBER_I8 = carbon_MARKER_I8, /* 8bit signed integer */
        carbon_FIELD_TYPE_NUMBER_I16 = carbon_MARKER_I16, /* 16bit signed integer */
        carbon_FIELD_TYPE_NUMBER_I32 = carbon_MARKER_I32, /* 32bit signed integer */
        carbon_FIELD_TYPE_NUMBER_I64 = carbon_MARKER_I64, /* 64bit signed integer */
        carbon_FIELD_TYPE_NUMBER_FLOAT = carbon_MARKER_FLOAT, /* 32bit float */

        /* binary data */
        carbon_FIELD_TYPE_BINARY = carbon_MARKER_BINARY, /* arbitrary binary object with known mime type */
        carbon_FIELD_TYPE_BINARY_CUSTOM = carbon_MARKER_CUSTOM_BINARY, /* arbitrary binary object with unknown mime type*/
};

enum carbon_column_type
{
        carbon_COLUMN_TYPE_U8,
        carbon_COLUMN_TYPE_U16,
        carbon_COLUMN_TYPE_U32,
        carbon_COLUMN_TYPE_U64,
        carbon_COLUMN_TYPE_I8,
        carbon_COLUMN_TYPE_I16,
        carbon_COLUMN_TYPE_I32,
        carbon_COLUMN_TYPE_I64,
        carbon_COLUMN_TYPE_FLOAT,
        carbon_COLUMN_TYPE_BOOLEAN
};

enum carbon_field_class
{
        carbon_FIELD_CLASS_CONSTANT,
        carbon_FIELD_CLASS_NUMBER,
        carbon_FIELD_CLASS_CHARACTER_STRING,
        carbon_FIELD_CLASS_BINARY_STRING,
        carbon_FIELD_CLASS_CONTAINER
};

enum carbon_constant
{
        carbon_CONSTANT_TRUE,
        carbon_CONSTANT_FALSE,
        carbon_CONSTANT_NULL
};

#define carbon_FIELD_TYPE_NULL_STR "null"
#define carbon_FIELD_TYPE_TRUE_STR "boolean (true)"
#define carbon_FIELD_TYPE_FALSE_STR "boolean (false)"
#define carbon_FIELD_TYPE_OBJECT_STR "object"
#define carbon_FIELD_TYPE_ARRAY_STR "array"
#define carbon_FIELD_TYPE_COLUMN_U8_STR "column-u8"
#define carbon_FIELD_TYPE_COLUMN_U16_STR "column-u16"
#define carbon_FIELD_TYPE_COLUMN_U32_STR "column-u32"
#define carbon_FIELD_TYPE_COLUMN_U64_STR "column-u64"
#define carbon_FIELD_TYPE_COLUMN_I8_STR "column-i8"
#define carbon_FIELD_TYPE_COLUMN_I16_STR "column-i16"
#define carbon_FIELD_TYPE_COLUMN_I32_STR "column-i32"
#define carbon_FIELD_TYPE_COLUMN_I64_STR "column-i64"
#define carbon_FIELD_TYPE_COLUMN_FLOAT_STR "column-float"
#define carbon_FIELD_TYPE_COLUMN_BOOLEAN_STR "column-boolean"
#define carbon_FIELD_TYPE_STRING_STR "string"
#define carbon_FIELD_TYPE_BINARY_STR "binary"
#define carbon_FIELD_TYPE_NUMBER_U8_STR "number (u8)"
#define carbon_FIELD_TYPE_NUMBER_U16_STR "number (u16)"
#define carbon_FIELD_TYPE_NUMBER_U32_STR "number (u32)"
#define carbon_FIELD_TYPE_NUMBER_U64_STR "number (u64)"
#define carbon_FIELD_TYPE_NUMBER_I8_STR "number (i8)"
#define carbon_FIELD_TYPE_NUMBER_I16_STR "number (i16)"
#define carbon_FIELD_TYPE_NUMBER_I32_STR "number (i32)"
#define carbon_FIELD_TYPE_NUMBER_I64_STR "number (i64)"
#define carbon_FIELD_TYPE_NUMBER_FLOAT_STR "number (float)"

ARK_BEGIN_DECL

ARK_EXPORT(const char *) carbon_field_type_str(struct err *err, enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_traversable(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_signed_integer(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_unsigned_integer(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_floating_number(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_number(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_integer(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_binary(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_boolean(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_array(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_column(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_object(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_null(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_type_is_string(enum carbon_field_type type);

ARK_EXPORT(enum carbon_field_class) carbon_field_type_get_class(enum carbon_field_type type, struct err *err);

ARK_EXPORT(bool) carbon_field_type_is_constant(enum carbon_field_type type);

ARK_EXPORT(bool) carbon_field_skip(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_object(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_array(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_column(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_binary(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_custom_binary(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_string(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_float(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_boolean(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_null(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_8(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_16(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_32(struct memfile *file);

ARK_EXPORT(bool) carbon_field_skip_64(struct memfile *file);

ARK_EXPORT(enum carbon_field_type) carbon_field_type_for_column(enum carbon_column_type type);

ARK_END_DECL

#endif
