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

#include "core/carbon/archive_iter.h"
#include "core/carbon/archive_int.h"

static bool
init_object_from_memfile(carbon_archive_object_t *obj, memfile_t *memfile)
{
    assert(obj);
    offset_t                  object_off;
    carbon_object_header_t       *header;
    carbon_archive_object_flags_t flags;

    object_off = memfile_tell(memfile);
    header = NG5_MEMFILE_READ_TYPE(memfile, carbon_object_header_t);
    if (NG5_UNLIKELY(header->marker != MARKER_SYMBOL_OBJECT_BEGIN)) {
        return false;
    }

    flags.value = header->flags;
    carbon_int_read_prop_offsets(&obj->prop_offsets, memfile, &flags);

    obj->object_id = header->oid;
    obj->offset = object_off;
    obj->next_obj_off = *NG5_MEMFILE_READ_TYPE(memfile, offset_t);
    carbon_memfile_open(&obj->memfile, memfile->memblock, NG5_MEMFILE_MODE_READONLY);

    return true;
}

#define STATE_AND_PROPERTY_EXISTS(state, property) \
    (iter->prop_cursor != state || iter->object.property != 0)

inline static offset_t
offset_by_state(carbon_archive_prop_iter_t *iter)
{
    switch (iter->prop_cursor) {
    case NG5_PROP_ITER_STATE_NULLS:
        return iter->object.prop_offsets.nulls;
    case NG5_PROP_ITER_STATE_BOOLS:
        return iter->object.prop_offsets.bools;
    case NG5_PROP_ITER_STATE_INT8S:
        return iter->object.prop_offsets.int8s;
    case NG5_PROP_ITER_STATE_INT16S:
        return iter->object.prop_offsets.int16s;
    case NG5_PROP_ITER_STATE_INT32S:
        return iter->object.prop_offsets.int32s;
    case NG5_PROP_ITER_STATE_INT64S:
        return iter->object.prop_offsets.int64s;
    case NG5_PROP_ITER_STATE_UINT8S:
        return iter->object.prop_offsets.uint8s;
    case NG5_PROP_ITER_STATE_UINT16S:
        return iter->object.prop_offsets.uint16s;
    case NG5_PROP_ITER_STATE_UINT32S:
        return iter->object.prop_offsets.uint32s;
    case NG5_PROP_ITER_STATE_UINT64S:
        return iter->object.prop_offsets.uint64s;
    case NG5_PROP_ITER_STATE_FLOATS:
        return iter->object.prop_offsets.floats;
    case NG5_PROP_ITER_STATE_STRINGS:
        return iter->object.prop_offsets.strings;
    case NG5_PROP_ITER_STATE_OBJECTS:
        return iter->object.prop_offsets.objects;
    case NG5_PROP_ITER_STATE_NULL_ARRAYS:
        return iter->object.prop_offsets.null_arrays;
    case NG5_PROP_ITER_STATE_BOOL_ARRAYS:
        return iter->object.prop_offsets.bool_arrays;
    case NG5_PROP_ITER_STATE_INT8_ARRAYS:
        return iter->object.prop_offsets.int8_arrays;
    case NG5_PROP_ITER_STATE_INT16_ARRAYS:
        return iter->object.prop_offsets.int16_arrays;
    case NG5_PROP_ITER_STATE_INT32_ARRAYS:
        return iter->object.prop_offsets.int32_arrays;
    case NG5_PROP_ITER_STATE_INT64_ARRAYS:
        return iter->object.prop_offsets.int64_arrays;
    case NG5_PROP_ITER_STATE_UINT8_ARRAYS:
        return iter->object.prop_offsets.uint8_arrays;
    case NG5_PROP_ITER_STATE_UINT16_ARRAYS:
        return iter->object.prop_offsets.uint16_arrays;
    case NG5_PROP_ITER_STATE_UINT32_ARRAYS:
        return iter->object.prop_offsets.uint32_arrays;
    case NG5_PROP_ITER_STATE_UINT64_ARRAYS:
        return iter->object.prop_offsets.uint64_arrays;
    case NG5_PROP_ITER_STATE_FLOAT_ARRAYS:
        return iter->object.prop_offsets.float_arrays;
    case NG5_PROP_ITER_STATE_STRING_ARRAYS:
        return iter->object.prop_offsets.string_arrays;
    case NG5_PROP_ITER_STATE_OBJECT_ARRAYS:
        return iter->object.prop_offsets.object_arrays;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR)
    }
}

static bool
prop_iter_read_colum_entry(carbon_archive_collection_iter_state_t *state, memfile_t *memfile)
{
    assert(state->current_column_group.current_column.current_entry.idx <
        state->current_column_group.current_column.num_elem);

    u32     current_idx = state->current_column_group.current_column.current_entry.idx;
    offset_t entry_off   = state->current_column_group.current_column.elem_offsets[current_idx];
    carbon_memfile_seek(memfile, entry_off);

    state->current_column_group.current_column.current_entry.array_length =
                        *NG5_MEMFILE_READ_TYPE(memfile, u32);
    state->current_column_group.current_column.current_entry.array_base =
                        NG5_MEMFILE_PEEK(memfile, void);

    return (++state->current_column_group.current_column.current_entry.idx) <
        state->current_column_group.current_column.num_elem;
}

