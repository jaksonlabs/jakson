/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in carbon
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
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-object-it.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-media.h>
#include <ark-js/carbon/carbon-int.h>

#define DEFINE_IN_PLACE_UPDATE_FUNCTION(type_name, field_type)                                                         \
ARK_EXPORT(bool) carbon_array_it_update_in_place_##type_name(struct carbon_array_it *it, type_name value)                \
{                                                                                                                      \
        offset_t datum;                                                                                                \
        error_if_null(it);                                                                                             \
        if (likely(it->field_access.it_field_type == field_type)) {                                                    \
                memfile_save_position(&it->memfile);                                                                   \
                carbon_int_array_it_offset(&datum, it);                                                                 \
                memfile_seek(&it->memfile, datum + sizeof(u8));                                                        \
                memfile_write(&it->memfile, &value, sizeof(type_name));                                                \
                memfile_restore_position(&it->memfile);                                                                \
                return true;                                                                                           \
        } else {                                                                                                       \
                error(&it->err, ARK_ERR_TYPEMISMATCH);                                                                 \
                return false;                                                                                          \
        }                                                                                                              \
}

DEFINE_IN_PLACE_UPDATE_FUNCTION(u8, carbon_FIELD_TYPE_NUMBER_U8)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u16, carbon_FIELD_TYPE_NUMBER_U16)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u32, carbon_FIELD_TYPE_NUMBER_U32)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u64, carbon_FIELD_TYPE_NUMBER_U64)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i8, carbon_FIELD_TYPE_NUMBER_I8)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i16, carbon_FIELD_TYPE_NUMBER_I16)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i32, carbon_FIELD_TYPE_NUMBER_I32)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i64, carbon_FIELD_TYPE_NUMBER_I64)
DEFINE_IN_PLACE_UPDATE_FUNCTION(float, carbon_FIELD_TYPE_NUMBER_FLOAT)

static bool update_in_place_constant(struct carbon_array_it *it, enum carbon_constant constant)
{
        error_if_null(it);

        memfile_save_position(&it->memfile);

        if (carbon_field_type_is_constant(it->field_access.it_field_type)) {
                u8 value;
                switch (constant) {
                case carbon_CONSTANT_TRUE:
                        value = carbon_FIELD_TYPE_TRUE;
                        break;
                case carbon_CONSTANT_FALSE:
                        value = carbon_FIELD_TYPE_FALSE;
                        break;
                case carbon_CONSTANT_NULL:
                        value = carbon_FIELD_TYPE_NULL;
                        break;
                default:
                error(&it->err, ARK_ERR_INTERNALERR);
                        break;
                }
                offset_t datum;
                carbon_int_array_it_offset(&datum, it);
                memfile_seek(&it->memfile, datum);
                memfile_write(&it->memfile, &value, sizeof(u8));
        } else {
                struct carbon_insert ins;
                carbon_array_it_remove(it);
                carbon_array_it_insert_begin(&ins, it);

                switch (constant) {
                case carbon_CONSTANT_TRUE:
                        carbon_insert_true(&ins);
                        break;
                case carbon_CONSTANT_FALSE:
                        carbon_insert_false(&ins);
                        break;
                case carbon_CONSTANT_NULL:
                        carbon_insert_null(&ins);
                        break;
                default:
                error(&it->err, ARK_ERR_INTERNALERR);
                        break;
                }

                carbon_array_it_insert_end(&ins);
        }

        memfile_restore_position(&it->memfile);
        return true;
}

ARK_EXPORT(bool) carbon_array_it_update_in_place_true(struct carbon_array_it *it)
{
        return update_in_place_constant(it, carbon_CONSTANT_TRUE);
}

ARK_EXPORT(bool) carbon_array_it_update_in_place_false(struct carbon_array_it *it)
{
        return update_in_place_constant(it, carbon_CONSTANT_FALSE);
}

ARK_EXPORT(bool) carbon_array_it_update_in_place_null(struct carbon_array_it *it)
{
        return update_in_place_constant(it, carbon_CONSTANT_NULL);
}

ARK_EXPORT(bool) carbon_array_it_create(struct carbon_array_it *it, struct memfile *memfile, struct err *err,
                                       offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->payload_start = payload_start;

        error_init(&it->err);
        spin_init(&it->lock);
        vec_create(&it->history, NULL, sizeof(offset_t), 40);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), err, ARK_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != carbon_MARKER_ARRAY_BEGIN, err, ARK_ERR_ILLEGALOP,
                "array begin marker ('[') not found");

        it->payload_start += sizeof(u8);

        carbon_int_field_access_create(&it->field_access);

        carbon_array_it_rewind(it);

        return true;
}

ARK_EXPORT(bool) carbon_array_it_copy(struct carbon_array_it *dst, struct carbon_array_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        carbon_array_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(u8));
        return true;
}

