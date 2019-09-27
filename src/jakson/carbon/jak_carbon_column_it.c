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

#include <jakson/carbon/jak_carbon_column_it.h>
#include <jakson/carbon/jak_carbon_array_it.h>
#include <jakson/carbon/jak_carbon_media.h>
#include <jakson/carbon/jak_carbon_insert.h>
#include <jakson/carbon/jak_carbon_int.h>

#define safe_cast(builtin_type, nvalues, it, field_type_expr)                                                          \
({                                                                                                                     \
        jak_carbon_field_type_e type;                                                                                  \
        const void *raw = jak_carbon_column_it_values(&type, nvalues, it);                                             \
        JAK_ERROR_IF(!(field_type_expr), &it->err, JAK_ERR_TYPEMISMATCH);                                              \
        (const builtin_type *) raw;                                                                                    \
})

bool jak_carbon_column_it_create(jak_carbon_column_it *it, jak_memfile *memfile, jak_error *err,
                             jak_offset_t column_start_offset)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(memfile);
        JAK_ERROR_IF_NULL(err);

        it->column_start_offset = column_start_offset;
        it->mod_size = 0;

        jak_error_init(&it->err);
        jak_spinlock_init(&it->lock);
        jak_memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        jak_memfile_seek(&it->memfile, column_start_offset);

        JAK_ERROR_IF(jak_memfile_remain_size(&it->memfile) < sizeof(jak_u8) + sizeof(jak_media_type), err, JAK_ERR_CORRUPTED);

        jak_u8 marker = *jak_memfile_read(&it->memfile, sizeof(jak_u8));
        JAK_ERROR_IF_WDETAILS(marker != CARBON_MCOLUMN_U8 &&
                              marker != CARBON_MCOLUMN_U16 &&
                              marker != CARBON_MCOLUMN_U32 &&
                              marker != CARBON_MCOLUMN_U64 &&
                              marker != CARBON_MCOLUMN_I8 &&
                              marker != CARBON_MCOLUMN_I16 &&
                              marker != CARBON_MCOLUMN_I32 &&
                              marker != CARBON_MCOLUMN_I64 &&
                              marker != CARBON_MCOLUMN_FLOAT &&
                              marker != CARBON_MCOLUMN_BOOLEAN, err, JAK_ERR_ILLEGALOP,
                              "column begin marker ('(') not found");

        jak_carbon_field_type_e type = (jak_carbon_field_type_e) marker;
        it->type = type;

        it->num_and_capacity_start_offset = jak_memfile_tell(&it->memfile);
        it->column_num_elements = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);
        it->column_capacity = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);

        jak_carbon_column_it_rewind(it);

        return true;
}

bool jak_carbon_column_it_clone(jak_carbon_column_it *dst, jak_carbon_column_it *src)
{
        jak_memfile_clone(&dst->memfile, &src->memfile);
        dst->num_and_capacity_start_offset = src->num_and_capacity_start_offset;
        dst->column_start_offset = src->column_start_offset;
        jak_error_cpy(&dst->err, &src->err);
        dst->type = src->type;
        dst->mod_size = src->mod_size;
        dst->column_capacity = src->column_capacity;
        dst->column_num_elements = src->column_num_elements;
        jak_spinlock_init(&dst->lock);
        return true;
}

bool jak_carbon_column_it_insert(jak_carbon_insert *inserter, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(it)
        return jak_carbon_int_insert_create_for_column(inserter, it);
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
                return jak_memfile_tell(&it->memfile);
        } else {
                JAK_ERROR(&it->err, JAK_ERR_NULLPTR);
                return 0;
        }
}

jak_offset_t jak_carbon_column_it_tell(jak_carbon_column_it *it, jak_u32 elem_idx)
{
        if (it) {
                jak_memfile_save_position(&it->memfile);
                jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                jak_u32 num_elements = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);
                jak_memfile_read_uintvar_stream(NULL, &it->memfile);
                jak_offset_t payload_start = jak_memfile_tell(&it->memfile);
                JAK_ERROR_IF(elem_idx >= num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
                jak_offset_t ret = payload_start + elem_idx * jak_carbon_int_get_type_value_size(it->type);
                jak_memfile_restore_position(&it->memfile);
                return ret;
        } else {
                JAK_ERROR_PRINT(JAK_ERR_NULLPTR);
                return 0;
        }
}