static bool
prop_iter_read_column(carbon_archive_collection_iter_state_t *state, memfile_t *memfile)
{
    assert(state->current_column_group.current_column.idx <
           state->current_column_group.num_columns);

    u32     current_idx = state->current_column_group.current_column.idx;
    offset_t column_off  = state->current_column_group.column_offs[current_idx];
    carbon_memfile_seek(memfile, column_off);
    const carbon_column_header_t *header = NG5_MEMFILE_READ_TYPE(memfile,
                                                                    carbon_column_header_t);

    assert(header->marker == MARKER_SYMBOL_COLUMN);
    state->current_column_group.current_column.name = header->column_name;
    state->current_column_group.current_column.type =
               carbon_int_field_type_to_basic_type(carbon_int_marker_to_field_type(header->value_type));

    state->current_column_group.current_column.num_elem = header->num_entries;
    state->current_column_group.current_column.elem_offsets = NG5_MEMFILE_READ_TYPE_LIST(
                                           memfile, offset_t, header->num_entries);
    state->current_column_group.current_column.elem_positions = NG5_MEMFILE_READ_TYPE_LIST(
                                           memfile, u32, header->num_entries);
    state->current_column_group.current_column.current_entry.idx = 0;

    return (++state->current_column_group.current_column.idx) <
        state->current_column_group.num_columns;
}

static bool
collection_iter_read_next_column_group(carbon_archive_collection_iter_state_t *state, memfile_t *memfile)
{
    assert(state->current_column_group_idx < state->num_column_groups);
    carbon_memfile_seek(memfile,
                        state->column_group_offsets[state->current_column_group_idx]);
    const carbon_column_group_header_t *header = NG5_MEMFILE_READ_TYPE(memfile,
                                                                          carbon_column_group_header_t);
    assert(header->marker == MARKER_SYMBOL_COLUMN_GROUP);
    state->current_column_group.num_columns = header->num_columns;
    state->current_column_group.num_objects = header->num_objects;
    state->current_column_group.object_ids  = NG5_MEMFILE_READ_TYPE_LIST(memfile,
                                                                carbon_object_id_t, header->num_objects);
    state->current_column_group.column_offs = NG5_MEMFILE_READ_TYPE_LIST(memfile,
                                                                offset_t, header->num_columns);
    state->current_column_group.current_column.idx = 0;

    return (++state->current_column_group_idx) < state->num_column_groups;
}

static void
prop_iter_cursor_init(carbon_archive_prop_iter_t *iter)
{
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_NULLS, prop_offsets.nulls));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_BOOLS, prop_offsets.bools));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT8S, prop_offsets.int8s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT16S, prop_offsets.int16s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT32S, prop_offsets.int32s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT64S, prop_offsets.int64s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT8S, prop_offsets.uint8s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT16S, prop_offsets.uint16s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT32S, prop_offsets.uint32s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT64S, prop_offsets.uint64s));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_FLOATS, prop_offsets.floats));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_STRINGS, prop_offsets.strings));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_OBJECTS, prop_offsets.objects));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_NULL_ARRAYS, prop_offsets.null_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_BOOL_ARRAYS, prop_offsets.bool_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT8_ARRAYS, prop_offsets.int8_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT16_ARRAYS, prop_offsets.int16_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT32_ARRAYS, prop_offsets.int32_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_INT64_ARRAYS, prop_offsets.int64_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT8_ARRAYS, prop_offsets.uint8_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT16_ARRAYS, prop_offsets.uint16_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT32_ARRAYS, prop_offsets.uint32_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_UINT64_ARRAYS, prop_offsets.uint64_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_FLOAT_ARRAYS, prop_offsets.float_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_STRING_ARRAYS, prop_offsets.string_arrays));
    assert(STATE_AND_PROPERTY_EXISTS(NG5_PROP_ITER_STATE_OBJECT_ARRAYS, prop_offsets.object_arrays));



    if (iter->mode == NG5_ARCHIVE_PROP_ITER_MODE_COLLECTION)
    {
        iter->mode_collection.collection_start_off = offset_by_state(iter);
        carbon_memfile_seek(&iter->record_table_memfile, iter->mode_collection.collection_start_off);
        const carbon_object_array_header_t *header = NG5_MEMFILE_READ_TYPE(&iter->record_table_memfile, carbon_object_array_header_t);
        iter->mode_collection.num_column_groups = header->num_entries;
        iter->mode_collection.current_column_group_idx = 0;
        iter->mode_collection.column_group_keys = NG5_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                carbon_string_id_t,
                                                                                iter->mode_collection.num_column_groups);
        iter->mode_collection.column_group_offsets = NG5_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                offset_t,
                                                                                iter->mode_collection.num_column_groups);

    } else
    {
        iter->mode_object.current_prop_group_off = offset_by_state(iter);
        carbon_memfile_seek(&iter->record_table_memfile, iter->mode_object.current_prop_group_off);
        carbon_int_embedded_fixed_props_read(&iter->mode_object.prop_group_header, &iter->record_table_memfile);
        iter->mode_object.prop_data_off = memfile_tell(&iter->record_table_memfile);
    }

}

