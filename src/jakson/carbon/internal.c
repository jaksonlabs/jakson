/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jakson/std/uintvar/stream.h>
#include <jakson/carbon.h>
#include <jakson/carbon/insert.h>
#include <jakson/carbon/mime.h>
#include <jakson/carbon/internal.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/key.h>
#include <jakson/carbon/commit.h>
#include <jakson/json/parser.h>
#include <jakson/carbon/object_it.h>

static void marker_insert(memfile *memfile, u8 marker);

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it);

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, carbon_object_it *it);

static bool is_slot_occupied(bool *is_empty_slot, bool *is_array_end, memfile *file, u8 end_marker);

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it);

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, carbon_object_it *it);

static void int_carbon_from_json_elem(carbon_insert *ins, const json_element *elem, bool is_root);

static void int_insert_prop_object(carbon_insert *oins, json_object *obj);

static void
insert_embedded_container(memfile *memfile, u8 begin_marker, u8 end_marker, u8 capacity)
{
        memfile_ensure_space(memfile, sizeof(u8));
        marker_insert(memfile, begin_marker);

        memfile_ensure_space(memfile, capacity + sizeof(u8));

        offset_t payload_begin = memfile_tell(memfile);
        memfile_seek(memfile, payload_begin + capacity);

        marker_insert(memfile, end_marker);

        /** seek to first entry in container */
        memfile_seek(memfile, payload_begin);
}

bool carbon_int_insert_object(memfile *memfile, carbon_map_derivable_e derivation, size_t nbytes)
{
        ERROR_IF_NULL(memfile);
        assert(derivation == CARBON_MAP_UNSORTED_MULTIMAP || derivation == CARBON_MAP_SORTED_MULTIMAP ||
               derivation == CARBON_MAP_UNSORTED_MAP || derivation == CARBON_MAP_SORTED_MAP);
        carbon_derived_e begin_marker;
        carbon_abstract_derive_map_to(&begin_marker, derivation);
        insert_embedded_container(memfile, begin_marker, CARBON_MOBJECT_END, nbytes);
        return true;
}

bool carbon_int_insert_array(memfile *memfile, carbon_list_derivable_e derivation, size_t nbytes)
{
        ERROR_IF_NULL(memfile);
        assert(derivation == CARBON_LIST_UNSORTED_MULTISET || derivation == CARBON_LIST_SORTED_MULTISET ||
               derivation == CARBON_LIST_UNSORTED_SET || derivation == CARBON_LIST_SORTED_SET);
        carbon_derived_e begin_marker;
        carbon_abstract_derive_list_to(&begin_marker, CARBON_LIST_CONTAINER_ARRAY, derivation);
        insert_embedded_container(memfile, begin_marker, CARBON_MARRAY_END, nbytes);
        return true;
}

bool carbon_int_insert_column(memfile *memfile_in, err *err_in, carbon_list_derivable_e derivation, carbon_column_type_e type,
                              size_t capactity)
{
        ERROR_IF_NULL(memfile_in);
        ERROR_IF_NULL(err_in);
        assert(derivation == CARBON_LIST_UNSORTED_MULTISET || derivation == CARBON_LIST_SORTED_MULTISET ||
               derivation == CARBON_LIST_UNSORTED_SET || derivation == CARBON_LIST_SORTED_SET);

        carbon_field_type_e column_type = carbon_field_type_for_column(derivation, type);

        memfile_ensure_space(memfile_in, sizeof(u8));
        marker_insert(memfile_in, column_type);

        u32 num_elements = 0;
        u32 cap_elements = capactity;

        memfile_write_uintvar_stream(NULL, memfile_in, num_elements);
        memfile_write_uintvar_stream(NULL, memfile_in, cap_elements);

        offset_t payload_begin = memfile_tell(memfile_in);

        size_t type_size = carbon_int_get_type_value_size(column_type);

        size_t nbytes = capactity * type_size;
        memfile_ensure_space(memfile_in, nbytes + sizeof(u8) + 2 * sizeof(u32));

        /** seek to first entry in column */
        memfile_seek(memfile_in, payload_begin);

        return true;
}

size_t carbon_int_get_type_size_encoded(carbon_field_type_e type)
{
        size_t type_size = sizeof(media_type); /** at least the media type marker is required */
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /** only media type marker is required */
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                        type_size += sizeof(u8);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                        type_size += sizeof(u16);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                        type_size += sizeof(u32);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                        type_size += sizeof(u64);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        type_size += sizeof(float);
                        break;
                default: ERROR_PRINT(ERR_INTERNALERR);
                        return 0;
        }
        return type_size;
}

size_t carbon_int_get_type_value_size(carbon_field_type_e type)
{
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        return sizeof(media_type); /** these constant values are determined by their media type markers */
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        return sizeof(u8);
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        return sizeof(u16);
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        return sizeof(u32);
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        return sizeof(u64);
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        return sizeof(float);
                default: ERROR_PRINT(ERR_INTERNALERR);
                        return 0;
        }
}

