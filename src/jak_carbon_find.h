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
#include <jak_carbon_dot.h>
#include <jak_carbon_path.h>

JAK_BEGIN_DECL

struct jak_carbon_find {
    struct jak_carbon *doc;
    enum carbon_field_type type;
    struct err err;
    struct jak_carbon_path_evaluator path_evaluater;

    bool value_is_nulled;

    union {
        struct jak_carbon_array_it *array_it;
        struct jak_carbon_column_it *column_it;
        struct jak_carbon_object_it *object_it;
        bool boolean;
        u64 unsigned_number;
        i64 signed_number;
        float float_number;

        struct {
            const char *base;
            u64 len;
        } string;

        struct jak_carbon_binary binary;
    } value;
};

JAK_DEFINE_ERROR_GETTER(jak_carbon_find)

bool carbon_find_open(struct jak_carbon_find *out, const char *dot_path, struct jak_carbon *doc);

bool carbon_find_close(struct jak_carbon_find *find);

bool carbon_find_create(struct jak_carbon_find *find, struct jak_carbon_dot_path *path, struct jak_carbon *doc);

bool carbon_find_has_result(struct jak_carbon_find *find);

const char *carbon_find_result_to_str(struct jak_string *dst_str, enum carbon_printer_impl print_type, struct jak_carbon_find *find);

const char *carbon_find_result_to_json_compact(struct jak_string *dst_str, struct jak_carbon_find *find);

char *carbon_find_result_to_json_compact_dup(struct jak_carbon_find *find);

bool carbon_find_result_type(enum carbon_field_type *type, struct jak_carbon_find *find);

struct jak_carbon_array_it *carbon_find_result_array(struct jak_carbon_find *find);

struct jak_carbon_object_it *carbon_find_result_object(struct jak_carbon_find *find);

struct jak_carbon_column_it *carbon_find_result_column(struct jak_carbon_find *find);

bool carbon_find_result_boolean(bool *out, struct jak_carbon_find *find);

bool carbon_find_result_unsigned(u64 *out, struct jak_carbon_find *find);

bool carbon_find_result_signed(i64 *out, struct jak_carbon_find *find);

bool carbon_find_result_float(float *out, struct jak_carbon_find *find);

const char *carbon_find_result_string(u64 *str_len, struct jak_carbon_find *find);

struct jak_carbon_binary *carbon_find_result_binary(struct jak_carbon_find *find);

bool carbon_find_drop(struct jak_carbon_find *find);

JAK_END_DECL

#endif
