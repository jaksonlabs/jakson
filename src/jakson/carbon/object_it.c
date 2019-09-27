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

#include <jakson/carbon.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/insert.h>
#include <jakson/carbon/string.h>
#include <jakson/carbon/prop.h>
#include <jakson/carbon/object_it.h>

bool carbon_object_it_create(carbon_object_it *it, memfile *memfile, err *err,
                             offset_t payload_start)
{
        ERROR_IF_NULL(it);
        ERROR_IF_NULL(memfile);
        ERROR_IF_NULL(err);

        it->object_contents_off = payload_start;
        it->mod_size = 0;
        it->object_end_reached = false;

        spinlock_init(&it->lock);
        error_init(&it->err);

        vector_create(&it->history, NULL, sizeof(offset_t), 40);

        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        ERROR_IF(memfile_remain_size(&it->memfile) < sizeof(u8), err, ERR_CORRUPTED);

        carbon_container_sub_type_e sub_type;
        carbon_abstract_get_container_subtype(&sub_type, &it->memfile);
        ERROR_IF_WDETAILS(sub_type != CARBON_CONTAINER_OBJECT, err, ERR_ILLEGALOP,
                              "object begin marker ('{') or abstract derived type marker for 'map' not found");
        memfile_skip(&it->memfile, sizeof(u8));

        it->object_contents_off += sizeof(u8);

        carbon_int_field_access_create(&it->field.value.data);

        carbon_object_it_rewind(it);

        return true;
}

bool carbon_object_it_copy(carbon_object_it *dst, carbon_object_it *src)
{
        ERROR_IF_NULL(dst);
        ERROR_IF_NULL(src);
        carbon_object_it_create(dst, &src->memfile, &src->err, src->object_contents_off - sizeof(u8));
        return true;
}

bool carbon_object_it_clone(carbon_object_it *dst, carbon_object_it *src)
{
        ERROR_IF_NULL(dst);
        ERROR_IF_NULL(src);
        memfile_clone(&dst->memfile, &src->memfile);
        dst->object_contents_off = src->object_contents_off;
        spinlock_init(&dst->lock);
        error_cpy(&dst->err, &src->err);
        dst->mod_size = src->mod_size;
        dst->object_end_reached = src->object_end_reached;
        vector_cpy(&dst->history, &src->history);
        dst->field.key.name_len = src->field.key.name_len;
        dst->field.key.name = src->field.key.name;
        dst->field.key.offset = src->field.key.offset;
        dst->field.value.offset = src->field.value.offset;
        carbon_int_field_access_clone(&dst->field.value.data, &src->field.value.data);
        return true;
}

bool carbon_object_it_drop(carbon_object_it *it)
{
        carbon_int_field_auto_close(&it->field.value.data);
        carbon_int_field_access_drop(&it->field.value.data);
        vector_drop(&it->history);
        return true;
}

bool carbon_object_it_rewind(carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        ERROR_IF(it->object_contents_off >= memfile_size(&it->memfile), &it->err, ERR_OUTOFBOUNDS);
        carbon_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->object_contents_off);
}

