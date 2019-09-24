/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jak_uintvar_stream.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_media.h>
#include <carbon-containers.h>
#include <jak_carbon_int.h>
#include <jak_carbon_string.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_int.h>
#include <jak_utils_numbers.h>

#define check_type_if_container_is_column(inserter, expected)                                                          \
if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN && inserter->context.column->type != expected)) {                 \
        JAK_ERROR_WDETAILS(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

#define check_type_range_if_container_is_column(inserter, expected1, expected2, expected3)                             \
if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN && inserter->context.column->type != expected1 &&                 \
        inserter->context.column->type != expected2 && inserter->context.column->type != expected3)) {                 \
        JAK_ERROR_WDETAILS(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool
write_field_data(jak_carbon_insert *inserter, jak_u8 field_type_marker, const void *base, jak_u64 nbytes);

static bool push_in_column(jak_carbon_insert *inserter, const void *base, jak_carbon_field_type_e type);

static bool push_media_type_for_array(jak_carbon_insert *inserter, jak_carbon_field_type_e type);

static void internal_create(jak_carbon_insert *inserter, jak_memfile *src, jak_offset_t pos);

static void write_binary_blob(jak_carbon_insert *inserter, const void *value, size_t nbytes);

bool jak_carbon_int_insert_create_for_array(jak_carbon_insert *inserter, jak_carbon_array_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        jak_carbon_array_it_lock(context);
        inserter->context_type = JAK_CARBON_ARRAY;
        inserter->context.array = context;
        inserter->position = 0;

        jak_offset_t pos = 0;
        if (context->array_end_reached) {
                pos = jak_memfile_tell(&context->memfile);
        } else {
                pos = jak_carbon_int_history_has(&context->history) ? jak_carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool jak_carbon_int_insert_create_for_column(jak_carbon_insert *inserter, jak_carbon_column_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        jak_carbon_column_it_lock(context);
        inserter->context_type = JAK_CARBON_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile, jak_memfile_tell(&context->memfile));
        return true;
}

bool jak_carbon_int_insert_create_for_object(jak_carbon_insert *inserter, jak_carbon_object_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        jak_carbon_object_it_lock(context);
        inserter->context_type = JAK_CARBON_OBJECT;
        inserter->context.object = context;

        jak_offset_t pos;
        if (context->object_end_reached) {
                pos = jak_memfile_tell(&context->memfile);
        } else {
                pos = jak_carbon_int_history_has(&context->history) ? jak_carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool jak_carbon_insert_null(jak_carbon_insert *inserter)
{
        if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_U8 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_U16 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_U32 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_U64 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_I8 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_I16 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_I32 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_I64 &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                         inserter->context.column->type != JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN)) {
                JAK_ERROR_WDETAILS(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");
        }

        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_NULL);
                case JAK_CARBON_COLUMN: {
                        switch (inserter->context.column->type) {
                                case JAK_CARBON_FIELD_TYPE_COLUMN_U8: {
                                        jak_u8 value = JAK_U8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_U16: {
                                        jak_u16 value = JAK_U16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_U32: {
                                        jak_u32 value = JAK_U32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_U64: {
                                        jak_u64 value = JAK_U64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_I8: {
                                        jak_i8 value = JAK_I8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_I16: {
                                        jak_i16 value = JAK_I16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_I32: {
                                        jak_i32 value = JAK_I32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_I64: {
                                        jak_i64 value = JAK_I64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                                        float value = JAK_FLOAT_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR)
                                        return false;
                        }
                }
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_true(jak_carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_TRUE);
                case JAK_CARBON_COLUMN: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_TRUE;
                        return push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                }
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_false(jak_carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_FALSE);
                case JAK_CARBON_COLUMN: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_FALSE;
                        return push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                }
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_u8(jak_carbon_insert *inserter, jak_u8 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_U8);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U8, &value, sizeof(jak_u8));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_U8);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_u16(jak_carbon_insert *inserter, jak_u16 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U16, &value, sizeof(jak_u16));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_u32(jak_carbon_insert *inserter, jak_u32 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_U32);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U32, &value, sizeof(jak_u32));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_U32);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_u64(jak_carbon_insert *inserter, jak_u64 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_U64);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U64, &value, sizeof(jak_u64));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_U64);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_i8(jak_carbon_insert *inserter, jak_i8 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_I8);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I8, &value, sizeof(jak_i8));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_I8);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_i16(jak_carbon_insert *inserter, jak_i16 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_I16);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I16, &value, sizeof(jak_i16));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_I16);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_i32(jak_carbon_insert *inserter, jak_i32 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_I32);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I32, &value, sizeof(jak_i32));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_I32);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_i64(jak_carbon_insert *inserter, jak_i64 value)
{
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_I64);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I64, &value, sizeof(jak_i64));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_I64);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_unsigned(jak_carbon_insert *inserter, jak_u64 value)
{
        JAK_ERROR_IF(inserter->context_type == JAK_CARBON_COLUMN, &inserter->err, JAK_ERR_INSERT_TOO_DANGEROUS)

        switch (jak_number_min_type_signed(value)) {
                case JAK_NUMBER_I8:
                        return jak_carbon_insert_u8(inserter, (jak_u8) value);
                case JAK_NUMBER_I16:
                        return jak_carbon_insert_u16(inserter, (jak_u16) value);
                case JAK_NUMBER_I32:
                        return jak_carbon_insert_u32(inserter, (jak_u32) value);
                case JAK_NUMBER_I64:
                        return jak_carbon_insert_u64(inserter, (jak_u64) value);
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_signed(jak_carbon_insert *inserter, jak_i64 value)
{
        JAK_ERROR_IF(inserter->context_type == JAK_CARBON_COLUMN, &inserter->err, JAK_ERR_INSERT_TOO_DANGEROUS)

        switch (jak_number_min_type_signed(value)) {
                case JAK_NUMBER_I8:
                        return jak_carbon_insert_i8(inserter, (jak_i8) value);
                case JAK_NUMBER_I16:
                        return jak_carbon_insert_i16(inserter, (jak_i16) value);
                case JAK_NUMBER_I32:
                        return jak_carbon_insert_i32(inserter, (jak_i32) value);
                case JAK_NUMBER_I64:
                        return jak_carbon_insert_i64(inserter, (jak_i64) value);
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_float(jak_carbon_insert *inserter, float value)
{
        JAK_ERROR_IF_NULL(inserter)
        check_type_if_container_is_column(inserter, JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT);
                        break;
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool jak_carbon_insert_string(jak_carbon_insert *inserter, const char *value)
{
        return jak_carbon_insert_nchar(inserter, value, strlen(value));
}

bool jak_carbon_insert_nchar(jak_carbon_insert *inserter, const char *value, jak_u64 value_len)
{
        JAK_UNUSED(inserter);
        JAK_UNUSED(value);
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_ARRAY, &inserter->err, JAK_ERR_UNSUPPCONTAINER);

        return jak_carbon_jak_string_nchar_write(&inserter->memfile, value, value_len);
}

static void insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        if (user_type && strlen(user_type) > 0) {
                /* write media type 'user binary' */
                push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM);

                /* write length of 'user_type' string with variable-length integer type */
                jak_u64 user_type_strlen = strlen(user_type);

                jak_memfile_write_uintvar_stream(NULL, &inserter->memfile, user_type_strlen);

                /* write 'user_type' string */
                jak_memfile_ensure_space(&inserter->memfile, user_type_strlen);
                jak_memfile_write(&inserter->memfile, user_type, user_type_strlen);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);

        } else {
                /* write media type 'binary' */
                push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_BINARY);

                /* write mime type with variable-length integer type */
                jak_u64 mime_type_id = jak_carbon_media_mime_type_by_ext(file_ext);

                /* write mime type id */
                jak_memfile_write_uintvar_stream(NULL, &inserter->memfile, mime_type_id);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }
}

