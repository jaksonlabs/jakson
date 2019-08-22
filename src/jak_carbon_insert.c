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
#include <jak_carbon_int.h>
#include <jak_carbon_string.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_int.h>
#include <jak_utils_numbers.h>

#define check_type_if_container_is_column(inserter, expected)                                                          \
if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN && inserter->context.column->type != expected)) {                 \
        error_with_details(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

#define check_type_range_if_container_is_column(inserter, expected1, expected2, expected3)                             \
if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN && inserter->context.column->type != expected1 &&                 \
        inserter->context.column->type != expected2 && inserter->context.column->type != expected3)) {                 \
        error_with_details(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool
write_field_data(jak_carbon_insert *inserter, jak_u8 field_type_marker, const void *base, jak_u64 nbytes);

static bool push_in_column(jak_carbon_insert *inserter, const void *base, enum carbon_field_type type);

static bool push_media_type_for_array(jak_carbon_insert *inserter, enum carbon_field_type type);

static void internal_create(jak_carbon_insert *inserter, struct jak_memfile *src, jak_offset_t pos);

static void write_binary_blob(jak_carbon_insert *inserter, const void *value, size_t nbytes);

bool carbon_int_insert_create_for_array(jak_carbon_insert *inserter, struct jak_carbon_array_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        carbon_array_it_lock(context);
        inserter->context_type = JAK_CARBON_ARRAY;
        inserter->context.array = context;
        inserter->position = 0;

        jak_offset_t pos = 0;
        if (context->array_end_reached) {
                pos = memfile_tell(&context->memfile);
        } else {
                pos = carbon_int_history_has(&context->history) ? carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool carbon_int_insert_create_for_column(jak_carbon_insert *inserter, struct jak_carbon_column_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        carbon_column_it_lock(context);
        inserter->context_type = JAK_CARBON_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile, memfile_tell(&context->memfile));
        return true;
}

bool carbon_int_insert_create_for_object(jak_carbon_insert *inserter, struct jak_carbon_object_it *context)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(context)
        carbon_object_it_lock(context);
        inserter->context_type = JAK_CARBON_OBJECT;
        inserter->context.object = context;

        jak_offset_t pos;
        if (context->object_end_reached) {
                pos = memfile_tell(&context->memfile);
        } else {
                pos = carbon_int_history_has(&context->history) ? carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool carbon_insert_null(jak_carbon_insert *inserter)
{
        if (JAK_UNLIKELY(inserter->context_type == JAK_CARBON_COLUMN &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_U8 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_U16 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_U32 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_U64 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_I8 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_I16 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_I32 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_I64 &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT &&
                         inserter->context.column->type != CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN)) {
                error_with_details(&inserter->err, JAK_ERR_TYPEMISMATCH, "Element type does not match container type");
        }

        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_NULL);
                case JAK_CARBON_COLUMN: {
                        switch (inserter->context.column->type) {
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U8: {
                                        jak_u8 value = U8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U16: {
                                        jak_u16 value = U16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U32: {
                                        jak_u32 value = U32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_U64: {
                                        jak_u64 value = U64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I8: {
                                        jak_i8 value = I8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I16: {
                                        jak_i16 value = I16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I32: {
                                        jak_i32 value = I32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_I64: {
                                        jak_i64 value = I64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT: {
                                        float value = FLOAT_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN: {
                                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                        break;
                                default: error(&inserter->err, JAK_ERR_INTERNALERR)
                                        return false;
                        }
                }
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_true(jak_carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_TRUE);
                case JAK_CARBON_COLUMN: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_TRUE;
                        return push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN);
                }
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_false(jak_carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_FALSE);
                case JAK_CARBON_COLUMN: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_FALSE;
                        return push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN);
                }
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_u8(jak_carbon_insert *inserter, jak_u8 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_U8);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U8, &value, sizeof(jak_u8));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_U8);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u16(jak_carbon_insert *inserter, jak_u16 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_U16);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U16, &value, sizeof(jak_u16));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_U16);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u32(jak_carbon_insert *inserter, jak_u32 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_U32);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U32, &value, sizeof(jak_u32));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_U32);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u64(jak_carbon_insert *inserter, jak_u64 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_U64);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U64, &value, sizeof(jak_u64));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_U64);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i8(jak_carbon_insert *inserter, jak_i8 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_I8);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I8, &value, sizeof(jak_i8));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_I8);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i16(jak_carbon_insert *inserter, jak_i16 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_I16);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I16, &value, sizeof(jak_i16));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_I16);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i32(jak_carbon_insert *inserter, jak_i32 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_I32);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I32, &value, sizeof(jak_i32));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_I32);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i64(jak_carbon_insert *inserter, jak_i64 value)
{
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_I64);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I64, &value, sizeof(jak_i64));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_I64);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_unsigned(jak_carbon_insert *inserter, jak_u64 value)
{
        error_if(inserter->context_type == JAK_CARBON_COLUMN, &inserter->err, JAK_ERR_INSERT_TOO_DANGEROUS)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_u8(inserter, (jak_u8) value);
                case NUMBER_I16:
                        return carbon_insert_u16(inserter, (jak_u16) value);
                case NUMBER_I32:
                        return carbon_insert_u32(inserter, (jak_u32) value);
                case NUMBER_I64:
                        return carbon_insert_u64(inserter, (jak_u64) value);
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_signed(jak_carbon_insert *inserter, jak_i64 value)
{
        error_if(inserter->context_type == JAK_CARBON_COLUMN, &inserter->err, JAK_ERR_INSERT_TOO_DANGEROUS)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_i8(inserter, (jak_i8) value);
                case NUMBER_I16:
                        return carbon_insert_i16(inserter, (jak_i16) value);
                case NUMBER_I32:
                        return carbon_insert_i32(inserter, (jak_i32) value);
                case NUMBER_I64:
                        return carbon_insert_i64(inserter, (jak_i64) value);
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_float(jak_carbon_insert *inserter, float value)
{
        JAK_ERROR_IF_NULL(inserter)
        check_type_if_container_is_column(inserter, CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT);
        switch (inserter->context_type) {
                case JAK_CARBON_ARRAY:
                        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
                        break;
                case JAK_CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT);
                        break;
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_string(jak_carbon_insert *inserter, const char *value)
{
        return carbon_insert_nchar(inserter, value, strlen(value));
}

bool carbon_insert_nchar(jak_carbon_insert *inserter, const char *value, jak_u64 value_len)
{
        JAK_UNUSED(inserter);
        JAK_UNUSED(value);
        error_if(inserter->context_type != JAK_CARBON_ARRAY, &inserter->err, JAK_ERR_UNSUPPCONTAINER);

        return carbon_string_nchar_write(&inserter->memfile, value, value_len);
}

static void insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        if (user_type && strlen(user_type) > 0) {
                /* write media type 'user binary' */
                push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM);

                /* write length of 'user_type' string with variable-length integer type */
                jak_u64 user_type_strlen = strlen(user_type);

                memfile_write_uintvar_stream(NULL, &inserter->memfile, user_type_strlen);

                /* write 'user_type' string */
                memfile_ensure_space(&inserter->memfile, user_type_strlen);
                memfile_write(&inserter->memfile, user_type, user_type_strlen);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);

        } else {
                /* write media type 'binary' */
                push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_BINARY);

                /* write mime type with variable-length integer type */
                jak_u64 mime_type_id = carbon_media_mime_type_by_ext(file_ext);

                /* write mime type id */
                memfile_write_uintvar_stream(NULL, &inserter->memfile, mime_type_id);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }
}

