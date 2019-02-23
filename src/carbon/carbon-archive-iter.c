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
    carbon_memfile_open(&obj->memfile, memfile->memblock, CARBON_MEMFILE_MODE_READONLY);

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

__unused static bool // TODO: remove __unused
prop_iter_read_colum_entry(carbon_archive_collection_iter_state_t *state, carbon_memfile_t *memfile)
{
    assert(state->current_column_group.current_column.current_entry.idx <
        state->current_column_group.current_column.num_elem);

    uint32_t     current_idx = state->current_column_group.current_column.current_entry.idx;
    carbon_off_t entry_off   = state->current_column_group.current_column.elem_offsets[current_idx];
    carbon_memfile_seek(memfile, entry_off);

    state->current_column_group.current_column.current_entry.array_length =
                        *CARBON_MEMFILE_READ_TYPE(memfile, uint32_t);
    state->current_column_group.current_column.current_entry.array_base =
                        CARBON_MEMFILE_PEEK(memfile, void);

    return (++state->current_column_group.current_column.current_entry.idx) <
        state->current_column_group.current_column.num_elem;
}

__unused static bool // TODO: remove __unused
prop_iter_read_column(carbon_archive_collection_iter_state_t *state, carbon_memfile_t *memfile)
{
    assert(state->current_column_group.current_column.idx <
           state->current_column_group.num_columns);

    uint32_t     current_idx = state->current_column_group.current_column.idx;
    carbon_off_t column_off  = state->current_column_group.column_offs[current_idx];
    carbon_memfile_seek(memfile, column_off);
    const carbon_column_header_t *header = CARBON_MEMFILE_READ_TYPE(memfile,
                                                                    carbon_column_header_t);

    assert(header->marker == MARKER_SYMBOL_COLUMN);
    state->current_column_group.current_column.name = header->column_name;
    state->current_column_group.current_column.type =
               carbon_int_field_type_to_basic_type(carbon_int_marker_to_field_type(header->value_type));

    state->current_column_group.current_column.num_elem = header->num_entries;
    state->current_column_group.current_column.elem_offsets = CARBON_MEMFILE_READ_TYPE_LIST(
                                           memfile, carbon_off_t, header->num_entries);
    state->current_column_group.current_column.elem_positions = CARBON_MEMFILE_READ_TYPE_LIST(
                                           memfile, uint32_t, header->num_entries);
    state->current_column_group.current_column.current_entry.idx = 0;

    return (++state->current_column_group.current_column.idx) <
        state->current_column_group.num_columns;
}

static bool
collection_iter_read_next_column_group(carbon_archive_collection_iter_state_t *state, carbon_memfile_t *memfile)
{
    assert(state->current_column_group_idx < state->num_column_groups);
    carbon_memfile_seek(memfile,
                        state->column_group_offsets[state->current_column_group_idx]);
    const carbon_column_group_header_t *header = CARBON_MEMFILE_READ_TYPE(memfile,
                                                                          carbon_column_group_header_t);
    assert(header->marker == MARKER_SYMBOL_COLUMN_GROUP);
    state->current_column_group.num_columns = header->num_columns;
    state->current_column_group.num_objects = header->num_objects;
    state->current_column_group.object_ids  = CARBON_MEMFILE_READ_TYPE_LIST(memfile,
                                                                carbon_object_id_t, header->num_objects);
    state->current_column_group.column_offs = CARBON_MEMFILE_READ_TYPE_LIST(memfile,
                                                                carbon_off_t, header->num_columns);
    state->current_column_group.current_column.idx = 0;

    return (++state->current_column_group_idx) < state->num_column_groups;
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



    if (iter->mode == CARBON_ARCHIVE_PROP_ITER_MODE_COLLECTION)
    {
        iter->mode_collection.collection_start_off = offset_by_state(iter);
        carbon_memfile_seek(&iter->record_table_memfile, iter->mode_collection.collection_start_off);
        const carbon_object_array_header_t *header = CARBON_MEMFILE_READ_TYPE(&iter->record_table_memfile, carbon_object_array_header_t);
        iter->mode_collection.num_column_groups = header->num_entries;
        iter->mode_collection.current_column_group_idx = 0;
        iter->mode_collection.column_group_keys = CARBON_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                carbon_string_id_t,
                                                                                iter->mode_collection.num_column_groups);
        iter->mode_collection.column_group_offsets = CARBON_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                carbon_off_t,
                                                                                iter->mode_collection.num_column_groups);

    } else
    {
        iter->mode_object.current_prop_group_off = offset_by_state(iter);
        carbon_memfile_seek(&iter->record_table_memfile, iter->mode_object.current_prop_group_off);
        carbon_int_embedded_fixed_props_read(&iter->mode_object.prop_group_header, &iter->record_table_memfile);
        iter->mode_object.prop_data_off = CARBON_MEMFILE_TELL(&iter->record_table_memfile);
    }

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

    iter->mode = iter->prop_cursor == CARBON_PROP_ITER_STATE_OBJECT_ARRAYS ?
                 CARBON_ARCHIVE_PROP_ITER_MODE_COLLECTION : CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT;

    if (iter->prop_cursor != CARBON_PROP_ITER_STATE_DONE) {
        prop_iter_cursor_init(iter);
    }
    return iter->prop_cursor;
}

