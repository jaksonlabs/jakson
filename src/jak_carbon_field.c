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

#include <jak_memfile.h>
#include <jak_carbon_field.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_media.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_object_it.h>

const char *jak_carbon_field_type_str(struct jak_error *err, jak_carbon_field_type_e type)
{
        switch (type) {
                case JAK_CARBON_FIELD_TYPE_NULL:
                        return JAK_CARBON_FIELD_TYPE_NULL_STR;
                case JAK_CARBON_FIELD_TYPE_TRUE:
                        return JAK_CARBON_FIELD_TYPE_TRUE_STR;
                case JAK_CARBON_FIELD_TYPE_FALSE:
                        return JAK_CARBON_FIELD_TYPE_FALSE_STR;
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                        return JAK_CARBON_FIELD_TYPE_OBJECT_STR;
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                        return JAK_CARBON_FIELD_TYPE_ARRAY_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U8_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U16_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U32_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U64_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I8_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I16_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I32_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I64_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT_STR;
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN_STR;
                case JAK_CARBON_FIELD_TYPE_STRING:
                        return JAK_CARBON_FIELD_TYPE_STRING_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U8:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_U8_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U16:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_U16_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U32:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_U32_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U64:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_U64_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I8:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_I8_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I16:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_I16_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I32:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_I32_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_I64:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_I64_STR;
                case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        return JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT_STR;
                case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM:
                case JAK_CARBON_FIELD_TYPE_BINARY:
                        return JAK_CARBON_FIELD_TYPE_BINARY_STR;
                default:
                        if (err) {
                                error(err, JAK_ERR_NOTFOUND);
                        }
                        return NULL;
        }
}

bool jak_carbon_field_type_is_traversable(jak_carbon_field_type_e type)
{
        switch (type) {
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        return true;
                default:
                        return false;
        }
}

bool jak_carbon_field_type_is_signed(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_NUMBER_I8 || type == JAK_CARBON_FIELD_TYPE_NUMBER_I16 ||
                type == JAK_CARBON_FIELD_TYPE_NUMBER_I32 || type == JAK_CARBON_FIELD_TYPE_NUMBER_I64 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 || type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 || type == JAK_CARBON_FIELD_TYPE_COLUMN_I64);
}

bool jak_carbon_field_type_is_unsigned(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_NUMBER_U8 || type == JAK_CARBON_FIELD_TYPE_NUMBER_U16 ||
                type == JAK_CARBON_FIELD_TYPE_NUMBER_U32 || type == JAK_CARBON_FIELD_TYPE_NUMBER_U64 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 || type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 || type == JAK_CARBON_FIELD_TYPE_COLUMN_U64);
}

bool jak_carbon_field_type_is_floating(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT || type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT);
}

bool jak_carbon_field_type_is_number(jak_carbon_field_type_e type)
{
        return jak_carbon_field_type_is_integer(type) || jak_carbon_field_type_is_floating(type);
}

bool jak_carbon_field_type_is_integer(jak_carbon_field_type_e type)
{
        return jak_carbon_field_type_is_signed(type) || jak_carbon_field_type_is_unsigned(type);
}

bool jak_carbon_field_type_is_binary(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_BINARY || type == JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM);
}

bool jak_carbon_field_type_is_boolean(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_TRUE || type == JAK_CARBON_FIELD_TYPE_FALSE ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
}

bool jak_carbon_field_type_is_string(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_STRING);
}

bool jak_carbon_field_type_is_constant(jak_carbon_field_type_e type)
{
        return (jak_carbon_field_type_is_null(type) || jak_carbon_field_type_is_boolean(type));
}

