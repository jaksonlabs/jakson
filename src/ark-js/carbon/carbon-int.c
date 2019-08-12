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

#include <ark-js/shared/stdx/varuint.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-media.h>
#include <ark-js/carbon/carbon-int.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-object-it.h>
#include <ark-js/carbon/carbon-key.h>
#include <ark-js/carbon/carbon-revision.h>
#include <ark-js/shared/json/json.h>

static void marker_insert(struct memfile *memfile, u8 marker);

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it);

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, struct carbon_object_it *it);

static bool is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct memfile *file, u8 end_marker);

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it);

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct carbon_object_it *it);

static void int_carbon_from_json_elem(struct carbon_insert *ins, const struct json_element *elem);

static void int_insert_prop_object(struct carbon_insert *oins, struct json_object_t *obj);

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

bool carbon_int_insert_object(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);
        insert_embedded_container(memfile, CARBON_MARKER_OBJECT_BEGIN, CARBON_MARKER_OBJECT_END, nbytes);
        return true;
}

bool carbon_int_insert_array(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);
        insert_embedded_container(memfile, CARBON_MARKER_ARRAY_BEGIN, CARBON_MARKER_ARRAY_END, nbytes);
        return true;
}

bool carbon_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum carbon_column_type type,
                              size_t capactity)
{
        error_if_null(memfile_in);
        error_if_null(err_in);

        ark_declare_and_init(enum carbon_field_type, column_type)

        switch (type) {
                case CARBON_COLUMN_TYPE_BOOLEAN:
                        column_type = CARBON_FIELD_TYPE_COLUMN_BOOLEAN;
                        break;
                case CARBON_COLUMN_TYPE_U8:
                        column_type = CARBON_FIELD_TYPE_COLUMN_U8;
                        break;
                case CARBON_COLUMN_TYPE_U16:
                        column_type = CARBON_FIELD_TYPE_COLUMN_U16;
                        break;
                case CARBON_COLUMN_TYPE_U32:
                        column_type = CARBON_FIELD_TYPE_COLUMN_U32;
                        break;
                case CARBON_COLUMN_TYPE_U64:
                        column_type = CARBON_FIELD_TYPE_COLUMN_U64;
                        break;
                case CARBON_COLUMN_TYPE_I8:
                        column_type = CARBON_FIELD_TYPE_COLUMN_I8;
                        break;
                case CARBON_COLUMN_TYPE_I16:
                        column_type = CARBON_FIELD_TYPE_COLUMN_I16;
                        break;
                case CARBON_COLUMN_TYPE_I32:
                        column_type = CARBON_FIELD_TYPE_COLUMN_I32;
                        break;
                case CARBON_COLUMN_TYPE_I64:
                        column_type = CARBON_FIELD_TYPE_COLUMN_I64;
                        break;
                case CARBON_COLUMN_TYPE_FLOAT:
                        column_type = CARBON_FIELD_TYPE_COLUMN_FLOAT;
                        break;
                default:
                        error_with_details(err_in, ARK_ERR_BADTYPE, "carbon column supports fixed-length types only")
        }

        memfile_ensure_space(memfile_in, sizeof(u8));
        marker_insert(memfile_in, column_type);

        u32 num_elements = 0;
        u32 cap_elements = capactity;

        memfile_write_varuint(memfile_in, num_elements);
        memfile_write_varuint(memfile_in, cap_elements);

        offset_t payload_begin = memfile_tell(memfile_in);

        size_t type_size = carbon_int_get_type_value_size(column_type);

        size_t nbytes = capactity * type_size;
        memfile_ensure_space(memfile_in, nbytes + sizeof(u8) + 2 * sizeof(u32));

        /* seek to first entry in column */
        memfile_seek(memfile_in, payload_begin);

        return true;
}

size_t carbon_int_get_type_size_encoded(enum carbon_field_type type)
{
        size_t type_size = sizeof(media_type_t); /* at least the media type marker is required */
        switch (type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                        /* only media type marker is required */
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                        type_size += sizeof(u8);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                        type_size += sizeof(u16);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                        type_size += sizeof(u32);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                        type_size += sizeof(u64);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        type_size += sizeof(float);
                        break;
                default: error_print(ARK_ERR_INTERNALERR);
                        return 0;
        }
        return type_size;
}

size_t carbon_int_get_type_value_size(enum carbon_field_type type)
{
        switch (type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        return sizeof(media_type_t); /* these constant values are determined by their media type markers */
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_COLUMN_U8:
                case CARBON_FIELD_TYPE_COLUMN_I8:
                        return sizeof(u8);
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_COLUMN_U16:
                case CARBON_FIELD_TYPE_COLUMN_I16:
                        return sizeof(u16);
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_COLUMN_U32:
                case CARBON_FIELD_TYPE_COLUMN_I32:
                        return sizeof(u32);
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                case CARBON_FIELD_TYPE_COLUMN_U64:
                case CARBON_FIELD_TYPE_COLUMN_I64:
                        return sizeof(u64);
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        return sizeof(float);
                default: error_print(ARK_ERR_INTERNALERR);
                        return 0;
        }
}

