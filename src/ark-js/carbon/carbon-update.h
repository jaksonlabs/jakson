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

#ifndef carbon_UPDATE_H
#define carbon_UPDATE_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/mem/block.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-path.h>
#include <ark-js/carbon/carbon-int.h>

NG5_BEGIN_DECL

struct carbon_update
{
        struct carbon_revise *context;
        struct carbon_path_evaluator path_evaluater;
        const struct carbon_dot_path *path;
        struct err err;
        bool is_found;
};

NG5_DEFINE_ERROR_GETTER(carbon_update)

NG5_EXPORT(bool) carbon_update_set_null(struct carbon_revise *context, const char *path);

NG5_EXPORT(bool) carbon_update_set_true(struct carbon_revise *context, const char *path);

NG5_EXPORT(bool) carbon_update_set_false(struct carbon_revise *context, const char *path);

NG5_EXPORT(bool) carbon_update_set_u8(struct carbon_revise *context, const char *path, u8 value);

NG5_EXPORT(bool) carbon_update_set_u16(struct carbon_revise *context, const char *path, u16 value);

NG5_EXPORT(bool) carbon_update_set_u32(struct carbon_revise *context, const char *path, u32 value);

NG5_EXPORT(bool) carbon_update_set_u64(struct carbon_revise *context, const char *path, u64 value);

NG5_EXPORT(bool) carbon_update_set_i8(struct carbon_revise *context, const char *path, i8 value);

NG5_EXPORT(bool) carbon_update_set_i16(struct carbon_revise *context, const char *path, i16 value);

NG5_EXPORT(bool) carbon_update_set_i32(struct carbon_revise *context, const char *path, i32 value);

NG5_EXPORT(bool) carbon_update_set_i64(struct carbon_revise *context, const char *path, i64 value);

NG5_EXPORT(bool) carbon_update_set_float(struct carbon_revise *context, const char *path, float value);

NG5_EXPORT(bool) carbon_update_set_unsigned(struct carbon_revise *context, const char *path, u64 value);

NG5_EXPORT(bool) carbon_update_set_signed(struct carbon_revise *context, const char *path, i64 value);

NG5_EXPORT(bool) carbon_update_set_string(struct carbon_revise *context, const char *path, const char *value);

NG5_EXPORT(bool) carbon_update_set_binary(struct carbon_revise *context, const char *path, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type);

NG5_EXPORT(struct carbon_insert *) carbon_update_set_array_begin(struct carbon_revise *context, const char *path,
        struct carbon_insert_array_state *state_out, u64 array_capacity);

NG5_EXPORT(bool) carbon_update_set_array_end(struct carbon_insert_array_state *state_in);

NG5_EXPORT(struct carbon_insert *) carbon_update_set_column_begin(struct carbon_revise *context, const char *path,
        struct carbon_insert_column_state *state_out, enum carbon_field_type type, u64 column_capacity);

NG5_EXPORT(bool) carbon_update_set_column_end(struct carbon_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) carbon_update_set_null_compiled(struct carbon_revise *context, const struct carbon_dot_path *path);

NG5_EXPORT(bool) carbon_update_set_true_compiled(struct carbon_revise *context, const struct carbon_dot_path *path);

NG5_EXPORT(bool) carbon_update_set_false_compiled(struct carbon_revise *context, const struct carbon_dot_path *path);

NG5_EXPORT(bool) carbon_update_set_u8_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, u8 value);

NG5_EXPORT(bool) carbon_update_set_u16_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, u16 value);

NG5_EXPORT(bool) carbon_update_set_u32_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, u32 value);

NG5_EXPORT(bool) carbon_update_set_u64_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, u64 value);

NG5_EXPORT(bool) carbon_update_set_i8_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, i8 value);

NG5_EXPORT(bool) carbon_update_set_i16_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, i16 value);

NG5_EXPORT(bool) carbon_update_set_i32_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, i32 value);

NG5_EXPORT(bool) carbon_update_set_i64_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, i64 value);

NG5_EXPORT(bool) carbon_update_set_float_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, float value);

NG5_EXPORT(bool) carbon_update_set_unsigned_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, u64 value);

NG5_EXPORT(bool) carbon_update_set_signed_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, i64 value);

NG5_EXPORT(bool) carbon_update_set_string_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, const char *value);

NG5_EXPORT(bool) carbon_update_set_binary_compiled(struct carbon_revise *context, const struct carbon_dot_path *path, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type);

NG5_EXPORT(struct carbon_insert *) carbon_update_set_array_begin_compiled(struct carbon_revise *context, const struct carbon_dot_path *path,
        struct carbon_insert_array_state *state_out, u64 array_capacity);