static void
prop_iter_state_init(carbon_archive_prop_iter_t *iter)
{
    iter->prop_cursor = CARBON_PROP_ITER_STATE_INIT;
    iter->mode = CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT;
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
                                     carbon_err_t *err,
                                     const carbon_archive_object_t *obj)
{
    return carbon_archive_prop_iter_from_memblock(iter, err, mask,
                                                  obj->memfile.memblock, obj->offset);
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
carbon_archive_prop_iter_next(carbon_archive_prop_iter_mode_e *type,
                              carbon_archive_value_vector_t *value_vector,
                              carbon_archive_collection_iter_t *collection_iter,
                              carbon_archive_prop_iter_t *prop_iter)
{
    CARBON_NON_NULL_OR_ERROR(type);
    CARBON_NON_NULL_OR_ERROR(prop_iter);

    if (prop_iter->prop_cursor != CARBON_PROP_ITER_STATE_DONE)
    {
        switch (prop_iter->mode) {
        case CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT: {
                value_vector->keys = prop_iter->mode_object.prop_group_header.keys;

                prop_iter->mode_object.type = get_basic_type(prop_iter->prop_cursor);
                prop_iter->mode_object.is_array = is_array_type(prop_iter->prop_cursor);

                value_vector->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;
                value_vector->prop_type = prop_iter->mode_object.type;
                value_vector->is_array = prop_iter->mode_object.is_array;

                if (value_vector && !carbon_archive_value_vector_from_prop_iter(value_vector, &prop_iter->err, prop_iter))
                {
                    CARBON_ERROR(&prop_iter->err, CARBON_ERR_VITEROPEN_FAILED);
                    return false;
                }
        } break;
        case CARBON_ARCHIVE_PROP_ITER_MODE_COLLECTION: {
            collection_iter->state = prop_iter->mode_collection;
            carbon_memfile_open(&collection_iter->record_table_memfile, prop_iter->record_table_memfile.memblock,
                                CARBON_MEMFILE_MODE_READONLY);
            carbon_error_init(&collection_iter->err);
        } break;
        default:
            CARBON_ERROR(&prop_iter->err, CARBON_ERR_INTERNALERR);
            return false;
        }
        *type = prop_iter->mode;
        prop_iter_state_next(prop_iter);
        return true;
    } else
    {
        return false;
    }
}

CARBON_EXPORT(const carbon_string_id_t *)
carbon_archive_collection_iter_get_keys(uint32_t *num_keys, carbon_archive_collection_iter_t *iter)
{
    if (num_keys && iter) {
        *num_keys = iter->state.num_column_groups;
        return iter->state.column_group_keys;
    } else {
        CARBON_ERROR(&iter->err, CARBON_ERR_NULLPTR);
        return NULL;
    }
}

CARBON_EXPORT(bool)
carbon_archive_collection_next_column_group(carbon_archive_column_group_iter_t *group_iter,
                                            carbon_archive_collection_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(group_iter)
    CARBON_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group_idx < iter->state.num_column_groups) {
        collection_iter_read_next_column_group(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&group_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            CARBON_MEMFILE_MODE_READONLY);
        group_iter->state = iter->state;
        carbon_error_init(&group_iter->err);
        return true;
    } else {
        return false;
    }
}

