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

#ifndef JAK_CARBON_FIND_H
#define JAK_CARBON_FIND_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_object_it.h>
#include <carbon-containers.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_path.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_find {
        jak_carbon *doc;
        jak_carbon_field_type_e type;
        jak_error err;
        jak_carbon_path_evaluator path_evaluater;

        bool value_is_nulled;

        union {
                jak_carbon_array_it *array_it;
                jak_carbon_column_it *column_it;
                jak_carbon_object_it *object_it;
                bool boolean;
                jak_u64 unsigned_number;
                jak_i64 signed_number;
                float float_number;

                struct {
                        const char *base;
                        jak_u64 len;
                } string;

                jak_carbon_binary binary;
        } value;
} jak_carbon_find;

JAK_DEFINE_ERROR_GETTER(jak_carbon_find)

bool jak_carbon_find_open(jak_carbon_find *out, const char *dot_path, jak_carbon *doc);
bool jak_carbon_find_close(jak_carbon_find *find);
bool jak_carbon_find_create(jak_carbon_find *find, jak_carbon_dot_path *path, jak_carbon *doc);
bool jak_carbon_find_drop(jak_carbon_find *find);

bool jak_carbon_find_has_result(jak_carbon_find *find);
const char *jak_carbon_find_result_to_str(jak_string *dst_str, jak_carbon_printer_impl_e print_type, jak_carbon_find *find);
const char *jak_carbon_find_result_to_json_compact(jak_string *dst_str, jak_carbon_find *find);
char *jak_carbon_find_result_to_json_compact_dup(jak_carbon_find *find);

bool jak_carbon_find_result_type(jak_carbon_field_type_e *type, jak_carbon_find *find);

jak_carbon_array_it *jak_carbon_find_result_array(jak_carbon_find *find);
jak_carbon_object_it *jak_carbon_find_result_object(jak_carbon_find *find);
jak_carbon_column_it *jak_carbon_find_result_column(jak_carbon_find *find);
bool jak_carbon_find_result_boolean(bool *out, jak_carbon_find *find);
bool jak_carbon_find_result_unsigned(jak_u64 *out, jak_carbon_find *find);
bool jak_carbon_find_result_signed(jak_i64 *out, jak_carbon_find *find);
bool jak_carbon_find_result_float(float *out, jak_carbon_find *find);
const char *jak_carbon_find_result_string(jak_u64 *str_len, jak_carbon_find *find);
jak_carbon_binary *jak_carbon_find_result_binary(jak_carbon_find *find);

JAK_END_DECL

#endif
