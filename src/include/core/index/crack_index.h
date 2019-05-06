/**
 * Copyright 2019 Marcus Pinnecke
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

#ifndef NG5_CRACK_INDEX_H
#define NG5_CRACK_INDEX_H

#include "shared/common.h"
#include "shared/error.h"
#include "std/vec.h"

struct crack_value
{
        u32 actual_key;
        const void *value;
};

struct crack_item
{
        u32 self_idx, prev_idx, next_idx;
        u32 less_than_key;

        struct vector ofType(struct crack_value) values;
        struct vector ofType(u32) values_uselist;
        struct vector ofType(u32) values_freelist;
};

/* In-Place Cracking */
struct crack_index
{
        struct vector ofType(struct crack_item) crack_items;
        struct vector ofType(struct crack_item *) crack_items_freelist;
        struct crack_item *lowest;
        struct err err;
};

NG5_DEFINE_GET_ERROR_FUNCTION(crack_index, struct crack_index, index);

NG5_EXPORT(bool) crack_index_create(struct crack_index *index, struct err *err, u64 capacity);

NG5_EXPORT(bool) crack_index_drop(struct crack_index *index);

NG5_EXPORT(bool) crack_index_push(struct crack_index *index, u32 key, const void *value);

NG5_EXPORT(const void *) crack_index_pop(struct crack_index *index, u32 key);

#endif
