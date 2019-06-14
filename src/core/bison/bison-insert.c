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

#define check_type_if_container_is_column(inserter, expected)                                                          \
if (unlikely(inserter->context_type == BISON_COLUMN && inserter->context.column->type != expected)) {                  \
        error_with_details(&inserter->err, NG5_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool push_in_array(struct bison_insert *inserter, const void *base, u64 nbytes);
static bool push_in_column(struct bison_insert *inserter, const void *base, enum bison_field_type type);

static bool push_media_type_for_array(struct bison_insert *inserter, enum bison_field_type type);
static void internal_create(struct bison_insert *inserter, struct memfile *src);
static void write_binary_blob(struct bison_insert *inserter, const void *value, size_t nbytes);

NG5_EXPORT(bool) bison_insert_create_for_array(struct bison_insert *inserter, struct bison_array_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_array_it_lock(context);
        inserter->context_type = BISON_ARRAY;
        inserter->context.array = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_insert_create_for_column(struct bison_insert *inserter, struct bison_column_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_column_it_lock(context);
        inserter->context_type = BISON_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter)
{
        check_type_if_container_is_column(inserter, BISON_FIELD_TYPE_NULL);
        switch (inserter->context_type) {
        case BISON_ARRAY:
                return push_media_type_for_array(inserter, BISON_FIELD_TYPE_NULL);
        case BISON_COLUMN: {
                u8 media_value = BISON_FIELD_TYPE_NULL;
                return push_in_column(inserter, &media_value, BISON_FIELD_TYPE_NULL);
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
                u8 media_value = BISON_FIELD_TYPE_TRUE;
                return push_in_column(inserter, &media_value, BISON_FIELD_TYPE_TRUE);
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
                u8 media_value = BISON_FIELD_TYPE_FALSE;
                return push_in_column(inserter, &media_value, BISON_FIELD_TYPE_FALSE);
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_U8);
                push_in_array(inserter, &value, sizeof(u8));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_U16);
                push_in_array(inserter, &value, sizeof(u16));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_U32);
                push_in_array(inserter, &value, sizeof(u32));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_U64);
                push_in_array(inserter, &value, sizeof(u64));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_I8);
                push_in_array(inserter, &value, sizeof(i8));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_I16);
                push_in_array(inserter, &value, sizeof(i16));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_I32);
                push_in_array(inserter, &value, sizeof(i32));
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_I64);
                push_in_array(inserter, &value, sizeof(i64));
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

        if (value <= UINT8_MAX) {
                return bison_insert_u8(inserter, (u8) value);
        } else if (value <= UINT16_MAX) {
                return bison_insert_u16(inserter, (u16) value);
        } else if (value <= UINT32_MAX) {
                return bison_insert_u32(inserter, (u32) value);
        } else if (value <= UINT64_MAX) {
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

        if (value >= INT8_MIN && value <= INT8_MAX) {
                return bison_insert_i8(inserter, (i8) value);
        } else if (value >= INT16_MIN && value <= INT16_MAX) {
                return bison_insert_i16(inserter, (i16) value);
        } else if (value >= INT32_MIN && value <= INT32_MAX) {
                return bison_insert_i32(inserter, (i32) value);
        } else if (value >= INT64_MIN && value <= INT64_MAX) {
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
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_NUMBER_FLOAT);
                push_in_array(inserter, &value, sizeof(float));
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
        ng5_unused(inserter);
        ng5_unused(value);
        error_if(inserter->context_type != BISON_ARRAY, &inserter->err, NG5_ERR_UNSUPPCONTAINER);

        size_t value_strlen = strlen(value);
        push_media_type_for_array(inserter, BISON_FIELD_TYPE_STRING);
        bison_int_ensure_space(&inserter->memfile, varuint_sizeof(value_strlen) + value_strlen);
        varuint_t enc_len = (varuint_t) memfile_peek(&inserter->memfile, sizeof(varuint_t));
        u8 varuin_nbytes = varuint_write(enc_len, value_strlen);
        memfile_skip(&inserter->memfile, varuin_nbytes);
        memfile_write(&inserter->memfile, value, value_strlen);
        return false;
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
                u8 required_blocks = varuint_required_blocks(user_type_strlen);
                bison_int_ensure_space(&inserter->memfile, required_blocks + user_type_strlen);
                varuint_t user_type_strlen_data = (varuint_t) memfile_peek(&inserter->memfile, 1);
                varuint_write(user_type_strlen_data, user_type_strlen);
                memfile_skip(&inserter->memfile, required_blocks);

                /* write 'user_type' string */
                memfile_write(&inserter->memfile, user_type, user_type_strlen);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);

        } else {
                /* write media type 'binary' */
                push_media_type_for_array(inserter, BISON_FIELD_TYPE_BINARY);

                /* write mime type with variable-length integer type */
                u64 mime_type_id = bison_media_mime_type_by_ext(file_ext);
                u8 required_blocks = varuint_required_blocks(mime_type_id);

                bison_int_ensure_space(&inserter->memfile, required_blocks);

                varuint_t mime_type = (varuint_t) memfile_peek(&inserter->memfile, 1);
                varuint_write(mime_type, mime_type_id);
                memfile_skip(&inserter->memfile, required_blocks);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }

        return true;
}