bool jak_carbon_column_it_values_info(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);

        if (nvalues) {
                jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
                jak_u32 num_elements = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);
                *nvalues = num_elements;
        }

        JAK_OPTIONAL_SET(type, it->type);

        return true;
}

bool jak_carbon_column_it_value_is_null(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it);
        jak_carbon_field_type_e type;
        jak_u32 nvalues = 0;
        jak_carbon_column_it_values_info(&type, &nvalues, it);
        JAK_ERROR_IF(pos >= nvalues, &it->err, JAK_ERR_OUTOFBOUNDS);
        switch (type) {
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET:
                        return JAK_IS_NULL_U8(jak_carbon_column_it_u8_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET:
                        return JAK_IS_NULL_U16(jak_carbon_column_it_u16_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET:
                        return JAK_IS_NULL_U32(jak_carbon_column_it_u32_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET:
                        return JAK_IS_NULL_U64(jak_carbon_column_it_u64_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET:
                        return JAK_IS_NULL_I8(jak_carbon_column_it_i8_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET:
                        return JAK_IS_NULL_I16(jak_carbon_column_it_i16_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET:
                        return JAK_IS_NULL_I32(jak_carbon_column_it_i32_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET:
                        return JAK_IS_NULL_I64(jak_carbon_column_it_i64_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET:
                        return JAK_IS_NULL_FLOAT(jak_carbon_column_it_float_values(NULL, it)[pos]);
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET:
                        return JAK_IS_NULL_BOOLEAN(jak_carbon_column_it_boolean_values(NULL, it)[pos]);
                default: JAK_ERROR(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
        }
}

const void *jak_carbon_column_it_values(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        jak_u32 num_elements = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);
        jak_u32 cap_elements = (jak_u32) jak_memfile_read_uintvar_stream(NULL, &it->memfile);
        jak_offset_t payload_start = jak_memfile_tell(&it->memfile);

        const void *result = jak_memfile_peek(&it->memfile, sizeof(void));

        JAK_OPTIONAL_SET(type, it->type);
        JAK_OPTIONAL_SET(nvalues, num_elements);

        jak_u32 skip = cap_elements * jak_carbon_int_get_type_value_size(it->type);
        jak_memfile_seek(&it->memfile, payload_start + skip);

        return result;
}

const jak_u8 *jak_carbon_column_it_boolean_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u8, nvalues, it, jak_carbon_field_type_is_column_bool_or_subtype(type));
}

const jak_u8 *jak_carbon_column_it_u8_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u8, nvalues, it, jak_carbon_field_type_is_column_u8_or_subtype(type));
}

const jak_u16 *jak_carbon_column_it_u16_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u16, nvalues, it, jak_carbon_field_type_is_column_u16_or_subtype(type));
}

const jak_u32 *jak_carbon_column_it_u32_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u32, nvalues, it, jak_carbon_field_type_is_column_u32_or_subtype(type));
}

const jak_u64 *jak_carbon_column_it_u64_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_u64, nvalues, it, jak_carbon_field_type_is_column_u64_or_subtype(type));
}

const jak_i8 *jak_carbon_column_it_i8_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i8, nvalues, it, jak_carbon_field_type_is_column_i8_or_subtype(type));
}

const jak_i16 *jak_carbon_column_it_i16_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i16, nvalues, it, jak_carbon_field_type_is_column_i16_or_subtype(type));
}

const jak_i32 *jak_carbon_column_it_i32_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i32, nvalues, it, jak_carbon_field_type_is_column_i32_or_subtype(type));
}

const jak_i64 *jak_carbon_column_it_i64_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(jak_i64, nvalues, it, jak_carbon_field_type_is_column_i64_or_subtype(type));
}

const float *jak_carbon_column_it_float_values(jak_u32 *nvalues, jak_carbon_column_it *it)
{
        return safe_cast(float, nvalues, it, jak_carbon_field_type_is_column_float_or_subtype(type));
}

