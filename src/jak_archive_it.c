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

#include <jak_archive_it.h>
#include <jak_archive_int.h>

static bool init_object_from_memfile(jak_archive_object *obj, struct jak_memfile *memfile)
{
        JAK_ASSERT(obj);
        jak_offset_t object_off;
        jak_object_header *header;
        jak_object_flags_u flags;

        object_off = memfile_tell(memfile);
        header = JAK_MEMFILE_READ_TYPE(memfile, jak_object_header);
        if (JAK_UNLIKELY(header->marker != JAK_MARKER_SYMBOL_OBJECT_BEGIN)) {
                return false;
        }

        flags.value = header->flags;
        jak_int_read_prop_offsets(&obj->prop_offsets, memfile, &flags);

        obj->object_id = header->oid;
        obj->offset = object_off;
        obj->next_obj_off = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
        memfile_open(&obj->memfile, memfile->memblock, READ_ONLY);

        return true;
}

#define STATE_AND_PROPERTY_EXISTS(state, property) \
    (iter->prop_cursor != state || iter->object.property != 0)

inline static jak_offset_t offset_by_state(jak_prop_iter *iter)
{
        switch (iter->prop_cursor) {
                case JAK_PROP_ITER_NULLS:
                        return iter->object.prop_offsets.nulls;
                case JAK_PROP_ITER_BOOLS:
                        return iter->object.prop_offsets.bools;
                case JAK_PROP_ITER_INT8S:
                        return iter->object.prop_offsets.int8s;
                case JAK_PROP_ITER_INT16S:
                        return iter->object.prop_offsets.int16s;
                case JAK_PROP_ITER_INT32S:
                        return iter->object.prop_offsets.int32s;
                case JAK_PROP_ITER_INT64S:
                        return iter->object.prop_offsets.int64s;
                case JAK_PROP_ITER_UINT8S:
                        return iter->object.prop_offsets.uint8s;
                case JAK_PROP_ITER_UINT16S:
                        return iter->object.prop_offsets.uint16s;
                case JAK_PROP_ITER_UINT32S:
                        return iter->object.prop_offsets.uint32s;
                case JAK_PROP_ITER_UINT64S:
                        return iter->object.prop_offsets.uint64s;
                case JAK_PROP_ITER_FLOATS:
                        return iter->object.prop_offsets.floats;
                case JAK_PROP_ITER_STRINGS:
                        return iter->object.prop_offsets.strings;
                case JAK_PROP_ITER_OBJECTS:
                        return iter->object.prop_offsets.objects;
                case JAK_PROP_ITER_NULL_ARRAYS:
                        return iter->object.prop_offsets.null_arrays;
                case JAK_PROP_ITER_BOOL_ARRAYS:
                        return iter->object.prop_offsets.bool_arrays;
                case JAK_PROP_ITER_INT8_ARRAYS:
                        return iter->object.prop_offsets.int8_arrays;
                case JAK_PROP_ITER_INT16_ARRAYS:
                        return iter->object.prop_offsets.int16_arrays;
                case JAK_PROP_ITER_INT32_ARRAYS:
                        return iter->object.prop_offsets.int32_arrays;
                case JAK_PROP_ITER_INT64_ARRAYS:
                        return iter->object.prop_offsets.int64_arrays;
                case JAK_PROP_ITER_UINT8_ARRAYS:
                        return iter->object.prop_offsets.uint8_arrays;
                case JAK_PROP_ITER_UINT16_ARRAYS:
                        return iter->object.prop_offsets.uint16_arrays;
                case JAK_PROP_ITER_UINT32_ARRAYS:
                        return iter->object.prop_offsets.uint32_arrays;
                case JAK_PROP_ITER_UINT64_ARRAYS:
                        return iter->object.prop_offsets.uint64_arrays;
                case JAK_PROP_ITER_FLOAT_ARRAYS:
                        return iter->object.prop_offsets.float_arrays;
                case JAK_PROP_ITER_STRING_ARRAYS:
                        return iter->object.prop_offsets.string_arrays;
                case JAK_PROP_ITER_OBJECT_ARRAYS:
                        return iter->object.prop_offsets.object_arrays;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR)
        }
}

static bool prop_iter_read_colum_entry(jak_collection_iter_state *state, struct jak_memfile *memfile)
{
        JAK_ASSERT(state->current_column_group.current_column.current_entry.idx
                   < state->current_column_group.current_column.num_elem);

        jak_u32 current_idx = state->current_column_group.current_column.current_entry.idx;
        jak_offset_t entry_off = state->current_column_group.current_column.elem_offsets[current_idx];
        memfile_seek(memfile, entry_off);

        state->current_column_group.current_column.current_entry.array_length = *JAK_MEMFILE_READ_TYPE(memfile,
                                                                                                       jak_u32);
        state->current_column_group.current_column.current_entry.array_base = JAK_MEMFILE_PEEK(memfile, void);

        return (++state->current_column_group.current_column.current_entry.idx)
               < state->current_column_group.current_column.num_elem;
}

static bool prop_iter_read_column(jak_collection_iter_state *state, struct jak_memfile *memfile)
{
        JAK_ASSERT(state->current_column_group.current_column.idx < state->current_column_group.num_columns);

        jak_u32 current_idx = state->current_column_group.current_column.idx;
        jak_offset_t column_off = state->current_column_group.column_offs[current_idx];
        memfile_seek(memfile, column_off);
        const jak_column_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_column_header);

        JAK_ASSERT(header->marker == JAK_MARKER_SYMBOL_COLUMN);
        state->current_column_group.current_column.name = header->column_name;
        state->current_column_group.current_column.type = jak_int_marker_to_field_type(header->value_type);

        state->current_column_group.current_column.num_elem = header->num_entries;
        state->current_column_group.current_column.elem_offsets =
                JAK_MEMFILE_READ_TYPE_LIST(memfile, jak_offset_t, header->num_entries);
        state->current_column_group.current_column.elem_positions =
                JAK_MEMFILE_READ_TYPE_LIST(memfile, jak_u32, header->num_entries);
        state->current_column_group.current_column.current_entry.idx = 0;

        return (++state->current_column_group.current_column.idx) < state->current_column_group.num_columns;
}

