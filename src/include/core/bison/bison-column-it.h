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

#ifndef BISON_COLUMN_IT_H
#define BISON_COLUMN_IT_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/async/spin.h"
#include "bison.h"

NG5_BEGIN_DECL

struct bison; /* forwarded from bison.h */
struct bison_insert; /* forwarded from bison-literal-inserter.h */

struct bison_column_it
{
        struct memfile memfile;

        offset_t column_start_offset;
        offset_t column_num_elements_offset;
        offset_t column_capacity_offset;
        offset_t payload_start;

        struct err err;
        enum bison_field_type type;
        u32 column_capacity;
        u32 column_num_elements;

        struct spinlock lock;
};

NG5_EXPORT(bool) bison_column_it_create(struct bison_column_it *it, struct memfile *memfile, struct err *err,
        offset_t column_start_offset);

NG5_EXPORT(bool) bison_column_it_insert(struct bison_insert *inserter, struct bison_column_it *it);

NG5_EXPORT(bool) bison_column_it_fast_forward(struct bison_column_it *it);

NG5_EXPORT(const void *) bison_column_it_values(enum bison_field_type *type, u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(bool) bison_column_it_values_info(enum bison_field_type *type, u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const u8 *) bison_column_it_boolean_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const u8 *) bison_column_it_u8_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const u16 *) bison_column_it_u16_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const u32 *) bison_column_it_u32_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const u64 *) bison_column_it_u64_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const i8 *) bison_column_it_i8_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const i16 *) bison_column_it_i16_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const i32 *) bison_column_it_i32_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const i64 *) bison_column_it_i64_values(u32 *nvalues, struct bison_column_it *it);

NG5_EXPORT(const float *) bison_column_it_float_values(u32 *nvalues, struct bison_column_it *it);


/**
 * Locks the iterator with a spinlock. A call to <code>bison_column_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_column_it_lock(struct bison_column_it *it);

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_column_it_unlock(struct bison_column_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
NG5_EXPORT(bool) bison_column_it_rewind(struct bison_column_it *it);

NG5_END_DECL

#endif