CARBON_EXPORT(const carbon_object_id_t *)
carbon_archive_column_group_get_object_ids(uint32_t *num_objects, carbon_archive_column_group_iter_t *iter)
{
    if (num_objects && iter) {
        *num_objects = iter->state.current_column_group.num_objects;
        return iter->state.current_column_group.object_ids;
    } else {
        CARBON_ERROR(&iter->err, CARBON_ERR_NULLPTR);
        return NULL;
    }
}


CARBON_EXPORT(bool)
carbon_archive_column_group_next_column(carbon_archive_column_iter_t *column_iter,
                                        carbon_archive_column_group_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(column_iter)
    CARBON_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group.current_column.idx < iter->state.current_column_group.num_columns) {
        prop_iter_read_column(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&column_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            CARBON_MEMFILE_MODE_READONLY);
        column_iter->state = iter->state;
        carbon_error_init(&column_iter->err);
        return true;
    } else {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_archive_column_get_name(carbon_string_id_t *name,
                               carbon_basic_type_e *type,
                               carbon_archive_column_iter_t *column_iter)
{
    CARBON_NON_NULL_OR_ERROR(column_iter)
    CARBON_OPTIONAL_SET(name, column_iter->state.current_column_group.current_column.name)
    CARBON_OPTIONAL_SET(type, column_iter->state.current_column_group.current_column.type)
    return true;
}

CARBON_EXPORT(const uint32_t *)
carbon_archive_column_get_entry_positions(uint32_t *num_entry, carbon_archive_column_iter_t *column_iter)
{
    if (num_entry && column_iter) {
        *num_entry = column_iter->state.current_column_group.current_column.num_elem;
        return column_iter->state.current_column_group.current_column.elem_positions;
    } else {
        CARBON_ERROR(&column_iter->err, CARBON_ERR_NULLPTR);
        return NULL;
    }
}

CARBON_EXPORT(bool)
carbon_archive_column_next_entry(carbon_archive_column_entry_iter_t *entry_iter, carbon_archive_column_iter_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(entry_iter)
    CARBON_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group.current_column.current_entry.idx <
        iter->state.current_column_group.current_column.num_elem)
    {
        prop_iter_read_colum_entry(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&entry_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            CARBON_MEMFILE_MODE_READONLY);
        entry_iter->state = iter->state;
        carbon_error_init(&entry_iter->err);
        return true;
    } else {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_archive_column_entry_get_type(carbon_basic_type_e *type, carbon_archive_column_entry_iter_t *entry)
{
    CARBON_NON_NULL_OR_ERROR(type)
    CARBON_NON_NULL_OR_ERROR(entry)
    *type = entry->state.current_column_group.current_column.type;
    return true;
}

#define DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name, basic_type)                            \
CARBON_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_column_entry_get_##name(uint32_t *array_length, carbon_archive_column_entry_iter_t *entry)              \
{                                                                                                                      \
    if (array_length && entry) {                                                                                       \
        if (entry->state.current_column_group.current_column.type == basic_type)                                       \
        {                                                                                                              \
            *array_length =  entry->state.current_column_group.current_column.current_entry.array_length;              \
            return (const built_in_type *) entry->state.current_column_group.current_column.current_entry.array_base;  \
        } else {                                                                                                       \
            CARBON_ERROR(&entry->err, CARBON_ERR_TYPEMISMATCH);                                                        \
            return NULL;                                                                                               \
        }                                                                                                              \
    } else {                                                                                                           \
        CARBON_ERROR(&entry->err, CARBON_ERR_NULLPTR);                                                                 \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_int8_t, int8s, CARBON_BASIC_TYPE_INT8);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_int16_t, int16s, CARBON_BASIC_TYPE_INT16);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_int32_t, int32s, CARBON_BASIC_TYPE_INT32);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_int64_t, int64s, CARBON_BASIC_TYPE_INT64);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_uint8_t, uint8s, CARBON_BASIC_TYPE_UINT8);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_uint16_t, uint16s, CARBON_BASIC_TYPE_UINT16);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_uint32_t, uint32s, CARBON_BASIC_TYPE_UINT32);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_uint64_t, uint64s, CARBON_BASIC_TYPE_UINT64);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_string_id_t, strings, CARBON_BASIC_TYPE_STRING);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_number_t, numbers, CARBON_BASIC_TYPE_NUMBER);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_boolean_t, booleans, CARBON_BASIC_TYPE_BOOLEAN);