static bool collection_iter_read_next_column_group(jak_collection_iter_state *state, struct jak_memfile *memfile)
{
        JAK_ASSERT(state->current_column_group_idx < state->num_column_groups);
        memfile_seek(memfile, state->column_group_offsets[state->current_column_group_idx]);
        const jak_column_group_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_column_group_header);
        JAK_ASSERT(header->marker == JAK_MARKER_SYMBOL_COLUMN_GROUP);
        state->current_column_group.num_columns = header->num_columns;
        state->current_column_group.num_objects = header->num_objects;
        state->current_column_group.object_ids = JAK_MEMFILE_READ_TYPE_LIST(memfile, jak_uid_t,
                                                                            header->num_objects);
        state->current_column_group.column_offs = JAK_MEMFILE_READ_TYPE_LIST(memfile, jak_offset_t,
                                                                             header->num_columns);
        state->current_column_group.current_column.idx = 0;

        return (++state->current_column_group_idx) < state->num_column_groups;
}

static void prop_iter_cursor_init(jak_prop_iter *iter)
{
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_NULLS, prop_offsets.nulls));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_BOOLS, prop_offsets.bools));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT8S, prop_offsets.int8s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT16S, prop_offsets.int16s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT32S, prop_offsets.int32s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT64S, prop_offsets.int64s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT8S, prop_offsets.uint8s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT16S, prop_offsets.uint16s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT32S, prop_offsets.uint32s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT64S, prop_offsets.uint64s));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_FLOATS, prop_offsets.floats));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_STRINGS, prop_offsets.strings));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_OBJECTS, prop_offsets.objects));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_NULL_ARRAYS, prop_offsets.null_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_BOOL_ARRAYS, prop_offsets.bool_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT8_ARRAYS, prop_offsets.int8_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT16_ARRAYS, prop_offsets.int16_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT32_ARRAYS, prop_offsets.int32_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_INT64_ARRAYS, prop_offsets.int64_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT8_ARRAYS, prop_offsets.uint8_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT16_ARRAYS, prop_offsets.uint16_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT32_ARRAYS, prop_offsets.uint32_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_UINT64_ARRAYS, prop_offsets.uint64_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_FLOAT_ARRAYS, prop_offsets.float_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_STRING_ARRAYS, prop_offsets.string_arrays));
        JAK_ASSERT(STATE_AND_PROPERTY_EXISTS(JAK_PROP_ITER_OBJECT_ARRAYS, prop_offsets.object_arrays));

        if (iter->mode == JAK_PROP_ITER_MODE_COLLECTION) {
                iter->mode_collection.collection_start_off = offset_by_state(iter);
                memfile_seek(&iter->record_table_memfile, iter->mode_collection.collection_start_off);
                const jak_object_array_header
                        *header = JAK_MEMFILE_READ_TYPE(&iter->record_table_memfile, jak_object_array_header);
                iter->mode_collection.num_column_groups = header->num_entries;
                iter->mode_collection.current_column_group_idx = 0;
                iter->mode_collection.column_group_keys = JAK_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                     jak_archive_field_sid_t,
                                                                                     iter->mode_collection.num_column_groups);
                iter->mode_collection.column_group_offsets = JAK_MEMFILE_READ_TYPE_LIST(&iter->record_table_memfile,
                                                                                        jak_offset_t,
                                                                                        iter->mode_collection.num_column_groups);

        } else {
                iter->mode_object.current_prop_group_off = offset_by_state(iter);
                memfile_seek(&iter->record_table_memfile, iter->mode_object.current_prop_group_off);
                jak_int_embedded_fixed_props_read(&iter->mode_object.prop_group_header, &iter->record_table_memfile);
                iter->mode_object.prop_data_off = memfile_tell(&iter->record_table_memfile);
        }

}

