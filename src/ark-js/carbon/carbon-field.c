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

#include <ark-js/shared/mem/file.h>
#include <ark-js/carbon/carbon-field.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-media.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-object-it.h>

NG5_EXPORT(const char *) carbon_field_type_str(struct err *err, enum carbon_field_type type)
{
        switch (type) {
        case carbon_FIELD_TYPE_NULL: return carbon_FIELD_TYPE_NULL_STR;
        case carbon_FIELD_TYPE_TRUE: return carbon_FIELD_TYPE_TRUE_STR;
        case carbon_FIELD_TYPE_FALSE: return carbon_FIELD_TYPE_FALSE_STR;
        case carbon_FIELD_TYPE_OBJECT: return carbon_FIELD_TYPE_OBJECT_STR;
        case carbon_FIELD_TYPE_ARRAY: return carbon_FIELD_TYPE_ARRAY_STR;
        case carbon_FIELD_TYPE_COLUMN_U8: return carbon_FIELD_TYPE_COLUMN_U8_STR;
        case carbon_FIELD_TYPE_COLUMN_U16: return carbon_FIELD_TYPE_COLUMN_U16_STR;
        case carbon_FIELD_TYPE_COLUMN_U32: return carbon_FIELD_TYPE_COLUMN_U32_STR;
        case carbon_FIELD_TYPE_COLUMN_U64: return carbon_FIELD_TYPE_COLUMN_U64_STR;
        case carbon_FIELD_TYPE_COLUMN_I8: return carbon_FIELD_TYPE_COLUMN_I8_STR;
        case carbon_FIELD_TYPE_COLUMN_I16: return carbon_FIELD_TYPE_COLUMN_I16_STR;
        case carbon_FIELD_TYPE_COLUMN_I32: return carbon_FIELD_TYPE_COLUMN_I32_STR;
        case carbon_FIELD_TYPE_COLUMN_I64: return carbon_FIELD_TYPE_COLUMN_I64_STR;
        case carbon_FIELD_TYPE_COLUMN_FLOAT: return carbon_FIELD_TYPE_COLUMN_FLOAT_STR;
        case carbon_FIELD_TYPE_COLUMN_BOOLEAN: return carbon_FIELD_TYPE_COLUMN_BOOLEAN_STR;
        case carbon_FIELD_TYPE_STRING: return carbon_FIELD_TYPE_STRING_STR;
        case carbon_FIELD_TYPE_NUMBER_U8: return carbon_FIELD_TYPE_NUMBER_U8_STR;
        case carbon_FIELD_TYPE_NUMBER_U16: return carbon_FIELD_TYPE_NUMBER_U16_STR;
        case carbon_FIELD_TYPE_NUMBER_U32: return carbon_FIELD_TYPE_NUMBER_U32_STR;
        case carbon_FIELD_TYPE_NUMBER_U64: return carbon_FIELD_TYPE_NUMBER_U64_STR;
        case carbon_FIELD_TYPE_NUMBER_I8: return carbon_FIELD_TYPE_NUMBER_I8_STR;
        case carbon_FIELD_TYPE_NUMBER_I16: return carbon_FIELD_TYPE_NUMBER_I16_STR;
        case carbon_FIELD_TYPE_NUMBER_I32: return carbon_FIELD_TYPE_NUMBER_I32_STR;
        case carbon_FIELD_TYPE_NUMBER_I64: return carbon_FIELD_TYPE_NUMBER_I64_STR;
        case carbon_FIELD_TYPE_NUMBER_FLOAT: return carbon_FIELD_TYPE_NUMBER_FLOAT_STR;
        case carbon_FIELD_TYPE_BINARY_CUSTOM:
        case carbon_FIELD_TYPE_BINARY:
                return carbon_FIELD_TYPE_BINARY_STR;
        default:
                error(err, NG5_ERR_NOTFOUND);
                return NULL;
        }
}

NG5_EXPORT(bool) carbon_field_type_is_traversable(enum carbon_field_type type)
{
        switch (type) {
        case carbon_FIELD_TYPE_OBJECT:
        case carbon_FIELD_TYPE_ARRAY:
        case carbon_FIELD_TYPE_COLUMN_U8:
        case carbon_FIELD_TYPE_COLUMN_U16:
        case carbon_FIELD_TYPE_COLUMN_U32:
        case carbon_FIELD_TYPE_COLUMN_U64:
        case carbon_FIELD_TYPE_COLUMN_I8:
        case carbon_FIELD_TYPE_COLUMN_I16:
        case carbon_FIELD_TYPE_COLUMN_I32:
        case carbon_FIELD_TYPE_COLUMN_I64:
        case carbon_FIELD_TYPE_COLUMN_FLOAT:
        case carbon_FIELD_TYPE_COLUMN_BOOLEAN:
                return true;
        default:
                return false;
        }
}