bool jak_carbon_insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_ARRAY, &inserter->err, JAK_ERR_UNSUPPCONTAINER);

        insert_binary(inserter, value, nbytes, file_ext, user_type);

        return true;
}

jak_carbon_insert *jak_carbon_insert_object_begin(jak_carbon_insert_object_state *out,
                                                     jak_carbon_insert *inserter, jak_u64 object_capacity)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(inserter)

        JAK_ERROR_IF_AND_RETURN(!out, &inserter->err, JAK_ERR_NULLPTR, NULL);
        if (!inserter) {
                JAK_ERROR_PRINT(JAK_ERR_NULLPTR);
                return false;
        }

        *out = (jak_carbon_insert_object_state) {
                .parent_inserter = inserter,
                .it = JAK_MALLOC(sizeof(jak_carbon_object_it)),
                .object_begin = jak_memfile_tell(&inserter->memfile),
                .object_end = 0
        };


        jak_carbon_int_insert_object(&inserter->memfile, object_capacity);
        jak_u64 payload_start = jak_memfile_tell(&inserter->memfile) - 1;

        jak_carbon_object_it_create(out->it, &inserter->memfile, &inserter->err, payload_start);
        jak_carbon_object_it_insert_begin(&out->inserter, out->it);

        return &out->inserter;
}

