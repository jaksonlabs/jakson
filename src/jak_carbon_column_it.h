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

#ifndef JAK_CARBON_COLUMN_IT_H
#define JAK_CARBON_COLUMN_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_spinlock.h>
#include <jak_carbon_field.h>
#include <jak_carbon.h>

JAK_BEGIN_DECL

struct jak_carbon; /* forwarded from carbon.h */
struct jak_carbon_insert; /* forwarded from carbon-literal-inserter.h */

struct jak_carbon_column_it {
    struct memfile memfile;

    offset_t num_and_capacity_start_offset;
    offset_t column_start_offset;

    struct err err;
    enum carbon_field_type type;

    /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
    i64 mod_size;

    u32 column_capacity;
    u32 column_num_elements;

    struct spinlock lock;
};

bool carbon_column_it_create(struct jak_carbon_column_it *it, struct memfile *memfile, struct err *err,
                             offset_t column_start_offset);

bool carbon_column_it_clone(struct jak_carbon_column_it *dst, struct jak_carbon_column_it *src);

bool carbon_column_it_insert(struct jak_carbon_insert *inserter, struct jak_carbon_column_it *it);

bool carbon_column_it_fast_forward(struct jak_carbon_column_it *it);

offset_t carbon_column_it_memfilepos(struct jak_carbon_column_it *it);

offset_t carbon_column_it_tell(struct jak_carbon_column_it *it, u32 elem_idx);

const void *carbon_column_it_values(enum carbon_field_type *type, u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_values_info(enum carbon_field_type *type, u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_value_is_null(struct jak_carbon_column_it *it, u32 pos);

const u8 *carbon_column_it_boolean_values(u32 *nvalues, struct jak_carbon_column_it *it);

const u8 *carbon_column_it_u8_values(u32 *nvalues, struct jak_carbon_column_it *it);

const u16 *carbon_column_it_u16_values(u32 *nvalues, struct jak_carbon_column_it *it);

const u32 *carbon_column_it_u32_values(u32 *nvalues, struct jak_carbon_column_it *it);

const u64 *carbon_column_it_u64_values(u32 *nvalues, struct jak_carbon_column_it *it);

const i8 *carbon_column_it_i8_values(u32 *nvalues, struct jak_carbon_column_it *it);

const i16 *carbon_column_it_i16_values(u32 *nvalues, struct jak_carbon_column_it *it);

const i32 *carbon_column_it_i32_values(u32 *nvalues, struct jak_carbon_column_it *it);

const i64 *carbon_column_it_i64_values(u32 *nvalues, struct jak_carbon_column_it *it);

const float *carbon_column_it_float_values(u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_remove(struct jak_carbon_column_it *it, u32 pos);

bool carbon_column_it_update_set_null(struct jak_carbon_column_it *it, u32 pos);

bool carbon_column_it_update_set_true(struct jak_carbon_column_it *it, u32 pos);

bool carbon_column_it_update_set_false(struct jak_carbon_column_it *it, u32 pos);

bool carbon_column_it_update_set_u8(struct jak_carbon_column_it *it, u32 pos, u8 value);

bool carbon_column_it_update_set_u16(struct jak_carbon_column_it *it, u32 pos, u16 value);

bool carbon_column_it_update_set_u32(struct jak_carbon_column_it *it, u32 pos, u32 value);

bool carbon_column_it_update_set_u64(struct jak_carbon_column_it *it, u32 pos, u64 value);

bool carbon_column_it_update_set_i8(struct jak_carbon_column_it *it, u32 pos, i8 value);

bool carbon_column_it_update_set_i16(struct jak_carbon_column_it *it, u32 pos, i16 value);

bool carbon_column_it_update_set_i32(struct jak_carbon_column_it *it, u32 pos, i32 value);

bool carbon_column_it_update_set_i64(struct jak_carbon_column_it *it, u32 pos, i64 value);

bool carbon_column_it_update_set_float(struct jak_carbon_column_it *it, u32 pos, float value);



/**
 * Locks the iterator with a spinlock. A call to <code>carbon_column_it_unlock</code> is required for unlocking.
 */
bool carbon_column_it_lock(struct jak_carbon_column_it *it);

/**
 * Unlocks the iterator
 */
bool carbon_column_it_unlock(struct jak_carbon_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool carbon_column_it_rewind(struct jak_carbon_column_it *it);

JAK_END_DECL

#endif