#define SET_STATE_FOR_FALL_THROUGH(iter, prop_offset_type, mask_group, mask_type, next_state)                          \
{                                                                                                                      \
    if ((iter->object.prop_offsets.prop_offset_type != 0) &&                                                           \
        (JAK_are_bits_set(iter->mask, mask_group | mask_type)))                                                  \
    {                                                                                                                  \
        iter->prop_cursor = next_state;                                                                                \
        break;                                                                                                         \
    }                                                                                                                  \
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

static jak_prop_iter_state_e prop_iter_state_next(jak_prop_iter *iter)
{
        switch (iter->prop_cursor) {
                case JAK_PROP_ITER_INIT: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                    nulls,
                                                                    JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                    JAK_ARCHIVE_ITER_MASK_NULL,
                                                                    JAK_PROP_ITER_NULLS)
                case JAK_PROP_ITER_NULLS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     bools,
                                                                     JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     JAK_ARCHIVE_ITER_MASK_BOOLEAN,
                                                                     JAK_PROP_ITER_BOOLS)
                case JAK_PROP_ITER_BOOLS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     int8s,
                                                                     JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     JAK_ARCHIVE_ITER_MASK_INT8,
                                                                     JAK_PROP_ITER_INT8S)
                case JAK_PROP_ITER_INT8S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                     int16s,
                                                                     JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                     JAK_ARCHIVE_ITER_MASK_INT16,
                                                                     JAK_PROP_ITER_INT16S)
                case JAK_PROP_ITER_INT16S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      int32s,
                                                                      JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      JAK_ARCHIVE_ITER_MASK_INT32,
                                                                      JAK_PROP_ITER_INT32S)
                case JAK_PROP_ITER_INT32S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      int64s,
                                                                      JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      JAK_ARCHIVE_ITER_MASK_INT64,
                                                                      JAK_PROP_ITER_INT64S)
                case JAK_PROP_ITER_INT64S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      uint8s,
                                                                      JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      JAK_ARCHIVE_ITER_MASK_UINT8,
                                                                      JAK_PROP_ITER_UINT8S)
                case JAK_PROP_ITER_UINT8S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      uint16s,
                                                                      JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      JAK_ARCHIVE_ITER_MASK_UINT16,
                                                                      JAK_PROP_ITER_UINT16S)
                case JAK_PROP_ITER_UINT16S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       uint32s,
                                                                       JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       JAK_ARCHIVE_ITER_MASK_UINT32,
                                                                       JAK_PROP_ITER_UINT32S)
                case JAK_PROP_ITER_UINT32S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       uint64s,
                                                                       JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       JAK_ARCHIVE_ITER_MASK_UINT64,
                                                                       JAK_PROP_ITER_UINT64S)
                case JAK_PROP_ITER_UINT64S: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       floats,
                                                                       JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       JAK_ARCHIVE_ITER_MASK_NUMBER,
                                                                       JAK_PROP_ITER_FLOATS)
                case JAK_PROP_ITER_FLOATS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                      strings,
                                                                      JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                      JAK_ARCHIVE_ITER_MASK_STRING,
                                                                      JAK_PROP_ITER_STRINGS)
                case JAK_PROP_ITER_STRINGS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       objects,
                                                                       JAK_ARCHIVE_ITER_MASK_PRIMITIVES,
                                                                       JAK_ARCHIVE_ITER_MASK_OBJECT,
                                                                       JAK_PROP_ITER_OBJECTS)
                case JAK_PROP_ITER_OBJECTS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                       null_arrays,
                                                                       JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                       JAK_ARCHIVE_ITER_MASK_NULL,
                                                                       JAK_PROP_ITER_NULL_ARRAYS)
                case JAK_PROP_ITER_NULL_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           bool_arrays,
                                                                           JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                           JAK_ARCHIVE_ITER_MASK_BOOLEAN,
                                                                           JAK_PROP_ITER_BOOL_ARRAYS)
                case JAK_PROP_ITER_BOOL_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           int8_arrays,
                                                                           JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                           JAK_ARCHIVE_ITER_MASK_INT8,
                                                                           JAK_PROP_ITER_INT8_ARRAYS)
                case JAK_PROP_ITER_INT8_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                           int16_arrays,
                                                                           JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                           JAK_ARCHIVE_ITER_MASK_INT16,
                                                                           JAK_PROP_ITER_INT16_ARRAYS)
                case JAK_PROP_ITER_INT16_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            int32_arrays,
                                                                            JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                            JAK_ARCHIVE_ITER_MASK_INT32,
                                                                            JAK_PROP_ITER_INT32_ARRAYS)
                case JAK_PROP_ITER_INT32_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            int64_arrays,
                                                                            JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                            JAK_ARCHIVE_ITER_MASK_INT64,
                                                                            JAK_PROP_ITER_INT64_ARRAYS)
                case JAK_PROP_ITER_INT64_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            uint8_arrays,
                                                                            JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                            JAK_ARCHIVE_ITER_MASK_UINT8,
                                                                            JAK_PROP_ITER_UINT8_ARRAYS)
                case JAK_PROP_ITER_UINT8_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            uint16_arrays,
                                                                            JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                            JAK_ARCHIVE_ITER_MASK_UINT16,
                                                                            JAK_PROP_ITER_UINT16_ARRAYS)
                case JAK_PROP_ITER_UINT16_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             uint32_arrays,
                                                                             JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                             JAK_ARCHIVE_ITER_MASK_UINT32,
                                                                             JAK_PROP_ITER_UINT32_ARRAYS)
                case JAK_PROP_ITER_UINT32_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             uint64_arrays,
                                                                             JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                             JAK_ARCHIVE_ITER_MASK_UINT64,
                                                                             JAK_PROP_ITER_UINT64_ARRAYS)
                case JAK_PROP_ITER_UINT64_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             float_arrays,
                                                                             JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                             JAK_ARCHIVE_ITER_MASK_NUMBER,
                                                                             JAK_PROP_ITER_FLOAT_ARRAYS)
                case JAK_PROP_ITER_FLOAT_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                            string_arrays,
                                                                            JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                            JAK_ARCHIVE_ITER_MASK_STRING,
                                                                            JAK_PROP_ITER_STRING_ARRAYS)
                case JAK_PROP_ITER_STRING_ARRAYS: SET_STATE_FOR_FALL_THROUGH(iter,
                                                                             object_arrays,
                                                                             JAK_ARCHIVE_ITER_MASK_ARRAYS,
                                                                             JAK_ARCHIVE_ITER_MASK_OBJECT,
                                                                             JAK_PROP_ITER_OBJECT_ARRAYS)
                case JAK_PROP_ITER_OBJECT_ARRAYS:
                        iter->prop_cursor = JAK_PROP_ITER_DONE;
                        break;

                case JAK_PROP_ITER_DONE:
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }

        iter->mode = iter->prop_cursor == JAK_PROP_ITER_OBJECT_ARRAYS ? JAK_PROP_ITER_MODE_COLLECTION
                                                                      : JAK_PROP_ITER_MODE_OBJECT;

        if (iter->prop_cursor != JAK_PROP_ITER_DONE) {
                prop_iter_cursor_init(iter);
        }
        return iter->prop_cursor;
}

#pragma GCC diagnostic pop

