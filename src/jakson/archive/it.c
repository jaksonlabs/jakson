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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/archive/it.h>
#include <jakson/archive/internal.h>

static bool init_object_from_memfile(archive_object *obj, memfile *memfile)
{
        JAK_ASSERT(obj);
        offset_t object_off;
        object_header *header;
        object_flags_u flags;

        object_off = memfile_tell(memfile);
        header = MEMFILE_READ_TYPE(memfile, object_header);
        if (UNLIKELY(header->marker != MARKER_SYMBOL_OBJECT_BEGIN)) {
                return false;
        }

        flags.value = header->flags;
        int_read_prop_offsets(&obj->prop_offsets, memfile, &flags);

        obj->object_id = header->oid;
        obj->offset = object_off;
        obj->next_obj_off = *MEMFILE_READ_TYPE(memfile, offset_t);
        memfile_open(&obj->memfile, memfile->memblock, READ_ONLY);

        return true;
}

#define STATE_AND_PROPERTY_EXISTS(state, property) \
    (iter->prop_cursor != state || iter->object.property != 0)

inline static offset_t offset_by_state(prop_iter *iter)
{
        switch (iter->prop_cursor) {
                case PROP_ITER_NULLS:
                        return iter->object.prop_offsets.nulls;
                case PROP_ITER_BOOLS:
                        return iter->object.prop_offsets.bools;
                case PROP_ITER_INT8S:
                        return iter->object.prop_offsets.int8s;
                case PROP_ITER_INT16S:
                        return iter->object.prop_offsets.int16s;
                case PROP_ITER_INT32S:
                        return iter->object.prop_offsets.int32s;
                case PROP_ITER_INT64S:
                        return iter->object.prop_offsets.int64s;
                case PROP_ITER_UINT8S:
                        return iter->object.prop_offsets.uint8s;
                case PROP_ITER_UINT16S:
                        return iter->object.prop_offsets.uint16s;
                case PROP_ITER_UINT32S:
                        return iter->object.prop_offsets.uint32s;
                case PROP_ITER_UINT64S:
                        return iter->object.prop_offsets.uint64s;
                case PROP_ITER_FLOATS:
                        return iter->object.prop_offsets.floats;
                case PROP_ITER_STRINGS:
                        return iter->object.prop_offsets.strings;
                case PROP_ITER_OBJECTS:
                        return iter->object.prop_offsets.objects;
                case PROP_ITER_NULL_ARRAYS:
                        return iter->object.prop_offsets.null_arrays;
                case PROP_ITER_BOOL_ARRAYS:
                        return iter->object.prop_offsets.bool_arrays;
                case PROP_ITER_INT8_ARRAYS:
                        return iter->object.prop_offsets.int8_arrays;
                case PROP_ITER_INT16_ARRAYS:
                        return iter->object.prop_offsets.int16_arrays;
                case PROP_ITER_INT32_ARRAYS:
                        return iter->object.prop_offsets.int32_arrays;
                case PROP_ITER_INT64_ARRAYS:
                        return iter->object.prop_offsets.int64_arrays;
                case PROP_ITER_UINT8_ARRAYS:
                        return iter->object.prop_offsets.uint8_arrays;
                case PROP_ITER_UINT16_ARRAYS:
                        return iter->object.prop_offsets.uint16_arrays;
                case PROP_ITER_UINT32_ARRAYS:
                        return iter->object.prop_offsets.uint32_arrays;
                case PROP_ITER_UINT64_ARRAYS:
                        return iter->object.prop_offsets.uint64_arrays;
                case PROP_ITER_FLOAT_ARRAYS:
                        return iter->object.prop_offsets.float_arrays;
                case PROP_ITER_STRING_ARRAYS:
                        return iter->object.prop_offsets.string_arrays;
                case PROP_ITER_OBJECT_ARRAYS:
                        return iter->object.prop_offsets.object_arrays;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR)
        }
}

static bool prop_iter_read_colum_entry(collection_iter_state *state, memfile *memfile)
{
        JAK_ASSERT(state->current_column_group.current_column.current_entry.idx
                   < state->current_column_group.current_column.num_elem);

        u32 current_idx = state->current_column_group.current_column.current_entry.idx;
        offset_t entry_off = state->current_column_group.current_column.elem_offsets[current_idx];
        memfile_seek(memfile, entry_off);

        state->current_column_group.current_column.current_entry.array_length = *MEMFILE_READ_TYPE(memfile,
                                                                                                       u32);
        state->current_column_group.current_column.current_entry.array_base = MEMFILE_PEEK(memfile, void);

        return (++state->current_column_group.current_column.current_entry.idx)
               < state->current_column_group.current_column.num_elem;
}

