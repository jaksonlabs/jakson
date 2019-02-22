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
#include <carbon/carbon-int-archive.h>

static bool
init_object_from_memfile(carbon_archive_object_t *obj, carbon_memfile_t *memfile)
{
    assert(obj);
    carbon_off_t                  object_off;
    carbon_object_header_t       *header;
    carbon_archive_object_flags_t flags;

    object_off = CARBON_MEMFILE_TELL(memfile);
    header = CARBON_MEMFILE_READ_TYPE(memfile, carbon_object_header_t);
    if (CARBON_UNLIKELY(header->marker != MARKER_SYMBOL_OBJECT_BEGIN)) {
        return false;
    }

    flags.value = header->flags;
    carbon_int_read_prop_offsets(&obj->prop_offsets, memfile, &flags);

    obj->object_id = header->oid;
    obj->offset = object_off;
    obj->next_obj_off = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);

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


static bool
carbon_archive_prop_iter_from_memblock(carbon_archive_prop_iter_t *iter,
                                       carbon_err_t *err,
                                       uint16_t mask,
                                       carbon_memblock_t *memblock,
                                       carbon_off_t object_offset)
{
    CARBON_NON_NULL_OR_ERROR(iter)
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(memblock)

    iter->mask = mask;
    if (!carbon_memfile_open(&iter->record_table_memfile, memblock,
                             CARBON_MEMFILE_MODE_READONLY)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILEOPEN_FAILED)
        return false;
    }
    if (!carbon_memfile_seek(&iter->record_table_memfile, object_offset))
    {
        CARBON_ERROR(err, CARBON_ERR_MEMFILESEEK_FAILED)
        return false;
    }
    if (!init_object_from_memfile(&iter->object, &iter->record_table_memfile)) {
        CARBON_ERROR(err, CARBON_ERR_INTERNALERR);
        return false;
    }

    carbon_error_init(&iter->err);
    prop_iter_state_init(iter);
    prop_iter_state_next(iter);

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_archive(carbon_archive_prop_iter_t *iter,
                                      carbon_err_t *err,
                                      uint16_t mask,
                                      carbon_archive_t *archive)
{
    return carbon_archive_prop_iter_from_memblock(iter, err, mask, archive->record_table.recordDataBase, 0);
}

CARBON_EXPORT(bool)
carbon_archive_prop_iter_from_object(carbon_archive_prop_iter_t *iter,
                                     uint16_t mask,
                                     carbon_archive_object_t *obj,
                                     carbon_archive_value_t *value)
{
    return carbon_archive_prop_iter_from_memblock(iter, &value->err, mask,
                                                  value->record_table_memfile.memblock, obj->offset);
}

