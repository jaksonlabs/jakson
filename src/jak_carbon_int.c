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

#include <jak_uintvar_stream.h>
#include <jak_carbon.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_media.h>
#include <jak_carbon_int.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_key.h>
#include <jak_carbon_commit.h>
#include <jak_json.h>
#include <jak_carbon_object_it.h>

static void marker_insert(jak_memfile *memfile, jak_u8 marker);

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it);

static bool is_slot_occupied(bool *is_empty_slot, bool *is_array_end, jak_memfile *file, jak_u8 end_marker);

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, jak_carbon_object_it *it);

static void int_carbon_from_json_elem(jak_carbon_insert *ins, const jak_json_element *elem, bool is_root);

static void int_insert_prop_object(jak_carbon_insert *oins, jak_json_object *obj);

static void
insert_embedded_container(jak_memfile *memfile, jak_u8 begin_marker, jak_u8 end_marker, jak_u8 capacity)
{
        jak_memfile_ensure_space(memfile, sizeof(jak_u8));
        marker_insert(memfile, begin_marker);

        jak_memfile_ensure_space(memfile, capacity + sizeof(jak_u8));

        jak_offset_t payload_begin = jak_memfile_tell(memfile);
        jak_memfile_seek(memfile, payload_begin + capacity);

        marker_insert(memfile, end_marker);

        /* seek to first entry in container */
        jak_memfile_seek(memfile, payload_begin);
}

bool jak_carbon_int_insert_object(jak_memfile *memfile, carbon_map_derivable_e derivation, size_t nbytes)
{
        JAK_ERROR_IF_NULL(memfile);
        assert(derivation == CARBON_MAP_UNSORTED_MULTIMAP || derivation == CARBON_MAP_SORTED_MULTIMAP ||
               derivation == CARBON_MAP_UNSORTED_MAP || derivation == CARBON_MAP_SORTED_MAP);
        carbon_derived_e begin_marker;
        carbon_abstract_derive_map_to(&begin_marker, derivation);
        insert_embedded_container(memfile, begin_marker, CARBON_MOBJECT_END, nbytes);
        return true;
}

bool jak_carbon_int_insert_array(jak_memfile *memfile, carbon_list_derivable_e derivation, size_t nbytes)
{
        JAK_ERROR_IF_NULL(memfile);
        assert(derivation == CARBON_LIST_UNSORTED_MULTISET || derivation == CARBON_LIST_SORTED_MULTISET ||
               derivation == CARBON_LIST_UNSORTED_SET || derivation == CARBON_LIST_SORTED_SET);
        carbon_derived_e begin_marker;
        carbon_abstract_derive_list_to(&begin_marker, CARBON_LIST_CONTAINER_ARRAY, derivation);
        insert_embedded_container(memfile, begin_marker, CARBON_MARRAY_END, nbytes);
        return true;
}

bool jak_carbon_int_insert_column(jak_memfile *jak_memfile_in, jak_error *err_in, carbon_list_derivable_e derivation, jak_carbon_column_type_e type,
                              size_t capactity)
{
        JAK_ERROR_IF_NULL(jak_memfile_in);
        JAK_ERROR_IF_NULL(err_in);
        assert(derivation == CARBON_LIST_UNSORTED_MULTISET || derivation == CARBON_LIST_SORTED_MULTISET ||
               derivation == CARBON_LIST_UNSORTED_SET || derivation == CARBON_LIST_SORTED_SET);

        jak_carbon_field_type_e column_type = jak_carbon_field_type_for_column(derivation, type);

        jak_memfile_ensure_space(jak_memfile_in, sizeof(jak_u8));
        marker_insert(jak_memfile_in, column_type);

        jak_u32 num_elements = 0;
        jak_u32 cap_elements = capactity;

        jak_memfile_write_uintvar_stream(NULL, jak_memfile_in, num_elements);
        jak_memfile_write_uintvar_stream(NULL, jak_memfile_in, cap_elements);

        jak_offset_t payload_begin = jak_memfile_tell(jak_memfile_in);

        size_t type_size = jak_carbon_int_get_type_value_size(column_type);

        size_t nbytes = capactity * type_size;
        jak_memfile_ensure_space(jak_memfile_in, nbytes + sizeof(jak_u8) + 2 * sizeof(jak_u32));

        /* seek to first entry in column */
        jak_memfile_seek(jak_memfile_in, payload_begin);

        return true;
}

size_t jak_carbon_int_get_type_size_encoded(jak_carbon_field_type_e type)
{
        size_t type_size = sizeof(jak_media_type); /* at least the media type marker is required */
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /* only media type marker is required */
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                        type_size += sizeof(jak_u8);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                        type_size += sizeof(jak_u16);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                        type_size += sizeof(jak_u32);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                        type_size += sizeof(jak_u64);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        type_size += sizeof(float);
                        break;
                default: JAK_ERROR_PRINT(JAK_ERR_INTERNALERR);
                        return 0;
        }
        return type_size;
}

size_t jak_carbon_int_get_type_value_size(jak_carbon_field_type_e type)
{
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        return sizeof(jak_media_type); /* these constant values are determined by their media type markers */
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
                        return sizeof(jak_u8);
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
                        return sizeof(jak_u16);
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
                        return sizeof(jak_u32);
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
                        return sizeof(jak_u64);
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        return sizeof(float);
                default: JAK_ERROR_PRINT(JAK_ERR_INTERNALERR);
                        return 0;
        }
}

bool jak_carbon_int_array_it_next(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it)
{
        if (jak_carbon_int_array_it_refresh(is_empty_slot, is_array_end, it)) {
                jak_carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}

bool jak_carbon_int_object_it_next(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it)
{
        if (jak_carbon_int_object_it_refresh(is_empty_slot, is_object_end, it)) {
                jak_carbon_int_object_it_prop_value_skip(it);
                return true;
        } else {
                return false;
        }
}

bool jak_carbon_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        if (object_it_is_slot_occupied(is_empty_slot, is_object_end, it)) {
                jak_carbon_int_object_it_prop_key_access(it);
                jak_carbon_int_field_data_access(&it->memfile, &it->err, &it->field.value.data);
                return true;
        } else {
                return false;
        }
}