static bool prop_iter_read_column(collection_iter_state *state, memfile *memfile)
{
        JAK_ASSERT(state->current_column_group.current_column.idx < state->current_column_group.num_columns);

        u32 current_idx = state->current_column_group.current_column.idx;
        offset_t column_off = state->current_column_group.column_offs[current_idx];
        memfile_seek(memfile, column_off);
        const column_header *header = MEMFILE_READ_TYPE(memfile, column_header);

        JAK_ASSERT(header->marker == MARKER_SYMBOL_COLUMN);
        state->current_column_group.current_column.name = header->column_name;
        state->current_column_group.current_column.type = int_marker_to_field_type(header->value_type);

        state->current_column_group.current_column.num_elem = header->num_entries;
        state->current_column_group.current_column.elem_offsets =
                MEMFILE_READ_TYPE_LIST(memfile, offset_t, header->num_entries);
        state->current_column_group.current_column.elem_positions =
                MEMFILE_READ_TYPE_LIST(memfile, u32, header->num_entries);
        state->current_column_group.current_column.current_entry.idx = 0;

        return (++state->current_column_group.current_column.idx) < state->current_column_group.num_columns;
}

static bool collection_iter_read_next_column_group(collection_iter_state *state, memfile *memfile)
{
        JAK_ASSERT(state->current_column_group_idx < state->num_column_groups);
        memfile_seek(memfile, state->column_group_offsets[state->current_column_group_idx]);
        const column_group_header *header = MEMFILE_READ_TYPE(memfile, column_group_header);
        JAK_ASSERT(header->marker == MARKER_SYMBOL_COLUMN_GROUP);
        state->current_column_group.num_columns = header->num_columns;
        state->current_column_group.num_objects = header->num_objects;
        state->current_column_group.object_ids = MEMFILE_READ_TYPE_LIST(memfile, unique_id_t,
                                                                            header->num_objects);
        state->current_column_group.column_offs = MEMFILE_READ_TYPE_LIST(memfile, offset_t,
                                                                             header->num_columns);
        state->current_column_group.current_column.idx = 0;

        return (++state->current_column_group_idx) < state->num_column_groups;
}

static void prop_iter_cursor_init(prop_iter *iter)
{
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_NULLS, prop_offsets.nulls));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_BOOLS, prop_offsets.bools));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT8S, prop_offsets.int8s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT16S, prop_offsets.int16s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT32S, prop_offsets.int32s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT64S, prop_offsets.int64s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT8S, prop_offsets.uint8s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT16S, prop_offsets.uint16s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT32S, prop_offsets.uint32s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT64S, prop_offsets.uint64s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_FLOATS, prop_offsets.floats));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_STRINGS, prop_offsets.strings));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_OBJECTS, prop_offsets.objects));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_NULL_ARRAYS, prop_offsets.null_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_BOOL_ARRAYS, prop_offsets.bool_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT8_ARRAYS, prop_offsets.int8_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT16_ARRAYS, prop_offsets.int16_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT32_ARRAYS, prop_offsets.int32_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_INT64_ARRAYS, prop_offsets.int64_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT8_ARRAYS, prop_offsets.uint8_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT16_ARRAYS, prop_offsets.uint16_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT32_ARRAYS, prop_offsets.uint32_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_UINT64_ARRAYS, prop_offsets.uint64_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_FLOAT_ARRAYS, prop_offsets.float_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_STRING_ARRAYS, prop_offsets.string_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(PROP_ITER_OBJECT_ARRAYS, prop_offsets.object_arrays));

        if (iter->mode == PROP_ITER_MODE_COLLECTION) {
                iter->mode_collection.collection_start_off = offset_by_state(iter);
                memfile_seek(&iter->record_table_memfile, iter->mode_collection.collection_start_off);
                const object_array_header
                        *header = MEMFILE_READ_TYPE(&iter->record_table_memfile, object_array_header);
                iter->mode_collection.num_column_groups = header->num_entries;
                iter->mode_collection.current_column_group_idx = 0;
                iter->mode_collection.column_group_keys = MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                     archive_field_sid_t,
                                                                                     iter->mode_collection.num_column_groups);
                iter->mode_collection.column_group_offsets = MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                        offset_t,
                                                                                        iter->mode_collection.num_column_groups);

        } else {
                iter->mode_object.current_prop_group_off = offset_by_state(iter);
                memfile_seek(&iter->record_table_memfile, iter->mode_object.current_prop_group_off);
                int_embedded_fixed_props_read(&iter->mode_object.prop_group_header, &iter->record_table_memfile);
                iter->mode_object.prop_data_off = memfile_tell(&iter->record_table_memfile);
        }

}

