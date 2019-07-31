/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#include <ark-js/carbon/bison.h>
#include <ark-js/carbon/bison-object-it.h>
#include <ark-js/carbon/bison-array-it.h>
#include <ark-js/carbon/bison-column-it.h>
#include <ark-js/carbon/bison-insert.h>

NG5_EXPORT(bool) bison_object_it_create(struct bison_object_it *it, struct memfile *memfile, struct err *err,
        offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->payload_start = payload_start;
        spin_init(&it->lock);
        error_init(&it->err);

        vec_create(&it->history, NULL, sizeof(offset_t), 40);

        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_OBJECT_BEGIN, err, NG5_ERR_ILLEGALOP,
                "object begin marker ('{') not found");

        it->payload_start += sizeof(u8);

        bison_int_field_access_create(&it->field_access);

        bison_object_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_object_it_copy(struct bison_object_it *dst, struct bison_object_it *src)
{
        error_if_null(dst);
        error_if_null(src);
        bison_object_it_create(dst, &src->memfile, &src->err, src->payload_start - sizeof(u8));
        return true;
}

NG5_EXPORT(bool) bison_object_it_drop(struct bison_object_it *it)
{
        bison_int_field_auto_close(&it->field_access);
        bison_int_field_access_drop(&it->field_access);
        vec_drop(&it->history);
        return true;
}

NG5_EXPORT(bool) bison_object_it_rewind(struct bison_object_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        bison_int_history_clear(&it->history);
        return memfile_seek(&it->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_object_it_next(struct bison_object_it *it)
{
        error_if_null(it);
        bool is_empty_slot, is_object_end;
        offset_t last_off = memfile_tell(&it->memfile);
        if (bison_int_object_it_next(&is_empty_slot, &is_object_end, it)) {
                bison_int_history_push(&it->history, last_off);
                return true;
        } else {
                /* skip remaining zeros until end of array is reached */
                if (!is_object_end) {
                        error_if(!is_empty_slot, &it->err, NG5_ERR_CORRUPTED);

                        while (*memfile_peek(&it->memfile, 1) == 0) {
                                memfile_skip(&it->memfile, 1);
                        }
                }
                char final = *memfile_peek(&it->memfile, sizeof(char));
                assert( final == BISON_MARKER_OBJECT_END);
                return false;
        }
}

NG5_EXPORT(const char *) bison_object_it_prop_name(u64 *key_len, struct bison_object_it *it)
{
        error_if_null(it)
        error_if_null(key_len)
        *key_len = it->key_len;
        return it->key;
}

NG5_EXPORT(bool) bison_object_it_prop_type(enum bison_field_type *type, struct bison_object_it *it)
{
        return bison_int_field_access_field_type(type, &it->field_access);
}

NG5_EXPORT(bool) bison_object_it_u8_value(u8 *value, struct bison_object_it *it)
{
        return bison_int_field_access_u8_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_u16_value(u16 *value, struct bison_object_it *it)
{
        return bison_int_field_access_u16_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_u32_value(u32 *value, struct bison_object_it *it)
{
        return bison_int_field_access_u32_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_u64_value(u64 *value, struct bison_object_it *it)
{
        return bison_int_field_access_u64_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_i8_value(i8 *value, struct bison_object_it *it)
{
        return bison_int_field_access_i8_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_i16_value(i16 *value, struct bison_object_it *it)
{
        return bison_int_field_access_i16_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_i32_value(i32 *value, struct bison_object_it *it)
{
        return bison_int_field_access_i32_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_i64_value(i64 *value, struct bison_object_it *it)
{
        return bison_int_field_access_i64_value(value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_float_value(bool *is_null_in, float *value, struct bison_object_it *it)
{
        return bison_int_field_access_float_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_signed_value(bool *is_null_in, i64 *value, struct bison_object_it *it)
{
        return bison_int_field_access_signed_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_unsigned_value(bool *is_null_in, u64 *value, struct bison_object_it *it)
{
        return bison_int_field_access_unsigned_value(is_null_in, value, &it->field_access, &it->err);
}

NG5_EXPORT(const char *) bison_object_it_string_value(u64 *strlen, struct bison_object_it *it)
{
        return bison_int_field_access_string_value(strlen, &it->field_access, &it->err);
}

NG5_EXPORT(bool) bison_object_it_binary_value(struct bison_binary *out, struct bison_object_it *it)
{
        return bison_int_field_access_binary_value(out, &it->field_access, &it->err);
}

NG5_EXPORT(struct bison_array_it *) bison_object_it_array_value(struct bison_object_it *it_in)
{
        return bison_int_field_access_array_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(struct bison_object_it *) bison_object_it_object_value(struct bison_object_it *it_in)
{
        return bison_int_field_access_object_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(struct bison_column_it *) bison_object_it_column_value(struct bison_object_it *it_in)
{
        return bison_int_field_access_column_value(&it_in->field_access, &it_in->err);
}

NG5_EXPORT(bool) bison_object_it_insert_begin(struct bison_insert *inserter, struct bison_object_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_int_insert_create_for_object(inserter, it);
}

NG5_EXPORT(bool) bison_object_it_insert_end(struct bison_insert *inserter)
{
        error_if_null(inserter)
        return bison_insert_drop(inserter);
}

NG5_EXPORT(bool) bison_object_it_lock(struct bison_object_it *it)
{
        error_if_null(it)
        spin_acquire(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_object_it_unlock(struct bison_object_it *it)
{
        error_if_null(it)
        spin_release(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_object_it_fast_forward(struct bison_object_it *it)
{
        error_if_null(it);
        while (bison_object_it_next(it))
        { }
        char last = *memfile_peek(&it->memfile, sizeof(u8));
        assert(last == BISON_MARKER_OBJECT_END);
        memfile_skip(&it->memfile, sizeof(u8));
        return true;
}