NG5_EXPORT(bool) carbon_field_type_is_signed_integer(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_NUMBER_I8 || type == carbon_FIELD_TYPE_NUMBER_I16 ||
                type == carbon_FIELD_TYPE_NUMBER_I32 || type == carbon_FIELD_TYPE_NUMBER_I64 ||
                type == carbon_FIELD_TYPE_COLUMN_I8 || type == carbon_FIELD_TYPE_COLUMN_I16 ||
                type == carbon_FIELD_TYPE_COLUMN_I32 || type == carbon_FIELD_TYPE_COLUMN_I64);
}

NG5_EXPORT(bool) carbon_field_type_is_unsigned_integer(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_NUMBER_U8 || type == carbon_FIELD_TYPE_NUMBER_U16 ||
                type == carbon_FIELD_TYPE_NUMBER_U32 || type == carbon_FIELD_TYPE_NUMBER_U64  ||
                type == carbon_FIELD_TYPE_COLUMN_U8 || type == carbon_FIELD_TYPE_COLUMN_U16 ||
                type == carbon_FIELD_TYPE_COLUMN_U32 || type == carbon_FIELD_TYPE_COLUMN_U64);
}

NG5_EXPORT(bool) carbon_field_type_is_floating_number(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_NUMBER_FLOAT || type == carbon_FIELD_TYPE_COLUMN_FLOAT);
}

NG5_EXPORT(bool) carbon_field_type_is_number(enum carbon_field_type type)
{
        return carbon_field_type_is_integer(type) || carbon_field_type_is_floating_number(type);
}

NG5_EXPORT(bool) carbon_field_type_is_integer(enum carbon_field_type type)
{
        return carbon_field_type_is_signed_integer(type) || carbon_field_type_is_unsigned_integer(type);
}

NG5_EXPORT(bool) carbon_field_type_is_binary(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_BINARY || type == carbon_FIELD_TYPE_BINARY_CUSTOM);
}

NG5_EXPORT(bool) carbon_field_type_is_boolean(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_TRUE || type == carbon_FIELD_TYPE_FALSE || type == carbon_FIELD_TYPE_COLUMN_BOOLEAN);
}

NG5_EXPORT(bool) carbon_field_type_is_string(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_STRING);
}

NG5_EXPORT(bool) carbon_field_type_is_constant(enum carbon_field_type type)
{
        return (carbon_field_type_is_null(type) || carbon_field_type_is_boolean(type));
}