bool jak_carbon_column_it_remove(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it);

        JAK_ERROR_IF(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS);
        jak_memfile_save_position(&it->memfile);

        jak_offset_t payload_start = jak_carbon_int_column_get_payload_off(it);

        /* remove element */
        size_t elem_size = jak_carbon_int_get_type_value_size(it->type);
        jak_memfile_seek(&it->memfile, payload_start + pos * elem_size);
        jak_memfile_inplace_remove(&it->memfile, elem_size);

        /* add an empty element at the end to restore the column capacity property */
        jak_memfile_seek(&it->memfile, payload_start + it->column_num_elements * elem_size);
        jak_memfile_inplace_insert(&it->memfile, elem_size);

        /* update element counter */
        jak_memfile_seek(&it->memfile, it->num_and_capacity_start_offset);
        jak_u32 num_elems = jak_memfile_peek_uintvar_stream(NULL, &it->memfile);
        JAK_ASSERT(num_elems > 0);
        num_elems--;
        signed_offset_t shift = jak_memfile_update_uintvar_stream(&it->memfile, num_elems);
        it->column_num_elements = num_elems;

        jak_memfile_restore_position(&it->memfile);
        jak_memfile_seek_from_here(&it->memfile, shift);

        return true;
}

bool jak_carbon_column_it_update_set_null(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        jak_memfile_save_position(&it->memfile);

        jak_offset_t payload_start = jak_carbon_int_column_get_payload_off(it);
        jak_memfile_seek(&it->memfile, payload_start + pos * jak_carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        jak_u8 null_value = JAK_CARBON_BOOLEAN_COLUMN_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET: {
                        jak_u8 null_value = JAK_U8_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET: {
                        jak_u16 null_value = JAK_U16_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u16));
                }
                        break;
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET: {
                        jak_u32 null_value = JAK_U32_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u32));
                }
                        break;
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET: {
                        jak_u64 null_value = JAK_U64_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u64));
                }
                        break;
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET: {
                        jak_i8 null_value = JAK_I8_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i8));
                }
                        break;
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET: {
                        jak_i16 null_value = JAK_I16_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i16));
                }
                        break;
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET: {
                        jak_i32 null_value = JAK_I32_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i32));
                }
                        break;
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                        jak_i64 null_value = JAK_I64_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i64));
                }
                        break;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET: {
                        float null_value = JAK_FLOAT_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        jak_memfile_restore_position(&it->memfile);
                        JAK_ERROR(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
                default:
                        jak_memfile_restore_position(&it->memfile);
                        JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                        return false;
        }

        jak_memfile_restore_position(&it->memfile);

        return true;
}