static carbon_basic_type_e
get_basic_type(carbon_prop_iter_state_e state)
{
    switch (state) {
    case CARBON_PROP_ITER_STATE_NULLS:
    case CARBON_PROP_ITER_STATE_NULL_ARRAYS:
        return CARBON_BASIC_TYPE_NULL;
    case CARBON_PROP_ITER_STATE_BOOLS:
    case CARBON_PROP_ITER_STATE_BOOL_ARRAYS:
        return CARBON_BASIC_TYPE_BOOLEAN;
    case CARBON_PROP_ITER_STATE_INT8S:
    case CARBON_PROP_ITER_STATE_INT8_ARRAYS:
        return CARBON_BASIC_TYPE_INT8;
    case CARBON_PROP_ITER_STATE_INT16S:
    case CARBON_PROP_ITER_STATE_INT16_ARRAYS:
        return CARBON_BASIC_TYPE_INT16;
    case CARBON_PROP_ITER_STATE_INT32S:
    case CARBON_PROP_ITER_STATE_INT32_ARRAYS:
        return CARBON_BASIC_TYPE_INT32;
    case CARBON_PROP_ITER_STATE_INT64S:
    case CARBON_PROP_ITER_STATE_INT64_ARRAYS:
        return CARBON_BASIC_TYPE_INT64;
    case CARBON_PROP_ITER_STATE_UINT8S:
    case CARBON_PROP_ITER_STATE_UINT8_ARRAYS:
        return CARBON_BASIC_TYPE_UINT8;
    case CARBON_PROP_ITER_STATE_UINT16S:
    case CARBON_PROP_ITER_STATE_UINT16_ARRAYS:
        return CARBON_BASIC_TYPE_UINT16;
    case CARBON_PROP_ITER_STATE_UINT32S:
    case CARBON_PROP_ITER_STATE_UINT32_ARRAYS:
        return CARBON_BASIC_TYPE_UINT32;
    case CARBON_PROP_ITER_STATE_UINT64S:
    case CARBON_PROP_ITER_STATE_UINT64_ARRAYS:
        return CARBON_BASIC_TYPE_UINT64;
    case CARBON_PROP_ITER_STATE_FLOATS:
    case CARBON_PROP_ITER_STATE_FLOAT_ARRAYS:
        return CARBON_BASIC_TYPE_NUMBER;
    case CARBON_PROP_ITER_STATE_STRINGS:
    case CARBON_PROP_ITER_STATE_STRING_ARRAYS:
        return CARBON_BASIC_TYPE_STRING;
    case CARBON_PROP_ITER_STATE_OBJECTS:
    case CARBON_PROP_ITER_STATE_OBJECT_ARRAYS:
        return CARBON_BASIC_TYPE_OBJECT;
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
carbon_archive_prop_iter_next(carbon_string_id_t *key, carbon_archive_value_t *value,
                              carbon_archive_prop_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(iter)

    if (iter->prop_cursor != CARBON_PROP_ITER_STATE_DONE)
    {
        if (iter->prop_pos_current == iter->prop_header.header->num_entries)
        {
            prop_iter_state_next(iter);
            return carbon_archive_prop_iter_next(key, value, iter);
        } else
        {
            iter->key = iter->prop_header.keys[iter->prop_pos_current];
            iter->type = get_basic_type(iter->prop_cursor);
            iter->is_array = is_array_type(iter->prop_cursor);
            CARBON_OPTIONAL_SET(key, iter->key);
            if (value && !carbon_archive_value_from_prop_iter(value, &iter->err, iter))
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

static void
value_iter_setup_array_values(carbon_archive_value_t *iter)
{
    switch (iter->prop_type) {
    case CARBON_BASIC_TYPE_INT8:
        READ_BASIC_TYPE_AT_POSITION(int8, carbon_int8_t);
        break;
    case CARBON_BASIC_TYPE_INT16:
        READ_BASIC_TYPE_AT_POSITION(int16, carbon_int16_t);
        break;
    case CARBON_BASIC_TYPE_INT32:
        READ_BASIC_TYPE_AT_POSITION(int32, carbon_int32_t);
        break;
    case CARBON_BASIC_TYPE_INT64:
        READ_BASIC_TYPE_AT_POSITION(int64, carbon_int64_t);
        break;
    case CARBON_BASIC_TYPE_UINT8:
        READ_BASIC_TYPE_AT_POSITION(uint8, carbon_uint8_t);
        break;
    case CARBON_BASIC_TYPE_UINT16:
        READ_BASIC_TYPE_AT_POSITION(uint16, carbon_uint8_t);
        break;
    case CARBON_BASIC_TYPE_UINT32:
        READ_BASIC_TYPE_AT_POSITION(uint32, carbon_uint32_t);
        break;
    case CARBON_BASIC_TYPE_UINT64:
        READ_BASIC_TYPE_AT_POSITION(uint64, carbon_uint64_t);
        break;
    case CARBON_BASIC_TYPE_NUMBER:
        READ_BASIC_TYPE_AT_POSITION(number, carbon_float_t);
        break;
    case CARBON_BASIC_TYPE_STRING:
        READ_BASIC_TYPE_AT_POSITION(boolean, carbon_bool_t);
        break;
    case CARBON_BASIC_TYPE_BOOLEAN:
        READ_BASIC_TYPE_AT_POSITION(boolean, carbon_bool_t);
        break;
    case CARBON_BASIC_TYPE_NULL:
        /* nothing to setup for nulls */
        break;
    case CARBON_BASIC_TYPE_OBJECT: {
        //const carbon_off_t *offs = CARBON_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
        //                                                         carbon_off_t, iter->value_idx_max);
        //carbon_off_t start_obj = offs[0];
        //carbon_memfile_seek(&iter->record_table_memfile, start_obj);
        //init_object_from_memfile(&iter->data.basic.object, &iter->record_table_memfile);
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR); // TODO: Implement
    } break;
    default:
    CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}

#define READ_BASIC_TYPE_AT_POSITION(dst, type)                                                                         \
{                                                                                                                      \
    carbon_memfile_skip(&value->record_table_memfile, value->value_idx * sizeof(type));                                \
    value->data.basic.dst = *CARBON_MEMFILE_READ_TYPE(&value->record_table_memfile, type);                             \
}


static void
value_iter_setup_basic_values(carbon_archive_value_t *value)
{
    switch (value->prop_type) {
    case CARBON_BASIC_TYPE_INT8:
        READ_BASIC_TYPE_AT_POSITION(int8, carbon_int8_t);
    break;
    case CARBON_BASIC_TYPE_INT16:
        READ_BASIC_TYPE_AT_POSITION(int16, carbon_int16_t);
    break;
    case CARBON_BASIC_TYPE_INT32:
        READ_BASIC_TYPE_AT_POSITION(int32, carbon_int32_t);
    break;
    case CARBON_BASIC_TYPE_INT64:
        READ_BASIC_TYPE_AT_POSITION(int64, carbon_int64_t);
    break;
    case CARBON_BASIC_TYPE_UINT8:
        READ_BASIC_TYPE_AT_POSITION(uint8, carbon_uint8_t);
    break;
    case CARBON_BASIC_TYPE_UINT16:
        READ_BASIC_TYPE_AT_POSITION(uint16, carbon_uint8_t);
    break;
    case CARBON_BASIC_TYPE_UINT32:
        READ_BASIC_TYPE_AT_POSITION(uint32, carbon_uint32_t);
    break;
    case CARBON_BASIC_TYPE_UINT64:
        READ_BASIC_TYPE_AT_POSITION(uint64, carbon_uint64_t);
    break;
    case CARBON_BASIC_TYPE_NUMBER:
        READ_BASIC_TYPE_AT_POSITION(number, carbon_float_t);
    break;
    case CARBON_BASIC_TYPE_STRING:
        READ_BASIC_TYPE_AT_POSITION(boolean, carbon_bool_t);
    break;
    case CARBON_BASIC_TYPE_BOOLEAN:
        READ_BASIC_TYPE_AT_POSITION(boolean, carbon_bool_t);
    break;
    case CARBON_BASIC_TYPE_NULL:
        /* nothing to setup for nulls */
        break;
    case CARBON_BASIC_TYPE_OBJECT: {
        const carbon_off_t *offs = CARBON_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile,
                                                                 carbon_off_t, value->value_idx_max);
        carbon_off_t start_obj = offs[0];
        carbon_memfile_seek(&value->record_table_memfile, start_obj);
        init_object_from_memfile(&value->data.basic.object, &value->record_table_memfile);
    } break;
    default:
    CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}

CARBON_EXPORT(bool)
carbon_archive_value_from_prop_iter(carbon_archive_value_t *value,
                                    carbon_err_t *err,
                                    carbon_archive_prop_iter_t *prop_iter)
{
    CARBON_NON_NULL_OR_ERROR(value);
    CARBON_NON_NULL_OR_ERROR(prop_iter);
    value->prop_iter = prop_iter;
    value->data_off = prop_iter->prop_type_off_data;
    if (!carbon_memfile_open(&value->record_table_memfile, prop_iter->record_table_memfile.memblock,
                        CARBON_MEMFILE_MODE_READONLY)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILEOPEN_FAILED);
        return false;
    }
    if (!carbon_memfile_skip(&value->record_table_memfile, value->data_off)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILESKIP_FAILED);
        return false;
    }
    value->prop_type = prop_iter->type;
    value->is_array = prop_iter->is_array;
    value->value_idx = prop_iter->prop_pos_current;
    value->value_idx_max = prop_iter->prop_header.header->num_entries;
    carbon_error_init(&value->err);

    if (value->is_array) {
        value_iter_setup_array_values(value);
    } else {
        value_iter_setup_basic_values(value);
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_get_basic_type(carbon_basic_type_e *type, const carbon_archive_value_t *value)
{
    CARBON_NON_NULL_OR_ERROR(type)
    CARBON_NON_NULL_OR_ERROR(value)
    *type = value->prop_type;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_is_array_type(bool *is_array, const carbon_archive_value_t *value)
{
    CARBON_NON_NULL_OR_ERROR(is_array)
    CARBON_NON_NULL_OR_ERROR(value)
    *is_array = value->is_array;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_is_object(bool *is_object, carbon_archive_value_t *value)
{
    CARBON_NON_NULL_OR_ERROR(is_object)
    CARBON_NON_NULL_OR_ERROR(value)

    *is_object = value->prop_type == CARBON_BASIC_TYPE_OBJECT && !value->is_array;

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_get_object(carbon_archive_object_t *object, carbon_archive_value_t *value)
{
    CARBON_NON_NULL_OR_ERROR(object)
    CARBON_NON_NULL_OR_ERROR(value)

    bool is_object;

    carbon_archive_value_is_object(&is_object, value);

    if (is_object) {
        *object = value->data.basic.object;
        return true;
    } else {
        CARBON_ERROR(&value->err, CARBON_ERR_ITER_NOOBJ);
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_archive_value_is_null(bool *is_null, carbon_archive_value_t *value)
{
    CARBON_NON_NULL_OR_ERROR(is_null)
    CARBON_NON_NULL_OR_ERROR(value)

    *is_null = value->prop_type == CARBON_BASIC_TYPE_NULL && !value->is_array;

    return true;
}

#define DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(type, name, basic_type, error_code)                                          \
                                                                                                                       \
CARBON_EXPORT(bool)                                                                                                    \
carbon_archive_value_is_##name(bool *is_##name, carbon_archive_value_t *value)                                         \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(is_##name)                                                                                \
    CARBON_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    *is_##name = value->prop_type == basic_type && !value->is_array;                                                   \
                                                                                                                       \
    return true;                                                                                                       \
}                                                                                                                      \
                                                                                                                       \
CARBON_EXPORT(bool)                                                                                                    \
carbon_archive_value_get_##name(type *name, carbon_archive_value_t *value)                                             \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(name)                                                                                     \
    CARBON_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    bool type_match;                                                                                                   \
                                                                                                                       \
    carbon_archive_value_is_##name(&type_match, value);                                                                \
    if (type_match) {                                                                                                  \
        *name = value->data.basic.name;                                                                                \
        return true;                                                                                                   \
    } else {                                                                                                           \
        CARBON_ERROR(&value->err, error_code);                                                                         \
        return false;                                                                                                  \
    }                                                                                                                  \
}

DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int8_t, int8, CARBON_BASIC_TYPE_INT8, CARBON_ERR_ITER_NOINT8)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int16_t, int16, CARBON_BASIC_TYPE_INT16, CARBON_ERR_ITER_NOINT16)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int32_t, int32, CARBON_BASIC_TYPE_INT32, CARBON_ERR_ITER_NOINT32)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_int64_t, int64, CARBON_BASIC_TYPE_INT64, CARBON_ERR_ITER_NOINT64)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint8_t, uint8, CARBON_BASIC_TYPE_UINT8, CARBON_ERR_ITER_NOUINT8)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint16_t, uint16, CARBON_BASIC_TYPE_UINT16, CARBON_ERR_ITER_NOUINT16)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint32_t, uint32, CARBON_BASIC_TYPE_UINT32, CARBON_ERR_ITER_NOUINT32)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_uint64_t, uint64, CARBON_BASIC_TYPE_UINT64, CARBON_ERR_ITER_NOUINT64)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_float_t, number, CARBON_BASIC_TYPE_NUMBER, CARBON_ERR_ITER_NONUMBER)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_string_id_t, string, CARBON_BASIC_TYPE_STRING, CARBON_ERR_ITER_NOSTRING)
DEFINE_ARCHIVE_BASIC_TYPE_GETTERS(carbon_bool_t, boolean, CARBON_BASIC_TYPE_BOOLEAN, CARBON_ERR_ITER_NOBOOL)


//
//
//CARBON_EXPORT(bool)
//carbon_archive_value_iter_position(uint32_t *pos, carbon_archive_value_iter_t *iter)
//{
//    CARBON_UNUSED(pos);
//    CARBON_UNUSED(iter);
//    return false;
//}
//
//CARBON_EXPORT(const void *)
//carbon_archive_value_iter_values(uint32_t *num_values, carbon_archive_value_iter_t *iter)
//{
//    CARBON_UNUSED(num_values);
//    CARBON_UNUSED(iter);
//    return false;
//}
//
//CARBON_EXPORT(bool)
//carbon_archive_value_iter_next_object(carbon_archive_prop_iter_t *iter, carbon_archive_value_iter_t *value_iter)
//{
//    CARBON_UNUSED(iter);
//    CARBON_UNUSED(value_iter);
//    return false;
//}
//
//CARBON_EXPORT(bool)
//carbon_archive_value_iter_close_and_drop(carbon_archive_value_iter_t *iter)
//{
//    CARBON_UNUSED(iter);
//    return false;
//}


void
carbon_int_reset_cabin_object_mem_file(carbon_archive_object_t *object)
{
    CARBON_UNUSED(object);
  //  carbon_memfile_seek(&object->file, object->self);
    abort();
}
