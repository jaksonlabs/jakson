/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in BISON
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
#include "core/bison/bison.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-object-it.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-int.h"

#define DEFINE_IN_PLACE_UPDATE_FUNCTION(type_name, field_type)                                                         \
NG5_EXPORT(bool) bison_array_it_update_in_place_##type_name(struct bison_array_it *it, type_name value)                \
{                                                                                                                      \
        offset_t datum;                                                                                                \
        error_if_null(it);                                                                                             \
        if (likely(it->field_access.it_field_type == field_type)) {                                                    \
                memfile_save_position(&it->memfile);                                                                   \
                bison_int_array_it_offset(&datum, it);                                                                 \
                memfile_seek(&it->memfile, datum + sizeof(u8));                                                        \
                memfile_write(&it->memfile, &value, sizeof(type_name));                                                \
                memfile_restore_position(&it->memfile);                                                                \
                return true;                                                                                           \
        } else {                                                                                                       \
                error(&it->err, NG5_ERR_TYPEMISMATCH);                                                                 \
                return false;                                                                                          \
        }                                                                                                              \
}

DEFINE_IN_PLACE_UPDATE_FUNCTION(u8, BISON_FIELD_TYPE_NUMBER_U8)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u16, BISON_FIELD_TYPE_NUMBER_U16)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u32, BISON_FIELD_TYPE_NUMBER_U32)
DEFINE_IN_PLACE_UPDATE_FUNCTION(u64, BISON_FIELD_TYPE_NUMBER_U64)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i8, BISON_FIELD_TYPE_NUMBER_I8)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i16, BISON_FIELD_TYPE_NUMBER_I16)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i32, BISON_FIELD_TYPE_NUMBER_I32)
DEFINE_IN_PLACE_UPDATE_FUNCTION(i64, BISON_FIELD_TYPE_NUMBER_I64)
DEFINE_IN_PLACE_UPDATE_FUNCTION(float, BISON_FIELD_TYPE_NUMBER_FLOAT)

static bool update_in_place_constant(struct bison_array_it *it, enum bison_constant constant)
{
        error_if_null(it);

        memfile_save_position(&it->memfile);

        if (bison_field_type_is_constant(it->field_access.it_field_type)) {
                u8 value;
                switch (constant) {
                case BISON_CONSTANT_TRUE:
                        value = BISON_FIELD_TYPE_TRUE;
                        break;
                case BISON_CONSTANT_FALSE:
                        value = BISON_FIELD_TYPE_FALSE;
                        break;
                case BISON_CONSTANT_NULL:
                        value = BISON_FIELD_TYPE_NULL;
                        break;
                default:
                error(&it->err, NG5_ERR_INTERNALERR);
                        break;
                }
                offset_t datum;
                bison_int_array_it_offset(&datum, it);
                memfile_seek(&it->memfile, datum);
                memfile_write(&it->memfile, &value, sizeof(u8));
        } else {
                struct bison_insert ins;
                bison_array_it_remove(it);
                bison_array_it_insert_begin(&ins, it);

                switch (constant) {
                case BISON_CONSTANT_TRUE:
                        bison_insert_true(&ins);
                        break;
                case BISON_CONSTANT_FALSE:
                        bison_insert_false(&ins);
                        break;
                case BISON_CONSTANT_NULL:
                        bison_insert_null(&ins);
                        break;
                default:
                error(&it->err, NG5_ERR_INTERNALERR);
                        break;
                }

                bison_array_it_insert_end(&ins);
        }

        memfile_restore_position(&it->memfile);
        return true;
}

NG5_EXPORT(bool) bison_array_it_update_in_place_true(struct bison_array_it *it)
{
        return update_in_place_constant(it, BISON_CONSTANT_TRUE);
}

NG5_EXPORT(bool) bison_array_it_update_in_place_false(struct bison_array_it *it)
{
        return update_in_place_constant(it, BISON_CONSTANT_FALSE);
}

NG5_EXPORT(bool) bison_array_it_update_in_place_null(struct bison_array_it *it)
{
        return update_in_place_constant(it, BISON_CONSTANT_NULL);
}

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct memfile *memfile, struct err *err,
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

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_ARRAY_BEGIN, err, NG5_ERR_ILLEGALOP,
                "array begin marker ('[') not found");

        it->payload_start += sizeof(u8);

        bison_int_field_access_create(&it->field_access);

        bison_array_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_array_it_copy(struct bison_array_it *dst, struct bison_array_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        bison_array_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(u8));
        return true;
}

