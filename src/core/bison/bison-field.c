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

#include "core/bison/bison-field.h"

NG5_EXPORT(const char *) bison_field_type_str(struct err *err, enum bison_field_type type)
{
        switch (type) {
        case BISON_FIELD_TYPE_NULL: return BISON_FIELD_TYPE_NULL_STR;
        case BISON_FIELD_TYPE_TRUE: return BISON_FIELD_TYPE_TRUE_STR;
        case BISON_FIELD_TYPE_FALSE: return BISON_FIELD_TYPE_FALSE_STR;
        case BISON_FIELD_TYPE_OBJECT: return BISON_FIELD_TYPE_OBJECT_STR;
        case BISON_FIELD_TYPE_ARRAY: return BISON_FIELD_TYPE_ARRAY_STR;
        case BISON_FIELD_TYPE_COLUMN: return BISON_FIELD_TYPE_COLUMN_STR;
        case BISON_FIELD_TYPE_STRING: return BISON_FIELD_TYPE_STRING_STR;
        case BISON_FIELD_TYPE_NUMBER_U8: return BISON_FIELD_TYPE_NUMBER_U8_STR;
        case BISON_FIELD_TYPE_NUMBER_U16: return BISON_FIELD_TYPE_NUMBER_U16_STR;
        case BISON_FIELD_TYPE_NUMBER_U32: return BISON_FIELD_TYPE_NUMBER_U32_STR;
        case BISON_FIELD_TYPE_NUMBER_U64: return BISON_FIELD_TYPE_NUMBER_U64_STR;
        case BISON_FIELD_TYPE_NUMBER_I8: return BISON_FIELD_TYPE_NUMBER_I8_STR;
        case BISON_FIELD_TYPE_NUMBER_I16: return BISON_FIELD_TYPE_NUMBER_I16_STR;
        case BISON_FIELD_TYPE_NUMBER_I32: return BISON_FIELD_TYPE_NUMBER_I32_STR;
        case BISON_FIELD_TYPE_NUMBER_I64: return BISON_FIELD_TYPE_NUMBER_I64_STR;
        case BISON_FIELD_TYPE_NUMBER_FLOAT: return BISON_FIELD_TYPE_NUMBER_FLOAT_STR;
        case BISON_FIELD_TYPE_BINARY_CUSTOM:
        case BISON_FIELD_TYPE_BINARY:
                return BISON_FIELD_TYPE_BINARY_STR;
        default:
                error(err, NG5_ERR_NOTFOUND);
                return NULL;
        }
}

NG5_EXPORT(bool) bison_field_type_is_traversable(enum bison_field_type type)
{
        switch (type) {
        case BISON_FIELD_TYPE_OBJECT:
        case BISON_FIELD_TYPE_ARRAY:
        case BISON_FIELD_TYPE_COLUMN:
                return true;
        default:
                return false;
        }
}

NG5_EXPORT(bool) bison_field_type_is_signed_integer(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_NUMBER_I8 || type == BISON_FIELD_TYPE_NUMBER_I16 ||
                type == BISON_FIELD_TYPE_NUMBER_I32 || type == BISON_FIELD_TYPE_NUMBER_I64);
}

NG5_EXPORT(bool) bison_field_type_is_unsigned_integer(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_NUMBER_U8 || type == BISON_FIELD_TYPE_NUMBER_U16 ||
                type == BISON_FIELD_TYPE_NUMBER_U32 || type == BISON_FIELD_TYPE_NUMBER_U64);
}

NG5_EXPORT(bool) bison_field_type_is_floating_number(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_NUMBER_FLOAT);
}

NG5_EXPORT(bool) bison_field_type_is_number(enum bison_field_type type)
{
        return bison_field_type_is_integer(type) || bison_field_type_is_floating_number(type);
}

NG5_EXPORT(bool) bison_field_type_is_integer(enum bison_field_type type)
{
        return bison_field_type_is_signed_integer(type) || bison_field_type_is_unsigned_integer(type);
}

NG5_EXPORT(bool) bison_field_type_is_binary(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_BINARY || type == BISON_FIELD_TYPE_BINARY_CUSTOM);
}

NG5_EXPORT(bool) bison_field_type_is_boolean(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_TRUE || type == BISON_FIELD_TYPE_FALSE);
}

NG5_EXPORT(bool) bison_field_type_is_string(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_STRING);
}

NG5_EXPORT(bool) bison_field_type_is_constant(enum bison_field_type type)
{
        return (bison_field_type_is_null(type) || bison_field_type_is_boolean(type));
}

NG5_EXPORT(enum bison_field_class) bison_field_type_get_class(enum bison_field_type type, struct err *err)
{
        switch (type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
                return BISON_FIELD_CLASS_CONSTANT;
        case BISON_FIELD_TYPE_OBJECT:
        case BISON_FIELD_TYPE_ARRAY:
        case BISON_FIELD_TYPE_COLUMN:
                return BISON_FIELD_CLASS_CONTAINER;
        case BISON_FIELD_TYPE_STRING:
                return BISON_FIELD_CLASS_CHARACTER_STRING;
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I8:
        case BISON_FIELD_TYPE_NUMBER_I16:
        case BISON_FIELD_TYPE_NUMBER_I32:
        case BISON_FIELD_TYPE_NUMBER_I64:
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                return BISON_FIELD_CLASS_NUMBER;
        case BISON_FIELD_TYPE_BINARY:
        case BISON_FIELD_TYPE_BINARY_CUSTOM:
                return BISON_FIELD_CLASS_BINARY_STRING;
        default:
                error(err, NG5_ERR_INTERNALERR);
                return 0;
        }
}

NG5_EXPORT(bool) bison_field_type_is_array(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_ARRAY);
}

NG5_EXPORT(bool) bison_field_type_is_column(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_COLUMN);
}

NG5_EXPORT(bool) bison_field_type_is_object(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_OBJECT);
}

NG5_EXPORT(bool) bison_field_type_is_null(enum bison_field_type type)
{
        return (type == BISON_FIELD_TYPE_NULL);
}