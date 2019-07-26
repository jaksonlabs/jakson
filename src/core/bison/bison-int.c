/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file is for internal usage only; do not call these functions from outside
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
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-key.h"
#include "core/bison/bison-revision.h"

static void marker_insert(struct memfile *memfile, u8 marker);

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);

        u8 array_begin_marker = BISON_MARKER_ARRAY_BEGIN;
        u8 array_end_marker = BISON_MARKER_ARRAY_END;

        memfile_ensure_space(memfile, sizeof(u8));
        marker_insert(memfile, array_begin_marker);

        memfile_ensure_space(memfile, nbytes + sizeof(u8));

        offset_t payload_begin = memfile_tell(memfile);
        memfile_seek(memfile, payload_begin + nbytes);

        marker_insert(memfile, array_end_marker);

        /* seek to first entry in array */
        memfile_seek(memfile, payload_begin);

        return true;
}

NG5_EXPORT(bool) bison_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum bison_field_type type, size_t capactity)
{
        error_if_null(memfile_in);
        error_if_null(err_in);

        unused(capactity);

        switch (type) {
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
                break; /* all types from above are fixed-length and therefore supported in a column */
        default:
                error_with_details(err_in, NG5_ERR_BADTYPE, "BISON column supports fixed-length types only")
        }

        u8 column_begin_marker = BISON_MARKER_COLUMN_BEGIN;

        memfile_ensure_space(memfile_in, sizeof(u8));
        marker_insert(memfile_in, column_begin_marker);
        memfile_ensure_space(memfile_in, sizeof(media_type_t));
        bison_media_write(memfile_in, type);

        u32 num_elements = 0;
        u32 cap_elements = capactity;

        memfile_write_varuint(memfile_in, num_elements);
        memfile_write_varuint(memfile_in, cap_elements);

        offset_t payload_begin = memfile_tell(memfile_in);

        size_t type_size = bison_int_get_type_value_size(type);

        size_t nbytes = capactity * type_size;
        memfile_ensure_space(memfile_in, nbytes + sizeof(u8) + 2 * sizeof(u32));

        /* seek to first entry in column */
        memfile_seek(memfile_in, payload_begin);

        return true;
}

NG5_EXPORT(size_t) bison_int_get_type_size_encoded(enum bison_field_type type)
{
        size_t type_size = sizeof(media_type_t); /* at least the media type marker is required */
        switch (type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
                /* only media type marker is required */
                break;
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_I8:
                type_size += sizeof(u8);
                break;
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_I16:
                type_size += sizeof(u16);
                break;
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_I32:
                type_size += sizeof(u32);
                break;
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I64:
                type_size += sizeof(u64);
                break;
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                type_size += sizeof(float);
                break;
        default:
                error_print(NG5_ERR_INTERNALERR);
                return 0;
        }
        return type_size;
}

NG5_EXPORT(size_t) bison_int_get_type_value_size(enum bison_field_type type)
{
        switch (type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
                return sizeof(media_type_t); /* these constant values are determined by their media type markers */
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_I8:
                return sizeof(u8);
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_I16:
                return sizeof(u16);
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_I32:
                return sizeof(u32);
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I64:
                return sizeof(u64);
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                return sizeof(float);
        default:
        error_print(NG5_ERR_INTERNALERR);
                return 0;
        }
}