bool carbon_insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(value)
        error_if(inserter->context_type != JAK_CARBON_ARRAY, &inserter->err, JAK_ERR_UNSUPPCONTAINER);

        insert_binary(inserter, value, nbytes, file_ext, user_type);

        return true;
}

jak_carbon_insert *carbon_insert_object_begin(struct jak_carbon_insert_object_state *out,
                                                     jak_carbon_insert *inserter, jak_u64 object_capacity)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(inserter)

        error_if_and_return(!out, &inserter->err, JAK_ERR_NULLPTR, NULL);
        if (!inserter) {
                error_print(JAK_ERR_NULLPTR);
                return false;
        }

        *out = (struct jak_carbon_insert_object_state) {
                .parent_inserter = inserter,
                .it = JAK_MALLOC(sizeof(struct jak_carbon_object_it)),
                .object_begin = memfile_tell(&inserter->memfile),
                .object_end = 0
        };


        carbon_int_insert_object(&inserter->memfile, object_capacity);
        jak_u64 payload_start = memfile_tell(&inserter->memfile) - 1;

        carbon_object_it_create(out->it, &inserter->memfile, &inserter->err, payload_start);
        carbon_object_it_insert_begin(&out->inserter, out->it);

        return &out->inserter;
}

bool carbon_insert_object_end(struct jak_carbon_insert_object_state *state)
{
        JAK_ERROR_IF_NULL(state);

        struct jak_carbon_object_it scan;
        carbon_object_it_create(&scan, &state->parent_inserter->memfile, &state->parent_inserter->err,
                                memfile_tell(&state->parent_inserter->memfile) - 1);
        while (carbon_object_it_next(&scan)) {}

        JAK_ASSERT(*memfile_peek(&scan.memfile, sizeof(char)) == JAK_CARBON_MARKER_OBJECT_END);
        memfile_read(&scan.memfile, sizeof(char));

        state->object_end = memfile_tell(&scan.memfile);

        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state->parent_inserter->memfile, memfile_tell(&scan.memfile) - 1);
        carbon_object_it_drop(&scan);
        carbon_insert_drop(&state->inserter);
        carbon_object_it_drop(state->it);
        free(state->it);
        return true;
}

