/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <jakson/jak_carbon.h>
#include <jakson/carbon/jak_carbon_object_it.h>
#include <jakson/carbon/jak_carbon_column_it.h>
#include <jakson/carbon/jak_carbon_insert.h>
#include <jakson/carbon/jak_carbon_string.h>
#include <jakson/carbon/jak_carbon_prop.h>
#include <jakson/carbon/jak_carbon_object_it.h>

bool jak_carbon_object_it_create(jak_carbon_object_it *it, jak_memfile *memfile, jak_error *err,
                             jak_offset_t payload_start)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(memfile);
        JAK_ERROR_IF_NULL(err);

        it->object_contents_off = payload_start;
        it->mod_size = 0;
        it->object_end_reached = false;

        jak_spinlock_init(&it->lock);
        jak_error_init(&it->err);

        jak_vector_create(&it->history, NULL, sizeof(jak_offset_t), 40);

        jak_memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        jak_memfile_seek(&it->memfile, payload_start);

        JAK_ERROR_IF(jak_memfile_remain_size(&it->memfile) < sizeof(jak_u8), err, JAK_ERR_CORRUPTED);

        jak_carbon_container_sub_type_e sub_type;
        carbon_abstract_get_container_subtype(&sub_type, &it->memfile);
        JAK_ERROR_IF_WDETAILS(sub_type != CARBON_CONTAINER_OBJECT, err, JAK_ERR_ILLEGALOP,
                              "object begin marker ('{') or abstract derived type marker for 'map' not found");
        jak_memfile_skip(&it->memfile, sizeof(jak_u8));

        it->object_contents_off += sizeof(jak_u8);

        jak_carbon_int_field_access_create(&it->field.value.data);

        jak_carbon_object_it_rewind(it);

        return true;
}

bool jak_carbon_object_it_copy(jak_carbon_object_it *dst, jak_carbon_object_it *src)
{
        JAK_ERROR_IF_NULL(dst);
        JAK_ERROR_IF_NULL(src);
        jak_carbon_object_it_create(dst, &src->memfile, &src->err, src->object_contents_off - sizeof(jak_u8));
        return true;
}

bool jak_carbon_object_it_clone(jak_carbon_object_it *dst, jak_carbon_object_it *src)
{
        JAK_ERROR_IF_NULL(dst);
        JAK_ERROR_IF_NULL(src);
        jak_memfile_clone(&dst->memfile, &src->memfile);
        dst->object_contents_off = src->object_contents_off;
        jak_spinlock_init(&dst->lock);
        jak_error_cpy(&dst->err, &src->err);
        dst->mod_size = src->mod_size;
        dst->object_end_reached = src->object_end_reached;
        jak_vector_cpy(&dst->history, &src->history);
        dst->field.key.name_len = src->field.key.name_len;
        dst->field.key.name = src->field.key.name;
        dst->field.key.offset = src->field.key.offset;
        dst->field.value.offset = src->field.value.offset;
        jak_carbon_int_field_access_clone(&dst->field.value.data, &src->field.value.data);
        return true;
}

bool jak_carbon_object_it_drop(jak_carbon_object_it *it)
{
        jak_carbon_int_field_auto_close(&it->field.value.data);
        jak_carbon_int_field_access_drop(&it->field.value.data);
        jak_vector_drop(&it->history);
        return true;
}

bool jak_carbon_object_it_rewind(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF(it->object_contents_off >= jak_memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        jak_carbon_int_history_clear(&it->history);
        return jak_memfile_seek(&it->memfile, it->object_contents_off);
}

bool jak_carbon_object_it_next(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        bool is_empty_slot;
        jak_offset_t last_off = jak_memfile_tell(&it->memfile);
        jak_carbon_int_field_access_drop(&it->field.value.data);
        if (jak_carbon_int_object_it_next(&is_empty_slot, &it->object_end_reached, it)) {
                jak_carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!it->object_end_reached) {
                        JAK_ERROR_IF(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);

                        while (*jak_memfile_peek(&it->memfile, 1) == 0) {
                                jak_memfile_skip(&it->memfile, 1);
                        }
                }

                JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(char)) == CARBON_MOBJECT_END);
                return false;
        }
}

bool jak_carbon_object_it_has_next(jak_carbon_object_it *it)
{
        bool has_next = jak_carbon_object_it_next(it);
        jak_carbon_object_it_prev(it);
        return has_next;
}

