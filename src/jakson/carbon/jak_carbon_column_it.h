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

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/std/jak_spinlock.h>
#include <jakson/carbon/jak_carbon_field.h>
#include <jakson/carbon.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_column_it {
        jak_memfile memfile;

        jak_offset_t num_and_capacity_start_offset;
        jak_offset_t column_start_offset;

        jak_error err;
        jak_carbon_field_type_e type;

        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        jak_i64 mod_size;

        jak_u32 column_capacity;
        jak_u32 column_num_elements;

        jak_spinlock lock;
} jak_carbon_column_it;

bool jak_carbon_column_it_create(jak_carbon_column_it *it, jak_memfile *memfile, jak_error *err, jak_offset_t column_start_offset);
bool jak_carbon_column_it_clone(jak_carbon_column_it *dst, jak_carbon_column_it *src);

bool jak_carbon_column_it_insert(jak_carbon_insert *inserter, jak_carbon_column_it *it);
bool jak_carbon_column_it_fast_forward(jak_carbon_column_it *it);
jak_offset_t jak_carbon_column_it_memfilepos(jak_carbon_column_it *it);
jak_offset_t jak_carbon_column_it_tell(jak_carbon_column_it *it, jak_u32 elem_idx);

const void *jak_carbon_column_it_values(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it);
bool jak_carbon_column_it_values_info(jak_carbon_field_type_e *type, jak_u32 *nvalues, jak_carbon_column_it *it);

bool jak_carbon_column_it_value_is_null(jak_carbon_column_it *it, jak_u32 pos);

const jak_u8 *jak_carbon_column_it_boolean_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u8 *jak_carbon_column_it_u8_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u16 *jak_carbon_column_it_u16_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u32 *jak_carbon_column_it_u32_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_u64 *jak_carbon_column_it_u64_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i8 *jak_carbon_column_it_i8_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i16 *jak_carbon_column_it_i16_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i32 *jak_carbon_column_it_i32_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const jak_i64 *jak_carbon_column_it_i64_values(jak_u32 *nvalues, jak_carbon_column_it *it);
const float *jak_carbon_column_it_float_values(jak_u32 *nvalues, jak_carbon_column_it *it);

bool jak_carbon_column_it_remove(jak_carbon_column_it *it, jak_u32 pos);

bool jak_carbon_column_it_update_set_null(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_true(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_false(jak_carbon_column_it *it, jak_u32 pos);
bool jak_carbon_column_it_update_set_u8(jak_carbon_column_it *it, jak_u32 pos, jak_u8 value);
bool jak_carbon_column_it_update_set_u16(jak_carbon_column_it *it, jak_u32 pos, jak_u16 value);
bool jak_carbon_column_it_update_set_u32(jak_carbon_column_it *it, jak_u32 pos, jak_u32 value);
bool jak_carbon_column_it_update_set_u64(jak_carbon_column_it *it, jak_u32 pos, jak_u64 value);
bool jak_carbon_column_it_update_set_i8(jak_carbon_column_it *it, jak_u32 pos, jak_i8 value);
bool jak_carbon_column_it_update_set_i16(jak_carbon_column_it *it, jak_u32 pos, jak_i16 value);
bool jak_carbon_column_it_update_set_i32(jak_carbon_column_it *it, jak_u32 pos, jak_i32 value);
bool jak_carbon_column_it_update_set_i64(jak_carbon_column_it *it, jak_u32 pos, jak_i64 value);
bool jak_carbon_column_it_update_set_float(jak_carbon_column_it *it, jak_u32 pos, float value);

/**
 * Locks the iterator with a spinlock. A call to <code>jak_carbon_column_it_unlock</code> is required for unlocking.
 */
bool jak_carbon_column_it_lock(jak_carbon_column_it *it);

/**
 * Unlocks the iterator
 */
bool jak_carbon_column_it_unlock(jak_carbon_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool jak_carbon_column_it_rewind(jak_carbon_column_it *it);

JAK_END_DECL

#endif
