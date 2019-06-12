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

#ifndef BISON_ARRAY_IT_H
#define BISON_ARRAY_IT_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/async/spin.h"

NG5_BEGIN_DECL

struct bison; /* forwarded from bison.h */
struct bison_insert; /* forwarded from bison-literal-inserter.h */

struct bison_array_it
{
        struct memfile memfile;
        offset_t payload_start;
        struct spinlock lock;
        struct err err;

        enum bison_field_type it_field_type;

        const void *it_field_data;
        u64 it_field_len;

        const char *it_mime_type;
        u64 it_mime_type_strlen;
};

NG5_DEFINE_ERROR_GETTER(bison_array_it);

/**
 * Constructs a new array iterator in a BISON document, where <code>payload_start</code> is a memory offset
 * that starts with the first (potentially empty) array entry. If there is some data before the array contents
 * (e.g., a header), <code>payload_start</code> must not include this data.
 */
NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct bison *doc, offset_t payload_start,
        enum access_mode mode);

/**
 * Drops the iterator.
 */
NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it, struct bison *doc);

/**
 * Locks the iterator with a spinlock. A call to <code>bison_array_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_array_it_lock(struct bison_array_it *it);

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_array_it_unlock(struct bison_array_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
NG5_EXPORT(bool) bison_array_it_rewind(struct bison_array_it *it);

/**
 * Positions the iterator to the last element of this array.
 */
NG5_EXPORT(bool) bison_array_it_fast_forward(struct bison_array_it *it);

/**
 * Positions the iterator to the slot after the current element, potentially pointing to next element.
 * The function returns true, if the slot is non-empty, and false otherwise.
 */
NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_field_type(enum bison_field_type *type, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_u8_value(u8 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_u16_value(u16 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_u32_value(u32 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_u64_value(u64 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_i8_value(i8 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_i16_value(i16 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_i32_value(i32 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_i64_value(i64 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_float_value(float *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_signed_value(i64 *value, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_unsigned_value(u64 *value, struct bison_array_it *it);

NG5_EXPORT(const char *) bison_array_it_string_value(u64 *strlen, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_binary_value(struct bison_binary *out, struct bison_array_it *it);

/**
 * Checks whether the slot before the current element is in use.
 */
NG5_EXPORT(bool) bison_array_it_has_prev(struct bison_array_it *it);

/**
 * Positions the iterator to the element before the current one.
 */
NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it);

/**
 * Inserts a new element at the current position of the iterator. In any case, <code>bison_insert_drop</code>
 * must be called to complete the process (even if no-op).
 */
NG5_EXPORT(bool) bison_array_it_insert(struct bison_insert *inserter, struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_remove(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_update(struct bison_array_it *it);

NG5_END_DECL

#endif