NG5_EXPORT(bool) bison_int_array_it_next(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        if (bison_int_array_it_refresh(is_empty_slot, is_array_end, it)) {
                bison_int_array_it_field_skip(it);
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        while (array_it_next_no_load(is_empty_slot, is_array_end, it))
        { }
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        error_if_null(it);
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                bison_int_array_it_field_type_read(it);
                bison_int_array_it_field_data_access(it);
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_int_array_it_field_type_read(struct bison_array_it *it)
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

NG5_EXPORT(bool) bison_int_array_it_field_data_access(struct bison_array_it *it)
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
        case BISON_FIELD_TYPE_BINARY: {
                /* read mime type with variable-length integer type */
                u64 mime_type_id = memfile_read_varuint(NULL, &it->memfile);

                it->it_mime_type = bison_media_mime_type_by_id(mime_type_id);
                it->it_mime_type_strlen = strlen(it->it_mime_type);

                /* read blob length */
                it->it_field_len = memfile_read_varuint(NULL, &it->memfile);

                /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
        } break;
        case BISON_FIELD_TYPE_BINARY_CUSTOM: {
                /* read mime type string */
                it->it_mime_type_strlen = memfile_read_varuint(NULL, &it->memfile);
                it->it_mime_type = memfile_read(&it->memfile, it->it_mime_type_strlen);

                /* read blob length */
                it->it_field_len = memfile_read_varuint(NULL, &it->memfile);

                /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
        } break;
        case BISON_FIELD_TYPE_ARRAY:
                it->nested_array_it_opened = true;
                it->nested_array_it_accessed = false;
                bison_array_it_create(it->nested_array_it, &it->memfile, &it->err,
                        memfile_tell(&it->memfile) - sizeof(u8));
                break;
        case BISON_FIELD_TYPE_COLUMN: {
                bison_column_it_create(it->nested_column_it, &it->memfile, &it->err,
                        memfile_tell(&it->memfile) - sizeof(u8));
        } break;
        case BISON_FIELD_TYPE_OBJECT:
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

NG5_EXPORT(bool) bison_int_array_it_skip_array(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_ARRAY, &it->err, NG5_ERR_TYPEMISMATCH);
        struct bison_array_it skip_it;
        bison_array_it_create(&skip_it, &it->memfile, &it->err, memfile_tell(&it->memfile) - sizeof(u8));
        bison_array_it_fast_forward(&skip_it);
        memfile_seek(&it->memfile, memfile_tell(&skip_it.memfile));
        bison_array_it_drop(&skip_it);
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_column(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_COLUMN, &it->err, NG5_ERR_TYPEMISMATCH);
        struct bison_column_it skip_it;
        bison_column_it_create(&skip_it, &it->memfile, &it->err,
                memfile_tell(&it->memfile) - sizeof(media_type_t));
        bison_column_it_fast_forward(&skip_it);
        memfile_seek(&it->memfile, memfile_tell(&skip_it.memfile));
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_binary(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_BINARY, &it->err, NG5_ERR_TYPEMISMATCH);
        /* read and skip mime type with variable-length integer type */
        u64 mime_type = memfile_read_varuint(NULL, &it->memfile);
        unused(mime_type);

        /* read blob length */
        u64 blob_len = memfile_read_varuint(NULL, &it->memfile);

        /* skip blob */
        memfile_skip(&it->memfile, blob_len);
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_custom_binary(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_BINARY_CUSTOM, &it->err, NG5_ERR_TYPEMISMATCH);
        /* read custom type string length, and skip the type string */
        u64 custom_type_str_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, custom_type_str_len);

        /* read blob length, and skip blob data */
        u64 blob_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, blob_len);
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_string(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_STRING, &it->err, NG5_ERR_TYPEMISMATCH);
        u64 strlen = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, strlen);
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_float(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_FLOAT, &it->err, NG5_ERR_TYPEMISMATCH);
        memfile_skip(&it->memfile, sizeof(float));
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_8(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I8 && it->it_field_type != BISON_FIELD_TYPE_NUMBER_U8,
                &it->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u8) == sizeof(i8));
        memfile_skip(&it->memfile, sizeof(u8));
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_16(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I16 && it->it_field_type != BISON_FIELD_TYPE_NUMBER_U16,
                &it->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u16) == sizeof(i16));
        memfile_skip(&it->memfile, sizeof(u16));
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_32(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I32 && it->it_field_type != BISON_FIELD_TYPE_NUMBER_U32,
                &it->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u32) == sizeof(i32));
        memfile_skip(&it->memfile, sizeof(u32));
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_skip_64(struct bison_array_it *it)
{
        error_if(it->it_field_type != BISON_FIELD_TYPE_NUMBER_I64 && it->it_field_type != BISON_FIELD_TYPE_NUMBER_U64,
                &it->err, NG5_ERR_TYPEMISMATCH);
        assert(sizeof(u64) == sizeof(i64));
        memfile_skip(&it->memfile, sizeof(u64));
        return true;
}