#define SET_STATE_FOR_FALL_THROUGH(iter, prop_offset_type, mask_group, mask_type, next_state)                          \
{                                                                                                                      \
    if ((iter->object.prop_offsets.prop_offset_type != 0) &&                                                           \
        (NG5_MASK_IS_BIT_SET(iter->mask, mask_group | mask_type)))                                                  \
    {                                                                                                                  \
        iter->prop_cursor = next_state;                                                                                \
        break;                                                                                                         \
    }                                                                                                                  \
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

static carbon_prop_iter_state_e
prop_iter_state_next(carbon_archive_prop_iter_t *iter)
{
    switch (iter->prop_cursor) {
    case NG5_PROP_ITER_STATE_INIT:
        SET_STATE_FOR_FALL_THROUGH(iter, nulls, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   NG5_ARCHIVE_ITER_MASK_NULL, NG5_PROP_ITER_STATE_NULLS)
    case NG5_PROP_ITER_STATE_NULLS:
        SET_STATE_FOR_FALL_THROUGH(iter, bools, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   NG5_ARCHIVE_ITER_MASK_BOOLEAN, NG5_PROP_ITER_STATE_BOOLS)
    case NG5_PROP_ITER_STATE_BOOLS:
        SET_STATE_FOR_FALL_THROUGH(iter, int8s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                                   NG5_ARCHIVE_ITER_MASK_INT8, NG5_PROP_ITER_STATE_INT8S)
    case NG5_PROP_ITER_STATE_INT8S:
        SET_STATE_FOR_FALL_THROUGH(iter, int16s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_INT16, NG5_PROP_ITER_STATE_INT16S)
    case NG5_PROP_ITER_STATE_INT16S:
        SET_STATE_FOR_FALL_THROUGH(iter, int32s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_INT32, NG5_PROP_ITER_STATE_INT32S)
    case NG5_PROP_ITER_STATE_INT32S:
        SET_STATE_FOR_FALL_THROUGH(iter, int64s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_INT64, NG5_PROP_ITER_STATE_INT64S)
    case NG5_PROP_ITER_STATE_INT64S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint8s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_UINT8, NG5_PROP_ITER_STATE_UINT8S)
    case NG5_PROP_ITER_STATE_UINT8S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint16s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_UINT16, NG5_PROP_ITER_STATE_UINT16S)
    case NG5_PROP_ITER_STATE_UINT16S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint32s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_UINT32, NG5_PROP_ITER_STATE_UINT32S)
    case NG5_PROP_ITER_STATE_UINT32S:
        SET_STATE_FOR_FALL_THROUGH(iter, uint64s, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_UINT64, NG5_PROP_ITER_STATE_UINT64S)
    case NG5_PROP_ITER_STATE_UINT64S:
        SET_STATE_FOR_FALL_THROUGH(iter, floats, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_NUMBER, NG5_PROP_ITER_STATE_FLOATS)
    case NG5_PROP_ITER_STATE_FLOATS:
        SET_STATE_FOR_FALL_THROUGH(iter, strings, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_STRING, NG5_PROP_ITER_STATE_STRINGS)
    case NG5_PROP_ITER_STATE_STRINGS:
        SET_STATE_FOR_FALL_THROUGH(iter, objects, NG5_ARCHIVE_ITER_MASK_PRIMITIVES,
                               NG5_ARCHIVE_ITER_MASK_OBJECT, NG5_PROP_ITER_STATE_OBJECTS)
    case NG5_PROP_ITER_STATE_OBJECTS:
        SET_STATE_FOR_FALL_THROUGH(iter, null_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_NULL, NG5_PROP_ITER_STATE_NULL_ARRAYS)
    case NG5_PROP_ITER_STATE_NULL_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, bool_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_BOOLEAN, NG5_PROP_ITER_STATE_BOOL_ARRAYS)
    case NG5_PROP_ITER_STATE_BOOL_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int8_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_INT8, NG5_PROP_ITER_STATE_INT8_ARRAYS)
    case NG5_PROP_ITER_STATE_INT8_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int16_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_INT16, NG5_PROP_ITER_STATE_INT16_ARRAYS)
    case NG5_PROP_ITER_STATE_INT16_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int32_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_INT32, NG5_PROP_ITER_STATE_INT32_ARRAYS)
    case NG5_PROP_ITER_STATE_INT32_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, int64_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_INT64, NG5_PROP_ITER_STATE_INT64_ARRAYS)
    case NG5_PROP_ITER_STATE_INT64_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint8_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_UINT8, NG5_PROP_ITER_STATE_UINT8_ARRAYS)
    case NG5_PROP_ITER_STATE_UINT8_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint16_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_UINT16, NG5_PROP_ITER_STATE_UINT16_ARRAYS)
    case NG5_PROP_ITER_STATE_UINT16_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint32_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_UINT32, NG5_PROP_ITER_STATE_UINT32_ARRAYS)
    case NG5_PROP_ITER_STATE_UINT32_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, uint64_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_UINT64, NG5_PROP_ITER_STATE_UINT64_ARRAYS)
    case NG5_PROP_ITER_STATE_UINT64_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, float_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_NUMBER, NG5_PROP_ITER_STATE_FLOAT_ARRAYS)
    case NG5_PROP_ITER_STATE_FLOAT_ARRAYS:
        SET_STATE_FOR_FALL_THROUGH(iter, string_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_STRING, NG5_PROP_ITER_STATE_STRING_ARRAYS)
    case NG5_PROP_ITER_STATE_STRING_ARRAYS:
    SET_STATE_FOR_FALL_THROUGH(iter, object_arrays, NG5_ARCHIVE_ITER_MASK_ARRAYS,
                               NG5_ARCHIVE_ITER_MASK_OBJECT, NG5_PROP_ITER_STATE_OBJECT_ARRAYS)
    case NG5_PROP_ITER_STATE_OBJECT_ARRAYS:
        iter->prop_cursor = NG5_PROP_ITER_STATE_DONE;
        break;

    case NG5_PROP_ITER_STATE_DONE:
        break;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }

    iter->mode = iter->prop_cursor == NG5_PROP_ITER_STATE_OBJECT_ARRAYS ?
                 NG5_ARCHIVE_PROP_ITER_MODE_COLLECTION : NG5_ARCHIVE_PROP_ITER_MODE_OBJECT;

    if (iter->prop_cursor != NG5_PROP_ITER_STATE_DONE) {
        prop_iter_cursor_init(iter);
    }
    return iter->prop_cursor;
}

#pragma GCC diagnostic pop

static void
prop_iter_state_init(carbon_archive_prop_iter_t *iter)
{
    iter->prop_cursor = NG5_PROP_ITER_STATE_INIT;
    iter->mode = NG5_ARCHIVE_PROP_ITER_MODE_OBJECT;
}