static void prop_iter_state_init(jak_prop_iter *iter)
{
        iter->prop_cursor = JAK_PROP_ITER_INIT;
        iter->mode = JAK_PROP_ITER_MODE_OBJECT;
}

static bool archive_prop_iter_from_memblock(jak_prop_iter *iter, jak_error *err, jak_u16 mask,
                                            jak_memblock *memblock, jak_offset_t object_offset)
{
        JAK_ERROR_IF_NULL(iter)
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(memblock)

        iter->mask = mask;
        if (!memfile_open(&iter->record_table_memfile, memblock, READ_ONLY)) {
                JAK_ERROR(err, JAK_ERR_MEMFILEOPEN_FAILED)
                return false;
        }
        if (!memfile_seek(&iter->record_table_memfile, object_offset)) {
                JAK_ERROR(err, JAK_ERR_MEMFILESEEK_FAILED)
                return false;
        }
        if (!init_object_from_memfile(&iter->object, &iter->record_table_memfile)) {
                JAK_ERROR(err, JAK_ERR_INTERNALERR);
                return false;
        }

        jak_error_init(&iter->err);
        prop_iter_state_init(iter);
        prop_iter_state_next(iter);

        return true;
}

bool jak_archive_prop_iter_from_archive(jak_prop_iter *iter, jak_error *err, jak_u16 mask,
                                        jak_archive *archive)
{
        return archive_prop_iter_from_memblock(iter, err, mask, archive->record_table.record_db, 0);
}

bool jak_archive_prop_iter_from_object(jak_prop_iter *iter, jak_u16 mask, jak_error *err,
                                       const jak_archive_object *obj)
{
        return archive_prop_iter_from_memblock(iter, err, mask, obj->memfile.memblock, obj->offset);
}

static enum jak_archive_field_type get_basic_type(jak_prop_iter_state_e state)
{
        switch (state) {
                case JAK_PROP_ITER_NULLS:
                case JAK_PROP_ITER_NULL_ARRAYS:
                        return JAK_FIELD_NULL;
                case JAK_PROP_ITER_BOOLS:
                case JAK_PROP_ITER_BOOL_ARRAYS:
                        return JAK_FIELD_BOOLEAN;
                case JAK_PROP_ITER_INT8S:
                case JAK_PROP_ITER_INT8_ARRAYS:
                        return JAK_FIELD_INT8;
                case JAK_PROP_ITER_INT16S:
                case JAK_PROP_ITER_INT16_ARRAYS:
                        return JAK_FIELD_INT16;
                case JAK_PROP_ITER_INT32S:
                case JAK_PROP_ITER_INT32_ARRAYS:
                        return JAK_FIELD_INT32;
                case JAK_PROP_ITER_INT64S:
                case JAK_PROP_ITER_INT64_ARRAYS:
                        return JAK_FIELD_INT64;
                case JAK_PROP_ITER_UINT8S:
                case JAK_PROP_ITER_UINT8_ARRAYS:
                        return JAK_FIELD_UINT8;
                case JAK_PROP_ITER_UINT16S:
                case JAK_PROP_ITER_UINT16_ARRAYS:
                        return JAK_FIELD_UINT16;
                case JAK_PROP_ITER_UINT32S:
                case JAK_PROP_ITER_UINT32_ARRAYS:
                        return JAK_FIELD_UINT32;
                case JAK_PROP_ITER_UINT64S:
                case JAK_PROP_ITER_UINT64_ARRAYS:
                        return JAK_FIELD_UINT64;
                case JAK_PROP_ITER_FLOATS:
                case JAK_PROP_ITER_FLOAT_ARRAYS:
                        return JAK_FIELD_FLOAT;
                case JAK_PROP_ITER_STRINGS:
                case JAK_PROP_ITER_STRING_ARRAYS:
                        return JAK_FIELD_STRING;
                case JAK_PROP_ITER_OBJECTS:
                case JAK_PROP_ITER_OBJECT_ARRAYS:
                        return JAK_FIELD_OBJECT;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }
}

static bool is_array_type(jak_prop_iter_state_e state)
{
        switch (state) {
                case JAK_PROP_ITER_NULLS:
                case JAK_PROP_ITER_BOOLS:
                case JAK_PROP_ITER_INT8S:
                case JAK_PROP_ITER_INT16S:
                case JAK_PROP_ITER_INT32S:
                case JAK_PROP_ITER_INT64S:
                case JAK_PROP_ITER_UINT8S:
                case JAK_PROP_ITER_UINT16S:
                case JAK_PROP_ITER_UINT32S:
                case JAK_PROP_ITER_UINT64S:
                case JAK_PROP_ITER_FLOATS:
                case JAK_PROP_ITER_STRINGS:
                case JAK_PROP_ITER_OBJECTS:
                        return false;
                case JAK_PROP_ITER_NULL_ARRAYS:
                case JAK_PROP_ITER_BOOL_ARRAYS:
                case JAK_PROP_ITER_INT8_ARRAYS:
                case JAK_PROP_ITER_INT16_ARRAYS:
                case JAK_PROP_ITER_INT32_ARRAYS:
                case JAK_PROP_ITER_INT64_ARRAYS:
                case JAK_PROP_ITER_UINT8_ARRAYS:
                case JAK_PROP_ITER_UINT16_ARRAYS:
                case JAK_PROP_ITER_UINT32_ARRAYS:
                case JAK_PROP_ITER_UINT64_ARRAYS:
                case JAK_PROP_ITER_FLOAT_ARRAYS:
                case JAK_PROP_ITER_STRING_ARRAYS:
                case JAK_PROP_ITER_OBJECT_ARRAYS:
                        return true;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }
}