NG5_EXPORT(offset_t) bison_int_column_get_payload_off(struct bison_column_it *it)
{
        memfile_save_position(&it->memfile);
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        memfile_skip_varuint(&it->memfile); // skip num of elements
        memfile_skip_varuint(&it->memfile); // skip capacity of elements
        offset_t result = memfile_tell(&it->memfile);
        memfile_restore_position(&it->memfile);
        return result;
}

NG5_EXPORT(offset_t) bison_int_payload_after_header(struct bison *doc)
{
        offset_t result = 0;
        enum bison_primary_key_type key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        if (likely(bison_key_skip(&key_type, &doc->memfile))) {
                if (key_type != BISON_KEY_NOKEY) {
                        bison_revision_skip(&doc->memfile);
                }
                result = memfile_tell(&doc->memfile);
        }

        memfile_restore_position(&doc->memfile);

        return result;
}

NG5_EXPORT(u64) bison_int_header_get_rev(struct bison *doc)
{
        assert(doc);
        u64 rev = 0;
        enum bison_primary_key_type key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        bison_key_skip(&key_type, &doc->memfile);
        if (key_type != BISON_KEY_NOKEY) {
                bison_revision_read(&rev, &doc->memfile);
        }

        memfile_restore_position(&doc->memfile);
        return rev;
}

NG5_EXPORT(bool) bison_int_array_it_field_skip(struct bison_array_it *it)
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
                bison_int_array_it_skip_8(it);
                break;
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_I16:
                bison_int_array_it_skip_16(it);
                break;
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_I32:
                bison_int_array_it_skip_32(it);
                break;
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I64:
                bison_int_array_it_skip_64(it);
                break;
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                bison_int_array_it_skip_float(it);
                break;
        case BISON_FIELD_TYPE_STRING:
                bison_int_array_it_skip_string(it);
                break;
        case BISON_FIELD_TYPE_BINARY:
                bison_int_array_it_skip_binary(it);
                break;
        case BISON_FIELD_TYPE_BINARY_CUSTOM:
                bison_int_array_it_skip_custom_binary(it);
                break;
        case BISON_FIELD_TYPE_ARRAY:
                bison_int_array_it_skip_array(it);
                break;
        case BISON_FIELD_TYPE_COLUMN:
                bison_int_array_it_skip_column(it);
                break;
        case BISON_FIELD_TYPE_OBJECT:
        default:
        error(&it->err, NG5_ERR_CORRUPTED);
                return false;
        }

        return true;
}

static void marker_insert(struct memfile *memfile, u8 marker)
{
        /* check whether marker can be written, otherwise make space for it */
        char c = *memfile_peek(memfile, sizeof(u8));
        if (c != 0) {
                memfile_move_right(memfile, sizeof(u8));
        }
        memfile_write(memfile, &marker, sizeof(u8));
}

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        error_if_null(it);
        bison_int_array_it_auto_close(it);
        char c = *memfile_peek(&it->memfile, 1);
        bool is_empty = c == 0, is_end = c == BISON_MARKER_ARRAY_END;
        ng5_optional_set(is_empty_slot, is_empty)
        ng5_optional_set(is_array_end, is_end)
        if (!is_empty && !is_end) {
                return true;
        } else {
                return false;
        }
}

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                bison_int_array_it_field_type_read(it);
                bison_int_array_it_field_skip(it);
                return true;
        } else {
                return false;
        }
}