static bool
carbon_archive_prop_iter_from_memblock(carbon_archive_prop_iter_t *iter,
                                       struct err *err,
                                       u16 mask,
                                       carbon_memblock_t *memblock,
                                       offset_t object_offset)
{
    NG5_NON_NULL_OR_ERROR(iter)
    NG5_NON_NULL_OR_ERROR(err)
    NG5_NON_NULL_OR_ERROR(memblock)

    iter->mask = mask;
    if (!carbon_memfile_open(&iter->record_table_memfile, memblock,
                             NG5_MEMFILE_MODE_READONLY)) {
        error(err, NG5_ERR_MEMFILEOPEN_FAILED)
        return false;
    }
    if (!carbon_memfile_seek(&iter->record_table_memfile, object_offset))
    {
        error(err, NG5_ERR_MEMFILESEEK_FAILED)
        return false;
    }
    if (!init_object_from_memfile(&iter->object, &iter->record_table_memfile)) {
        error(err, NG5_ERR_INTERNALERR);
        return false;
    }

    carbon_error_init(&iter->err);
    prop_iter_state_init(iter);
    prop_iter_state_next(iter);

    return true;
}

NG5_EXPORT(bool)
carbon_archive_prop_iter_from_archive(carbon_archive_prop_iter_t *iter,
                                      struct err *err,
                                      u16 mask,
                                      carbon_archive_t *archive)
{
    return carbon_archive_prop_iter_from_memblock(iter, err, mask, archive->record_table.recordDataBase, 0);
}

NG5_EXPORT(bool)
carbon_archive_prop_iter_from_object(carbon_archive_prop_iter_t *iter,
                                     u16 mask,
                                     struct err *err,
                                     const carbon_archive_object_t *obj)
{
    return carbon_archive_prop_iter_from_memblock(iter, err, mask,
                                                  obj->memfile.memblock, obj->offset);
}

static carbon_basic_type_e
get_basic_type(carbon_prop_iter_state_e state)
{
    switch (state) {
    case NG5_PROP_ITER_STATE_NULLS:
    case NG5_PROP_ITER_STATE_NULL_ARRAYS:
        return NG5_BASIC_TYPE_NULL;
    case NG5_PROP_ITER_STATE_BOOLS:
    case NG5_PROP_ITER_STATE_BOOL_ARRAYS:
        return NG5_BASIC_TYPE_BOOLEAN;
    case NG5_PROP_ITER_STATE_INT8S:
    case NG5_PROP_ITER_STATE_INT8_ARRAYS:
        return NG5_BASIC_TYPE_INT8;
    case NG5_PROP_ITER_STATE_INT16S:
    case NG5_PROP_ITER_STATE_INT16_ARRAYS:
        return NG5_BASIC_TYPE_INT16;
    case NG5_PROP_ITER_STATE_INT32S:
    case NG5_PROP_ITER_STATE_INT32_ARRAYS:
        return NG5_BASIC_TYPE_INT32;
    case NG5_PROP_ITER_STATE_INT64S:
    case NG5_PROP_ITER_STATE_INT64_ARRAYS:
        return NG5_BASIC_TYPE_INT64;
    case NG5_PROP_ITER_STATE_UINT8S:
    case NG5_PROP_ITER_STATE_UINT8_ARRAYS:
        return NG5_BASIC_TYPE_UINT8;
    case NG5_PROP_ITER_STATE_UINT16S:
    case NG5_PROP_ITER_STATE_UINT16_ARRAYS:
        return NG5_BASIC_TYPE_UINT16;
    case NG5_PROP_ITER_STATE_UINT32S:
    case NG5_PROP_ITER_STATE_UINT32_ARRAYS:
        return NG5_BASIC_TYPE_UINT32;
    case NG5_PROP_ITER_STATE_UINT64S:
    case NG5_PROP_ITER_STATE_UINT64_ARRAYS:
        return NG5_BASIC_TYPE_UINT64;
    case NG5_PROP_ITER_STATE_FLOATS:
    case NG5_PROP_ITER_STATE_FLOAT_ARRAYS:
        return NG5_BASIC_TYPE_NUMBER;
    case NG5_PROP_ITER_STATE_STRINGS:
    case NG5_PROP_ITER_STATE_STRING_ARRAYS:
        return NG5_BASIC_TYPE_STRING;
    case NG5_PROP_ITER_STATE_OBJECTS:
    case NG5_PROP_ITER_STATE_OBJECT_ARRAYS:
        return NG5_BASIC_TYPE_OBJECT;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }
}

static bool
is_array_type(carbon_prop_iter_state_e state)
{
    switch (state) {
    case NG5_PROP_ITER_STATE_NULLS:
    case NG5_PROP_ITER_STATE_BOOLS:
    case NG5_PROP_ITER_STATE_INT8S:
    case NG5_PROP_ITER_STATE_INT16S:
    case NG5_PROP_ITER_STATE_INT32S:
    case NG5_PROP_ITER_STATE_INT64S:
    case NG5_PROP_ITER_STATE_UINT8S:
    case NG5_PROP_ITER_STATE_UINT16S:
    case NG5_PROP_ITER_STATE_UINT32S:
    case NG5_PROP_ITER_STATE_UINT64S:
    case NG5_PROP_ITER_STATE_FLOATS:
    case NG5_PROP_ITER_STATE_STRINGS:
    case NG5_PROP_ITER_STATE_OBJECTS:
        return false;
    case NG5_PROP_ITER_STATE_NULL_ARRAYS:
    case NG5_PROP_ITER_STATE_BOOL_ARRAYS:
    case NG5_PROP_ITER_STATE_INT8_ARRAYS:
    case NG5_PROP_ITER_STATE_INT16_ARRAYS:
    case NG5_PROP_ITER_STATE_INT32_ARRAYS:
    case NG5_PROP_ITER_STATE_INT64_ARRAYS:
    case NG5_PROP_ITER_STATE_UINT8_ARRAYS:
    case NG5_PROP_ITER_STATE_UINT16_ARRAYS:
    case NG5_PROP_ITER_STATE_UINT32_ARRAYS:
    case NG5_PROP_ITER_STATE_UINT64_ARRAYS:
    case NG5_PROP_ITER_STATE_FLOAT_ARRAYS:
    case NG5_PROP_ITER_STATE_STRING_ARRAYS:
    case NG5_PROP_ITER_STATE_OBJECT_ARRAYS:
        return true;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }
}


