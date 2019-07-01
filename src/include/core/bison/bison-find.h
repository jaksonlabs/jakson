/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef BISON_FIND_H
#define BISON_FIND_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/bison/bison.h"
#include "core/bison/bison-column-it.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-dot.h"
#include "core/bison/bison-path.h"

NG5_BEGIN_DECL

struct bison_find
{
        struct bison *doc;
        enum bison_field_type type;
        struct err err;
        struct bison_path_evaluator path_evaluater;

        bool value_is_nulled;

        union {
                struct bison_array_it *array_it;
                struct bison_column_it *column_it;
                bool boolean;
                u64 unsigned_number;
                i64 signed_number;
                float float_number;

                struct {
                        const char *base;
                        u64 len;
                } string;

                struct bison_binary binary;
        } value;
};

NG5_DEFINE_ERROR_GETTER(bison_find)

NG5_EXPORT(bool) bison_find_create(struct bison_find *find, struct bison_dot_path *path, struct bison *doc);

NG5_EXPORT(bool) bison_find_has_result(struct bison_find *find);

NG5_EXPORT(bool) bison_find_result_type(enum bison_field_type *type, struct bison_find *find);

NG5_EXPORT(struct bison_array_it *) bison_find_result_array(struct bison_find *find);

NG5_EXPORT(struct bison_column_it *) bison_find_result_column(struct bison_find *find);

NG5_EXPORT(bool) bison_find_result_boolean(bool *out, struct bison_find *find);

NG5_EXPORT(bool) bison_find_result_unsigned(u64 *out, struct bison_find *find);

NG5_EXPORT(bool) bison_find_result_signed(i64 *out, struct bison_find *find);

NG5_EXPORT(bool) bison_find_result_float(float *out, struct bison_find *find);

NG5_EXPORT(const char *) bison_find_result_string(u64 *str_len, struct bison_find *find);

NG5_EXPORT(struct bison_binary *) bison_find_result_binary(struct bison_find *find);

NG5_EXPORT(bool) bison_find_drop(struct bison_find *find);


NG5_END_DECL

#endif
