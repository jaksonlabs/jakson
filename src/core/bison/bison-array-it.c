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
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-int.h"

static void auto_close_nested_array_it(struct bison_array_it *it);
static void auto_close_nested_column_it(struct bison_array_it *it);

static inline void history_clear(struct bison_array_it *it);
static inline void push_to_history(struct bison_array_it *it, offset_t off);
static inline offset_t peek_from_history(struct bison_array_it *it);
static inline offset_t prop_from_history(struct bison_array_it *it);
static inline bool has_history(struct bison_array_it *it);

#define DEFINE_IN_PLACE_UPDATE_FUNCTION(type_name, field_type)                                                         \
NG5_EXPORT(bool) bison_array_it_update_in_place_##type_name(struct bison_array_it *it, type_name value)                \
{                                                                                                                      \
        offset_t datum;                                                                                                \
        error_if_null(it);                                                                                             \
        if (likely(it->it_field_type == field_type)) {                                                                 \
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

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct memfile *memfile, struct err *err,
                                       offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->payload_start = payload_start;
        it->nested_array_it_opened = false;
        it->nested_array_it_accessed = false;
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
        it->nested_array_it = malloc(sizeof(struct bison_array_it));
        it->nested_column_it = malloc(sizeof(struct bison_column_it));
        ng5_zero_memory(it->nested_array_it, sizeof(struct bison_array_it))
        ng5_zero_memory(it->nested_column_it, sizeof(struct bison_column_it))
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
        bison_int_array_it_auto_close(it);
        vec_drop(&it->history);
        free (it->nested_array_it);
        free (it->nested_column_it);
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
        history_clear(it);
        return memfile_seek(&it->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it)
{
        error_if_null(it);
        bool is_empty_slot, is_array_end;
        offset_t last_off = memfile_tell(&it->memfile);
        if (bison_int_array_it_next(&is_empty_slot, &is_array_end, it)) {
                push_to_history(it, last_off);
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
                bison_int_array_it_auto_close(it);
                return false;
        }
}

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it)
{
        error_if_null(it);
        if (has_history(it)) {
                offset_t prev_off = prop_from_history(it);
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
        if (has_history(it)) {
                *off = peek_from_history(it);
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
        error_if_null(type)
        error_if_null(it)
        *type = it->it_field_type;
        return true;
}

NG5_EXPORT(bool) bison_array_it_u8_value(u8 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_U8, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(u8 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_u16_value(u16 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_U16, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(u16 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_u32_value(u32 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_U32, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(u32 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_u64_value(u64 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_U64, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(u64 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_i8_value(i8 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I8, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(i8 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_i16_value(i16 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I16, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(i16 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_i32_value(i32 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I32, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(i32 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_i64_value(i64 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I64, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(i64 *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_float_value(bool *is_null_in, float *value, struct bison_array_it *it)
{
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_FLOAT, &it->err, NG5_ERR_TYPEMISMATCH);
        float read_value = *(float *) it->it_field_data;
        ng5_optional_set(value, read_value);
        ng5_optional_set(is_null_in, is_null_float(read_value));

        return true;
}

NG5_EXPORT(bool) bison_array_it_signed_value(bool *is_null_in, i64 *value, struct bison_array_it *it)
{
        error_if_null(it)
        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_I8: {
                i8 read_value;
                bison_array_it_i8_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i8(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I16: {
                i16 read_value;
                bison_array_it_i16_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i16(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I32: {
                i32 read_value;
                bison_array_it_i32_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i32(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I64: {
                i64 read_value;
                bison_array_it_i64_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i64(read_value));
        } break;
        default:
                error(&it->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_array_it_unsigned_value(bool *is_null_in, u64 *value, struct bison_array_it *it)
{
        error_if_null(it)
        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_U8: {
                u8 read_value;
                bison_array_it_u8_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u8(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U16: {
                u16 read_value;
                bison_array_it_u16_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u16(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U32: {
                u32 read_value;
                bison_array_it_u32_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u32(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U64: {
                u64 read_value;
                bison_array_it_u64_value(&read_value, it);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u64(read_value));
        } break;
        default:
        error(&it->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
        return true;
}

NG5_EXPORT(const char *) bison_array_it_string_value(u64 *strlen, struct bison_array_it *it)
{
        error_if_null(strlen);
        error_if_and_return(it == NULL, &it->err, NG5_ERR_NULLPTR, NULL);
        error_if(it->it_field_type != BISON_FIELD_TYPE_STRING, &it->err, NG5_ERR_TYPEMISMATCH);
        *strlen = it->it_field_len;
        return it->it_field_data;
}

NG5_EXPORT(bool) bison_array_it_binary_value(struct bison_binary *out, struct bison_array_it *it)
{
        error_if_null(out)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_BINARY && it->it_field_type != BISON_FIELD_TYPE_BINARY_CUSTOM,
                &it->err, NG5_ERR_TYPEMISMATCH);
        out->blob = it->it_field_data;
        out->blob_len = it->it_field_len;
        out->mime_type = it->it_mime_type;
        out->mime_type_strlen = it->it_mime_type_strlen;
        return true;
}

NG5_EXPORT(struct bison_array_it *) bison_array_it_array_value(struct bison_array_it *it_in)
{
        error_if_and_return(!it_in, &it_in->err, NG5_ERR_NULLPTR, NULL);
        error_if(it_in->it_field_type != BISON_FIELD_TYPE_ARRAY, &it_in->err, NG5_ERR_TYPEMISMATCH);
        it_in->nested_array_it_accessed = true;
        return it_in->nested_array_it;
}

NG5_EXPORT(struct bison_column_it *) bison_array_it_column_value(struct bison_array_it *it_in)
{
        error_if_and_return(!it_in, &it_in->err, NG5_ERR_NULLPTR, NULL);
        error_if(it_in->it_field_type != BISON_FIELD_TYPE_COLUMN, &it_in->err, NG5_ERR_TYPEMISMATCH);
        return it_in->nested_column_it;
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
                case BISON_FIELD_TYPE_COLUMN: {
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
        ng5_unused(it);
        error_if_null(it);
        enum bison_field_type type;
        if (bison_array_it_field_type(&type, it)) {
                offset_t prev_off = prop_from_history(it);
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

NG5_EXPORT(bool) bison_array_it_update(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_int_array_it_auto_close(struct bison_array_it *it)
{
        error_if_null(it)
        if (it->nested_array_it_opened && !it->nested_array_it_accessed) {
                auto_close_nested_array_it(it);
                it->nested_array_it_opened = false;
                it->nested_array_it_accessed = false;
        }
        auto_close_nested_column_it(it);
        return true;
}

static void auto_close_nested_array_it(struct bison_array_it *it)
{
        if (((char *) it->nested_array_it)[0] != 0) {
                bison_array_it_drop(it->nested_array_it);
                ng5_zero_memory(it->nested_array_it, sizeof(struct bison_array_it));
        }
}

static void auto_close_nested_column_it(struct bison_array_it *it)
{
        if (((char *) it->nested_column_it)[0] != 0) {
                ng5_zero_memory(it->nested_column_it, sizeof(struct bison_column_it));
        }
}

static inline void history_clear(struct bison_array_it *it)
{
        assert(it);
        vec_clear(&it->history);
}

static inline void push_to_history(struct bison_array_it *it, offset_t off)
{
        assert(it);
        vec_push(&it->history, &off, sizeof(offset_t));
}

static inline offset_t prop_from_history(struct bison_array_it *it)
{
        assert(it);
        assert(has_history(it));
        return *(offset_t *) vec_pop(&it->history);
}

static inline offset_t peek_from_history(struct bison_array_it *it)
{
        assert(it);
        assert(has_history(it));
        return *(offset_t *) vec_peek(&it->history);
}

static inline bool has_history(struct bison_array_it *it)
{
        assert(it);
        return !vec_is_empty(&it->history);
}