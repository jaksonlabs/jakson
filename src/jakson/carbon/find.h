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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/carbon.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/object_it.h>
#include <jakson/carbon/containers.h>
#include <jakson/carbon/dot.h>
#include <jakson/carbon/path.h>

BEGIN_DECL

typedef struct carbon_find {
        carbon *doc;
        carbon_field_type_e type;
        err err;
        carbon_path_evaluator path_evaluater;

        bool value_is_nulled;

        union {
                carbon_array_it *array_it;
                carbon_column_it *column_it;
                carbon_object_it *object_it;
                bool boolean;
                u64 unsigned_number;
                i64 signed_number;
                float float_number;

                struct {
                        const char *base;
                        u64 len;
                } string;

                carbon_binary binary;
        } value;
} carbon_find;

bool carbon_find_begin(carbon_find *out, const char *dot_path, carbon *doc);
fn_result carbon_find_end(carbon_find *find);
fn_result carbon_find_create(carbon_find *find, carbon_dot_path *path, carbon *doc);
fn_result carbon_find_drop(carbon_find *find);

fn_result ofType(bool) carbon_find_has_result(carbon_find *find);
fn_result ofType(const char *) carbon_find_result_to_str(string_buffer *dst_str, carbon_printer_impl_e print_type, carbon_find *find);
fn_result ofType(const char *) carbon_find_result_to_json_compact(string_buffer *dst_str, carbon_find *find);
fn_result ofType(char *) carbon_find_result_to_json_compact_dup(carbon_find *find);

fn_result carbon_find_result_type(carbon_field_type_e *type, carbon_find *find);

fn_result carbon_find_update_array_type(carbon_find *find, carbon_list_derivable_e derivation);
fn_result ofType(bool) carbon_find_array_is_multiset(carbon_find *find);
fn_result ofType(bool) carbon_find_array_is_sorted(carbon_find *find);

fn_result carbon_find_update_column_type(carbon_find *find, carbon_list_derivable_e derivation);
fn_result ofType(bool) carbon_find_column_is_multiset(carbon_find *find);
fn_result ofType(bool) carbon_find_column_is_sorted(carbon_find *find);

fn_result carbon_find_update_object_type(carbon_find *find, carbon_map_derivable_e derivation);
fn_result ofType(bool) carbon_find_object_is_multimap(carbon_find *find);
fn_result ofType(bool) carbon_find_object_is_sorted(carbon_find *find);

fn_result ofType(bool) carbon_find_multimap(carbon_find *find);
fn_result ofType(bool) carbon_find_multiset(carbon_find *find);
fn_result ofType(bool) carbon_find_sorted(carbon_find *find);

fn_result ofType(carbon_array_it *) carbon_find_result_array(carbon_find *find);
fn_result ofType(carbon_object_it *) carbon_find_result_object(carbon_find *find);
fn_result ofType(carbon_column_it *) carbon_find_result_column(carbon_find *find);
fn_result ofType(bool) carbon_find_result_boolean(carbon_find *find);
fn_result carbon_find_result_unsigned(u64 *out, carbon_find *find);
fn_result carbon_find_result_signed(i64 *out, carbon_find *find);
fn_result carbon_find_result_float(float *out, carbon_find *find);
fn_result ofType(const char *) carbon_find_result_string(u64 *str_len, carbon_find *find);
fn_result ofType(carbon_binary *) carbon_find_result_binary(carbon_find *find);

END_DECL

#endif
