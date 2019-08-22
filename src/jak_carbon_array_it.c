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

#include <jak_uintvar_stream.h>
#include <jak_carbon.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_object_it.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_media.h>
#include <jak_carbon_int.h>

#define DEFINE_IN_PLACE_UPDATE_FUNCTION(type_name, field_type)                                                         \
bool carbon_array_it_update_in_place_##type_name(struct jak_carbon_array_it *it, type_name value)                \
{                                                                                                                      \
        offset_t datum = 0;                                                                                                \
        error_if_null(it);                                                                                             \
        if (likely(it->field_access.it_field_type == field_type)) {                                                    \
                memfile_save_position(&it->memfile);                                                                   \
                carbon_int_array_it_offset(&datum, it);                                                                 \
                memfile_seek(&it->memfile, datum + sizeof(u8));                                                        \
                memfile_write(&it->memfile, &value, sizeof(type_name));                                                \
                memfile_restore_position(&it->memfile);                                                                \
                return true;                                                                                           \
        } else {                                                                                                       \
                error(&it->err, JAK_ERR_TYPEMISMATCH);                                                                 \
                return false;                                                                                          \
        }                                                                                                              \
}

DEFINE_IN_PLACE_UPDATE_FUNCTION(u8, CARBON_FIELD_TYPE_NUMBER_U8)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u16, CARBON_FIELD_TYPE_NUMBER_U16)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u32, CARBON_FIELD_TYPE_NUMBER_U32)

DEFINE_IN_PLACE_UPDATE_FUNCTION(u64, CARBON_FIELD_TYPE_NUMBER_U64)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i8, CARBON_FIELD_TYPE_NUMBER_I8)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i16, CARBON_FIELD_TYPE_NUMBER_I16)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i32, CARBON_FIELD_TYPE_NUMBER_I32)

DEFINE_IN_PLACE_UPDATE_FUNCTION(i64, CARBON_FIELD_TYPE_NUMBER_I64)

DEFINE_IN_PLACE_UPDATE_FUNCTION(float, CARBON_FIELD_TYPE_NUMBER_FLOAT)

static bool update_in_place_constant(struct jak_carbon_array_it *it, enum carbon_constant constant)
{
        error_if_null(it);

        memfile_save_position(&it->memfile);

        if (carbon_field_type_is_constant(it->field_access.it_field_type)) {
                u8 value;
                switch (constant) {
                        case CARBON_CONSTANT_TRUE:
                                value = CARBON_FIELD_TYPE_TRUE;
                                break;
                        case CARBON_CONSTANT_FALSE:
                                value = CARBON_FIELD_TYPE_FALSE;
                                break;
                        case CARBON_CONSTANT_NULL:
                                value = CARBON_FIELD_TYPE_NULL;
                                break;
                        default: error(&it->err, JAK_ERR_INTERNALERR);
                                break;
                }
                offset_t datum = 0;
                carbon_int_array_it_offset(&datum, it);
                memfile_seek(&it->memfile, datum);
                memfile_write(&it->memfile, &value, sizeof(u8));
        } else {
                struct jak_carbon_insert ins;
                carbon_array_it_remove(it);
                carbon_array_it_insert_begin(&ins, it);

                switch (constant) {
                        case CARBON_CONSTANT_TRUE:
                                carbon_insert_true(&ins);
                                break;
                        case CARBON_CONSTANT_FALSE:
                                carbon_insert_false(&ins);
                                break;
                        case CARBON_CONSTANT_NULL:
                                carbon_insert_null(&ins);
                                break;
                        default: error(&it->err, JAK_ERR_INTERNALERR);
                                break;
                }

                carbon_array_it_insert_end(&ins);
        }

        memfile_restore_position(&it->memfile);
        return true;
}

bool carbon_array_it_update_in_place_true(struct jak_carbon_array_it *it)
{
        return update_in_place_constant(it, CARBON_CONSTANT_TRUE);
}

