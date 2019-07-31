/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef carbon_GET_H
#define CARBON_GET_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/types.h>
#include <ark-js/carbon/carbon.h>

struct carbon_array_it; /* forwarded from carbon-array-it.h */
struct carbon_column_it; /* forwarded from carbon-column-it.h */

ARK_BEGIN_DECL

ARK_EXPORT(u64) carbon_get_or_default_unsigned(struct carbon *doc, const char *path, u64 default_val);

ARK_EXPORT(i64) carbon_get_or_default_signed(struct carbon *doc, const char *path, i64 default_val);

ARK_EXPORT(float) carbon_get_or_default_float(struct carbon *doc, const char *path, float default_val);

ARK_EXPORT(bool) carbon_get_or_default_boolean(struct carbon *doc, const char *path, bool default_val);

ARK_EXPORT(const char *) carbon_get_or_default_string(u64 *len_out, struct carbon *doc, const char *path, const char *default_val);

ARK_EXPORT(struct carbon_binary *) carbon_get_or_default_binary(struct carbon *doc, const char *path, struct carbon_binary *default_val);

ARK_EXPORT(struct carbon_array_it *) carbon_get_array_or_null(struct carbon *doc, const char *path);

ARK_EXPORT(struct carbon_column_it *) carbon_get_column_or_null(struct carbon *doc, const char *path);

ARK_END_DECL

#endif