NG5_EXPORT(bool)
carbon_archive_prop_iter_next(carbon_archive_prop_iter_mode_e *type,
                              carbon_archive_value_vector_t *value_vector,
                              carbon_archive_collection_iter_t *collection_iter,
                              carbon_archive_prop_iter_t *prop_iter)
{
    NG5_NON_NULL_OR_ERROR(type);
    NG5_NON_NULL_OR_ERROR(prop_iter);

    if (prop_iter->prop_cursor != NG5_PROP_ITER_STATE_DONE)
    {
        switch (prop_iter->mode) {
        case NG5_ARCHIVE_PROP_ITER_MODE_OBJECT: {
                value_vector->keys = prop_iter->mode_object.prop_group_header.keys;

                prop_iter->mode_object.type = get_basic_type(prop_iter->prop_cursor);
                prop_iter->mode_object.is_array = is_array_type(prop_iter->prop_cursor);

                value_vector->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;
                value_vector->prop_type = prop_iter->mode_object.type;
                value_vector->is_array = prop_iter->mode_object.is_array;

                if (value_vector && !carbon_archive_value_vector_from_prop_iter(value_vector, &prop_iter->err, prop_iter))
                {
                    error(&prop_iter->err, NG5_ERR_VITEROPEN_FAILED);
                    return false;
                }
        } break;
        case NG5_ARCHIVE_PROP_ITER_MODE_COLLECTION: {
            collection_iter->state = prop_iter->mode_collection;
            carbon_memfile_open(&collection_iter->record_table_memfile, prop_iter->record_table_memfile.memblock,
                                NG5_MEMFILE_MODE_READONLY);
            carbon_error_init(&collection_iter->err);
        } break;
        default:
            error(&prop_iter->err, NG5_ERR_INTERNALERR);
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

NG5_EXPORT(const carbon_string_id_t *)
carbon_archive_collection_iter_get_keys(u32 *num_keys, carbon_archive_collection_iter_t *iter)
{
    if (num_keys && iter) {
        *num_keys = iter->state.num_column_groups;
        return iter->state.column_group_keys;
    } else {
        error(&iter->err, NG5_ERR_NULLPTR);
        return NULL;
    }
}

NG5_EXPORT(bool)
carbon_archive_collection_next_column_group(carbon_archive_column_group_iter_t *group_iter,
                                            carbon_archive_collection_iter_t *iter)
{
    NG5_NON_NULL_OR_ERROR(group_iter)
    NG5_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group_idx < iter->state.num_column_groups) {
        collection_iter_read_next_column_group(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&group_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            NG5_MEMFILE_MODE_READONLY);
        group_iter->state = iter->state;
        carbon_error_init(&group_iter->err);
        return true;
    } else {
        return false;
    }
}

NG5_EXPORT(const carbon_object_id_t *)
carbon_archive_column_group_get_object_ids(u32 *num_objects, carbon_archive_column_group_iter_t *iter)
{
    if (num_objects && iter) {
        *num_objects = iter->state.current_column_group.num_objects;
        return iter->state.current_column_group.object_ids;
    } else {
        error(&iter->err, NG5_ERR_NULLPTR);
        return NULL;
    }
}


NG5_EXPORT(bool)
carbon_archive_column_group_next_column(carbon_archive_column_iter_t *column_iter,
                                        carbon_archive_column_group_iter_t *iter)
{
    NG5_NON_NULL_OR_ERROR(column_iter)
    NG5_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group.current_column.idx < iter->state.current_column_group.num_columns) {
        prop_iter_read_column(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&column_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            NG5_MEMFILE_MODE_READONLY);
        column_iter->state = iter->state;
        carbon_error_init(&column_iter->err);
        return true;
    } else {
        return false;
    }
}

NG5_EXPORT(bool)
carbon_archive_column_get_name(carbon_string_id_t *name,
                               carbon_basic_type_e *type,
                               carbon_archive_column_iter_t *column_iter)
{
    NG5_NON_NULL_OR_ERROR(column_iter)
    NG5_OPTIONAL_SET(name, column_iter->state.current_column_group.current_column.name)
    NG5_OPTIONAL_SET(type, column_iter->state.current_column_group.current_column.type)
    return true;
}

NG5_EXPORT(const u32 *)
carbon_archive_column_get_entry_positions(u32 *num_entry, carbon_archive_column_iter_t *column_iter)
{
    if (num_entry && column_iter) {
        *num_entry = column_iter->state.current_column_group.current_column.num_elem;
        return column_iter->state.current_column_group.current_column.elem_positions;
    } else {
        error(&column_iter->err, NG5_ERR_NULLPTR);
        return NULL;
    }
}

NG5_EXPORT(bool)
carbon_archive_column_next_entry(carbon_archive_column_entry_iter_t *entry_iter, carbon_archive_column_iter_t *iter)
{
    NG5_NON_NULL_OR_ERROR(entry_iter)
    NG5_NON_NULL_OR_ERROR(iter)

    if (iter->state.current_column_group.current_column.current_entry.idx <
        iter->state.current_column_group.current_column.num_elem)
    {
        prop_iter_read_colum_entry(&iter->state, &iter->record_table_memfile);
        carbon_memfile_open(&entry_iter->record_table_memfile, iter->record_table_memfile.memblock,
                            NG5_MEMFILE_MODE_READONLY);
        entry_iter->state = iter->state;
        carbon_error_init(&entry_iter->err);
        return true;
    } else {
        return false;
    }
}

NG5_EXPORT(bool)
carbon_archive_column_entry_get_type(carbon_basic_type_e *type, carbon_archive_column_entry_iter_t *entry)
{
    NG5_NON_NULL_OR_ERROR(type)
    NG5_NON_NULL_OR_ERROR(entry)
    *type = entry->state.current_column_group.current_column.type;
    return true;
}

#define DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name, basic_type)                            \
NG5_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_column_entry_get_##name(u32 *array_length, carbon_archive_column_entry_iter_t *entry)              \
{                                                                                                                      \
    if (array_length && entry) {                                                                                       \
        if (entry->state.current_column_group.current_column.type == basic_type)                                       \
        {                                                                                                              \
            *array_length =  entry->state.current_column_group.current_column.current_entry.array_length;              \
            return (const built_in_type *) entry->state.current_column_group.current_column.current_entry.array_base;  \
        } else {                                                                                                       \
            error(&entry->err, NG5_ERR_TYPEMISMATCH);                                                        \
            return NULL;                                                                                               \
        }                                                                                                              \
    } else {                                                                                                           \
        error(&entry->err, NG5_ERR_NULLPTR);                                                                 \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_i8, int8s, NG5_BASIC_TYPE_INT8);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_i16, int16s, NG5_BASIC_TYPE_INT16);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_i32, int32s, NG5_BASIC_TYPE_INT32);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_i64, int64s, NG5_BASIC_TYPE_INT64);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_u8, uint8s, NG5_BASIC_TYPE_UINT8);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_u16, uint16s, NG5_BASIC_TYPE_UINT16);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_u32, uint32s, NG5_BASIC_TYPE_UINT32);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_u64, uint64s, NG5_BASIC_TYPE_UINT64);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_string_id_t, strings, NG5_BASIC_TYPE_STRING);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_number_t, numbers, NG5_BASIC_TYPE_NUMBER);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_boolean_t, booleans, NG5_BASIC_TYPE_BOOLEAN);