bool carbon_array_it_update_in_place_false(struct jak_carbon_array_it *it)
{
        return update_in_place_constant(it, CARBON_CONSTANT_FALSE);
}

bool carbon_array_it_update_in_place_null(struct jak_carbon_array_it *it)
{
        return update_in_place_constant(it, CARBON_CONSTANT_NULL);
}

bool carbon_array_it_create(struct jak_carbon_array_it *it, struct memfile *memfile, struct err *err,
                            offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        JAK_zero_memory(it, sizeof(struct jak_carbon_array_it));

        it->payload_start = payload_start;
        it->mod_size = 0;
        it->array_end_reached = false;
        it->field_offset = 0;

        error_init(&it->err);
        spin_init(&it->lock);
        vec_create(&it->history, NULL, sizeof(offset_t), 40);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), err, JAK_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != JAK_CARBON_MARKER_ARRAY_BEGIN, err, JAK_ERR_ILLEGALOP,
                              "array begin marker ('[') not found");

        it->payload_start += sizeof(u8);

        carbon_int_field_access_create(&it->field_access);

        carbon_array_it_rewind(it);

        return true;
}

bool carbon_array_it_copy(struct jak_carbon_array_it *dst, struct jak_carbon_array_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        carbon_array_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(u8));
        return true;
}

bool carbon_array_it_clone(struct jak_carbon_array_it *dst, struct jak_carbon_array_it *src)
{
        memfile_clone(&dst->memfile, &src->memfile);
        dst->payload_start = src->payload_start;
        spin_init(&dst->lock);
        error_cpy(&dst->err, &src->err);
        dst->mod_size = src->mod_size;
        dst->array_end_reached = src->array_end_reached;
        vec_cpy(&dst->history, &src->history);
        carbon_int_field_access_clone(&dst->field_access, &src->field_access);
        dst->field_offset = src->field_offset;
        return true;
}

bool carbon_array_it_readonly(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        it->memfile.mode = READ_ONLY;
        return true;
}

bool carbon_array_it_length(u64 *len, struct jak_carbon_array_it *it)
{
        error_if_null(len)
        error_if_null(it)

        u64 num_elem = 0;
        carbon_array_it_rewind(it);
        while (carbon_array_it_next(it)) {
                num_elem++;
        }
        *len = num_elem;

        return true;
}

bool carbon_array_it_is_empty(struct jak_carbon_array_it *it)
{
        carbon_array_it_rewind(it);
        return carbon_array_it_next(it);
}

bool carbon_array_it_drop(struct jak_carbon_array_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        carbon_int_field_access_drop(&it->field_access);
        vec_drop(&it->history);
        return true;
}

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_array_it_unlock</code> is required for unlocking.
 */
bool carbon_array_it_lock(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
bool carbon_array_it_unlock(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

bool carbon_array_it_rewind(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, JAK_ERR_OUTOFBOUNDS);
        carbon_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->payload_start);
}

static void auto_adjust_pos_after_mod(struct jak_carbon_array_it *it)
{
        if (carbon_int_field_access_object_it_opened(&it->field_access)) {
                memfile_skip(&it->memfile, it->field_access.nested_object_it->mod_size);
        } else if (carbon_int_field_access_array_it_opened(&it->field_access)) {
                //memfile_skip(&it->memfile, it->field_access.nested_array_it->mod_size);
                //abort(); // TODO: implement!
        }
}

bool carbon_array_it_has_next(struct jak_carbon_array_it *it)
{
        bool has_next = carbon_array_it_next(it);
        carbon_array_it_prev(it);
        return has_next;
}

bool carbon_array_it_is_unit(struct jak_carbon_array_it *it)
{
        bool has_next = carbon_array_it_next(it);
        if (has_next) {
                has_next = carbon_array_it_next(it);
                carbon_array_it_prev(it);
                carbon_array_it_prev(it);
                return !has_next;
        }
        carbon_array_it_prev(it);
        return false;
}

