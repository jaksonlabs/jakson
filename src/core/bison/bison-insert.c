/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include "stdx/varuint.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-string.h"
#include "core/bison/bison-object-it.h"

#define check_type_if_container_is_column(inserter, expected)                                                          \
if (unlikely(inserter->context_type == BISON_COLUMN && inserter->context.column->type != expected)) {                  \
        error_with_details(&inserter->err, NG5_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

#define check_type_range_if_container_is_column(inserter, expected1, expected2, expected3)                             \
if (unlikely(inserter->context_type == BISON_COLUMN && inserter->context.column->type != expected1 &&                  \
        inserter->context.column->type != expected2 && inserter->context.column->type != expected3)) {                 \
        error_with_details(&inserter->err, NG5_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool write_field_data(struct bison_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes);
static bool push_in_column(struct bison_insert *inserter, const void *base, enum bison_field_type type);

static bool push_media_type_for_array(struct bison_insert *inserter, enum bison_field_type type);
static void internal_create(struct bison_insert *inserter, struct memfile *src);
static void write_binary_blob(struct bison_insert *inserter, const void *value, size_t nbytes);

NG5_EXPORT(bool) bison_int_insert_create_for_array(struct bison_insert *inserter, struct bison_array_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_array_it_lock(context);
        inserter->context_type = BISON_ARRAY;
        inserter->context.array = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_int_insert_create_for_column(struct bison_insert *inserter, struct bison_column_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_column_it_lock(context);
        inserter->context_type = BISON_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_int_insert_create_for_object(struct bison_insert *inserter, struct bison_object_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_object_it_lock(context);
        inserter->context_type = BISON_OBJECT;
        inserter->context.object = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter)
{
        check_type_range_if_container_is_column(inserter, BISON_FIELD_TYPE_NULL, BISON_FIELD_TYPE_TRUE,
                BISON_FIELD_TYPE_FALSE);

        switch (inserter->context_type) {
        case BISON_ARRAY:
                return push_media_type_for_array(inserter, BISON_FIELD_TYPE_NULL);
        case BISON_COLUMN: {
                u8 value;

                switch (inserter->context.column->type) {
                        case BISON_FIELD_TYPE_NULL:
                                value = BISON_FIELD_TYPE_NULL;
                                break;
                        case BISON_FIELD_TYPE_TRUE:
                        case BISON_FIELD_TYPE_FALSE:
                                value = BISON_BOOLEAN_COLUMN_NULL;
                                break;
                        default:
                                error(&inserter->err, NG5_ERR_INTERNALERR)
                                return false;
                }

                return push_in_column(inserter, &value, BISON_FIELD_TYPE_NULL);
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_insert_true(struct bison_insert *inserter)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_TRUE);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                return push_media_type_for_array(inserter, BISON_FIELD_TYPE_TRUE);
        case BISON_COLUMN: {
                u8 value = BISON_BOOLEAN_COLUMN_TRUE;
                return push_in_column(inserter, &value, BISON_FIELD_TYPE_TRUE);
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_insert_false(struct bison_insert *inserter)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_FALSE);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                return push_media_type_for_array(inserter, BISON_FIELD_TYPE_FALSE);
        case BISON_COLUMN: {
                u8 value = BISON_BOOLEAN_COLUMN_FALSE;
                return push_in_column(inserter, &value, BISON_FIELD_TYPE_FALSE);
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) bison_insert_u8(struct bison_insert *inserter, u8 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_U8);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U8, &value, sizeof(u8));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_U8);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_u16(struct bison_insert *inserter, u16 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_U16);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U16, &value, sizeof(u16));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_U16);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_u32(struct bison_insert *inserter, u32 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_U32);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U32, &value, sizeof(u32));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_U32);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_u64(struct bison_insert *inserter, u64 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_U64);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U64, &value, sizeof(u64));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_U64);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_i8(struct bison_insert *inserter, i8 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_I8);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I8, &value, sizeof(i8));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_I8);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_i16(struct bison_insert *inserter, i16 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_I16);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I16, &value, sizeof(i16));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_I16);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_i32(struct bison_insert *inserter, i32 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_I32);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I32, &value, sizeof(i32));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_I32);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_i64(struct bison_insert *inserter, i64 value)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_I64);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I64, &value, sizeof(i64));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_I64);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_unsigned(struct bison_insert *inserter, u64 value)
{
        error_if(inserter->context_type == BISON_COLUMN, &inserter->err, NG5_ERR_INSERT_TOO_DANGEROUS)

        if (value <= BISON_U8_MAX) {
                return bison_insert_u8(inserter, (u8) value);
        } else if (value <= BISON_U16_MAX) {
                return bison_insert_u16(inserter, (u16) value);
        } else if (value <= BISON_U32_MAX) {
                return bison_insert_u32(inserter, (u32) value);
        } else if (value <= BISON_U64_MAX) {
                return bison_insert_u64(inserter, (u64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_signed(struct bison_insert *inserter, i64 value)
{
        error_if(inserter->context_type == BISON_COLUMN, &inserter->err, NG5_ERR_INSERT_TOO_DANGEROUS)

        if (value >= BISON_I8_MIN && value <= BISON_I8_MAX) {
                return bison_insert_i8(inserter, (i8) value);
        } else if (value >= BISON_I16_MIN && value <= BISON_I16_MAX) {
                return bison_insert_i16(inserter, (i16) value);
        } else if (value >= BISON_I32_MIN && value <= BISON_I32_MAX) {
                return bison_insert_i32(inserter, (i32) value);
        } else if (value >= BISON_I64_MIN && value <= BISON_I64_MAX) {
                return bison_insert_i64(inserter, (i64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_float(struct bison_insert *inserter, float value)
{
        error_if_null(inserter)
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NUMBER_FLOAT);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
                break;
        case BISON_COLUMN:
                push_in_column(inserter, &value, BISON_FIELD_TYPE_NUMBER_FLOAT);
                break;
        default:
        error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_string(struct bison_insert *inserter, const char *value)
{
        unused(inserter);
        unused(value);
        error_if(inserter->context_type != BISON_ARRAY, &inserter->err, NG5_ERR_UNSUPPCONTAINER);

        return bison_string_write(&inserter->memfile, value);
}

NG5_EXPORT(bool) bison_insert_binary(struct bison_insert *inserter, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type)
{
        error_if_null(inserter)
        error_if_null(value)
        error_if(inserter->context_type != BISON_ARRAY, &inserter->err, NG5_ERR_UNSUPPCONTAINER);

        if (user_type && strlen(user_type) > 0) {
                /* write media type 'user binary' */
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_BINARY_CUSTOM);

                /* write length of 'user_type' string with variable-length integer type */
                u64 user_type_strlen = strlen(user_type);

                memfile_write_varuint(&inserter->memfile, user_type_strlen);

                /* write 'user_type' string */
                memfile_ensure_space(&inserter->memfile, user_type_strlen);
                memfile_write(&inserter->memfile, user_type, user_type_strlen);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);

        } else {
                /* write media type 'binary' */
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_BINARY);

                /* write mime type with variable-length integer type */
                u64 mime_type_id = bison_media_mime_type_by_ext(file_ext);

                /* write mime type id */
                memfile_write_varuint(&inserter->memfile, mime_type_id);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }

        return true;
}

NG5_EXPORT(struct bison_insert *) bison_insert_object_begin(struct bison_insert_object_state *out,
        struct bison_insert *inserter, u64 object_capacity)
{
        error_if_null(out)
        error_if_null(inserter)

        error_if_and_return(!out, &inserter->err, NG5_ERR_NULLPTR, NULL);
        if (!inserter) {
                error_print(NG5_ERR_NULLPTR);
                return false;
        }

        *out = (struct bison_insert_object_state) {
                .parent_inserter = inserter,
                .it = malloc(sizeof(struct bison_object_it))
        };

        bison_int_insert_object(&inserter->memfile, object_capacity);
        u64 payload_start = memfile_tell(&inserter->memfile) - 1;

        bison_object_it_create(out->it, &inserter->memfile, &inserter->err, payload_start);
        bison_object_it_insert_begin(&out->inserter, out->it);

        return &out->inserter;
}

NG5_EXPORT(bool) bison_insert_object_end(struct bison_insert_object_state *state)
{
        error_if_null(state)

        bison_insert_drop(&state->inserter);
        free(state->it);

        return true;
}

NG5_EXPORT(struct bison_insert *) bison_insert_array_begin(struct bison_insert_array_state *state_out,
        struct bison_insert *inserter_in, u64 array_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        if (!inserter_in) {
                error_print(NG5_ERR_NULLPTR);
                return false;
        }

        error_if(inserter_in->context_type != BISON_ARRAY, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);

        *state_out = (struct bison_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = malloc(sizeof(struct bison_array_it))
        };

        bison_int_insert_array(&inserter_in->memfile, array_capacity);
        u64 payload_start = memfile_tell(&inserter_in->memfile) - 1;

        bison_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        bison_array_it_insert_begin(&state_out->nested_inserter, state_out->nested_array);

        return &state_out->nested_inserter;
}

NG5_EXPORT(bool) bison_insert_array_end(struct bison_insert_array_state *state_in)
{
        error_if_null(state_in);

        struct bison_array_it scan;
        bison_array_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                memfile_tell(&state_in->parent_inserter->memfile) - 1);
        while (bison_array_it_next(&scan))
                { }

        char final = *memfile_read(&scan.memfile, sizeof(char));
        assert(final == BISON_MARKER_ARRAY_END);
        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile) - 1);
        bison_array_it_drop(&scan);
        bison_insert_drop(&state_in->nested_inserter);
        bison_array_it_drop(state_in->nested_array);
        free(state_in->nested_array);
        return true;
}

NG5_EXPORT(struct bison_insert *) bison_insert_column_begin(struct bison_insert_column_state *state_out,
        struct bison_insert *inserter_in, enum bison_field_type type, u64 column_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if_and_return(!inserter_in, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if(inserter_in->context_type != BISON_ARRAY, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);

        *state_out = (struct bison_insert_column_state) {
                .parent_inserter = inserter_in,
                .nested_column = malloc(sizeof(struct bison_column_it)),
                .type = type
        };

        u64 container_start_off = memfile_tell(&inserter_in->memfile);
        bison_int_insert_column(&inserter_in->memfile, &inserter_in->err, type, column_capacity);

        bison_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err, container_start_off);
        bison_column_it_insert(&state_out->nested_inserter, state_out->nested_column);

        return &state_out->nested_inserter;
}

NG5_EXPORT(bool) bison_insert_column_end(struct bison_insert_column_state *state_in)
{
        error_if_null(state_in);

        struct bison_column_it scan;
        bison_column_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                state_in->nested_column->column_start_offset);
        bison_column_it_fast_forward(&scan);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile));

        bison_insert_drop(&state_in->nested_inserter);
        free(state_in->nested_column);
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_null(struct bison_insert *inserter, const char *key)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, BISON_FIELD_TYPE_NULL);
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_true(struct bison_insert *inserter, const char *key)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, BISON_FIELD_TYPE_TRUE);
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_false(struct bison_insert *inserter, const char *key)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, BISON_FIELD_TYPE_FALSE);
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_u8(struct bison_insert *inserter, const char *key, u8 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U8, &value, sizeof(u8));
}

