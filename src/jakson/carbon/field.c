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

#include <jakson/mem/file.h>
#include <jakson/carbon/field.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/mime.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/abstract.h>

const char *carbon_field_type_str(err *err, carbon_field_type_e type)
{
        switch (type) {
                case CARBON_FIELD_NULL:
                        return CARBON_FIELD_TYPE_NULL_STR;
                case CARBON_FIELD_TRUE:
                        return CARBON_FIELD_TYPE_TRUE_STR;
                case CARBON_FIELD_FALSE:
                        return CARBON_FIELD_TYPE_FALSE_STR;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                        return CARBON_FIELD_TYPE_OBJECT_UNSORTED_MULTIMAP_STR;
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                        return CARBON_FIELD_TYPE_OBJECT_SORTED_MULTIMAP_STR;
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                        return CARBON_FIELD_TYPE_OBJECT_UNSORTED_MAP_STR;
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        return CARBON_FIELD_TYPE_OBJECT_SORTED_MAP_STR;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_ARRAY_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_ARRAY_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_ARRAY_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        return CARBON_FIELD_TYPE_ARRAY_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U8_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U8_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U8_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U8_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U16_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U16_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U16_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U16_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U32_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U32_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U32_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U32_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U64_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_U64_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U64_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_U64_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I8_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I8_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I8_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I8_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I16_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I16_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I16_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I16_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I32_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I32_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I32_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I32_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I64_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_I64_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I64_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_I64_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_FLOAT_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_FLOAT_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_FLOAT_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_FLOAT_SORTED_SET_STR;
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_BOOLEAN_UNSORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        return CARBON_FIELD_TYPE_COLUMN_BOOLEAN_SORTED_MULTISET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_BOOLEAN_UNSORTED_SET_STR;
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        return CARBON_FIELD_TYPE_COLUMN_BOOLEAN_SORTED_SET_STR;
                case CARBON_FIELD_STRING:
                        return CARBON_FIELD_TYPE_STRING_STR;
                case CARBON_FIELD_NUMBER_U8:
                        return CARBON_FIELD_TYPE_NUMBER_U8_STR;
                case CARBON_FIELD_NUMBER_U16:
                        return CARBON_FIELD_TYPE_NUMBER_U16_STR;
                case CARBON_FIELD_NUMBER_U32:
                        return CARBON_FIELD_TYPE_NUMBER_U32_STR;
                case CARBON_FIELD_NUMBER_U64:
                        return CARBON_FIELD_TYPE_NUMBER_U64_STR;
                case CARBON_FIELD_NUMBER_I8:
                        return CARBON_FIELD_TYPE_NUMBER_I8_STR;
                case CARBON_FIELD_NUMBER_I16:
                        return CARBON_FIELD_TYPE_NUMBER_I16_STR;
                case CARBON_FIELD_NUMBER_I32:
                        return CARBON_FIELD_TYPE_NUMBER_I32_STR;
                case CARBON_FIELD_NUMBER_I64:
                        return CARBON_FIELD_TYPE_NUMBER_I64_STR;
                case CARBON_FIELD_NUMBER_FLOAT:
                        return CARBON_FIELD_TYPE_NUMBER_FLOAT_STR;
                case CARBON_FIELD_BINARY_CUSTOM:
                case CARBON_FIELD_BINARY:
                        return CARBON_FIELD_TYPE_BINARY_STR;
                default:
                        if (err) {
                                ERROR(err, ERR_NOTFOUND);
                        }
                        return NULL;
        }
}

bool carbon_field_type_is_traversable(carbon_field_type_e type)
{
        return carbon_field_type_is_object_or_subtype(type) ||
                carbon_field_type_is_list_or_subtype(type);
}

bool carbon_field_type_is_signed(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_NUMBER_I8 || type == CARBON_FIELD_NUMBER_I16 ||
                type == CARBON_FIELD_NUMBER_I32 || type == CARBON_FIELD_NUMBER_I64 ||
                type == CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET);
}

bool carbon_field_type_is_unsigned(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_NUMBER_U8 || type == CARBON_FIELD_NUMBER_U16 ||
                type == CARBON_FIELD_NUMBER_U32 || type == CARBON_FIELD_NUMBER_U64 ||
                type == CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET ||
                type == CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET);
}

bool carbon_field_type_is_floating(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_NUMBER_FLOAT || type == CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET);
}