bool carbon_int_array_it_next(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it)
{
        if (carbon_int_array_it_refresh(is_empty_slot, is_array_end, it)) {
                carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_next(bool *is_empty_slot, bool *is_object_end, struct carbon_object_it *it)
{
        if (carbon_int_object_it_refresh(is_empty_slot, is_object_end, it)) {
                carbon_int_object_it_prop_value_skip(it);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, struct carbon_object_it *it)
{
        error_if_null(it);
        if (object_it_is_slot_occupied(is_empty_slot, is_object_end, it)) {
                carbon_int_object_it_prop_key_access(it);
                carbon_int_field_data_access(&it->memfile, &it->err, &it->field_access);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_object_it_prop_key_access(struct carbon_object_it *it)
{
        error_if_null(it)
        //memfile_skip(&it->memfile, sizeof(media_type_t));

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        it->key = memfile_peek(&it->memfile, it->key_len);
        memfile_skip(&it->memfile, it->key_len);
        it->value_off = memfile_tell(&it->memfile);
        it->field_access.it_field_type = *ARK_MEMFILE_PEEK(&it->memfile, u8);

        return true;
}

bool carbon_int_object_it_prop_key_skip(struct carbon_object_it *it)
{
        error_if_null(it)
        memfile_skip(&it->memfile, sizeof(media_type_t));

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, it->key_len);

        it->value_off = memfile_tell(&it->memfile);
        it->field_access.it_field_type = *ARK_MEMFILE_READ_TYPE(&it->memfile, u8);
        return true;
}

bool carbon_int_object_it_prop_value_skip(struct carbon_object_it *it)
{
        error_if_null(it)
        memfile_seek(&it->memfile, it->value_off);
        return carbon_field_skip(&it->memfile);
}

bool carbon_int_object_it_prop_skip(struct carbon_object_it *it)
{
        error_if_null(it)

        it->key_len = memfile_read_varuint(NULL, &it->memfile);
        memfile_skip(&it->memfile, it->key_len);

        return carbon_field_skip(&it->memfile);
}

bool carbon_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, struct carbon_object_it *it)
{
        while (object_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool carbon_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it)
{
        while (array_it_next_no_load(is_empty_slot, is_array_end, it)) {}
        return true;
}

bool carbon_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it)
{
        error_if_null(it);
        carbon_int_field_access_drop(&it->field_access);
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_array_it_field_type_read(it);
                carbon_int_field_data_access(&it->memfile, &it->err, &it->field_access);
                return true;
        } else {
                return false;
        }
}

bool carbon_int_array_it_field_type_read(struct carbon_array_it *it)
{
        error_if_null(it)
        error_if(memfile_remain_size(&it->memfile) < 1, &it->err, ARK_ERR_ILLEGALOP);
        memfile_save_position(&it->memfile);
        u8 media_type = *memfile_read(&it->memfile, 1);
        error_if(media_type == 0, &it->err, ARK_ERR_NOTFOUND)
        error_if(media_type == CARBON_MARKER_ARRAY_END, &it->err, ARK_ERR_OUTOFBOUNDS)
        it->field_access.it_field_type = media_type;
        memfile_restore_position(&it->memfile);
        return true;
}

bool carbon_int_field_data_access(struct memfile *file, struct err *err, struct field_access *field_access)
{
        memfile_save_position(file);
        memfile_skip(file, sizeof(media_type_t));

        switch (field_access->it_field_type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        break;
                case CARBON_FIELD_TYPE_STRING: {
                        u8 nbytes;
                        varuint_t len = (varuint_t) memfile_peek(file, 1);
                        field_access->it_field_len = varuint_read(&nbytes, len);

                        memfile_skip(file, nbytes);
                }
                        break;
                case CARBON_FIELD_TYPE_BINARY: {
                        /* read mime type with variable-length integer type */
                        u64 mime_type_id = memfile_read_varuint(NULL, file);

                        field_access->it_mime_type = carbon_media_mime_type_by_id(mime_type_id);
                        field_access->it_mime_type_strlen = strlen(field_access->it_mime_type);

                        /* read blob length */
                        field_access->it_field_len = memfile_read_varuint(NULL, file);

                        /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                        /* read mime type string */
                        field_access->it_mime_type_strlen = memfile_read_varuint(NULL, file);
                        field_access->it_mime_type = memfile_read(file, field_access->it_mime_type_strlen);

                        /* read blob length */
                        field_access->it_field_len = memfile_read_varuint(NULL, file);

                        /* the memfile points now to the actual blob data, which is used by the iterator to set the field */
                }
                        break;
                case CARBON_FIELD_TYPE_ARRAY:
                        carbon_int_field_access_create(field_access);
                        field_access->nested_array_it_is_created = true;
                        carbon_array_it_create(field_access->nested_array_it, file, err,
                                               memfile_tell(file) - sizeof(u8));
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8:
                case CARBON_FIELD_TYPE_COLUMN_U16:
                case CARBON_FIELD_TYPE_COLUMN_U32:
                case CARBON_FIELD_TYPE_COLUMN_U64:
                case CARBON_FIELD_TYPE_COLUMN_I8:
                case CARBON_FIELD_TYPE_COLUMN_I16:
                case CARBON_FIELD_TYPE_COLUMN_I32:
                case CARBON_FIELD_TYPE_COLUMN_I64:
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        carbon_int_field_access_create(field_access);
                        carbon_column_it_create(field_access->nested_column_it, file, err,
                                                memfile_tell(file) - sizeof(u8));
                }
                        break;
                case CARBON_FIELD_TYPE_OBJECT:
                        carbon_int_field_access_create(field_access);
                        field_access->nested_object_it_is_created = true;
                        carbon_object_it_create(field_access->nested_object_it, file, err,
                                                memfile_tell(file) - sizeof(u8));
                        break;
                default: error(err, ARK_ERR_CORRUPTED)
                        return false;
        }

        field_access->it_field_data = memfile_peek(file, 1);
        memfile_restore_position(file);
        return true;
}

offset_t carbon_int_column_get_payload_off(struct carbon_column_it *it)
{
        memfile_save_position(&it->memfile);
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        memfile_skip_varuint(&it->memfile); // skip num of elements
        memfile_skip_varuint(&it->memfile); // skip capacity of elements
        offset_t result = memfile_tell(&it->memfile);
        memfile_restore_position(&it->memfile);
        return result;
}

offset_t carbon_int_payload_after_header(struct carbon *doc)
{
        offset_t result = 0;
        enum carbon_key_type key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        if (likely(carbon_key_skip(&key_type, &doc->memfile))) {
                if (key_type != CARBON_KEY_NOKEY) {
                        carbon_revision_skip(&doc->memfile);
                }
                result = memfile_tell(&doc->memfile);
        }

        memfile_restore_position(&doc->memfile);

        return result;
}

u64 carbon_int_header_get_rev(struct carbon *doc)
{
        assert(doc);
        u64 rev = 0;
        enum carbon_key_type key_type;

        memfile_save_position(&doc->memfile);
        memfile_seek(&doc->memfile, 0);

        carbon_key_skip(&key_type, &doc->memfile);
        if (key_type != CARBON_KEY_NOKEY) {
                carbon_revision_read(&rev, &doc->memfile);
        }

        memfile_restore_position(&doc->memfile);
        return rev;
}

void carbon_int_history_push(struct vector ofType(offset_t) *vec, offset_t off)
{
        assert(vec);
        vec_push(vec, &off, 1);
}

void carbon_int_history_clear(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        vec_clear(vec);
}

offset_t carbon_int_history_pop(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        assert(carbon_int_history_has(vec));
        return *(offset_t *) vec_pop(vec);
}

offset_t carbon_int_history_peek(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        assert(carbon_int_history_has(vec));
        return *(offset_t *) vec_peek(vec);
}

bool carbon_int_history_has(struct vector ofType(offset_t) *vec)
{
        assert(vec);
        return !vec_is_empty(vec);
}

bool carbon_int_field_access_create(struct field_access *field)
{
        field->nested_array_it_is_created = false;
        field->nested_array_it_accessed = false;
        field->nested_object_it_is_created = false;
        field->nested_object_it_accessed = false;
        field->nested_column_it_is_created = false;
        field->nested_array_it = ark_malloc(sizeof(struct carbon_array_it));
        field->nested_object_it = ark_malloc(sizeof(struct carbon_object_it));
        field->nested_column_it = ark_malloc(sizeof(struct carbon_column_it));
        return true;
}

bool carbon_int_field_access_clone(struct field_access *dst, struct field_access *src)
{
        error_if_null(dst)
        error_if_null(src)

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
        dst->nested_array_it = ark_malloc(sizeof(struct carbon_array_it));
        dst->nested_object_it = ark_malloc(sizeof(struct carbon_object_it));
        dst->nested_column_it = ark_malloc(sizeof(struct carbon_column_it));

        if (carbon_int_field_access_object_it_opened(src)) {
                carbon_object_it_clone(dst->nested_object_it, src->nested_object_it);
        } else if (carbon_int_field_access_column_it_opened(src)) {
                carbon_column_it_clone(dst->nested_column_it, src->nested_column_it);
        } else if (carbon_int_field_access_array_it_opened(src)) {
                carbon_array_it_clone(dst->nested_array_it, src->nested_array_it);
        }
        return true;
}

bool carbon_int_field_access_drop(struct field_access *field)
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

bool carbon_int_field_access_object_it_opened(struct field_access *field)
{
        assert(field);
        return field->nested_object_it_is_created;//field->nested_object_it != 0;
}

bool carbon_int_field_access_array_it_opened(struct field_access *field)
{
        assert(field);
        return field->nested_array_it_is_created != 0;
}

bool carbon_int_field_access_column_it_opened(struct field_access *field)
{
        assert(field);
        return field->nested_column_it_is_created != 0;
}

void carbon_int_auto_close_nested_array_it(struct field_access *field)
{
        if (carbon_int_field_access_array_it_opened(field)) {
                carbon_array_it_drop(field->nested_array_it);
                ark_zero_memory(field->nested_array_it, sizeof(struct carbon_array_it));
        }
}

void carbon_int_auto_close_nested_object_it(struct field_access *field)
{
        if (carbon_int_field_access_object_it_opened(field)) {
                carbon_object_it_drop(field->nested_object_it);
                ark_zero_memory(field->nested_object_it, sizeof(struct carbon_object_it));
        }
}

void carbon_int_auto_close_nested_column_it(struct field_access *field)
{
        if (carbon_int_field_access_column_it_opened(field)) {
                ark_zero_memory(field->nested_column_it, sizeof(struct carbon_column_it));
        }
}

bool carbon_int_field_auto_close(struct field_access *field)
{
        error_if_null(field)
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

bool carbon_int_field_access_field_type(enum carbon_field_type *type, struct field_access *field)
{
        error_if_null(type)
        error_if_null(field)
        *type = field->it_field_type;
        return true;
}

bool carbon_int_field_access_u8_value(u8 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_U8, err, ARK_ERR_TYPEMISMATCH);
        *value = *(u8 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u16_value(u16 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_U16, err, ARK_ERR_TYPEMISMATCH);
        *value = *(u16 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u32_value(u32 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_U32, err, ARK_ERR_TYPEMISMATCH);
        *value = *(u32 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_u64_value(u64 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_U64, err, ARK_ERR_TYPEMISMATCH);
        *value = *(u64 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i8_value(i8 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_I8, err, ARK_ERR_TYPEMISMATCH);
        *value = *(i8 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i16_value(i16 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_I16, err, ARK_ERR_TYPEMISMATCH);
        *value = *(i16 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i32_value(i32 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_I32, err, ARK_ERR_TYPEMISMATCH);
        *value = *(i32 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_i64_value(i64 *value, struct field_access *field, struct err *err)
{
        error_if_null(value)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_I64, err, ARK_ERR_TYPEMISMATCH);
        *value = *(i64 *) field->it_field_data;
        return true;
}

bool carbon_int_field_access_float_value(bool *is_null_in, float *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_NUMBER_FLOAT, err, ARK_ERR_TYPEMISMATCH);
        float read_value = *(float *) field->it_field_data;
        ark_optional_set(value, read_value);
        ark_optional_set(is_null_in, is_null_float(read_value));

        return true;
}

bool carbon_int_field_access_signed_value(bool *is_null_in, i64 *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_TYPE_NUMBER_I8: {
                        i8 read_value;
                        carbon_int_field_access_i8_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_i8(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I16: {
                        i16 read_value;
                        carbon_int_field_access_i16_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_i16(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I32: {
                        i32 read_value;
                        carbon_int_field_access_i32_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_i32(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I64: {
                        i64 read_value;
                        carbon_int_field_access_i64_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_i64(read_value));
                }
                        break;
                default: error(err, ARK_ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

bool carbon_int_field_access_unsigned_value(bool *is_null_in, u64 *value, struct field_access *field, struct err *err)
{
        error_if_null(field)
        switch (field->it_field_type) {
                case CARBON_FIELD_TYPE_NUMBER_U8: {
                        u8 read_value;
                        carbon_int_field_access_u8_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_u8(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U16: {
                        u16 read_value;
                        carbon_int_field_access_u16_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_u16(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U32: {
                        u32 read_value;
                        carbon_int_field_access_u32_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_u32(read_value));
                }
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U64: {
                        u64 read_value;
                        carbon_int_field_access_u64_value(&read_value, field, err);
                        ark_optional_set(value, read_value);
                        ark_optional_set(is_null_in, is_null_u64(read_value));
                }
                        break;
                default: error(err, ARK_ERR_TYPEMISMATCH);
                        return false;
        }
        return true;
}

const char *carbon_int_field_access_string_value(u64 *strlen, struct field_access *field, struct err *err)
{
        error_if_null(strlen);
        error_if_and_return(field == NULL, err, ARK_ERR_NULLPTR, NULL);
        error_if(field->it_field_type != CARBON_FIELD_TYPE_STRING, err, ARK_ERR_TYPEMISMATCH);
        *strlen = field->it_field_len;
        return field->it_field_data;
}

bool carbon_int_field_access_binary_value(struct carbon_binary *out, struct field_access *field, struct err *err)
{
        error_if_null(out)
        error_if_null(field)
        error_if(field->it_field_type != CARBON_FIELD_TYPE_BINARY &&
                 field->it_field_type != CARBON_FIELD_TYPE_BINARY_CUSTOM,
                 err, ARK_ERR_TYPEMISMATCH);
        out->blob = field->it_field_data;
        out->blob_len = field->it_field_len;
        out->mime_type = field->it_mime_type;
        out->mime_type_strlen = field->it_mime_type_strlen;
        return true;
}

struct carbon_array_it *carbon_int_field_access_array_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, ARK_ERR_NULLPTR);
        error_if(field->it_field_type != CARBON_FIELD_TYPE_ARRAY, err, ARK_ERR_TYPEMISMATCH);
        field->nested_array_it_accessed = true;
        return field->nested_array_it;
}

struct carbon_object_it *carbon_int_field_access_object_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, ARK_ERR_NULLPTR);
        error_if(field->it_field_type != CARBON_FIELD_TYPE_OBJECT, err, ARK_ERR_TYPEMISMATCH);
        field->nested_object_it_accessed = true;
        return field->nested_object_it;
}

struct carbon_column_it *carbon_int_field_access_column_value(struct field_access *field, struct err *err)
{
        error_print_if(!field, ARK_ERR_NULLPTR);
        error_if(field->it_field_type != CARBON_FIELD_TYPE_COLUMN_U8 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_U16 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_U32 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_U64 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_I8 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_I16 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_I32 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_I64 &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_FLOAT &&
                 field->it_field_type != CARBON_FIELD_TYPE_COLUMN_BOOLEAN, err, ARK_ERR_TYPEMISMATCH);
        return field->nested_column_it;
}

bool carbon_int_field_remove(struct memfile *memfile, struct err *err, enum carbon_field_type type)
{
        assert((enum carbon_field_type) *memfile_peek(memfile, sizeof(u8)) == type);
        offset_t start_off = memfile_tell(memfile);
        memfile_skip(memfile, sizeof(u8));
        size_t rm_nbytes = sizeof(u8); /* at least the type marker must be removed */
        switch (type) {
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                        /* nothing to do */
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                        rm_nbytes += sizeof(u8);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                        rm_nbytes += sizeof(u16);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                        rm_nbytes += sizeof(u32);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                        rm_nbytes += sizeof(u64);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        rm_nbytes += sizeof(float);
                        break;
                case CARBON_FIELD_TYPE_STRING: {
                        u8 len_nbytes;  /* number of bytes used to store string length */
                        u64 str_len; /* the number of characters of the string field */

                        str_len = memfile_read_varuint(&len_nbytes, memfile);

                        rm_nbytes += len_nbytes + str_len;
                }
                        break;
                case CARBON_FIELD_TYPE_BINARY: {
                        u8 mime_type_nbytes; /* number of bytes for mime type */
                        u8 blob_length_nbytes; /* number of bytes to store blob length */
                        u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes used for mime type id */
                        memfile_read_varuint(&mime_type_nbytes, memfile);

                        /* get bytes used for blob length info */
                        blob_nbytes = memfile_read_varuint(&blob_length_nbytes, memfile);

                        rm_nbytes += mime_type_nbytes + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_TYPE_BINARY_CUSTOM: {
                        u8 custom_type_strlen_nbytes; /* number of bytes for type name string length info */
                        u8 custom_type_strlen; /* number of characters to encode type name string */
                        u8 blob_length_nbytes; /* number of bytes to store blob length */
                        u64 blob_nbytes; /* number of bytes to store actual blob data */

                        /* get bytes for custom type string len, and the actual length */
                        custom_type_strlen = memfile_read_varuint(&custom_type_strlen_nbytes, memfile);
                        memfile_skip(memfile, custom_type_strlen);

                        /* get bytes used for blob length info */
                        blob_nbytes = memfile_read_varuint(&blob_length_nbytes, memfile);

                        rm_nbytes += custom_type_strlen_nbytes + custom_type_strlen + blob_length_nbytes + blob_nbytes;
                }
                        break;
                case CARBON_FIELD_TYPE_ARRAY: {
                        struct carbon_array_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_array_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_array_it_fast_forward(&it);
                        offset_t end_off = carbon_array_it_tell(&it);
                        carbon_array_it_drop(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8:
                case CARBON_FIELD_TYPE_COLUMN_U16:
                case CARBON_FIELD_TYPE_COLUMN_U32:
                case CARBON_FIELD_TYPE_COLUMN_U64:
                case CARBON_FIELD_TYPE_COLUMN_I8:
                case CARBON_FIELD_TYPE_COLUMN_I16:
                case CARBON_FIELD_TYPE_COLUMN_I32:
                case CARBON_FIELD_TYPE_COLUMN_I64:
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        struct carbon_column_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_column_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_column_it_fast_forward(&it);
                        offset_t end_off = carbon_column_it_tell(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                case CARBON_FIELD_TYPE_OBJECT: {
                        struct carbon_object_it it;

                        offset_t begin_off = memfile_tell(memfile);
                        carbon_object_it_create(&it, memfile, err, begin_off - sizeof(u8));
                        carbon_object_it_fast_forward(&it);
                        offset_t end_off = carbon_object_it_tell(&it);
                        carbon_object_it_drop(&it);

                        assert(begin_off < end_off);
                        rm_nbytes += (end_off - begin_off);
                }
                        break;
                default: error(err, ARK_ERR_INTERNALERR)
                        return false;
        }
        memfile_seek(memfile, start_off);
        memfile_inplace_remove(memfile, rm_nbytes);

        return true;
}

static void int_insert_array_array(struct carbon_insert *array_ins, struct json_array *array)
{
        struct carbon_insert_array_state state;
        struct carbon_insert *sub_ins = carbon_insert_array_begin(&state, array_ins,
                array->elements.elements.num_elems * 256);
        for (u32 i = 0; i < array->elements.elements.num_elems; i++) {
                const struct json_element *elem = vec_get(&array->elements.elements, i, struct json_element);
                int_carbon_from_json_elem(sub_ins, elem);
        }
        carbon_insert_array_end(&state);
}

static void int_insert_array_string(struct carbon_insert *array_ins, struct json_string *string)
{
        carbon_insert_string(array_ins, string->value);
}

static void int_insert_array_number(struct carbon_insert *array_ins, struct json_number *number)
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
                default:
                        error_print(ARK_ERR_UNSUPPORTEDTYPE)
        }
}

static void int_insert_array_true(struct carbon_insert *array_ins)
{
        carbon_insert_true(array_ins);
}

static void int_insert_array_false(struct carbon_insert *array_ins)
{
        carbon_insert_false(array_ins);
}

static void int_insert_array_null(struct carbon_insert *array_ins)
{
        carbon_insert_null(array_ins);
}

static void int_insert_array_elements(struct carbon_insert *array_ins, struct json_array *array)
{
        for (u32 i = 0; i < array->elements.elements.num_elems; i++) {
                struct json_element *elem = vec_get(&array->elements.elements, i, struct json_element);
                switch(elem->value.value_type) {
                        case JSON_VALUE_OBJECT: {
                                struct carbon_insert_object_state state;
                                struct carbon_insert *sub_obj = carbon_insert_object_begin(&state, array_ins,
                                        elem->value.value.object->value->members.num_elems * 256);
                                int_insert_prop_object(sub_obj, elem->value.value.object);
                                carbon_insert_object_end(&state);
                        } break;
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
                        default:
                                error_print(ARK_ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

#define insert_into_column(ins, elem, field_type, column_type, ctype, accessor)                                             \
({                                                                                                                     \
        struct carbon_insert_column_state state;                                                                       \
        u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems *                                 \
                                carbon_int_get_type_value_size(field_type);                                            \
        struct carbon_insert *cins = carbon_insert_column_begin(&state, ins,                                           \
                                                                column_type, approx_cap_nbytes);                       \
        for (u32 k = 0; k < elem->value.value.array->elements.elements.num_elems; k++) {                               \
                struct json_element *array_elem = vec_get(&elem->value.value.array->elements.elements,                 \
                                                          k, struct json_element);                                     \
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
        struct carbon_insert_column_state state;                                                                       \
        u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems *                                 \
                                carbon_int_get_type_value_size(field_type);                                            \
        struct carbon_insert *cins = carbon_insert_prop_column_begin(&state, ins, key,                                          \
                                                                column_type, approx_cap_nbytes);                       \
        for (u32 k = 0; k < prop->value.value.value.array->elements.elements.num_elems; k++) {                               \
                struct json_element *array_elem = vec_get(&prop->value.value.value.array->elements.elements,                 \
                                                          k, struct json_element);                                     \
                if (array_elem->value.value_type == JSON_VALUE_NULL) {                                                 \
                        carbon_insert_null(cins);                                                                      \
                } else {                                                                                               \
                        carbon_insert_##ctype(cins, (ctype) array_elem->value.value.number->value.accessor);           \
                }                                                                                                      \
        }                                                                                                              \
        carbon_insert_prop_column_end(&state);                                                                              \
})

static void int_insert_prop_object(struct carbon_insert *oins, struct json_object_t *obj)
{
        for (u32 i = 0; i < obj->value->members.num_elems; i++) {
                struct json_prop *prop = vec_get(&obj->value->members, i, struct json_prop);
                switch (prop->value.value.value_type) {
                        case JSON_VALUE_OBJECT: {
                                struct carbon_insert_object_state state;
                                struct carbon_insert *sub_obj = carbon_insert_prop_object_begin(&state, oins,
                                        prop->key.value, prop->value.value.value.object->value->members.num_elems * 256);
                                int_insert_prop_object(sub_obj, prop->value.value.value.object);
                                carbon_insert_prop_object_end(&state);
                        } break;
                        case JSON_VALUE_ARRAY: {
                                enum json_list_type type;
                                json_array_get_type(&type, prop->value.value.value.array);
                                switch (type) {
                                        case JSON_LIST_TYPE_EMPTY: {
                                                struct carbon_insert_array_state state;
                                                carbon_insert_prop_array_begin(&state, oins, prop->key.value, 0);
                                                carbon_insert_prop_array_end(&state);
                                        } break;
                                        case JSON_LIST_TYPE_VARIABLE_OR_NESTED: {
                                                struct carbon_insert_array_state state;
                                                u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems * 256;
                                                struct carbon_insert *array_ins = carbon_insert_prop_array_begin(&state, oins,
                                                                                         prop->key.value, approx_cap_nbytes);
                                                int_insert_array_elements(array_ins, prop->value.value.value.array);
                                                carbon_insert_prop_array_end(&state);
                                        } break;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                        CARBON_FIELD_TYPE_NUMBER_U8, CARBON_COLUMN_TYPE_U8,
                                                        u8, unsigned_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_U16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_U16, CARBON_COLUMN_TYPE_U16,
                                                                        u16, unsigned_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_U32, CARBON_COLUMN_TYPE_U32,
                                                                        u32, unsigned_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_U64, CARBON_COLUMN_TYPE_U64,
                                                                        u64, unsigned_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_I8, CARBON_COLUMN_TYPE_I8,
                                                                        i8, signed_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_I16:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_I16, CARBON_COLUMN_TYPE_I16,
                                                                        i16, signed_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_I32, CARBON_COLUMN_TYPE_I32,
                                                                        i32, signed_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_I64, CARBON_COLUMN_TYPE_I64,
                                                                        i64, signed_integer);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_FLOAT:
                                                prop_insert_into_column(oins, prop, prop->key.value,
                                                                        CARBON_FIELD_TYPE_NUMBER_FLOAT, CARBON_COLUMN_TYPE_FLOAT,
                                                                        float, float_number);
                                                break;
                                        case JSON_LIST_TYPE_FIXED_NULL: {
                                                struct carbon_insert_array_state state;
                                                u64 approx_cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                struct carbon_insert *array_ins = carbon_insert_prop_array_begin(&state, oins, prop->key.value,
                                                                                                            approx_cap_nbytes);
                                                for (u32 k = 0; k < prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        carbon_insert_null(array_ins);
                                                }
                                                carbon_insert_prop_array_end(&state);
                                        } break;
                                        case JSON_LIST_TYPE_FIXED_BOOLEAN: {
                                                struct carbon_insert_column_state state;
                                                u64 cap_nbytes = prop->value.value.value.array->elements.elements.num_elems;
                                                struct carbon_insert *array_ins = carbon_insert_prop_column_begin(&state, oins,
                                                                                                             prop->key.value,
                                                                                                             CARBON_COLUMN_TYPE_BOOLEAN, cap_nbytes);
                                                for (u32 k = 0; k < prop->value.value.value.array->elements.elements.num_elems; k++) {
                                                        struct json_element *array_elem = vec_get(
                                                                &prop->value.value.value.array->elements.elements, i,
                                                                struct json_element);
                                                        if (array_elem->value.value_type == JSON_VALUE_TRUE) {
                                                                carbon_insert_true(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_FALSE) {
                                                                carbon_insert_false(array_ins);
                                                        } else if (array_elem->value.value_type == JSON_VALUE_NULL) {
                                                                carbon_insert_null(array_ins);
                                                        } else {
                                                                error_print(ARK_ERR_UNSUPPORTEDTYPE);
                                                        }
                                                }
                                                carbon_insert_prop_column_end(&state);
                                        } break;
                                        default:
                                                error_print(ARK_ERR_UNSUPPORTEDTYPE)
                                                break;
                                }
                        } break;
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
                                        default:
                                                error_print(ARK_ERR_UNSUPPORTEDTYPE);
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
                        default:
                                error_print(ARK_ERR_UNSUPPORTEDTYPE)
                                break;
                }
        }
}

static void int_carbon_from_json_elem(struct carbon_insert *ins, const struct json_element *elem)
{
        switch (elem->value.value_type) {
                case JSON_VALUE_OBJECT: {
                        struct carbon_insert_object_state state;
                        struct carbon_insert *oins = carbon_insert_object_begin(&state, ins,
                                elem->value.value.object->value->members.num_elems * 256);
                        int_insert_prop_object(oins, elem->value.value.object);
                        carbon_insert_object_end(&state);
                } break;
                case JSON_VALUE_ARRAY:
                {
                        enum json_list_type type;
                        json_array_get_type(&type, elem->value.value.array);
                        switch (type) {
                                case JSON_LIST_TYPE_EMPTY: {
                                        struct carbon_insert_array_state state;
                                        carbon_insert_array_begin(&state, ins, 0);
                                        carbon_insert_array_end(&state);
                                } break;
                                case JSON_LIST_TYPE_VARIABLE_OR_NESTED: {
                                        struct carbon_insert_array_state state;
                                        u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems * 256;
                                        struct carbon_insert *array_ins = carbon_insert_array_begin(&state, ins,
                                                approx_cap_nbytes);
                                        int_insert_array_elements(array_ins, elem->value.value.array);
                                        carbon_insert_array_end(&state);
                                } break;
                                case JSON_LIST_TYPE_FIXED_U8:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_U8, CARBON_COLUMN_TYPE_U8,
                                                u8, unsigned_integer);
                                break;
                                case JSON_LIST_TYPE_FIXED_U16:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_U16, CARBON_COLUMN_TYPE_U16,
                                                           u16, unsigned_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_U32:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_U32, CARBON_COLUMN_TYPE_U32,
                                                           u32, unsigned_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_U64:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_U64, CARBON_COLUMN_TYPE_U64,
                                                           u64, unsigned_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_I8:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_I8, CARBON_COLUMN_TYPE_I8,
                                                           i8, signed_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_I16:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_I16, CARBON_COLUMN_TYPE_I16,
                                                           u16, signed_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_I32:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_I32, CARBON_COLUMN_TYPE_I32,
                                                           u32, signed_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_I64:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_I64, CARBON_COLUMN_TYPE_I64,
                                                           u64, signed_integer);
                                        break;
                                case JSON_LIST_TYPE_FIXED_FLOAT:
                                        insert_into_column(ins, elem, CARBON_FIELD_TYPE_NUMBER_FLOAT, CARBON_COLUMN_TYPE_FLOAT,
                                                           float, float_number);
                                        break;
                                case JSON_LIST_TYPE_FIXED_NULL: {
                                        struct carbon_insert_array_state state;
                                        u64 approx_cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                        struct carbon_insert *array_ins = carbon_insert_array_begin(&state, ins,
                                                                                                    approx_cap_nbytes);
                                        for (u32 i = 0; i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                carbon_insert_null(array_ins);
                                        }
                                        carbon_insert_array_end(&state);
                                } break;
                                case JSON_LIST_TYPE_FIXED_BOOLEAN: {
                                        struct carbon_insert_column_state state;
                                        u64 cap_nbytes = elem->value.value.array->elements.elements.num_elems;
                                        struct carbon_insert *array_ins = carbon_insert_column_begin(&state, ins,
                                                                            CARBON_COLUMN_TYPE_BOOLEAN, cap_nbytes);
                                        for (u32 i = 0; i < elem->value.value.array->elements.elements.num_elems; i++) {
                                                struct json_element *array_elem = vec_get(
                                                        &elem->value.value.array->elements.elements, i,
                                                        struct json_element);
                                                if (array_elem->value.value_type == JSON_VALUE_TRUE) {
                                                        carbon_insert_true(array_ins);
                                                } else if (array_elem->value.value_type == JSON_VALUE_FALSE) {
                                                        carbon_insert_false(array_ins);
                                                } else if (array_elem->value.value_type == JSON_VALUE_NULL) {
                                                        carbon_insert_null(array_ins);
                                                } else {
                                                        error_print(ARK_ERR_UNSUPPORTEDTYPE);
                                                }
                                        }
                                        carbon_insert_column_end(&state);
                                } break;
                                default:
                                        error_print(ARK_ERR_UNSUPPORTEDTYPE)
                                        break;
                        }
                } break;
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
                                default:
                                        error_print(ARK_ERR_UNSUPPORTEDTYPE)
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
                default:
                        error_print(ARK_ERR_UNSUPPORTEDTYPE)
                        break;
        }
}

bool carbon_int_from_json(struct carbon *doc, const struct json *data,
                          enum carbon_key_type key_type, const void *primary_key, int mode)
{
        unused(data)
        unused(primary_key)

        struct carbon_new context;
        struct carbon_insert *ins = carbon_create_begin(&context, doc, key_type, mode);
        int_carbon_from_json_elem(ins, data->element);

        carbon_create_end(&context);

        return true;
}

static void marker_insert(struct memfile *memfile, u8 marker)
{
        /* check whether marker can be written, otherwise make space for it */
        char c = *memfile_peek(memfile, sizeof(u8));
        if (c != 0) {
                memfile_inplace_insert(memfile, sizeof(u8));
        }
        memfile_write(memfile, &marker, sizeof(u8));
}

static bool array_it_is_slot_occupied(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_array_end, &it->memfile, CARBON_MARKER_ARRAY_END);
}

static bool object_it_is_slot_occupied(bool *is_empty_slot, bool *is_object_end, struct carbon_object_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        return is_slot_occupied(is_empty_slot, is_object_end, &it->memfile, CARBON_MARKER_OBJECT_END);
}

static bool is_slot_occupied(bool *is_empty_slot, bool *is_end_reached, struct memfile *file, u8 end_marker)
{
        error_if_null(file);
        char c = *memfile_peek(file, 1);
        bool is_empty = c == 0, is_end = c == end_marker;
        ark_optional_set(is_empty_slot, is_empty)
        ark_optional_set(is_end_reached, is_end)
        if (!is_empty && !is_end) {
                return true;
        } else {
                return false;
        }
}

static bool object_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct carbon_object_it *it)
{
        if (object_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_object_it_prop_skip(it);
                return true;
        } else {
                return false;
        }
}

static bool array_it_next_no_load(bool *is_empty_slot, bool *is_array_end, struct carbon_array_it *it)
{
        if (array_it_is_slot_occupied(is_empty_slot, is_array_end, it)) {
                carbon_int_array_it_field_type_read(it);
                carbon_field_skip(&it->memfile);
                return true;
        } else {
                return false;
        }
}