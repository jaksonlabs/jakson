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

#ifndef JAK_CARBON_GET_H
#define JAK_CARBON_GET_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_carbon.h>

struct jak_carbon_array_it; /* forwarded from carbon-array-it.h */
struct jak_carbon_column_it; /* forwarded from carbon-column-it.h */

JAK_BEGIN_DECL

jak_u64 carbon_get_or_default_unsigned(struct jak_carbon *doc, const char *path, jak_u64 default_val);

jak_i64 carbon_get_or_default_signed(struct jak_carbon *doc, const char *path, jak_i64 default_val);

float carbon_get_or_default_float(struct jak_carbon *doc, const char *path, float default_val);

bool carbon_get_or_default_boolean(struct jak_carbon *doc, const char *path, bool default_val);

const char *carbon_get_or_default_string(jak_u64 *len_out, struct jak_carbon *doc, const char *path, const char *default_val);

struct jak_carbon_binary *
carbon_get_or_default_binary(struct jak_carbon *doc, const char *path, struct jak_carbon_binary *default_val);

struct jak_carbon_array_it *carbon_get_array_or_null(struct jak_carbon *doc, const char *path);

struct jak_carbon_column_it *carbon_get_column_or_null(struct jak_carbon *doc, const char *path);

JAK_END_DECL

#endif
