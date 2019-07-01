/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in BISON
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

#include "core/bison/bison-column-it.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-int.h"

#define safe_cast(builtin_type, nvalues, it, field_type_expr)                                                          \
({                                                                                                                     \
        enum bison_field_type type;                                                                                    \
        const void *raw = bison_column_it_values(&type, nvalues, it);                                                  \
        error_if(!(field_type_expr), &it->err, NG5_ERR_TYPEMISMATCH);                                                  \
        (const builtin_type *) raw;                                                                                    \
})

NG5_EXPORT(bool) bison_column_it_create(struct bison_column_it *it, struct memfile *memfile, struct err *err,
        offset_t column_start_offset)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->column_start_offset = column_start_offset;
        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, column_start_offset);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8) + sizeof(media_type_t), err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_COLUMN_BEGIN, err, NG5_ERR_ILLEGALOP,
                "column begin marker ('(') not found");

        enum bison_field_type type = (enum bison_field_type) *memfile_read(&it->memfile, sizeof(media_type_t));
        it->type = type;

        it->column_num_elements_offset = memfile_tell(&it->memfile);
        it->column_num_elements = *memfile_read(&it->memfile, sizeof(u32));
        it->column_capacity_offset = memfile_tell(&it->memfile);
        it->column_capacity = *memfile_read(&it->memfile, sizeof(u32));

        /* header consists of column begin marker and contained element type */
        it->payload_start = memfile_tell(&it->memfile);
        bison_column_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_column_it_clone(struct bison_column_it *dst, struct bison_column_it *src)
{
        error_if_null(dst)
        error_if_null(src)

        bison_column_it_create(dst, &src->memfile, &src->err, src->column_start_offset);

        return true;
}

NG5_EXPORT(bool) bison_column_it_insert(struct bison_insert *inserter, struct bison_column_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_int_insert_create_for_column(inserter, it);
}

NG5_EXPORT(bool) bison_column_it_fast_forward(struct bison_column_it *it)
{
        error_if_null(it);
        bison_column_it_values(NULL, NULL, it);
        return true;
}

NG5_EXPORT(offset_t) bison_column_it_tell(struct bison_column_it *it)
{
        if (likely(it != NULL)) {
                return memfile_tell(&it->memfile);
        } else {
                error(&it->err, NG5_ERR_NULLPTR);
                return 0;
        }
}

NG5_EXPORT(bool) bison_column_it_values_info(enum bison_field_type *type, u32 *nvalues, struct bison_column_it *it)
{
        error_if_null(it);

        if (nvalues) {
                memfile_seek(&it->memfile, it->column_num_elements_offset);
                u32 num_elements = *NG5_MEMFILE_PEEK(&it->memfile, u32);
                *nvalues = num_elements;
        }

        ng5_optional_set(type, it->type);

        return true;
}

NG5_EXPORT(const void *) bison_column_it_values(enum bison_field_type *type, u32 *nvalues, struct bison_column_it *it)
{
        error_if_null(it);
        memfile_seek(&it->memfile, it->column_num_elements_offset);
        u32 num_elements = *NG5_MEMFILE_READ_TYPE(&it->memfile, u32);

        memfile_seek(&it->memfile, it->column_capacity_offset);
        u32 cap_elements = *NG5_MEMFILE_READ_TYPE(&it->memfile, u32);

        memfile_seek(&it->memfile, it->payload_start);
        const void *result = memfile_peek(&it->memfile, sizeof(void));

        ng5_optional_set(type, it->type);
        ng5_optional_set(nvalues, num_elements);

        u32 skip = cap_elements * bison_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, it->payload_start + skip);

        char end = *memfile_read(&it->memfile, sizeof(u8));

        error_if_and_return(end != BISON_MARKER_COLUMN_END, &it->err, NG5_ERR_CORRUPTED, NULL);

        return result;
}

NG5_EXPORT(const u8 *) bison_column_it_boolean_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(u8, nvalues, it, (type == BISON_FIELD_TYPE_TRUE || type == BISON_FIELD_TYPE_FALSE));
}

NG5_EXPORT(const u8 *) bison_column_it_u8_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(u8, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_U8));
}

NG5_EXPORT(const u16 *) bison_column_it_u16_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(u16, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_U16));
}

NG5_EXPORT(const u32 *) bison_column_it_u32_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(u32, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_U32));
}

NG5_EXPORT(const u64 *) bison_column_it_u64_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(u64, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_U64));
}

NG5_EXPORT(const i8 *) bison_column_it_i8_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(i8, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_I8));
}

NG5_EXPORT(const i16 *) bison_column_it_i16_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(i16, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_I16));
}

NG5_EXPORT(const i32 *) bison_column_it_i32_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(i32, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_I32));
}

NG5_EXPORT(const i64 *) bison_column_it_i64_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(i64, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_I64));
}

