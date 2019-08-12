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

#ifndef CARBON_FIND_H
#define CARBON_FIND_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-path.h>

ARK_BEGIN_DECL

struct carbon_find {
    struct carbon *doc;
    enum carbon_field_type type;
    struct err err;
    struct carbon_path_evaluator path_evaluater;

    bool value_is_nulled;

    union {
        struct carbon_array_it *array_it;
        struct carbon_column_it *column_it;
        bool boolean;
        u64 unsigned_number;
        i64 signed_number;
        float float_number;

        struct {
            const char *base;
            u64 len;
        } string;

        struct carbon_binary binary;
    } value;
};

ARK_DEFINE_ERROR_GETTER(carbon_find)

bool carbon_find_open(struct carbon_find *out, const char *dot_path, struct carbon *doc);

bool carbon_find_close(struct carbon_find *find);

bool carbon_find_create(struct carbon_find *find, struct carbon_dot_path *path, struct carbon *doc);

bool carbon_find_has_result(struct carbon_find *find);

bool carbon_find_result_type(enum carbon_field_type *type, struct carbon_find *find);

struct carbon_array_it *carbon_find_result_array(struct carbon_find *find);

struct carbon_column_it *carbon_find_result_column(struct carbon_find *find);

bool carbon_find_result_boolean(bool *out, struct carbon_find *find);

bool carbon_find_result_unsigned(u64 *out, struct carbon_find *find);

bool carbon_find_result_signed(i64 *out, struct carbon_find *find);

bool carbon_find_result_float(float *out, struct carbon_find *find);

const char *carbon_find_result_string(u64 *str_len, struct carbon_find *find);

struct carbon_binary *carbon_find_result_binary(struct carbon_find *find);

bool carbon_find_drop(struct carbon_find *find);

ARK_END_DECL

#endif
