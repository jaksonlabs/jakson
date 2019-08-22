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

struct jak_carbon_column_it {
        struct jak_memfile memfile;

        jak_offset_t num_and_capacity_start_offset;
        jak_offset_t column_start_offset;

        struct jak_error err;
        enum carbon_field_type type;

        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        jak_i64 mod_size;

        jak_u32 column_capacity;
        jak_u32 column_num_elements;

        struct spinlock lock;
};

bool carbon_column_it_create(struct jak_carbon_column_it *it, struct jak_memfile *memfile, struct jak_error *err,
                             jak_offset_t column_start_offset);

bool carbon_column_it_clone(struct jak_carbon_column_it *dst, struct jak_carbon_column_it *src);

bool carbon_column_it_insert(jak_carbon_insert *inserter, struct jak_carbon_column_it *it);

bool carbon_column_it_fast_forward(struct jak_carbon_column_it *it);

jak_offset_t carbon_column_it_memfilepos(struct jak_carbon_column_it *it);

jak_offset_t carbon_column_it_tell(struct jak_carbon_column_it *it, jak_u32 elem_idx);

const void *carbon_column_it_values(enum carbon_field_type *type, jak_u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_values_info(enum carbon_field_type *type, jak_u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_value_is_null(struct jak_carbon_column_it *it, jak_u32 pos);

const jak_u8 *carbon_column_it_boolean_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_u8 *carbon_column_it_u8_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_u16 *carbon_column_it_u16_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_u32 *carbon_column_it_u32_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_u64 *carbon_column_it_u64_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_i8 *carbon_column_it_i8_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_i16 *carbon_column_it_i16_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_i32 *carbon_column_it_i32_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const jak_i64 *carbon_column_it_i64_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

const float *carbon_column_it_float_values(jak_u32 *nvalues, struct jak_carbon_column_it *it);

bool carbon_column_it_remove(struct jak_carbon_column_it *it, jak_u32 pos);

bool carbon_column_it_update_set_null(struct jak_carbon_column_it *it, jak_u32 pos);

bool carbon_column_it_update_set_true(struct jak_carbon_column_it *it, jak_u32 pos);

bool carbon_column_it_update_set_false(struct jak_carbon_column_it *it, jak_u32 pos);

bool carbon_column_it_update_set_u8(struct jak_carbon_column_it *it, jak_u32 pos, jak_u8 value);

bool carbon_column_it_update_set_u16(struct jak_carbon_column_it *it, jak_u32 pos, jak_u16 value);

bool carbon_column_it_update_set_u32(struct jak_carbon_column_it *it, jak_u32 pos, jak_u32 value);

bool carbon_column_it_update_set_u64(struct jak_carbon_column_it *it, jak_u32 pos, jak_u64 value);

bool carbon_column_it_update_set_i8(struct jak_carbon_column_it *it, jak_u32 pos, jak_i8 value);

bool carbon_column_it_update_set_i16(struct jak_carbon_column_it *it, jak_u32 pos, jak_i16 value);

bool carbon_column_it_update_set_i32(struct jak_carbon_column_it *it, jak_u32 pos, jak_i32 value);

bool carbon_column_it_update_set_i64(struct jak_carbon_column_it *it, jak_u32 pos, jak_i64 value);

bool carbon_column_it_update_set_float(struct jak_carbon_column_it *it, jak_u32 pos, float value);



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
