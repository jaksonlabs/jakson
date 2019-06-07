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

NG5_BEGIN_DECL

struct bison; /* forwarded from bison.h */

struct bison_array_it {
        struct bison *doc;
        offset_t payload_start;
        struct err err;
};

NG5_DEFINE_ERROR_GETTER(bison_array_it);

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct bison *doc, offset_t payload_start);

NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it, struct bison *doc);

NG5_EXPORT(bool) bison_array_it_rewind(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_has_next(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_swap(struct bison_array_it *lhs, struct bison_array_it *rhs);

NG5_EXPORT(bool) bison_array_it_push_back(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_remove(struct bison_array_it *it);

NG5_EXPORT(bool) bison_array_it_update(struct bison_array_it *it);

NG5_END_DECL

#endif