bool jak_archive_prop_iter_next(jak_prop_iter_mode_e *type, jak_archive_value_vector *value_vector,
                                jak_independent_iter_state *collection_iter, jak_prop_iter *prop_iter)
{
        JAK_ERROR_IF_NULL(type);
        JAK_ERROR_IF_NULL(prop_iter);

        if (prop_iter->prop_cursor != JAK_PROP_ITER_DONE) {
                switch (prop_iter->mode) {
                        case JAK_PROP_ITER_MODE_OBJECT: {
                                value_vector->keys = prop_iter->mode_object.prop_group_header.keys;

                                prop_iter->mode_object.type = get_basic_type(prop_iter->prop_cursor);
                                prop_iter->mode_object.is_array = is_array_type(prop_iter->prop_cursor);

                                value_vector->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;
                                value_vector->prop_type = prop_iter->mode_object.type;
                                value_vector->is_array = prop_iter->mode_object.is_array;

                                if (value_vector
                                    && !jak_archive_value_vector_from_prop_iter(value_vector, &prop_iter->err,
                                                                                prop_iter)) {
                                        JAK_ERROR(&prop_iter->err, JAK_ERR_VITEROPEN_FAILED);
                                        return false;
                                }
                        }
                                break;
                        case JAK_PROP_ITER_MODE_COLLECTION: {
                                collection_iter->state = prop_iter->mode_collection;
                                memfile_open(&collection_iter->record_table_memfile,
                                             prop_iter->record_table_memfile.memblock,
                                             READ_ONLY);
                                jak_error_init(&collection_iter->err);
                        }
                                break;
                        default: JAK_ERROR(&prop_iter->err, JAK_ERR_INTERNALERR);
                                return false;
                }
                *type = prop_iter->mode;
                prop_iter_state_next(prop_iter);
                return true;
        } else {
                return false;
        }
}