#define SET_STATE_FOR_FALL_THROUGH(iter, prop_offset_type, mask_group, mask_type, next_state)                          \
{                                                                                                                      \
    if ((iter->object.prop_offsets.prop_offset_type != 0) &&                                                           \
        (ARE_BITS_SET(iter->mask, mask_group | mask_type)))                                                  \
    {                                                                                                                  \
        iter->prop_cursor = next_state;                                                                                \
        break;                                                                                                         \
    }                                                                                                                  \
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

static prop_iter_state_e prop_iter_state_next(prop_iter *iter)
{
        switch (iter->prop_cursor) {
                case PROP_ITER_INIT: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                    nulls,
                                                                    ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                    ARCHIVE_ITER_MASK_NULL,
                                                                    PROP_ITER_NULLS)
                case PROP_ITER_NULLS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     bools,
                                                                     ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     ARCHIVE_ITER_MASK_BOOLEAN,
                                                                     PROP_ITER_BOOLS)
                case PROP_ITER_BOOLS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     int8s,
                                                                     ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     ARCHIVE_ITER_MASK_INT8,
                                                                     PROP_ITER_INT8S)
                case PROP_ITER_INT8S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     int16s,
                                                                     ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     ARCHIVE_ITER_MASK_INT16,
                                                                     PROP_ITER_INT16S)
                case PROP_ITER_INT16S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      int32s,
                                                                      ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      ARCHIVE_ITER_MASK_INT32,
                                                                      PROP_ITER_INT32S)
                case PROP_ITER_INT32S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      int64s,
                                                                      ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      ARCHIVE_ITER_MASK_INT64,
                                                                      PROP_ITER_INT64S)
                case PROP_ITER_INT64S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      uint8s,
                                                                      ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      ARCHIVE_ITER_MASK_UINT8,
                                                                      PROP_ITER_UINT8S)
                case PROP_ITER_UINT8S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      uint16s,
                                                                      ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      ARCHIVE_ITER_MASK_UINT16,
                                                                      PROP_ITER_UINT16S)
                case PROP_ITER_UINT16S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       uint32s,
                                                                       ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       ARCHIVE_ITER_MASK_UINT32,
                                                                       PROP_ITER_UINT32S)
                case PROP_ITER_UINT32S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       uint64s,
                                                                       ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       ARCHIVE_ITER_MASK_UINT64,
                                                                       PROP_ITER_UINT64S)
                case PROP_ITER_UINT64S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       floats,
                                                                       ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       ARCHIVE_ITER_MASK_NUMBER,
                                                                       PROP_ITER_FLOATS)
                case PROP_ITER_FLOATS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      strings,
                                                                      ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      ARCHIVE_ITER_MASK_STRING,
                                                                      PROP_ITER_STRINGS)
                case PROP_ITER_STRINGS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       objects,
                                                                       ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       ARCHIVE_ITER_MASK_OBJECT,
                                                                       PROP_ITER_OBJECTS)
                case PROP_ITER_OBJECTS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       null_arrays,
                                                                       ARCHIVE_ITER_MASK_ARRAYS,
                                                                       ARCHIVE_ITER_MASK_NULL,
                                                                       PROP_ITER_NULL_ARRAYS)
                case PROP_ITER_NULL_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           bool_arrays,
                                                                           ARCHIVE_ITER_MASK_ARRAYS,
                                                                           ARCHIVE_ITER_MASK_BOOLEAN,
                                                                           PROP_ITER_BOOL_ARRAYS)
                case PROP_ITER_BOOL_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           int8_arrays,
                                                                           ARCHIVE_ITER_MASK_ARRAYS,
                                                                           ARCHIVE_ITER_MASK_INT8,
                                                                           PROP_ITER_INT8_ARRAYS)
                case PROP_ITER_INT8_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           int16_arrays,
                                                                           ARCHIVE_ITER_MASK_ARRAYS,
                                                                           ARCHIVE_ITER_MASK_INT16,
                                                                           PROP_ITER_INT16_ARRAYS)
                case PROP_ITER_INT16_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            int32_arrays,
                                                                            ARCHIVE_ITER_MASK_ARRAYS,
                                                                            ARCHIVE_ITER_MASK_INT32,
                                                                            PROP_ITER_INT32_ARRAYS)
                case PROP_ITER_INT32_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            int64_arrays,
                                                                            ARCHIVE_ITER_MASK_ARRAYS,
                                                                            ARCHIVE_ITER_MASK_INT64,
                                                                            PROP_ITER_INT64_ARRAYS)
                case PROP_ITER_INT64_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            uint8_arrays,
                                                                            ARCHIVE_ITER_MASK_ARRAYS,
                                                                            ARCHIVE_ITER_MASK_UINT8,
                                                                            PROP_ITER_UINT8_ARRAYS)
                case PROP_ITER_UINT8_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            uint16_arrays,
                                                                            ARCHIVE_ITER_MASK_ARRAYS,
                                                                            ARCHIVE_ITER_MASK_UINT16,
                                                                            PROP_ITER_UINT16_ARRAYS)
                case PROP_ITER_UINT16_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             uint32_arrays,
                                                                             ARCHIVE_ITER_MASK_ARRAYS,
                                                                             ARCHIVE_ITER_MASK_UINT32,
                                                                             PROP_ITER_UINT32_ARRAYS)
                case PROP_ITER_UINT32_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             uint64_arrays,
                                                                             ARCHIVE_ITER_MASK_ARRAYS,
                                                                             ARCHIVE_ITER_MASK_UINT64,
                                                                             PROP_ITER_UINT64_ARRAYS)
                case PROP_ITER_UINT64_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             float_arrays,
                                                                             ARCHIVE_ITER_MASK_ARRAYS,
                                                                             ARCHIVE_ITER_MASK_NUMBER,
                                                                             PROP_ITER_FLOAT_ARRAYS)
                case PROP_ITER_FLOAT_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            string_arrays,
                                                                            ARCHIVE_ITER_MASK_ARRAYS,
                                                                            ARCHIVE_ITER_MASK_STRING,
                                                                            PROP_ITER_STRING_ARRAYS)
                case PROP_ITER_STRING_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             object_arrays,
                                                                             ARCHIVE_ITER_MASK_ARRAYS,
                                                                             ARCHIVE_ITER_MASK_OBJECT,
                                                                             PROP_ITER_OBJECT_ARRAYS)
                case PROP_ITER_OBJECT_ARRAYS:
                        iter->prop_cursor = PROP_ITER_DONE;
                        break;

                case PROP_ITER_DONE:
                        break;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }

        iter->mode = iter->prop_cursor == PROP_ITER_OBJECT_ARRAYS ? PROP_ITER_MODE_COLLECTION
                                                                      : PROP_ITER_MODE_OBJECT;

        if (iter->prop_cursor != PROP_ITER_DONE) {
                prop_iter_cursor_init(iter);
        }
        return iter->prop_cursor;
}