bool jak_carbon_insert_object_end(jak_carbon_insert_object_state *state)
{
        JAK_ERROR_IF_NULL(state);

        jak_carbon_object_it scan;
        jak_carbon_object_it_create(&scan, &state->parent_inserter->memfile, &state->parent_inserter->err,
                                jak_memfile_tell(&state->parent_inserter->memfile) - 1);
        while (jak_carbon_object_it_next(&scan)) {}

        JAK_ASSERT(*jak_memfile_peek(&scan.memfile, sizeof(char)) == CARBON_MOBJECT_END);
        jak_memfile_read(&scan.memfile, sizeof(char));

        state->object_end = jak_memfile_tell(&scan.memfile);

        jak_memfile_skip(&scan.memfile, 1);

        jak_memfile_seek(&state->parent_inserter->memfile, jak_memfile_tell(&scan.memfile) - 1);
        jak_carbon_object_it_drop(&scan);
        jak_carbon_insert_drop(&state->inserter);
        jak_carbon_object_it_drop(state->it);
        free(state->it);
        return true;
}

jak_carbon_insert *jak_carbon_insert_array_begin(jak_carbon_insert_array_state *state_out,
                                                    jak_carbon_insert *inserter_in, jak_u64 array_capacity)
{
        JAK_ERROR_IF_AND_RETURN(!state_out, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        if (!inserter_in) {
                JAK_ERROR_PRINT(JAK_ERR_NULLPTR);
                return false;
        }

        JAK_ERROR_IF(inserter_in->context_type != JAK_CARBON_ARRAY && inserter_in->context_type != JAK_CARBON_OBJECT,
                 &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);

        *state_out = (jak_carbon_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = JAK_MALLOC(sizeof(jak_carbon_array_it)),
                .array_begin = jak_memfile_tell(&inserter_in->memfile),
                .array_end = 0
        };

        jak_carbon_int_insert_array(&inserter_in->memfile, array_capacity);
        jak_u64 payload_start = jak_memfile_tell(&inserter_in->memfile) - 1;

        jak_carbon_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        jak_carbon_array_it_insert_begin(&state_out->nested_inserter, state_out->nested_array);

        return &state_out->nested_inserter;
}

bool jak_carbon_insert_array_end(jak_carbon_insert_array_state *state_in)
{
        JAK_ERROR_IF_NULL(state_in);

        jak_carbon_array_it scan;
        jak_carbon_array_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                               jak_memfile_tell(&state_in->parent_inserter->memfile) - 1);

        jak_carbon_array_it_fast_forward(&scan);

        state_in->array_end = jak_memfile_tell(&scan.memfile);
        jak_memfile_skip(&scan.memfile, 1);

        jak_memfile_seek(&state_in->parent_inserter->memfile, jak_memfile_tell(&scan.memfile) - 1);
        jak_carbon_array_it_drop(&scan);
        jak_carbon_insert_drop(&state_in->nested_inserter);
        jak_carbon_array_it_drop(state_in->nested_array);
        free(state_in->nested_array);
        return true;
}