bool jak_carbon_field_skip(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        jak_u8 type_marker = *JAK_MEMFILE_PEEK(file, jak_u8);

        switch (type_marker) {
                case JAK_CARBON_FIELD_TYPE_NULL:
                        jak_carbon_field_skip_null(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_TRUE:
                case JAK_CARBON_FIELD_TYPE_FALSE:
                        jak_carbon_field_skip_boolean(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I8:
                        jak_carbon_field_skip_8(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I16:
                        jak_carbon_field_skip_16(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I32:
                        jak_carbon_field_skip_32(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U64:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I64:
                        jak_carbon_field_skip_64(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        jak_carbon_field_skip_float(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_STRING:
                        jak_carbon_field_skip_string(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_BINARY:
                        jak_carbon_field_skip_binary(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        jak_carbon_field_skip_custom_binary(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                        jak_carbon_field_skip_array(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        jak_carbon_field_skip_column(file);
                        break;
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                        jak_carbon_field_skip_object(file);
                        break;
                default: error(&file->err, JAK_ERR_CORRUPTED);
                        return false;
        }
        return true;
}

bool jak_carbon_field_skip_object(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_OBJECT, &file->err, JAK_ERR_TYPEMISMATCH);
        jak_carbon_object_it skip_it;
        jak_carbon_object_it_create(&skip_it, file, &file->err, memfile_tell(file) - sizeof(jak_u8));
        jak_carbon_object_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        jak_carbon_object_it_drop(&skip_it);
        return true;
}

bool jak_carbon_field_skip_array(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_ARRAY, &file->err, JAK_ERR_TYPEMISMATCH);
        jak_carbon_array_it skip_it;
        jak_carbon_array_it_create(&skip_it, file, &file->err, memfile_tell(file) - sizeof(jak_u8));
        jak_carbon_array_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        jak_carbon_array_it_drop(&skip_it);
        return true;
}

bool jak_carbon_field_skip_column(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_U8 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_U16 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_U32 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_U64 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_I8 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_I16 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_I32 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_I64 &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN &&
                 type_marker != JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT, &file->err, JAK_ERR_TYPEMISMATCH);

        jak_carbon_column_it skip_it;
        jak_carbon_column_it_create(&skip_it, file, &file->err,
                                memfile_tell(file) - sizeof(jak_u8));
        jak_carbon_column_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        return true;
}

bool jak_carbon_field_skip_binary(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_BINARY, &file->err, JAK_ERR_TYPEMISMATCH);
        /* read and skip mime type with variable-length integer type */
        jak_u64 mime_type = memfile_read_uintvar_stream(NULL, file);
        JAK_UNUSED(mime_type);

        /* read blob length */
        jak_u64 blob_len = memfile_read_uintvar_stream(NULL, file);

        /* skip blob */
        memfile_skip(file, blob_len);
        return true;
}

bool jak_carbon_field_skip_custom_binary(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM, &file->err, JAK_ERR_TYPEMISMATCH);
        /* read custom type string length, and skip the type string */
        jak_u64 custom_type_str_len = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, custom_type_str_len);

        /* read blob length, and skip blob data */
        jak_u64 blob_len = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, blob_len);
        return true;
}

bool jak_carbon_field_skip_string(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_STRING, &file->err, JAK_ERR_TYPEMISMATCH);
        jak_u64 strlen = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, strlen);
        return true;
}

bool jak_carbon_field_skip_float(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT, &file->err, JAK_ERR_TYPEMISMATCH);
        memfile_skip(file, sizeof(float));
        return true;
}

bool jak_carbon_field_skip_boolean(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_TRUE && type_marker != JAK_CARBON_FIELD_TYPE_FALSE, &file->err,
                 JAK_ERR_TYPEMISMATCH);
        return true;
}

bool jak_carbon_field_skip_null(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NULL, &file->err, JAK_ERR_TYPEMISMATCH);
        return true;
}

bool jak_carbon_field_skip_8(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_I8 && type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_U8,
                 &file->err, JAK_ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(jak_u8) == sizeof(jak_i8));
        memfile_skip(file, sizeof(jak_u8));
        return true;
}

bool jak_carbon_field_skip_16(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_I16 && type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_U16,
                 &file->err, JAK_ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(jak_u16) == sizeof(jak_i16));
        memfile_skip(file, sizeof(jak_u16));
        return true;
}