ARK_EXPORT(bool) carbon_array_it_readonly(struct carbon_array_it *it)
{
        error_if_null(it);
        it->memfile.mode = READ_ONLY;
        return true;
}

ARK_EXPORT(bool) carbon_array_it_drop(struct carbon_array_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        carbon_int_field_access_drop(&it->field_access);
        vec_drop(&it->history);
        return true;
}

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_array_it_unlock</code> is required for unlocking.
 */
ARK_EXPORT(bool) carbon_array_it_lock(struct carbon_array_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
ARK_EXPORT(bool) carbon_array_it_unlock(struct carbon_array_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

ARK_EXPORT(bool) carbon_array_it_rewind(struct carbon_array_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, ARK_ERR_OUTOFBOUNDS);
        carbon_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->payload_start);
}

ARK_EXPORT(bool) carbon_array_it_next(struct carbon_array_it *it)
{
        error_if_null(it);
        bool is_empty_slot, is_array_end;
        offset_t last_off = memfile_tell(&it->memfile);
        if (carbon_int_array_it_next(&is_empty_slot, &is_array_end, it)) {
                carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!is_array_end) {
                        error_if(!is_empty_slot, &it->err, ARK_ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }
                char final = *memfile_peek(&it->memfile, sizeof(char));
                assert( final == carbon_MARKER_ARRAY_END);
                carbon_int_field_auto_close(&it->field_access);
                return false;
        }
}