bool jak_carbon_int_object_it_prop_key_access(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        //jak_memfile_skip(&it->memfile, sizeof(jak_media_type));

        it->field.key.offset = jak_memfile_tell(&it->memfile);
        it->field.key.name_len = jak_memfile_read_uintvar_stream(NULL, &it->memfile);
        it->field.key.name = jak_memfile_peek(&it->memfile, it->field.key.name_len);
        jak_memfile_skip(&it->memfile, it->field.key.name_len);
        it->field.value.offset = jak_memfile_tell(&it->memfile);
        it->field.value.data.it_field_type = *JAK_MEMFILE_PEEK(&it->memfile, jak_u8);

        return true;
}

bool jak_carbon_int_object_it_prop_value_skip(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        jak_memfile_seek(&it->memfile, it->field.value.offset);
        return jak_carbon_field_skip(&it->memfile);
}

bool jak_carbon_int_object_it_prop_skip(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)

        it->field.key.name_len = jak_memfile_read_uintvar_stream(NULL, &it->memfile);
        jak_memfile_skip(&it->memfile, it->field.key.name_len);

        return jak_carbon_field_skip(&it->memfile);
}

bool jak_carbon_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_object_it *it)
{
        while (object_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool jak_carbon_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it)
{
        while (array_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool jak_carbon_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_carbon_int_field_access_drop(&it->field_access);
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                jak_carbon_int_array_it_field_type_read(it);
                jak_carbon_int_field_data_access(&it->memfile, &it->err, &it->field_access);
                return true;
        } else {
                return false;
        }
}

bool jak_carbon_int_array_it_field_type_read(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF(jak_memfile_remain_size(&it->memfile) < 1, &it->err, JAK_ERR_ILLEGALOP);
        jak_memfile_save_position(&it->memfile);
        it->field_offset = jak_memfile_tell(&it->memfile);
        jak_u8 media_type = *jak_memfile_read(&it->memfile, 1);
        JAK_ERROR_IF(media_type == 0, &it->err, JAK_ERR_NOTFOUND)
        JAK_ERROR_IF(media_type == CARBON_MARRAY_END, &it->err, JAK_ERR_OUTOFBOUNDS)
        it->field_access.it_field_type = media_type;
        jak_memfile_restore_position(&it->memfile);
        return true;
}

bool jak_carbon_int_field_data_access(jak_memfile *file, jak_error *err, jak_field_access *field_access)
{
        jak_memfile_save_position(file);
        jak_memfile_skip(file, sizeof(jak_media_type));

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
                        jak_u8 nbytes;
                        jak_uintvar_stream_t len = (jak_uintvar_stream_t) jak_memfile_peek(file, 1);
                        field_access->it_field_len = jak_uintvar_stream_read(&nbytes, len);

                        jak_memfile_skip(file, nbytes);
                }
                        break;
                case CARBON_FIELD_BINARY: {
                        /* read mime type with variable-length integer type */
                        jak_u64 mime_type_id = jak_memfile_read_uintvar_stream(NULL, file);

                        field_access->it_mime_type = jak_carbon_media_mime_type_by_id(mime_type_id);
                        field_access->it_mime_type_strlen = strlen(field_access->it_mime_type);

                        /* read blob length */
                        field_access->it_field_len = jak_memfile_read_uintvar_stream(NULL, file);

                        /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_BINARY_CUSTOM: {
                        /* read mime type string */
                        field_access->it_mime_type_strlen = jak_memfile_read_uintvar_stream(NULL, file);
                        field_access->it_mime_type = jak_memfile_read(file, field_access->it_mime_type_strlen);

                        /* read blob length */
                        field_access->it_field_len = jak_memfile_read_uintvar_stream(NULL, file);

                        /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                        jak_carbon_int_field_access_create(field_access);
                        field_access->nested_array_it_is_created = true;
                        jak_carbon_array_it_create(field_access->nested_array_it, file, err,
                                               jak_memfile_tell(file) - sizeof(jak_u8));
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
                        jak_carbon_int_field_access_create(field_access);
                        field_access->nested_column_it_is_created = true;
                        jak_carbon_column_it_create(field_access->nested_column_it, file, err,
                                                jak_memfile_tell(file) - sizeof(jak_u8));
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                        jak_carbon_int_field_access_create(field_access);
                        field_access->nested_object_it_is_created = true;
                        jak_carbon_object_it_create(field_access->nested_object_it, file, err,
                                                jak_memfile_tell(file) - sizeof(jak_u8));
                        break;
                default: JAK_ERROR(err, JAK_ERR_CORRUPTED)
                        return false;
        }

        field_access->it_field_data = jak_memfile_peek(file, 1);
        jak_memfile_restore_position(file);
        return true;
}

jak_offset_t jak_carbon_int_column_get_payload_off(jak_carbon_column_it *it)
{
        jak_memfile_save_position(&it->memfile);
        jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        jak_memfile_skip_uintvar_stream(&it->memfile); // skip num of elements
        jak_memfile_skip_uintvar_stream(&it->memfile); // skip capacity of elements
        jak_offset_t result = jak_memfile_tell(&it->memfile);
        jak_memfile_restore_position(&it->memfile);
        return result;
}

jak_offset_t jak_carbon_int_payload_after_header(jak_carbon *doc)
{
        jak_offset_t result = 0;
        jak_carbon_key_e key_type;

        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);

        if (JAK_LIKELY(jak_carbon_key_skip(&key_type, &doc->memfile))) {
                if (key_type != JAK_CARBON_KEY_NOKEY) {
                        jak_carbon_commit_hash_skip(&doc->memfile);
                }
                result = jak_memfile_tell(&doc->memfile);
        }

        jak_memfile_restore_position(&doc->memfile);

        return result;
}

