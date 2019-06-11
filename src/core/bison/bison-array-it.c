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
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"

static bool field_type_read(struct bison_array_it *it)
{
        error_if_null(it)
        error_if(memfile_remain_size(&it->memfile) < 1, &it->err, NG5_ERR_ILLEGALOP);
        memfile_save_position(&it->memfile);
        u8 media_type = *memfile_read(&it->memfile, 1);
        error_if(media_type == 0, &it->err, NG5_ERR_NOTFOUND)
        error_if(media_type == BISON_MARKER_ARRAY_END, &it->err, NG5_ERR_OUTOFBOUNDS)
        it->it_field_type = media_type;
        memfile_restore_position(&it->memfile);
        return true;
}

static bool field_data_access(struct bison_array_it *it)
{
        error_if_null(it)
        memfile_save_position(&it->memfile);
        memfile_skip(&it->memfile, sizeof(media_type_t));
        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I8:
        case BISON_FIELD_TYPE_NUMBER_I16:
        case BISON_FIELD_TYPE_NUMBER_I32:
        case BISON_FIELD_TYPE_NUMBER_I64:
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                break;
        case BISON_FIELD_TYPE_STRING: {
                u8 nbytes;
                varuint_t len = (varuint_t) memfile_peek(&it->memfile, 1);
                it->it_field_len = varuint_read(&nbytes, len);
                memfile_skip(&it->memfile, nbytes);
        } break;
        case BISON_FIELD_TYPE_OBJECT:
        case BISON_FIELD_TYPE_ARRAY:
        case BISON_FIELD_TYPE_NUMBER_U8_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U16_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U32_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U64_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I8_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I16_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I32_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I64_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN:
        case BISON_FIELD_TYPE_NCHAR_COLUMN:
        case BISON_FIELD_TYPE_BINARY:
        case BISON_FIELD_TYPE_NBINARY_COLUMN:
                print_error_and_die(NG5_ERR_NOTIMPLEMENTED)
                break;
        default:
                error(&it->err, NG5_ERR_CORRUPTED)
                return false;
        }

        it->it_field_data = memfile_peek(&it->memfile, 1);
        memfile_restore_position(&it->memfile);
        return true;
}

static bool field_skip(struct bison_array_it *it)
{
        error_if_null(it)
        memfile_skip(&it->memfile, sizeof(media_type_t));

        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
                break;
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_I8:
                assert(sizeof(u8) == sizeof(i8));
                memfile_skip(&it->memfile, sizeof(u8));
                break;
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_I16:
                assert(sizeof(u16) == sizeof(i16));
                memfile_skip(&it->memfile, sizeof(u16));
                break;
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_I32:
                assert(sizeof(u32) == sizeof(i32));
                memfile_skip(&it->memfile, sizeof(u32));
                break;
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I64:
                assert(sizeof(u64) == sizeof(i64));
                memfile_skip(&it->memfile, sizeof(u64));
                break;
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                memfile_skip(&it->memfile, sizeof(float));
                break;
        case BISON_FIELD_TYPE_STRING: {
                u8 nbytes;
                varuint_t varlen = (varuint_t) memfile_peek(&it->memfile, sizeof(varuint_t));
                u64 strlen = varuint_read(&nbytes, varlen);
                memfile_skip(&it->memfile, nbytes + strlen);
        } break;
        case BISON_FIELD_TYPE_OBJECT:
        case BISON_FIELD_TYPE_ARRAY:

        case BISON_FIELD_TYPE_NUMBER_U8_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U16_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U32_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_U64_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I8_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I16_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I32_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_I64_COLUMN:
        case BISON_FIELD_TYPE_NUMBER_FLOAT_COLUMN:
        case BISON_FIELD_TYPE_NCHAR_COLUMN:
        case BISON_FIELD_TYPE_BINARY:
        case BISON_FIELD_TYPE_NBINARY_COLUMN:
        default:
                error(&it->err, NG5_ERR_CORRUPTED);
                return false;
        }

        return true;
}

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct bison *doc, offset_t payload_start,
        enum access_mode mode)
{
        error_if_null(it);
        error_if_null(doc);

        it->payload_start = payload_start;
        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, doc->memblock, mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), &doc->err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_ARRAY_BEGIN, &doc->err, NG5_ERR_ILLEGALOP,
                "array begin marker ('[') not found");

        it->payload_start += sizeof(u8);
        bison_array_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it, struct bison *doc)
{
        ng5_unused(it);
        ng5_unused(doc);
        return false;
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
        return memfile_seek(&it->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it)
{
        error_if_null(it);
        char c = *memfile_peek(&it->memfile, 1);
        bool is_empty_slot = c == 0;
        bool is_array_end = c == BISON_MARKER_ARRAY_END;
        if (!is_empty_slot && !is_array_end) {
                field_type_read(it);
                field_data_access(it);
                field_skip(it);
                return true;
        } else {
                if (!is_array_end) {
                        bool null_found;
                        u8 byte;
                        do {
                                byte = *memfile_read(&it->memfile, sizeof(u8));
                                null_found = byte == 0;
                        } while (null_found);
                        assert(byte == BISON_MARKER_ARRAY_END);
                }

                return false;
        }
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
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I64, &it->err, NG5_ERR_TYPEMISMATCH);
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

NG5_EXPORT(bool) bison_array_it_float_value(float *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_FLOAT, &it->err, NG5_ERR_TYPEMISMATCH);
        *value = *(float *) it->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_array_it_signed_value(i64 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_I8: {
                i8 read_value;
                bison_array_it_i8_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_I16: {
                i16 read_value;
                bison_array_it_i16_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_I32: {
                i32 read_value;
                bison_array_it_i32_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_I64: {
                i64 read_value;
                bison_array_it_i64_value(&read_value, it);
                *value = read_value;
        } break;
        default:
                error(&it->err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_array_it_unsigned_value(u64 *value, struct bison_array_it *it)
{
        error_if_null(value)
        error_if_null(it)
        switch (it->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_U8: {
                u8 read_value;
                bison_array_it_u8_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_U16: {
                u16 read_value;
                bison_array_it_u16_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_U32: {
                u32 read_value;
                bison_array_it_u32_value(&read_value, it);
                *value = read_value;
        } break;
        case BISON_FIELD_TYPE_NUMBER_U64: {
                u64 read_value;
                bison_array_it_u64_value(&read_value, it);
                *value = read_value;
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
        *strlen = it->it_field_len;
        return it->it_field_data;
}

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_insert(struct bison_insert *inserter, struct bison_array_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_insert_create(inserter, it);
}

NG5_EXPORT(bool) bison_array_it_remove(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_update(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}