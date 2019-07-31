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

#include <ark-js/shared/stdx/varuint.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-media.h>
#include <ark-js/carbon/carbon-int.h>
#include <ark-js/carbon/carbon-string.h>
#include <ark-js/carbon/carbon-object-it.h>

#define check_type_if_container_is_column(inserter, expected)                                                          \
if (unlikely(inserter->context_type == carbon_COLUMN && inserter->context.column->type != expected)) {                  \
        error_with_details(&inserter->err, NG5_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

#define check_type_range_if_container_is_column(inserter, expected1, expected2, expected3)                             \
if (unlikely(inserter->context_type == carbon_COLUMN && inserter->context.column->type != expected1 &&                  \
        inserter->context.column->type != expected2 && inserter->context.column->type != expected3)) {                 \
        error_with_details(&inserter->err, NG5_ERR_TYPEMISMATCH, "Element type does not match container type");        \
}

static bool write_field_data(struct carbon_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes);
static bool push_in_column(struct carbon_insert *inserter, const void *base, enum carbon_field_type type);

static bool push_media_type_for_array(struct carbon_insert *inserter, enum carbon_field_type type);
static void internal_create(struct carbon_insert *inserter, struct memfile *src);
static void write_binary_blob(struct carbon_insert *inserter, const void *value, size_t nbytes);

NG5_EXPORT(bool) carbon_int_insert_create_for_array(struct carbon_insert *inserter, struct carbon_array_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        carbon_array_it_lock(context);
        inserter->context_type = carbon_ARRAY;
        inserter->context.array = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) carbon_int_insert_create_for_column(struct carbon_insert *inserter, struct carbon_column_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        carbon_column_it_lock(context);
        inserter->context_type = carbon_COLUMN;
        inserter->context.column = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) carbon_int_insert_create_for_object(struct carbon_insert *inserter, struct carbon_object_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        carbon_object_it_lock(context);
        inserter->context_type = carbon_OBJECT;
        inserter->context.object = context;
        internal_create(inserter, &context->memfile);
        return true;
}

NG5_EXPORT(bool) carbon_insert_null(struct carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_BOOLEAN);

        switch (inserter->context_type) {
        case carbon_ARRAY:
                return push_media_type_for_array(inserter, carbon_FIELD_TYPE_NULL);
        case carbon_COLUMN: {
                switch (inserter->context.column->type) {
                        case carbon_FIELD_TYPE_COLUMN_U8: {
                                u8 value = U8_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_U16: {
                                u16 value = U16_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_U32: {
                                u32 value = U32_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_U64: {
                                u64 value = U64_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_I8: {
                                i8 value = I8_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_I16: {
                                i16 value = I16_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_I32: {
                                i32 value = I32_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_I64: {
                                i64 value = I64_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_FLOAT: {
                                float value = FLOAT_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        case carbon_FIELD_TYPE_COLUMN_BOOLEAN: {
                                u8 value = carbon_BOOLEAN_COLUMN_NULL;
                                return push_in_column(inserter, &value, inserter->context.column->type);
                        } break;
                        default:
                                error(&inserter->err, NG5_ERR_INTERNALERR)
                                return false;
                }
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) carbon_insert_true(struct carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                return push_media_type_for_array(inserter, carbon_FIELD_TYPE_TRUE);
        case carbon_COLUMN: {
                u8 value = carbon_BOOLEAN_COLUMN_TRUE;
                return push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_BOOLEAN);
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) carbon_insert_false(struct carbon_insert *inserter)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_BOOLEAN);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                return push_media_type_for_array(inserter, carbon_FIELD_TYPE_FALSE);
        case carbon_COLUMN: {
                u8 value = carbon_BOOLEAN_COLUMN_FALSE;
                return push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_BOOLEAN);
        }
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
}

NG5_EXPORT(bool) carbon_insert_u8(struct carbon_insert *inserter, u8 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_U8);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U8, &value, sizeof(u8));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_U8);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_u16(struct carbon_insert *inserter, u16 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_U16);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U16, &value, sizeof(u16));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_U16);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_u32(struct carbon_insert *inserter, u32 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_U32);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U32, &value, sizeof(u32));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_U32);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_u64(struct carbon_insert *inserter, u64 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_U64);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U64, &value, sizeof(u64));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value,carbon_FIELD_TYPE_COLUMN_U64);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_i8(struct carbon_insert *inserter, i8 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_I8);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I8, &value, sizeof(i8));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_I8);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_i16(struct carbon_insert *inserter, i16 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_I16);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I16, &value, sizeof(i16));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_I16);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_i32(struct carbon_insert *inserter, i32 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_I32);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I32, &value, sizeof(i32));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_I32);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_i64(struct carbon_insert *inserter, i64 value)
{
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_I64);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I64, &value, sizeof(i64));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_I64);
                break;
        default:
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_unsigned(struct carbon_insert *inserter, u64 value)
{
        error_if(inserter->context_type == carbon_COLUMN, &inserter->err, NG5_ERR_INSERT_TOO_DANGEROUS)

        if (value <= carbon_U8_MAX) {
                return carbon_insert_u8(inserter, (u8) value);
        } else if (value <= carbon_U16_MAX) {
                return carbon_insert_u16(inserter, (u16) value);
        } else if (value <= carbon_U32_MAX) {
                return carbon_insert_u32(inserter, (u32) value);
        } else if (value <= carbon_U64_MAX) {
                return carbon_insert_u64(inserter, (u64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_signed(struct carbon_insert *inserter, i64 value)
{
        error_if(inserter->context_type == carbon_COLUMN, &inserter->err, NG5_ERR_INSERT_TOO_DANGEROUS)

        if (value >= carbon_I8_MIN && value <= carbon_I8_MAX) {
                return carbon_insert_i8(inserter, (i8) value);
        } else if (value >= carbon_I16_MIN && value <= carbon_I16_MAX) {
                return carbon_insert_i16(inserter, (i16) value);
        } else if (value >= carbon_I32_MIN && value <= carbon_I32_MAX) {
                return carbon_insert_i32(inserter, (i32) value);
        } else if (value >= carbon_I64_MIN && value <= carbon_I64_MAX) {
                return carbon_insert_i64(inserter, (i64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_float(struct carbon_insert *inserter, float value)
{
        error_if_null(inserter)
        check_type_if_container_is_column(inserter, carbon_FIELD_TYPE_COLUMN_FLOAT);
        switch (inserter->context_type) {
        case carbon_ARRAY:
                write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
                break;
        case carbon_COLUMN:
                push_in_column(inserter, &value, carbon_FIELD_TYPE_COLUMN_FLOAT);
                break;
        default:
        error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_string(struct carbon_insert *inserter, const char *value)
{
        unused(inserter);
        unused(value);
        error_if(inserter->context_type != carbon_ARRAY, &inserter->err, NG5_ERR_UNSUPPCONTAINER);

        return carbon_string_write(&inserter->memfile, value);
}

static void insert_binary(struct carbon_insert *inserter, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type)
{
        if (user_type && strlen(user_type) > 0) {
                /* write media type 'user binary' */
                push_media_type_for_array(inserter, carbon_FIELD_TYPE_BINARY_CUSTOM);

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
                push_media_type_for_array(inserter, carbon_FIELD_TYPE_BINARY);

                /* write mime type with variable-length integer type */
                u64 mime_type_id = carbon_media_mime_type_by_ext(file_ext);

                /* write mime type id */
                memfile_write_varuint(&inserter->memfile, mime_type_id);

                /* write binary blob */
                write_binary_blob(inserter, value, nbytes);
        }
}

NG5_EXPORT(bool) carbon_insert_binary(struct carbon_insert *inserter, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type)
{
        error_if_null(inserter)
        error_if_null(value)
        error_if(inserter->context_type != carbon_ARRAY, &inserter->err, NG5_ERR_UNSUPPCONTAINER);

        insert_binary(inserter, value, nbytes, file_ext, user_type);

        return true;
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_object_begin(struct carbon_insert_object_state *out,
        struct carbon_insert *inserter, u64 object_capacity)
{
        error_if_null(out)
        error_if_null(inserter)

        error_if_and_return(!out, &inserter->err, NG5_ERR_NULLPTR, NULL);
        if (!inserter) {
                error_print(NG5_ERR_NULLPTR);
                return false;
        }

        *out = (struct carbon_insert_object_state) {
                .parent_inserter = inserter,
                .it = malloc(sizeof(struct carbon_object_it))
        };

        carbon_int_insert_object(&inserter->memfile, object_capacity);
        u64 payload_start = memfile_tell(&inserter->memfile) - 1;

        carbon_object_it_create(out->it, &inserter->memfile, &inserter->err, payload_start);
        carbon_object_it_insert_begin(&out->inserter, out->it);

        return &out->inserter;
}

NG5_EXPORT(bool) carbon_insert_object_end(struct carbon_insert_object_state *state)
{
        error_if_null(state);

        struct carbon_object_it scan;
        carbon_object_it_create(&scan, &state->parent_inserter->memfile, &state->parent_inserter->err,
                memfile_tell(&state->parent_inserter->memfile) - 1);
        while (carbon_object_it_next(&scan))
        { }

        char final = *memfile_read(&scan.memfile, sizeof(char));
        assert(final == carbon_MARKER_OBJECT_END);
        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state->parent_inserter->memfile, memfile_tell(&scan.memfile) - 1);
        carbon_object_it_drop(&scan);
        carbon_insert_drop(&state->inserter);
        carbon_object_it_drop(state->it);
        free(state->it);
        return true;
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_array_begin(struct carbon_insert_array_state *state_out,
        struct carbon_insert *inserter_in, u64 array_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        if (!inserter_in) {
                error_print(NG5_ERR_NULLPTR);
                return false;
        }

        error_if(inserter_in->context_type != carbon_ARRAY && inserter_in->context_type != carbon_OBJECT, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);

        *state_out = (struct carbon_insert_array_state) {
                .parent_inserter = inserter_in,
                .nested_array = malloc(sizeof(struct carbon_array_it))
        };

        carbon_int_insert_array(&inserter_in->memfile, array_capacity);
        u64 payload_start = memfile_tell(&inserter_in->memfile) - 1;

        carbon_array_it_create(state_out->nested_array, &inserter_in->memfile, &inserter_in->err, payload_start);
        carbon_array_it_insert_begin(&state_out->nested_inserter, state_out->nested_array);

        return &state_out->nested_inserter;
}

NG5_EXPORT(bool) carbon_insert_array_end(struct carbon_insert_array_state *state_in)
{
        error_if_null(state_in);

        struct carbon_array_it scan;
        carbon_array_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                memfile_tell(&state_in->parent_inserter->memfile) - 1);
        while (carbon_array_it_next(&scan))
                { }

        char final = *memfile_read(&scan.memfile, sizeof(char));
        assert(final == carbon_MARKER_ARRAY_END);
        memfile_skip(&scan.memfile, 1);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile) - 1);
        carbon_array_it_drop(&scan);
        carbon_insert_drop(&state_in->nested_inserter);
        carbon_array_it_drop(state_in->nested_array);
        free(state_in->nested_array);
        return true;
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_column_begin(struct carbon_insert_column_state *state_out,
        struct carbon_insert *inserter_in, enum carbon_column_type type, u64 column_capacity)
{
        error_if_and_return(!state_out, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if_and_return(!inserter_in, &inserter_in->err, NG5_ERR_NULLPTR, NULL);
        error_if(inserter_in->context_type != carbon_ARRAY && inserter_in->context_type != carbon_OBJECT, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);

        enum carbon_field_type field_type = carbon_field_type_for_column(type);

        *state_out = (struct carbon_insert_column_state) {
                .parent_inserter = inserter_in,
                .nested_column = malloc(sizeof(struct carbon_column_it)),
                .type = field_type
        };

        u64 container_start_off = memfile_tell(&inserter_in->memfile);
        carbon_int_insert_column(&inserter_in->memfile, &inserter_in->err, type, column_capacity);

        carbon_column_it_create(state_out->nested_column, &inserter_in->memfile, &inserter_in->err, container_start_off);
        carbon_column_it_insert(&state_out->nested_inserter, state_out->nested_column);

        return &state_out->nested_inserter;
}

NG5_EXPORT(bool) carbon_insert_column_end(struct carbon_insert_column_state *state_in)
{
        error_if_null(state_in);

        struct carbon_column_it scan;
        carbon_column_it_create(&scan, &state_in->parent_inserter->memfile, &state_in->parent_inserter->err,
                state_in->nested_column->column_start_offset);
        carbon_column_it_fast_forward(&scan);

        memfile_seek(&state_in->parent_inserter->memfile, memfile_tell(&scan.memfile));

        carbon_insert_drop(&state_in->nested_inserter);
        free(state_in->nested_column);
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_null(struct carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, carbon_FIELD_TYPE_NULL);
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_true(struct carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, carbon_FIELD_TYPE_TRUE);
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_false(struct carbon_insert *inserter, const char *key)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        push_media_type_for_array(inserter, carbon_FIELD_TYPE_FALSE);
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_u8(struct carbon_insert *inserter, const char *key, u8 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U8, &value, sizeof(u8));
}

NG5_EXPORT(bool) carbon_insert_prop_u16(struct carbon_insert *inserter, const char *key, u16 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U16, &value, sizeof(u16));
}

NG5_EXPORT(bool) carbon_insert_prop_u32(struct carbon_insert *inserter, const char *key, u32 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U32, &value, sizeof(u32));
}

NG5_EXPORT(bool) carbon_insert_prop_u64(struct carbon_insert *inserter, const char *key, u64 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_U64, &value, sizeof(u64));
}

NG5_EXPORT(bool) carbon_insert_prop_i8(struct carbon_insert *inserter, const char *key, i8 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I8, &value, sizeof(i8));
}

NG5_EXPORT(bool) carbon_insert_prop_i16(struct carbon_insert *inserter, const char *key, i16 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I16, &value, sizeof(i16));
}

NG5_EXPORT(bool) carbon_insert_prop_i32(struct carbon_insert *inserter, const char *key, i32 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I32, &value, sizeof(i32));
}

NG5_EXPORT(bool) carbon_insert_prop_i64(struct carbon_insert *inserter, const char *key, i64 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_I64, &value, sizeof(i64));
}

NG5_EXPORT(bool) carbon_insert_prop_unsigned(struct carbon_insert *inserter, const char *key, u64 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER)

        if (value <= carbon_U8_MAX) {
                return carbon_insert_prop_u8(inserter, key, (u8) value);
        } else if (value <= carbon_U16_MAX) {
                return carbon_insert_prop_u16(inserter, key, (u16) value);
        } else if (value <= carbon_U32_MAX) {
                return carbon_insert_prop_u32(inserter, key, (u32) value);
        } else if (value <= carbon_U64_MAX) {
                return carbon_insert_prop_u64(inserter, key, (u64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_signed(struct carbon_insert *inserter, const char *key, i64 value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER)

        if (value >= carbon_I8_MIN && value <= carbon_I8_MAX) {
                return carbon_insert_prop_i8(inserter, key, (i8) value);
        } else if (value >= carbon_I16_MIN && value <= carbon_I16_MAX) {
                return carbon_insert_prop_i16(inserter, key, (i16) value);
        } else if (value >= carbon_I32_MIN && value <= carbon_I32_MAX) {
                return carbon_insert_prop_i32(inserter, key, (i32) value);
        } else if (value >= carbon_I64_MIN && value <= carbon_I64_MAX) {
                return carbon_insert_prop_i64(inserter, key, (i64) value);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) carbon_insert_prop_float(struct carbon_insert *inserter, const char *key, float value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return write_field_data(inserter, carbon_FIELD_TYPE_NUMBER_FLOAT, &value, sizeof(float));
}

NG5_EXPORT(bool) carbon_insert_prop_string(struct carbon_insert *inserter, const char *key, const char *value)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_string_write(&inserter->memfile, value);
}

NG5_EXPORT(bool) carbon_insert_prop_binary(struct carbon_insert *inserter, const char *key, const void *value,
        size_t nbytes, const char *file_ext, const char *user_type)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        insert_binary(inserter, value, nbytes, file_ext, user_type);
        return true;
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_prop_object_begin(struct carbon_insert_object_state *out,
        struct carbon_insert *inserter, const char *key, u64 object_capacity)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_object_begin(out, inserter, object_capacity);
}

NG5_EXPORT(bool) carbon_insert_prop_object_end(struct carbon_insert_object_state *state)
{
        return carbon_insert_object_end(state);
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_prop_array_begin(struct carbon_insert_array_state *state,
        struct carbon_insert *inserter, const char *key, u64 array_capacity)
{
        error_if(inserter->context_type != carbon_OBJECT, &inserter->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter->memfile, key);
        return carbon_insert_array_begin(state, inserter, array_capacity);
}

NG5_EXPORT(bool) carbon_insert_prop_array_end(struct carbon_insert_array_state *state)
{
        return carbon_insert_array_end(state);
}

NG5_EXPORT(struct carbon_insert *) carbon_insert_prop_column_begin(struct carbon_insert_column_state *state_out,
        struct carbon_insert *inserter_in, const char *key, enum carbon_column_type type, u64 column_capacity)
{
        error_if(inserter_in->context_type != carbon_OBJECT, &inserter_in->err, NG5_ERR_UNSUPPCONTAINER);
        carbon_string_nomarker_write(&inserter_in->memfile, key);
        return carbon_insert_column_begin(state_out, inserter_in, type, column_capacity);
}

NG5_EXPORT(bool) carbon_insert_prop_column_end(struct carbon_insert_column_state *state_in)
{
        return carbon_insert_column_end(state_in);
}

NG5_EXPORT(bool) carbon_insert_drop(struct carbon_insert *inserter)
{
        error_if_null(inserter)
        if (inserter->context_type == carbon_ARRAY) {
                carbon_array_it_unlock(inserter->context.array);
        } else if (inserter->context_type == carbon_COLUMN) {
                carbon_column_it_unlock(inserter->context.column);
        } else if (inserter->context_type == carbon_OBJECT) {
                carbon_object_it_unlock(inserter->context.object);
        } else {
                error(&inserter->err, NG5_ERR_INTERNALERR);
        }

        return true;
}

static bool write_field_data(struct carbon_insert *inserter, u8 field_type_marker, const void *base, u64 nbytes)
{
        assert(inserter->context_type == carbon_ARRAY || inserter->context_type == carbon_OBJECT);

        memfile_ensure_space(&inserter->memfile, sizeof(u8) + nbytes);
        memfile_write(&inserter->memfile, &field_type_marker, sizeof(u8));
        return memfile_write(&inserter->memfile, base, nbytes);
}

static bool push_in_column(struct carbon_insert *inserter, const void *base, enum carbon_field_type type)
{
        assert(inserter->context_type == carbon_COLUMN);

        size_t type_size = carbon_int_get_type_value_size(type);

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

                size_t payload_start = carbon_int_column_get_payload_off(inserter->context.column);
                memfile_seek(&inserter->memfile, payload_start + (num_elems-1) * type_size);
                memfile_ensure_space(&inserter->memfile, (new_capacity - capacity) * type_size);

                memfile_restore_position(&inserter->memfile);
        }

        size_t payload_start = carbon_int_column_get_payload_off(inserter->context.column);
        memfile_seek(&inserter->memfile, payload_start + (num_elems - 1) * type_size);
        memfile_write(&inserter->memfile, base, type_size);

        memfile_restore_position(&inserter->memfile);
        return true;
}

static bool push_media_type_for_array(struct carbon_insert *inserter, enum carbon_field_type type)
{
        memfile_ensure_space(&inserter->memfile, sizeof(media_type_t));
        return carbon_media_write(&inserter->memfile, type);
}

static void internal_create(struct carbon_insert *inserter, struct memfile *src)
{
        memfile_dup(&inserter->memfile, src);
        error_init(&inserter->err);
        inserter->position = memfile_tell(src);
}

static void write_binary_blob(struct carbon_insert *inserter, const void *value, size_t nbytes)
{
        /* write blob length */
        memfile_write_varuint(&inserter->memfile, nbytes);

        /* write blob */
        memfile_ensure_space(&inserter->memfile, nbytes);
        memfile_write(&inserter->memfile, value, nbytes);
}