#pragma GCC diagnostic pop

static void prop_iter_state_init(prop_iter *iter)
{
        iter->prop_cursor = PROP_ITER_INIT;
        iter->mode = PROP_ITER_MODE_OBJECT;
}

static bool archive_prop_iter_from_memblock(prop_iter *iter, err *err, u16 mask,
                                            memblock *memblock, offset_t object_offset)
{
        ERROR_IF_NULL(iter)
        ERROR_IF_NULL(err)
        ERROR_IF_NULL(memblock)

        iter->mask = mask;
        if (!memfile_open(&iter->record_table_memfile, memblock, READ_ONLY)) {
                ERROR(err, ERR_MEMFILEOPEN_FAILED)
                return false;
        }
        if (!memfile_seek(&iter->record_table_memfile, object_offset)) {
                ERROR(err, ERR_MEMFILESEEK_FAILED)
                return false;
        }
        if (!init_object_from_memfile(&iter->object, &iter->record_table_memfile)) {
                ERROR(err, ERR_INTERNALERR);
                return false;
        }

        error_init(&iter->err);
        prop_iter_state_init(iter);
        prop_iter_state_next(iter);

        return true;
}

bool archive_prop_iter_from_archive(prop_iter *iter, err *err, u16 mask,
                                        archive *archive)
{
        return archive_prop_iter_from_memblock(iter, err, mask, archive->record_table.record_db, 0);
}

bool archive_prop_iter_from_object(prop_iter *iter, u16 mask, err *err,
                                       const archive_object *obj)
{
        return archive_prop_iter_from_memblock(iter, err, mask, obj->memfile.memblock, obj->offset);
}

static enum archive_field_type get_basic_type(prop_iter_state_e state)
{
        switch (state) {
                case PROP_ITER_NULLS:
                case PROP_ITER_NULL_ARRAYS:
                        return FIELD_NULL;
                case PROP_ITER_BOOLS:
                case PROP_ITER_BOOL_ARRAYS:
                        return FIELD_BOOLEAN;
                case PROP_ITER_INT8S:
                case PROP_ITER_INT8_ARRAYS:
                        return FIELD_INT8;
                case PROP_ITER_INT16S:
                case PROP_ITER_INT16_ARRAYS:
                        return FIELD_INT16;
                case PROP_ITER_INT32S:
                case PROP_ITER_INT32_ARRAYS:
                        return FIELD_INT32;
                case PROP_ITER_INT64S:
                case PROP_ITER_INT64_ARRAYS:
                        return FIELD_INT64;
                case PROP_ITER_UINT8S:
                case PROP_ITER_UINT8_ARRAYS:
                        return FIELD_UINT8;
                case PROP_ITER_UINT16S:
                case PROP_ITER_UINT16_ARRAYS:
                        return FIELD_UINT16;
                case PROP_ITER_UINT32S:
                case PROP_ITER_UINT32_ARRAYS:
                        return FIELD_UINT32;
                case PROP_ITER_UINT64S:
                case PROP_ITER_UINT64_ARRAYS:
                        return FIELD_UINT64;
                case PROP_ITER_FLOATS:
                case PROP_ITER_FLOAT_ARRAYS:
                        return FIELD_FLOAT;
                case PROP_ITER_STRINGS:
                case PROP_ITER_STRING_ARRAYS:
                        return FIELD_STRING;
                case PROP_ITER_OBJECTS:
                case PROP_ITER_OBJECT_ARRAYS:
                        return FIELD_OBJECT;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }
}

static bool is_array_type(prop_iter_state_e state)
{
        switch (state) {
                case PROP_ITER_NULLS:
                case PROP_ITER_BOOLS:
                case PROP_ITER_INT8S:
                case PROP_ITER_INT16S:
                case PROP_ITER_INT32S:
                case PROP_ITER_INT64S:
                case PROP_ITER_UINT8S:
                case PROP_ITER_UINT16S:
                case PROP_ITER_UINT32S:
                case PROP_ITER_UINT64S:
                case PROP_ITER_FLOATS:
                case PROP_ITER_STRINGS:
                case PROP_ITER_OBJECTS:
                        return false;
                case PROP_ITER_NULL_ARRAYS:
                case PROP_ITER_BOOL_ARRAYS:
                case PROP_ITER_INT8_ARRAYS:
                case PROP_ITER_INT16_ARRAYS:
                case PROP_ITER_INT32_ARRAYS:
                case PROP_ITER_INT64_ARRAYS:
                case PROP_ITER_UINT8_ARRAYS:
                case PROP_ITER_UINT16_ARRAYS:
                case PROP_ITER_UINT32_ARRAYS:
                case PROP_ITER_UINT64_ARRAYS:
                case PROP_ITER_FLOAT_ARRAYS:
                case PROP_ITER_STRING_ARRAYS:
                case PROP_ITER_OBJECT_ARRAYS:
                        return true;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }
}