jak_u64 jak_carbon_int_header_get_commit_hash(jak_carbon *doc)
{
        JAK_ASSERT(doc);
        jak_u64 rev = 0;
        jak_carbon_key_e key_type;

        jak_memfile_save_position(&doc->memfile);
        jak_memfile_seek(&doc->memfile, 0);

        jak_carbon_key_skip(&key_type, &doc->memfile);
        if (key_type != JAK_CARBON_KEY_NOKEY) {
                jak_carbon_commit_hash_read(&rev, &doc->memfile);
        }

        jak_memfile_restore_position(&doc->memfile);
        return rev;
}

void jak_carbon_int_history_push(jak_vector ofType(jak_offset_t) *vec, jak_offset_t off)
{
        JAK_ASSERT(vec);
        jak_vector_push(vec, &off, 1);
}

void jak_carbon_int_history_clear(jak_vector ofType(jak_offset_t) *vec)
{
        JAK_ASSERT(vec);
        jak_vector_clear(vec);
}

jak_offset_t jak_carbon_int_history_pop(jak_vector ofType(jak_offset_t) *vec)
{
        JAK_ASSERT(vec);
        JAK_ASSERT(jak_carbon_int_history_has(vec));
        return *(jak_offset_t *) jak_vector_pop(vec);
}

jak_offset_t jak_carbon_int_history_peek(jak_vector ofType(jak_offset_t) *vec)
{
        JAK_ASSERT(vec);
        JAK_ASSERT(jak_carbon_int_history_has(vec));
        return *(jak_offset_t *) jak_vector_peek(vec);
}

bool jak_carbon_int_history_has(jak_vector ofType(jak_offset_t) *vec)
{
        JAK_ASSERT(vec);
        return !jak_vector_is_empty(vec);
}

bool jak_carbon_int_field_access_create(jak_field_access *field)
{
        field->nested_array_it_is_created = false;
        field->nested_array_it_accessed = false;
        field->nested_object_it_is_created = false;
        field->nested_object_it_accessed = false;
        field->nested_column_it_is_created = false;
        field->nested_array_it = JAK_MALLOC(sizeof(jak_carbon_array_it));
        field->nested_object_it = JAK_MALLOC(sizeof(jak_carbon_object_it));
        field->nested_column_it = JAK_MALLOC(sizeof(jak_carbon_column_it));
        return true;
}

bool jak_carbon_int_field_access_clone(jak_field_access *dst, jak_field_access *src)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)

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
        dst->nested_array_it = JAK_MALLOC(sizeof(jak_carbon_array_it));
        dst->nested_object_it = JAK_MALLOC(sizeof(jak_carbon_object_it));
        dst->nested_column_it = JAK_MALLOC(sizeof(jak_carbon_column_it));

        if (jak_carbon_int_field_access_object_it_opened(src)) {
                jak_carbon_object_it_clone(dst->nested_object_it, src->nested_object_it);
        } else if (jak_carbon_int_field_access_column_it_opened(src)) {
                jak_carbon_column_it_clone(dst->nested_column_it, src->nested_column_it);
        } else if (jak_carbon_int_field_access_array_it_opened(src)) {
                jak_carbon_array_it_clone(dst->nested_array_it, src->nested_array_it);
        }
        return true;
}

bool jak_carbon_int_field_access_drop(jak_field_access *field)
{
        jak_carbon_int_field_auto_close(field);
        free(field->nested_array_it);
        free(field->nested_object_it);
        free(field->nested_column_it);
        field->nested_array_it = NULL;
        field->nested_object_it = NULL;
        field->nested_column_it = NULL;
        return true;
}

bool jak_carbon_int_field_access_object_it_opened(jak_field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_object_it_is_created && field->nested_object_it != NULL;
}

bool jak_carbon_int_field_access_array_it_opened(jak_field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_array_it_is_created && field->nested_array_it != NULL;
}

bool jak_carbon_int_field_access_column_it_opened(jak_field_access *field)
{
        JAK_ASSERT(field);
        return field->nested_column_it_is_created && field->nested_column_it != NULL;
}

void jak_carbon_int_auto_close_nested_array_it(jak_field_access *field)
{
        if (jak_carbon_int_field_access_array_it_opened(field)) {
                jak_carbon_array_it_drop(field->nested_array_it);
                JAK_ZERO_MEMORY(field->nested_array_it, sizeof(jak_carbon_array_it));
        }
}

void jak_carbon_int_auto_close_nested_object_it(jak_field_access *field)
{
        if (jak_carbon_int_field_access_object_it_opened(field)) {
                jak_carbon_object_it_drop(field->nested_object_it);
                JAK_ZERO_MEMORY(field->nested_object_it, sizeof(jak_carbon_object_it));
        }
}

void jak_carbon_int_auto_close_nested_column_it(jak_field_access *field)
{
        if (jak_carbon_int_field_access_column_it_opened(field)) {
                JAK_ZERO_MEMORY(field->nested_column_it, sizeof(jak_carbon_column_it));
        }
}

bool jak_carbon_int_field_auto_close(jak_field_access *field)
{
        JAK_ERROR_IF_NULL(field)
        if (field->nested_array_it_is_created && !field->nested_array_it_accessed) {
                jak_carbon_int_auto_close_nested_array_it(field);
                field->nested_array_it_is_created = false;
                field->nested_array_it_accessed = false;
        }
        if (field->nested_object_it_is_created && !field->nested_object_it_accessed) {
                jak_carbon_int_auto_close_nested_object_it(field);
                field->nested_object_it_is_created = false;
                field->nested_object_it_accessed = false;
        }
        if (field->nested_column_it_is_created) {
                jak_carbon_int_auto_close_nested_column_it(field);
                field->nested_object_it_is_created = false;
        }

        return true;
}