ARK_EXPORT(bool) carbon_array_it_prev(struct carbon_array_it *it)
{
        error_if_null(it);
        if (carbon_int_history_has(&it->history)) {
                offset_t prev_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                return carbon_int_array_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

ARK_EXPORT(offset_t) carbon_array_it_tell(struct carbon_array_it *it)
{
        if (likely(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, ARK_ERR_NULLPTR);
                return 0;
        }
}

ARK_EXPORT(bool) carbon_int_array_it_offset(offset_t *off, struct carbon_array_it *it)
{
        error_if_null(off)
        error_if_null(it)
        if (carbon_int_history_has(&it->history)) {
                *off = carbon_int_history_peek(&it->history);
                return true;
        }
        return false;
}

ARK_EXPORT(bool) carbon_array_it_fast_forward(struct carbon_array_it *it)
{
        error_if_null(it);
        while (carbon_array_it_next(it))
                { }
        char last = *memfile_peek(&it->memfile, sizeof(char));
        assert(last == carbon_MARKER_ARRAY_END);
        memfile_skip(&it->memfile, sizeof(char));
        return true;
}

ARK_EXPORT(bool) carbon_array_it_field_type(enum carbon_field_type *type, struct carbon_array_it *it)
{
        return carbon_int_field_access_field_type(type, &it->field_access);
}

ARK_EXPORT(bool) carbon_array_it_u8_value(u8 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_u8_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_u16_value(u16 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_u16_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_u32_value(u32 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_u32_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_u64_value(u64 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_u64_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_i8_value(i8 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_i8_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_i16_value(i16 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_i16_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_i32_value(i32 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_i32_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_i64_value(i64 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_i64_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_float_value(bool *is_null_in, float *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_signed_value(bool *is_null_in, i64 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_unsigned_value(bool *is_null_in, u64 *value, struct carbon_array_it *it)
{
        return carbon_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(const char *) carbon_array_it_string_value(u64 *strlen, struct carbon_array_it *it)
{
        return carbon_int_field_access_string_value(strlen, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_array_it_binary_value(struct carbon_binary *out, struct carbon_array_it *it)
{
        return carbon_int_field_access_binary_value(out, &it->field_access, &it->err);
}

ARK_EXPORT(struct carbon_array_it *) carbon_array_it_array_value(struct carbon_array_it *it_in)
{
        return carbon_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(struct carbon_object_it *) carbon_array_it_object_value(struct carbon_array_it *it_in)
{
        return carbon_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(struct carbon_column_it *) carbon_array_it_column_value(struct carbon_array_it *it_in)
{
        return carbon_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(bool) carbon_array_it_insert_begin(struct carbon_insert *inserter, struct carbon_array_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return carbon_int_insert_create_for_array(inserter, it);
}

ARK_EXPORT(bool) carbon_array_it_insert_end(struct carbon_insert *inserter)
{
        error_if_null(inserter)
        return carbon_insert_drop(inserter);
}

static bool remove_field(struct memfile *memfile, struct err *err, enum carbon_field_type type)
{
        assert((enum carbon_field_type) *memfile_peek(memfile, sizeof(u8)) == type);
        offset_t start_off = memfile_tell(memfile);
        memfile_skip(memfile, sizeof(u8));
        size_t rm_nbytes = sizeof(u8); /* at least the type marker must be removed */
        switch (type) {
                case carbon_FIELD_TYPE_NULL:
                case carbon_FIELD_TYPE_TRUE:
                case carbon_FIELD_TYPE_FALSE:
                        /* nothing to do */
                        break;
                case carbon_FIELD_TYPE_NUMBER_U8:
                case carbon_FIELD_TYPE_NUMBER_I8:
                        rm_nbytes += sizeof(u8);
                        break;
                case carbon_FIELD_TYPE_NUMBER_U16:
                case carbon_FIELD_TYPE_NUMBER_I16:
                        rm_nbytes += sizeof(u16);
                        break;
                case carbon_FIELD_TYPE_NUMBER_U32:
                case carbon_FIELD_TYPE_NUMBER_I32:
                        rm_nbytes += sizeof(u32);
                        break;
                case carbon_FIELD_TYPE_NUMBER_U64:
                case carbon_FIELD_TYPE_NUMBER_I64:
                        rm_nbytes += sizeof(u64);
                        break;
                case carbon_FIELD_TYPE_NUMBER_FLOAT:
                        rm_nbytes += sizeof(float);
                        break;
                case carbon_FIELD_TYPE_STRING: {
                        u8 len_nbytes;  /* number of bytes used to store string length */
                        u64 str_len; /* the number of characters of the string field */

                        str_len = memfile_read_varuint(&len_nbytes, memfile);

                        rm_nbytes += len_nbytes + str_len;
                } break;
                case carbon_FIELD_TYPE_BINARY: {
                        u8 mime_type_nbytes; /* number of bytes for mime type */
                        u8 blob_length_nbytes; /* number of bytes to store blob length */
                        u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes used for mime type id */
                        memfile_read_varuint(&mime_type_nbytes, memfile);

                        /* get bytes used for blob length info */
                        blob_nbytes = memfile_read_varuint(&blob_length_nbytes, memfile);

                        rm_nbytes += mime_type_nbytes + blob_length_nbytes + blob_nbytes;
                } break;
                case carbon_FIELD_TYPE_BINARY_CUSTOM: {
                        u8 custom_type_strlen_nbytes; /* number of bytes for type name string length info */
                        u8 custom_type_strlen; /* number of characters to encode type name string */
                        u8 blob_length_nbytes; /* number of bytes to store blob length */
                        u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes for custom type string len, and the actual length */
                        custom_type_strlen = memfile_read_varuint(&custom_type_strlen_nbytes, memfile);
                        memfile_skip(memfile, custom_type_strlen);

                        /* get bytes used for blob length info */
                        blob_nbytes = memfile_read_varuint(&blob_length_nbytes, memfile);

                        rm_nbytes += custom_type_strlen_nbytes + custom_type_strlen + blob_length_nbytes + blob_nbytes;
                } break;
                case carbon_FIELD_TYPE_ARRAY: {
                        struct carbon_array_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_array_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_array_it_fast_forward(&it);
                        offset_t end_off = carbon_array_it_tell(&it);
                        carbon_array_it_drop(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                } break;
                case carbon_FIELD_TYPE_COLUMN_U8:
                case carbon_FIELD_TYPE_COLUMN_U16:
                case carbon_FIELD_TYPE_COLUMN_U32:
                case carbon_FIELD_TYPE_COLUMN_U64:
                case carbon_FIELD_TYPE_COLUMN_I8:
                case carbon_FIELD_TYPE_COLUMN_I16:
                case carbon_FIELD_TYPE_COLUMN_I32:
                case carbon_FIELD_TYPE_COLUMN_I64:
                case carbon_FIELD_TYPE_COLUMN_FLOAT:
                case carbon_FIELD_TYPE_COLUMN_BOOLEAN: {
                        struct carbon_column_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_column_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_column_it_fast_forward(&it);
                        offset_t end_off = carbon_column_it_tell(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                } break;
                case carbon_FIELD_TYPE_OBJECT:
                        print_error_and_die(ARK_ERR_NOTIMPLEMENTED)
                        break;
                default:
                        error(err, ARK_ERR_INTERNALERR)
                        return false;
        }
        memfile_seek(memfile, start_off);
        memfile_move_left(memfile, rm_nbytes);

        return true;
}

ARK_EXPORT(bool) carbon_array_it_remove(struct carbon_array_it *it)
{
        unused(it);
        error_if_null(it);
        enum carbon_field_type type;
        if (carbon_array_it_field_type(&type, it)) {
                offset_t prev_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                if (remove_field(&it->memfile, &it->err, type)) {
                        memfile_seek(&it->memfile, prev_off);
                        carbon_int_array_it_refresh(NULL, NULL, it);
                        return true;
                } else {
                        return false;
                }
        } else {
                error(&it->err, ARK_ERR_ILLEGALSTATE);
                return false;
        }
}