bool carbon_field_type_is_number(carbon_field_type_e type)
{
        return carbon_field_type_is_integer(type) || carbon_field_type_is_floating(type);
}

bool carbon_field_type_is_integer(carbon_field_type_e type)
{
        return carbon_field_type_is_signed(type) || carbon_field_type_is_unsigned(type);
}

bool carbon_field_type_is_binary(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_BINARY || type == CARBON_FIELD_BINARY_CUSTOM);
}

bool carbon_field_type_is_boolean(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_TRUE || type == CARBON_FIELD_FALSE ||
                type == CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET);
}

bool carbon_field_type_is_string(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_STRING);
}

bool carbon_field_type_is_constant(carbon_field_type_e type)
{
        return (carbon_field_type_is_null(type) || carbon_field_type_is_boolean(type));
}

bool carbon_field_skip(memfile *file)
{
        ERROR_IF_NULL(file)
        u8 type_marker = *MEMFILE_PEEK(file, u8);

        switch (type_marker) {
                case CARBON_FIELD_NULL:
                        carbon_field_skip_null(file);
                        break;
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        carbon_field_skip_boolean(file);
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                        carbon_field_skip_8(file);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                        carbon_field_skip_16(file);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                        carbon_field_skip_32(file);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                        carbon_field_skip_64(file);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        carbon_field_skip_float(file);
                        break;
                case CARBON_FIELD_STRING:
                        carbon_field_skip_string(file);
                        break;
                case CARBON_FIELD_BINARY:
                        carbon_field_skip_binary(file);
                        break;
                case CARBON_FIELD_BINARY_CUSTOM:
                        carbon_field_skip_custom_binary(file);
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        carbon_field_skip_array(file);
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        carbon_field_skip_column(file);
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        carbon_field_skip_object(file);
                        break;
                default: ERROR(&file->err, ERR_CORRUPTED);
                        return false;
        }
        return true;
}

fn_result carbon_field_skip_object(memfile *file)
{
        fn_result result = carbon_abstract_is_instanceof_object(file);
        if (LIKELY(FN_IS_OK(result) && FN_BOOL(result))) {
                carbon_object_it skip_it;
                carbon_object_it_create(&skip_it, file, &file->err, memfile_tell(file));
                carbon_object_it_fast_forward(&skip_it);
                memfile_seek(file, memfile_tell(&skip_it.memfile));
                carbon_object_it_drop(&skip_it);
                return FN_OK();
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "marker does not encode an object container or sub type");
        }
}

fn_result carbon_field_skip_array(memfile *file)
{
        fn_result result = carbon_abstract_is_instanceof_array(file);
        if (LIKELY(FN_IS_OK(result) && FN_BOOL(result))) {
                carbon_array_it skip_it;
                carbon_array_it_create(&skip_it, file, &file->err, memfile_tell(file));
                carbon_array_it_fast_forward(&skip_it);
                memfile_seek(file, memfile_tell(&skip_it.memfile));
                carbon_array_it_drop(&skip_it);
                return FN_OK();
        } else {
                return FN_FAIL(ERR_TYPEMISMATCH, "marker does not encode an array container or sub type");
        }
}

bool carbon_field_skip_column(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(!carbon_field_type_is_column_or_subtype(type_marker), &file->err, ERR_TYPEMISMATCH);

        carbon_column_it skip_it;
        carbon_column_it_create(&skip_it, file, &file->err,
                                memfile_tell(file) - sizeof(u8));
        carbon_column_it_fast_forward(&skip_it);
        memfile_seek(file, memfile_tell(&skip_it.memfile));
        return true;
}

bool carbon_field_skip_binary(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_BINARY, &file->err, ERR_TYPEMISMATCH);
        /** read and skip mime type with variable-length integer type */
        u64 mime_type = memfile_read_uintvar_stream(NULL, file);
        UNUSED(mime_type);

        /** read blob length */
        u64 blob_len = memfile_read_uintvar_stream(NULL, file);

        /** skip blob */
        memfile_skip(file, blob_len);
        return true;
}

bool carbon_field_skip_custom_binary(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_BINARY_CUSTOM, &file->err, ERR_TYPEMISMATCH);
        /** read custom type string_buffer length, and skip the type string_buffer */
        u64 custom_type_str_len = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, custom_type_str_len);

        /** read blob length, and skip blob data */
        u64 blob_len = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, blob_len);
        return true;
}

