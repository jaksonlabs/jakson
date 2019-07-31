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

#ifndef BISON_OBJECT_IT_H
#define BISON_OBJECT_IT_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/carbon/bison-field.h>
#include <ark-js/carbon/bison-array-it.h>

NG5_BEGIN_DECL

struct bison_object_it
{
        struct memfile memfile;
        offset_t payload_start;
        struct spinlock lock;
        struct err err;

        struct vector ofType(offset_t) history;

        u64 key_len;
        const char *key;

        offset_t value_off;

        struct field_access field_access;
};

NG5_EXPORT(bool) bison_object_it_create(struct bison_object_it *it, struct memfile *memfile, struct err *err,
        offset_t payload_start);

NG5_EXPORT(bool) bison_object_it_copy(struct bison_object_it *dst, struct bison_object_it *src);

NG5_EXPORT(bool) bison_object_it_drop(struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_rewind(struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_next(struct bison_object_it *it);

NG5_EXPORT(const char *) bison_object_it_prop_name(u64 *key_len, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_prop_type(enum bison_field_type *type, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_insert_begin(struct bison_insert *inserter, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_insert_end(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_object_it_lock(struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_unlock(struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_fast_forward(struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_u8_value(u8 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_u16_value(u16 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_u32_value(u32 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_u64_value(u64 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_i8_value(i8 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_i16_value(i16 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_i32_value(i32 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_i64_value(i64 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_float_value(bool *is_null_in, float *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_signed_value(bool *is_null_in, i64 *value, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_unsigned_value(bool *is_null_in, u64 *value, struct bison_object_it *it);

NG5_EXPORT(const char *) bison_object_it_string_value(u64 *strlen, struct bison_object_it *it);

NG5_EXPORT(bool) bison_object_it_binary_value(struct bison_binary *out, struct bison_object_it *it);

NG5_EXPORT(struct bison_array_it *) bison_object_it_array_value(struct bison_object_it *it_in);

NG5_EXPORT(struct bison_object_it *) bison_object_it_object_value(struct bison_object_it *it_in);

NG5_EXPORT(struct bison_column_it *) bison_object_it_column_value(struct bison_object_it *it_in);



NG5_END_DECL

#endif
