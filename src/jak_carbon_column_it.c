/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in carbon
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

#include <jak_carbon_column_it.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_media.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_int.h>

#define safe_cast(builtin_type, nvalues, it, field_type_expr)                                                          \
({                                                                                                                     \
        enum carbon_field_type type;                                                                                    \
        const void *raw = carbon_column_it_values(&type, nvalues, it);                                                  \
        error_if(!(field_type_expr), &it->err, JAK_ERR_TYPEMISMATCH);                                                  \
        (const builtin_type *) raw;                                                                                    \
})

bool carbon_column_it_create(struct jak_carbon_column_it *it, struct memfile *memfile, struct err *err,
                             offset_t column_start_offset)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->column_start_offset = column_start_offset;
        it->mod_size = 0;

        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, column_start_offset);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8) + sizeof(media_type_t), err, JAK_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != JAK_CARBON_MARKER_COLUMN_U8 &&
                              marker != JAK_CARBON_MARKER_COLUMN_U16 &&
                              marker != JAK_CARBON_MARKER_COLUMN_U32 &&
                              marker != JAK_CARBON_MARKER_COLUMN_U64 &&
                              marker != JAK_CARBON_MARKER_COLUMN_I8 &&
                              marker != JAK_CARBON_MARKER_COLUMN_I16 &&
                              marker != JAK_CARBON_MARKER_COLUMN_I32 &&
                              marker != JAK_CARBON_MARKER_COLUMN_I64 &&
                              marker != JAK_CARBON_MARKER_COLUMN_FLOAT &&
                              marker != JAK_CARBON_MARKER_COLUMN_BOOLEAN, err, JAK_ERR_ILLEGALOP,
                              "column begin marker ('(') not found");

        enum carbon_field_type type = (enum carbon_field_type) marker;
        it->type = type;

        it->num_and_capacity_start_offset = memfile_tell(&it->memfile);
        it->column_num_elements = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        it->column_capacity = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);

        carbon_column_it_rewind(it);

        return true;
}

bool carbon_column_it_clone(struct jak_carbon_column_it *dst, struct jak_carbon_column_it *src)
{
        memfile_clone(&dst->memfile, &src->memfile);
        dst->num_and_capacity_start_offset = src->num_and_capacity_start_offset;
        dst->column_start_offset = src->column_start_offset;
        error_cpy(&dst->err, &src->err);
        dst->type = src->type;
        dst->mod_size = src->mod_size;
        dst->column_capacity = src->column_capacity;
        dst->column_num_elements = src->column_num_elements;
        spin_init(&dst->lock);
        return true;
}

bool carbon_column_it_insert(struct jak_carbon_insert *inserter, struct jak_carbon_column_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return carbon_int_insert_create_for_column(inserter, it);
}

bool carbon_column_it_fast_forward(struct jak_carbon_column_it *it)
{
        error_if_null(it);
        carbon_column_it_values(NULL, NULL, it);
        return true;
}

offset_t carbon_column_it_memfilepos(struct jak_carbon_column_it *it)
{
        if (likely(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, JAK_ERR_NULLPTR);
                return 0;
        }
}

offset_t carbon_column_it_tell(struct jak_carbon_column_it *it, u32 elem_idx)
{
        if (it) {
                memfile_save_position(&it->memfile);
                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                u32 num_elements = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);
                memfile_read_uintvar_stream(NULL, &it->memfile);
                offset_t payload_start = memfile_tell(&it->memfile);
                error_if(elem_idx >= num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
                offset_t ret = payload_start + elem_idx * carbon_int_get_type_value_size(it->type);
                memfile_restore_position(&it->memfile);
                return ret;
        } else {
                error_print(JAK_ERR_NULLPTR);
                return 0;
        }
}