DECLARE_CARBON_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_uint32_t, nulls, CARBON_BASIC_TYPE_NULL);

CARBON_EXPORT(bool)
carbon_archive_column_entry_get_objects(carbon_archive_column_entry_object_iter_t *iter,
                                        carbon_archive_column_entry_iter_t *entry)
{
    CARBON_NON_NULL_OR_ERROR(iter)
    CARBON_NON_NULL_OR_ERROR(entry)

    iter->entry_state = entry->state;
    carbon_memfile_open(&iter->memfile, entry->record_table_memfile.memblock, CARBON_MEMFILE_MODE_READONLY);
    carbon_memfile_seek(&iter->memfile, entry->state.current_column_group.current_column.elem_offsets[entry->state.current_column_group.current_column.current_entry.idx - 1] + sizeof(uint32_t));
    iter->next_obj_off = CARBON_MEMFILE_TELL(&iter->memfile);
    carbon_error_init(&iter->err);

    return true;
}

CARBON_EXPORT(const carbon_archive_object_t *)
carbon_archive_column_entry_object_iter_next_object(carbon_archive_column_entry_object_iter_t *iter)
{
    if (iter) {
        if (iter->next_obj_off != 0) {
            carbon_memfile_seek(&iter->memfile, iter->next_obj_off);
            if (init_object_from_memfile(&iter->obj, &iter->memfile)) {
                iter->next_obj_off = iter->obj.next_obj_off;
                return &iter->obj;
            } else {
                CARBON_ERROR(&iter->err, CARBON_ERR_INTERNALERR);
                return NULL;
            }
        } else {
            return NULL;
        }
    } else {
        CARBON_ERROR(&iter->err, CARBON_ERR_NULLPTR);
        return NULL;
    }
}

CARBON_EXPORT(bool)
carbon_archive_object_get_object_id(carbon_object_id_t *id, const carbon_archive_object_t *object)
{
    CARBON_NON_NULL_OR_ERROR(id)
    CARBON_NON_NULL_OR_ERROR(object)
    *id = object->object_id;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_object_get_prop_iter(carbon_archive_prop_iter_t *iter, const carbon_archive_object_t *object)
{
    // XXXX carbon_archive_prop_iter_from_object()
    CARBON_UNUSED(iter);
    CARBON_UNUSED(object);
    return false;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_get_object_id(carbon_object_id_t *id, const carbon_archive_value_vector_t *iter)
{
    CARBON_NON_NULL_OR_ERROR(id)
    CARBON_NON_NULL_OR_ERROR(iter)
    *id = iter->object_id;
    return true;
}

CARBON_EXPORT(const carbon_string_id_t *)
carbon_archive_value_vector_get_keys(uint32_t *num_keys, carbon_archive_value_vector_t *iter)
{
    if (num_keys && iter) {
        *num_keys = iter->value_max_idx;
        return iter->keys;
    } else {
        CARBON_ERROR(&iter->err, CARBON_ERR_NULLPTR)
        return NULL;
    }
}

static void
value_vector_init_object_basic(carbon_archive_value_vector_t *value)
{
    value->data.object.offsets = CARBON_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, carbon_off_t,
                                                               value->value_max_idx);
}

static void
value_vector_init_object_array(carbon_archive_value_vector_t *value)
{
    CARBON_UNUSED(value);
    abort(); // TODO: Implement XXX
}

static void
value_vector_init_fixed_length_types_basic(carbon_archive_value_vector_t *value)
{
    assert(!value->is_array);

    switch (value->prop_type) {
    case CARBON_BASIC_TYPE_INT8:
        value->data.basic.values.int8s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int8_t);
        break;
    case CARBON_BASIC_TYPE_INT16:
        value->data.basic.values.int16s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int16_t);
        break;
    case CARBON_BASIC_TYPE_INT32:
        value->data.basic.values.int32s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int32_t);
        break;
    case CARBON_BASIC_TYPE_INT64:
        value->data.basic.values.int64s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int64_t);
        break;
    case CARBON_BASIC_TYPE_UINT8:
        value->data.basic.values.uint8s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint8_t);
        break;
    case CARBON_BASIC_TYPE_UINT16:
        value->data.basic.values.uint16s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint16_t);
        break;
    case CARBON_BASIC_TYPE_UINT32:
        value->data.basic.values.uint32s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint32_t);
        break;
    case CARBON_BASIC_TYPE_UINT64:
        value->data.basic.values.uint64s = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint64_t);
        break;
    case CARBON_BASIC_TYPE_NUMBER:
        value->data.basic.values.numbers = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_number_t);
        break;
    case CARBON_BASIC_TYPE_STRING:
        value->data.basic.values.strings = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_string_id_t);
        break;
    case CARBON_BASIC_TYPE_BOOLEAN:
        value->data.basic.values.booleans = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_boolean_t);
        break;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}