bool jak_carbon_object_it_prev(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        if (jak_carbon_int_history_has(&it->history)) {
                jak_offset_t prev_off = jak_carbon_int_history_pop(&it->history);
                jak_memfile_seek(&it->memfile, prev_off);
                return jak_carbon_int_object_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

jak_offset_t jak_carbon_object_it_jak_memfile_pos(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        return jak_memfile_tell(&it->memfile);
}

bool jak_carbon_object_it_tell(jak_offset_t *key_off, jak_offset_t *value_off, jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        JAK_OPTIONAL_SET(key_off, it->field.key.offset);
        JAK_OPTIONAL_SET(value_off, it->field.value.offset);
        return true;
}

const char *jak_carbon_object_it_prop_name(jak_u64 *key_len, jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF_NULL(key_len)
        *key_len = it->field.key.name_len;
        return it->field.key.name;
}

static jak_i64 prop_remove(jak_carbon_object_it *it, jak_carbon_field_type_e type)
{
        jak_i64 prop_size = jak_carbon_prop_size(&it->memfile);
        jak_carbon_jak_string_nomarker_remove(&it->memfile);
        if (jak_carbon_int_field_remove(&it->memfile, &it->err, type)) {
                jak_carbon_int_object_it_refresh(NULL, NULL, it);
                return prop_size;
        } else {
                return 0;
        }
}

bool jak_carbon_object_it_remove(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_carbon_field_type_e type;
        if (jak_carbon_object_it_prop_type(&type, it)) {
                jak_offset_t prop_off = jak_carbon_int_history_pop(&it->history);
                jak_memfile_seek(&it->memfile, prop_off);
                it->mod_size -= prop_remove(it, type);
                return true;
        } else {
                JAK_ERROR(&it->err, JAK_ERR_ILLEGALSTATE);
                return false;
        }
}

bool jak_carbon_object_it_prop_type(jak_carbon_field_type_e *type, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_field_type(type, &it->field.value.data);
}

bool jak_carbon_object_it_u8_value(jak_u8 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_u8_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_u16_value(jak_u16 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_u16_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_u32_value(jak_u32 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_u32_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_u64_value(jak_u64 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_u64_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_i8_value(jak_i8 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_i8_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_i16_value(jak_i16 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_i16_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_i32_value(jak_i32 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_i32_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_i64_value(jak_i64 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_i64_value(value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_float_value(bool *is_null_in, float *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_float_value(is_null_in, value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_signed_value(is_null_in, value, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_unsigned_value(is_null_in, value, &it->field.value.data, &it->err);
}

const char *jak_carbon_object_it_jak_string_value(jak_u64 *strlen, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_jak_string_value(strlen, &it->field.value.data, &it->err);
}

bool jak_carbon_object_it_binary_value(jak_carbon_binary *out, jak_carbon_object_it *it)
{
        return jak_carbon_int_field_access_binary_value(out, &it->field.value.data, &it->err);
}

jak_carbon_array_it *jak_carbon_object_it_array_value(jak_carbon_object_it *it_in)
{
        return jak_carbon_int_field_access_array_value(&it_in->field.value.data, &it_in->err);
}

jak_carbon_object_it *jak_carbon_object_it_object_value(jak_carbon_object_it *it_in)
{
        return jak_carbon_int_field_access_object_value(&it_in->field.value.data, &it_in->err);
}

jak_carbon_column_it *jak_carbon_object_it_column_value(jak_carbon_object_it *it_in)
{
        return jak_carbon_int_field_access_column_value(&it_in->field.value.data, &it_in->err);
}

bool jak_carbon_object_it_insert_begin(jak_carbon_insert *inserter, jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(it)
        return jak_carbon_int_insert_create_for_object(inserter, it);
}

bool jak_carbon_object_it_insert_end(jak_carbon_insert *inserter)
{
        JAK_ERROR_IF_NULL(inserter)
        return jak_carbon_insert_drop(inserter);
}

bool jak_carbon_object_it_lock(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        jak_spinlock_acquire(&it->lock);
        return true;
}

bool jak_carbon_object_it_unlock(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it)
        jak_spinlock_release(&it->lock);
        return true;
}

bool jak_carbon_object_it_fast_forward(jak_carbon_object_it *it)
{
        JAK_ERROR_IF_NULL(it);
        while (jak_carbon_object_it_next(it)) {}

        JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(jak_u8)) == CARBON_MOBJECT_END);
        jak_memfile_skip(&it->memfile, sizeof(jak_u8));
        return true;
}