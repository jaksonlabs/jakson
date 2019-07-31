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

#ifndef CARBON_COLUMN_IT_H
#define CARBON_COLUMN_IT_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/carbon/carbon-field.h>
#include <ark-js/carbon/carbon.h>

ARK_BEGIN_DECL

struct carbon; /* forwarded from carbon.h */
struct carbon_insert; /* forwarded from carbon-literal-inserter.h */

struct carbon_column_it
{
        struct memfile memfile;


        //offset_t column_num_elements_offset;
        //offset_t column_capacity_offset;

        offset_t num_and_capacity_start_offset;

        offset_t column_start_offset;
        //offset_t payload_start;

        struct err err;
        enum carbon_field_type type;

        u32 column_capacity;
        u32 column_num_elements;

        struct spinlock lock;
};

ARK_EXPORT(bool) carbon_column_it_create(struct carbon_column_it *it, struct memfile *memfile, struct err *err,
        offset_t column_start_offset);

ARK_EXPORT(bool) carbon_column_it_insert(struct carbon_insert *inserter, struct carbon_column_it *it);

ARK_EXPORT(bool) carbon_column_it_fast_forward(struct carbon_column_it *it);

ARK_EXPORT(offset_t) carbon_column_it_tell(struct carbon_column_it *it);

ARK_EXPORT(const void *) carbon_column_it_values(enum carbon_field_type *type, u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(bool) carbon_column_it_values_info(enum carbon_field_type *type, u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const u8 *) carbon_column_it_boolean_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const u8 *) carbon_column_it_u8_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const u16 *) carbon_column_it_u16_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const u32 *) carbon_column_it_u32_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const u64 *) carbon_column_it_u64_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const i8 *) carbon_column_it_i8_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const i16 *) carbon_column_it_i16_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const i32 *) carbon_column_it_i32_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const i64 *) carbon_column_it_i64_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(const float *) carbon_column_it_float_values(u32 *nvalues, struct carbon_column_it *it);

ARK_EXPORT(bool) carbon_column_it_remove(struct carbon_column_it *it, u32 pos);

ARK_EXPORT(bool) carbon_column_it_update_set_null(struct carbon_column_it *it, u32 pos);

ARK_EXPORT(bool) carbon_column_it_update_set_true(struct carbon_column_it *it, u32 pos);

ARK_EXPORT(bool) carbon_column_it_update_set_false(struct carbon_column_it *it, u32 pos);

ARK_EXPORT(bool) carbon_column_it_update_set_u8(struct carbon_column_it *it, u32 pos, u8 value);

ARK_EXPORT(bool) carbon_column_it_update_set_u16(struct carbon_column_it *it, u32 pos, u16 value);

ARK_EXPORT(bool) carbon_column_it_update_set_u32(struct carbon_column_it *it, u32 pos, u32 value);

ARK_EXPORT(bool) carbon_column_it_update_set_u64(struct carbon_column_it *it, u32 pos, u64 value);

ARK_EXPORT(bool) carbon_column_it_update_set_i8(struct carbon_column_it *it, u32 pos, i8 value);

ARK_EXPORT(bool) carbon_column_it_update_set_i16(struct carbon_column_it *it, u32 pos, i16 value);

ARK_EXPORT(bool) carbon_column_it_update_set_i32(struct carbon_column_it *it, u32 pos, i32 value);

ARK_EXPORT(bool) carbon_column_it_update_set_i64(struct carbon_column_it *it, u32 pos, i64 value);

ARK_EXPORT(bool) carbon_column_it_update_set_float(struct carbon_column_it *it, u32 pos, float value);



/**
 * Locks the iterator with a spinlock. A call to <code>carbon_column_it_unlock</code> is required for unlocking.
 */
ARK_EXPORT(bool) carbon_column_it_lock(struct carbon_column_it *it);

/**
 * Unlocks the iterator
 */
ARK_EXPORT(bool) carbon_column_it_unlock(struct carbon_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
ARK_EXPORT(bool) carbon_column_it_rewind(struct carbon_column_it *it);

ARK_END_DECL

#endif