DECLARE_NG5_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(carbon_u32, nulls, NG5_BASIC_TYPE_NULL);

NG5_EXPORT(bool)
carbon_archive_column_entry_get_objects(carbon_archive_column_entry_object_iter_t *iter,
                                        carbon_archive_column_entry_iter_t *entry)
{
    NG5_NON_NULL_OR_ERROR(iter)
    NG5_NON_NULL_OR_ERROR(entry)

    iter->entry_state = entry->state;
    carbon_memfile_open(&iter->memfile, entry->record_table_memfile.memblock, NG5_MEMFILE_MODE_READONLY);
    carbon_memfile_seek(&iter->memfile, entry->state.current_column_group.current_column.elem_offsets[entry->state.current_column_group.current_column.current_entry.idx - 1] + sizeof(u32));
    iter->next_obj_off = memfile_tell(&iter->memfile);
    carbon_error_init(&iter->err);

    return true;
}

NG5_EXPORT(const carbon_archive_object_t *)
carbon_archive_column_entry_object_iter_next_object(carbon_archive_column_entry_object_iter_t *iter)
{
    if (iter) {
        if (iter->next_obj_off != 0) {
            carbon_memfile_seek(&iter->memfile, iter->next_obj_off);
            if (init_object_from_memfile(&iter->obj, &iter->memfile)) {
                iter->next_obj_off = iter->obj.next_obj_off;
                return &iter->obj;
            } else {
                error(&iter->err, NG5_ERR_INTERNALERR);
                return NULL;
            }
        } else {
            return NULL;
        }
    } else {
        error(&iter->err, NG5_ERR_NULLPTR);
        return NULL;
    }
}

NG5_EXPORT(bool)
carbon_archive_object_get_object_id(carbon_object_id_t *id, const carbon_archive_object_t *object)
{
    NG5_NON_NULL_OR_ERROR(id)
    NG5_NON_NULL_OR_ERROR(object)
    *id = object->object_id;
    return true;
}

NG5_EXPORT(bool)
carbon_archive_object_get_prop_iter(carbon_archive_prop_iter_t *iter, const carbon_archive_object_t *object)
{
    // XXXX carbon_archive_prop_iter_from_object()
    NG5_UNUSED(iter);
    NG5_UNUSED(object);
    return false;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_get_object_id(carbon_object_id_t *id, const carbon_archive_value_vector_t *iter)
{
    NG5_NON_NULL_OR_ERROR(id)
    NG5_NON_NULL_OR_ERROR(iter)
    *id = iter->object_id;
    return true;
}

NG5_EXPORT(const carbon_string_id_t *)
carbon_archive_value_vector_get_keys(u32 *num_keys, carbon_archive_value_vector_t *iter)
{
    if (num_keys && iter) {
        *num_keys = iter->value_max_idx;
        return iter->keys;
    } else {
        error(&iter->err, NG5_ERR_NULLPTR)
        return NULL;
    }
}

static void
value_vector_init_object_basic(carbon_archive_value_vector_t *value)
{
    value->data.object.offsets = NG5_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, offset_t,
                                                               value->value_max_idx);
}

static void
value_vector_init_object_array(carbon_archive_value_vector_t *value)
{
    NG5_UNUSED(value);
    abort(); // TODO: Implement XXX
}