bool archive_prop_iter_next(prop_iter_mode_e *type, archive_value_vector *value_vector,
                                independent_iter_state *collection_iter, prop_iter *prop_iter)
{
        ERROR_IF_NULL(type);
        ERROR_IF_NULL(prop_iter);

        if (prop_iter->prop_cursor != PROP_ITER_DONE) {
                switch (prop_iter->mode) {
                        case PROP_ITER_MODE_OBJECT: {
                                value_vector->keys = prop_iter->mode_object.prop_group_header.keys;

                                prop_iter->mode_object.type = get_basic_type(prop_iter->prop_cursor);
                                prop_iter->mode_object.is_array = is_array_type(prop_iter->prop_cursor);

                                value_vector->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;
                                value_vector->prop_type = prop_iter->mode_object.type;
                                value_vector->is_array = prop_iter->mode_object.is_array;

                                if (value_vector
                                    && !archive_value_vector_from_prop_iter(value_vector, &prop_iter->err,
                                                                                prop_iter)) {
                                        ERROR(&prop_iter->err, ERR_VITEROPEN_FAILED);
                                        return false;
                                }
                        }
                                break;
                        case PROP_ITER_MODE_COLLECTION: {
                                collection_iter->state = prop_iter->mode_collection;
                                memfile_open(&collection_iter->record_table_memfile,
                                             prop_iter->record_table_memfile.memblock,
                                             READ_ONLY);
                                error_init(&collection_iter->err);
                        }
                                break;
                        default: ERROR(&prop_iter->err, ERR_INTERNALERR);
                                return false;
                }
                *type = prop_iter->mode;
                prop_iter_state_next(prop_iter);
                return true;
        } else {
                return false;
        }
}

const archive_field_sid_t *
archive_collection_iter_get_keys(u32 *num_keys, independent_iter_state *iter)
{
        if (num_keys && iter) {
                *num_keys = iter->state.num_column_groups;
                return iter->state.column_group_keys;
        } else {
                ERROR(&iter->err, ERR_NULLPTR);
                return NULL;
        }
}

bool archive_collection_next_column_group(independent_iter_state *group_iter,
                                              independent_iter_state *iter)
{
        ERROR_IF_NULL(group_iter)
        ERROR_IF_NULL(iter)

        if (iter->state.current_column_group_idx < iter->state.num_column_groups) {
                collection_iter_read_next_column_group(&iter->state, &iter->record_table_memfile);
                memfile_open(&group_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                group_iter->state = iter->state;
                error_init(&group_iter->err);
                return true;
        } else {
                return false;
        }
}

const unique_id_t *
archive_column_group_get_object_ids(u32 *num_objects, independent_iter_state *iter)
{
        if (num_objects && iter) {
                *num_objects = iter->state.current_column_group.num_objects;
                return iter->state.current_column_group.object_ids;
        } else {
                ERROR(&iter->err, ERR_NULLPTR);
                return NULL;
        }
}

bool archive_column_group_next_column(independent_iter_state *column_iter,
                                          independent_iter_state *iter)
{
        ERROR_IF_NULL(column_iter)
        ERROR_IF_NULL(iter)

        if (iter->state.current_column_group.current_column.idx < iter->state.current_column_group.num_columns) {
                prop_iter_read_column(&iter->state, &iter->record_table_memfile);
                memfile_open(&column_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                column_iter->state = iter->state;
                error_init(&column_iter->err);
                return true;
        } else {
                return false;
        }
}

bool archive_column_get_name(archive_field_sid_t *name, enum archive_field_type *type,
                                 independent_iter_state *column_iter)
{
        ERROR_IF_NULL(column_iter)
        OPTIONAL_SET(name, column_iter->state.current_column_group.current_column.name)
        OPTIONAL_SET(type, column_iter->state.current_column_group.current_column.type)
        return true;
}

const u32 *
archive_column_get_entry_positions(u32 *num_entry, independent_iter_state *column_iter)
{
        if (num_entry && column_iter) {
                *num_entry = column_iter->state.current_column_group.current_column.num_elem;
                return column_iter->state.current_column_group.current_column.elem_positions;
        } else {
                ERROR(&column_iter->err, ERR_NULLPTR);
                return NULL;
        }
}

bool
archive_column_next_entry(independent_iter_state *entry_iter, independent_iter_state *iter)
{
        ERROR_IF_NULL(entry_iter)
        ERROR_IF_NULL(iter)

        if (iter->state.current_column_group.current_column.current_entry.idx
            < iter->state.current_column_group.current_column.num_elem) {
                prop_iter_read_colum_entry(&iter->state, &iter->record_table_memfile);
                memfile_open(&entry_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                entry_iter->state = iter->state;
                error_init(&entry_iter->err);
                return true;
        } else {
                return false;
        }
}

bool archive_column_entry_get_type(enum archive_field_type *type, independent_iter_state *entry)
{
        ERROR_IF_NULL(type)
        ERROR_IF_NULL(entry)
        *type = entry->state.current_column_group.current_column.type;
        return true;
}

#define DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name, basic_type)                            \
const built_in_type *                                                                                   \
archive_column_entry_get_##name(u32 *array_length, independent_iter_state *entry)              \
{                                                                                                                      \
    if (array_length && entry) {                                                                                       \
        if (entry->state.current_column_group.current_column.type == basic_type)                                       \
        {                                                                                                              \
            *array_length =  entry->state.current_column_group.current_column.current_entry.array_length;              \
            return (const built_in_type *) entry->state.current_column_group.current_column.current_entry.array_base;  \
        } else {                                                                                                       \
            ERROR(&entry->err, ERR_TYPEMISMATCH);                                                        \
            return NULL;                                                                                               \
        }                                                                                                              \
    } else {                                                                                                           \
        ERROR(&entry->err, ERR_NULLPTR);                                                                 \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i8_t, int8s, FIELD_INT8);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i16_t, int16s, FIELD_INT16);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i32_t, int32s, FIELD_INT32);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_i64_t, int64s, FIELD_INT64);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u8_t, uint8s, FIELD_UINT8);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u16_t, uint16s, FIELD_UINT16);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u32_t, uint32s, FIELD_UINT32);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u64_t, uint64s, FIELD_UINT64);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_sid_t, strings, FIELD_STRING);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_number_t, numbers, FIELD_FLOAT);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_boolean_t, booleans, FIELD_BOOLEAN);

