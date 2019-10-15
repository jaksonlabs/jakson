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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/std/spinlock.h>
#include <jakson/carbon/field.h>
#include <jakson/carbon.h>

BEGIN_DECL

typedef struct carbon_column_it {
        memfile memfile;

        offset_t num_and_capacity_start_offset;
        offset_t column_start_offset;

        err err;
        carbon_field_type_e type;

        carbon_list_derivable_e abstract_type;

        /** in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        i64 mod_size;

        u32 column_capacity;
        u32 column_num_elements;

        spinlock lock;
} carbon_column_it;

bool carbon_column_it_create(carbon_column_it *it, memfile *memfile, err *err, offset_t column_start_offset);
bool carbon_column_it_clone(carbon_column_it *dst, carbon_column_it *src);

bool carbon_column_it_insert(carbon_insert *inserter, carbon_column_it *it);
bool carbon_column_it_fast_forward(carbon_column_it *it);
offset_t carbon_column_it_memfilepos(carbon_column_it *it);
offset_t carbon_column_it_tell(carbon_column_it *it, u32 elem_idx);

const void *carbon_column_it_values(carbon_field_type_e *type, u32 *nvalues, carbon_column_it *it);
bool carbon_column_it_values_info(carbon_field_type_e *type, u32 *nvalues, carbon_column_it *it);

bool carbon_column_it_value_is_null(carbon_column_it *it, u32 pos);

const u8 *carbon_column_it_boolean_values(u32 *nvalues, carbon_column_it *it);
const u8 *carbon_column_it_u8_values(u32 *nvalues, carbon_column_it *it);
const u16 *carbon_column_it_u16_values(u32 *nvalues, carbon_column_it *it);
const u32 *carbon_column_it_u32_values(u32 *nvalues, carbon_column_it *it);
const u64 *carbon_column_it_u64_values(u32 *nvalues, carbon_column_it *it);
const i8 *carbon_column_it_i8_values(u32 *nvalues, carbon_column_it *it);
const i16 *carbon_column_it_i16_values(u32 *nvalues, carbon_column_it *it);
const i32 *carbon_column_it_i32_values(u32 *nvalues, carbon_column_it *it);
const i64 *carbon_column_it_i64_values(u32 *nvalues, carbon_column_it *it);
const float *carbon_column_it_float_values(u32 *nvalues, carbon_column_it *it);

bool carbon_column_it_remove(carbon_column_it *it, u32 pos);

fn_result ofType(bool) carbon_column_it_is_multiset(carbon_column_it *it);
fn_result ofType(bool) carbon_column_it_is_sorted(carbon_column_it *it);
fn_result carbon_column_it_update_type(carbon_column_it *it, carbon_list_derivable_e derivation);

bool carbon_column_it_update_set_null(carbon_column_it *it, u32 pos);
bool carbon_column_it_update_set_true(carbon_column_it *it, u32 pos);
bool carbon_column_it_update_set_false(carbon_column_it *it, u32 pos);
bool carbon_column_it_update_set_u8(carbon_column_it *it, u32 pos, u8 value);
bool carbon_column_it_update_set_u16(carbon_column_it *it, u32 pos, u16 value);
bool carbon_column_it_update_set_u32(carbon_column_it *it, u32 pos, u32 value);
bool carbon_column_it_update_set_u64(carbon_column_it *it, u32 pos, u64 value);
bool carbon_column_it_update_set_i8(carbon_column_it *it, u32 pos, i8 value);
bool carbon_column_it_update_set_i16(carbon_column_it *it, u32 pos, i16 value);
bool carbon_column_it_update_set_i32(carbon_column_it *it, u32 pos, i32 value);
bool carbon_column_it_update_set_i64(carbon_column_it *it, u32 pos, i64 value);
bool carbon_column_it_update_set_float(carbon_column_it *it, u32 pos, float value);

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_column_it_unlock</code> is required for unlocking.
 */
bool carbon_column_it_lock(carbon_column_it *it);

/**
 * Unlocks the iterator
 */
bool carbon_column_it_unlock(carbon_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool carbon_column_it_rewind(carbon_column_it *it);

END_DECL

#endif