static void
value_vector_init_fixed_length_types_basic(carbon_archive_value_vector_t *value)
{
    assert(!value->is_array);

    switch (value->prop_type) {
    case NG5_BASIC_TYPE_INT8:
        value->data.basic.values.int8s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i8);
        break;
    case NG5_BASIC_TYPE_INT16:
        value->data.basic.values.int16s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i16);
        break;
    case NG5_BASIC_TYPE_INT32:
        value->data.basic.values.int32s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i32);
        break;
    case NG5_BASIC_TYPE_INT64:
        value->data.basic.values.int64s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i64);
        break;
    case NG5_BASIC_TYPE_UINT8:
        value->data.basic.values.uint8s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u8);
        break;
    case NG5_BASIC_TYPE_UINT16:
        value->data.basic.values.uint16s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u16);
        break;
    case NG5_BASIC_TYPE_UINT32:
        value->data.basic.values.uint32s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u32);
        break;
    case NG5_BASIC_TYPE_UINT64:
        value->data.basic.values.uint64s = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u64);
        break;
    case NG5_BASIC_TYPE_NUMBER:
        value->data.basic.values.numbers = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_number_t);
        break;
    case NG5_BASIC_TYPE_STRING:
        value->data.basic.values.strings = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_string_id_t);
        break;
    case NG5_BASIC_TYPE_BOOLEAN:
        value->data.basic.values.booleans = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_boolean_t);
        break;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }
}

static void
value_vector_init_fixed_length_types_null_arrays(carbon_archive_value_vector_t *value)
{
    assert(value->is_array);
    assert(value->prop_type == NG5_BASIC_TYPE_NULL);
    value->data.arrays.meta.num_nulls_contained = NG5_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, u32,
                                                                                value->value_max_idx);
}

static void
value_vector_init_fixed_length_types_non_null_arrays(carbon_archive_value_vector_t *value)
{
    assert (value->is_array);

    value->data.arrays.meta.array_lengths = NG5_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, u32,
                                                                     value->value_max_idx);

    switch (value->prop_type) {
    case NG5_BASIC_TYPE_INT8:
        value->data.arrays.values.int8s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i8);
        break;
    case NG5_BASIC_TYPE_INT16:
        value->data.arrays.values.int16s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i16);
        break;
    case NG5_BASIC_TYPE_INT32:
        value->data.arrays.values.int32s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i32);
        break;
    case NG5_BASIC_TYPE_INT64:
        value->data.arrays.values.int64s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_i64);
        break;
    case NG5_BASIC_TYPE_UINT8:
        value->data.arrays.values.uint8s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u8);
        break;
    case NG5_BASIC_TYPE_UINT16:
        value->data.arrays.values.uint16s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u16);
        break;
    case NG5_BASIC_TYPE_UINT32:
        value->data.arrays.values.uint32s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u32);
        break;
    case NG5_BASIC_TYPE_UINT64:
        value->data.arrays.values.uint64s_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_u64);
        break;
    case NG5_BASIC_TYPE_NUMBER:
        value->data.arrays.values.numbers_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_number_t);
        break;
    case NG5_BASIC_TYPE_STRING:
        value->data.arrays.values.strings_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_string_id_t);
        break;
    case NG5_BASIC_TYPE_BOOLEAN:
        value->data.arrays.values.booleans_base = NG5_MEMFILE_PEEK(&value->record_table_memfile, carbon_boolean_t);
        break;
    default:
    carbon_print_error_and_die(NG5_ERR_INTERNALERR);
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

NG5_EXPORT(bool)
carbon_archive_value_vector_from_prop_iter(carbon_archive_value_vector_t *value,
                                           struct err *err,
                                           carbon_archive_prop_iter_t *prop_iter)
{
    NG5_NON_NULL_OR_ERROR(value);
    NG5_NON_NULL_OR_ERROR(prop_iter);

    error_IF_AND_RETURN (prop_iter->mode != NG5_ARCHIVE_PROP_ITER_MODE_OBJECT, &prop_iter->err,
                                NG5_ERR_ITER_OBJECT_NEEDED, false)

    carbon_error_init(&value->err);

    value->prop_iter = prop_iter;
    value->data_off = prop_iter->mode_object.prop_data_off;
    value->object_id = prop_iter->object.object_id;

    if (!carbon_memfile_open(&value->record_table_memfile, prop_iter->record_table_memfile.memblock,
                        NG5_MEMFILE_MODE_READONLY)) {
        error(err, NG5_ERR_MEMFILEOPEN_FAILED);
        return false;
    }
    if (!carbon_memfile_skip(&value->record_table_memfile, value->data_off)) {
        error(err, NG5_ERR_MEMFILESKIP_FAILED);
        return false;
    }

    value->prop_type = prop_iter->mode_object.type;
    value->is_array = prop_iter->mode_object.is_array;
    value->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;

    switch (value->prop_type) {
    case NG5_BASIC_TYPE_OBJECT:
        value_vector_init_object(value);
        break;
    case NG5_BASIC_TYPE_NULL:
        if (value->is_array) {
            value_vector_init_fixed_length_types_null_arrays(value);
        }
        break;
    case NG5_BASIC_TYPE_INT8:
    case NG5_BASIC_TYPE_INT16:
    case NG5_BASIC_TYPE_INT32:
    case NG5_BASIC_TYPE_INT64:
    case NG5_BASIC_TYPE_UINT8:
    case NG5_BASIC_TYPE_UINT16:
    case NG5_BASIC_TYPE_UINT32:
    case NG5_BASIC_TYPE_UINT64:
    case NG5_BASIC_TYPE_NUMBER:
    case NG5_BASIC_TYPE_STRING:
    case NG5_BASIC_TYPE_BOOLEAN:
        value_vector_init_fixed_length_types(value);
        break;
    default:
        carbon_print_error_and_die(NG5_ERR_INTERNALERR);
    }

    return true;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_get_basic_type(carbon_basic_type_e *type, const carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(type)
    NG5_NON_NULL_OR_ERROR(value)
    *type = value->prop_type;
    return true;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_is_array_type(bool *is_array, const carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(is_array)
    NG5_NON_NULL_OR_ERROR(value)
    *is_array = value->is_array;
    return true;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_get_length(u32 *length, const carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(length)
    NG5_NON_NULL_OR_ERROR(value)
    *length = value->value_max_idx;
    return true;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_is_of_objects(bool *is_object, carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(is_object)
    NG5_NON_NULL_OR_ERROR(value)

    *is_object = value->prop_type == NG5_BASIC_TYPE_OBJECT && !value->is_array;

    return true;
}

NG5_EXPORT(bool)
carbon_archive_value_vector_get_object_at(carbon_archive_object_t *object, u32 idx,
                                          carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(object)
    NG5_NON_NULL_OR_ERROR(value)

    if (idx >= value->value_max_idx) {
        error(&value->err, NG5_ERR_OUTOFBOUNDS);
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
        error(&value->err, NG5_ERR_ITER_NOOBJ);
        return false;
    }
}

#define DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name, basic_type)                                            \
NG5_EXPORT(bool)                                                                                                    \
carbon_archive_value_vector_is_##name(bool *type_match, carbon_archive_value_vector_t *value)                          \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(type_match)                                                                               \
    NG5_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    *type_match = value->prop_type == basic_type;                                                                      \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8, NG5_BASIC_TYPE_INT8)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16, NG5_BASIC_TYPE_INT16)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32, NG5_BASIC_TYPE_INT32)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64, NG5_BASIC_TYPE_INT64)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8, NG5_BASIC_TYPE_UINT8)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16, NG5_BASIC_TYPE_UINT16)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32, NG5_BASIC_TYPE_UINT32)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64, NG5_BASIC_TYPE_UINT64)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string, NG5_BASIC_TYPE_STRING)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number, NG5_BASIC_TYPE_NUMBER)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean, NG5_BASIC_TYPE_BOOLEAN)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null, NG5_BASIC_TYPE_NULL)