NG5_EXPORT(bool) carbon_field_skip(struct memfile *file)
{
        error_if_null(file)
        u8 type_marker = *NG5_MEMFILE_PEEK(file, u8);

        switch (type_marker) {
        case carbon_FIELD_TYPE_NULL:
                carbon_field_skip_null(file);
                break;
        case carbon_FIELD_TYPE_TRUE:
        case carbon_FIELD_TYPE_FALSE:
                carbon_field_skip_boolean(file);
                break;
        case carbon_FIELD_TYPE_NUMBER_U8:
        case carbon_FIELD_TYPE_NUMBER_I8:
                carbon_field_skip_8(file);
                break;
        case carbon_FIELD_TYPE_NUMBER_U16:
        case carbon_FIELD_TYPE_NUMBER_I16:
                carbon_field_skip_16(file);
                break;
        case carbon_FIELD_TYPE_NUMBER_U32:
        case carbon_FIELD_TYPE_NUMBER_I32:
                carbon_field_skip_32(file);
                break;
        case carbon_FIELD_TYPE_NUMBER_U64:
        case carbon_FIELD_TYPE_NUMBER_I64:
                carbon_field_skip_64(file);
                break;
        case carbon_FIELD_TYPE_NUMBER_FLOAT:
                carbon_field_skip_float(file);
                break;
        case carbon_FIELD_TYPE_STRING:
                carbon_field_skip_string(file);
                break;
        case carbon_FIELD_TYPE_BINARY:
                carbon_field_skip_binary(file);
                break;
        case carbon_FIELD_TYPE_BINARY_CUSTOM:
                carbon_field_skip_custom_binary(file);
                break;
        case carbon_FIELD_TYPE_ARRAY:
                carbon_field_skip_array(file);
                break;
        case carbon_FIELD_TYPE_COLUMN_U8:
        case carbon_FIELD_TYPE_COLUMN_U16:
        case carbon_FIELD_TYPE_COLUMN_U32:
        case carbon_FIELD_TYPE_COLUMN_U64:
        case carbon_FIELD_TYPE_COLUMN_I8:
        case carbon_FIELD_TYPE_COLUMN_I16:
        case carbon_FIELD_TYPE_COLUMN_I32:
        case carbon_FIELD_TYPE_COLUMN_I64:
        case carbon_FIELD_TYPE_COLUMN_FLOAT:
        case carbon_FIELD_TYPE_COLUMN_BOOLEAN:
                carbon_field_skip_column(file);
                break;
        case carbon_FIELD_TYPE_OBJECT:
                carbon_field_skip_object(file);
                break;
        default:
        error(&file->err, NG5_ERR_CORRUPTED);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_object(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_OBJECT, &file->err, NG5_ERR_TYPEMISMATCH);
        struct carbon_object_it skip_it;
        carbon_object_it_create(&skip_it, file, &file->err, memfile_tell(file) - sizeof(u8));
        carbon_object_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        carbon_object_it_drop(&skip_it);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_array(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_ARRAY, &file->err, NG5_ERR_TYPEMISMATCH);
        struct carbon_array_it skip_it;
        carbon_array_it_create(&skip_it, file, &file->err, memfile_tell(file) - sizeof(u8));
        carbon_array_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        carbon_array_it_drop(&skip_it);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_column(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_COLUMN_U8 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_U16 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_U32 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_U64 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_I8 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_I16 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_I32 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_I64 &&
                type_marker != carbon_FIELD_TYPE_COLUMN_BOOLEAN &&
                type_marker != carbon_FIELD_TYPE_COLUMN_FLOAT, &file->err, NG5_ERR_TYPEMISMATCH);

        struct carbon_column_it skip_it;
        carbon_column_it_create(&skip_it, file, &file->err,
                memfile_tell(file) - sizeof(u8));
        carbon_column_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_binary(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_BINARY, &file->err, NG5_ERR_TYPEMISMATCH);
        /* read and skip mime type with variable-length integer type */
        u64 mime_type = memfile_read_varuint(NULL, file);
        unused(mime_type);

        /* read blob length */
        u64 blob_len = memfile_read_varuint(NULL, file);

        /* skip blob */
        memfile_skip(file, blob_len);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_custom_binary(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_BINARY_CUSTOM, &file->err, NG5_ERR_TYPEMISMATCH);
        /* read custom type string length, and skip the type string */
        u64 custom_type_str_len = memfile_read_varuint(NULL, file);
        memfile_skip(file, custom_type_str_len);

        /* read blob length, and skip blob data */
        u64 blob_len = memfile_read_varuint(NULL, file);
        memfile_skip(file, blob_len);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_string(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_STRING, &file->err, NG5_ERR_TYPEMISMATCH);
        u64 strlen = memfile_read_varuint(NULL, file);
        memfile_skip(file, strlen);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_float(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NUMBER_FLOAT, &file->err, NG5_ERR_TYPEMISMATCH);
        memfile_skip(file, sizeof(float));
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_boolean(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_TRUE && type_marker != carbon_FIELD_TYPE_FALSE, &file->err, NG5_ERR_TYPEMISMATCH);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_null(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NULL, &file->err, NG5_ERR_TYPEMISMATCH);
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_8(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NUMBER_I8 && type_marker != carbon_FIELD_TYPE_NUMBER_U8,
                &file->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u8) == sizeof(i8));
        memfile_skip(file, sizeof(u8));
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_16(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NUMBER_I16 && type_marker != carbon_FIELD_TYPE_NUMBER_U16,
                &file->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u16) == sizeof(i16));
        memfile_skip(file, sizeof(u16));
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_32(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NUMBER_I32 && type_marker != carbon_FIELD_TYPE_NUMBER_U32,
                &file->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u32) == sizeof(i32));
        memfile_skip(file, sizeof(u32));
        return true;
}

NG5_EXPORT(bool) carbon_field_skip_64(struct memfile *file)
{
        u8 type_marker = *NG5_MEMFILE_READ_TYPE(file, u8);

        error_if(type_marker != carbon_FIELD_TYPE_NUMBER_I64 && type_marker != carbon_FIELD_TYPE_NUMBER_U64,
                &file->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u64) == sizeof(i64));
        memfile_skip(file, sizeof(u64));
        return true;
}