static void
value_vector_init_fixed_length_types_null_arrays(carbon_archive_value_vector_t *value)
{
    assert(value->is_array);
    assert(value->prop_type == CARBON_BASIC_TYPE_NULL);
    value->data.arrays.meta.num_nulls_contained = CARBON_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, uint32_t,
                                                                                value->value_max_idx);
}

static void
value_vector_init_fixed_length_types_non_null_arrays(carbon_archive_value_vector_t *value)
{
    assert (value->is_array);

    value->data.arrays.meta.array_lengths = CARBON_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, uint32_t,
                                                                     value->value_max_idx);

    switch (value->prop_type) {
    case CARBON_BASIC_TYPE_INT8:
        value->data.arrays.values.int8s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int8_t);
        break;
    case CARBON_BASIC_TYPE_INT16:
        value->data.arrays.values.int16s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int16_t);
        break;
    case CARBON_BASIC_TYPE_INT32:
        value->data.arrays.values.int32s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int32_t);
        break;
    case CARBON_BASIC_TYPE_INT64:
        value->data.arrays.values.int64s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_int64_t);
        break;
    case CARBON_BASIC_TYPE_UINT8:
        value->data.arrays.values.uint8s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint8_t);
        break;
    case CARBON_BASIC_TYPE_UINT16:
        value->data.arrays.values.uint16s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint16_t);
        break;
    case CARBON_BASIC_TYPE_UINT32:
        value->data.arrays.values.uint32s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint32_t);
        break;
    case CARBON_BASIC_TYPE_UINT64:
        value->data.arrays.values.uint64s_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_uint64_t);
        break;
    case CARBON_BASIC_TYPE_NUMBER:
        value->data.arrays.values.numbers_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_number_t);
        break;
    case CARBON_BASIC_TYPE_STRING:
        value->data.arrays.values.strings_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_string_id_t);
        break;
    case CARBON_BASIC_TYPE_BOOLEAN:
        value->data.arrays.values.booleans_base = CARBON_MEMFILE_PEEK(&value->record_table_memfile, carbon_boolean_t);
        break;
    default:
    CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }
}


static void
value_vector_init_fixed_length_types(carbon_archive_value_vector_t *value)
{
    if (value->is_array)
    {
        value_vector_init_fixed_length_types_non_null_arrays(value);
    } else
    {
        value_vector_init_fixed_length_types_basic(value);
    }
}


