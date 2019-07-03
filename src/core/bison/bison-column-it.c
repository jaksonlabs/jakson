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
#include "core/bison/bison-array-it.h"
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
                case BISON_FIELD_TYPE_FALSE: {
                        u8 null_value = BISON_BOOLEAN_COLUMN_NULL;
                        memfile_write(&it->memfile, &null_value, sizeof(u8));
                } break;
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

#define push_array_element(num_values, data, data_cast_type, null_check, insert_func)                                  \
for (u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (likely(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins);                                                                               \
        } else {                                                                                                       \
                bison_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

#define push_array_element_wvalue(num_values, data, data_cast_type, null_check, insert_func)                           \
for (u32 i = 0; i < num_values; i++) {                                                                                 \
        data_cast_type datum = ((data_cast_type *)data)[i];                                                            \
        if (likely(null_check(datum) == false)) {                                                                      \
                insert_func(&array_ins, datum);                                                                        \
        } else {                                                                                                       \
                bison_insert_null(&array_ins);                                                                         \
        }                                                                                                              \
}

static bool rewrite_column_to_array(struct bison_column_it *it)
{
        struct bison_array_it array_it;
        struct bison_insert array_ins;

        memfile_save_position(&it->memfile);

        printf("\n\n");
        memfile_hexdump_print(&it->memfile); // TODO: debug remove

        /* Potentially tailing space after the last ']' marker of the outer most array is used for temporary space */
        memfile_seek_to_end(&it->memfile);
        offset_t array_marker_begin = memfile_tell(&it->memfile);

        size_t capacity = it->column_num_elements * bison_int_get_type_value_size(it->type);
        bison_int_insert_array(&it->memfile, capacity);
        bison_array_it_create(&array_it, &it->memfile, &it->err, array_marker_begin);
        bison_array_it_insert_begin(&array_ins, &array_it);

        printf("\n\n");
        memfile_hexdump_print(&it->memfile); // TODO: debug remove

        enum bison_field_type type;
        u32 num_values;
        const void *data = bison_column_it_values(&type, &num_values, it);
        switch (type) {
                case BISON_FIELD_TYPE_NULL:
                        while (num_values--) {
                                bison_insert_null(&array_ins);
                        }
                        break;
                case BISON_FIELD_TYPE_TRUE:
                        push_array_element(num_values, data, u8, is_null_boolean, bison_insert_true);
                        break;
                case BISON_FIELD_TYPE_FALSE:
                        push_array_element(num_values, data, u8, is_null_boolean, bison_insert_false);
                break;
                case BISON_FIELD_TYPE_NUMBER_U8:
                        push_array_element_wvalue(num_values, data, u8, is_null_u8, bison_insert_u8);
                break;
                case BISON_FIELD_TYPE_NUMBER_U16:
                        push_array_element_wvalue(num_values, data, u16, is_null_u16, bison_insert_u16);
                break;
                case BISON_FIELD_TYPE_NUMBER_U32:
                        push_array_element_wvalue(num_values, data, u32, is_null_u32, bison_insert_u32);
                break;
                case BISON_FIELD_TYPE_NUMBER_U64:
                        push_array_element_wvalue(num_values, data, u64, is_null_u64, bison_insert_u64);
                break;
                case BISON_FIELD_TYPE_NUMBER_I8:
                        push_array_element_wvalue(num_values, data, i8, is_null_i8, bison_insert_i8);
                break;
                case BISON_FIELD_TYPE_NUMBER_I16:
                        push_array_element_wvalue(num_values, data, i16, is_null_i16, bison_insert_i16);
                break;
                case BISON_FIELD_TYPE_NUMBER_I32:
                        push_array_element_wvalue(num_values, data, i32, is_null_i32, bison_insert_i32);
                break;
                case BISON_FIELD_TYPE_NUMBER_I64:
                        push_array_element_wvalue(num_values, data, i64, is_null_i64, bison_insert_i64);
                break;
                case BISON_FIELD_TYPE_NUMBER_FLOAT:
                        push_array_element_wvalue(num_values, data, float, is_null_float, bison_insert_float);
                break;
                default:
                        error(&it->err, NG5_ERR_UNSUPPORTEDTYPE);
                        return false;
        }

        bison_array_it_insert_end(&array_ins);
        offset_t array_marker_end = bison_array_it_tell(&array_it);
        bison_array_it_drop(&array_it);

        assert(array_marker_begin < array_marker_end);
        //offset_t array_size = array_marker_end - array_marker_begin;


        printf("\n\n");
        memfile_hexdump_print(&it->memfile); // TODO: debug remove

        memfile_restore_position(&it->memfile);
        return true;
}

NG5_EXPORT(bool) bison_column_it_update_set_true(struct bison_column_it *it, u32 pos)
{
        error_if_null(it)
        error_if(pos >= it->column_num_elements, &it->err, NG5_ERR_OUTOFBOUNDS)

        memfile_save_position(&it->memfile);

        memfile_seek(&it->memfile, it->payload_start + pos * bison_int_get_type_value_size(it->type));

        switch (it->type) {
        case BISON_FIELD_TYPE_NULL:
                /* special case: the column must be re-written to a 'true' column with null-encoded values */
                // TODO: implement
                error_print(NG5_ERR_NOTIMPLEMENTED)
                break;
        case BISON_FIELD_TYPE_TRUE:
                /* nothing to do */
                break;
        case BISON_FIELD_TYPE_FALSE: {
                u8 value = BISON_BOOLEAN_COLUMN_TRUE;
                memfile_write(&it->memfile, &value, sizeof(u8));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U8: {
                u8 null_value = U8_NULL;
                memfile_write(&it->memfile, &null_value, sizeof(u8));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U16: {
                u16 null_value = U16_NULL;
                memfile_write(&it->memfile, &null_value, sizeof(u16));
        } break;
        case BISON_FIELD_TYPE_NUMBER_U32: {
                //u32 null_value = U32_NULL;
                //memfile_write(&it->memfile, &null_value, sizeof(u32));
                rewrite_column_to_array(it);


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