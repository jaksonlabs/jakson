/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#ifndef BISON_LITERAL_INSERTER_H
#define BISON_LITERAL_INSERTER_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/async/spin.h"

NG5_BEGIN_DECL

struct bison_array_it; /* forwarded from bison-array-it.h */

struct bison_insert
{
        struct bison_array_it *context;
        struct memfile memfile;
        offset_t position;
        struct err err;
};

NG5_EXPORT(bool) bison_insert_create(struct bison_insert *inserter, struct bison_array_it *context);

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_true(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_false(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_unsigned(struct bison_insert *inserter, u64 value);

NG5_EXPORT(bool) bison_insert_signed(struct bison_insert *inserter, i64 value);

NG5_EXPORT(bool) bison_insert_float(struct bison_insert *inserter, float value);

NG5_EXPORT(bool) bison_insert_string(struct bison_insert *inserter, const char *value);

NG5_EXPORT(bool) bison_insert_binary(struct bison_insert *inserter, const void *value, size_t nbytes);

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter);

NG5_END_DECL

#endif
