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

#include "shared/common.h"
#include "shared/error.h"

#ifndef BISON_FIELD_H
#define BISON_FIELD_H

enum bison_field_type
{
        /* constants */
                BISON_FIELD_TYPE_NULL = 'n', /* null */
        BISON_FIELD_TYPE_TRUE = 't', /* true */
        BISON_FIELD_TYPE_FALSE = 'f', /* false */

        /* containers */
                BISON_FIELD_TYPE_OBJECT = 'o', /* object */
        BISON_FIELD_TYPE_ARRAY = '[', /* variable-type array of elements of varying type */
        BISON_FIELD_TYPE_COLUMN = '(', /* fixed-type array of elements of particular type */

        /* character strings */
                BISON_FIELD_TYPE_STRING = 's', /* UTF-8 string */

        /* numbers */
                BISON_FIELD_TYPE_NUMBER_U8 = 'c', /* 8bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U16 = 'd', /* 16bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U32 = 'i', /* 32bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_U64 = 'l', /* 64bit unsigned integer */
        BISON_FIELD_TYPE_NUMBER_I8 = 'C', /* 8bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I16 = 'D', /* 16bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I32 = 'I', /* 32bit signed integer */
        BISON_FIELD_TYPE_NUMBER_I64 = 'L', /* 64bit signed integer */
        BISON_FIELD_TYPE_NUMBER_FLOAT = 'r', /* 32bit float */

        /* binary data */
                BISON_FIELD_TYPE_BINARY = 'b', /* arbitrary binary object with known mime type */
        BISON_FIELD_TYPE_BINARY_CUSTOM = 'x', /* arbitrary binary object with unknown mime type*/
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
#define BISON_FIELD_TYPE_COLUMN_STR "column"
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

NG5_END_DECL

#endif