bool carbon_array_it_next(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        bool is_empty_slot = true;

        auto_adjust_pos_after_mod(it);
        offset_t last_off = memfile_tell(&it->memfile);

        if (carbon_int_array_it_next(&is_empty_slot, &it->array_end_reached, it)) {
                carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!it->array_end_reached) {
                        error_if(!is_empty_slot, &it->err, JAK_ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }
                assert(*memfile_peek(&it->memfile, sizeof(char)) == JAK_CARBON_MARKER_ARRAY_END);
                carbon_int_field_auto_close(&it->field_access);
                return false;
        }
}

bool carbon_array_it_prev(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        if (carbon_int_history_has(&it->history)) {
                offset_t prev_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                return carbon_int_array_it_refresh(NULL, NULL, it);
        } else {
                return false;
        }
}

offset_t carbon_array_it_memfilepos(struct jak_carbon_array_it *it)
{
        if (likely(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, JAK_ERR_NULLPTR);
                return 0;
        }
}

offset_t carbon_array_it_tell(struct jak_carbon_array_it *it)
{
        return it ? it->field_offset : 0;
}

bool carbon_int_array_it_offset(offset_t *off, struct jak_carbon_array_it *it)
{
        error_if_null(off)
        error_if_null(it)
        if (carbon_int_history_has(&it->history)) {
                *off = carbon_int_history_peek(&it->history);
                return true;
        }
        return false;
}

bool carbon_array_it_fast_forward(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        while (carbon_array_it_next(it)) {}

        assert(*memfile_peek(&it->memfile, sizeof(char)) == JAK_CARBON_MARKER_ARRAY_END);
        memfile_skip(&it->memfile, sizeof(char));
        return true;
}

bool carbon_array_it_field_type(enum carbon_field_type *type, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_field_type(type, &it->field_access);
}

bool carbon_array_it_u8_value(u8 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_u8_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_u16_value(u16 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_u16_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_u32_value(u32 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_u32_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_u64_value(u64 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_u64_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_i8_value(i8 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_i8_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_i16_value(i16 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_i16_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_i32_value(i32 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_i32_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_i64_value(i64 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_i64_value(value, &it->field_access, &it->err);
}

bool carbon_array_it_float_value(bool *is_null_in, float *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

bool carbon_array_it_signed_value(bool *is_null_in, i64 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

bool carbon_array_it_unsigned_value(bool *is_null_in, u64 *value, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

const char *carbon_array_it_string_value(u64 *strlen, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_string_value(strlen, &it->field_access, &it->err);
}

bool carbon_array_it_binary_value(struct jak_carbon_binary *out, struct jak_carbon_array_it *it)
{
        return carbon_int_field_access_binary_value(out, &it->field_access, &it->err);
}

struct jak_carbon_array_it *carbon_array_it_array_value(struct jak_carbon_array_it *it_in)
{
        return carbon_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

struct jak_carbon_object_it *carbon_array_it_object_value(struct jak_carbon_array_it *it_in)
{
        return carbon_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

struct jak_carbon_column_it *carbon_array_it_column_value(struct jak_carbon_array_it *it_in)
{
        return carbon_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

bool carbon_array_it_insert_begin(struct jak_carbon_insert *inserter, struct jak_carbon_array_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return carbon_int_insert_create_for_array(inserter, it);
}

bool carbon_array_it_insert_end(struct jak_carbon_insert *inserter)
{
        error_if_null(inserter)
        return carbon_insert_drop(inserter);
}

bool carbon_array_it_remove(struct jak_carbon_array_it *it)
{
        error_if_null(it);
        enum carbon_field_type type;
        if (carbon_array_it_field_type(&type, it)) {
                offset_t prev_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prev_off);
                if (carbon_int_field_remove(&it->memfile, &it->err, type)) {
                        carbon_int_array_it_refresh(NULL, NULL, it);
                        return true;
                } else {
                        return false;
                }
        } else {
                error(&it->err, JAK_ERR_ILLEGALSTATE);
                return false;
        }
}