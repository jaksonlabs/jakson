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
        carbon_field_type_e type;                                                                                    \
        const void *raw = jak_carbon_column_it_values(&type, nvalues, it);                                                  \
        error_if(!(field_type_expr), &it->err, JAK_ERR_TYPEMISMATCH);                                                  \
        (const builtin_type *) raw;                                                                                    \
})

bool jak_carbon_column_it_create(jak_carbon_column_it *it, struct jak_memfile *memfile, struct jak_error *err,
                             jak_offset_t column_start_offset)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(memfile);
        JAK_ERROR_IF_NULL(err);

        it->column_start_offset = column_start_offset;
        it->mod_size = 0;

        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, column_start_offset);

        error_if(memfile_remain_size(&it->memfile) < sizeof(jak_u8) + sizeof(media_type_t), err, JAK_ERR_CORRUPTED);

        jak_u8 marker = *memfile_read(&it->memfile, sizeof(jak_u8));
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

        carbon_field_type_e type = (carbon_field_type_e) marker;
        it->type = type;

        it->num_and_capacity_start_offset = memfile_tell(&it->memfile);
        it->column_num_elements = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        it->column_capacity = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);

        jak_carbon_column_it_rewind(it);

        return true;
}

bool jak_carbon_column_it_clone(jak_carbon_column_it *dst, jak_carbon_column_it *src)
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

bool jak_carbon_column_it_insert(jak_carbon_insert *inserter, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(it)
        return carbon_int_insert_create_for_column(inserter, it);
}

bool jak_carbon_column_it_fast_forward(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_carbon_column_it_values(NULL, NULL, it);
        return true;
}

jak_offset_t jak_carbon_column_it_memfilepos(jak_carbon_column_it *it)
{
        if (JAK_LIKELY(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, JAK_ERR_NULLPTR);
                return 0;
        }
}

jak_offset_t jak_carbon_column_it_tell(jak_carbon_column_it *it, jak_u32 elem_idx)
{
        if (it) {
                memfile_save_position(&it->memfile);
                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                jak_u32 num_elements = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);
                memfile_read_uintvar_stream(NULL, &it->memfile);
                jak_offset_t payload_start = memfile_tell(&it->memfile);
                error_if(elem_idx >= num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
                jak_offset_t ret = payload_start + elem_idx * carbon_int_get_type_value_size(it->type);
                memfile_restore_position(&it->memfile);
                return ret;
        } else {
                error_print(JAK_ERR_NULLPTR);
                return 0;
        }
}

bool jak_carbon_column_it_values_info(carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);

        if (nvalues) {
                memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                jak_u32 num_elements = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);
                *nvalues = num_elements;
        }

        JAK_optional_set(type, it->type);

        return true;
}

bool jak_carbon_column_it_value_is_null(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it);
        carbon_field_type_e type;
        jak_u32 nvalues = 0;
        jak_carbon_column_it_values_info(&type, &nvalues, it);
        error_if(pos >= nvalues, &it->err, JAK_ERR_OUTOFBOUNDS);
        switch (type) {
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8:
                        return is_null_u8(jak_carbon_column_it_u8_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16:
                        return is_null_u16(jak_carbon_column_it_u16_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32:
                        return is_null_u32(jak_carbon_column_it_u32_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64:
                        return is_null_u64(jak_carbon_column_it_u64_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8:
                        return is_null_i8(jak_carbon_column_it_i8_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16:
                        return is_null_i16(jak_carbon_column_it_i16_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32:
                        return is_null_i32(jak_carbon_column_it_i32_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64:
                        return is_null_i64(jak_carbon_column_it_i64_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT:
                        return is_null_float(jak_carbon_column_it_float_values(NULL, it)[pos]);
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN:
                        return is_null_boolean(jak_carbon_column_it_boolean_values(NULL, it)[pos]);
                default: error(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
        }
}

const void *jak_carbon_column_it_values(carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        jak_u32 num_elements = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        jak_u32 cap_elements = (jak_u32) memfile_read_uintvar_stream(NULL, &it->memfile);
        jak_offset_t payload_start = memfile_tell(&it->memfile);

        const void *result = memfile_peek(&it->memfile, sizeof(void));

        JAK_optional_set(type, it->type);
        JAK_optional_set(nvalues, num_elements);

        jak_u32 skip = cap_elements * carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start + skip);

        return result;
}

const jak_u8 *jak_carbon_column_it_boolean_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u8, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN));
}

const jak_u8 *jak_carbon_column_it_u8_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u8, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_U8));
}

const jak_u16 *jak_carbon_column_it_u16_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u16, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_U16));
}