static void
value_vector_init_object(carbon_archive_value_vector_t *value)
{
    if (value->is_array)
    {
        value_vector_init_object_array(value);
    } else
    {
        value_vector_init_object_basic(value);
    }
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_from_prop_iter(carbon_archive_value_vector_t *value,
                                           carbon_err_t *err,
                                           carbon_archive_prop_iter_t *prop_iter)
{
    CARBON_NON_NULL_OR_ERROR(value);
    CARBON_NON_NULL_OR_ERROR(prop_iter);

    CARBON_ERROR_IF_AND_RETURN (prop_iter->mode != CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT, &prop_iter->err,
                                CARBON_ERR_ITER_OBJECT_NEEDED, false)

    carbon_error_init(&value->err);

    value->prop_iter = prop_iter;
    value->data_off = prop_iter->mode_object.prop_data_off;
    value->object_id = prop_iter->object.object_id;

    if (!carbon_memfile_open(&value->record_table_memfile, prop_iter->record_table_memfile.memblock,
                        CARBON_MEMFILE_MODE_READONLY)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILEOPEN_FAILED);
        return false;
    }
    if (!carbon_memfile_skip(&value->record_table_memfile, value->data_off)) {
        CARBON_ERROR(err, CARBON_ERR_MEMFILESKIP_FAILED);
        return false;
    }

    value->prop_type = prop_iter->mode_object.type;
    value->is_array = prop_iter->mode_object.is_array;
    value->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;

    switch (value->prop_type) {
    case CARBON_BASIC_TYPE_OBJECT:
        value_vector_init_object(value);
        break;
    case CARBON_BASIC_TYPE_NULL:
        if (value->is_array) {
            value_vector_init_fixed_length_types_null_arrays(value);
        }
        break;
    case CARBON_BASIC_TYPE_INT8:
    case CARBON_BASIC_TYPE_INT16:
    case CARBON_BASIC_TYPE_INT32:
    case CARBON_BASIC_TYPE_INT64:
    case CARBON_BASIC_TYPE_UINT8:
    case CARBON_BASIC_TYPE_UINT16:
    case CARBON_BASIC_TYPE_UINT32:
    case CARBON_BASIC_TYPE_UINT64:
    case CARBON_BASIC_TYPE_NUMBER:
    case CARBON_BASIC_TYPE_STRING:
    case CARBON_BASIC_TYPE_BOOLEAN:
        value_vector_init_fixed_length_types(value);
        break;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_get_basic_type(carbon_basic_type_e *type, const carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(type)
    CARBON_NON_NULL_OR_ERROR(value)
    *type = value->prop_type;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_is_array_type(bool *is_array, const carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(is_array)
    CARBON_NON_NULL_OR_ERROR(value)
    *is_array = value->is_array;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_get_length(uint32_t *length, const carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(length)
    CARBON_NON_NULL_OR_ERROR(value)
    *length = value->value_max_idx;
    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_is_of_objects(bool *is_object, carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(is_object)
    CARBON_NON_NULL_OR_ERROR(value)

    *is_object = value->prop_type == CARBON_BASIC_TYPE_OBJECT && !value->is_array;

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_value_vector_get_object_at(carbon_archive_object_t *object, uint32_t idx,
                                          carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(object)
    CARBON_NON_NULL_OR_ERROR(value)

    if (idx >= value->value_max_idx) {
        CARBON_ERROR(&value->err, CARBON_ERR_OUTOFBOUNDS);
        return false;
    }

    bool is_object;

    carbon_archive_value_vector_is_of_objects(&is_object, value);

    if (is_object) {
        carbon_memfile_seek(&value->record_table_memfile, value->data.object.offsets[idx]);
        init_object_from_memfile(&value->data.object.object, &value->record_table_memfile);
        *object = value->data.object.object;
        return true;
    } else {
        CARBON_ERROR(&value->err, CARBON_ERR_ITER_NOOBJ);
        return false;
    }
}

#define DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name, basic_type)                                            \
CARBON_EXPORT(bool)                                                                                                    \
carbon_archive_value_vector_is_##name(bool *type_match, carbon_archive_value_vector_t *value)                          \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(type_match)                                                                               \
    CARBON_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    *type_match = value->prop_type == basic_type;                                                                      \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8, CARBON_BASIC_TYPE_INT8)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16, CARBON_BASIC_TYPE_INT16)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32, CARBON_BASIC_TYPE_INT32)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64, CARBON_BASIC_TYPE_INT64)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8, CARBON_BASIC_TYPE_UINT8)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16, CARBON_BASIC_TYPE_UINT16)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32, CARBON_BASIC_TYPE_UINT32)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64, CARBON_BASIC_TYPE_UINT64)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string, CARBON_BASIC_TYPE_STRING)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number, CARBON_BASIC_TYPE_NUMBER)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean, CARBON_BASIC_TYPE_BOOLEAN)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null, CARBON_BASIC_TYPE_NULL)