jak_carbon_insert *jak_carbon_insert_column_begin(jak_carbon_insert_column_state *state_out,
                                                     jak_carbon_insert *inserter_in,
                                                     jak_carbon_column_type_e type,
                                                     jak_u64 column_capacity)
{
        JAK_ERROR_IF_AND_RETURN(!state_out, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        JAK_ERROR_IF_AND_RETURN(!inserter_in, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        JAK_ERROR_IF(inserter_in->context_type != JAK_CARBON_ARRAY && inserter_in->context_type != JAK_CARBON_OBJECT,
                 &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);

        jak_carbon_field_type_e field_type = jak_carbon_field_type_for_column(type);

        *state_out = (jak_carbon_insert_column_state) {
                .parent_inserter = inserter_in,
                .nested_column = JAK_MALLOC(sizeof(jak_carbon_column_it)),
                .type = field_type,
                .column_begin = jak_memfile_tell(&inserter_in->memfile),
                .column_end = 0
        };

        jak_u64 container_start_off = jak_memfile_tell(&inserter_in->memfile);
        jak_carbon_int_insert_column(&inserter_in->memfile, &inserter_in->err, type, column_capacity);

        jak_carbon_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err,
                                container_start_off);
        jak_carbon_column_it_insert(&state_out->nested_inserter, state_out->nested_column);

        return &state_out->nested_inserter;
}

bool jak_carbon_insert_column_end(jak_carbon_insert_column_state *state_in)
{
        JAK_ERROR_IF_NULL(state_in);

        jak_carbon_column_it scan;
        jak_carbon_column_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                                state_in->nested_column->column_start_offset);
        jak_carbon_column_it_fast_forward(&scan);

        state_in->column_end = jak_memfile_tell(&scan.memfile);
        jak_memfile_seek(&state_in->parent_inserter->memfile, jak_memfile_tell(&scan.memfile));

        jak_carbon_insert_drop(&state_in->nested_inserter);
        free(state_in->nested_column);
        return true;
}

static void inserter_refresh_mod_size(jak_carbon_insert *inserter, jak_i64 mod_size)
{
        JAK_ASSERT(mod_size > 0);

        jak_i64 *target = NULL;
        switch (inserter->context_type) {
                case JAK_CARBON_OBJECT:
                        target = &inserter->context.object->mod_size;
                        break;
                case JAK_CARBON_ARRAY:
                        target = &inserter->context.array->mod_size;
                        break;
                case JAK_CARBON_COLUMN:
                        target = &inserter->context.column->mod_size;
                        break;
                default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPCONTAINER);
        }
        *target += mod_size;
}

