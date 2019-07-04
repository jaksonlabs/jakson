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

#ifndef BISON_GET_H
#define BISON_GET_H

#include "shared/common.h"
#include "shared/types.h"
#include "core/bison/bison.h"

struct bison_array_it; /* forwarded from bison-array-it.h */
struct bison_column_it; /* forwarded from bison-column-it.h */

NG5_BEGIN_DECL

NG5_EXPORT(u64) bison_get_or_default_unsigned(struct bison *doc, const char *path, u64 default_val);

NG5_EXPORT(i64) bison_get_or_default_signed(struct bison *doc, const char *path, i64 default_val);

NG5_EXPORT(float) bison_get_or_default_float(struct bison *doc, const char *path, float default_val);

NG5_EXPORT(bool) bison_get_or_default_boolean(struct bison *doc, const char *path, bool default_val);

NG5_EXPORT(const char *) bison_get_or_default_string(u64 *len_out, struct bison *doc, const char *path, const char *default_val);

NG5_EXPORT(struct bison_binary *) bison_get_or_default_binary(struct bison *doc, const char *path, struct bison_binary *default_val);

NG5_EXPORT(struct bison_array_it *) bison_get_array_or_null(struct bison *doc, const char *path);

NG5_EXPORT(struct bison_column_it *) bison_get_column_or_null(struct bison *doc, const char *path);

NG5_END_DECL

#endif