#define push_array_element(num_values, data, data_cast_type, null_check, insert_func)                                  \
for (jak_u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (JAK_LIKELY(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins);                                                                               \
        } else {                                                                                                       \
                jak_carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

#define push_array_element_wvalue(num_values, data, data_cast_type, null_check, insert_func)                           \
for (jak_u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (JAK_LIKELY(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins, datum);                                                                        \
        } else {                                                                                                       \
                jak_carbon_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

static bool rewrite_column_to_array(jak_carbon_column_it *it)
{
        carbon_abstract_type_class_e type_class;
        carbon_list_derivable_e list_type;
        jak_carbon_array_it array_it;
        jak_carbon_insert array_ins;

        jak_memfile_save_position(&it->memfile);
        assert(jak_carbon_field_type_is_column_or_subtype(jak_memfile_peek_byte(&it->memfile)));

        carbon_abstract_get_class(&type_class, &it->memfile);
        carbon_abstract_class_to_list_derivable(&list_type, type_class);

        /* Potentially tailing space after the last ']' marker of the outer most array is used for temporary space */
        jak_memfile_seek_to_end(&it->memfile);
        jak_offset_t array_marker_begin = jak_memfile_tell(&it->memfile);

        size_t capacity = it->column_num_elements * jak_carbon_int_get_type_value_size(it->type);
        jak_carbon_int_insert_array(&it->memfile, list_type, capacity);
        jak_carbon_array_it_create(&array_it, &it->memfile, &it->err, array_marker_begin);
        jak_carbon_array_it_insert_begin(&array_ins, &array_it);

        jak_carbon_field_type_e type;
        jak_u32 num_values;
        const void *data = jak_carbon_column_it_values(&type, &num_values, it);
        switch (type) {
                case CARBON_FIELD_NULL:
                        while (num_values--) {
                                jak_carbon_insert_null(&array_ins);
                        }
                        break;
                case CARBON_FIELD_TRUE:
                        push_array_element(num_values, data, jak_u8, JAK_IS_NULL_BOOLEAN, jak_carbon_insert_true);
                        break;
                case CARBON_FIELD_FALSE:
                        push_array_element(num_values, data, jak_u8, JAK_IS_NULL_BOOLEAN, jak_carbon_insert_false);
                        break;
                case CARBON_FIELD_NUMBER_U8:
                        push_array_element_wvalue(num_values, data, jak_u8, JAK_IS_NULL_U8, jak_carbon_insert_u8);
                        break;
                case CARBON_FIELD_NUMBER_U16:
                        push_array_element_wvalue(num_values, data, jak_u16, JAK_IS_NULL_U16, jak_carbon_insert_u16);
                        break;
                case CARBON_FIELD_NUMBER_U32:
                        push_array_element_wvalue(num_values, data, jak_u32, JAK_IS_NULL_U32, jak_carbon_insert_u32);
                        break;
                case CARBON_FIELD_NUMBER_U64:
                        push_array_element_wvalue(num_values, data, jak_u64, JAK_IS_NULL_U64, jak_carbon_insert_u64);
                        break;
                case CARBON_FIELD_NUMBER_I8:
                        push_array_element_wvalue(num_values, data, jak_i8, JAK_IS_NULL_I8, jak_carbon_insert_i8);
                        break;
                case CARBON_FIELD_NUMBER_I16:
                        push_array_element_wvalue(num_values, data, jak_i16, JAK_IS_NULL_I16, jak_carbon_insert_i16);
                        break;
                case CARBON_FIELD_NUMBER_I32:
                        push_array_element_wvalue(num_values, data, jak_i32, JAK_IS_NULL_I32, jak_carbon_insert_i32);
                        break;
                case CARBON_FIELD_NUMBER_I64:
                        push_array_element_wvalue(num_values, data, jak_i64, JAK_IS_NULL_I64, jak_carbon_insert_i64);
                        break;
                case CARBON_FIELD_NUMBER_FLOAT:
                        push_array_element_wvalue(num_values, data, float, JAK_IS_NULL_FLOAT, jak_carbon_insert_float);
                        break;
                default: JAK_ERROR(&it->err, JAK_ERR_UNSUPPORTEDTYPE);
                        return false;
        }

        jak_carbon_array_it_insert_end(&array_ins);
        JAK_ASSERT(array_marker_begin < jak_carbon_array_it_memfilepos(&array_it));
        jak_carbon_array_it_drop(&array_it);

        jak_memfile_restore_position(&it->memfile);
        return true;
}

bool jak_carbon_column_it_update_set_true(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF(pos >= it->column_num_elements, &it->err, JAK_ERR_OUTOFBOUNDS)

        jak_memfile_save_position(&it->memfile);

        jak_offset_t payload_start = jak_carbon_int_column_get_payload_off(it);
        jak_memfile_seek(&it->memfile, payload_start + pos * jak_carbon_int_get_type_value_size(it->type));

        switch (it->type) {
                case CARBON_FIELD_COLUMN_BOOLEAN_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_BOOLEAN_SORTED_SET: {
                        jak_u8 value = JAK_CARBON_BOOLEAN_COLUMN_TRUE;
                        jak_memfile_write(&it->memfile, &value, sizeof(jak_u8));
                }
                        break;
                case CARBON_FIELD_COLUMN_U8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U8_SORTED_SET: {
                        jak_u8 null_value = JAK_U8_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u8));
                }
                        break;
                case CARBON_FIELD_COLUMN_U16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U16_SORTED_SET: {
                        jak_u16 null_value = JAK_U16_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u16));
                }
                        break;
                case CARBON_FIELD_COLUMN_U32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U32_SORTED_SET: {
                        //jak_u32 null_value = JAK_U32_NULL;
                        //jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u32));
                        rewrite_column_to_array(it);


                }
                        break;
                case CARBON_FIELD_COLUMN_U64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_U64_SORTED_SET: {
                        jak_u64 null_value = JAK_U64_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_u64));
                }
                        break;
                case CARBON_FIELD_COLUMN_I8_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I8_SORTED_SET: {
                        jak_i8 null_value = JAK_I8_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i8));
                }
                        break;
                case CARBON_FIELD_COLUMN_I16_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I16_SORTED_SET: {
                        jak_i16 null_value = JAK_I16_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i16));
                }
                        break;
                case CARBON_FIELD_COLUMN_I32_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I32_SORTED_SET: {
                        jak_i32 null_value = JAK_I32_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i32));
                }
                        break;
                case CARBON_FIELD_COLUMN_I64_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_I64_SORTED_SET: {
                        jak_i64 null_value = JAK_I64_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(jak_i64));
                }
                        break;
                case CARBON_FIELD_COLUMN_FLOAT_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_COLUMN_FLOAT_SORTED_SET: {
                        float null_value = JAK_FLOAT_NULL;
                        jak_memfile_write(&it->memfile, &null_value, sizeof(float));
                }
                        break;
                case CARBON_FIELD_NULL:
                case CARBON_FIELD_TRUE:
                case CARBON_FIELD_FALSE:
                case CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_SORTED_MULTIMAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_UNSORTED_MAP:
                case CARBON_FIELD_DERIVED_OBJECT_CARBON_SORTED_MAP:
                case CARBON_FIELD_ARRAY_UNSORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_MULTISET:
                case CARBON_FIELD_DERIVED_ARRAY_UNSORTED_SET:
                case CARBON_FIELD_DERIVED_ARRAY_SORTED_SET:
                case CARBON_FIELD_STRING:
                case CARBON_FIELD_NUMBER_U8:
                case CARBON_FIELD_NUMBER_U16:
                case CARBON_FIELD_NUMBER_U32:
                case CARBON_FIELD_NUMBER_U64:
                case CARBON_FIELD_NUMBER_I8:
                case CARBON_FIELD_NUMBER_I16:
                case CARBON_FIELD_NUMBER_I32:
                case CARBON_FIELD_NUMBER_I64:
                case CARBON_FIELD_NUMBER_FLOAT:
                case CARBON_FIELD_BINARY:
                case CARBON_FIELD_BINARY_CUSTOM:
                        jak_memfile_restore_position(&it->memfile);
                        JAK_ERROR(&it->err, JAK_ERR_UNSUPPCONTAINER)
                        return false;
                default:
                        jak_memfile_restore_position(&it->memfile);
                        JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                        return false;
        }

        jak_memfile_restore_position(&it->memfile);

        return true;
}