bool carbon_field_skip_string(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_STRING, &file->err, ERR_TYPEMISMATCH);
        u64 strlen = memfile_read_uintvar_stream(NULL, file);
        memfile_skip(file, strlen);
        return true;
}

bool carbon_field_skip_float(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NUMBER_FLOAT, &file->err, ERR_TYPEMISMATCH);
        memfile_skip(file, sizeof(float));
        return true;
}

bool carbon_field_skip_boolean(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_TRUE && type_marker != CARBON_FIELD_FALSE, &file->err,
                 ERR_TYPEMISMATCH);
        return true;
}

bool carbon_field_skip_null(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NULL, &file->err, ERR_TYPEMISMATCH);
        return true;
}

bool carbon_field_skip_8(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NUMBER_I8 && type_marker != CARBON_FIELD_NUMBER_U8,
                 &file->err, ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(u8) == sizeof(i8));
        memfile_skip(file, sizeof(u8));
        return true;
}

bool carbon_field_skip_16(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NUMBER_I16 && type_marker != CARBON_FIELD_NUMBER_U16,
                 &file->err, ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(u16) == sizeof(i16));
        memfile_skip(file, sizeof(u16));
        return true;
}

bool carbon_field_skip_32(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NUMBER_I32 && type_marker != CARBON_FIELD_NUMBER_U32,
                 &file->err, ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(u32) == sizeof(i32));
        memfile_skip(file, sizeof(u32));
        return true;
}

bool carbon_field_skip_64(memfile *file)
{
        u8 type_marker = *MEMFILE_READ_TYPE(file, u8);

        ERROR_IF(type_marker != CARBON_FIELD_NUMBER_I64 && type_marker != CARBON_FIELD_NUMBER_U64,
                 &file->err, ERR_TYPEMISMATCH);
        JAK_ASSERT(sizeof(u64) == sizeof(i64));
        memfile_skip(file, sizeof(u64));
        return true;
}

carbon_field_type_e carbon_field_type_for_column(carbon_list_derivable_e derivation, carbon_column_type_e type)
{
        switch (derivation) {
                case CARBON_LIST_UNSORTED_MULTISET:
                        switch (type) {
                                case CARBON_COLUMN_TYPE_U8:
                                        return CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U16:
                                        return CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U32:
                                        return CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U64:
                                        return CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I8:
                                        return CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I16:
                                        return CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I32:
                                        return CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I64:
                                        return CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_FLOAT:
                                        return CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_BOOLEAN:
                                        return CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET;
                                default: ERROR_PRINT(ERR_INTERNALERR)
                                        return 0;
                        }
                case CARBON_LIST_SORTED_MULTISET:
                        switch (type) {
                                case CARBON_COLUMN_TYPE_U8:
                                        return CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U16:
                                        return CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U32:
                                        return CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_U64:
                                        return CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I8:
                                        return CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I16:
                                        return CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I32:
                                        return CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_I64:
                                        return CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_FLOAT:
                                        return CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET;
                                case CARBON_COLUMN_TYPE_BOOLEAN:
                                        return CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET;
                                default: ERROR_PRINT(ERR_INTERNALERR)
                                        return 0;
                        }
                case CARBON_LIST_UNSORTED_SET:
                        switch (type) {
                                case CARBON_COLUMN_TYPE_U8:
                                        return CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_U16:
                                        return CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_U32:
                                        return CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_U64:
                                        return CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_I8:
                                        return CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_I16:
                                        return CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_I32:
                                        return CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_I64:
                                        return CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_FLOAT:
                                        return CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET;
                                case CARBON_COLUMN_TYPE_BOOLEAN:
                                        return CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET;
                                default: ERROR_PRINT(ERR_INTERNALERR)
                                        return 0;
                        }
                case CARBON_LIST_SORTED_SET:
                        switch (type) {
                                case CARBON_COLUMN_TYPE_U8:
                                        return CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET;
                                case CARBON_COLUMN_TYPE_U16:
                                        return CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET;
                                case CARBON_COLUMN_TYPE_U32:
                                        return CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET;
                                case CARBON_COLUMN_TYPE_U64:
                                        return CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET;
                                case CARBON_COLUMN_TYPE_I8:
                                        return CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET;
                                case CARBON_COLUMN_TYPE_I16:
                                        return CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET;
                                case CARBON_COLUMN_TYPE_I32:
                                        return CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET;
                                case CARBON_COLUMN_TYPE_I64:
                                        return CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET;
                                case CARBON_COLUMN_TYPE_FLOAT:
                                        return CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET;
                                case CARBON_COLUMN_TYPE_BOOLEAN:
                                        return CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET;
                                default: ERROR_PRINT(ERR_INTERNALERR)
                                        return 0;
                        }
                default: ERROR_PRINT(ERR_INTERNALERR)
                        return 0;
        }
}