bool jak_carbon_field_skip_32(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_I32 && type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_U32,
                 &file->err, JAK_ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(jak_u32) == sizeof(jak_i32));
        memfile_skip(file, sizeof(jak_u32));
        return true;
}

bool jak_carbon_field_skip_64(struct jak_memfile *file)
{
        jak_u8 type_marker = *JAK_MEMFILE_READ_TYPE(file, jak_u8);

        error_if(type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_I64 && type_marker != JAK_CARBON_FIELD_TYPE_NUMBER_U64,
                 &file->err, JAK_ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(jak_u64) == sizeof(jak_i64));
        memfile_skip(file, sizeof(jak_u64));
        return true;
}

jak_carbon_field_type_e jak_carbon_field_type_for_column(jak_carbon_column_type_e type)
{
        switch (type) {
                case JAK_CARBON_COLUMN_TYPE_U8:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U8;
                case JAK_CARBON_COLUMN_TYPE_U16:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U16;
                case JAK_CARBON_COLUMN_TYPE_U32:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U32;
                case JAK_CARBON_COLUMN_TYPE_U64:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_U64;
                case JAK_CARBON_COLUMN_TYPE_I8:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I8;
                case JAK_CARBON_COLUMN_TYPE_I16:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I16;
                case JAK_CARBON_COLUMN_TYPE_I32:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I32;
                case JAK_CARBON_COLUMN_TYPE_I64:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_I64;
                case JAK_CARBON_COLUMN_TYPE_FLOAT:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT;
                case JAK_CARBON_COLUMN_TYPE_BOOLEAN:
                        return JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN;
                default: error_print(JAK_ERR_INTERNALERR)
                        return 0;
        }
}

jak_carbon_field_type_e
jak_carbon_field_type_column_entry_to_regular_type(jak_carbon_field_type_e type, bool is_null, bool is_true)
{
        if (is_null) {
                return JAK_CARBON_FIELD_TYPE_NULL;
        } else {
                switch (type) {
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_U8;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_U16;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_U32;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_U64;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_I8;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_I16;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_I32;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_I64;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                                return JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT;
                        case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                                return is_true ? JAK_CARBON_FIELD_TYPE_TRUE : JAK_CARBON_FIELD_TYPE_FALSE;
                        default: error_print(JAK_ERR_INTERNALERR)
                                return 0;
                }
        }
}

jak_carbon_field_class_e jak_carbon_field_type_get_class(jak_carbon_field_type_e type, struct jak_error *err)
{
        switch (type) {
                case JAK_CARBON_FIELD_TYPE_NULL:
                case JAK_CARBON_FIELD_TYPE_TRUE:
                case JAK_CARBON_FIELD_TYPE_FALSE:
                        return JAK_CARBON_FIELD_CLASS_CONSTANT;
                case JAK_CARBON_FIELD_TYPE_OBJECT:
                case JAK_CARBON_FIELD_TYPE_ARRAY:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_U64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I8:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I16:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I32:
                case JAK_CARBON_FIELD_TYPE_COLUMN_I64:
                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        return JAK_CARBON_FIELD_CLASS_CONTAINER;
                case JAK_CARBON_FIELD_TYPE_STRING:
                        return JAK_CARBON_FIELD_CLASS_CHARACTER_STRING;
                case JAK_CARBON_FIELD_TYPE_NUMBER_U8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_U64:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I8:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I16:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I32:
                case JAK_CARBON_FIELD_TYPE_NUMBER_I64:
                case JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        return JAK_CARBON_FIELD_CLASS_NUMBER;
                case JAK_CARBON_FIELD_TYPE_BINARY:
                case JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        return JAK_CARBON_FIELD_CLASS_BINARY_STRING;
                default: error(err, JAK_ERR_INTERNALERR);
                        return 0;
        }
}

bool jak_carbon_field_type_is_array(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_ARRAY);
}

bool jak_carbon_field_type_is_column(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
}

bool jak_carbon_field_type_is_object(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_OBJECT);
}

bool jak_carbon_field_type_is_null(jak_carbon_field_type_e type)
{
        return (type == JAK_CARBON_FIELD_TYPE_NULL);
}