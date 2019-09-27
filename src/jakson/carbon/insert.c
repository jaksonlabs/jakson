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

#include <jakson/std/uintvar/stream.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/insert.h>
#include <jakson/carbon/mime.h>
#include <jakson/carbon/containers.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/string.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/internal.h>
#include <jakson/utils/numbers.h>

#define check_type_if_container_is_column(inserter, expr)                                                              \
if (UNLIKELY(inserter->context_type == CARBON_COLUMN && !(expr))) {                                            \
        ERROR_WDETAILS(&inserter->err, ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

#define check_type_range_if_container_is_column(inserter, expected1, expected2, expected3)                             \
if (UNLIKELY(inserter->context_type == CARBON_COLUMN && inserter->context.column->type != expected1 &&                 \
        inserter->context.column->type != expected2 && inserter->context.column->type != expected3)) {                 \
        ERROR_WDETAILS(&inserter->err, ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool
write_field_data(carbon_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes);

static bool push_in_column(carbon_insert *inserter, const void *base, carbon_field_type_e type);

static bool push_media_type_for_array(carbon_insert *inserter, carbon_field_type_e type);

static void internal_create(carbon_insert *inserter, memfile *src, offset_t pos);

static void write_binary_blob(carbon_insert *inserter, const void *value, size_t nbytes);

bool carbon_int_insert_create_for_array(carbon_insert *inserter, carbon_array_it *context)
{
        ERROR_IF_NULL(inserter)
        ERROR_IF_NULL(context)
        carbon_array_it_lock(context);
        inserter->context_type = CARBON_ARRAY;
        inserter->context.array = context;
        inserter->position = 0;

        offset_t pos = 0;
        if (context->array_end_reached) {
                pos = memfile_tell(&context->memfile);
        } else {
                pos = carbon_int_history_has(&context->history) ? carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool carbon_int_insert_create_for_column(carbon_insert *inserter, carbon_column_it *context)
{
        ERROR_IF_NULL(inserter)
        ERROR_IF_NULL(context)
        carbon_column_it_lock(context);
        inserter->context_type = CARBON_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile, memfile_tell(&context->memfile));
        return true;
}

bool carbon_int_insert_create_for_object(carbon_insert *inserter, carbon_object_it *context)
{
        ERROR_IF_NULL(inserter)
        ERROR_IF_NULL(context)
        carbon_object_it_lock(context);
        inserter->context_type = CARBON_OBJECT;
        inserter->context.object = context;

        offset_t pos;
        if (context->object_end_reached) {
                pos = memfile_tell(&context->memfile);
        } else {
                pos = carbon_int_history_has(&context->history) ? carbon_int_history_peek(&context->history) : 0;
        }

        internal_create(inserter, &context->memfile, pos);
        return true;
}

bool carbon_insert_null(carbon_insert *inserter)
{
        if (UNLIKELY(inserter->context_type == CARBON_COLUMN &&
                !carbon_field_type_is_column_or_subtype(inserter->context.column->type))) {
                ERROR_WDETAILS(&inserter->err, ERR_TYPEMISMATCH, "Element type does not match container type");
        }

        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_FIELD_NULL);
                case CARBON_COLUMN: {
                        switch (inserter->context.column->type) {
                                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET: {
                                        u8 value = U8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET: {
                                        u16 value = U16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET: {
                                        u32 value = U32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET: {
                                        u64 value = U64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET: {
                                        i8 value = I8_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET: {
                                        i16 value = I16_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET: {
                                        i32 value = I32_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                                        i64 value = I64_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET: {
                                        float value = FLOAT_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                                        u8 value = CARBON_BOOLEAN_COLUMN_NULL;
                                        return push_in_column(inserter, &value, inserter->context.column->type);
                                }
                                default: ERROR(&inserter->err, ERR_INTERNALERR)
                                        return false;
                        }
                }
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_true(carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_bool_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_FIELD_TRUE);
                case CARBON_COLUMN: {
                        u8 value = CARBON_BOOLEAN_COLUMN_TRUE;
                        return push_in_column(inserter, &value, CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET);
                }
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_false(carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_bool_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        return push_media_type_for_array(inserter, CARBON_FIELD_FALSE);
                case CARBON_COLUMN: {
                        u8 value = CARBON_BOOLEAN_COLUMN_FALSE;
                        return push_in_column(inserter, &value, CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET);
                }
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_u8(carbon_insert *inserter, u8 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_u8_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_U8, &value, sizeof(u8));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u16(carbon_insert *inserter, u16 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_u16_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_U16, &value, sizeof(u16));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u32(carbon_insert *inserter, u32 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_u32_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_U32, &value, sizeof(u32));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_u64(carbon_insert *inserter, u64 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_u64_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_U64, &value, sizeof(u64));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i8(carbon_insert *inserter, i8 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_i8_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_I8, &value, sizeof(i8));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i16(carbon_insert *inserter, i16 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_i16_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_I16, &value, sizeof(i16));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i32(carbon_insert *inserter, i32 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_i32_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_I32, &value, sizeof(i32));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_i64(carbon_insert *inserter, i64 value)
{
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_i64_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_I64, &value, sizeof(i64));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_unsigned(carbon_insert *inserter, u64 value)
{
        ERROR_IF(inserter->context_type == CARBON_COLUMN, &inserter->err, ERR_INSERT_TOO_DANGEROUS)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_u8(inserter, (u8) value);
                case NUMBER_I16:
                        return carbon_insert_u16(inserter, (u16) value);
                case NUMBER_I32:
                        return carbon_insert_u32(inserter, (u32) value);
                case NUMBER_I64:
                        return carbon_insert_u64(inserter, (u64) value);
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_signed(carbon_insert *inserter, i64 value)
{
        ERROR_IF(inserter->context_type == CARBON_COLUMN, &inserter->err, ERR_INSERT_TOO_DANGEROUS)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_i8(inserter, (i8) value);
                case NUMBER_I16:
                        return carbon_insert_i16(inserter, (i16) value);
                case NUMBER_I32:
                        return carbon_insert_i32(inserter, (i32) value);
                case NUMBER_I64:
                        return carbon_insert_i64(inserter, (i64) value);
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_float(carbon_insert *inserter, float value)
{
        ERROR_IF_NULL(inserter)
        check_type_if_container_is_column(inserter, carbon_field_type_is_column_float_or_subtype(inserter->context.column->type));
        switch (inserter->context_type) {
                case CARBON_ARRAY:
                        write_field_data(inserter, CARBON_FIELD_NUMBER_FLOAT, &value, sizeof(float));
                        break;
                case CARBON_COLUMN:
                        push_in_column(inserter, &value, CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET);
                        break;
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
        return true;
}

bool carbon_insert_string(carbon_insert *inserter, const char *value)
{
        return carbon_insert_nchar(inserter, value, strlen(value));
}

bool carbon_insert_nchar(carbon_insert *inserter, const char *value, u64 value_len)
{
        UNUSED(inserter);
        UNUSED(value);
        ERROR_IF(inserter->context_type != CARBON_ARRAY, &inserter->err, ERR_UNSUPPCONTAINER);

        return carbon_string_nchar_write(&inserter->memfile, value, value_len);
}

static void insert_binary(carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        if (user_type && strlen(user_type) > 0) {
                /** write media type 'user binary' */
                push_media_type_for_array(inserter, CARBON_FIELD_BINARY_CUSTOM);

                /** write length of 'user_type' string_buffer with variable-length integer type */
                u64 user_type_strlen = strlen(user_type);

                memfile_write_uintvar_stream(NULL, &inserter->memfile, user_type_strlen);

                /** write 'user_type' string_buffer */
                memfile_ensure_space(&inserter->memfile, user_type_strlen);
                memfile_write(&inserter->memfile, user_type, user_type_strlen);

                /** write binary blob */
                write_binary_blob(inserter, value, nbytes);

        } else {
                /** write media type 'binary' */
                push_media_type_for_array(inserter, CARBON_FIELD_BINARY);

                /** write mime type with variable-length integer type */
                u64 mime_type_id = carbon_media_mime_type_by_ext(file_ext);

                /** write mime type id */
                memfile_write_uintvar_stream(NULL, &inserter->memfile, mime_type_id);

                /** write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }
}

bool carbon_insert_binary(carbon_insert *inserter, const void *value, size_t nbytes,
                          const char *file_ext, const char *user_type)
{
        ERROR_IF_NULL(inserter)
        ERROR_IF_NULL(value)
        ERROR_IF(inserter->context_type != CARBON_ARRAY, &inserter->err, ERR_UNSUPPCONTAINER);

        insert_binary(inserter, value, nbytes, file_ext, user_type);

        return true;
}

carbon_insert *__carbon_insert_map_begin(carbon_insert_object_state *out,
                                                  carbon_insert *inserter, carbon_map_derivable_e derivation,
                                                  u64 object_capacity)
{
        ERROR_IF_NULL(out)
        ERROR_IF_NULL(inserter)

        ERROR_IF_AND_RETURN(!out, &inserter->err, ERR_NULLPTR, NULL);
        if (!inserter) {
                ERROR_PRINT(ERR_NULLPTR);
                return false;
        }

        *out = (carbon_insert_object_state) {
                .parent_inserter = inserter,
                .it = MALLOC(sizeof(carbon_object_it)),
                .object_begin = memfile_tell(&inserter->memfile),
                .object_end = 0
        };


        carbon_int_insert_object(&inserter->memfile, derivation, object_capacity);
        u64 payload_start = memfile_tell(&inserter->memfile) - 1;

        carbon_object_it_create(out->it, &inserter->memfile, &inserter->err, payload_start);
        carbon_object_it_insert_begin(&out->inserter, out->it);

        return &out->inserter;
}

carbon_insert *carbon_insert_object_begin(carbon_insert_object_state *out,
                                                  carbon_insert *inserter,
                                                  u64 object_capacity)
{
        return __carbon_insert_map_begin(out, inserter, CARBON_MAP_UNSORTED_MULTIMAP, object_capacity);
}

bool carbon_insert_object_end(carbon_insert_object_state *state)
{
        ERROR_IF_NULL(state);

        carbon_object_it scan;
        carbon_object_it_create(&scan, &state->parent_inserter->memfile, &state->parent_inserter->err,
                                memfile_tell(&state->parent_inserter->memfile) - 1);
        while (carbon_object_it_next(&scan)) {}

        JAK_ASSERT(*memfile_peek(&scan.memfile, sizeof(char)) == CARBON_MOBJECT_END);
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

carbon_insert *carbon_insert_map_begin(carbon_insert_object_state *out, carbon_insert *inserter, carbon_map_derivable_e derivation, u64 object_capacity)
{
        return __carbon_insert_map_begin(out, inserter, derivation, object_capacity);
}

bool carbon_insert_map_end(carbon_insert_object_state *state)
{
        return carbon_insert_object_end(state);
}

carbon_insert *__carbon_insert_array_list_begin(carbon_insert_array_state *state_out,
                                                 carbon_insert *inserter_in, carbon_list_derivable_e derivation, u64 array_capacity)
{
        ERROR_IF_AND_RETURN(!state_out, &inserter_in->err, ERR_NULLPTR, NULL);
        if (!inserter_in) {
                ERROR_PRINT(ERR_NULLPTR);
                return false;
        }

        ERROR_IF(inserter_in->context_type != CARBON_ARRAY && inserter_in->context_type != CARBON_OBJECT,
                     &inserter_in->err, ERR_UNSUPPCONTAINER);

        *state_out = (carbon_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = MALLOC(sizeof(carbon_array_it)),
                .array_begin = memfile_tell(&inserter_in->memfile),
                .array_end = 0
        };

        carbon_int_insert_array(&inserter_in->memfile, derivation, array_capacity);
        u64 payload_start = memfile_tell(&inserter_in->memfile) - 1;

        carbon_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        carbon_array_it_insert_begin(&state_out->nested_inserter, state_out->nested_array);

        return &state_out->nested_inserter;
}

carbon_insert *carbon_insert_array_begin(carbon_insert_array_state *state_out,
                                                    carbon_insert *inserter_in, u64 array_capacity)
{
        return __carbon_insert_array_list_begin(state_out, inserter_in, CARBON_LIST_UNSORTED_MULTISET, array_capacity);
}

bool carbon_insert_array_end(carbon_insert_array_state *state_in)
{
        ERROR_IF_NULL(state_in);

        carbon_array_it scan;
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

carbon_insert *carbon_insert_array_list_begin(carbon_insert_array_state *state_out, carbon_insert *inserter_in, carbon_list_derivable_e derivation, u64 array_capacity)
{
        return __carbon_insert_array_list_begin(state_out, inserter_in, derivation, array_capacity);
}

bool carbon_insert_array_list_end(carbon_insert_array_state *state_in)
{
        return carbon_insert_array_end(state_in);
}


carbon_insert *__carbon_insert_column_list_begin(carbon_insert_column_state *state_out,
                                                     carbon_insert *inserter_in,
                                                     carbon_list_derivable_e derivation,
                                                     carbon_column_type_e type,
                                                     u64 column_capacity)
{
        ERROR_IF_AND_RETURN(!state_out, &inserter_in->err, ERR_NULLPTR, NULL);
        ERROR_IF_AND_RETURN(!inserter_in, &inserter_in->err, ERR_NULLPTR, NULL);
        ERROR_IF(inserter_in->context_type != CARBON_ARRAY && inserter_in->context_type != CARBON_OBJECT,
                     &inserter_in->err, ERR_UNSUPPCONTAINER);

        carbon_field_type_e field_type = carbon_field_type_for_column(derivation, type);

        *state_out = (carbon_insert_column_state) {
                .parent_inserter = inserter_in,
                .nested_column = MALLOC(sizeof(carbon_column_it)),
                .type = field_type,
                .column_begin = memfile_tell(&inserter_in->memfile),
                .column_end = 0
        };

        u64 container_start_off = memfile_tell(&inserter_in->memfile);
        carbon_int_insert_column(&inserter_in->memfile, &inserter_in->err, derivation, type, column_capacity);

        carbon_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err,
                                    container_start_off);
        carbon_column_it_insert(&state_out->nested_inserter, state_out->nested_column);

        return &state_out->nested_inserter;
}

carbon_insert *carbon_insert_column_begin(carbon_insert_column_state *state_out,
                                                     carbon_insert *inserter_in,
                                                     carbon_column_type_e type,
                                                     u64 column_capacity)
{
        return __carbon_insert_column_list_begin(state_out, inserter_in, CARBON_LIST_UNSORTED_MULTISET, type, column_capacity);
}

bool carbon_insert_column_end(carbon_insert_column_state *state_in)
{
        ERROR_IF_NULL(state_in);

        carbon_column_it scan;
        carbon_column_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                                state_in->nested_column->column_start_offset);
        carbon_column_it_fast_forward(&scan);

        state_in->column_end = memfile_tell(&scan.memfile);
        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile));

        carbon_insert_drop(&state_in->nested_inserter);
        free(state_in->nested_column);
        return true;
}

carbon_insert *carbon_insert_column_list_begin(carbon_insert_column_state *state_out, carbon_insert *inserter_in, carbon_list_derivable_e derivation, carbon_column_type_e type, u64 column_capacity)
{
        return __carbon_insert_column_list_begin(state_out, inserter_in, derivation, type, column_capacity);
}

bool carbon_insert_column_list_end(carbon_insert_column_state *state_in)
{
        return carbon_insert_column_end(state_in);
}

static void inserter_refresh_mod_size(carbon_insert *inserter, i64 mod_size)
{
        JAK_ASSERT(mod_size > 0);

        i64 *target = NULL;
        switch (inserter->context_type) {
                case CARBON_OBJECT:
                        target = &inserter->context.object->mod_size;
                        break;
                case CARBON_ARRAY:
                        target = &inserter->context.array->mod_size;
                        break;
                case CARBON_COLUMN:
                        target = &inserter->context.column->mod_size;
                        break;
                default: ERROR_PRINT(ERR_UNSUPPCONTAINER);
        }
        *target += mod_size;
}

bool carbon_insert_prop_null(carbon_insert *inserter, const char *key)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_FIELD_NULL);
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_true(carbon_insert *inserter, const char *key)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_FIELD_TRUE);
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_false(carbon_insert *inserter, const char *key)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, CARBON_FIELD_FALSE);
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u8(carbon_insert *inserter, const char *key, u8 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_U8, &value, sizeof(u8));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u16(carbon_insert *inserter, const char *key, u16 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_U16, &value, sizeof(u16));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u32(carbon_insert *inserter, const char *key, u32 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_U32, &value, sizeof(u32));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_u64(carbon_insert *inserter, const char *key, u64 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_U64, &value, sizeof(u64));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i8(carbon_insert *inserter, const char *key, i8 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_I8, &value, sizeof(i8));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i16(carbon_insert *inserter, const char *key, i16 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_I16, &value, sizeof(i16));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i32(carbon_insert *inserter, const char *key, i32 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_I32, &value, sizeof(i32));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_i64(carbon_insert *inserter, const char *key, i64 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_I64, &value, sizeof(i64));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_unsigned(carbon_insert *inserter, const char *key, u64 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER)

        switch (number_min_type_unsigned(value)) {
                case NUMBER_U8:
                        return carbon_insert_prop_u8(inserter, key, (u8) value);
                case NUMBER_U16:
                        return carbon_insert_prop_u16(inserter, key, (u16) value);
                case NUMBER_U32:
                        return carbon_insert_prop_u32(inserter, key, (u32) value);
                case NUMBER_U64:
                        return carbon_insert_prop_u64(inserter, key, (u64) value);
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_prop_signed(carbon_insert *inserter, const char *key, i64 value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER)

        switch (number_min_type_signed(value)) {
                case NUMBER_I8:
                        return carbon_insert_prop_i8(inserter, key, (i8) value);
                case NUMBER_I16:
                        return carbon_insert_prop_i16(inserter, key, (i16) value);
                case NUMBER_I32:
                        return carbon_insert_prop_i32(inserter, key, (i32) value);
                case NUMBER_I64:
                        return carbon_insert_prop_i64(inserter, key, (i64) value);
                default: ERROR(&inserter->err, ERR_INTERNALERR);
                        return false;
        }
}

bool carbon_insert_prop_float(carbon_insert *inserter, const char *key, float value)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        write_field_data(inserter, CARBON_FIELD_NUMBER_FLOAT, &value, sizeof(float));
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_string(carbon_insert *inserter, const char *key, const char *value)
{
        return carbon_insert_prop_nchar(inserter, key, value, strlen(value));
}

bool carbon_insert_prop_nchar(carbon_insert *inserter, const char *key, const char *value, u64 value_len)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        carbon_string_nchar_write(&inserter->memfile, value, value_len);
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

bool carbon_insert_prop_binary(carbon_insert *inserter, const char *key, const void *value,
                               size_t nbytes, const char *file_ext, const char *user_type)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        offset_t prop_start = memfile_tell(&inserter->memfile);
        carbon_string_nomarker_write(&inserter->memfile, key);
        insert_binary(inserter, value, nbytes, file_ext, user_type);
        offset_t prop_end = memfile_tell(&inserter->memfile);
        inserter_refresh_mod_size(inserter, prop_end - prop_start);
        return true;
}

static carbon_insert *__carbon_insert_prop_object_container_begin(carbon_insert_object_state *out,
                                                       carbon_insert *inserter, carbon_map_derivable_e derivation, const char *key,
                                                       u64 object_capacity)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_map_begin(out, inserter, derivation, object_capacity);
}

carbon_insert *carbon_insert_prop_object_begin(carbon_insert_object_state *out,
                                                          carbon_insert *inserter, const char *key,
                                                          u64 object_capacity)
{
        return __carbon_insert_prop_object_container_begin(out, inserter, CARBON_MAP_UNSORTED_MULTIMAP,
                                                           key, object_capacity);
}

u64 carbon_insert_prop_object_end(carbon_insert_object_state *state)
{
        carbon_insert_object_end(state);
        return state->object_end - state->object_begin;
}

carbon_insert *carbon_insert_prop_map_begin(carbon_insert_object_state *out, carbon_insert *inserter, carbon_map_derivable_e derivation, const char *key, u64 object_capacity)
{
        return __carbon_insert_prop_object_container_begin(out, inserter, derivation,
                                                           key, object_capacity);
}

u64 carbon_insert_prop_map_end(carbon_insert_object_state *state)
{
        return carbon_insert_prop_object_end(state);
}

carbon_insert *carbon_insert_prop_array_begin(carbon_insert_array_state *state,
                                                         carbon_insert *inserter, const char *key,
                                                         u64 array_capacity)
{
        ERROR_IF(inserter->context_type != CARBON_OBJECT, &inserter->err, ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_array_begin(state, inserter, array_capacity);
}

u64 carbon_insert_prop_array_end(carbon_insert_array_state *state)
{
        carbon_insert_array_end(state);
        return state->array_end - state->array_begin;
}

carbon_insert *carbon_insert_prop_column_begin(carbon_insert_column_state *state_out,
                                                          carbon_insert *inserter_in, const char *key,
                                                          carbon_column_type_e type, u64 column_capacity)
{
        ERROR_IF(inserter_in->context_type != CARBON_OBJECT, &inserter_in->err, ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter_in->memfile, key);
        return carbon_insert_column_begin(state_out, inserter_in, type, column_capacity);
}

u64 carbon_insert_prop_column_end(carbon_insert_column_state *state_in)
{
        carbon_insert_column_end(state_in);
        return state_in->column_end - state_in->column_begin;
}

bool carbon_insert_drop(carbon_insert *inserter)
{
        ERROR_IF_NULL(inserter)
        if (inserter->context_type == CARBON_ARRAY) {
                carbon_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == CARBON_COLUMN) {
                carbon_column_it_unlock(inserter->context.column);
        } else if (inserter->context_type == CARBON_OBJECT) {
                carbon_object_it_unlock(inserter->context.object);
        } else {
                ERROR(&inserter->err, ERR_INTERNALERR);
        }

        return true;
}

static bool
write_field_data(carbon_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes)
{
        JAK_ASSERT(inserter->context_type == CARBON_ARRAY || inserter->context_type == CARBON_OBJECT);

        memfile_ensure_space(&inserter->memfile, sizeof(u8) + nbytes);
        memfile_write(&inserter->memfile, &field_type_marker, sizeof(u8));
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(carbon_insert *inserter, const void *base, carbon_field_type_e type)
{
        JAK_ASSERT(inserter->context_type == CARBON_COLUMN);

        size_t type_size = carbon_int_get_type_value_size(type);

        memfile_save_position(&inserter->memfile);

        // Increase element counter
        memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
        u32 num_elems = memfile_peek_uintvar_stream(NULL, &inserter->memfile);
        num_elems++;
        memfile_update_uintvar_stream(&inserter->memfile, num_elems);
        inserter->context.column->column_num_elements = num_elems;

        u32 capacity = memfile_read_uintvar_stream(NULL, &inserter->memfile);

        if (UNLIKELY(num_elems > capacity)) {
                memfile_save_position(&inserter->memfile);

                u32 new_capacity = (capacity + 1) * 1.7f;

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

static bool push_media_type_for_array(carbon_insert *inserter, carbon_field_type_e type)
{
        memfile_ensure_space(&inserter->memfile, sizeof(media_type));
        return carbon_media_write(&inserter->memfile, type);
}

static void internal_create(carbon_insert *inserter, memfile *src, offset_t pos)
{
        memfile_clone(&inserter->memfile, src);
        error_init(&inserter->err);
        inserter->position = pos ? pos : memfile_tell(src);
        memfile_seek(&inserter->memfile, inserter->position);
}

static void write_binary_blob(carbon_insert *inserter, const void *value, size_t nbytes)
{
        /** write blob length */
        memfile_write_uintvar_stream(NULL, &inserter->memfile, nbytes);

        /** write blob */
        memfile_ensure_space(&inserter->memfile, nbytes);
        memfile_write(&inserter->memfile, value, nbytes);
}