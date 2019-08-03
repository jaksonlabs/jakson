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

#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-object-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-string.h>
#include <ark-js/carbon/carbon-prop.h>

ARK_EXPORT(bool) carbon_object_it_create(struct carbon_object_it *it, struct memfile *memfile, struct err *err,
        offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->payload_start = payload_start;
        it->mod_size = 0;
        it->object_end_reached = false;

        spin_init(&it->lock);
        error_init(&it->err);

        vec_create(&it->history, NULL, sizeof(offset_t), 40);

        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), err, ARK_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != CARBON_MARKER_OBJECT_BEGIN, err, ARK_ERR_ILLEGALOP,
                "object begin marker ('{') not found");

        it->payload_start += sizeof(u8);

        carbon_int_field_access_create(&it->field_access);

        carbon_object_it_rewind(it);

        return true;
}

ARK_EXPORT(bool) carbon_object_it_copy(struct carbon_object_it *dst, struct carbon_object_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        carbon_object_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(u8));
        return true;
}

ARK_EXPORT(bool) carbon_object_it_clone(struct carbon_object_it *dst, struct carbon_object_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        memfile_clone(&dst->memfile, &src->memfile);
        dst->payload_start = src->payload_start;
        spin_init(&dst->lock);
        error_cpy(&dst->err, &src->err);
        dst->mod_size = src->mod_size;
        dst->object_end_reached = src->object_end_reached;
        vec_cpy(&dst->history, &src->history);
        dst->key_len = src->key_len;
        dst->key = src->key;
        dst->value_off = src->value_off;
        carbon_int_field_access_clone(&dst->field_access, &src->field_access);
        return true;
}

ARK_EXPORT(bool) carbon_object_it_drop(struct carbon_object_it *it)
{
        carbon_int_field_auto_close(&it->field_access);
        carbon_int_field_access_drop(&it->field_access);
        vec_drop(&it->history);
        return true;
}

ARK_EXPORT(bool) carbon_object_it_rewind(struct carbon_object_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, ARK_ERR_OUTOFBOUNDS);
        carbon_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->payload_start);
}

ARK_EXPORT(bool) carbon_object_it_next(struct carbon_object_it *it)
{
        error_if_null(it);
        bool is_empty_slot;
        offset_t last_off = memfile_tell(&it->memfile);
        if (carbon_int_object_it_next(&is_empty_slot, &it->object_end_reached, it)) {
                carbon_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!it->object_end_reached) {
                        error_if(!is_empty_slot, &it->err, ARK_ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }

                memfile_hexdump_printf(stderr, &it->memfile); // TODO: Debug Remove
                assert(*memfile_peek(&it->memfile, sizeof(char)) == CARBON_MARKER_OBJECT_END);
                return false;
        }
}

ARK_EXPORT(offset_t) carbon_object_it_tell(struct carbon_object_it *it)
{
        error_if_null(it)
        return memfile_tell(&it->memfile);
}

ARK_EXPORT(const char *) carbon_object_it_prop_name(u64 *key_len, struct carbon_object_it *it)
{
        error_if_null(it)
        error_if_null(key_len)
        *key_len = it->key_len;
        return it->key;
}

static i64 prop_remove(struct carbon_object_it *it, enum carbon_field_type type)
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

ARK_EXPORT(bool) carbon_object_it_remove(struct carbon_object_it *it)
{
        error_if_null(it);
        enum carbon_field_type type;
        if (carbon_object_it_prop_type(&type, it)) {
                offset_t prop_off = carbon_int_history_pop(&it->history);
                memfile_seek(&it->memfile, prop_off);
                it->mod_size -= prop_remove(it, type);
                return true;
        } else {
                error(&it->err, ARK_ERR_ILLEGALSTATE);
                return false;
        }
}

ARK_EXPORT(bool) carbon_object_it_prop_type(enum carbon_field_type *type, struct carbon_object_it *it)
{
        return carbon_int_field_access_field_type(type, &it->field_access);
}

ARK_EXPORT(bool) carbon_object_it_u8_value(u8 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_u8_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_u16_value(u16 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_u16_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_u32_value(u32 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_u32_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_u64_value(u64 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_u64_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_i8_value(i8 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_i8_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_i16_value(i16 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_i16_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_i32_value(i32 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_i32_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_i64_value(i64 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_i64_value(value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_float_value(bool *is_null_in, float *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_signed_value(bool *is_null_in, i64 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_unsigned_value(bool *is_null_in, u64 *value, struct carbon_object_it *it)
{
        return carbon_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

ARK_EXPORT(const char *) carbon_object_it_string_value(u64 *strlen, struct carbon_object_it *it)
{
        return carbon_int_field_access_string_value(strlen, &it->field_access, &it->err);
}

ARK_EXPORT(bool) carbon_object_it_binary_value(struct carbon_binary *out, struct carbon_object_it *it)
{
        return carbon_int_field_access_binary_value(out, &it->field_access, &it->err);
}

ARK_EXPORT(struct carbon_array_it *) carbon_object_it_array_value(struct carbon_object_it *it_in)
{
        return carbon_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(struct carbon_object_it *) carbon_object_it_object_value(struct carbon_object_it *it_in)
{
        return carbon_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(struct carbon_column_it *) carbon_object_it_column_value(struct carbon_object_it *it_in)
{
        return carbon_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

ARK_EXPORT(bool) carbon_object_it_insert_begin(struct carbon_insert *inserter, struct carbon_object_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return carbon_int_insert_create_for_object(inserter, it);
}

ARK_EXPORT(bool) carbon_object_it_insert_end(struct carbon_insert *inserter)
{
        error_if_null(inserter)
        return carbon_insert_drop(inserter);
}

ARK_EXPORT(bool) carbon_object_it_lock(struct carbon_object_it *it)
{
        error_if_null(it)
        spin_acquire(&it->lock);
        return true;
}

ARK_EXPORT(bool) carbon_object_it_unlock(struct carbon_object_it *it)
{
        error_if_null(it)
        spin_release(&it->lock);
        return true;
}

ARK_EXPORT(bool) carbon_object_it_fast_forward(struct carbon_object_it *it)
{
        error_if_null(it);
        while (carbon_object_it_next(it))
        { }

        assert(*memfile_peek(&it->memfile, sizeof(u8)) == CARBON_MARKER_OBJECT_END);
        memfile_skip(&it->memfile, sizeof(u8));
        return true;
}