NG5_EXPORT(const float *) bison_column_it_float_values(u32 *nvalues, struct bison_column_it *it)
{
        return safe_cast(float, nvalues, it, (type == BISON_FIELD_TYPE_NUMBER_FLOAT));
}

NG5_EXPORT(bool) bison_column_it_remove(struct bison_column_it *it, u32 pos)
{
        error_if_null(it);

        error_if(pos >= it->column_num_elements, &it->err, NG5_ERR_OUTOFBOUNDS);
        memfile_save_position(&it->memfile);

        /* remove element */
        size_t elem_size = bison_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, it->payload_start + pos * elem_size);
        memfile_move_left(&it->memfile, elem_size);

        /* add an empty element at the end to restore the column capacity property */
        memfile_seek(&it->memfile, it->payload_start + it->column_num_elements * elem_size);
        memfile_move_right(&it->memfile, elem_size);

        /* update element counter */
        memfile_seek(&it->memfile, it->column_num_elements_offset);
        u32 num_elements = *NG5_MEMFILE_READ_TYPE(&it->memfile, u32);
        num_elements--;
        memfile_seek(&it->memfile, it->column_num_elements_offset);
        memfile_write(&it->memfile, &num_elements, sizeof(u32));

        memfile_restore_position(&it->memfile);

        return true;
}

NG5_EXPORT(bool) bison_column_it_update_set_null(struct bison_column_it *it, u32 pos)
{
        error_if_null(it)
        error_if(pos >= it->column_num_elements, &it->err, NG5_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        memfile_seek(&it->memfile, it->payload_start + pos * bison_int_get_type_value_size(it->type));

        switch (it->type) {
                case BISON_FIELD_TYPE_NULL:
                        /* nothing to do */
                        break;
                case BISON_FIELD_TYPE_TRUE:
                        memfile_write(&it->memfile, BOOLEAN_NULL, sizeof(u8));
                        break;
                case BISON_FIELD_TYPE_FALSE:
                        memfile_write(&it->memfile, BOOLEAN_NULL, sizeof(u8));
                        break;
                case BISON_FIELD_TYPE_NUMBER_U8: {
                        u8 null_value = U8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u8));
                } break;
                case BISON_FIELD_TYPE_NUMBER_U16: {
                        u16 null_value = U16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u16));
                } break;
                case BISON_FIELD_TYPE_NUMBER_U32: {
                        u32 null_value = U32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u32));
                } break;
                case BISON_FIELD_TYPE_NUMBER_U64: {
                        u64 null_value = U64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u64));
                } break;
                case BISON_FIELD_TYPE_NUMBER_I8: {
                        i8 null_value = I8_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i8));
                } break;
                case BISON_FIELD_TYPE_NUMBER_I16: {
                        i16 null_value = I16_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i16));
                } break;
                case BISON_FIELD_TYPE_NUMBER_I32: {
                        i32 null_value = I32_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i32));
                } break;
                case BISON_FIELD_TYPE_NUMBER_I64: {
                        i64 null_value = I64_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(i64));
                } break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT: {
                        float null_value = FLOAT_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(float));
                } break;
                case BISON_FIELD_TYPE_OBJECT:
                case BISON_FIELD_TYPE_ARRAY:
                case BISON_FIELD_TYPE_COLUMN:
                case BISON_FIELD_TYPE_STRING:
                case BISON_FIELD_TYPE_BINARY:
                case BISON_FIELD_TYPE_BINARY_CUSTOM:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, NG5_ERR_UNSUPPCONTAINER)
                        return false;
                default:
                        memfile_restore_position(&it->memfile);
                        error(&it->err, NG5_ERR_INTERNALERR);
                        return false;
        }

        memfile_restore_position(&it->memfile);

        return true;
}

NG5_EXPORT(bool) bison_column_it_update_set_true(struct bison_column_it *it, u32 pos)
{
        unused(it)
        unused(pos)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_false(struct bison_column_it *it, u32 pos)
{
        unused(it)
        unused(pos)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_u8(struct bison_column_it *it, u32 pos, u8 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_u16(struct bison_column_it *it, u32 pos, u16 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_u32(struct bison_column_it *it, u32 pos, u32 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_u64(struct bison_column_it *it, u32 pos, u64 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_i8(struct bison_column_it *it, u32 pos, i8 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_i16(struct bison_column_it *it, u32 pos, i16 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_i32(struct bison_column_it *it, u32 pos, i32 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_i64(struct bison_column_it *it, u32 pos, i64 value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

NG5_EXPORT(bool) bison_column_it_update_set_float(struct bison_column_it *it, u32 pos, float value)
{
        unused(it)
        unused(pos)
        unused(value)
        error_print(NG5_ERR_NOTIMPLEMENTED); // TODO: implement
        return false;
}

/**
 * Locks the iterator with a spinlock. A call to <code>bison_column_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_column_it_lock(struct bison_column_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_column_it_unlock(struct bison_column_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_column_it_rewind(struct bison_column_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->memfile, it->payload_start);
}