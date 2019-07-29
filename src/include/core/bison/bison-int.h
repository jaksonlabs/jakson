/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file is for internal usage only; do not call these functions from outside
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

#ifndef BISON_INT_H
#define BISON_INT_H

#include "shared/common.h"
#include "core/mem/file.h"
#include "stdx/varuint.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-field.h"
#include "bison-array-it.h"

NG5_BEGIN_DECL

struct bison_insert
{
        enum bison_container_type context_type;
        union {
                struct bison_array_it *array;
                struct bison_column_it *column;
                struct bison_object_it *object;
        } context;

        struct memfile memfile;
        offset_t position;
        struct err err;
};

struct bison_insert_array_state
{
        struct bison_insert *parent_inserter;

        struct bison_array_it *nested_array;
        struct bison_insert nested_inserter;
};

struct bison_insert_object_state
{
        struct bison_insert *parent_inserter;

        struct bison_object_it *it;
        struct bison_insert inserter;
};

struct bison_insert_column_state
{
        struct bison_insert *parent_inserter;

        enum bison_field_type type;
        struct bison_column_it *nested_column;
        struct bison_insert nested_inserter;

};

NG5_EXPORT(bool) bison_int_insert_object(struct memfile *memfile, size_t nbytes);

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes);

NG5_EXPORT(bool) bison_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum bison_field_type type, size_t capactity);

/**
 * Returns the number of bytes required to store a field type including its type marker in a byte sequence.
 */
NG5_EXPORT(size_t) bison_int_get_type_size_encoded(enum bison_field_type type);

/**
 * Returns the number of bytes required to store a field value of a particular type exclusing its type marker.
 */
NG5_EXPORT(size_t) bison_int_get_type_value_size(enum bison_field_type type);

NG5_EXPORT(bool) bison_int_array_it_next(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_object_it_next(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_it_prop_key_access(struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_it_prop_key_skip(struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_it_prop_value_skip(struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_it_prop_skip(struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, struct bison_object_it *it);

NG5_EXPORT(bool) bison_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_field_type_read(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_field_data_access(struct bison_array_it *it);

NG5_EXPORT(offset_t) bison_int_column_get_payload_off(struct bison_column_it *it);

NG5_EXPORT(offset_t) bison_int_payload_after_header(struct bison *doc);

NG5_EXPORT(u64) bison_int_header_get_rev(struct bison *doc);

NG5_EXPORT(void) bison_int_history_push(struct vector ofType(offset_t) *vec, offset_t off);

NG5_EXPORT(void) bison_int_history_clear(struct vector ofType(offset_t) *vec);

NG5_EXPORT(offset_t) bison_int_history_pop(struct vector ofType(offset_t) *vec);

NG5_EXPORT(offset_t) bison_int_history_peek(struct vector ofType(offset_t) *vec);

NG5_EXPORT(bool) bison_int_history_has(struct vector ofType(offset_t) *vec);

NG5_EXPORT(bool) bison_int_field_access_create(struct field_access *field);

NG5_EXPORT(bool) bison_int_field_access_drop(struct field_access *field);

NG5_EXPORT(bool) bison_int_field_auto_close(struct field_access *it);

NG5_EXPORT(void) bison_int_auto_close_nested_array_it(struct field_access *field);

NG5_EXPORT(void) bison_int_auto_close_nested_object_it(struct field_access *field);

NG5_EXPORT(void) bison_int_auto_close_nested_column_it(struct field_access *field);

NG5_EXPORT(bool) bison_int_field_access_field_type(enum bison_field_type *type, struct field_access *field);

NG5_EXPORT(bool) bison_int_field_access_u8_value(u8 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_u16_value(u16 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_u32_value(u32 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_u64_value(u64 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_i8_value(i8 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_i16_value(i16 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_i32_value(i32 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_i64_value(i64 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_float_value(bool *is_null_in, float *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_signed_value(bool *is_null_in, i64 *value, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_unsigned_value(bool *is_null_in, u64 *value, struct field_access *field, struct err *err);

NG5_EXPORT(const char *) bison_int_field_access_string_value(u64 *strlen, struct field_access *field, struct err *err);

NG5_EXPORT(bool) bison_int_field_access_binary_value(struct bison_binary *out, struct field_access *field, struct err *err);

NG5_EXPORT(struct bison_array_it *) bison_int_field_access_array_value(struct field_access *field, struct err *err);

NG5_EXPORT(struct bison_object_it *) bison_int_field_access_object_value(struct field_access *field, struct err *err);

NG5_EXPORT(struct bison_column_it *) bison_int_field_access_column_value(struct field_access *field, struct err *err);


NG5_END_DECL

#endif