const jak_archive_field_sid_t *
jak_archive_collection_iter_get_keys(jak_u32 *num_keys, jak_independent_iter_state *iter)
{
        if (num_keys && iter) {
                *num_keys = iter->state.num_column_groups;
                return iter->state.column_group_keys;
        } else {
                JAK_ERROR(&iter->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_archive_collection_next_column_group(jak_independent_iter_state *group_iter,
                                              jak_independent_iter_state *iter)
{
        JAK_ERROR_IF_NULL(group_iter)
        JAK_ERROR_IF_NULL(iter)

        if (iter->state.current_column_group_idx < iter->state.num_column_groups) {
                collection_iter_read_next_column_group(&iter->state, &iter->record_table_memfile);
                memfile_open(&group_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                group_iter->state = iter->state;
                jak_error_init(&group_iter->err);
                return true;
        } else {
                return false;
        }
}

const jak_uid_t *
jak_archive_column_group_get_object_ids(jak_u32 *num_objects, jak_independent_iter_state *iter)
{
        if (num_objects && iter) {
                *num_objects = iter->state.current_column_group.num_objects;
                return iter->state.current_column_group.object_ids;
        } else {
                JAK_ERROR(&iter->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_archive_column_group_next_column(jak_independent_iter_state *column_iter,
                                          jak_independent_iter_state *iter)
{
        JAK_ERROR_IF_NULL(column_iter)
        JAK_ERROR_IF_NULL(iter)

        if (iter->state.current_column_group.current_column.idx < iter->state.current_column_group.num_columns) {
                prop_iter_read_column(&iter->state, &iter->record_table_memfile);
                memfile_open(&column_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                column_iter->state = iter->state;
                jak_error_init(&column_iter->err);
                return true;
        } else {
                return false;
        }
}

bool jak_archive_column_get_name(jak_archive_field_sid_t *name, enum jak_archive_field_type *type,
                                 jak_independent_iter_state *column_iter)
{
        JAK_ERROR_IF_NULL(column_iter)
        JAK_optional_set(name, column_iter->state.current_column_group.current_column.name)
        JAK_optional_set(type, column_iter->state.current_column_group.current_column.type)
        return true;
}

const jak_u32 *
jak_archive_column_get_entry_positions(jak_u32 *num_entry, jak_independent_iter_state *column_iter)
{
        if (num_entry && column_iter) {
                *num_entry = column_iter->state.current_column_group.current_column.num_elem;
                return column_iter->state.current_column_group.current_column.elem_positions;
        } else {
                JAK_ERROR(&column_iter->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool
jak_archive_column_next_entry(jak_independent_iter_state *entry_iter, jak_independent_iter_state *iter)
{
        JAK_ERROR_IF_NULL(entry_iter)
        JAK_ERROR_IF_NULL(iter)

        if (iter->state.current_column_group.current_column.current_entry.idx
            < iter->state.current_column_group.current_column.num_elem) {
                prop_iter_read_colum_entry(&iter->state, &iter->record_table_memfile);
                memfile_open(&entry_iter->record_table_memfile, iter->record_table_memfile.memblock, READ_ONLY);
                entry_iter->state = iter->state;
                jak_error_init(&entry_iter->err);
                return true;
        } else {
                return false;
        }
}

bool jak_archive_column_entry_get_type(enum jak_archive_field_type *type, jak_independent_iter_state *entry)
{
        JAK_ERROR_IF_NULL(type)
        JAK_ERROR_IF_NULL(entry)
        *type = entry->state.current_column_group.current_column.type;
        return true;
}

#define DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(built_in_type, name, basic_type)                            \
const built_in_type *                                                                                   \
jak_archive_column_entry_get_##name(jak_u32 *array_length, jak_independent_iter_state *entry)              \
{                                                                                                                      \
    if (array_length && entry) {                                                                                       \
        if (entry->state.current_column_group.current_column.type == basic_type)                                       \
        {                                                                                                              \
            *array_length =  entry->state.current_column_group.current_column.current_entry.array_length;              \
            return (const built_in_type *) entry->state.current_column_group.current_column.current_entry.array_base;  \
        } else {                                                                                                       \
            JAK_ERROR(&entry->err, JAK_ERR_TYPEMISMATCH);                                                        \
            return NULL;                                                                                               \
        }                                                                                                              \
    } else {                                                                                                           \
        JAK_ERROR(&entry->err, JAK_ERR_NULLPTR);                                                                 \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i8_t, int8s, JAK_FIELD_INT8);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i16_t, int16s, JAK_FIELD_INT16);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i32_t, int32s, JAK_FIELD_INT32);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_i64_t, int64s, JAK_FIELD_INT64);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u8_t, uint8s, JAK_FIELD_UINT8);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u16_t, uint16s, JAK_FIELD_UINT16);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, uint32s, JAK_FIELD_UINT32);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u64_t, uint64s, JAK_FIELD_UINT64);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_sid_t, strings, JAK_FIELD_STRING);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_number_t, numbers, JAK_FIELD_FLOAT);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_boolean_t, booleans, JAK_FIELD_BOOLEAN);

DECLARE_JAK_ARCHIVE_COLUMN_ENTRY_GET_BASIC_TYPE(jak_archive_field_u32_t, nulls, JAK_FIELD_NULL);

bool jak_archive_column_entry_get_objects(jak_column_object_iter *iter, jak_independent_iter_state *entry)
{
        JAK_ERROR_IF_NULL(iter)
        JAK_ERROR_IF_NULL(entry)

        iter->entry_state = entry->state;
        memfile_open(&iter->memfile, entry->record_table_memfile.memblock, READ_ONLY);
        memfile_seek(&iter->memfile,
                     entry->state.current_column_group.current_column.elem_offsets[
                             entry->state.current_column_group.current_column.current_entry.idx - 1] + sizeof(jak_u32));
        iter->next_obj_off = memfile_tell(&iter->memfile);
        jak_error_init(&iter->err);

        return true;
}

const jak_archive_object *jak_archive_column_entry_object_iter_next_object(jak_column_object_iter *iter)
{
        if (iter) {
                if (iter->next_obj_off != 0) {
                        memfile_seek(&iter->memfile, iter->next_obj_off);
                        if (init_object_from_memfile(&iter->obj, &iter->memfile)) {
                                iter->next_obj_off = iter->obj.next_obj_off;
                                return &iter->obj;
                        } else {
                                JAK_ERROR(&iter->err, JAK_ERR_INTERNALERR);
                                return NULL;
                        }
                } else {
                        return NULL;
                }
        } else {
                JAK_ERROR(&iter->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_archive_object_get_object_id(jak_uid_t *id, const jak_archive_object *object)
{
        JAK_ERROR_IF_NULL(id)
        JAK_ERROR_IF_NULL(object)
        *id = object->object_id;
        return true;
}

bool jak_archive_object_get_prop_iter(jak_prop_iter *iter, const jak_archive_object *object)
{
        // XXXX jak_archive_prop_iter_from_object()
        JAK_UNUSED(iter);
        JAK_UNUSED(object);
        return false;
}

bool jak_archive_value_vector_get_object_id(jak_uid_t *id, const jak_archive_value_vector *iter)
{
        JAK_ERROR_IF_NULL(id)
        JAK_ERROR_IF_NULL(iter)
        *id = iter->object_id;
        return true;
}

const jak_archive_field_sid_t *
jak_archive_value_vector_get_keys(jak_u32 *num_keys, jak_archive_value_vector *iter)
{
        if (num_keys && iter) {
                *num_keys = iter->value_max_idx;
                return iter->keys;
        } else {
                JAK_ERROR(&iter->err, JAK_ERR_NULLPTR)
                return NULL;
        }
}

static void value_vector_init_object_basic(jak_archive_value_vector *value)
{
        value->data.object.offsets =
                JAK_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, jak_offset_t, value->value_max_idx);
}

static void value_vector_init_fixed_length_types_basic(jak_archive_value_vector *value)
{
        JAK_ASSERT(!value->is_array);

        switch (value->prop_type) {
                case JAK_FIELD_INT8:
                        value->data.basic.values.int8s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                          jak_archive_field_i8_t);
                        break;
                case JAK_FIELD_INT16:
                        value->data.basic.values.int16s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                           jak_archive_field_i16_t);
                        break;
                case JAK_FIELD_INT32:
                        value->data.basic.values.int32s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                           jak_archive_field_i32_t);
                        break;
                case JAK_FIELD_INT64:
                        value->data.basic.values.int64s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                           jak_archive_field_i64_t);
                        break;
                case JAK_FIELD_UINT8:
                        value->data.basic.values.uint8s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                           jak_archive_field_u8_t);
                        break;
                case JAK_FIELD_UINT16:
                        value->data.basic.values.uint16s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                            jak_archive_field_u16_t);
                        break;
                case JAK_FIELD_UINT32:
                        value->data.basic.values.uint32s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                            jak_archive_field_u32_t);
                        break;
                case JAK_FIELD_UINT64:
                        value->data.basic.values.uint64s = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                            jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_FLOAT:
                        value->data.basic.values.numbers = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                            jak_archive_field_number_t);
                        break;
                case JAK_FIELD_STRING:
                        value->data.basic.values.strings = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                            jak_archive_field_sid_t);
                        break;
                case JAK_FIELD_BOOLEAN:
                        value->data.basic.values.booleans = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                             jak_archive_field_boolean_t);
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }
}

static void value_vector_init_fixed_length_types_null_arrays(jak_archive_value_vector *value)
{
        JAK_ASSERT(value->is_array);
        JAK_ASSERT(value->prop_type == JAK_FIELD_NULL);
        value->data.arrays.meta.num_nulls_contained =
                JAK_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, jak_u32, value->value_max_idx);
}