#define DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(names, name, built_in_type, err_code)                       \
CARBON_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_value_vector_get_##names(uint32_t *num_values, carbon_archive_value_vector_t *value)                    \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (carbon_archive_value_vector_is_array_type(&is_array, value) &&                                                 \
        carbon_archive_value_vector_is_##name(&type_match, value) && !is_array)                                        \
    {                                                                                                                  \
        *num_values = value->value_max_idx;                                                                            \
        return value->data.basic.values.names;                                                                         \
    } else                                                                                                             \
    {                                                                                                                  \
        CARBON_ERROR(&value->err, err_code);                                                                           \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}


DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, int8, carbon_int8_t, CARBON_ERR_ITER_NOINT8)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, int16, carbon_int16_t, CARBON_ERR_ITER_NOINT16)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, int32, carbon_int32_t, CARBON_ERR_ITER_NOINT32)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, int64, carbon_int64_t, CARBON_ERR_ITER_NOINT64)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, uint8, carbon_uint8_t, CARBON_ERR_ITER_NOUINT8)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, uint16, carbon_uint16_t, CARBON_ERR_ITER_NOUINT16)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, uint32, carbon_uint32_t, CARBON_ERR_ITER_NOUINT32)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, uint64, carbon_uint64_t, CARBON_ERR_ITER_NOUINT64)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, string, carbon_string_id_t, CARBON_ERR_ITER_NOSTRING)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, number, carbon_number_t, CARBON_ERR_ITER_NONUMBER)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, boolean, carbon_boolean_t, CARBON_ERR_ITER_NOBOOL)

CARBON_EXPORT(const carbon_uint32_t *)
carbon_archive_value_vector_get_null_arrays(uint32_t *num_values, carbon_archive_value_vector_t *value)
{
    CARBON_NON_NULL_OR_ERROR(value)

    bool is_array;
    bool type_match;

    if (carbon_archive_value_vector_is_array_type(&is_array, value) &&
        carbon_archive_value_vector_is_null(&type_match, value) && is_array)
    {
        *num_values = value->value_max_idx;
        return value->data.arrays.meta.num_nulls_contained;
    } else
    {
        return NULL;
    }
}

#define DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type, base)                               \
CARBON_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_value_vector_get_##name##_arrays_at(uint32_t *array_length, uint32_t idx,                               \
                                               carbon_archive_value_vector_t *value)                                   \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (idx < value->value_max_idx && carbon_archive_value_vector_is_array_type(&is_array, value) &&                   \
        carbon_archive_value_vector_is_##name(&type_match, value) && is_array)                                         \
    {                                                                                                                  \
        uint32_t skip_length = 0;                                                                                      \
        for (uint32_t i = 0; i < idx; i++) {                                                                           \
            skip_length += value->data.arrays.meta.array_lengths[i];                                                   \
        }                                                                                                              \
        *array_length = value->data.arrays.meta.array_lengths[idx];                                                    \
        return value->data.arrays.values.base + skip_length;                                                           \
    } else                                                                                                             \
    {                                                                                                                  \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, carbon_int8_t, int8s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, carbon_int16_t, int16s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, carbon_int32_t, int32s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, carbon_int64_t, int64s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, carbon_uint8_t, uint8s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, carbon_uint16_t, uint16s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, carbon_uint32_t, uint32s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, carbon_uint64_t, uint64s_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, carbon_string_id_t, strings_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, carbon_number_t, numbers_base)
DECLARE_CARBON_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, carbon_boolean_t, booleans_base)


void
carbon_int_reset_cabin_object_mem_file(carbon_archive_object_t *object)
{
    CARBON_UNUSED(object);
  //  carbon_memfile_seek(&object->file, object->self);
    abort();
}