bool carbon_column_it_values_info(enum carbon_field_type *type, u32 *nvalues, struct jak_carbon_column_it *it)
{
        error_if_null(it);

        if (nvalues) {
                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                u32 num_elements = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);
                *nvalues = num_elements;
        }

        JAK_optional_set(type, it->type);

        return true;
}

bool carbon_column_it_value_is_null(struct jak_carbon_column_it *it, u32 pos)
{
        error_if_null(it);
        enum carbon_field_type type;
        u32 nvalues = 0;
        carbon_column_it_values_info(&type, &nvalues, it);
        error_if(pos >= nvalues, &it->err, JAK_ERR_OUTOFBOUNDS);
        switch (type) {
                case CARBON_FIELD_TYPE_COLUMN_U8:
                        return is_null_u8(carbon_column_it_u8_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_U16:
                        return is_null_u16(carbon_column_it_u16_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_U32:
                        return is_null_u32(carbon_column_it_u32_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_U64:
                        return is_null_u64(carbon_column_it_u64_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_I8:
                        return is_null_i8(carbon_column_it_i8_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_I16:
                        return is_null_i16(carbon_column_it_i16_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_I32:
                        return is_null_i32(carbon_column_it_i32_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_I64:
                        return is_null_i64(carbon_column_it_i64_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_FLOAT:
                        return is_null_float(carbon_column_it_float_values(NULL, it)[pos]);
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN:
                        return is_null_boolean(carbon_column_it_boolean_values(NULL, it)[pos]);
                default:
                        error(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
        }
}

const void *carbon_column_it_values(enum carbon_field_type *type, u32 *nvalues, struct jak_carbon_column_it *it)
{
        error_if_null(it);
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        u32 num_elements = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        u32 cap_elements = (u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        offset_t payload_start = memfile_tell(&it->memfile);

        const void *result = memfile_peek(&it->memfile, sizeof(void));

        JAK_optional_set(type, it->type);
        JAK_optional_set(nvalues, num_elements);

        u32 skip = cap_elements * carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start + skip);

        return result;
}

const u8 *carbon_column_it_boolean_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(u8, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN));
}

const u8 *carbon_column_it_u8_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(u8, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_U8));
}

const u16 *carbon_column_it_u16_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(u16, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_U16));
}

const u32 *carbon_column_it_u32_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(u32, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_U32));
}

const u64 *carbon_column_it_u64_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(u64, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_U64));
}

const i8 *carbon_column_it_i8_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(i8, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_I8));
}

const i16 *carbon_column_it_i16_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(i16, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_I16));
}

const i32 *carbon_column_it_i32_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(i32, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_I32));
}

const i64 *carbon_column_it_i64_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(i64, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_I64));
}

const float *carbon_column_it_float_values(u32 *nvalues, struct jak_carbon_column_it *it)
{
        return safe_cast(float, nvalues, it, (type == CARBON_FIELD_TYPE_COLUMN_FLOAT));
}

bool carbon_column_it_remove(struct jak_carbon_column_it *it, u32 pos)
{
        error_if_null(it);

        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
        memfile_save_position(&it->memfile);

        offset_t payload_start = carbon_int_column_get_payload_off(it);

        /* remove element */
        size_t elem_size = carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start + pos * elem_size);
        memfile_inplace_remove(&it->memfile, elem_size);

        /* add an empty element at the end to restore the column capacity property */
        memfile_seek(&it->memfile, payload_start + it->column_num_elements * elem_size);
        memfile_inplace_insert(&it->memfile, elem_size);

        /* update element counter */
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        u32 num_elems = memfile_peek_uintvar_stream(NULL, &it->memfile);
        assert(num_elems > 0);
        num_elems--;
        signed_offset_t shift = memfile_update_uintvar_stream(&it->memfile, num_elems);
        it->column_num_elements = num_elems;

        memfile_restore_position(&it->memfile);
        memfile_seek_from_here(&it->memfile, shift);

        return true;
}