const jak_u32 *jak_carbon_column_it_u32_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u32, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_U32));
}

const jak_u64 *jak_carbon_column_it_u64_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u64, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_U64));
}

const jak_i8 *jak_carbon_column_it_i8_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i8, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_I8));
}

const jak_i16 *jak_carbon_column_it_i16_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i16, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_I16));
}

const jak_i32 *jak_carbon_column_it_i32_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i32, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_I32));
}

const jak_i64 *jak_carbon_column_it_i64_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i64, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_I64));
}

const float *jak_carbon_column_it_float_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(float, nvalues, it, (type == CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT));
}

bool jak_carbon_column_it_remove(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it);

        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
        memfile_save_position(&it->memfile);

        jak_offset_t payload_start = carbon_int_column_get_payload_off(it);

        /* remove element */
        size_t elem_size = carbon_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, payload_start + pos * elem_size);
        memfile_inplace_remove(&it->memfile, elem_size);

        /* add an empty element at the end to restore the column capacity property */
        memfile_seek(&it->memfile, payload_start + it->column_num_elements * elem_size);
        memfile_inplace_insert(&it->memfile, elem_size);

        /* update element counter */
        memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        jak_u32 num_elems = memfile_peek_uintvar_stream(NULL, &it->memfile);
        JAK_ASSERT(num_elems > 0);
        num_elems--;
        signed_offset_t shift = memfile_update_uintvar_stream(&it->memfile, num_elems);
        it->column_num_elements = num_elems;

        memfile_restore_position(&it->memfile);
        memfile_seek_from_here(&it->memfile, shift);

        return true;
}