NG5_EXPORT(bool) carbon_update_set_array_end_compiled(struct carbon_insert_array_state *state_in);

NG5_EXPORT(struct carbon_insert *) carbon_update_set_column_begin_compiled(struct carbon_revise *context, const struct carbon_dot_path *path,
        struct carbon_insert_column_state *state_out, enum carbon_field_type type, u64 column_capacity);

NG5_EXPORT(bool) carbon_update_set_column_end_compiled(struct carbon_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) carbon_update_one_set_null(const char *dot_path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_true(const char *dot_path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_false(const char *dot_path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_u8(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, u8 value);

NG5_EXPORT(bool) carbon_update_one_set_u16(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, u16 value);

NG5_EXPORT(bool) carbon_update_one_set_u32(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, u32 value);

NG5_EXPORT(bool) carbon_update_one_set_u64(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, u64 value);

NG5_EXPORT(bool) carbon_update_one_set_i8(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, i8 value);

NG5_EXPORT(bool) carbon_update_one_set_i16(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, i16 value);

NG5_EXPORT(bool) carbon_update_one_set_i32(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, i32 value);

NG5_EXPORT(bool) carbon_update_one_set_i64(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, i64 value);

NG5_EXPORT(bool) carbon_update_one_set_float(const char *dot_path, struct carbon *rev_doc, struct carbon *doc,
        float value);

NG5_EXPORT(bool) carbon_update_one_set_unsigned(const char *dot_path, struct carbon *rev_doc, struct carbon *doc,
        u64 value);

NG5_EXPORT(bool) carbon_update_one_set_signed(const char *dot_path, struct carbon *rev_doc, struct carbon *doc, i64 value);

NG5_EXPORT(bool) carbon_update_one_set_string(const char *dot_path, struct carbon *rev_doc, struct carbon *doc,
        const char *value);

NG5_EXPORT(bool) carbon_update_one_set_binary(const char *dot_path, struct carbon *rev_doc, struct carbon *doc,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type);

NG5_EXPORT(struct carbon_insert *) carbon_update_one_set_array_begin(struct carbon_insert_array_state *state_out,
        const char *dot_path, struct carbon *rev_doc, struct carbon *doc, u64 array_capacity);

NG5_EXPORT(bool) carbon_update_one_set_array_end(struct carbon_insert_array_state *state_in);

NG5_EXPORT(struct carbon_insert *) carbon_update_one_set_column_begin(struct carbon_insert_column_state *state_out,
        const char *dot_path, struct carbon *rev_doc, struct carbon *doc, enum carbon_field_type type,
        u64 column_capacity);

NG5_EXPORT(bool) carbon_update_one_set_column_end(struct carbon_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) carbon_update_one_set_null_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_true_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_false_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc);

NG5_EXPORT(bool) carbon_update_one_set_u8_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, u8 value);

NG5_EXPORT(bool) carbon_update_one_set_u16_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, u16 value);

NG5_EXPORT(bool) carbon_update_one_set_u32_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, u32 value);

NG5_EXPORT(bool) carbon_update_one_set_u64_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, u64 value);

NG5_EXPORT(bool) carbon_update_one_set_i8_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, i8 value);

NG5_EXPORT(bool) carbon_update_one_set_i16_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, i16 value);

NG5_EXPORT(bool) carbon_update_one_set_i32_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, i32 value);

NG5_EXPORT(bool) carbon_update_one_set_i64_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, i64 value);

NG5_EXPORT(bool) carbon_update_one_set_float_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc,
        float value);

NG5_EXPORT(bool) carbon_update_one_set_unsigned_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc,
        u64 value);

NG5_EXPORT(bool) carbon_update_one_set_signed_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, i64 value);

NG5_EXPORT(bool) carbon_update_one_set_string_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc,
        const char *value);

NG5_EXPORT(bool) carbon_update_one_set_binary_compiled(const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type);

NG5_EXPORT(struct carbon_insert *) carbon_update_one_set_array_begin_compiled(struct carbon_insert_array_state *state_out,
        const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, u64 array_capacity);

NG5_EXPORT(bool) carbon_update_one_set_array_end_compiled(struct carbon_insert_array_state *state_in);

NG5_EXPORT(struct carbon_insert *) carbon_update_one_set_column_begin_compiled(struct carbon_insert_column_state *state_out,
        const struct carbon_dot_path *path, struct carbon *rev_doc, struct carbon *doc, enum carbon_field_type type,
        u64 column_capacity);

NG5_EXPORT(bool) carbon_update_one_set_column_end_compiled(struct carbon_insert_column_state *state_in);


NG5_END_DECL

#endif
