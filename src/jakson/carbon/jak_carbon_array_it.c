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

#include <jakson/std/uintvar/jak_uintvar_stream.h>
#include <jakson/carbon.h>
#include <jakson/carbon/jak_carbon_array_it.h>
#include <jakson/carbon/jak_carbon_column_it.h>
#include <jakson/carbon/jak_carbon_object_it.h>
#include <jakson/carbon/jak_carbon_insert.h>
#include <jakson/carbon/jak_carbon_media.h>
#include <jakson/carbon/jak_carbon_int.h>

#define DEFINE_IN_PLACE_UPDATE_FUNCTION(type_name, field_type)                                                         \
bool jak_carbon_array_it_update_in_place_##type_name(jak_carbon_array_it *it, jak_##type_name value)                \
{                                                                                                                      \
        jak_offset_t datum = 0;                                                                                                \
        JAK_ERROR_IF_NULL(it);                                                                                             \
        if (JAK_LIKELY(it->field_access.it_field_type == field_type)) {                                                    \
                jak_memfile_save_position(&it->memfile);                                                                   \
                jak_carbon_int_array_it_offset(&datum, it);                                                                 \
                jak_memfile_seek(&it->memfile, datum + sizeof(jak_u8));                                                        \
                jak_memfile_write(&it->memfile, &value, sizeof(jak_##type_name));                                                \
                jak_memfile_restore_position(&it->memfile);                                                                \
                return true;                                                                                           \
        } else {                                                                                                       \
                JAK_ERROR(&it->err, JAK_ERR_TYPEMISMATCH);                                                                 \
                return false;                                                                                          \
        }                                                                                                              \
}

DEFINE_IN_PLACE_UPDATE_FUNCTION(u8, CARBON_FIELD_NUMBER_U8)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u16, CARBON_FIELD_NUMBER_U16)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u32, CARBON_FIELD_NUMBER_U32)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u64, CARBON_FIELD_NUMBER_U64)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i8, CARBON_FIELD_NUMBER_I8)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i16, CARBON_FIELD_NUMBER_I16)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i32, CARBON_FIELD_NUMBER_I32)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i64, CARBON_FIELD_NUMBER_I64)

DEFINE_IN_PLACE_UPDATE_FUNCTION(float, CARBON_FIELD_NUMBER_FLOAT)

static bool update_in_place_constant(jak_carbon_array_it *it, jak_carbon_constant_e constant)
{
        JAK_ERROR_IF_NULL(it);

        jak_memfile_save_position(&it->memfile);

        if (jak_carbon_field_type_is_constant(it->field_access.it_field_type)) {
                jak_u8 value;
                switch (constant) {
                        case JAK_CARBON_CONSTANT_TRUE:
                                value = CARBON_FIELD_TRUE;
                                break;
                        case JAK_CARBON_CONSTANT_FALSE:
                                value = CARBON_FIELD_FALSE;
                                break;
                        case JAK_CARBON_CONSTANT_NULL:
                                value = CARBON_FIELD_NULL;
                                break;
                        default: JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                                break;
                }
                jak_offset_t datum = 0;
                jak_carbon_int_array_it_offset(&datum, it);
                jak_memfile_seek(&it->memfile, datum);
                jak_memfile_write(&it->memfile, &value, sizeof(jak_u8));
        } else {
                jak_carbon_insert ins;
                jak_carbon_array_it_remove(it);
                jak_carbon_array_it_insert_begin(&ins, it);

                switch (constant) {
                        case JAK_CARBON_CONSTANT_TRUE:
                                jak_carbon_insert_true(&ins);
                                break;
                        case JAK_CARBON_CONSTANT_FALSE:
                                jak_carbon_insert_false(&ins);
                                break;
                        case JAK_CARBON_CONSTANT_NULL:
                                jak_carbon_insert_null(&ins);
                                break;
                        default: JAK_ERROR(&it->err, JAK_ERR_INTERNALERR);
                                break;
                }

                jak_carbon_array_it_insert_end(&ins);
        }

        jak_memfile_restore_position(&it->memfile);
        return true;
}

bool jak_carbon_array_it_update_in_place_true(jak_carbon_array_it *it)
{
        return update_in_place_constant(it, JAK_CARBON_CONSTANT_TRUE);
}

bool jak_carbon_array_it_update_in_place_false(jak_carbon_array_it *it)
{
        return update_in_place_constant(it, JAK_CARBON_CONSTANT_FALSE);
}

bool jak_carbon_array_it_update_in_place_null(jak_carbon_array_it *it)
{
        return update_in_place_constant(it, JAK_CARBON_CONSTANT_NULL);
}