jak_carbon_insert *carbon_insert_array_begin(struct jak_carbon_insert_array_state *state_out,
                                                    jak_carbon_insert *inserter_in, jak_u64 array_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        if (!inserter_in) {
                error_print(JAK_ERR_NULLPTR);
                return false;
        }

        error_if(inserter_in->context_type != JAK_CARBON_ARRAY && inserter_in->context_type != JAK_CARBON_OBJECT,
                 &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);

        *state_out = (struct jak_carbon_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = JAK_MALLOC(sizeof(struct jak_carbon_array_it)),
                .array_begin = memfile_tell(&inserter_in->memfile),
                .array_end = 0
        };

        carbon_int_insert_array(&inserter_in->memfile, array_capacity);
        jak_u64 payload_start = memfile_tell(&inserter_in->memfile) - 1;

        carbon_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        carbon_array_it_insert_begin(&state_out->nested_inserter, state_out->nested_array);

        return &state_out->nested_inserter;
}

bool carbon_insert_array_end(struct jak_carbon_insert_array_state *state_in)
{
        JAK_ERROR_IF_NULL(state_in);

        struct jak_carbon_array_it scan;
        carbon_array_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                               memfile_tell(&state_in->parent_inserter->memfile) - 1);

        carbon_array_it_fast_forward(&scan);

        state_in->array_end = memfile_tell(&scan.memfile);
        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile) - 1);
        carbon_array_it_drop(&scan);
        carbon_insert_drop(&state_in->nested_inserter);
        carbon_array_it_drop(state_in->nested_array);
        free(state_in->nested_array);
        return true;
}

jak_carbon_insert *carbon_insert_column_begin(struct jak_carbon_insert_column_state *state_out,
                                                     jak_carbon_insert *inserter_in,
                                                     enum carbon_column_type type,
                                                     jak_u64 column_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        error_if_and_return(!inserter_in, &inserter_in->err, JAK_ERR_NULLPTR, NULL);
        error_if(inserter_in->context_type != JAK_CARBON_ARRAY && inserter_in->context_type != JAK_CARBON_OBJECT,
                 &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);

        enum carbon_field_type field_type = carbon_field_type_for_column(type);

        *state_out = (struct jak_carbon_insert_column_state) {
                .parent_inserter = inserter_in,
                .nested_column = JAK_MALLOC(sizeof(struct jak_carbon_column_it)),
                .type = field_type,
                .column_begin = memfile_tell(&inserter_in->memfile),
                .column_end = 0
        };

        jak_u64 container_start_off = memfile_tell(&inserter_in->memfile);
        carbon_int_insert_column(&inserter_in->memfile, &inserter_in->err, type, column_capacity);

        carbon_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err,
                                container_start_off);
        carbon_column_it_insert(&state_out->nested_inserter, state_out->nested_column);

        return &state_out->nested_inserter;
}