NG5_EXPORT(struct bison_insert *) bison_insert_array_begin(struct bison_insert_array_state *state_out,
        struct bison_insert *inserter_in, u64 array_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if_and_return(!inserter_in, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if(inserter_in->context_type != BISON_ARRAY, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);

        *state_out = (struct bison_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = malloc(sizeof(struct bison_array_it))
        };

        bison_int_insert_array(&inserter_in->memfile, array_capacity);
        u64 payload_start = memfile_tell(&inserter_in->memfile) - 1;

        bison_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        bison_array_it_insert(&state_out->nested_inserter, state_out->nested_array);

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

        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile));
        bison_array_it_drop(&scan);
        bison_array_it_drop(state_in->nested_array);

        free(state_in->nested_array);
        bison_insert_drop(&state_in->nested_inserter);
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

        bison_int_insert_column(&inserter_in->memfile, &inserter_in->err, type, column_capacity);

        u64 payload_start = memfile_tell(&inserter_in->memfile) - sizeof(u8) - sizeof(media_type_t) - 2 * sizeof(u32);

        bison_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err, payload_start);
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

        free(state_in->nested_column);
        bison_insert_drop(&state_in->nested_inserter);
        return true;
}

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter)
{
        error_if_null(inserter)
        if (inserter->context_type == BISON_ARRAY) {
                bison_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == BISON_COLUMN) {
                bison_column_it_unlock(inserter->context.column);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
        }

        return true;
}

static bool push_in_array(struct bison_insert *inserter, const void *base, u64 nbytes)
{
        assert(inserter->context_type == BISON_ARRAY);

        bison_int_ensure_space(&inserter->memfile, nbytes);
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(struct bison_insert *inserter, const void *base, enum bison_field_type type)
{
        assert(inserter->context_type == BISON_COLUMN);

        size_t type_size = bison_int_get_type_value_size(type);

        memfile_save_position(&inserter->memfile);

        memfile_seek(&inserter->memfile, inserter->context.column->column_num_elements_offset);
        u32 num_elements = *NG5_MEMFILE_READ_TYPE(&inserter->memfile, u32);
        num_elements++;
        memfile_seek(&inserter->memfile, inserter->context.column->column_num_elements_offset);
        memfile_write(&inserter->memfile, &num_elements, sizeof(u32));

        memfile_seek(&inserter->memfile, inserter->context.column->column_capacity_offset);
        u32 capacity = *NG5_MEMFILE_READ_TYPE(&inserter->memfile, u32);

        memfile_restore_position(&inserter->memfile);

        if (unlikely(num_elements > capacity)) {
                memfile_save_position(&inserter->memfile);

                u32 new_capacity = (capacity + 1) * 1.7f;

                bison_int_ensure_space(&inserter->memfile, (new_capacity - capacity) * type_size);
                memfile_seek(&inserter->memfile, inserter->context.column->column_capacity_offset);
                memfile_write(&inserter->memfile, &new_capacity, sizeof(u32));

                memfile_restore_position(&inserter->memfile);
        }

        memfile_write(&inserter->memfile, base, type_size);
        return true;
}

static bool push_media_type_for_array(struct bison_insert *inserter, enum bison_field_type type)
{
        bison_int_ensure_space(&inserter->memfile, sizeof(media_type_t));
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
        /* write binary blob length with variable-length integer type */
        u8 required_blocks = varuint_required_blocks(nbytes);
        bison_int_ensure_space(&inserter->memfile, required_blocks + nbytes);

        varuint_t num_bytes = (varuint_t) memfile_peek(&inserter->memfile, 1);
        varuint_write(num_bytes, nbytes);
        memfile_skip(&inserter->memfile, required_blocks);

        /* write binary blob */
        memfile_write(&inserter->memfile, value, nbytes);
}