carbon_field_type_e
carbon_field_type_column_entry_to_regular_type(carbon_field_type_e type, bool is_null, bool is_true)
{
        if (is_null) {
                return CARBON_FIELD_NULL;
        } else {
                switch (type) {
                        case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                                return CARBON_FIELD_NUMBER_U8;
                        case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                                return CARBON_FIELD_NUMBER_U16;
                        case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                                return CARBON_FIELD_NUMBER_U32;
                        case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                                return CARBON_FIELD_NUMBER_U64;
                        case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                                return CARBON_FIELD_NUMBER_I8;
                        case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                                return CARBON_FIELD_NUMBER_I16;
                        case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                                return CARBON_FIELD_NUMBER_I32;
                        case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                                return CARBON_FIELD_NUMBER_I64;
                        case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                                return CARBON_FIELD_NUMBER_FLOAT;
                        case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                        case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                                return is_true ? CARBON_FIELD_TRUE : CARBON_FIELD_FALSE;
                        default: ERROR_PRINT(ERR_INTERNALERR)
                                return 0;
                }
        }
}

carbon_field_class_e carbon_field_type_get_class(carbon_field_type_e type, err *err)
{
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        return CARBON_FIELD_CLASS_CONSTANT;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        return CARBON_FIELD_CLASS_CONTAINER;
                case CARBON_FIELD_STRING:
                        return CARBON_FIELD_CLASS_CHARACTER_STRING;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                        return CARBON_FIELD_CLASS_NUMBER;
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        return CARBON_FIELD_CLASS_BINARY_STRING;
                default: ERROR(err, ERR_INTERNALERR);
                        return 0;
        }
}

bool carbon_field_type_is_array_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_ARRAY_UNSORTED_MULTISET || type == CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET || type == CARBON_FIELD_DERIVED_ARRAY_SORTED_SET);
}

bool carbon_field_type_is_column_u8_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET);
}

bool carbon_field_type_is_column_u16_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET);
}

bool carbon_field_type_is_column_u32_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET);
}

bool carbon_field_type_is_column_u64_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET);
}

bool carbon_field_type_is_column_i8_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET);
}

bool carbon_field_type_is_column_i16_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET);
}

bool carbon_field_type_is_column_i32_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET);
}

bool carbon_field_type_is_column_i64_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET);
}

bool carbon_field_type_is_column_float_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET);
}

bool carbon_field_type_is_column_bool_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET ||
                type == CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET);
}



bool carbon_field_type_is_list_or_subtype(carbon_field_type_e type)
{
        return carbon_field_type_is_array_or_subtype(type) || carbon_field_type_is_column_or_subtype(type);
}

bool carbon_field_type_is_column_or_subtype(carbon_field_type_e type)
{
        return carbon_field_type_is_column_u8_or_subtype(type) ||
                carbon_field_type_is_column_u16_or_subtype(type) ||
                carbon_field_type_is_column_u32_or_subtype(type) ||
                carbon_field_type_is_column_u64_or_subtype(type) ||
                carbon_field_type_is_column_i8_or_subtype(type) ||
                carbon_field_type_is_column_i16_or_subtype(type) ||
                carbon_field_type_is_column_i32_or_subtype(type) ||
                carbon_field_type_is_column_i64_or_subtype(type) ||
                carbon_field_type_is_column_float_or_subtype(type) ||
                carbon_field_type_is_column_bool_or_subtype(type);
}

bool carbon_field_type_is_object_or_subtype(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP || type == CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP ||
                        type == CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP ||
                        type == CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP);
}

bool carbon_field_type_is_null(carbon_field_type_e type)
{
        return (type == CARBON_FIELD_NULL);
}