bool jak_carbon_array_it_create(jak_carbon_array_it *it, jak_memfile *memfile, jak_error *err,
                            jak_offset_t payload_start)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF_NULL(memfile);
        JAK_ERROR_IF_NULL(err);

        JAK_ZERO_MEMORY(it, sizeof(jak_carbon_array_it));

        it->payload_start = payload_start;
        it->mod_size = 0;
        it->array_end_reached = false;
        it->field_offset = 0;

        jak_error_init(&it->err);
        jak_spinlock_init(&it->lock);
        jak_vector_create(&it->history, NULL, sizeof(jak_offset_t), 40);
        jak_memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        jak_memfile_seek(&it->memfile, payload_start);

        JAK_ERROR_IF(jak_memfile_remain_size(&it->memfile) < sizeof(jak_u8), err, JAK_ERR_CORRUPTED);

        jak_u8 marker = *jak_memfile_read(&it->memfile, sizeof(jak_u8));
        JAK_ERROR_IF_WDETAILS(marker != CARBON_MARRAY_BEGIN, err, JAK_ERR_ILLEGALOP,
                              "array begin marker ('[') not found");

        it->payload_start += sizeof(jak_u8);

        jak_carbon_int_field_access_create(&it->field_access);

        jak_carbon_array_it_rewind(it);

        return true;
}

bool jak_carbon_array_it_copy(jak_carbon_array_it *dst, jak_carbon_array_it *src)
{
        JAK_ERROR_IF_NULL(dst);
        JAK_ERROR_IF_NULL(src);
        jak_carbon_array_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(jak_u8));
        return true;
}

bool jak_carbon_array_it_clone(jak_carbon_array_it *dst, jak_carbon_array_it *src)
{
        jak_memfile_clone(&dst->memfile, &src->memfile);
        dst->payload_start = src->payload_start;
        jak_spinlock_init(&dst->lock);
        jak_error_cpy(&dst->err, &src->err);
        dst->mod_size = src->mod_size;
        dst->array_end_reached = src->array_end_reached;
        jak_vector_cpy(&dst->history, &src->history);
        jak_carbon_int_field_access_clone(&dst->field_access, &src->field_access);
        dst->field_offset = src->field_offset;
        return true;
}

bool jak_carbon_array_it_readonly(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        it->memfile.mode = JAK_READ_ONLY;
        return true;
}

bool jak_carbon_array_it_length(jak_u64 *len, jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(len)
        JAK_ERROR_IF_NULL(it)

        jak_u64 num_elem = 0;
        jak_carbon_array_it_rewind(it);
        while (jak_carbon_array_it_next(it)) {
                num_elem++;
        }
        *len = num_elem;

        return true;
}

bool jak_carbon_array_it_is_empty(jak_carbon_array_it *it)
{
        jak_carbon_array_it_rewind(it);
        return jak_carbon_array_it_next(it);
}

bool jak_carbon_array_it_drop(jak_carbon_array_it *it)
{
        jak_carbon_int_field_auto_close(&it->field_access);
        jak_carbon_int_field_access_drop(&it->field_access);
        jak_vector_drop(&it->history);
        return true;
}

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_array_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_array_it_lock(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_spinlock_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
bool jak_carbon_array_it_unlock(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_spinlock_release(&it->lock);
        return true;
}

bool jak_carbon_array_it_rewind(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        JAK_ERROR_IF(it->payload_start >= jak_memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        jak_carbon_int_history_clear(&it->history);
        return jak_memfile_seek(&it->memfile, it->payload_start);
}

static void auto_adjust_pos_after_mod(jak_carbon_array_it *it)
{
        if (jak_carbon_int_field_access_object_it_opened(&it->field_access)) {
                jak_memfile_skip(&it->memfile, it->field_access.nested_object_it->mod_size);
        } else if (jak_carbon_int_field_access_array_it_opened(&it->field_access)) {
                //jak_memfile_skip(&it->memfile, it->field_access.nested_array_it->mod_size);
                //abort(); // TODO: implement!
        }
}

bool jak_carbon_array_it_has_next(jak_carbon_array_it *it)
{
        bool has_next = jak_carbon_array_it_next(it);
        jak_carbon_array_it_prev(it);
        return has_next;
}

bool jak_carbon_array_it_is_unit(jak_carbon_array_it *it)
{
        bool has_next = jak_carbon_array_it_next(it);
        if (has_next) {
                has_next = jak_carbon_array_it_next(it);
                jak_carbon_array_it_prev(it);
                jak_carbon_array_it_prev(it);
                return !has_next;
        }
        jak_carbon_array_it_prev(it);
        return false;
}

bool jak_carbon_array_it_next(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        bool is_empty_slot = true;

        auto_adjust_pos_after_mod(it);
        jak_offset_t last_off = jak_memfile_tell(&it->memfile);

        if (jak_carbon_int_array_it_next(&is_empty_slot, &it->array_end_reached, it)) {
                jak_carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!it->array_end_reached) {
                        JAK_ERROR_IF(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);

                        while (*jak_memfile_peek(&it->memfile, 1) == 0) {
                                jak_memfile_skip(&it->memfile, 1);
                        }
                }
                JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARRAY_END);
                jak_carbon_int_field_auto_close(&it->field_access);
                return false;
        }
}