bool carbon_object_it_next(carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        bool is_empty_slot;
        offset_t last_off = memfile_tell(&it->memfile);
        carbon_int_field_access_drop(&it->field.value.data);
        if (carbon_int_object_it_next(&is_empty_slot, &it->object_end_reached, it)) {
                carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /** skip remaining zeros until end of array is reached */
                if (!it->object_end_reached) {
                        ERROR_IF(!is_empty_slot, &it->err, ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }

                JAK_ASSERT(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MOBJECT_END);
                return false;
        }
}

bool carbon_object_it_has_next(carbon_object_it *it)
{
        bool has_next = carbon_object_it_next(it);
        carbon_object_it_prev(it);
        return has_next;
}

bool carbon_object_it_prev(carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        if (carbon_int_history_has(&it->history)) {
                offset_t prev_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                return carbon_int_object_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

offset_t carbon_object_it_memfile_pos(carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        return memfile_tell(&it->memfile);
}

bool carbon_object_it_tell(offset_t *key_off, offset_t *value_off, carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        OPTIONAL_SET(key_off, it->field.key.offset);
        OPTIONAL_SET(value_off, it->field.value.offset);
        return true;
}

const char *carbon_object_it_prop_name(u64 *key_len, carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        ERROR_IF_NULL(key_len)
        *key_len = it->field.key.name_len;
        return it->field.key.name;
}

static i64 prop_remove(carbon_object_it *it, carbon_field_type_e type)
{
        i64 prop_size = carbon_prop_size(&it->memfile);
        carbon_string_nomarker_remove(&it->memfile);
        if (carbon_int_field_remove(&it->memfile, &it->err, type)) {
                carbon_int_object_it_refresh(NULL, NULL, it);
                return prop_size;
        } else {
                return 0;
        }
}

bool carbon_object_it_remove(carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        carbon_field_type_e type;
        if (carbon_object_it_prop_type(&type, it)) {
                offset_t prop_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prop_off);
                it->mod_size -= prop_remove(it, type);
                return true;
        } else {
                ERROR(&it->err, ERR_ILLEGALSTATE);
                return false;
        }
}

bool carbon_object_it_prop_type(carbon_field_type_e *type, carbon_object_it *it)
{
        return carbon_int_field_access_field_type(type, &it->field.value.data);
}

bool carbon_object_it_u8_value(u8 *value, carbon_object_it *it)
{
        return carbon_int_field_access_u8_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_u16_value(u16 *value, carbon_object_it *it)
{
        return carbon_int_field_access_u16_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_u32_value(u32 *value, carbon_object_it *it)
{
        return carbon_int_field_access_u32_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_u64_value(u64 *value, carbon_object_it *it)
{
        return carbon_int_field_access_u64_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_i8_value(i8 *value, carbon_object_it *it)
{
        return carbon_int_field_access_i8_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_i16_value(i16 *value, carbon_object_it *it)
{
        return carbon_int_field_access_i16_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_i32_value(i32 *value, carbon_object_it *it)
{
        return carbon_int_field_access_i32_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_i64_value(i64 *value, carbon_object_it *it)
{
        return carbon_int_field_access_i64_value(value, &it->field.value.data, &it->err);
}

bool carbon_object_it_float_value(bool *is_null_in, float *value, carbon_object_it *it)
{
        return carbon_int_field_access_float_value(is_null_in, value, &it->field.value.data, &it->err);
}

bool carbon_object_it_signed_value(bool *is_null_in, i64 *value, carbon_object_it *it)
{
        return carbon_int_field_access_signed_value(is_null_in, value, &it->field.value.data, &it->err);
}

bool carbon_object_it_unsigned_value(bool *is_null_in, u64 *value, carbon_object_it *it)
{
        return carbon_int_field_access_unsigned_value(is_null_in, value, &it->field.value.data, &it->err);
}

const char *carbon_object_it_string_value(u64 *strlen, carbon_object_it *it)
{
        return carbon_int_field_access_string_value(strlen, &it->field.value.data, &it->err);
}

bool carbon_object_it_binary_value(carbon_binary *out, carbon_object_it *it)
{
        return carbon_int_field_access_binary_value(out, &it->field.value.data, &it->err);
}

carbon_array_it *carbon_object_it_array_value(carbon_object_it *it_in)
{
        return carbon_int_field_access_array_value(&it_in->field.value.data, &it_in->err);
}

carbon_object_it *carbon_object_it_object_value(carbon_object_it *it_in)
{
        return carbon_int_field_access_object_value(&it_in->field.value.data, &it_in->err);
}

carbon_column_it *carbon_object_it_column_value(carbon_object_it *it_in)
{
        return carbon_int_field_access_column_value(&it_in->field.value.data, &it_in->err);
}

bool carbon_object_it_insert_begin(carbon_insert *inserter, carbon_object_it *it)
{
        ERROR_IF_NULL(inserter)
        ERROR_IF_NULL(it)
        return carbon_int_insert_create_for_object(inserter, it);
}

bool carbon_object_it_insert_end(carbon_insert *inserter)
{
        ERROR_IF_NULL(inserter)
        return carbon_insert_drop(inserter);
}

bool carbon_object_it_lock(carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        spinlock_acquire(&it->lock);
        return true;
}

bool carbon_object_it_unlock(carbon_object_it *it)
{
        ERROR_IF_NULL(it)
        spinlock_release(&it->lock);
        return true;
}

bool carbon_object_it_fast_forward(carbon_object_it *it)
{
        ERROR_IF_NULL(it);
        while (carbon_object_it_next(it)) {}

        JAK_ASSERT(*memfile_peek(&it->memfile, sizeof(u8)) == CARBON_MOBJECT_END);
        memfile_skip(&it->memfile, sizeof(u8));
        return true;
}