DECLARE_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(archive_field_u32_t, nulls, FIELD_NULL);

bool archive_column_entry_get_objects(column_object_iter *iter, independent_iter_state *entry)
{
        ERROR_IF_NULL(iter)
        ERROR_IF_NULL(entry)

        iter->entry_state = entry->state;
        memfile_open(&iter->memfile, entry->record_table_memfile.memblock, READ_ONLY);
        memfile_seek(&iter->memfile,
                     entry->state.current_column_group.current_column.elem_offsets[
                             entry->state.current_column_group.current_column.current_entry.idx - 1] + sizeof(u32));
        iter->next_obj_off = memfile_tell(&iter->memfile);
        error_init(&iter->err);

        return true;
}

const archive_object *archive_column_entry_object_iter_next_object(column_object_iter *iter)
{
        if (iter) {
                if (iter->next_obj_off != 0) {
                        memfile_seek(&iter->memfile, iter->next_obj_off);
                        if (init_object_from_memfile(&iter->obj, &iter->memfile)) {
                                iter->next_obj_off = iter->obj.next_obj_off;
                                return &iter->obj;
                        } else {
                                ERROR(&iter->err, ERR_INTERNALERR);
                                return NULL;
                        }
                } else {
                        return NULL;
                }
        } else {
                ERROR(&iter->err, ERR_NULLPTR);
                return NULL;
        }
}

bool archive_object_get_object_id(unique_id_t *id, const archive_object *object)
{
        ERROR_IF_NULL(id)
        ERROR_IF_NULL(object)
        *id = object->object_id;
        return true;
}

bool archive_object_get_prop_iter(prop_iter *iter, const archive_object *object)
{
        // XXXX archive_prop_iter_from_object()
        UNUSED(iter);
        UNUSED(object);
        return false;
}

bool archive_value_vector_get_object_id(unique_id_t *id, const archive_value_vector *iter)
{
        ERROR_IF_NULL(id)
        ERROR_IF_NULL(iter)
        *id = iter->object_id;
        return true;
}

const archive_field_sid_t *
archive_value_vector_get_keys(u32 *num_keys, archive_value_vector *iter)
{
        if (num_keys && iter) {
                *num_keys = iter->value_max_idx;
                return iter->keys;
        } else {
                ERROR(&iter->err, ERR_NULLPTR)
                return NULL;
        }
}

static void value_vector_init_object_basic(archive_value_vector *value)
{
        value->data.object.offsets =
                MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, offset_t, value->value_max_idx);
}

static void value_vector_init_fixed_length_types_basic(archive_value_vector *value)
{
        JAK_ASSERT(!value->is_array);

        switch (value->prop_type) {
                case FIELD_INT8:
                        value->data.basic.values.int8s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                          archive_field_i8_t);
                        break;
                case FIELD_INT16:
                        value->data.basic.values.int16s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                           archive_field_i16_t);
                        break;
                case FIELD_INT32:
                        value->data.basic.values.int32s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                           archive_field_i32_t);
                        break;
                case FIELD_INT64:
                        value->data.basic.values.int64s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                           archive_field_i64_t);
                        break;
                case FIELD_UINT8:
                        value->data.basic.values.uint8s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                           archive_field_u8_t);
                        break;
                case FIELD_UINT16:
                        value->data.basic.values.uint16s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                            archive_field_u16_t);
                        break;
                case FIELD_UINT32:
                        value->data.basic.values.uint32s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                            archive_field_u32_t);
                        break;
                case FIELD_UINT64:
                        value->data.basic.values.uint64s = MEMFILE_PEEK(&value->record_table_memfile,
                                                                            archive_field_u64_t);
                        break;
                case FIELD_FLOAT:
                        value->data.basic.values.numbers = MEMFILE_PEEK(&value->record_table_memfile,
                                                                            archive_field_number_t);
                        break;
                case FIELD_STRING:
                        value->data.basic.values.strings = MEMFILE_PEEK(&value->record_table_memfile,
                                                                            archive_field_sid_t);
                        break;
                case FIELD_BOOLEAN:
                        value->data.basic.values.booleans = MEMFILE_PEEK(&value->record_table_memfile,
                                                                             archive_field_boolean_t);
                        break;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }
}