static void value_vector_init_fixed_length_types_non_null_arrays(jak_archive_value_vector *value)
{
        JAK_ASSERT (value->is_array);

        value->data.arrays.meta.array_lengths =
                JAK_MEMFILE_READ_TYPE_LIST(&value->record_table_memfile, jak_u32, value->value_max_idx);

        switch (value->prop_type) {
                case JAK_FIELD_INT8:
                        value->data.arrays.values.int8s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                jak_archive_field_i8_t);
                        break;
                case JAK_FIELD_INT16:
                        value->data.arrays.values.int16s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 jak_archive_field_i16_t);
                        break;
                case JAK_FIELD_INT32:
                        value->data.arrays.values.int32s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 jak_archive_field_i32_t);
                        break;
                case JAK_FIELD_INT64:
                        value->data.arrays.values.int64s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 jak_archive_field_i64_t);
                        break;
                case JAK_FIELD_UINT8:
                        value->data.arrays.values.uint8s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                 jak_archive_field_u8_t);
                        break;
                case JAK_FIELD_UINT16:
                        value->data.arrays.values.uint16s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  jak_archive_field_u16_t);
                        break;
                case JAK_FIELD_UINT32:
                        value->data.arrays.values.uint32s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  jak_archive_field_u32_t);
                        break;
                case JAK_FIELD_UINT64:
                        value->data.arrays.values.uint64s_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_FLOAT:
                        value->data.arrays.values.numbers_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  jak_archive_field_number_t);
                        break;
                case JAK_FIELD_STRING:
                        value->data.arrays.values.strings_base = JAK_MEMFILE_PEEK(&value->record_table_memfile,
                                                                                  jak_archive_field_sid_t);
                        break;
                case JAK_FIELD_BOOLEAN:
                        value->data.arrays.values.booleans_base =
                                JAK_MEMFILE_PEEK(&value->record_table_memfile, jak_archive_field_boolean_t);
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }
}

static void value_vector_init_fixed_length_types(jak_archive_value_vector *value)
{
        if (value->is_array) {
                value_vector_init_fixed_length_types_non_null_arrays(value);
        } else {
                value_vector_init_fixed_length_types_basic(value);
        }
}

static void value_vector_init_object(jak_archive_value_vector *value)
{
        if (value->is_array) {
                //value_vector_init_object_array(value);
        } else {
                value_vector_init_object_basic(value);
        }
}

bool jak_archive_value_vector_from_prop_iter(jak_archive_value_vector *value, jak_error *err,
                                             jak_prop_iter *prop_iter)
{
        JAK_ERROR_IF_NULL(value);
        JAK_ERROR_IF_NULL(prop_iter);

        JAK_ERROR_IF_AND_RETURN (prop_iter->mode != JAK_PROP_ITER_MODE_OBJECT,
                             &prop_iter->err,
                             JAK_ERR_ITER_OBJECT_NEEDED,
                             false)

        jak_error_init(&value->err);

        value->prop_iter = prop_iter;
        value->data_off = prop_iter->mode_object.prop_data_off;
        value->object_id = prop_iter->object.object_id;

        if (!memfile_open(&value->record_table_memfile, prop_iter->record_table_memfile.memblock, READ_ONLY)) {
                JAK_ERROR(err, JAK_ERR_MEMFILEOPEN_FAILED);
                return false;
        }
        if (!memfile_skip(&value->record_table_memfile, value->data_off)) {
                JAK_ERROR(err, JAK_ERR_MEMFILESKIP_FAILED);
                return false;
        }

        value->prop_type = prop_iter->mode_object.type;
        value->is_array = prop_iter->mode_object.is_array;
        value->value_max_idx = prop_iter->mode_object.prop_group_header.header->num_entries;

        switch (value->prop_type) {
                case JAK_FIELD_OBJECT:
                        value_vector_init_object(value);
                        break;
                case JAK_FIELD_NULL:
                        if (value->is_array) {
                                value_vector_init_fixed_length_types_null_arrays(value);
                        }
                        break;
                case JAK_FIELD_INT8:
                case JAK_FIELD_INT16:
                case JAK_FIELD_INT32:
                case JAK_FIELD_INT64:
                case JAK_FIELD_UINT8:
                case JAK_FIELD_UINT16:
                case JAK_FIELD_UINT32:
                case JAK_FIELD_UINT64:
                case JAK_FIELD_FLOAT:
                case JAK_FIELD_STRING:
                case JAK_FIELD_BOOLEAN:
                        value_vector_init_fixed_length_types(value);
                        break;
                default: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_INTERNALERR);
        }

        return true;
}

bool jak_archive_value_vector_get_basic_type(enum jak_archive_field_type *type,
                                                 const jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(type)
        JAK_ERROR_IF_NULL(value)
        *type = value->prop_type;
        return true;
}

bool jak_archive_value_vector_is_array_type(bool *is_array, const jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(is_array)
        JAK_ERROR_IF_NULL(value)
        *is_array = value->is_array;
        return true;
}

bool jak_archive_value_vector_get_length(jak_u32 *length, const jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(length)
        JAK_ERROR_IF_NULL(value)
        *length = value->value_max_idx;
        return true;
}

bool jak_archive_value_vector_is_of_objects(bool *is_object, jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(is_object)
        JAK_ERROR_IF_NULL(value)

        *is_object = value->prop_type == JAK_FIELD_OBJECT && !value->is_array;

        return true;
}

