/**
 * Copyright 2019 Marcus Pinnecke
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

#include <carbon/carbon-archive-iter.h>

static bool
base_object_in_record_table(carbon_archive_object_t *obj, carbon_memfile_t *record_table_memfile)
{
    assert(obj);
    carbon_object_header_t       *header;
    carbon_archive_object_flags_t flags;

    header = CARBON_MEMFILE_READ_TYPE(record_table_memfile, carbon_object_header_t);
    if (CARBON_UNLIKELY(header->marker != MARKER_SYMBOL_OBJECT_BEGIN)) {
        return false;
    }

    flags.value = header->flags;
    carbon_int_read_prop_offsets(&obj->prop_offsets, record_table_memfile, &flags);

    obj->object_id = header->oid;
    obj->next_obj_off = *CARBON_MEMFILE_READ_TYPE(record_table_memfile, carbon_off_t);

    return true;
}

#define STATE_AND_PROPERTY_EXISTS(state, property) \
    (iter->prop_cursor != state || iter->object.property != 0)

inline static carbon_off_t
offset_by_state(carbon_archive_prop_iter_t *iter)
{
    switch (iter->prop_cursor) {
    case CARBON_PROP_ITER_STATE_NULLS:
        return iter->object.prop_offsets.nulls;
    case CARBON_PROP_ITER_STATE_BOOLS:
        return iter->object.prop_offsets.bools;
    case CARBON_PROP_ITER_STATE_INT8S:
        return iter->object.prop_offsets.int8s;
    case CARBON_PROP_ITER_STATE_INT16S:
        return iter->object.prop_offsets.int16s;
    case CARBON_PROP_ITER_STATE_INT32S:
        return iter->object.prop_offsets.int32s;
    case CARBON_PROP_ITER_STATE_INT64S:
        return iter->object.prop_offsets.int64s;
    case CARBON_PROP_ITER_STATE_UINT8S:
        return iter->object.prop_offsets.uint8s;
    case CARBON_PROP_ITER_STATE_UINT16S:
        return iter->object.prop_offsets.uint16s;
    case CARBON_PROP_ITER_STATE_UINT32S:
        return iter->object.prop_offsets.uint32s;
    case CARBON_PROP_ITER_STATE_UINT64S:
        return iter->object.prop_offsets.uint64s;
    case CARBON_PROP_ITER_STATE_FLOATS:
        return iter->object.prop_offsets.floats;
    case CARBON_PROP_ITER_STATE_STRINGS:
        return iter->object.prop_offsets.strings;
    case CARBON_PROP_ITER_STATE_OBJECTS:
        return iter->object.prop_offsets.objects;
    case CARBON_PROP_ITER_STATE_NULL_ARRAYS:
        return iter->object.prop_offsets.null_arrays;
    case CARBON_PROP_ITER_STATE_BOOL_ARRAYS:
        return iter->object.prop_offsets.bool_arrays;
    case CARBON_PROP_ITER_STATE_INT8_ARRAYS:
        return iter->object.prop_offsets.int8_arrays;
    case CARBON_PROP_ITER_STATE_INT16_ARRAYS:
        return iter->object.prop_offsets.int16_arrays;
    case CARBON_PROP_ITER_STATE_INT32_ARRAYS:
        return iter->object.prop_offsets.int32_arrays;
    case CARBON_PROP_ITER_STATE_INT64_ARRAYS:
        return iter->object.prop_offsets.int64_arrays;
    case CARBON_PROP_ITER_STATE_UINT8_ARRAYS:
        return iter->object.prop_offsets.uint8_arrays;
    case CARBON_PROP_ITER_STATE_UINT16_ARRAYS:
        return iter->object.prop_offsets.uint16_arrays;
    case CARBON_PROP_ITER_STATE_UINT32_ARRAYS:
        return iter->object.prop_offsets.uint32_arrays;
    case CARBON_PROP_ITER_STATE_UINT64_ARRAYS:
        return iter->object.prop_offsets.uint64_arrays;
    case CARBON_PROP_ITER_STATE_FLOAT_ARRAYS:
        return iter->object.prop_offsets.float_arrays;
    case CARBON_PROP_ITER_STATE_STRING_ARRAYS:
        return iter->object.prop_offsets.string_arrays;
    case CARBON_PROP_ITER_STATE_OBJECT_ARRAYS:
        return iter->object.prop_offsets.object_arrays;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR)
    }
}

static void
prop_iter_cursor_init(carbon_archive_prop_iter_t *iter)
{
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_NULLS, prop_offsets.nulls));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_BOOLS, prop_offsets.bools));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT8S, prop_offsets.int8s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT16S, prop_offsets.int16s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT32S, prop_offsets.int32s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT64S, prop_offsets.int64s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT8S, prop_offsets.uint8s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT16S, prop_offsets.uint16s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT32S, prop_offsets.uint32s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT64S, prop_offsets.uint64s));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_FLOATS, prop_offsets.floats));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_STRINGS, prop_offsets.strings));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_OBJECTS, prop_offsets.objects));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_NULL_ARRAYS, prop_offsets.null_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_BOOL_ARRAYS, prop_offsets.bool_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT8_ARRAYS, prop_offsets.int8_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT16_ARRAYS, prop_offsets.int16_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT32_ARRAYS, prop_offsets.int32_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_INT64_ARRAYS, prop_offsets.int64_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT8_ARRAYS, prop_offsets.uint8_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT16_ARRAYS, prop_offsets.uint16_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT32_ARRAYS, prop_offsets.uint32_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_UINT64_ARRAYS, prop_offsets.uint64_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_FLOAT_ARRAYS, prop_offsets.float_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_STRING_ARRAYS, prop_offsets.string_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(CARBON_PROP_ITER_STATE_OBJECT_ARRAYS, prop_offsets.object_arrays));

    iter->prop_type_off_current = offset_by_state(iter);
    carbon_memfile_seek(&iter->record_table_memfile, iter->prop_type_off_current);
    carbon_int_embedded_fixed_props_read(&iter->prop_header, &iter->record_table_memfile);
    iter->prop_type_off_data = CARBON_MEMFILE_TELL(&iter->record_table_memfile);
    iter->prop_pos_current = 0;
}

#define SET_STATE_FOR_FALL_THROUGH(iter, prop_offset_type, mask_group, mask_type, next_state)                          \
{                                                                                                                      \
    if ((iter->object.prop_offsets.prop_offset_type != 0) &&                                                           \
        (CARBON_MASK_IS_BIT_SET(iter->mask, mask_group | mask_type)))                                                  \
    {                                                                                                                  \
        iter->prop_cursor = next_state;                                                                                \
        break;                                                                                                         \
    }                                                                                                                  \
}

static carbon_prop_iter_state_e
prop_iter_state_next(carbon_archive_prop_iter_t *iter)
{
    switch (iter->prop_cursor) {
    case CARBON_PROP_ITER_STATE_INIT:
        SET_STATE_FOR_FALL_THROUGH(iter, nulls, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   CARBON_ARCHIVE_ITER_MASK_NULL, CARBON_PROP_ITER_STATE_NULLS)
    case CARBON_PROP_ITER_STATE_NULLS:
        SET_STATE_FOR_FALL_THROUGH(iter, bools, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   CARBON_ARCHIVE_ITER_MASK_BOOLEAN, CARBON_PROP_ITER_STATE_BOOLS)
    case CARBON_PROP_ITER_STATE_BOOLS:
        SET_STATE_FOR_FALL_THROUGH(iter, int8s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   CARBON_ARCHIVE_ITER_MASK_INT8, CARBON_PROP_ITER_STATE_INT8S)
    case CARBON_PROP_ITER_STATE_INT8S:
        SET_STATE_FOR_FALL_THROUGH(iter, int16s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_INT16, CARBON_PROP_ITER_STATE_INT16S)
    case CARBON_PROP_ITER_STATE_INT16S:
        SET_STATE_FOR_FALL_THROUGH(iter, int32s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_INT32, CARBON_PROP_ITER_STATE_INT32S)
    case CARBON_PROP_ITER_STATE_INT32S:
        SET_STATE_FOR_FALL_THROUGH(iter, int64s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_INT64, CARBON_PROP_ITER_STATE_INT64S)
    case CARBON_PROP_ITER_STATE_INT64S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint8s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_UINT8, CARBON_PROP_ITER_STATE_UINT8S)
    case CARBON_PROP_ITER_STATE_UINT8S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint16s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_UINT16, CARBON_PROP_ITER_STATE_UINT16S)
    case CARBON_PROP_ITER_STATE_UINT16S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint32s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_UINT32, CARBON_PROP_ITER_STATE_UINT32S)
    case CARBON_PROP_ITER_STATE_UINT32S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint64s, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_UINT64, CARBON_PROP_ITER_STATE_UINT64S)
    case CARBON_PROP_ITER_STATE_UINT64S:
        SET_STATE_FOR_FALL_THROUGH(iter, floats, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_NUMBER, CARBON_PROP_ITER_STATE_FLOATS)
    case CARBON_PROP_ITER_STATE_FLOATS:
        SET_STATE_FOR_FALL_THROUGH(iter, strings, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_STRING, CARBON_PROP_ITER_STATE_STRINGS)
    case CARBON_PROP_ITER_STATE_STRINGS:
        SET_STATE_FOR_FALL_THROUGH(iter, objects, CARBON_ARCHIVE_ITER_MASK_PRIMITIVES,
                               CARBON_ARCHIVE_ITER_MASK_OBJECT, CARBON_PROP_ITER_STATE_OBJECTS)
    case CARBON_PROP_ITER_STATE_OBJECTS:
        SET_STATE_FOR_FALL_THROUGH(iter, null_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_NULL, CARBON_PROP_ITER_STATE_NULL_ARRAYS)
    case CARBON_PROP_ITER_STATE_NULL_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, bool_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_BOOLEAN, CARBON_PROP_ITER_STATE_BOOL_ARRAYS)
    case CARBON_PROP_ITER_STATE_BOOL_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int8_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_INT8, CARBON_PROP_ITER_STATE_INT8_ARRAYS)
    case CARBON_PROP_ITER_STATE_INT8_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int16_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_INT16, CARBON_PROP_ITER_STATE_INT16_ARRAYS)
    case CARBON_PROP_ITER_STATE_INT16_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int32_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_INT32, CARBON_PROP_ITER_STATE_INT32_ARRAYS)
    case CARBON_PROP_ITER_STATE_INT32_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int64_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_INT64, CARBON_PROP_ITER_STATE_INT64_ARRAYS)
    case CARBON_PROP_ITER_STATE_INT64_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint8_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_UINT8, CARBON_PROP_ITER_STATE_UINT8_ARRAYS)
    case CARBON_PROP_ITER_STATE_UINT8_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint16_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_UINT16, CARBON_PROP_ITER_STATE_UINT16_ARRAYS)
    case CARBON_PROP_ITER_STATE_UINT16_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint32_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_UINT32, CARBON_PROP_ITER_STATE_UINT32_ARRAYS)
    case CARBON_PROP_ITER_STATE_UINT32_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint64_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_UINT64, CARBON_PROP_ITER_STATE_UINT64_ARRAYS)
    case CARBON_PROP_ITER_STATE_UINT64_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, float_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_NUMBER, CARBON_PROP_ITER_STATE_FLOAT_ARRAYS)
    case CARBON_PROP_ITER_STATE_FLOAT_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, string_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_STRING, CARBON_PROP_ITER_STATE_STRING_ARRAYS)
    case CARBON_PROP_ITER_STATE_STRING_ARRAYS:
    SET_STATE_FOR_FALL_THROUGH(iter, object_arrays, CARBON_ARCHIVE_ITER_MASK_ARRAYS,
                               CARBON_ARCHIVE_ITER_MASK_OBJECT, CARBON_PROP_ITER_STATE_OBJECT_ARRAYS)
    case CARBON_PROP_ITER_STATE_OBJECT_ARRAYS:
        iter->prop_cursor = CARBON_PROP_ITER_STATE_DONE;
        break;

    case CARBON_PROP_ITER_STATE_DONE:
        break;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
    if (iter->prop_cursor != CARBON_PROP_ITER_STATE_DONE) {
        prop_iter_cursor_init(iter);
    }
    return iter->prop_cursor;
}

static void
prop_iter_state_init(carbon_archive_prop_iter_t *iter)
{
    iter->prop_cursor = CARBON_PROP_ITER_STATE_INIT;

}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_archive(carbon_archive_prop_iter_t *iter,
                                      carbon_err_t *err,
                                      uint16_t mask,
                                      carbon_archive_t *archive)
{
    CARBON_NON_NULL_OR_ERROR(iter)
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(archive)

    iter->archive = archive;
    iter->mask    = mask;
    if (!carbon_memfile_open(&iter->record_table_memfile, archive->record_table.recordDataBase,
                        CARBON_MEMFILE_MODE_READONLY)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILEOPEN_FAILED)
        return false;
    }
    if (!base_object_in_record_table(&iter->object, &iter->record_table_memfile)) {
        CARBON_ERROR(err, CARBON_ERR_INTERNALERR);
        return false;
    }

    carbon_error_init(&iter->err);
    prop_iter_state_init(iter);
    prop_iter_state_next(iter);

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_object(carbon_archive_prop_iter_t *iter,
                                     uint16_t mask,
                                     carbon_archive_object_t *obj)
{
    CARBON_UNUSED(iter);
    CARBON_UNUSED(mask);
    CARBON_UNUSED(obj);
    return false;
}

static carbon_type_e
get_basic_type(carbon_prop_iter_state_e state)
{
    switch (state) {
    case CARBON_PROP_ITER_STATE_NULLS:
    case CARBON_PROP_ITER_STATE_NULL_ARRAYS:
        return CARBON_TYPE_VOID;
    case CARBON_PROP_ITER_STATE_BOOLS:
    case CARBON_PROP_ITER_STATE_BOOL_ARRAYS:
        return CARBON_TYPE_BOOL;
    case CARBON_PROP_ITER_STATE_INT8S:
    case CARBON_PROP_ITER_STATE_INT8_ARRAYS:
        return CARBON_TYPE_INT8;
    case CARBON_PROP_ITER_STATE_INT16S:
    case CARBON_PROP_ITER_STATE_INT16_ARRAYS:
        return CARBON_TYPE_INT16;
    case CARBON_PROP_ITER_STATE_INT32S:
    case CARBON_PROP_ITER_STATE_INT32_ARRAYS:
        return CARBON_TYPE_INT32;
    case CARBON_PROP_ITER_STATE_INT64S:
    case CARBON_PROP_ITER_STATE_INT64_ARRAYS:
        return CARBON_TYPE_INT64;
    case CARBON_PROP_ITER_STATE_UINT8S:
    case CARBON_PROP_ITER_STATE_UINT8_ARRAYS:
        return CARBON_TYPE_UINT8;
    case CARBON_PROP_ITER_STATE_UINT16S:
    case CARBON_PROP_ITER_STATE_UINT16_ARRAYS:
        return CARBON_TYPE_UINT16;
    case CARBON_PROP_ITER_STATE_UINT32S:
    case CARBON_PROP_ITER_STATE_UINT32_ARRAYS:
        return CARBON_TYPE_UINT32;
    case CARBON_PROP_ITER_STATE_UINT64S:
    case CARBON_PROP_ITER_STATE_UINT64_ARRAYS:
        return CARBON_TYPE_UINT64;
    case CARBON_PROP_ITER_STATE_FLOATS:
    case CARBON_PROP_ITER_STATE_FLOAT_ARRAYS:
        return CARBON_TYPE_FLOAT;
    case CARBON_PROP_ITER_STATE_STRINGS:
    case CARBON_PROP_ITER_STATE_STRING_ARRAYS:
        return CARBON_TYPE_STRING;
    case CARBON_PROP_ITER_STATE_OBJECTS:
    case CARBON_PROP_ITER_STATE_OBJECT_ARRAYS:
        return CARBON_TYPE_OBJECT;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}

static bool
is_array_type(carbon_prop_iter_state_e state)
{
    switch (state) {
    case CARBON_PROP_ITER_STATE_NULLS:
    case CARBON_PROP_ITER_STATE_BOOLS:
    case CARBON_PROP_ITER_STATE_INT8S:
    case CARBON_PROP_ITER_STATE_INT16S:
    case CARBON_PROP_ITER_STATE_INT32S:
    case CARBON_PROP_ITER_STATE_INT64S:
    case CARBON_PROP_ITER_STATE_UINT8S:
    case CARBON_PROP_ITER_STATE_UINT16S:
    case CARBON_PROP_ITER_STATE_UINT32S:
    case CARBON_PROP_ITER_STATE_UINT64S:
    case CARBON_PROP_ITER_STATE_FLOATS:
    case CARBON_PROP_ITER_STATE_STRINGS:
    case CARBON_PROP_ITER_STATE_OBJECTS:
        return false;
    case CARBON_PROP_ITER_STATE_NULL_ARRAYS:
    case CARBON_PROP_ITER_STATE_BOOL_ARRAYS:
    case CARBON_PROP_ITER_STATE_INT8_ARRAYS:
    case CARBON_PROP_ITER_STATE_INT16_ARRAYS:
    case CARBON_PROP_ITER_STATE_INT32_ARRAYS:
    case CARBON_PROP_ITER_STATE_INT64_ARRAYS:
    case CARBON_PROP_ITER_STATE_UINT8_ARRAYS:
    case CARBON_PROP_ITER_STATE_UINT16_ARRAYS:
    case CARBON_PROP_ITER_STATE_UINT32_ARRAYS:
    case CARBON_PROP_ITER_STATE_UINT64_ARRAYS:
    case CARBON_PROP_ITER_STATE_FLOAT_ARRAYS:
    case CARBON_PROP_ITER_STATE_STRING_ARRAYS:
    case CARBON_PROP_ITER_STATE_OBJECT_ARRAYS:
        return true;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_next(carbon_string_id_t *key, carbon_archive_value_iter_t *value_iter,
                              carbon_archive_prop_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(iter)

    if (iter->prop_cursor != CARBON_PROP_ITER_STATE_DONE)
    {
        if (iter->prop_pos_current == iter->prop_header.header->num_entries)
        {
            prop_iter_state_next(iter);
            return carbon_archive_prop_iter_next(key, value_iter, iter);
        } else
        {
            iter->key = iter->prop_header.keys[iter->prop_pos_current];
            iter->type = get_basic_type(iter->prop_cursor);
            iter->is_array = is_array_type(iter->prop_cursor);
            CARBON_OPTIONAL_SET(key, iter->key);
            if (value_iter && !carbon_archive_value_iter_from_prop_iter(value_iter, &iter->err, iter))
            {
                CARBON_ERROR(&iter->err, CARBON_ERR_VITEROPEN_FAILED);
                return false;
            }
            iter->prop_pos_current++;
            return true;
        }
    } else
    {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_get_object_id(carbon_object_id_t *id, carbon_archive_prop_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(id)
    CARBON_NON_NULL_OR_ERROR(iter)
    *id = iter->object.object_id;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_from_prop_iter(carbon_archive_value_iter_t *value_iter,
                                         carbon_err_t *err,
                                         carbon_archive_prop_iter_t *prop_iter)
{
    CARBON_NON_NULL_OR_ERROR(value_iter);
    CARBON_NON_NULL_OR_ERROR(prop_iter);
    value_iter->prop_iter = prop_iter;
    if (!carbon_memfile_open(&value_iter->record_table_memfile, prop_iter->record_table_memfile.memblock,
                        CARBON_MEMFILE_MODE_READONLY)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILEOPEN_FAILED);
        return false;
    }
    if (!carbon_memfile_skip(&value_iter->record_table_memfile, prop_iter->prop_type_off_data)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILESKIP_FAILED);
        return false;
    }
    value_iter->prop_type = prop_iter->type;
    value_iter->is_array = prop_iter->is_array;
    carbon_error_init(&value_iter->err);

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_get_basic_type(carbon_type_e *type, const carbon_archive_value_iter_t *value_iter)
{
    CARBON_NON_NULL_OR_ERROR(type)
    CARBON_NON_NULL_OR_ERROR(value_iter)
    *type = value_iter->prop_type;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_is_array_type(bool *is_array, const carbon_archive_value_iter_t *value_iter)
{
    CARBON_NON_NULL_OR_ERROR(is_array)
    CARBON_NON_NULL_OR_ERROR(value_iter)
    *is_array = value_iter->is_array;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_position(uint32_t *pos, carbon_archive_value_iter_t *iter)
{
    CARBON_UNUSED(pos);
    CARBON_UNUSED(iter);
    return false;
}

CARBON_EXPORT(const void *)
carbon_archive_value_iter_values(uint32_t *num_values, carbon_archive_value_iter_t *iter)
{
    CARBON_UNUSED(num_values);
    CARBON_UNUSED(iter);
    return false;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_next_object(carbon_archive_prop_iter_t *iter, carbon_archive_value_iter_t *value_iter)
{
    CARBON_UNUSED(iter);
    CARBON_UNUSED(value_iter);
    return false;
}

CARBON_EXPORT(bool)
carbon_archive_value_iter_close_and_drop(carbon_archive_value_iter_t *iter)
{
    CARBON_UNUSED(iter);
    return false;
}


void
carbon_int_reset_cabin_object_mem_file(carbon_archive_object_t *object)
{
    CARBON_UNUSED(object);
  //  carbon_memfile_seek(&object->file, object->self);
    abort();
}