NG5_EXPORT(bool) bison_array_it_readonly(struct bison_array_it *it)
{
        error_if_null(it);
        it->memfile.mode = READ_ONLY;
        return true;
}

NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it)
{
        bison_int_field_auto_close(&it->field_access);
        bison_int_field_access_drop(&it->field_access);
        vec_drop(&it->history);
        return true;
}

/**
 * Locks the iterator with a spinlock. A call to <code>bison_array_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_array_it_lock(struct bison_array_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_array_it_unlock(struct bison_array_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_array_it_rewind(struct bison_array_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        bison_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it)
{
        error_if_null(it);
        bool is_empty_slot, is_array_end;
        offset_t last_off = memfile_tell(&it->memfile);
        if (bison_int_array_it_next(&is_empty_slot, &is_array_end, it)) {
                bison_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!is_array_end) {
                        error_if(!is_empty_slot, &it->err, NG5_ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }
                char final = *memfile_peek(&it->memfile, sizeof(char));
                assert( final == BISON_MARKER_ARRAY_END);
                bison_int_field_auto_close(&it->field_access);
                return false;
        }
}

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it)
{
        error_if_null(it);
        if (bison_int_history_has(&it->history)) {
                offset_t prev_off = bison_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                return bison_int_array_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

NG5_EXPORT(offset_t) bison_array_it_tell(struct bison_array_it *it)
{
        if (likely(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, NG5_ERR_NULLPTR);
                return 0;
        }
}

NG5_EXPORT(bool) bison_int_array_it_offset(offset_t *off, struct bison_array_it *it)
{
        error_if_null(off)
        error_if_null(it)
        if (bison_int_history_has(&it->history)) {
                *off = bison_int_history_peek(&it->history);
                return true;
        }
        return false;
}

NG5_EXPORT(bool) bison_array_it_fast_forward(struct bison_array_it *it)
{
        error_if_null(it);
        while (bison_array_it_next(it))
                { }
        char last = *memfile_peek(&it->memfile, sizeof(char));
        assert(last == BISON_MARKER_ARRAY_END);
        memfile_skip(&it->memfile, sizeof(char));
        return true;
}

NG5_EXPORT(bool) bison_array_it_field_type(enum bison_field_type *type, struct bison_array_it *it)
{
        return bison_int_field_access_field_type(type, &it->field_access);
}

NG5_EXPORT(bool) bison_array_it_u8_value(u8 *value, struct bison_array_it *it)
{
        return bison_int_field_access_u8_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_u16_value(u16 *value, struct bison_array_it *it)
{
        return bison_int_field_access_u16_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_u32_value(u32 *value, struct bison_array_it *it)
{
        return bison_int_field_access_u32_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_u64_value(u64 *value, struct bison_array_it *it)
{
        return bison_int_field_access_u64_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_i8_value(i8 *value, struct bison_array_it *it)
{
        return bison_int_field_access_i8_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_i16_value(i16 *value, struct bison_array_it *it)
{
        return bison_int_field_access_i16_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_i32_value(i32 *value, struct bison_array_it *it)
{
        return bison_int_field_access_i32_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_i64_value(i64 *value, struct bison_array_it *it)
{
        return bison_int_field_access_i64_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_float_value(bool *is_null_in, float *value, struct bison_array_it *it)
{
        return bison_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_signed_value(bool *is_null_in, i64 *value, struct bison_array_it *it)
{
        return bison_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_unsigned_value(bool *is_null_in, u64 *value, struct bison_array_it *it)
{
        return bison_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(const char *) bison_array_it_string_value(u64 *strlen, struct bison_array_it *it)
{
        return bison_int_field_access_string_value(strlen, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_array_it_binary_value(struct bison_binary *out, struct bison_array_it *it)
{
        return bison_int_field_access_binary_value(out, &it->field_access, &it->err);
}

NG5_EXPORT(struct bison_array_it *) bison_array_it_array_value(struct bison_array_it *it_in)
{
        return bison_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(struct bison_object_it *) bison_array_it_object_value(struct bison_array_it *it_in)
{
        return bison_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(struct bison_column_it *) bison_array_it_column_value(struct bison_array_it *it_in)
{
        return bison_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(bool) bison_array_it_insert_begin(struct bison_insert *inserter, struct bison_array_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_int_insert_create_for_array(inserter, it);
}

NG5_EXPORT(bool) bison_array_it_insert_end(struct bison_insert *inserter)
{
        error_if_null(inserter)
        return bison_insert_drop(inserter);
}

static bool remove_field(struct memfile *memfile, struct err *err, enum bison_field_type type)
{
        assert((enum bison_field_type) *memfile_peek(memfile, sizeof(u8)) == type);
        offset_t start_off = memfile_tell(memfile);
        memfile_skip(memfile, sizeof(u8));
        size_t rm_nbytes = sizeof(u8); /* at least the type marker must be removed */
        switch (type) {
                case BISON_FIELD_TYPE_NULL:
                case BISON_FIELD_TYPE_TRUE:
                case BISON_FIELD_TYPE_FALSE:
                        /* nothing to do */
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8:
                case BISON_FIELD_TYPE_NUMBER_I8:
                        rm_nbytes += sizeof(u8);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U16:
                case BISON_FIELD_TYPE_NUMBER_I16:
                        rm_nbytes += sizeof(u16);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U32:
                case BISON_FIELD_TYPE_NUMBER_I32:
                        rm_nbytes += sizeof(u32);
                        break;
                case BISON_FIELD_TYPE_NUMBER_U64:
                case BISON_FIELD_TYPE_NUMBER_I64:
                        rm_nbytes += sizeof(u64);
                        break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT:
                        rm_nbytes += sizeof(float);
                        break;
                case BISON_FIELD_TYPE_STRING: {
                        u8 len_nbytes;  /* number of bytes used to store string length */
                        u64 str_len; /* the number of characters of the string field */

                        str_len = memfile_read_varuint(&len_nbytes, memfile);

                        rm_nbytes += len_nbytes + str_len;
                } break;
                case BISON_FIELD_TYPE_BINARY: {
                        u8 mime_type_nbytes; /* number of bytes for mime type */
                        u8 blob_length_nbytes; /* number of bytes to store blob length */
                        u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes used for mime type id */
                        memfile_read_varuint(&mime_type_nbytes, memfile);

                        /* get bytes used for blob length info */
                        blob_nbytes = memfile_read_varuint(&blob_length_nbytes, memfile);

                        rm_nbytes += mime_type_nbytes + blob_length_nbytes + blob_nbytes;
                } break;
                case BISON_FIELD_TYPE_BINARY_CUSTOM: {
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
                case BISON_FIELD_TYPE_ARRAY: {
                        struct bison_array_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        bison_array_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        bison_array_it_fast_forward(&it);
                        offset_t end_off = bison_array_it_tell(&it);
                        bison_array_it_drop(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                } break;
                case BISON_FIELD_TYPE_COLUMN_U8:
                case BISON_FIELD_TYPE_COLUMN_U16:
                case BISON_FIELD_TYPE_COLUMN_U32:
                case BISON_FIELD_TYPE_COLUMN_U64:
                case BISON_FIELD_TYPE_COLUMN_I8:
                case BISON_FIELD_TYPE_COLUMN_I16:
                case BISON_FIELD_TYPE_COLUMN_I32:
                case BISON_FIELD_TYPE_COLUMN_I64:
                case BISON_FIELD_TYPE_COLUMN_FLOAT:
                case BISON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        struct bison_column_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        bison_column_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        bison_column_it_fast_forward(&it);
                        offset_t end_off = bison_column_it_tell(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                } break;
                case BISON_FIELD_TYPE_OBJECT:
                        print_error_and_die(NG5_ERR_NOTIMPLEMENTED)
                        break;
                default:
                        error(err, NG5_ERR_INTERNALERR)
                        return false;
        }
        memfile_seek(memfile, start_off);
        memfile_move_left(memfile, rm_nbytes);

        return true;
}

NG5_EXPORT(bool) bison_array_it_remove(struct bison_array_it *it)
{
        unused(it);
        error_if_null(it);
        enum bison_field_type type;
        if (bison_array_it_field_type(&type, it)) {
                offset_t prev_off = bison_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                if (remove_field(&it->memfile, &it->err, type)) {
                        memfile_seek(&it->memfile, prev_off);
                        bison_int_array_it_refresh(NULL, NULL, it);
                        return true;
                } else {
                        return false;
                }
        } else {
                error(&it->err, NG5_ERR_ILLEGALSTATE);
                return false;
        }
}