bool carbon_int_array_it_next(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it)
{
        if (carbon_int_array_it_refresh(is_empty_slot, is_array_end, it)) {
                carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_next(bool *is_empty_slot, bool *is_object_end, carbon_object_it *it)
{
        if (carbon_int_object_it_refresh(is_empty_slot, is_object_end, it)) {
                carbon_int_object_it_prop_value_skip(it);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        if (object_it_is_slot_occupied(is_empty_slot, is_object_end, it)) {
                carbon_int_object_it_prop_key_access(it);
                carbon_int_field_data_access(&it->memfile, &it->err, &it->field.value.data);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_prop_key_access(carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        //memfile_skip(&it->mem, sizeof(media_type));

        it->field.key.offset = memfile_tell(&it->memfile);
        it->field.key.name_len = memfile_read_uintvar_stream(NULL, &it->memfile);
        it->field.key.name = memfile_peek(&it->memfile, it->field.key.name_len);
        memfile_skip(&it->memfile, it->field.key.name_len);
        it->field.value.offset = memfile_tell(&it->memfile);
        it->field.value.data.it_field_type = *MEMFILE_PEEK(&it->memfile, u8);

        return true;
}

bool carbon_int_object_it_prop_value_skip(carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        memfile_seek(&it->memfile, it->field.value.offset);
        return carbon_field_skip(&it->memfile);
}

bool carbon_int_object_it_prop_skip(carbon_object_it *it)
{
        ERROR_IF_NULL(it)

        it->field.key.name_len = memfile_read_uintvar_stream(NULL, &it->memfile);
        memfile_skip(&it->memfile, it->field.key.name_len);

        return carbon_field_skip(&it->memfile);
}

bool carbon_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, carbon_object_it *it)
{
        while (object_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool carbon_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it)
{
        while (array_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool carbon_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it)
{
        ERROR_IF_NULL(it);
        carbon_int_field_access_drop(&it->field_access);
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_array_it_field_type_read(it);
                carbon_int_field_data_access(&it->memfile, &it->err, &it->field_access);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_array_it_field_type_read(carbon_array_it *it)
{
        ERROR_IF_NULL(it)
        ERROR_IF(memfile_remain_size(&it->memfile) < 1, &it->err, ERR_ILLEGALOP);
        memfile_save_position(&it->memfile);
        it->field_offset = memfile_tell(&it->memfile);
        u8 media_type = *memfile_read(&it->memfile, 1);
        ERROR_IF(media_type == 0, &it->err, ERR_NOTFOUND)
        ERROR_IF(media_type == CARBON_MARRAY_END, &it->err, ERR_OUTOFBOUNDS)
        it->field_access.it_field_type = media_type;
        memfile_restore_position(&it->memfile);
        return true;
}

bool carbon_int_field_data_access(memfile *file, err *err, field_access *field_access)
{
        memfile_save_position(file);
        memfile_skip(file, sizeof(media_type));

        switch (field_access->it_field_type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                        break;
                case CARBON_FIELD_STRING: {
                        u8 nbytes;
                        uintvar_stream_t len = (uintvar_stream_t) memfile_peek(file, 1);
                        field_access->it_field_len = uintvar_stream_read(&nbytes, len);

                        memfile_skip(file, nbytes);
                }
                        break;
                case CARBON_FIELD_BINARY: {
                        /** read mime type with variable-length integer type */
                        u64 mime_type_id = memfile_read_uintvar_stream(NULL, file);

                        field_access->it_mime_type = carbon_media_mime_type_by_id(mime_type_id);
                        field_access->it_mime_type_strlen = strlen(field_access->it_mime_type);

                        /** read blob length */
                        field_access->it_field_len = memfile_read_uintvar_stream(NULL, file);

                        /** the mem points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_BINARY_CUSTOM: {
                        /** read mime type string_buffer */
                        field_access->it_mime_type_strlen = memfile_read_uintvar_stream(NULL, file);
                        field_access->it_mime_type = memfile_read(file, field_access->it_mime_type_strlen);

                        /** read blob length */
                        field_access->it_field_len = memfile_read_uintvar_stream(NULL, file);

                        /** the mem points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        carbon_int_field_access_create(field_access);
                        field_access->nested_array_it_is_created = true;
                        carbon_array_it_create(field_access->nested_array_it, file, err,
                                               memfile_tell(file) - sizeof(u8));
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        carbon_int_field_access_create(field_access);
                        field_access->nested_column_it_is_created = true;
                        carbon_column_it_create(field_access->nested_column_it, file, err,
                                                memfile_tell(file) - sizeof(u8));
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        carbon_int_field_access_create(field_access);
                        field_access->nested_object_it_is_created = true;
                        carbon_object_it_create(field_access->nested_object_it, file, err,
                                                memfile_tell(file) - sizeof(u8));
                        break;
                default: ERROR(err, ERR_CORRUPTED)
                        return false;
        }

        field_access->it_field_data = memfile_peek(file, 1);
        memfile_restore_position(file);
        return true;
}

offset_t carbon_int_column_get_payload_off(carbon_column_it *it)
{
        memfile_save_position(&it->memfile);
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        memfile_skip_uintvar_stream(&it->memfile); // skip num of elements
        memfile_skip_uintvar_stream(&it->memfile); // skip capacity of elements
        offset_t result = memfile_tell(&it->memfile);
        memfile_restore_position(&it->memfile);
        return result;
}

offset_t carbon_int_payload_after_header(carbon *doc)
{
        offset_t result = 0;
        carbon_key_e key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        if (LIKELY(carbon_key_skip(&key_type, &doc->memfile))) {
                if (key_type != CARBON_KEY_NOKEY) {
                        carbon_commit_hash_skip(&doc->memfile);
                }
                result = memfile_tell(&doc->memfile);
        }

        memfile_restore_position(&doc->memfile);

        return result;
}

u64 carbon_int_header_get_commit_hash(carbon *doc)
{
        JAK_ASSERT(doc);
        u64 rev = 0;
        carbon_key_e key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_skip(&key_type, &doc->memfile);
        if (key_type != CARBON_KEY_NOKEY) {
                carbon_commit_hash_read(&rev, &doc->memfile);
        }

        memfile_restore_position(&doc->memfile);
        return rev;
}

void carbon_int_history_push(vector ofType(offset_t) *vec, offset_t off)
{
        JAK_ASSERT(vec);
        vector_push(vec, &off, 1);
}

void carbon_int_history_clear(vector ofType(offset_t) *vec)
{
        JAK_ASSERT(vec);
        vector_clear(vec);
}

offset_t carbon_int_history_pop(vector ofType(offset_t) *vec)
{
        JAK_ASSERT(vec);
        JAK_ASSERT(carbon_int_history_has(vec));
        return *(offset_t *) vector_pop(vec);
}

offset_t carbon_int_history_peek(vector ofType(offset_t) *vec)
{
        JAK_ASSERT(vec);
        JAK_ASSERT(carbon_int_history_has(vec));
        return *(offset_t *) vector_peek(vec);
}

bool carbon_int_history_has(vector ofType(offset_t) *vec)
{
        JAK_ASSERT(vec);
        return !vector_is_empty(vec);
}

bool carbon_int_field_access_create(field_access *field)
{
        field->nested_array_it_is_created = false;
        field->nested_array_it_accessed = false;
        field->nested_object_it_is_created = false;
        field->nested_object_it_accessed = false;
        field->nested_column_it_is_created = false;
        field->nested_array_it = MALLOC(sizeof(carbon_array_it));
        field->nested_object_it = MALLOC(sizeof(carbon_object_it));
        field->nested_column_it = MALLOC(sizeof(carbon_column_it));
        return true;
}

bool carbon_int_field_access_clone(field_access *dst, field_access *src)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)

        dst->it_field_type = src->it_field_type;
        dst->it_field_data = src->it_field_data;
        dst->it_field_len = src->it_field_len;
        dst->it_mime_type = src->it_mime_type;
        dst->it_mime_type_strlen = src->it_mime_type_strlen;
        dst->nested_array_it_is_created = src->nested_array_it_is_created;
        dst->nested_array_it_accessed = src->nested_array_it_accessed;
        dst->nested_object_it_is_created = src->nested_object_it_is_created;
        dst->nested_object_it_accessed = src->nested_object_it_accessed;
        dst->nested_column_it_is_created = src->nested_column_it_is_created;
        dst->nested_array_it = MALLOC(sizeof(carbon_array_it));
        dst->nested_object_it = MALLOC(sizeof(carbon_object_it));
        dst->nested_column_it = MALLOC(sizeof(carbon_column_it));

        if (carbon_int_field_access_object_it_opened(src)) {
                carbon_object_it_clone(dst->nested_object_it, src->nested_object_it);
        } else if (carbon_int_field_access_column_it_opened(src)) {
                carbon_column_it_clone(dst->nested_column_it, src->nested_column_it);
        } else if (carbon_int_field_access_array_it_opened(src)) {
                carbon_array_it_clone(dst->nested_array_it, src->nested_array_it);
        }
        return true;
}

bool carbon_int_field_access_drop(field_access *field)
{
        carbon_int_field_auto_close(field);
        free(field->nested_array_it);
        free(field->nested_object_it);
        free(field->nested_column_it);
        field->nested_array_it = NULL;
        field->nested_object_it = NULL;
        field->nested_column_it = NULL;
        return true;
}

bool carbon_int_field_access_object_it_opened(field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_object_it_is_created && field->nested_object_it != NULL;
}

bool carbon_int_field_access_array_it_opened(field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_array_it_is_created && field->nested_array_it != NULL;
}

bool carbon_int_field_access_column_it_opened(field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_column_it_is_created && field->nested_column_it != NULL;
}

void carbon_int_auto_close_nested_array_it(field_access *field)
{
        if (carbon_int_field_access_array_it_opened(field)) {
                carbon_array_it_drop(field->nested_array_it);
                ZERO_MEMORY(field->nested_array_it, sizeof(carbon_array_it));
        }
}

void carbon_int_auto_close_nested_object_it(field_access *field)
{
        if (carbon_int_field_access_object_it_opened(field)) {
                carbon_object_it_drop(field->nested_object_it);
                ZERO_MEMORY(field->nested_object_it, sizeof(carbon_object_it));
        }
}

void carbon_int_auto_close_nested_column_it(field_access *field)
{
        if (carbon_int_field_access_column_it_opened(field)) {
                ZERO_MEMORY(field->nested_column_it, sizeof(carbon_column_it));
        }
}

bool carbon_int_field_auto_close(field_access *field)
{
        ERROR_IF_NULL(field)
        if (field->nested_array_it_is_created && !field->nested_array_it_accessed) {
                carbon_int_auto_close_nested_array_it(field);
                field->nested_array_it_is_created = false;
                field->nested_array_it_accessed = false;
        }
        if (field->nested_object_it_is_created && !field->nested_object_it_accessed) {
                carbon_int_auto_close_nested_object_it(field);
                field->nested_object_it_is_created = false;
                field->nested_object_it_accessed = false;
        }
        if (field->nested_column_it_is_created) {
                carbon_int_auto_close_nested_column_it(field);
                field->nested_object_it_is_created = false;
        }

        return true;
}

bool carbon_int_field_access_field_type(carbon_field_type_e *type, field_access *field)
{
        ERROR_IF_NULL(type)
        ERROR_IF_NULL(field)
        *type = field->it_field_type;
        return true;
}

bool carbon_int_field_access_u8_value(u8 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U8, err, ERR_TYPEMISMATCH);
        *value = *(u8 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u16_value(u16 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U16, err, ERR_TYPEMISMATCH);
        *value = *(u16 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u32_value(u32 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U32, err, ERR_TYPEMISMATCH);
        *value = *(u32 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u64_value(u64 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U64, err, ERR_TYPEMISMATCH);
        *value = *(u64 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i8_value(i8 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I8, err, ERR_TYPEMISMATCH);
        *value = *(i8 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i16_value(i16 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I16, err, ERR_TYPEMISMATCH);
        *value = *(i16 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i32_value(i32 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I32, err, ERR_TYPEMISMATCH);
        *value = *(i32 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i64_value(i64 *value, field_access *field, err *err)
{
        ERROR_IF_NULL(value)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I64, err, ERR_TYPEMISMATCH);
        *value = *(i64 *) field->it_field_data;
        return true;
}

bool
carbon_int_field_access_float_value(bool *is_null_in, float *value, field_access *field, err *err)
{
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_FLOAT, err, ERR_TYPEMISMATCH);
        float read_value = *(float *) field->it_field_data;
        OPTIONAL_SET(value, read_value);
        OPTIONAL_SET(is_null_in, IS_NULL_FLOAT(read_value));

        return true;
}

bool carbon_int_field_access_signed_value(bool *is_null_in, i64 *value, field_access *field,
                                          err *err)
{
        ERROR_IF_NULL(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_NUMBER_I8: {
                        i8 read_value;
                        carbon_int_field_access_i8_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_I8(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I16: {
                        i16 read_value;
                        carbon_int_field_access_i16_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_I16(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I32: {
                        i32 read_value;
                        carbon_int_field_access_i32_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_I32(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I64: {
                        i64 read_value;
                        carbon_int_field_access_i64_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_I64(read_value));
                }
                        break;
                default: ERROR(err, ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

bool carbon_int_field_access_unsigned_value(bool *is_null_in, u64 *value, field_access *field,
                                            err *err)
{
        ERROR_IF_NULL(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_NUMBER_U8: {
                        u8 read_value;
                        carbon_int_field_access_u8_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_U8(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U16: {
                        u16 read_value;
                        carbon_int_field_access_u16_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_U16(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U32: {
                        u32 read_value;
                        carbon_int_field_access_u32_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_U32(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U64: {
                        u64 read_value;
                        carbon_int_field_access_u64_value(&read_value, field, err);
                        OPTIONAL_SET(value, read_value);
                        OPTIONAL_SET(is_null_in, IS_NULL_U64(read_value));
                }
                        break;
                default: ERROR(err, ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

const char *carbon_int_field_access_string_value(u64 *strlen, field_access *field, err *err)
{
        ERROR_IF_NULL(strlen);
        ERROR_IF_AND_RETURN(field == NULL, err, ERR_NULLPTR, NULL);
        ERROR_IF(field->it_field_type != CARBON_FIELD_STRING, err, ERR_TYPEMISMATCH);
        *strlen = field->it_field_len;
        return field->it_field_data;
}

bool
carbon_int_field_access_binary_value(carbon_binary *out, field_access *field, err *err)
{
        ERROR_IF_NULL(out)
        ERROR_IF_NULL(field)
        ERROR_IF(field->it_field_type != CARBON_FIELD_BINARY &&
                 field->it_field_type != CARBON_FIELD_BINARY_CUSTOM,
                 err, ERR_TYPEMISMATCH);
        out->blob = field->it_field_data;
        out->blob_len = field->it_field_len;
        out->mime_type = field->it_mime_type;
        out->mime_type_strlen = field->it_mime_type_strlen;
        return true;
}

carbon_array_it *carbon_int_field_access_array_value(field_access *field, err *err)
{
        ERROR_PRINT_IF(!field, ERR_NULLPTR);
        ERROR_IF(!carbon_field_type_is_array_or_subtype(field->it_field_type), err, ERR_TYPEMISMATCH);
        field->nested_array_it_accessed = true;
        return field->nested_array_it;
}

carbon_object_it *carbon_int_field_access_object_value(field_access *field, err *err)
{
        ERROR_PRINT_IF(!field, ERR_NULLPTR);
        ERROR_IF(!carbon_field_type_is_object_or_subtype(field->it_field_type), err, ERR_TYPEMISMATCH);
        field->nested_object_it_accessed = true;
        return field->nested_object_it;
}

carbon_column_it *carbon_int_field_access_column_value(field_access *field, err *err)
{
        ERROR_PRINT_IF(!field, ERR_NULLPTR);
        ERROR_IF(!carbon_field_type_is_column_or_subtype(field->it_field_type), err, ERR_TYPEMISMATCH);
        return field->nested_column_it;
}

bool carbon_int_field_remove(memfile *memfile, err *err, carbon_field_type_e type)
{
        JAK_ASSERT((carbon_field_type_e) *memfile_peek(memfile, sizeof(u8)) == type);
        offset_t start_off = memfile_tell(memfile);
        memfile_skip(memfile, sizeof(u8));
        size_t rm_nbytes = sizeof(u8); /** at least the type marker must be removed */
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /** nothing to do */
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                        rm_nbytes += sizeof(u8);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                        rm_nbytes += sizeof(u16);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                        rm_nbytes += sizeof(u32);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                        rm_nbytes += sizeof(u64);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        rm_nbytes += sizeof(float);
                        break;
                case CARBON_FIELD_STRING: {
                        u8 len_nbytes;  /** number of bytes used to store string_buffer length */
                        u64 str_len; /** the number of characters of the string_buffer field */

                        str_len = memfile_read_uintvar_stream(&len_nbytes, memfile);

                        rm_nbytes += len_nbytes + str_len;
                }
                        break;
                case CARBON_FIELD_BINARY: {
                        u8 mime_type_nbytes; /** number of bytes for mime type */
                        u8 blob_length_nbytes; /** number of bytes to store blob length */
                        u64 blob_nbytes; /** number of bytes to store actual blob data */

                        /** get bytes used for mime type id */
                        memfile_read_uintvar_stream(&mime_type_nbytes, memfile);

                        /** get bytes used for blob length info */
                        blob_nbytes = memfile_read_uintvar_stream(&blob_length_nbytes, memfile);

                        rm_nbytes += mime_type_nbytes + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_BINARY_CUSTOM: {
                        u8 custom_type_strlen_nbytes; /** number of bytes for type name string_buffer length info */
                        u8 custom_type_strlen; /** number of characters to encode type name string_buffer */
                        u8 blob_length_nbytes; /** number of bytes to store blob length */
                        u64 blob_nbytes; /** number of bytes to store actual blob data */

                        /** get bytes for custom type string_buffer len, and the actual length */
                        custom_type_strlen = memfile_read_uintvar_stream(&custom_type_strlen_nbytes, memfile);
                        memfile_skip(memfile, custom_type_strlen);

                        /** get bytes used for blob length info */
                        blob_nbytes = memfile_read_uintvar_stream(&blob_length_nbytes, memfile);

                        rm_nbytes += custom_type_strlen_nbytes + custom_type_strlen + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                        carbon_array_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_array_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_array_it_fast_forward(&it);
                        offset_t end_off = carbon_array_it_memfilepos(&it);
                        carbon_array_it_drop(&it);

                        JAK_ASSERT(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        carbon_column_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_column_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_column_it_fast_forward(&it);
                        offset_t end_off = carbon_column_it_memfilepos(&it);

                        JAK_ASSERT(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                        carbon_object_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_object_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_object_it_fast_forward(&it);
                        offset_t end_off = carbon_object_it_memfile_pos(&it);
                        carbon_object_it_drop(&it);

                        JAK_ASSERT(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                default: ERROR(err, ERR_INTERNALERR)
                        return false;
        }
        memfile_seek(memfile, start_off);
        memfile_inplace_remove(memfile, rm_nbytes);

        return true;
}

static void int_insert_array_array(carbon_insert *array_ins, json_array *array)
{
        carbon_insert_array_state state;
        carbon_insert *sub_ins = carbon_insert_array_begin(&state, array_ins,
                                                                      array->elements.elements.num_elems * 256);
        for (u32 i = 0; i < array->elements.elements.num_elems; i++) {
                const json_element *elem = VECTOR_GET(&array->elements.elements, i, json_element);
                int_carbon_from_json_elem(sub_ins, elem, false);
        }
        carbon_insert_array_end(&state);
}

static void int_insert_array_string(carbon_insert *array_ins, json_string *string)
{
        carbon_insert_string(array_ins, string->value);
}

static void int_insert_array_number(carbon_insert *array_ins, json_number *number)
{
        switch (number->value_type) {
                case JSON_NUMBER_FLOAT:
                        carbon_insert_float(array_ins, number->value.float_number);
                        break;
                case JSON_NUMBER_UNSIGNED:
                        carbon_insert_unsigned(array_ins, number->value.unsigned_integer);
                        break;
                case JSON_NUMBER_SIGNED:
                        carbon_insert_signed(array_ins, number->value.signed_integer);
                        break;
                default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
        }
}

static void int_insert_array_true(carbon_insert *array_ins)
{
        carbon_insert_true(array_ins);
}

static void int_insert_array_false(carbon_insert *array_ins)
{
        carbon_insert_false(array_ins);
}

static void int_insert_array_null(carbon_insert *array_ins)
{
        carbon_insert_null(array_ins);
}

static void int_insert_array_elements(carbon_insert *array_ins, json_array *array)
{
        for (u32 i = 0; i < array->elements.elements.num_elems; i++) {
                json_element *elem = VECTOR_GET(&array->elements.elements, i, json_element);
                switch (elem->value.value_type) {
                        case JSON_VALUE_OBJECT: {
                                carbon_insert_object_state state;
                                carbon_insert *sub_obj = carbon_insert_object_begin(&state, array_ins,
                                                                                               elem->value.value.object->value->members.num_elems *
                                                                                               256);
                                int_insert_prop_object(sub_obj, elem->value.value.object);
                                carbon_insert_object_end(&state);
                        }
                                break;
                        case JSON_VALUE_ARRAY:
                                int_insert_array_array(array_ins, elem->value.value.array);
                                break;
                        case JSON_VALUE_STRING:
                                int_insert_array_string(array_ins, elem->value.value.string);
                                break;
                        case JSON_VALUE_NUMBER:
                                int_insert_array_number(array_ins, elem->value.value.number);
                                break;
                        case JSON_VALUE_TRUE:
                                int_insert_array_true(array_ins);
                                break;
                        case JSON_VALUE_FALSE:
                                int_insert_array_false(array_ins);
                                break;
                        case JSON_VALUE_NULL:
                                int_insert_array_null(array_ins);
                                break;
                        default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

#define insert_into_array(ins, elem, ctype, accessor)                                                                  \
{                                                                                                                      \
        for (u32 k = 0; k < elem->value.value.array->elements.elements.num_elems; k++) {                               \
                json_element *array_elem = VECTOR_GET(                                                             \
                        &elem->value.value.array->elements.elements, k, json_element);                          \
                if (array_elem->value.value_type == JSON_VALUE_NULL) {                                                 \
                        carbon_insert_null(ins);                                                                       \
                } else {                                                                                               \
                        carbon_insert_##ctype(ins, array_elem->value.value.number->value.accessor);                    \
                }                                                                                                      \
        }                                                                                                              \
}

#define insert_into_column(ins, elem, field_type, column_type, ctype, accessor)                                        \
({                                                                                                                     \
        carbon_insert_column_state state;                                                                       \
        u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems *                                 \
                                carbon_int_get_type_value_size(field_type);                                            \
        carbon_insert *cins = carbon_insert_column_begin(&state, ins,                                           \
                                                                column_type, approx_cap_nbytes);                       \
        for (u32 k = 0; k < elem->value.value.array->elements.elements.num_elems; k++) {                               \
                json_element *array_elem = VECTOR_GET(&elem->value.value.array->elements.elements,                 \
                                                          k, json_element);                                     \
                if (array_elem->value.value_type == JSON_VALUE_NULL) {                                                 \
                        carbon_insert_null(cins);                                                                      \
                } else {                                                                                               \
                        carbon_insert_##ctype(cins, (ctype) array_elem->value.value.number->value.accessor);           \
                }                                                                                                      \
        }                                                                                                              \
        carbon_insert_column_end(&state);                                                                              \
})

#define prop_insert_into_column(ins, prop, key, field_type, column_type, ctype, accessor)                                   \
({                                                                                                                     \
        carbon_insert_column_state state;                                                                       \
        u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems *                                 \
                                carbon_int_get_type_value_size(field_type);                                            \
        carbon_insert *cins = carbon_insert_prop_column_begin(&state, ins, key,                                          \
                                                                column_type, approx_cap_nbytes);                       \
        for (u32 k = 0; k < prop->value.value.value.array->elements.elements.num_elems; k++) {                               \
                json_element *array_elem = VECTOR_GET(&prop->value.value.value.array->elements.elements,                 \
                                                          k, json_element);                                     \
                if (array_elem->value.value_type == JSON_VALUE_NULL) {                                                 \
                        carbon_insert_null(cins);                                                                      \
                } else {                                                                                               \
                        carbon_insert_##ctype(cins, (ctype) array_elem->value.value.number->value.accessor);           \
                }                                                                                                      \
        }                                                                                                              \
        carbon_insert_prop_column_end(&state);                                                                              \
})

static void int_insert_prop_object(carbon_insert *oins, json_object *obj)
{
        for (u32 i = 0; i < obj->value->members.num_elems; i++) {
                json_prop *prop = VECTOR_GET(&obj->value->members, i, json_prop);
                switch (prop->value.value.value_type) {
                        case JSON_VALUE_OBJECT: {
                                carbon_insert_object_state state;
                                carbon_insert *sub_obj = carbon_insert_prop_object_begin(&state, oins,
                                                                                                    prop->key.value,
                                                                                                    prop->value.value.value.object->value->members.num_elems *
                                                                                                    256);
                                int_insert_prop_object(sub_obj, prop->value.value.value.object);
                                carbon_insert_prop_object_end(&state);
                        }
                                break;
                        case JSON_VALUE_ARRAY: {
                                json_list_type_e type;
                                json_array_get_type(&type, prop->value.value.value.array);
                                switch (type) {
                                        case JSON_LIST_EMPTY: {
                                                carbon_insert_array_state state;
                                                carbon_insert_prop_array_begin(&state, oins, prop->key.value, 0);
                                                carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JSON_LIST_VARIABLE_OR_NESTED: {
                                                carbon_insert_array_state state;
                                                u64 approx_cap_nbytes =
                                                        prop->value.value.value.array->elements.elements.num_elems *
                                                        256;
                                                carbon_insert *array_ins = carbon_insert_prop_array_begin(
                                                        &state, oins,
                                                        prop->key.value, approx_cap_nbytes);
                                                int_insert_array_elements(array_ins, prop->value.value.value.array);
                                                carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JSON_LIST_FIXED_U8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U8,
                                                                        CARBON_COLUMN_TYPE_U8,
                                                                        u8, unsigned_integer);
                                                break;
                                        case JSON_LIST_FIXED_U16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U16,
                                                                        CARBON_COLUMN_TYPE_U16,
                                                                        u16, unsigned_integer);
                                                break;
                                        case JSON_LIST_FIXED_U32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U32,
                                                                        CARBON_COLUMN_TYPE_U32,
                                                                        u32, unsigned_integer);
                                                break;
                                        case JSON_LIST_FIXED_U64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U64,
                                                                        CARBON_COLUMN_TYPE_U64,
                                                                        u64, unsigned_integer);
                                                break;
                                        case JSON_LIST_FIXED_I8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I8,
                                                                        CARBON_COLUMN_TYPE_I8,
                                                                        i8, signed_integer);
                                                break;
                                        case JSON_LIST_FIXED_I16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I16,
                                                                        CARBON_COLUMN_TYPE_I16,
                                                                        i16, signed_integer);
                                                break;
                                        case JSON_LIST_FIXED_I32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I32,
                                                                        CARBON_COLUMN_TYPE_I32,
                                                                        i32, signed_integer);
                                                break;
                                        case JSON_LIST_FIXED_I64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I64,
                                                                        CARBON_COLUMN_TYPE_I64,
                                                                        i64, signed_integer);
                                                break;
                                        case JSON_LIST_FIXED_FLOAT:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_FLOAT,
                                                                        CARBON_COLUMN_TYPE_FLOAT,
                                                                        float, float_number);
                                                break;
                                        case JSON_LIST_FIXED_NULL: {
                                                carbon_insert_array_state state;
                                                u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                carbon_insert *array_ins = carbon_insert_prop_array_begin(
                                                        &state, oins, prop->key.value,
                                                        approx_cap_nbytes);
                                                for (u32 k = 0; k <
                                                                    prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        carbon_insert_null(array_ins);
                                                }
                                                carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JSON_LIST_FIXED_BOOLEAN: {
                                                carbon_insert_column_state state;
                                                u64 cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                carbon_insert *array_ins = carbon_insert_prop_column_begin(
                                                        &state, oins,
                                                        prop->key.value,
                                                        CARBON_COLUMN_TYPE_BOOLEAN, cap_nbytes);
                                                for (u32 k = 0; k <
                                                                    prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        json_element *array_elem = VECTOR_GET(
                                                                &prop->value.value.value.array->elements.elements, k,
                                                                json_element);
                                                        if (array_elem->value.value_type == JSON_VALUE_TRUE) {
                                                                carbon_insert_true(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_FALSE) {
                                                                carbon_insert_false(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_NULL) {
                                                                carbon_insert_null(array_ins);
                                                        } else {
                                                                ERROR_PRINT(ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                                carbon_insert_prop_column_end(&state);
                                        }
                                                break;
                                        default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                                                break;
                                }
                        }
                                break;
                        case JSON_VALUE_STRING:
                                carbon_insert_prop_string(oins, prop->key.value, prop->value.value.value.string->value);
                                break;
                        case JSON_VALUE_NUMBER:
                                switch (prop->value.value.value.number->value_type) {
                                        case JSON_NUMBER_FLOAT:
                                                carbon_insert_prop_float(oins, prop->key.value,
                                                                         prop->value.value.value.number->value.float_number);
                                                break;
                                        case JSON_NUMBER_UNSIGNED:
                                                carbon_insert_prop_unsigned(oins, prop->key.value,
                                                                            prop->value.value.value.number->value.unsigned_integer);
                                                break;
                                        case JSON_NUMBER_SIGNED:
                                                carbon_insert_prop_signed(oins, prop->key.value,
                                                                          prop->value.value.value.number->value.signed_integer);
                                                break;
                                        default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE);
                                                break;
                                }
                                break;
                        case JSON_VALUE_TRUE:
                                carbon_insert_prop_true(oins, prop->key.value);
                                break;
                        case JSON_VALUE_FALSE:
                                carbon_insert_prop_false(oins, prop->key.value);
                                break;
                        case JSON_VALUE_NULL:
                                carbon_insert_prop_null(oins, prop->key.value);
                                break;
                        default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

static void int_carbon_from_json_elem(carbon_insert *ins, const json_element *elem, bool is_root)
{
        switch (elem->value.value_type) {
                case JSON_VALUE_OBJECT: {
                        carbon_insert_object_state state;
                        carbon_insert *oins = carbon_insert_object_begin(&state, ins,
                                                                                    elem->value.value.object->value->members.num_elems *
                                                                                    256);
                        int_insert_prop_object(oins, elem->value.value.object);
                        carbon_insert_object_end(&state);
                }
                        break;
                case JSON_VALUE_ARRAY: {
                        json_list_type_e type;
                        json_array_get_type(&type, elem->value.value.array);
                        switch (type) {
                                case JSON_LIST_EMPTY: {
                                        if (is_root) {
                                                /** nothing to do */
                                        } else {
                                                carbon_insert_array_state state;
                                                carbon_insert_array_begin(&state, ins, 0);
                                                carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JSON_LIST_VARIABLE_OR_NESTED: {
                                        u64 approx_cap_nbytes =
                                                elem->value.value.array->elements.elements.num_elems * 256;
                                        if (is_root) {
                                                int_insert_array_elements(ins, elem->value.value.array);
                                        } else {
                                                carbon_insert_array_state state;
                                                carbon_insert *array_ins = carbon_insert_array_begin(&state,
                                                                                                                ins,
                                                                                                                approx_cap_nbytes);
                                                int_insert_array_elements(array_ins, elem->value.value.array);
                                                carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JSON_LIST_FIXED_U8:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U8,
                                                                   CARBON_COLUMN_TYPE_U8,
                                                                   u8, unsigned_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_U16:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U16,
                                                                   CARBON_COLUMN_TYPE_U16,
                                                                   u16, unsigned_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_U32:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U32,
                                                                   CARBON_COLUMN_TYPE_U32,
                                                                   u32, unsigned_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_U64:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U64,
                                                                   CARBON_COLUMN_TYPE_U64,
                                                                   u64, unsigned_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_I8:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I8,
                                                                   CARBON_COLUMN_TYPE_I8,
                                                                   i8, signed_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_I16:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I16,
                                                                   CARBON_COLUMN_TYPE_I16,
                                                                   u16, signed_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_I32:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I32,
                                                                   CARBON_COLUMN_TYPE_I32,
                                                                   u32, signed_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_I64:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I64,
                                                                   CARBON_COLUMN_TYPE_I64,
                                                                   u64, signed_integer);
                                        }
                                        break;
                                case JSON_LIST_FIXED_FLOAT:
                                        if (is_root) {
                                                insert_into_array(ins, elem, float, float_number)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_FLOAT,
                                                                   CARBON_COLUMN_TYPE_FLOAT,
                                                                   float, float_number);
                                        }
                                        break;
                                case JSON_LIST_FIXED_NULL: {
                                        u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                        if (is_root) {
                                                for (u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        carbon_insert_null(ins);
                                                }
                                        } else {
                                                carbon_insert_array_state state;
                                                carbon_insert *array_ins = carbon_insert_array_begin(&state,
                                                                                                                ins,
                                                                                                                approx_cap_nbytes);
                                                for (u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        carbon_insert_null(array_ins);
                                                }
                                                carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JSON_LIST_FIXED_BOOLEAN: {
                                        if (is_root) {
                                                for (u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        json_element *array_elem = VECTOR_GET(
                                                                &elem->value.value.array->elements.elements, i,
                                                                json_element);
                                                        if (array_elem->value.value_type == JSON_VALUE_TRUE) {
                                                                carbon_insert_true(ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_FALSE) {
                                                                carbon_insert_false(ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_NULL) {
                                                                carbon_insert_null(ins);
                                                        } else {
                                                                ERROR_PRINT(ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                        } else {
                                                carbon_insert_column_state state;
                                                u64 cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                                carbon_insert *array_ins = carbon_insert_column_begin(&state,
                                                                                                                 ins,
                                                                                                                 CARBON_COLUMN_TYPE_BOOLEAN,
                                                                                                                 cap_nbytes);
                                                for (u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        json_element *array_elem = VECTOR_GET(
                                                                &elem->value.value.array->elements.elements, i,
                                                                json_element);
                                                        if (array_elem->value.value_type == JSON_VALUE_TRUE) {
                                                                carbon_insert_true(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_FALSE) {
                                                                carbon_insert_false(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_NULL) {
                                                                carbon_insert_null(array_ins);
                                                        } else {
                                                                ERROR_PRINT(ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                                carbon_insert_column_end(&state);
                                        }
                                }
                                        break;
                                default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                                        break;
                        }
                }
                        break;
                case JSON_VALUE_STRING:
                        carbon_insert_string(ins, elem->value.value.string->value);
                        break;
                case JSON_VALUE_NUMBER:
                        switch (elem->value.value.number->value_type) {
                                case JSON_NUMBER_FLOAT:
                                        carbon_insert_float(ins, elem->value.value.number->value.float_number);
                                        break;
                                case JSON_NUMBER_UNSIGNED:
                                        carbon_insert_unsigned(ins, elem->value.value.number->value.unsigned_integer);
                                        break;
                                case JSON_NUMBER_SIGNED:
                                        carbon_insert_signed(ins, elem->value.value.number->value.signed_integer);
                                        break;
                                default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                                        break;
                        }
                        break;
                case JSON_VALUE_TRUE:
                        carbon_insert_true(ins);
                        break;
                case JSON_VALUE_FALSE:
                        carbon_insert_false(ins);
                        break;
                case JSON_VALUE_NULL:
                        carbon_insert_null(ins);
                        break;
                default: ERROR_PRINT(ERR_UNSUPPORTEDTYPE)
                        break;
        }
}

bool carbon_int_from_json(carbon *doc, const json *data,
                          carbon_key_e key_type, const void *primary_key, int mode)
{
        UNUSED(data)
        UNUSED(primary_key)

        carbon_new context;
        carbon_insert *ins = carbon_create_begin(&context, doc, key_type, mode);
        int_carbon_from_json_elem(ins, data->element, true);

        carbon_create_end(&context);

        return true;
}

static void marker_insert(memfile *memfile, u8 marker)
{
        /** check whether marker can be written, otherwise make space for it */
        char c = *memfile_peek(memfile, sizeof(u8));
        if (c != 0) {
                memfile_inplace_insert(memfile, sizeof(u8));
        }
        memfile_write(memfile, &marker, sizeof(u8));
}

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_array_end, &it->memfile, CARBON_MARRAY_END);
}

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, carbon_object_it *it)
{
        carbon_int_field_auto_close(&it->field.value.data);
        return is_slot_occupied(is_empty_slot, is_object_end, &it->memfile, CARBON_MOBJECT_END);
}

static bool is_slot_occupied(bool *is_empty_slot, bool *is_end_reached, memfile *file, u8 end_marker)
{
        ERROR_IF_NULL(file);
        char c = *memfile_peek(file, 1);
        bool is_empty = c == 0, is_end = c == end_marker;
        OPTIONAL_SET(is_empty_slot, is_empty)
        OPTIONAL_SET(is_end_reached, is_end)
        if (!is_empty && !is_end) {
                return true;
        } else {
                return false;
        }
}

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, carbon_object_it *it)
{
        if (object_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_object_it_prop_skip(it);
                return true;
        } else {
                return false;
        }
}

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, carbon_array_it *it)
{
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_array_it_field_type_read(it);
                carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}