bool jak_carbon_column_it_update_set_null(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it)
        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        jak_offset_t payload_start = carbon_int_column_get_payload_off(it);
        memfile_seek(&it->memfile, payload_start + pos * carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN: {
                        jak_u8 null_value = JAK_CARBON_BOOLEAN_COLUMN_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8: {
                        jak_u8 null_value = U8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16: {
                        jak_u16 null_value = U16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u16));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32: {
                        jak_u32 null_value = U32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u32));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64: {
                        jak_u64 null_value = U64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u64));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8: {
                        jak_i8 null_value = I8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16: {
                        jak_i16 null_value = I16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i16));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32: {
                        jak_i32 null_value = I32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i32));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64: {
                        jak_i64 null_value = I64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i64));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT: {
                        float null_value = FLOAT_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_OBJECT:
                case CARBON_JAK_FIELD_TYPE_ARRAY:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
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
for (jak_u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (JAK_LIKELY(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins);                                                                               \
        } else {                                                                                                       \
                carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

#define push_array_element_wvalue(num_values, data, data_cast_type, null_check, insert_func)                           \
for (jak_u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (JAK_LIKELY(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins, datum);                                                                        \
        } else {                                                                                                       \
                carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

static bool rewrite_column_to_array(jak_carbon_column_it *it)
{
        jak_carbon_array_it array_it;
        jak_carbon_insert array_ins;

        memfile_save_position(&it->memfile);

        /* Potentially tailing space after the last ']' marker of the outer most array is used for temporary space */
        memfile_seek_to_end(&it->memfile);
        jak_offset_t array_marker_begin = memfile_tell(&it->memfile);

        size_t capacity = it->column_num_elements * carbon_int_get_type_value_size(it->type);
        carbon_int_insert_array(&it->memfile, capacity);
        jak_carbon_array_it_create(&array_it, &it->memfile, &it->err, array_marker_begin);
        jak_carbon_array_it_insert_begin(&array_ins, &array_it);

        carbon_field_type_e type;
        jak_u32 num_values;
        const void *data = jak_carbon_column_it_values(&type, &num_values, it);
        switch (type) {
                case CARBON_JAK_FIELD_TYPE_NULL:
                        while (num_values--) {
                                carbon_insert_null(&array_ins);
                        }
                        break;
                case CARBON_JAK_FIELD_TYPE_TRUE:
                        push_array_element(num_values, data, jak_u8, is_null_boolean, carbon_insert_true);
                        break;
                case CARBON_JAK_FIELD_TYPE_FALSE:
                        push_array_element(num_values, data, jak_u8, is_null_boolean, carbon_insert_false);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                        push_array_element_wvalue(num_values, data, jak_u8, is_null_u8, carbon_insert_u8);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                        push_array_element_wvalue(num_values, data, jak_u16, is_null_u16, carbon_insert_u16);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                        push_array_element_wvalue(num_values, data, jak_u32, is_null_u32, carbon_insert_u32);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                        push_array_element_wvalue(num_values, data, jak_u64, is_null_u64, carbon_insert_u64);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                        push_array_element_wvalue(num_values, data, jak_i8, is_null_i8, carbon_insert_i8);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                        push_array_element_wvalue(num_values, data, jak_i16, is_null_i16, carbon_insert_i16);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                        push_array_element_wvalue(num_values, data, jak_i32, is_null_i32, carbon_insert_i32);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                        push_array_element_wvalue(num_values, data, jak_i64, is_null_i64, carbon_insert_i64);
                        break;
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                        push_array_element_wvalue(num_values, data, float, is_null_float, carbon_insert_float);
                        break;
                default: error(&it->err, JAK_ERR_UNSUPPORTEDTYPE);
                        return false;
        }

        jak_carbon_array_it_insert_end(&array_ins);
        JAK_ASSERT(array_marker_begin < jak_carbon_array_it_memfilepos(&array_it));
        jak_carbon_array_it_drop(&array_it);

        memfile_restore_position(&it->memfile);
        return true;
}

bool jak_carbon_column_it_update_set_true(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it)
        error_if(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        jak_offset_t payload_start = carbon_int_column_get_payload_off(it);
        memfile_seek(&it->memfile, payload_start + pos * carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_JAK_FIELD_TYPE_COLUMN_BOOLEAN: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_TRUE;
                        memfile_write(&it->memfile, &value, sizeof(jak_u8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U8: {
                        jak_u8 null_value = U8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U16: {
                        jak_u16 null_value = U16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u16));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U32: {
                        //jak_u32 null_value = U32_NULL;
                        //memfile_write(&it->memfile, &null_value, sizeof(jak_u32));
                        rewrite_column_to_array(it);


                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_U64: {
                        jak_u64 null_value = U64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_u64));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I8: {
                        jak_i8 null_value = I8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i8));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I16: {
                        jak_i16 null_value = I16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i16));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I32: {
                        jak_i32 null_value = I32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i32));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_I64: {
                        jak_i64 null_value = I64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(jak_i64));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_COLUMN_FLOAT: {
                        float null_value = FLOAT_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_JAK_FIELD_TYPE_NULL:
                case CARBON_JAK_FIELD_TYPE_TRUE:
                case CARBON_JAK_FIELD_TYPE_FALSE:
                case CARBON_JAK_FIELD_TYPE_OBJECT:
                case CARBON_JAK_FIELD_TYPE_ARRAY:
                case CARBON_JAK_FIELD_TYPE_STRING:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_U64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I8:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I16:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I32:
                case CARBON_JAK_FIELD_TYPE_NUMBER_I64:
                case CARBON_JAK_FIELD_TYPE_NUMBER_FLOAT:
                case CARBON_JAK_FIELD_TYPE_BINARY:
                case CARBON_JAK_FIELD_TYPE_BINARY_CUSTOM:
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

bool jak_carbon_column_it_update_set_false(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u8(jak_carbon_column_it *it, jak_u32 pos, jak_u8 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u16(jak_carbon_column_it *it, jak_u32 pos, jak_u16 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u32(jak_carbon_column_it *it, jak_u32 pos, jak_u32 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u64(jak_carbon_column_it *it, jak_u32 pos, jak_u64 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i8(jak_carbon_column_it *it, jak_u32 pos, jak_i8 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i16(jak_carbon_column_it *it, jak_u32 pos, jak_i16 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i32(jak_carbon_column_it *it, jak_u32 pos, jak_i32 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i64(jak_carbon_column_it *it, jak_u32 pos, jak_i64 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_float(jak_carbon_column_it *it, jak_u32 pos, float value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        error_print(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_column_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_column_it_lock(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
bool jak_carbon_column_it_unlock(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        spin_release(&it->lock);
        return true;
}

bool jak_carbon_column_it_rewind(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_offset_t playload_start = carbon_int_column_get_payload_off(it);
        error_if(playload_start >= memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->memfile, playload_start);
}