bool carbon_column_it_update_set_null(struct jak_carbon_column_it *it, u32 pos)
{
        error_if_null(it)
        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        offset_t payload_start = carbon_int_column_get_payload_off(it);
        memfile_seek(&it->memfile, payload_start + pos * carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        u8 null_value = JAK_CARBON_BOOLEAN_COLUMN_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8: {
                        u8 null_value = U8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U16: {
                        u16 null_value = U16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u16));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U32: {
                        u32 null_value = U32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u32));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U64: {
                        u64 null_value = U64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u64));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I8: {
                        i8 null_value = I8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I16: {
                        i16 null_value = I16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i16));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I32: {
                        i32 null_value = I32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i32));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I64: {
                        i64 null_value = I64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i64));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                        float null_value = FLOAT_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                case CARBON_FIELD_TYPE_OBJECT:
                case CARBON_FIELD_TYPE_ARRAY:
                case CARBON_FIELD_TYPE_STRING:
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_FIELD_TYPE_BINARY:
                case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
                default:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, JAK_ERR_INTERNALERR);
                        return false;
        }

        memfile_restore_position(&it->memfile);

        return true;
}

#define push_array_element(num_values, data, data_cast_type, null_check, insert_func)                                  \
for (u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (likely(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins);                                                                               \
        } else {                                                                                                       \
                carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

#define push_array_element_wvalue(num_values, data, data_cast_type, null_check, insert_func)                           \
for (u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (likely(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins, datum);                                                                        \
        } else {                                                                                                       \
                carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

static bool rewrite_column_to_array(struct jak_carbon_column_it *it)
{
        struct jak_carbon_array_it array_it;
        struct jak_carbon_insert array_ins;

        memfile_save_position(&it->memfile);

        /* Potentially tailing space after the last ']' marker of the outer most array is used for temporary space */
        memfile_seek_to_end(&it->memfile);
        offset_t array_marker_begin = memfile_tell(&it->memfile);

        size_t capacity = it->column_num_elements * carbon_int_get_type_value_size(it->type);
        carbon_int_insert_array(&it->memfile, capacity);
        carbon_array_it_create(&array_it, &it->memfile, &it->err, array_marker_begin);
        carbon_array_it_insert_begin(&array_ins, &array_it);

        enum carbon_field_type type;
        u32 num_values;
        const void *data = carbon_column_it_values(&type, &num_values, it);
        switch (type) {
                case CARBON_FIELD_TYPE_NULL:
                        while (num_values--) {
                                carbon_insert_null(&array_ins);
                        }
                        break;
                case CARBON_FIELD_TYPE_TRUE:
                        push_array_element(num_values, data, u8, is_null_boolean, carbon_insert_true);
                        break;
                case CARBON_FIELD_TYPE_FALSE:
                        push_array_element(num_values, data, u8, is_null_boolean, carbon_insert_false);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U8:
                        push_array_element_wvalue(num_values, data, u8, is_null_u8, carbon_insert_u8);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U16:
                        push_array_element_wvalue(num_values, data, u16, is_null_u16, carbon_insert_u16);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U32:
                        push_array_element_wvalue(num_values, data, u32, is_null_u32, carbon_insert_u32);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_U64:
                        push_array_element_wvalue(num_values, data, u64, is_null_u64, carbon_insert_u64);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I8:
                        push_array_element_wvalue(num_values, data, i8, is_null_i8, carbon_insert_i8);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I16:
                        push_array_element_wvalue(num_values, data, i16, is_null_i16, carbon_insert_i16);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I32:
                        push_array_element_wvalue(num_values, data, i32, is_null_i32, carbon_insert_i32);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_I64:
                        push_array_element_wvalue(num_values, data, i64, is_null_i64, carbon_insert_i64);
                        break;
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                        push_array_element_wvalue(num_values, data, float, is_null_float, carbon_insert_float);
                        break;
                default: error(&it->err, JAK_ERR_UNSUPPORTEDTYPE);
                        return false;
        }

        carbon_array_it_insert_end(&array_ins);
        assert(array_marker_begin < carbon_array_it_memfilepos(&array_it));
        carbon_array_it_drop(&array_it);

        memfile_restore_position(&it->memfile);
        return true;
}

bool carbon_column_it_update_set_true(struct jak_carbon_column_it *it, u32 pos)
{
        error_if_null(it)
        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        offset_t payload_start = carbon_int_column_get_payload_off(it);
        memfile_seek(&it->memfile, payload_start + pos * carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_FIELD_TYPE_COLUMN_BOOLEAN: {
                        u8 value = JAK_CARBON_BOOLEAN_COLUMN_TRUE;
                        memfile_write(&it->memfile, &value, sizeof(u8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U8: {
                        u8 null_value = U8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U16: {
                        u16 null_value = U16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u16));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U32: {
                        //u32 null_value = U32_NULL;
                        //memfile_write(&it->memfile, &null_value, sizeof(u32));
                        rewrite_column_to_array(it);


                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_U64: {
                        u64 null_value = U64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u64));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I8: {
                        i8 null_value = I8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i8));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I16: {
                        i16 null_value = I16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i16));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I32: {
                        i32 null_value = I32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i32));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_I64: {
                        i64 null_value = I64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i64));
                }
                        break;
                case CARBON_FIELD_TYPE_COLUMN_FLOAT: {
                        float null_value = FLOAT_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_FIELD_TYPE_NULL:
                case CARBON_FIELD_TYPE_TRUE:
                case CARBON_FIELD_TYPE_FALSE:
                case CARBON_FIELD_TYPE_OBJECT:
                case CARBON_FIELD_TYPE_ARRAY:
                case CARBON_FIELD_TYPE_STRING:
                case CARBON_FIELD_TYPE_NUMBER_U8:
                case CARBON_FIELD_TYPE_NUMBER_U16:
                case CARBON_FIELD_TYPE_NUMBER_U32:
                case CARBON_FIELD_TYPE_NUMBER_U64:
                case CARBON_FIELD_TYPE_NUMBER_I8:
                case CARBON_FIELD_TYPE_NUMBER_I16:
                case CARBON_FIELD_TYPE_NUMBER_I32:
                case CARBON_FIELD_TYPE_NUMBER_I64:
                case CARBON_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_FIELD_TYPE_BINARY:
                case CARBON_FIELD_TYPE_BINARY_CUSTOM:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
                default:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, JAK_ERR_INTERNALERR);
                        return false;
        }

        memfile_restore_position(&it->memfile);

        return true;
}

bool carbon_column_it_update_set_false(struct jak_carbon_column_it *it, u32 pos)
{
        unused(it)
        unused(pos)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_u8(struct jak_carbon_column_it *it, u32 pos, u8 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_u16(struct jak_carbon_column_it *it, u32 pos, u16 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_u32(struct jak_carbon_column_it *it, u32 pos, u32 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_u64(struct jak_carbon_column_it *it, u32 pos, u64 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_i8(struct jak_carbon_column_it *it, u32 pos, i8 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_i16(struct jak_carbon_column_it *it, u32 pos, i16 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_i32(struct jak_carbon_column_it *it, u32 pos, i32 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_i64(struct jak_carbon_column_it *it, u32 pos, i64 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool carbon_column_it_update_set_float(struct jak_carbon_column_it *it, u32 pos, float value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_column_it_unlock</code> is required for unlocking.
 */
bool carbon_column_it_lock(struct jak_carbon_column_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
bool carbon_column_it_unlock(struct jak_carbon_column_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

bool carbon_column_it_rewind(struct jak_carbon_column_it *it)
{
        error_if_null(it);
        offset_t playload_start = carbon_int_column_get_payload_off(it);
        error_if(playload_start >= memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->memfile, playload_start);
}