bool carbon_insert_column_end(struct jak_carbon_insert_column_state *state_in)
{
        JAK_ERROR_IF_NULL(state_in);

        struct jak_carbon_column_it scan;
        carbon_column_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                                state_in->nested_column->column_start_offset);
        carbon_column_it_fast_forward(&scan);

        state_in->column_end = memfile_tell(&scan.memfile);
        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile));

        carbon_insert_drop(&state_in->nested_inserter);
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
                default: error_print(JAK_ERR_UNSUPPCONTAINER);
        }
        *target += mod_size;
}

bool carbon_insert_prop_null(jak_carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_NULL);
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_true(jak_carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_TRUE);
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_false(jak_carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_JAK_FIELD_TYPE_FALSE);
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u8(jak_carbon_insert *inserter, const char *key, jak_u8 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U8, &value, sizeof(jak_u8));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u16(jak_carbon_insert *inserter, const char *key, jak_u16 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U16, &value, sizeof(jak_u16));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u32(jak_carbon_insert *inserter, const char *key, jak_u32 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U32, &value, sizeof(jak_u32));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u64(jak_carbon_insert *inserter, const char *key, jak_u64 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_U64, &value, sizeof(jak_u64));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i8(jak_carbon_insert *inserter, const char *key, jak_i8 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I8, &value, sizeof(jak_i8));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i16(jak_carbon_insert *inserter, const char *key, jak_i16 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I16, &value, sizeof(jak_i16));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i32(jak_carbon_insert *inserter, const char *key, jak_i32 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I32, &value, sizeof(jak_i32));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i64(jak_carbon_insert *inserter, const char *key, jak_i64 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_I64, &value, sizeof(jak_i64));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_unsigned(jak_carbon_insert *inserter, const char *key, jak_u64 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER)

        switch (number_min_type_unsigned(value)) {
                case NUMBER_U8:
                        return carbon_insert_prop_u8(inserter, key, (jak_u8) value);
                case NUMBER_U16:
                        return carbon_insert_prop_u16(inserter, key, (jak_u16) value);
                case NUMBER_U32:
                        return carbon_insert_prop_u32(inserter, key, (jak_u32) value);
                case NUMBER_U64:
                        return carbon_insert_prop_u64(inserter, key, (jak_u64) value);
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_prop_signed(jak_carbon_insert *inserter, const char *key, jak_i64 value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_prop_i8(inserter, key, (jak_i8) value);
                case NUMBER_I16:
                        return carbon_insert_prop_i16(inserter, key, (jak_i16) value);
                case NUMBER_I32:
                        return carbon_insert_prop_i32(inserter, key, (jak_i32) value);
                case NUMBER_I64:
                        return carbon_insert_prop_i64(inserter, key, (jak_i64) value);
                default: error(&inserter->err, JAK_ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_prop_float(jak_carbon_insert *inserter, const char *key, float value)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_string(jak_carbon_insert *inserter, const char *key, const char *value)
{
        return carbon_insert_prop_nchar(inserter, key, value, strlen(value));
}

bool carbon_insert_prop_nchar(jak_carbon_insert *inserter, const char *key, const char *value, jak_u64 value_len)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        carbon_string_nchar_write(&inserter->memfile, value, value_len);
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_binary(jak_carbon_insert *inserter, const char *key, const void *value,
                               size_t nbytes, const char *file_ext, const char *user_type)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        jak_offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        insert_binary(inserter, value, nbytes, file_ext, user_type);
        jak_offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

jak_carbon_insert *carbon_insert_prop_object_begin(struct jak_carbon_insert_object_state *out,
                                                          jak_carbon_insert *inserter, const char *key,
                                                          jak_u64 object_capacity)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_object_begin(out, inserter, object_capacity);
}

jak_u64 carbon_insert_prop_object_end(struct jak_carbon_insert_object_state *state)
{
        carbon_insert_object_end(state);
        return state->object_end - state->object_begin;
}

jak_carbon_insert *carbon_insert_prop_array_begin(struct jak_carbon_insert_array_state *state,
                                                         jak_carbon_insert *inserter, const char *key,
                                                         jak_u64 array_capacity)
{
        error_if(inserter->context_type != JAK_CARBON_OBJECT, &inserter->err, JAK_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_array_begin(state, inserter, array_capacity);
}

jak_u64 carbon_insert_prop_array_end(struct jak_carbon_insert_array_state *state)
{
        carbon_insert_array_end(state);
        return state->array_end - state->array_begin;
}

jak_carbon_insert *carbon_insert_prop_column_begin(struct jak_carbon_insert_column_state *state_out,
                                                          jak_carbon_insert *inserter_in, const char *key,
                                                          enum carbon_column_type type, jak_u64 column_capacity)
{
        error_if(inserter_in->context_type != JAK_CARBON_OBJECT, &inserter_in->err, JAK_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter_in->memfile, key);
        return carbon_insert_column_begin(state_out, inserter_in, type, column_capacity);
}

jak_u64 carbon_insert_prop_column_end(struct jak_carbon_insert_column_state *state_in)
{
        carbon_insert_column_end(state_in);
        return state_in->column_end - state_in->column_begin;
}

bool carbon_insert_drop(jak_carbon_insert *inserter)
{
        JAK_ERROR_IF_NULL(inserter)
        if (inserter->context_type == JAK_CARBON_ARRAY) {
                carbon_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == JAK_CARBON_COLUMN) {
                carbon_column_it_unlock(inserter->context.column);
        } else if (inserter->context_type == JAK_CARBON_OBJECT) {
                carbon_object_it_unlock(inserter->context.object);
        } else {
                error(&inserter->err, JAK_ERR_INTERNALERR);
        }

        return true;
}

static bool
write_field_data(jak_carbon_insert *inserter, jak_u8 field_type_marker, const void *base, jak_u64 nbytes)
{
        JAK_ASSERT(inserter->context_type == JAK_CARBON_ARRAY || inserter->context_type == JAK_CARBON_OBJECT);

        memfile_ensure_space(&inserter->memfile, sizeof(jak_u8) + nbytes);
        memfile_write(&inserter->memfile, &field_type_marker, sizeof(jak_u8));
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(jak_carbon_insert *inserter, const void *base, enum carbon_field_type type)
{
        JAK_ASSERT(inserter->context_type == JAK_CARBON_COLUMN);

        size_t type_size = carbon_int_get_type_value_size(type);

        memfile_save_position(&inserter->memfile);

        // Increase element counter
        memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
        jak_u32 num_elems = memfile_peek_uintvar_stream(NULL, &inserter->memfile);
        num_elems++;
        memfile_update_uintvar_stream(&inserter->memfile, num_elems);
        inserter->context.column->column_num_elements = num_elems;

        jak_u32 capacity = memfile_read_uintvar_stream(NULL, &inserter->memfile);

        if (JAK_UNLIKELY(num_elems > capacity)) {
                memfile_save_position(&inserter->memfile);

                jak_u32 new_capacity = (capacity + 1) * 1.7f;

                // Update capacity counter
                memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
                memfile_skip_uintvar_stream(&inserter->memfile); // skip num element counter
                memfile_update_uintvar_stream(&inserter->memfile, new_capacity);
                inserter->context.column->column_capacity = new_capacity;

                size_t payload_start = carbon_int_column_get_payload_off(inserter->context.column);
                memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
                memfile_ensure_space(&inserter->memfile, (new_capacity - capacity) * type_size);

                memfile_restore_position(&inserter->memfile);
        }

        size_t payload_start = carbon_int_column_get_payload_off(inserter->context.column);
        memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
        memfile_write(&inserter->memfile, base, type_size);

        memfile_restore_position(&inserter->memfile);
        return true;
}

static bool push_media_type_for_array(jak_carbon_insert *inserter, enum carbon_field_type type)
{
        memfile_ensure_space(&inserter->memfile, sizeof(media_type_t));
        return carbon_media_write(&inserter->memfile, type);
}

static void internal_create(jak_carbon_insert *inserter, struct jak_memfile *src, jak_offset_t pos)
{
        memfile_clone(&inserter->memfile, src);
        error_init(&inserter->err);
        inserter->position = pos ? pos : memfile_tell(src);
        memfile_seek(&inserter->memfile, inserter->position);
}

static void write_binary_blob(jak_carbon_insert *inserter, const void *value, size_t nbytes)
{
        /* write blob length */
        memfile_write_uintvar_stream(NULL, &inserter->memfile, nbytes);

        /* write blob */
        memfile_ensure_space(&inserter->memfile, nbytes);
        memfile_write(&inserter->memfile, value, nbytes);
}