NG5_EXPORT(bool) bison_insert_prop_u16(struct bison_insert *inserter, const char *key, u16 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U16, &value, sizeof(u16));
}

NG5_EXPORT(bool) bison_insert_prop_u32(struct bison_insert *inserter, const char *key, u32 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U32, &value, sizeof(u32));
}

NG5_EXPORT(bool) bison_insert_prop_u64(struct bison_insert *inserter, const char *key, u64 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_U64, &value, sizeof(u64));
}

NG5_EXPORT(bool) bison_insert_prop_i8(struct bison_insert *inserter, const char *key, i8 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I8, &value, sizeof(i8));
}

NG5_EXPORT(bool) bison_insert_prop_i16(struct bison_insert *inserter, const char *key, i16 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I16, &value, sizeof(i16));
}

NG5_EXPORT(bool) bison_insert_prop_i32(struct bison_insert *inserter, const char *key, i32 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I32, &value, sizeof(i32));
}

NG5_EXPORT(bool) bison_insert_prop_i64(struct bison_insert *inserter, const char *key, i64 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_I64, &value, sizeof(i64));
}

NG5_EXPORT(bool) bison_insert_prop_unsigned(struct bison_insert *inserter, const char *key, u64 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER)

        if (value <= BISON_U8_MAX) {
                return bison_insert_prop_u8(inserter, key, (u8) value);
        } else if (value <= BISON_U16_MAX) {
                return bison_insert_prop_u16(inserter, key, (u16) value);
        } else if (value <= BISON_U32_MAX) {
                return bison_insert_prop_u32(inserter, key, (u32) value);
        } else if (value <= BISON_U64_MAX) {
                return bison_insert_prop_u64(inserter, key, (u64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_signed(struct bison_insert *inserter, const char *key, i64 value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER)

        if (value >= BISON_I8_MIN && value <= BISON_I8_MAX) {
                return bison_insert_prop_i8(inserter, key, (i8) value);
        } else if (value >= BISON_I16_MIN && value <= BISON_I16_MAX) {
                return bison_insert_prop_i16(inserter, key, (i16) value);
        } else if (value >= BISON_I32_MIN && value <= BISON_I32_MAX) {
                return bison_insert_prop_i32(inserter, key, (i32) value);
        } else if (value >= BISON_I64_MIN && value <= BISON_I64_MAX) {
                return bison_insert_prop_i64(inserter, key, (i64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_insert_prop_float(struct bison_insert *inserter, const char *key, float value)
{
        error_if(inserter->context_type != BISON_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        bison_string_write(&inserter->memfile, key);
        return write_field_data(inserter, BISON_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
}
//
//NG5_EXPORT(bool) bison_insert_prop_string(struct bison_insert *inserter, const char *key, const char *value)
//{
//
//}
//
//NG5_EXPORT(bool) bison_insert_prop_binary(struct bison_insert *inserter, const char *key, const void *value,
//        size_t nbytes, const char *file_ext, const char *user_type)
//{
//
//}
//
//NG5_EXPORT(struct bison_insert *) bison_insert_prop_object_begin(struct bison_insert_object_state *out,
//        struct bison_insert *inserter, const char *key, u64 object_capacity)
//{
//
//}
//
//NG5_EXPORT(bool) bison_insert_prop_object_end(struct bison_insert_object_state *state)
//{
//
//}
//
//NG5_EXPORT(struct bison_insert *) bison_insert_prop_array_begin(struct bison_insert_array_state *state_out,
//        struct bison_insert *inserter_in, const char *key, u64 array_capacity)
//{
//
//}
//
//NG5_EXPORT(bool) bison_insert_prop_array_end(struct bison_insert_array_state *state_in)
//{
//
//}
//
//NG5_EXPORT(struct bison_insert *) bison_insert_prop_column_begin(struct bison_insert_column_state *state_out,
//        struct bison_insert *inserter_in, const char *key, enum bison_field_type type, u64 column_capacity)
//{
//
//}
//
//NG5_EXPORT(bool) bison_insert_prop_column_end(struct bison_insert_column_state *state_in)
//{
//
//}

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter)
{
        error_if_null(inserter)
        if (inserter->context_type == BISON_ARRAY) {
                bison_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == BISON_COLUMN) {
                bison_column_it_unlock(inserter->context.column);
        } else if (inserter->context_type == BISON_OBJECT) {
                bison_object_it_unlock(inserter->context.object);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
        }

        return true;
}

static bool write_field_data(struct bison_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes)
{
        assert(inserter->context_type == BISON_ARRAY || inserter->context_type == BISON_OBJECT);

        memfile_ensure_space(&inserter->memfile, sizeof(u8) + nbytes);
        memfile_write(&inserter->memfile, &field_type_marker, sizeof(u8));
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(struct bison_insert *inserter, const void *base, enum bison_field_type type)
{
        assert(inserter->context_type == BISON_COLUMN);

        size_t type_size = bison_int_get_type_value_size(type);

        memfile_save_position(&inserter->memfile);

        // Increase element counter
        memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
        u32 num_elems = memfile_peek_varuint(NULL, &inserter->memfile);
        num_elems++;
        memfile_update_varuint(&inserter->memfile, num_elems);
        inserter->context.column->column_num_elements = num_elems;

        u32 capacity = memfile_read_varuint(NULL, &inserter->memfile);

        if (unlikely(num_elems > capacity)) {
                memfile_save_position(&inserter->memfile);

                u32 new_capacity = (capacity + 1) * 1.7f;

                // Update capacity counter
                memfile_seek(&inserter->memfile, inserter->context.column->num_and_capacity_start_offset);
                memfile_skip_varuint(&inserter->memfile); // skip num element counter
                memfile_update_varuint(&inserter->memfile, new_capacity);
                inserter->context.column->column_capacity = new_capacity;

                size_t payload_start = bison_int_column_get_payload_off(inserter->context.column);
                memfile_seek(&inserter->memfile, payload_start + (num_elems-1) * type_size);
                memfile_ensure_space(&inserter->memfile, (new_capacity - capacity) * type_size);

                memfile_restore_position(&inserter->memfile);
        }

        size_t payload_start = bison_int_column_get_payload_off(inserter->context.column);
        memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
        memfile_write(&inserter->memfile, base, type_size);

        memfile_restore_position(&inserter->memfile);
        return true;
}

static bool push_media_type_for_array(struct bison_insert *inserter, enum bison_field_type type)
{
        memfile_ensure_space(&inserter->memfile, sizeof(media_type_t));
        return bison_media_write(&inserter->memfile, type);
}

static void internal_create(struct bison_insert *inserter, struct memfile *src)
{
        memfile_dup(&inserter->memfile, src);
        error_init(&inserter->err);
        inserter->position = memfile_tell(src);
}

static void write_binary_blob(struct bison_insert *inserter, const void *value, size_t nbytes)
{
        /* write blob length */
        memfile_write_varuint(&inserter->memfile, nbytes);

        /* write blob */
        memfile_ensure_space(&inserter->memfile, nbytes);
        memfile_write(&inserter->memfile, value, nbytes);
}