#define DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(names, name, built_in_type, err_code)                       \
NG5_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_value_vector_get_##names(u32 *num_values, carbon_archive_value_vector_t *value)                    \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (carbon_archive_value_vector_is_array_type(&is_array, value) &&                                                 \
        carbon_archive_value_vector_is_##name(&type_match, value) && !is_array)                                        \
    {                                                                                                                  \
        NG5_OPTIONAL_SET(num_values, value->value_max_idx)                                                          \
        return value->data.basic.values.names;                                                                         \
    } else                                                                                                             \
    {                                                                                                                  \
        error(&value->err, err_code);                                                                           \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}


DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, int8, carbon_i8, NG5_ERR_ITER_NOINT8)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, int16, carbon_i16, NG5_ERR_ITER_NOINT16)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, int32, carbon_i32, NG5_ERR_ITER_NOINT32)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, int64, carbon_i64, NG5_ERR_ITER_NOINT64)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, uint8, carbon_u8, NG5_ERR_ITER_NOUINT8)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, uint16, carbon_u16, NG5_ERR_ITER_NOUINT16)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, uint32, carbon_u32, NG5_ERR_ITER_NOUINT32)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, uint64, carbon_u64, NG5_ERR_ITER_NOUINT64)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, string, carbon_string_id_t, NG5_ERR_ITER_NOSTRING)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, number, carbon_number_t, NG5_ERR_ITER_NONUMBER)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, boolean, carbon_boolean_t, NG5_ERR_ITER_NOBOOL)

NG5_EXPORT(const carbon_u32 *)
carbon_archive_value_vector_get_null_arrays(u32 *num_values, carbon_archive_value_vector_t *value)
{
    NG5_NON_NULL_OR_ERROR(value)

    bool is_array;
    bool type_match;

    if (carbon_archive_value_vector_is_array_type(&is_array, value) &&
        carbon_archive_value_vector_is_null(&type_match, value) && is_array)
    {
        NG5_OPTIONAL_SET(num_values, value->value_max_idx);
        return value->data.arrays.meta.num_nulls_contained;
    } else
    {
        return NULL;
    }
}

#define DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type, base)                               \
NG5_EXPORT(const built_in_type *)                                                                                   \
carbon_archive_value_vector_get_##name##_arrays_at(u32 *array_length, u32 idx,                               \
                                               carbon_archive_value_vector_t *value)                                   \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (idx < value->value_max_idx && carbon_archive_value_vector_is_array_type(&is_array, value) &&                   \
        carbon_archive_value_vector_is_##name(&type_match, value) && is_array)                                         \
    {                                                                                                                  \
        u32 skip_length = 0;                                                                                      \
        for (u32 i = 0; i < idx; i++) {                                                                           \
            skip_length += value->data.arrays.meta.array_lengths[i];                                                   \
        }                                                                                                              \
        *array_length = value->data.arrays.meta.array_lengths[idx];                                                    \
        return value->data.arrays.values.base + skip_length;                                                           \
    } else                                                                                                             \
    {                                                                                                                  \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, carbon_i8, int8s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, carbon_i16, int16s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, carbon_i32, int32s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, carbon_i64, int64s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, carbon_u8, uint8s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, carbon_u16, uint16s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, carbon_u32, uint32s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, carbon_u64, uint64s_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, carbon_string_id_t, strings_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, carbon_number_t, numbers_base)
DECLARE_NG5_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, carbon_boolean_t, booleans_base)


void
carbon_int_reset_cabin_object_mem_file(carbon_archive_object_t *object)
{
    NG5_UNUSED(object);
  //  carbon_memfile_seek(&object->file, object->self);
    abort();
}