bool jak_carbon_array_it_prev(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        if (jak_carbon_int_history_has(&it->history)) {
                jak_offset_t prev_off = jak_carbon_int_history_pop(&it->history);
                jak_memfile_seek(&it->memfile, prev_off);
                return jak_carbon_int_array_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

jak_offset_t jak_carbon_array_it_memfilepos(jak_carbon_array_it *it)
{
        if (JAK_LIKELY(it != NULL)) {
                return jak_memfile_tell(&it->memfile);
        } else {
                JAK_ERROR(&it->err, JAK_ERR_NULLPTR);
                return 0;
        }
}

jak_offset_t jak_carbon_array_it_tell(jak_carbon_array_it *it)
{
        return it ? it->field_offset : 0;
}

bool jak_carbon_int_array_it_offset(jak_offset_t *off, jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(off)
        JAK_ERROR_IF_NULL(it)
        if (jak_carbon_int_history_has(&it->history)) {
                *off = jak_carbon_int_history_peek(&it->history);
                return true;
        }
        return false;
}

bool jak_carbon_array_it_fast_forward(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        while (jak_carbon_array_it_next(it)) {}

        JAK_ASSERT(*jak_memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARRAY_END);
        jak_memfile_skip(&it->memfile, sizeof(char));
        return true;
}

bool jak_carbon_array_it_field_type(jak_carbon_field_type_e *type, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_field_type(type, &it->field_access);
}

bool jak_carbon_array_it_u8_value(jak_u8 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_u8_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_u16_value(jak_u16 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_u16_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_u32_value(jak_u32 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_u32_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_u64_value(jak_u64 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_u64_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_i8_value(jak_i8 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_i8_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_i16_value(jak_i16 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_i16_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_i32_value(jak_i32 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_i32_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_i64_value(jak_i64 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_i64_value(value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_float_value(bool *is_null_in, float *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

bool jak_carbon_array_it_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

const char *jak_carbon_array_it_jak_string_value(jak_u64 *strlen, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_jak_string_value(strlen, &it->field_access, &it->err);
}

bool jak_carbon_array_it_binary_value(jak_carbon_binary *out, jak_carbon_array_it *it)
{
        return jak_carbon_int_field_access_binary_value(out, &it->field_access, &it->err);
}

jak_carbon_array_it *jak_carbon_array_it_array_value(jak_carbon_array_it *it_in)
{
        return jak_carbon_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

jak_carbon_object_it *jak_carbon_array_it_object_value(jak_carbon_array_it *it_in)
{
        return jak_carbon_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

jak_carbon_column_it *jak_carbon_array_it_column_value(jak_carbon_array_it *it_in)
{
        return jak_carbon_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

bool jak_carbon_array_it_insert_begin(jak_carbon_insert *inserter, jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(inserter)
        JAK_ERROR_IF_NULL(it)
        return jak_carbon_int_insert_create_for_array(inserter, it);
}

bool jak_carbon_array_it_insert_end(jak_carbon_insert *inserter)
{
        JAK_ERROR_IF_NULL(inserter)
        return jak_carbon_insert_drop(inserter);
}

bool jak_carbon_array_it_remove(jak_carbon_array_it *it)
{
        JAK_ERROR_IF_NULL(it);
        jak_carbon_field_type_e type;
        if (jak_carbon_array_it_field_type(&type, it)) {
                jak_offset_t prev_off = jak_carbon_int_history_pop(&it->history);
                jak_memfile_seek(&it->memfile, prev_off);
                if (jak_carbon_int_field_remove(&it->memfile, &it->err, type)) {
                        jak_carbon_int_array_it_refresh(NULL, NULL, it);
                        return true;
                } else {
                        return false;
                }
        } else {
                JAK_ERROR(&it->err, JAK_ERR_ILLEGALSTATE);
                return false;
        }
}