bool jak_carbon_column_it_update_set_false(jak_carbon_column_it *it, jak_u32 pos)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u8(jak_carbon_column_it *it, jak_u32 pos, jak_u8 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u16(jak_carbon_column_it *it, jak_u32 pos, jak_u16 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u32(jak_carbon_column_it *it, jak_u32 pos, jak_u32 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_u64(jak_carbon_column_it *it, jak_u32 pos, jak_u64 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i8(jak_carbon_column_it *it, jak_u32 pos, jak_i8 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i16(jak_carbon_column_it *it, jak_u32 pos, jak_i16 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i32(jak_carbon_column_it *it, jak_u32 pos, jak_i32 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_i64(jak_carbon_column_it *it, jak_u32 pos, jak_i64 value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

bool jak_carbon_column_it_update_set_float(jak_carbon_column_it *it, jak_u32 pos, float value)
{
        JAK_UNUSED(it)
        JAK_UNUSED(pos)
        JAK_UNUSED(value)
        JAK_ERROR_PRINT(JAK_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_column_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_column_it_lock(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_spinlock_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
bool jak_carbon_column_it_unlock(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_spinlock_release(&it->lock);
        return true;
}

bool jak_carbon_column_it_rewind(jak_carbon_column_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_offset_t playload_start = jak_carbon_int_column_get_payload_off(it);
        JAK_ERROR_IF(playload_start >= jak_memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        return jak_memfile_seek(&it->memfile, playload_start);
}