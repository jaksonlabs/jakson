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
#include "core/bison/bison-object-it.h"
#include "core/bison/bison-key.h"
#include "core/bison/bison-revision.h"

static void marker_insert(struct memfile *memfile, u8 marker);

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it);

static bool is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct memfile *file, u8 end_marker);

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_object_it *it);

static void insert_embedded_container(struct memfile *memfile, u8 begin_marker, u8 end_marker, u8 capacity)
{
        memfile_ensure_space(memfile, sizeof(u8));
        marker_insert(memfile, begin_marker);

        memfile_ensure_space(memfile, capacity + sizeof(u8));

        offset_t payload_begin = memfile_tell(memfile);
        memfile_seek(memfile, payload_begin + capacity);

        marker_insert(memfile, end_marker);

        /* seek to first entry in container */
        memfile_seek(memfile, payload_begin);
}

NG5_EXPORT(bool) bison_int_insert_object(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);
        insert_embedded_container(memfile, BISON_MARKER_OBJECT_BEGIN, BISON_MARKER_OBJECT_END, nbytes);
        return true;
}

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);
        insert_embedded_container(memfile, BISON_MARKER_ARRAY_BEGIN, BISON_MARKER_ARRAY_END, nbytes);
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
                bison_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_int_object_it_next(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it)
{
        if (bison_int_object_it_refresh(is_empty_slot, is_object_end, it)) {
                bison_int_object_it_prop_value_skip(it);
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it)
{
        error_if_null(it);
        if (object_it_is_slot_occupied(is_empty_slot, is_object_end, it)) {
                bison_int_object_it_prop_key_access(it);
                return true;
        } else {
                return false;
        }
}

NG5_EXPORT(bool) bison_int_object_it_prop_key_access(struct bison_object_it *it)
{
        error_if_null(it)
        memfile_save_position(&it->memfile);
        memfile_skip(&it->memfile, sizeof(media_type_t));

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        it->key = memfile_peek(&it->memfile, it->key_len);
        memfile_skip(&it->memfile, it->key_len);

        it->value_off = memfile_tell(&it->memfile);
        it->field_access.it_field_type = *NG5_MEMFILE_READ_TYPE(&it->memfile, u8);
        it->field_access.it_field_data = memfile_peek(&it->memfile, sizeof(u8));

        memfile_restore_position(&it->memfile);
        return true;
}

NG5_EXPORT(bool) bison_int_object_it_prop_key_skip(struct bison_object_it *it)
{
        error_if_null(it)
        memfile_skip(&it->memfile, sizeof(media_type_t));

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, it->key_len);

        it->value_off = memfile_tell(&it->memfile);
        it->field_access.it_field_type = *NG5_MEMFILE_READ_TYPE(&it->memfile, u8);
        return true;
}

NG5_EXPORT(bool) bison_int_object_it_prop_value_skip(struct bison_object_it *it)
{
        error_if_null(it)
        memfile_seek(&it->memfile, it->value_off);
        return bison_field_skip(&it->memfile);
}

NG5_EXPORT(bool) bison_int_object_it_prop_skip(struct bison_object_it *it)
{
        error_if_null(it)
        memfile_skip(&it->memfile, sizeof(media_type_t));

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, it->key_len);

        return bison_field_skip(&it->memfile);
}

NG5_EXPORT(bool) bison_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, struct bison_object_it *it)
{
        while (object_it_next_no_load(is_empty_slot, is_array_end, it))
        { }
        return true;
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
        it->field_access.it_field_type = media_type;
        memfile_restore_position(&it->memfile);
        return true;
}

static bool field_data_access(struct memfile *file, struct err *err, struct field_access *field_access)
{
        memfile_save_position(file);
        memfile_skip(file, sizeof(media_type_t));

        switch (field_access->it_field_type) {
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
                varuint_t len = (varuint_t) memfile_peek(file, 1);
                field_access->it_field_len = varuint_read(&nbytes, len);

                memfile_skip(file, nbytes);
        } break;
        case BISON_FIELD_TYPE_BINARY: {
                /* read mime type with variable-length integer type */
                u64 mime_type_id = memfile_read_varuint(NULL, file);

                field_access->it_mime_type = bison_media_mime_type_by_id(mime_type_id);
                field_access->it_mime_type_strlen = strlen(field_access->it_mime_type);

                /* read blob length */
                field_access->it_field_len = memfile_read_varuint(NULL, file);

                /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
        } break;
        case BISON_FIELD_TYPE_BINARY_CUSTOM: {
                /* read mime type string */
                field_access->it_mime_type_strlen = memfile_read_varuint(NULL, file);
                field_access->it_mime_type = memfile_read(file, field_access->it_mime_type_strlen);

                /* read blob length */
                field_access->it_field_len = memfile_read_varuint(NULL, file);

                /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
        } break;
        case BISON_FIELD_TYPE_ARRAY:
                field_access->nested_array_it_opened = true;
                field_access->nested_array_it_accessed = false;
                bison_array_it_create(field_access->nested_array_it, file, err,
                        memfile_tell(file) - sizeof(u8));
                break;
        case BISON_FIELD_TYPE_COLUMN: {
                bison_column_it_create(field_access->nested_column_it, file, err,
                        memfile_tell(file) - sizeof(u8));
        } break;
        case BISON_FIELD_TYPE_OBJECT:
                field_access->nested_object_it_opened = true;
                field_access->nested_object_it_accessed = false;
                bison_object_it_create(field_access->nested_object_it, file, err,
                        memfile_tell(file) - sizeof(u8));
                break;
        default:
                error(err, NG5_ERR_CORRUPTED)
                return false;
        }

        field_access->it_field_data = memfile_peek(file, 1);
        memfile_restore_position(file);
        return true;
}