bool jak_archive_value_vector_get_object_at(jak_archive_object *object, jak_u32 idx,
                                                jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(object)
        JAK_ERROR_IF_NULL(value)

        if (idx >= value->value_max_idx) {
                JAK_ERROR(&value->err, JAK_ERR_OUTOFBOUNDS);
                return false;
        }

        bool is_object;

        jak_archive_value_vector_is_of_objects(&is_object, value);

        if (is_object) {
                memfile_seek(&value->record_table_memfile, value->data.object.offsets[idx]);
                init_object_from_memfile(&value->data.object.object, &value->record_table_memfile);
                *object = value->data.object.object;
                return true;
        } else {
                JAK_ERROR(&value->err, JAK_ERR_ITER_NOOBJ);
                return false;
        }
}

#define DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(name, basic_type)                                            \
bool                                                                                                    \
jak_archive_value_vector_is_##name(bool *type_match, jak_archive_value_vector *value)                          \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(type_match)                                                                               \
    JAK_ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    *type_match = value->prop_type == basic_type;                                                                      \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int8, JAK_FIELD_INT8)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int16, JAK_FIELD_INT16)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int32, JAK_FIELD_INT32)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(int64, JAK_FIELD_INT64)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint8, JAK_FIELD_UINT8)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint16, JAK_FIELD_UINT16)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint32, JAK_FIELD_UINT32)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(uint64, JAK_FIELD_UINT64)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(string, JAK_FIELD_STRING)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(number, JAK_FIELD_FLOAT)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(boolean, JAK_FIELD_BOOLEAN)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_IS_BASIC_TYPE(null, JAK_FIELD_NULL)

#define DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(names, name, built_in_type, err_code)                       \
const built_in_type *                                                                                   \
jak_archive_value_vector_get_##names(jak_u32 *num_values, jak_archive_value_vector *value)                    \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (jak_archive_value_vector_is_array_type(&is_array, value) &&                                                 \
        jak_archive_value_vector_is_##name(&type_match, value) && !is_array)                                        \
    {                                                                                                                  \
        JAK_optional_set(num_values, value->value_max_idx)                                                          \
        return value->data.basic.values.names;                                                                         \
    } else                                                                                                             \
    {                                                                                                                  \
        JAK_ERROR(&value->err, err_code);                                                                           \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int8s, int8, jak_archive_field_i8_t, JAK_ERR_ITER_NOINT8)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int16s, int16, jak_archive_field_i16_t, JAK_ERR_ITER_NOINT16)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int32s, int32, jak_archive_field_i32_t, JAK_ERR_ITER_NOINT32)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(int64s, int64, jak_archive_field_i64_t, JAK_ERR_ITER_NOINT64)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint8s, uint8, jak_archive_field_u8_t, JAK_ERR_ITER_NOUINT8)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint16s, uint16, jak_archive_field_u16_t, JAK_ERR_ITER_NOUINT16)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint32s, uint32, jak_archive_field_u32_t, JAK_ERR_ITER_NOUINT32)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(uint64s, uint64, jak_archive_field_u64_t, JAK_ERR_ITER_NOUINT64)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(strings, string, jak_archive_field_sid_t, JAK_ERR_ITER_NOSTRING)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(numbers, number, jak_archive_field_number_t, JAK_ERR_ITER_NONUMBER)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_BASIC_TYPE(booleans, boolean, jak_archive_field_boolean_t, JAK_ERR_ITER_NOBOOL)

const jak_archive_field_u32_t *
jak_archive_value_vector_get_null_arrays(jak_u32 *num_values, jak_archive_value_vector *value)
{
        JAK_ERROR_IF_NULL(value)

        bool is_array;
        bool type_match;

        if (jak_archive_value_vector_is_array_type(&is_array, value) &&
            jak_archive_value_vector_is_null(&type_match, value)
            && is_array) {
                JAK_optional_set(num_values, value->value_max_idx);
                return value->data.arrays.meta.num_nulls_contained;
        } else {
                return NULL;
        }
}

#define DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(name, built_in_type, base)                               \
const built_in_type *                                                                                   \
jak_archive_value_vector_get_##name##_arrays_at(jak_u32 *array_length, jak_u32 idx,                               \
                                               jak_archive_value_vector *value)                                   \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(value)                                                                                    \
                                                                                                                       \
    bool is_array;                                                                                                     \
    bool type_match;                                                                                                   \
                                                                                                                       \
    if (idx < value->value_max_idx && jak_archive_value_vector_is_array_type(&is_array, value) &&                   \
        jak_archive_value_vector_is_##name(&type_match, value) && is_array)                                         \
    {                                                                                                                  \
        jak_u32 skip_length = 0;                                                                                      \
        for (jak_u32 i = 0; i < idx; i++) {                                                                           \
            skip_length += value->data.arrays.meta.array_lengths[i];                                                   \
        }                                                                                                              \
        *array_length = value->data.arrays.meta.array_lengths[idx];                                                    \
        return value->data.arrays.values.base + skip_length;                                                           \
    } else                                                                                                             \
    {                                                                                                                  \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int8, jak_archive_field_i8_t, int8s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int16, jak_archive_field_i16_t, int16s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int32, jak_archive_field_i32_t, int32s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(int64, jak_archive_field_i64_t, int64s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint8, jak_archive_field_u8_t, uint8s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint16, jak_archive_field_u16_t, uint16s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint32, jak_archive_field_u32_t, uint32s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(uint64, jak_archive_field_u64_t, uint64s_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(string, jak_archive_field_sid_t, strings_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(number, jak_archive_field_number_t, numbers_base)

DECLARE_JAK_ARCHIVE_VALUE_VECTOR_GET_ARRAY_TYPE_AT(boolean, jak_archive_field_boolean_t, booleans_base)

void jak_archive_int_reset_carbon_object_mem_file(jak_archive_object *object)
{
        JAK_UNUSED(object);
        //  memfile_seek(&object->file, object->self);
        abort();
}