static void value_vector_init_fixed_length_types_null_arrays(archive_value_vector *value)
{
        JAK_ASSERT(value->is_array);
        JAK_ASSERT(value->prop_type == FIELD_NULL);
        value->data.arrays.meta.num_nulls_contained =
                MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, u32, value->value_max_idx);
}

static void value_vector_init_fixed_length_types_non_null_arrays(archive_value_vector *value)
{
        JAK_ASSERT (value->is_array);

        value->data.arrays.meta.array_lengths =
                MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, u32, value->value_max_idx);

        switch (value->prop_type) {
                case FIELD_INT8:
                        value->data.arrays.values.int8s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                archive_field_i8_t);
                        break;
                case FIELD_INT16:
                        value->data.arrays.values.int16s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 archive_field_i16_t);
                        break;
                case FIELD_INT32:
                        value->data.arrays.values.int32s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 archive_field_i32_t);
                        break;
                case FIELD_INT64:
                        value->data.arrays.values.int64s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 archive_field_i64_t);
                        break;
                case FIELD_UINT8:
                        value->data.arrays.values.uint8s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 archive_field_u8_t);
                        break;
                case FIELD_UINT16:
                        value->data.arrays.values.uint16s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  archive_field_u16_t);
                        break;
                case FIELD_UINT32:
                        value->data.arrays.values.uint32s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  archive_field_u32_t);
                        break;
                case FIELD_UINT64:
                        value->data.arrays.values.uint64s_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  archive_field_u64_t);
                        break;
                case FIELD_FLOAT:
                        value->data.arrays.values.numbers_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  archive_field_number_t);
                        break;
                case FIELD_STRING:
                        value->data.arrays.values.strings_base = MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  archive_field_sid_t);
                        break;
                case FIELD_BOOLEAN:
                        value->data.arrays.values.booleans_base =
                                MEMFILE_PEEK(&value->record_table_memfile, archive_field_boolean_t);
                        break;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }
}

static void value_vector_init_fixed_length_types(archive_value_vector *value)
{
        if (value->is_array) {
                value_vector_init_fixed_length_types_non_null_arrays(value);
        } else {
                value_vector_init_fixed_length_types_basic(value);
        }
}

static void value_vector_init_object(archive_value_vector *value)
{
        if (value->is_array) {
                //value_vector_init_object_array(value);
        } else {
                value_vector_init_object_basic(value);
        }
}

bool archive_value_vector_from_prop_iter(archive_value_vector *value, err *err,
                                             prop_iter *prop_iter)
{
        ERROR_IF_NULL(value);
        ERROR_IF_NULL(prop_iter);

        ERROR_IF_AND_RETURN (prop_iter->mode != PROP_ITER_MODE_OBJECT,
                             &prop_iter->err,
                             ERR_ITER_OBJECT_NEEDED,
                             false)

        error_init(&value->err);

        value->prop_iter = prop_iter;
        value->data_off = prop_iter->mode_object.prop_data_off;
        value->object_id = prop_iter->object.object_id;

        if (!memfile_open(&value->record_table_memfile, prop_iter->record_table_memfile.memblock, READ_ONLY)) {
                ERROR(err, ERR_MEMFILEOPEN_FAILED);
                return false;
        }
        if (!memfile_skip(&value->record_table_memfile, value->data_off)) {
                ERROR(err, ERR_MEMFILESKIP_FAILED);
                return false;
        }

        value->prop_type = prop_iter->mode_object.type;
        value->is_array = prop_iter->mode_object.is_array;
        value->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;

        switch (value->prop_type) {
                case FIELD_OBJECT:
                        value_vector_init_object(value);
                        break;
                case FIELD_NULL:
                        if (value->is_array) {
                                value_vector_init_fixed_length_types_null_arrays(value);
                        }
                        break;
                case FIELD_INT8:
                case FIELD_INT16:
                case FIELD_INT32:
                case FIELD_INT64:
                case FIELD_UINT8:
                case FIELD_UINT16:
                case FIELD_UINT32:
                case FIELD_UINT64:
                case FIELD_FLOAT:
                case FIELD_STRING:
                case FIELD_BOOLEAN:
                        value_vector_init_fixed_length_types(value);
                        break;
                default: ERROR_PRINT_AND_DIE(ERR_INTERNALERR);
        }

        return true;
}

bool archive_value_vector_get_basic_type(enum archive_field_type *type,
                                                 const archive_value_vector *value)
{
        ERROR_IF_NULL(type)
        ERROR_IF_NULL(value)
        *type = value->prop_type;
        return true;
}