NG5_EXPORT(enum carbon_field_type) carbon_field_type_for_column(enum carbon_column_type type)
{
        switch (type) {
        case carbon_COLUMN_TYPE_U8: return carbon_FIELD_TYPE_COLUMN_U8;
        case carbon_COLUMN_TYPE_U16: return carbon_FIELD_TYPE_COLUMN_U16;
        case carbon_COLUMN_TYPE_U32: return carbon_FIELD_TYPE_COLUMN_U32;
        case carbon_COLUMN_TYPE_U64: return carbon_FIELD_TYPE_COLUMN_U64;
        case carbon_COLUMN_TYPE_I8: return carbon_FIELD_TYPE_COLUMN_I8;
        case carbon_COLUMN_TYPE_I16: return carbon_FIELD_TYPE_COLUMN_I16;
        case carbon_COLUMN_TYPE_I32: return carbon_FIELD_TYPE_COLUMN_I32;
        case carbon_COLUMN_TYPE_I64: return carbon_FIELD_TYPE_COLUMN_I64;
        case carbon_COLUMN_TYPE_FLOAT: return carbon_FIELD_TYPE_COLUMN_FLOAT;
        case carbon_COLUMN_TYPE_BOOLEAN: return carbon_FIELD_TYPE_COLUMN_BOOLEAN;
        default:
                error_print(NG5_ERR_INTERNALERR)
                return 0;
        }
}

NG5_EXPORT(enum carbon_field_class) carbon_field_type_get_class(enum carbon_field_type type, struct err *err)
{
        switch (type) {
        case carbon_FIELD_TYPE_NULL:
        case carbon_FIELD_TYPE_TRUE:
        case carbon_FIELD_TYPE_FALSE:
                return carbon_FIELD_CLASS_CONSTANT;
        case carbon_FIELD_TYPE_OBJECT:
        case carbon_FIELD_TYPE_ARRAY:
        case carbon_FIELD_TYPE_COLUMN_U8:
        case carbon_FIELD_TYPE_COLUMN_U16:
        case carbon_FIELD_TYPE_COLUMN_U32:
        case carbon_FIELD_TYPE_COLUMN_U64:
        case carbon_FIELD_TYPE_COLUMN_I8:
        case carbon_FIELD_TYPE_COLUMN_I16:
        case carbon_FIELD_TYPE_COLUMN_I32:
        case carbon_FIELD_TYPE_COLUMN_I64:
        case carbon_FIELD_TYPE_COLUMN_FLOAT:
        case carbon_FIELD_TYPE_COLUMN_BOOLEAN:
                return carbon_FIELD_CLASS_CONTAINER;
        case carbon_FIELD_TYPE_STRING:
                return carbon_FIELD_CLASS_CHARACTER_STRING;
        case carbon_FIELD_TYPE_NUMBER_U8:
        case carbon_FIELD_TYPE_NUMBER_U16:
        case carbon_FIELD_TYPE_NUMBER_U32:
        case carbon_FIELD_TYPE_NUMBER_U64:
        case carbon_FIELD_TYPE_NUMBER_I8:
        case carbon_FIELD_TYPE_NUMBER_I16:
        case carbon_FIELD_TYPE_NUMBER_I32:
        case carbon_FIELD_TYPE_NUMBER_I64:
        case carbon_FIELD_TYPE_NUMBER_FLOAT:
                return carbon_FIELD_CLASS_NUMBER;
        case carbon_FIELD_TYPE_BINARY:
        case carbon_FIELD_TYPE_BINARY_CUSTOM:
                return carbon_FIELD_CLASS_BINARY_STRING;
        default:
                error(err, NG5_ERR_INTERNALERR);
                return 0;
        }
}

NG5_EXPORT(bool) carbon_field_type_is_array(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_ARRAY);
}

NG5_EXPORT(bool) carbon_field_type_is_column(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_COLUMN_U8 ||
                type == carbon_FIELD_TYPE_COLUMN_U16 ||
                type == carbon_FIELD_TYPE_COLUMN_U32 ||
                type == carbon_FIELD_TYPE_COLUMN_U64 ||
                type == carbon_FIELD_TYPE_COLUMN_I8 ||
                type == carbon_FIELD_TYPE_COLUMN_I16 ||
                type == carbon_FIELD_TYPE_COLUMN_I32 ||
                type == carbon_FIELD_TYPE_COLUMN_I64 ||
                type == carbon_FIELD_TYPE_COLUMN_FLOAT ||
                type == carbon_FIELD_TYPE_COLUMN_BOOLEAN);
}

NG5_EXPORT(bool) carbon_field_type_is_object(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_OBJECT);
}

NG5_EXPORT(bool) carbon_field_type_is_null(enum carbon_field_type type)
{
        return (type == carbon_FIELD_TYPE_NULL);
}