NG5_EXPORT(bool) bison_int_array_it_field_data_access(struct bison_array_it *it)
{
        return field_data_access(&it->memfile, &it->err, &it->field_access);
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

NG5_EXPORT(void) bison_int_history_push(struct vector ofType(offset_t) *vec, offset_t off)
{
        assert(vec);
        vec_push(vec, &off, sizeof(offset_t));
}

NG5_EXPORT(void) bison_int_history_clear(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        vec_clear(vec);
}

NG5_EXPORT(offset_t) bison_int_history_pop(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        assert(bison_int_history_has(vec));
        return *(offset_t *) vec_pop(vec);
}

NG5_EXPORT(offset_t) bison_int_history_peek(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        assert(bison_int_history_has(vec));
        return *(offset_t *) vec_peek(vec);
}

NG5_EXPORT(bool) bison_int_history_has(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        return !vec_is_empty(vec);
}

NG5_EXPORT(bool) bison_int_field_access_create(struct field_access *field)
{
        field->nested_array_it_opened = false;
        field->nested_array_it_accessed = false;
        field->nested_object_it_opened = false;
        field->nested_object_it_accessed = false;
        field->nested_array_it = malloc(sizeof(struct bison_array_it));
        field->nested_object_it = malloc(sizeof(struct bison_object_it));
        field->nested_column_it = malloc(sizeof(struct bison_column_it));
        ng5_zero_memory(field->nested_array_it, sizeof(struct bison_array_it))
        ng5_zero_memory(field->nested_object_it, sizeof(struct bison_object_it))
        ng5_zero_memory(field->nested_column_it, sizeof(struct bison_column_it))
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_drop(struct field_access *field)
{
        bison_int_field_auto_close(field);
        free (field->nested_array_it);
        free (field->nested_object_it);
        free (field->nested_column_it);
        return true;
}

NG5_EXPORT(void) bison_int_auto_close_nested_array_it(struct field_access *field)
{
        if (((char *) field->nested_array_it)[0] != 0) {
                bison_array_it_drop(field->nested_array_it);
                ng5_zero_memory(field->nested_array_it, sizeof(struct bison_array_it));
        }
}

NG5_EXPORT(void) bison_int_auto_close_nested_object_it(struct field_access *field)
{
        if (((char *) field->nested_object_it)[0] != 0) {
                bison_object_it_drop(field->nested_object_it);
                ng5_zero_memory(field->nested_object_it, sizeof(struct bison_object_it));
        }
}

NG5_EXPORT(void) bison_int_auto_close_nested_column_it(struct field_access *field)
{
        if (((char *) field->nested_column_it)[0] != 0) {
                ng5_zero_memory(field->nested_column_it, sizeof(struct bison_column_it));
        }
}

NG5_EXPORT(bool) bison_int_field_auto_close(struct field_access *field)
{
        error_if_null(field)
        if (field->nested_array_it_opened && !field->nested_array_it_accessed) {
                bison_int_auto_close_nested_array_it(field);
                field->nested_array_it_opened = false;
                field->nested_array_it_accessed = false;
        }
        if (field->nested_object_it_opened && !field->nested_object_it_accessed) {
                bison_int_auto_close_nested_object_it(field);
                field->nested_array_it_opened = false;
                field->nested_array_it_accessed = false;
        }
        bison_int_auto_close_nested_column_it(field);
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_field_type(enum bison_field_type *type, struct field_access *field)
{
        error_if_null(type)
        error_if_null(field)
        *type = field->it_field_type;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_u8_value(u8 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_U8, err, NG5_ERR_TYPEMISMATCH);
        *value = *(u8 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_u16_value(u16 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_U16, err, NG5_ERR_TYPEMISMATCH);
        *value = *(u16 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_u32_value(u32 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_U32, err, NG5_ERR_TYPEMISMATCH);
        *value = *(u32 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_u64_value(u64 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_U64, err, NG5_ERR_TYPEMISMATCH);
        *value = *(u64 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_i8_value(i8 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_I8, err, NG5_ERR_TYPEMISMATCH);
        *value = *(i8 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_i16_value(i16 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_I16, err, NG5_ERR_TYPEMISMATCH);
        *value = *(i16 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_i32_value(i32 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_I32, err, NG5_ERR_TYPEMISMATCH);
        *value = *(i32 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_i64_value(i64 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_I64, err, NG5_ERR_TYPEMISMATCH);
        *value = *(i64 *) field->it_field_data;
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_float_value(bool *is_null_in, float *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_NUMBER_FLOAT, err, NG5_ERR_TYPEMISMATCH);
        float read_value = *(float *) field->it_field_data;
        ng5_optional_set(value, read_value);
        ng5_optional_set(is_null_in, is_null_float(read_value));

        return true;
}

NG5_EXPORT(bool) bison_int_field_access_signed_value(bool *is_null_in, i64 *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        switch (field->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_I8: {
                i8 read_value;
                bison_int_field_access_i8_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i8(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I16: {
                i16 read_value;
                bison_int_field_access_i16_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i16(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I32: {
                i32 read_value;
                bison_int_field_access_i32_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i32(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_I64: {
                i64 read_value;
                bison_int_field_access_i64_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_i64(read_value));
        } break;
        default:
                error(err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
        return true;
}

NG5_EXPORT(bool) bison_int_field_access_unsigned_value(bool *is_null_in, u64 *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        switch (field->it_field_type) {
        case BISON_FIELD_TYPE_NUMBER_U8: {
                u8 read_value;
                bison_int_field_access_u8_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u8(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U16: {
                u16 read_value;
                bison_int_field_access_u16_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u16(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U32: {
                u32 read_value;
                bison_int_field_access_u32_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u32(read_value));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U64: {
                u64 read_value;
                bison_int_field_access_u64_value(&read_value, field, err);
                ng5_optional_set(value, read_value);
                ng5_optional_set(is_null_in, is_null_u64(read_value));
        } break;
        default:
                error(err, NG5_ERR_TYPEMISMATCH);
                return false;
        }
        return true;
}

NG5_EXPORT(const char *) bison_int_field_access_string_value(u64 *strlen, struct field_access *field, struct err *err)
{
        error_if_null(strlen);
        error_if_and_return(field == NULL, err, NG5_ERR_NULLPTR, NULL);
        error_if(field->it_field_type != BISON_FIELD_TYPE_STRING, err, NG5_ERR_TYPEMISMATCH);
        *strlen = field->it_field_len;
        return field->it_field_data;
}

NG5_EXPORT(bool) bison_int_field_access_binary_value(struct bison_binary *out, struct field_access *field, struct err *err)
{
        error_if_null(out)
        error_if_null(field)
        error_if(field->it_field_type != BISON_FIELD_TYPE_BINARY && field->it_field_type != BISON_FIELD_TYPE_BINARY_CUSTOM,
                err, NG5_ERR_TYPEMISMATCH);
        out->blob = field->it_field_data;
        out->blob_len = field->it_field_len;
        out->mime_type = field->it_mime_type;
        out->mime_type_strlen = field->it_mime_type_strlen;
        return true;
}

NG5_EXPORT(struct bison_array_it *) bison_int_field_access_array_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, NG5_ERR_NULLPTR);
        error_if(field->it_field_type != BISON_FIELD_TYPE_ARRAY, err, NG5_ERR_TYPEMISMATCH);
        field->nested_array_it_accessed = true;
        return field->nested_array_it;
}

NG5_EXPORT(struct bison_object_it *) bison_int_field_access_object_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, NG5_ERR_NULLPTR);
        error_if(field->it_field_type != BISON_FIELD_TYPE_OBJECT, err, NG5_ERR_TYPEMISMATCH);
        field->nested_object_it_accessed = true;
        return field->nested_object_it;
}

NG5_EXPORT(struct bison_column_it *) bison_int_field_access_column_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, NG5_ERR_NULLPTR);
        error_if(field->it_field_type != BISON_FIELD_TYPE_COLUMN, err, NG5_ERR_TYPEMISMATCH);
        return field->nested_column_it;
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
        bison_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_array_end, &it->memfile, BISON_MARKER_ARRAY_END);
}

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it)
{
        bison_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_object_end, &it->memfile, BISON_MARKER_OBJECT_END);
}

static bool is_slot_occupied(bool *is_empty_slot, bool *is_end_reached, struct memfile *file, u8 end_marker)
{
        error_if_null(file);
        char c = *memfile_peek(file, 1);
        bool is_empty = c == 0, is_end = c == end_marker;
        ng5_optional_set(is_empty_slot, is_empty)
        ng5_optional_set(is_end_reached, is_end)
        if (!is_empty && !is_end) {
                return true;
        } else {
                return false;
        }
}

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_object_it *it)
{
        if (object_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                bison_int_object_it_prop_skip(it);
                return true;
        } else {
                return false;
        }
}

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it)
{
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                bison_int_array_it_field_type_read(it);
                bison_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}