bool archive_value_vector_is_array_type(bool *is_array, const archive_value_vector *value)
{
        ERROR_IF_NULL(is_array)
        ERROR_IF_NULL(value)
        *is_array = value->is_array;
        return true;
}

bool archive_value_vector_get_length(u32 *length, const archive_value_vector *value)
{
        ERROR_IF_NULL(length)
        ERROR_IF_NULL(value)
        *length = value->value_max_idx;
        return true;
}

bool archive_value_vector_is_of_objects(bool *is_object, archive_value_vector *value)
{
        ERROR_IF_NULL(is_object)
        ERROR_IF_NULL(value)

        *is_object = value->prop_type == FIELD_OBJECT && !value->is_array;

        return true;
}

bool archive_value_vector_get_object_at(archive_object *object, u32 idx,
                                                archive_value_vector *value)
{
        ERROR_IF_NULL(object)
        ERROR_IF_NULL(value)

        if (idx >= value->value_max_idx) {
                ERROR(&value->err, ERR_OUTOFBOUNDS);
                return false;
        }

        bool is_object;

        archive_value_vector_is_of_objects(&is_object, value);

        if (is_object) {
                memfile_seek(&value->record_table_memfile, value->data.object.offsets[idx]);
                init_object_from_memfile(&value->data.object.object, &value->record_table_memfile);
                *object = value->data.object.object;
                return true;
        } else {
                ERROR(&value->err, ERR_ITER_NOOBJ);
                return false;
        }
}

#define DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name, basic_type)                                            \
bool                                                                                                    \
archive_value_vector_is_##name(bool *type_match, archive_value_vector *value)                          \
{                                                                                                                      \
    ERROR_IF_NULL(type_match)                                                                               \
    ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    *type_match = value->prop_type == basic_type;                                                                      \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8, FIELD_INT8)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16, FIELD_INT16)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32, FIELD_INT32)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64, FIELD_INT64)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8, FIELD_UINT8)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16, FIELD_UINT16)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32, FIELD_UINT32)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64, FIELD_UINT64)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string, FIELD_STRING)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number, FIELD_FLOAT)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean, FIELD_BOOLEAN)

DECLARE_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null, FIELD_NULL)

#define DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(names, name, built_in_type, err_code)                       \
const built_in_type *                                                                                   \
archive_value_vector_get_##names(u32 *num_values, archive_value_vector *value)                    \
{                                                                                                                      \
    ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (archive_value_vector_is_array_type(&is_array, value) &&                                                 \
        archive_value_vector_is_##name(&type_match, value) && !is_array)                                        \
    {                                                                                                                  \
        OPTIONAL_SET(num_values, value->value_max_idx)                                                          \
        return value->data.basic.values.names;                                                                         \
    } else                                                                                                             \
    {                                                                                                                  \
        ERROR(&value->err, err_code);                                                                           \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, int8, archive_field_i8_t, ERR_ITER_NOINT8)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, int16, archive_field_i16_t, ERR_ITER_NOINT16)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, int32, archive_field_i32_t, ERR_ITER_NOINT32)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, int64, archive_field_i64_t, ERR_ITER_NOINT64)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, uint8, archive_field_u8_t, ERR_ITER_NOUINT8)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, uint16, archive_field_u16_t, ERR_ITER_NOUINT16)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, uint32, archive_field_u32_t, ERR_ITER_NOUINT32)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, uint64, archive_field_u64_t, ERR_ITER_NOUINT64)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, string, archive_field_sid_t, ERR_ITER_NOSTRING)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, number, archive_field_number_t, ERR_ITER_NONUMBER)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, boolean, archive_field_boolean_t, ERR_ITER_NOBOOL)

const archive_field_u32_t *
archive_value_vector_get_null_arrays(u32 *num_values, archive_value_vector *value)
{
        ERROR_IF_NULL(value)

        bool is_array;
        bool type_match;

        if (archive_value_vector_is_array_type(&is_array, value) &&
            archive_value_vector_is_null(&type_match, value)
            && is_array) {
                OPTIONAL_SET(num_values, value->value_max_idx);
                return value->data.arrays.meta.num_nulls_contained;
        } else {
                return NULL;
        }
}

#define DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type, base)                               \
const built_in_type *                                                                                   \
archive_value_vector_get_##name##_arrays_at(u32 *array_length, u32 idx,                               \
                                               archive_value_vector *value)                                   \
{                                                                                                                      \
    ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (idx < value->value_max_idx && archive_value_vector_is_array_type(&is_array, value) &&                   \
        archive_value_vector_is_##name(&type_match, value) && is_array)                                         \
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

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, archive_field_i8_t, int8s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, archive_field_i16_t, int16s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, archive_field_i32_t, int32s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, archive_field_i64_t, int64s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, archive_field_u8_t, uint8s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, archive_field_u16_t, uint16s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, archive_field_u32_t, uint32s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, archive_field_u64_t, uint64s_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, archive_field_sid_t, strings_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, archive_field_number_t, numbers_base)

DECLARE_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, archive_field_boolean_t, booleans_base)

void archive_int_reset_carbon_object_mem_file(archive_object *object)
{
        UNUSED(object);
        //  memfile_seek(&object->file, object->self);
        abort();
}