bool jak_carbon_insert_prop_null(jak_carbon_insert *inserter, const char *key)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_NULL);
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_true(jak_carbon_insert *inserter, const char *key)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_TRUE);
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_false(jak_carbon_insert *inserter, const char *key)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, JAK_CARBON_FIELD_TYPE_FALSE);
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_u8(jak_carbon_insert *inserter, const char *key, jak_u8 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U8, &value, sizeof(jak_u8));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_u16(jak_carbon_insert *inserter, const char *key, jak_u16 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U16, &value, sizeof(jak_u16));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_u32(jak_carbon_insert *inserter, const char *key, jak_u32 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U32, &value, sizeof(jak_u32));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_u64(jak_carbon_insert *inserter, const char *key, jak_u64 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_U64, &value, sizeof(jak_u64));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_i8(jak_carbon_insert *inserter, const char *key, jak_i8 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I8, &value, sizeof(jak_i8));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_i16(jak_carbon_insert *inserter, const char *key, jak_i16 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I16, &value, sizeof(jak_i16));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_i32(jak_carbon_insert *inserter, const char *key, jak_i32 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I32, &value, sizeof(jak_i32));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_i64(jak_carbon_insert *inserter, const char *key, jak_i64 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_I64, &value, sizeof(jak_i64));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_unsigned(jak_carbon_insert *inserter, const char *key, jak_u64 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER)

        switch (jak_number_min_type_unsigned(value)) {
                case JAK_NUMBER_U8:
                        return jak_carbon_insert_prop_u8(inserter, key, (jak_u8) value);
                case JAK_NUMBER_U16:
                        return jak_carbon_insert_prop_u16(inserter, key, (jak_u16) value);
                case JAK_NUMBER_U32:
                        return jak_carbon_insert_prop_u32(inserter, key, (jak_u32) value);
                case JAK_NUMBER_U64:
                        return jak_carbon_insert_prop_u64(inserter, key, (jak_u64) value);
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_prop_signed(jak_carbon_insert *inserter, const char *key, jak_i64 value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER)

        switch (jak_number_min_type_signed(value)) {
                case JAK_NUMBER_I8:
                        return jak_carbon_insert_prop_i8(inserter, key, (jak_i8) value);
                case JAK_NUMBER_I16:
                        return jak_carbon_insert_prop_i16(inserter, key, (jak_i16) value);
                case JAK_NUMBER_I32:
                        return jak_carbon_insert_prop_i32(inserter, key, (jak_i32) value);
                case JAK_NUMBER_I64:
                        return jak_carbon_insert_prop_i64(inserter, key, (jak_i64) value);
                default: JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool jak_carbon_insert_prop_float(jak_carbon_insert *inserter, const char *key, float value)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, JAK_CARBON_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_string(jak_carbon_insert *inserter, const char *key, const char *value)
{
        return jak_carbon_insert_prop_nchar(inserter, key, value, strlen(value));
}

bool jak_carbon_insert_prop_nchar(jak_carbon_insert *inserter, const char *key, const char *value, jak_u64 value_len)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        jak_carbon_jak_string_nchar_write(&inserter->memfile, value, value_len);
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool jak_carbon_insert_prop_binary(jak_carbon_insert *inserter, const char *key, const void *value,
                               size_t nbytes, const char *file_ext, const char *user_type)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = jak_memfile_tell(&inserter->memfile);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        insert_binary(inserter, value, nbytes, file_ext, user_type);
        jak_offset_t prop_end = jak_memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

jak_carbon_insert *jak_carbon_insert_prop_object_begin(jak_carbon_insert_object_state *out,
                                                          jak_carbon_insert *inserter, const char *key,
                                                          jak_u64 object_capacity)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        return jak_carbon_insert_object_begin(out, inserter, object_capacity);
}

jak_u64 jak_carbon_insert_prop_object_end(jak_carbon_insert_object_state *state)
{
        jak_carbon_insert_object_end(state);
        return state->object_end - state->object_begin;
}

jak_carbon_insert *jak_carbon_insert_prop_array_begin(jak_carbon_insert_array_state *state,
                                                         jak_carbon_insert *inserter, const char *key,
                                                         jak_u64 array_capacity)
{
        JAK_ERROR_IF(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_carbon_jak_string_nomarker_write(&inserter->memfile, key);
        return jak_carbon_insert_array_begin(state, inserter, array_capacity);
}

jak_u64 jak_carbon_insert_prop_array_end(jak_carbon_insert_array_state *state)
{
        jak_carbon_insert_array_end(state);
        return state->array_end - state->array_begin;
}

jak_carbon_insert *jak_carbon_insert_prop_column_begin(jak_carbon_insert_column_state *state_out,
                                                          jak_carbon_insert *inserter_in, const char *key,
                                                          jak_carbon_column_type_e type, jak_u64 column_capacity)
{
        JAK_ERROR_IF(inserter_in->context_type != JAK_CARBON_OBJECT, &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);
        jak_carbon_jak_string_nomarker_write(&inserter_in->memfile, key);
        return jak_carbon_insert_column_begin(state_out, inserter_in, type, column_capacity);
}

jak_u64 jak_carbon_insert_prop_column_end(jak_carbon_insert_column_state *state_in)
{
        jak_carbon_insert_column_end(state_in);
        return state_in->column_end - state_in->column_begin;
}

bool jak_carbon_insert_drop(jak_carbon_insert *inserter)
{
        JAK_ERROR_IF_NULL(inserter)
        if (inserter->context_type == JAK_CARBON_ARRAY) {
                jak_carbon_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == JAK_CARBON_COLUMN) {
                jak_carbon_column_it_unlock(inserter->context.column);
        } else if (inserter->context_type == JAK_CARBON_OBJECT) {
                jak_carbon_object_it_unlock(inserter->context.object);
        } else {
                JAK_ERROR(&inserter->err, JAK_ERR_INTERNALERR);
        }

        return true;
}

static bool
write_field_data(jak_carbon_insert *inserter, jak_u8 field_type_marker, const void *base, jak_u64 nbytes)
{
        JAK_ASSERT(inserter->context_type == JAK_CARBON_ARRAY || inserter->context_type == JAK_CARBON_OBJECT);

        jak_memfile_ensure_space(&inserter->memfile, sizeof(jak_u8) + nbytes);
        jak_memfile_write(&inserter->memfile, &field_type_marker, sizeof(jak_u8));
        return jak_memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(jak_carbon_insert *inserter, const void *base, jak_carbon_field_type_e type)
{
        JAK_ASSERT(inserter->context_type == JAK_CARBON_COLUMN);

        size_t type_size = jak_carbon_int_get_type_value_size(type);

        jak_memfile_save_position(&inserter->memfile);

        // Increase element counter
        jak_memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
        jak_u32 num_elems = jak_memfile_peek_uintvar_stream(NULL, &inserter->memfile);
        num_elems++;
        jak_memfile_update_uintvar_stream(&inserter->memfile, num_elems);
        inserter->context.column->column_num_elements = num_elems;

        jak_u32 capacity = jak_memfile_read_uintvar_stream(NULL, &inserter->memfile);

        if (JAK_UNLIKELY(num_elems > capacity)) {
                jak_memfile_save_position(&inserter->memfile);

                jak_u32 new_capacity = (capacity + 1) * 1.7f;

                // Update capacity counter
                jak_memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
                jak_memfile_skip_uintvar_stream(&inserter->memfile); // skip num element counter
                jak_memfile_update_uintvar_stream(&inserter->memfile, new_capacity);
                inserter->context.column->column_capacity = new_capacity;

                size_t payload_start = jak_carbon_int_column_get_payload_off(inserter->context.column);
                jak_memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
                jak_memfile_ensure_space(&inserter->memfile, (new_capacity - capacity) * type_size);

                jak_memfile_restore_position(&inserter->memfile);
        }

        size_t payload_start = jak_carbon_int_column_get_payload_off(inserter->context.column);
        jak_memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
        jak_memfile_write(&inserter->memfile, base, type_size);

        jak_memfile_restore_position(&inserter->memfile);
        return true;
}

static bool push_media_type_for_array(jak_carbon_insert *inserter, jak_carbon_field_type_e type)
{
        jak_memfile_ensure_space(&inserter->memfile, sizeof(jak_media_type));
        return jak_carbon_media_write(&inserter->memfile, type);
}

static void internal_create(jak_carbon_insert *inserter, jak_memfile *src, jak_offset_t pos)
{
        jak_memfile_clone(&inserter->memfile, src);
        jak_error_init(&inserter->err);
        inserter->position = pos ? pos : jak_memfile_tell(src);
        jak_memfile_seek(&inserter->memfile, inserter->position);
}

static void write_binary_blob(jak_carbon_insert *inserter, const void *value, size_t nbytes)
{
        /* write blob length */
        jak_memfile_write_uintvar_stream(NULL, &inserter->memfile, nbytes);

        /* write blob */
        jak_memfile_ensure_space(&inserter->memfile, nbytes);
        jak_memfile_write(&inserter->memfile, value, nbytes);
}