bool jak_carbon_int_field_access_field_type(jak_carbon_field_type_e *type, jak_field_access *field)
{
        JAK_ERROR_IF_NULL(type)
        JAK_ERROR_IF_NULL(field)
        *type = field->it_field_type;
        return true;
}

bool jak_carbon_int_field_access_u8_value(jak_u8 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U8, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_u8 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_u16_value(jak_u16 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U16, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_u16 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_u32_value(jak_u32 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U32, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_u32 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_u64_value(jak_u64 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_U64, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_u64 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_i8_value(jak_i8 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I8, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_i8 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_i16_value(jak_i16 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I16, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_i16 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_i32_value(jak_i32 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I32, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_i32 *) field->it_field_data;
        return true;
}

bool jak_carbon_int_field_access_i64_value(jak_i64 *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(value)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_I64, err, JAK_ERR_TYPEMISMATCH);
        *value = *(jak_i64 *) field->it_field_data;
        return true;
}

bool
jak_carbon_int_field_access_float_value(bool *is_null_in, float *value, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_NUMBER_FLOAT, err, JAK_ERR_TYPEMISMATCH);
        float read_value = *(float *) field->it_field_data;
        JAK_OPTIONAL_SET(value, read_value);
        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_FLOAT(read_value));

        return true;
}

bool jak_carbon_int_field_access_signed_value(bool *is_null_in, jak_i64 *value, jak_field_access *field,
                                          jak_error *err)
{
        JAK_ERROR_IF_NULL(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_NUMBER_I8: {
                        jak_i8 read_value;
                        jak_carbon_int_field_access_i8_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_I8(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I16: {
                        jak_i16 read_value;
                        jak_carbon_int_field_access_i16_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_I16(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I32: {
                        jak_i32 read_value;
                        jak_carbon_int_field_access_i32_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_I32(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_I64: {
                        jak_i64 read_value;
                        jak_carbon_int_field_access_i64_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_I64(read_value));
                }
                        break;
                default: JAK_ERROR(err, JAK_ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

bool jak_carbon_int_field_access_unsigned_value(bool *is_null_in, jak_u64 *value, jak_field_access *field,
                                            jak_error *err)
{
        JAK_ERROR_IF_NULL(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_NUMBER_U8: {
                        jak_u8 read_value;
                        jak_carbon_int_field_access_u8_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_U8(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U16: {
                        jak_u16 read_value;
                        jak_carbon_int_field_access_u16_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_U16(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U32: {
                        jak_u32 read_value;
                        jak_carbon_int_field_access_u32_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_U32(read_value));
                }
                        break;
                case CARBON_FIELD_NUMBER_U64: {
                        jak_u64 read_value;
                        jak_carbon_int_field_access_u64_value(&read_value, field, err);
                        JAK_OPTIONAL_SET(value, read_value);
                        JAK_OPTIONAL_SET(is_null_in, JAK_IS_NULL_U64(read_value));
                }
                        break;
                default: JAK_ERROR(err, JAK_ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

const char *jak_carbon_int_field_access_jak_string_value(jak_u64 *strlen, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(strlen);
        JAK_ERROR_IF_AND_RETURN(field == NULL, err, JAK_ERR_NULLPTR, NULL);
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_STRING, err, JAK_ERR_TYPEMISMATCH);
        *strlen = field->it_field_len;
        return field->it_field_data;
}

bool
jak_carbon_int_field_access_binary_value(jak_carbon_binary *out, jak_field_access *field, jak_error *err)
{
        JAK_ERROR_IF_NULL(out)
        JAK_ERROR_IF_NULL(field)
        JAK_ERROR_IF(field->it_field_type != CARBON_FIELD_BINARY &&
                 field->it_field_type != CARBON_FIELD_BINARY_CUSTOM,
                 err, JAK_ERR_TYPEMISMATCH);
        out->blob = field->it_field_data;
        out->blob_len = field->it_field_len;
        out->mime_type = field->it_mime_type;
        out->mime_type_strlen = field->it_mime_type_strlen;
        return true;
}

jak_carbon_array_it *jak_carbon_int_field_access_array_value(jak_field_access *field, jak_error *err)
{
        JAK_ERROR_PRINT_IF(!field, JAK_ERR_NULLPTR);
        JAK_ERROR_IF(!jak_carbon_field_type_is_array_or_subtype(field->it_field_type), err, JAK_ERR_TYPEMISMATCH);
        field->nested_array_it_accessed = true;
        return field->nested_array_it;
}

jak_carbon_object_it *jak_carbon_int_field_access_object_value(jak_field_access *field, jak_error *err)
{
        JAK_ERROR_PRINT_IF(!field, JAK_ERR_NULLPTR);
        JAK_ERROR_IF(!jak_carbon_field_type_is_object_or_subtype(field->it_field_type), err, JAK_ERR_TYPEMISMATCH);
        field->nested_object_it_accessed = true;
        return field->nested_object_it;
}

jak_carbon_column_it *jak_carbon_int_field_access_column_value(jak_field_access *field, jak_error *err)
{
        JAK_ERROR_PRINT_IF(!field, JAK_ERR_NULLPTR);
        JAK_ERROR_IF(!jak_carbon_field_type_is_column_or_subtype(field->it_field_type), err, JAK_ERR_TYPEMISMATCH);
        return field->nested_column_it;
}

bool jak_carbon_int_field_remove(jak_memfile *memfile, jak_error *err, jak_carbon_field_type_e type)
{
        JAK_ASSERT((jak_carbon_field_type_e) *jak_memfile_peek(memfile, sizeof(jak_u8)) == type);
        jak_offset_t start_off = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, sizeof(jak_u8));
        size_t rm_nbytes = sizeof(jak_u8); /* at least the type marker must be removed */
        switch (type) {
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                        /* nothing to do */
                        break;
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_I8:
                        rm_nbytes += sizeof(jak_u8);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_I16:
                        rm_nbytes += sizeof(jak_u16);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_I32:
                        rm_nbytes += sizeof(jak_u32);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I64:
                        rm_nbytes += sizeof(jak_u64);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        rm_nbytes += sizeof(float);
                        break;
                case CARBON_FIELD_STRING: {
                        jak_u8 len_nbytes;  /* number of bytes used to store string length */
                        jak_u64 str_len; /* the number of characters of the string field */

                        str_len = jak_memfile_read_uintvar_stream(&len_nbytes, memfile);

                        rm_nbytes += len_nbytes + str_len;
                }
                        break;
                case CARBON_FIELD_BINARY: {
                        jak_u8 mime_type_nbytes; /* number of bytes for mime type */
                        jak_u8 blob_length_nbytes; /* number of bytes to store blob length */
                        jak_u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes used for mime type id */
                        jak_memfile_read_uintvar_stream(&mime_type_nbytes, memfile);

                        /* get bytes used for blob length info */
                        blob_nbytes = jak_memfile_read_uintvar_stream(&blob_length_nbytes, memfile);

                        rm_nbytes += mime_type_nbytes + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_BINARY_CUSTOM: {
                        jak_u8 custom_type_strlen_nbytes; /* number of bytes for type name string length info */
                        jak_u8 custom_type_strlen; /* number of characters to encode type name string */
                        jak_u8 blob_length_nbytes; /* number of bytes to store blob length */
                        jak_u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes for custom type string len, and the actual length */
                        custom_type_strlen = jak_memfile_read_uintvar_stream(&custom_type_strlen_nbytes, memfile);
                        jak_memfile_skip(memfile, custom_type_strlen);

                        /* get bytes used for blob length info */
                        blob_nbytes = jak_memfile_read_uintvar_stream(&blob_length_nbytes, memfile);

                        rm_nbytes += custom_type_strlen_nbytes + custom_type_strlen + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET: {
                        jak_carbon_array_it it;

                        jak_offset_t begin_off = jak_memfile_tell(memfile);
                        jak_carbon_array_it_create(&it, memfile, err, begin_off - sizeof(jak_u8));
                        jak_carbon_array_it_fast_forward(&it);
                        jak_offset_t end_off = jak_carbon_array_it_memfilepos(&it);
                        jak_carbon_array_it_drop(&it);

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
                        jak_carbon_column_it it;

                        jak_offset_t begin_off = jak_memfile_tell(memfile);
                        jak_carbon_column_it_create(&it, memfile, err, begin_off - sizeof(jak_u8));
                        jak_carbon_column_it_fast_forward(&it);
                        jak_offset_t end_off = jak_carbon_column_it_memfilepos(&it);

                        JAK_ASSERT(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP: {
                        jak_carbon_object_it it;

                        jak_offset_t begin_off = jak_memfile_tell(memfile);
                        jak_carbon_object_it_create(&it, memfile, err, begin_off - sizeof(jak_u8));
                        jak_carbon_object_it_fast_forward(&it);
                        jak_offset_t end_off = jak_carbon_object_it_jak_memfile_pos(&it);
                        jak_carbon_object_it_drop(&it);

                        JAK_ASSERT(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                default: JAK_ERROR(err, JAK_ERR_INTERNALERR)
                        return false;
        }
        jak_memfile_seek(memfile, start_off);
        jak_memfile_inplace_remove(memfile, rm_nbytes);

        return true;
}

static void int_insert_array_array(jak_carbon_insert *array_ins, jak_json_array *array)
{
        jak_carbon_insert_array_state state;
        jak_carbon_insert *sub_ins = jak_carbon_insert_array_begin(&state, array_ins,
                                                                      array->elements.elements.num_elems * 256);
        for (jak_u32 i = 0; i < array->elements.elements.num_elems; i++) {
                const jak_json_element *elem = JAK_VECTOR_GET(&array->elements.elements, i, jak_json_element);
                int_carbon_from_json_elem(sub_ins, elem, false);
        }
        jak_carbon_insert_array_end(&state);
}

static void int_insert_array_string(jak_carbon_insert *array_ins, jak_json_string *string)
{
        jak_carbon_insert_string(array_ins, string->value);
}

static void int_insert_array_number(jak_carbon_insert *array_ins, jak_json_number *number)
{
        switch (number->value_type) {
                case JAK_JSON_NUMBER_FLOAT:
                        jak_carbon_insert_float(array_ins, number->value.float_number);
                        break;
                case JAK_JSON_NUMBER_UNSIGNED:
                        jak_carbon_insert_unsigned(array_ins, number->value.unsigned_integer);
                        break;
                case JAK_JSON_NUMBER_SIGNED:
                        jak_carbon_insert_signed(array_ins, number->value.signed_integer);
                        break;
                default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
        }
}

static void int_insert_array_true(jak_carbon_insert *array_ins)
{
        jak_carbon_insert_true(array_ins);
}

static void int_insert_array_false(jak_carbon_insert *array_ins)
{
        jak_carbon_insert_false(array_ins);
}

static void int_insert_array_null(jak_carbon_insert *array_ins)
{
        jak_carbon_insert_null(array_ins);
}

static void int_insert_array_elements(jak_carbon_insert *array_ins, jak_json_array *array)
{
        for (jak_u32 i = 0; i < array->elements.elements.num_elems; i++) {
                jak_json_element *elem = JAK_VECTOR_GET(&array->elements.elements, i, jak_json_element);
                switch (elem->value.value_type) {
                        case JAK_JSON_VALUE_OBJECT: {
                                jak_carbon_insert_object_state state;
                                jak_carbon_insert *sub_obj = jak_carbon_insert_object_begin(&state, array_ins,
                                                                                               elem->value.value.object->value->members.num_elems *
                                                                                               256);
                                int_insert_prop_object(sub_obj, elem->value.value.object);
                                jak_carbon_insert_object_end(&state);
                        }
                                break;
                        case JAK_JSON_VALUE_ARRAY:
                                int_insert_array_array(array_ins, elem->value.value.array);
                                break;
                        case JAK_JSON_VALUE_STRING:
                                int_insert_array_string(array_ins, elem->value.value.string);
                                break;
                        case JAK_JSON_VALUE_NUMBER:
                                int_insert_array_number(array_ins, elem->value.value.number);
                                break;
                        case JAK_JSON_VALUE_TRUE:
                                int_insert_array_true(array_ins);
                                break;
                        case JAK_JSON_VALUE_FALSE:
                                int_insert_array_false(array_ins);
                                break;
                        case JAK_JSON_VALUE_NULL:
                                int_insert_array_null(array_ins);
                                break;
                        default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

#define insert_into_array(ins, elem, ctype, accessor)                                                                  \
{                                                                                                                      \
        for (jak_u32 k = 0; k < elem->value.value.array->elements.elements.num_elems; k++) {                               \
                jak_json_element *array_elem = JAK_VECTOR_GET(                                                             \
                        &elem->value.value.array->elements.elements, k, jak_json_element);                          \
                if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {                                                 \
                        jak_carbon_insert_null(ins);                                                                       \
                } else {                                                                                               \
                        jak_carbon_insert_##ctype(ins, array_elem->value.value.number->value.accessor);                    \
                }                                                                                                      \
        }                                                                                                              \
}

#define insert_into_column(ins, elem, field_type, column_type, ctype, accessor)                                        \
({                                                                                                                     \
        jak_carbon_insert_column_state state;                                                                       \
        jak_u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems *                                 \
                                jak_carbon_int_get_type_value_size(field_type);                                            \
        jak_carbon_insert *cins = jak_carbon_insert_column_begin(&state, ins,                                           \
                                                                column_type, approx_cap_nbytes);                       \
        for (jak_u32 k = 0; k < elem->value.value.array->elements.elements.num_elems; k++) {                               \
                jak_json_element *array_elem = JAK_VECTOR_GET(&elem->value.value.array->elements.elements,                 \
                                                          k, jak_json_element);                                     \
                if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {                                                 \
                        jak_carbon_insert_null(cins);                                                                      \
                } else {                                                                                               \
                        jak_carbon_insert_##ctype(cins, (jak_##ctype) array_elem->value.value.number->value.accessor);           \
                }                                                                                                      \
        }                                                                                                              \
        jak_carbon_insert_column_end(&state);                                                                              \
})

#define prop_insert_into_column(ins, prop, key, field_type, column_type, ctype, accessor)                                   \
({                                                                                                                     \
        jak_carbon_insert_column_state state;                                                                       \
        jak_u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems *                                 \
                                jak_carbon_int_get_type_value_size(field_type);                                            \
        jak_carbon_insert *cins = jak_carbon_insert_prop_column_begin(&state, ins, key,                                          \
                                                                column_type, approx_cap_nbytes);                       \
        for (jak_u32 k = 0; k < prop->value.value.value.array->elements.elements.num_elems; k++) {                               \
                jak_json_element *array_elem = JAK_VECTOR_GET(&prop->value.value.value.array->elements.elements,                 \
                                                          k, jak_json_element);                                     \
                if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {                                                 \
                        jak_carbon_insert_null(cins);                                                                      \
                } else {                                                                                               \
                        jak_carbon_insert_##ctype(cins, (jak_##ctype) array_elem->value.value.number->value.accessor);           \
                }                                                                                                      \
        }                                                                                                              \
        jak_carbon_insert_prop_column_end(&state);                                                                              \
})

static void int_insert_prop_object(jak_carbon_insert *oins, jak_json_object *obj)
{
        for (jak_u32 i = 0; i < obj->value->members.num_elems; i++) {
                jak_json_prop *prop = JAK_VECTOR_GET(&obj->value->members, i, jak_json_prop);
                switch (prop->value.value.value_type) {
                        case JAK_JSON_VALUE_OBJECT: {
                                jak_carbon_insert_object_state state;
                                jak_carbon_insert *sub_obj = jak_carbon_insert_prop_object_begin(&state, oins,
                                                                                                    prop->key.value,
                                                                                                    prop->value.value.value.object->value->members.num_elems *
                                                                                                    256);
                                int_insert_prop_object(sub_obj, prop->value.value.value.object);
                                jak_carbon_insert_prop_object_end(&state);
                        }
                                break;
                        case JAK_JSON_VALUE_ARRAY: {
                                jak_json_list_type_e type;
                                jak_json_array_get_type(&type, prop->value.value.value.array);
                                switch (type) {
                                        case JAK_JSON_LIST_EMPTY: {
                                                jak_carbon_insert_array_state state;
                                                jak_carbon_insert_prop_array_begin(&state, oins, prop->key.value, 0);
                                                jak_carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JAK_JSON_LIST_VARIABLE_OR_NESTED: {
                                                jak_carbon_insert_array_state state;
                                                jak_u64 approx_cap_nbytes =
                                                        prop->value.value.value.array->elements.elements.num_elems *
                                                        256;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_prop_array_begin(
                                                        &state, oins,
                                                        prop->key.value, approx_cap_nbytes);
                                                int_insert_array_elements(array_ins, prop->value.value.value.array);
                                                jak_carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JAK_JSON_LIST_FIXED_U8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U8,
                                                                        JAK_CARBON_COLUMN_TYPE_U8,
                                                                        u8, unsigned_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_U16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U16,
                                                                        JAK_CARBON_COLUMN_TYPE_U16,
                                                                        u16, unsigned_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_U32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U32,
                                                                        JAK_CARBON_COLUMN_TYPE_U32,
                                                                        u32, unsigned_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_U64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_U64,
                                                                        JAK_CARBON_COLUMN_TYPE_U64,
                                                                        u64, unsigned_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_I8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I8,
                                                                        JAK_CARBON_COLUMN_TYPE_I8,
                                                                        i8, signed_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_I16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I16,
                                                                        JAK_CARBON_COLUMN_TYPE_I16,
                                                                        i16, signed_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_I32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I32,
                                                                        JAK_CARBON_COLUMN_TYPE_I32,
                                                                        i32, signed_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_I64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_I64,
                                                                        JAK_CARBON_COLUMN_TYPE_I64,
                                                                        i64, signed_integer);
                                                break;
                                        case JAK_JSON_LIST_FIXED_FLOAT:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_NUMBER_FLOAT,
                                                                        JAK_CARBON_COLUMN_TYPE_FLOAT,
                                                                        float, float_number);
                                                break;
                                        case JAK_JSON_LIST_FIXED_NULL: {
                                                jak_carbon_insert_array_state state;
                                                jak_u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_prop_array_begin(
                                                        &state, oins, prop->key.value,
                                                        approx_cap_nbytes);
                                                for (jak_u32 k = 0; k <
                                                                    prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        jak_carbon_insert_null(array_ins);
                                                }
                                                jak_carbon_insert_prop_array_end(&state);
                                        }
                                                break;
                                        case JAK_JSON_LIST_FIXED_BOOLEAN: {
                                                jak_carbon_insert_column_state state;
                                                jak_u64 cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_prop_column_begin(
                                                        &state, oins,
                                                        prop->key.value,
                                                        JAK_CARBON_COLUMN_TYPE_BOOLEAN, cap_nbytes);
                                                for (jak_u32 k = 0; k <
                                                                    prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        jak_json_element *array_elem = JAK_VECTOR_GET(
                                                                &prop->value.value.value.array->elements.elements, k,
                                                                jak_json_element);
                                                        if (array_elem->value.value_type == JAK_JSON_VALUE_TRUE) {
                                                                jak_carbon_insert_true(array_ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_FALSE) {
                                                                jak_carbon_insert_false(array_ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {
                                                                jak_carbon_insert_null(array_ins);
                                                        } else {
                                                                JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                                jak_carbon_insert_prop_column_end(&state);
                                        }
                                                break;
                                        default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                                                break;
                                }
                        }
                                break;
                        case JAK_JSON_VALUE_STRING:
                                jak_carbon_insert_prop_string(oins, prop->key.value, prop->value.value.value.string->value);
                                break;
                        case JAK_JSON_VALUE_NUMBER:
                                switch (prop->value.value.value.number->value_type) {
                                        case JAK_JSON_NUMBER_FLOAT:
                                                jak_carbon_insert_prop_float(oins, prop->key.value,
                                                                         prop->value.value.value.number->value.float_number);
                                                break;
                                        case JAK_JSON_NUMBER_UNSIGNED:
                                                jak_carbon_insert_prop_unsigned(oins, prop->key.value,
                                                                            prop->value.value.value.number->value.unsigned_integer);
                                                break;
                                        case JAK_JSON_NUMBER_SIGNED:
                                                jak_carbon_insert_prop_signed(oins, prop->key.value,
                                                                          prop->value.value.value.number->value.signed_integer);
                                                break;
                                        default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE);
                                                break;
                                }
                                break;
                        case JAK_JSON_VALUE_TRUE:
                                jak_carbon_insert_prop_true(oins, prop->key.value);
                                break;
                        case JAK_JSON_VALUE_FALSE:
                                jak_carbon_insert_prop_false(oins, prop->key.value);
                                break;
                        case JAK_JSON_VALUE_NULL:
                                jak_carbon_insert_prop_null(oins, prop->key.value);
                                break;
                        default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

static void int_carbon_from_json_elem(jak_carbon_insert *ins, const jak_json_element *elem, bool is_root)
{
        switch (elem->value.value_type) {
                case JAK_JSON_VALUE_OBJECT: {
                        jak_carbon_insert_object_state state;
                        jak_carbon_insert *oins = jak_carbon_insert_object_begin(&state, ins,
                                                                                    elem->value.value.object->value->members.num_elems *
                                                                                    256);
                        int_insert_prop_object(oins, elem->value.value.object);
                        jak_carbon_insert_object_end(&state);
                }
                        break;
                case JAK_JSON_VALUE_ARRAY: {
                        jak_json_list_type_e type;
                        jak_json_array_get_type(&type, elem->value.value.array);
                        switch (type) {
                                case JAK_JSON_LIST_EMPTY: {
                                        if (is_root) {
                                                /* nothing to do */
                                        } else {
                                                jak_carbon_insert_array_state state;
                                                jak_carbon_insert_array_begin(&state, ins, 0);
                                                jak_carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JAK_JSON_LIST_VARIABLE_OR_NESTED: {
                                        jak_u64 approx_cap_nbytes =
                                                elem->value.value.array->elements.elements.num_elems * 256;
                                        if (is_root) {
                                                int_insert_array_elements(ins, elem->value.value.array);
                                        } else {
                                                jak_carbon_insert_array_state state;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_array_begin(&state,
                                                                                                                ins,
                                                                                                                approx_cap_nbytes);
                                                int_insert_array_elements(array_ins, elem->value.value.array);
                                                jak_carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JAK_JSON_LIST_FIXED_U8:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U8,
                                                                   JAK_CARBON_COLUMN_TYPE_U8,
                                                                   u8, unsigned_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_U16:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U16,
                                                                   JAK_CARBON_COLUMN_TYPE_U16,
                                                                   u16, unsigned_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_U32:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U32,
                                                                   JAK_CARBON_COLUMN_TYPE_U32,
                                                                   u32, unsigned_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_U64:
                                        if (is_root) {
                                                insert_into_array(ins, elem, unsigned, unsigned_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_U64,
                                                                   JAK_CARBON_COLUMN_TYPE_U64,
                                                                   u64, unsigned_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_I8:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I8,
                                                                   JAK_CARBON_COLUMN_TYPE_I8,
                                                                   i8, signed_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_I16:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I16,
                                                                   JAK_CARBON_COLUMN_TYPE_I16,
                                                                   u16, signed_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_I32:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I32,
                                                                   JAK_CARBON_COLUMN_TYPE_I32,
                                                                   u32, signed_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_I64:
                                        if (is_root) {
                                                insert_into_array(ins, elem, signed, signed_integer)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_I64,
                                                                   JAK_CARBON_COLUMN_TYPE_I64,
                                                                   u64, signed_integer);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_FLOAT:
                                        if (is_root) {
                                                insert_into_array(ins, elem, float, float_number)
                                        } else {
                                                insert_into_column(ins, elem, CARBON_FIELD_NUMBER_FLOAT,
                                                                   JAK_CARBON_COLUMN_TYPE_FLOAT,
                                                                   float, float_number);
                                        }
                                        break;
                                case JAK_JSON_LIST_FIXED_NULL: {
                                        jak_u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                        if (is_root) {
                                                for (jak_u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        jak_carbon_insert_null(ins);
                                                }
                                        } else {
                                                jak_carbon_insert_array_state state;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_array_begin(&state,
                                                                                                                ins,
                                                                                                                approx_cap_nbytes);
                                                for (jak_u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        jak_carbon_insert_null(array_ins);
                                                }
                                                jak_carbon_insert_array_end(&state);
                                        }
                                }
                                        break;
                                case JAK_JSON_LIST_FIXED_BOOLEAN: {
                                        if (is_root) {
                                                for (jak_u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        jak_json_element *array_elem = JAK_VECTOR_GET(
                                                                &elem->value.value.array->elements.elements, i,
                                                                jak_json_element);
                                                        if (array_elem->value.value_type == JAK_JSON_VALUE_TRUE) {
                                                                jak_carbon_insert_true(ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_FALSE) {
                                                                jak_carbon_insert_false(ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {
                                                                jak_carbon_insert_null(ins);
                                                        } else {
                                                                JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                        } else {
                                                jak_carbon_insert_column_state state;
                                                jak_u64 cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                                jak_carbon_insert *array_ins = jak_carbon_insert_column_begin(&state,
                                                                                                                 ins,
                                                                                                                 JAK_CARBON_COLUMN_TYPE_BOOLEAN,
                                                                                                                 cap_nbytes);
                                                for (jak_u32 i = 0;
                                                     i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                        jak_json_element *array_elem = JAK_VECTOR_GET(
                                                                &elem->value.value.array->elements.elements, i,
                                                                jak_json_element);
                                                        if (array_elem->value.value_type == JAK_JSON_VALUE_TRUE) {
                                                                jak_carbon_insert_true(array_ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_FALSE) {
                                                                jak_carbon_insert_false(array_ins);
                                                        } else if (array_elem->value.value_type == JAK_JSON_VALUE_NULL) {
                                                                jak_carbon_insert_null(array_ins);
                                                        } else {
                                                                JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                                jak_carbon_insert_column_end(&state);
                                        }
                                }
                                        break;
                                default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                                        break;
                        }
                }
                        break;
                case JAK_JSON_VALUE_STRING:
                        jak_carbon_insert_string(ins, elem->value.value.string->value);
                        break;
                case JAK_JSON_VALUE_NUMBER:
                        switch (elem->value.value.number->value_type) {
                                case JAK_JSON_NUMBER_FLOAT:
                                        jak_carbon_insert_float(ins, elem->value.value.number->value.float_number);
                                        break;
                                case JAK_JSON_NUMBER_UNSIGNED:
                                        jak_carbon_insert_unsigned(ins, elem->value.value.number->value.unsigned_integer);
                                        break;
                                case JAK_JSON_NUMBER_SIGNED:
                                        jak_carbon_insert_signed(ins, elem->value.value.number->value.signed_integer);
                                        break;
                                default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                                        break;
                        }
                        break;
                case JAK_JSON_VALUE_TRUE:
                        jak_carbon_insert_true(ins);
                        break;
                case JAK_JSON_VALUE_FALSE:
                        jak_carbon_insert_false(ins);
                        break;
                case JAK_JSON_VALUE_NULL:
                        jak_carbon_insert_null(ins);
                        break;
                default: JAK_ERROR_PRINT(JAK_ERR_UNSUPPORTEDTYPE)
                        break;
        }
}

bool jak_carbon_int_from_json(jak_carbon *doc, const jak_json *data,
                          jak_carbon_key_e key_type, const void *primary_key, int mode)
{
        JAK_UNUSED(data)
        JAK_UNUSED(primary_key)

        jak_carbon_new context;
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, doc, key_type, mode);
        int_carbon_from_json_elem(ins, data->element, true);

        jak_carbon_create_end(&context);

        return true;
}

static void marker_insert(jak_memfile *memfile, jak_u8 marker)
{
        /* check whether marker can be written, otherwise make space for it */
        char c = *jak_memfile_peek(memfile, sizeof(jak_u8));
        if (c != 0) {
                jak_memfile_inplace_insert(memfile, sizeof(jak_u8));
        }
        jak_memfile_write(memfile, &marker, sizeof(jak_u8));
}

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it)
{
        jak_carbon_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_array_end, &it->memfile, CARBON_MARRAY_END);
}

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it)
{
        jak_carbon_int_field_auto_close(&it->field.value.data);
        return is_slot_occupied(is_empty_slot, is_object_end, &it->memfile, CARBON_MOBJECT_END);
}

static bool is_slot_occupied(bool *is_empty_slot, bool *is_end_reached, jak_memfile *file, jak_u8 end_marker)
{
        JAK_ERROR_IF_NULL(file);
        char c = *jak_memfile_peek(file, 1);
        bool is_empty = c == 0, is_end = c == end_marker;
        JAK_OPTIONAL_SET(is_empty_slot, is_empty)
        JAK_OPTIONAL_SET(is_end_reached, is_end)
        if (!is_empty && !is_end) {
                return true;
        } else {
                return false;
        }
}

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, jak_carbon_object_it *it)
{
        if (object_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                jak_carbon_int_object_it_prop_skip(it);
                return true;
        } else {
                return false;
        }
}

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it)
{
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                jak_carbon_int_array_it_field_type_read(it);
                jak_carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}