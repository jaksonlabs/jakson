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

#ifndef BISON_UPDATE_H
#define BISON_UPDATE_H


#include "shared/common.h"
#include "shared/error.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/async/spin.h"
#include "core/bison/bison.h"
#include "core/bison/bison-dot.h"
#include "core/bison/bison-path.h"
#include "core/bison/bison-int.h"

NG5_BEGIN_DECL

struct bison_update
{
        struct bison_revise *context;
        struct bison_path_evaluator path_evaluater;
        const struct bison_dot_path *path;
        struct err err;
        bool is_found;
};

NG5_DEFINE_ERROR_GETTER(bison_update)

NG5_EXPORT(bool) bison_update_set_null(struct bison_revise *context, const char *path);

NG5_EXPORT(bool) bison_update_set_true(struct bison_revise *context, const char *path);

NG5_EXPORT(bool) bison_update_set_false(struct bison_revise *context, const char *path);

NG5_EXPORT(bool) bison_update_set_u8(struct bison_revise *context, const char *path, u8 value);

NG5_EXPORT(bool) bison_update_set_u16(struct bison_revise *context, const char *path, u16 value);

NG5_EXPORT(bool) bison_update_set_u32(struct bison_revise *context, const char *path, u32 value);

NG5_EXPORT(bool) bison_update_set_u64(struct bison_revise *context, const char *path, u64 value);

NG5_EXPORT(bool) bison_update_set_i8(struct bison_revise *context, const char *path, i8 value);

NG5_EXPORT(bool) bison_update_set_i16(struct bison_revise *context, const char *path, i16 value);

NG5_EXPORT(bool) bison_update_set_i32(struct bison_revise *context, const char *path, i32 value);

NG5_EXPORT(bool) bison_update_set_i64(struct bison_revise *context, const char *path, i64 value);

NG5_EXPORT(bool) bison_update_set_float(struct bison_revise *context, const char *path, float value);

NG5_EXPORT(bool) bison_update_set_unsigned(struct bison_revise *context, const char *path, u64 value);

NG5_EXPORT(bool) bison_update_set_signed(struct bison_revise *context, const char *path, i64 value);

NG5_EXPORT(bool) bison_update_set_string(struct bison_revise *context, const char *path, const char *value);

NG5_EXPORT(bool) bison_update_set_binary(struct bison_revise *context, const char *path, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type);

NG5_EXPORT(struct bison_insert *) bison_update_set_array_begin(struct bison_revise *context, const char *path,
        struct bison_insert_array_state *state_out, u64 array_capacity);

NG5_EXPORT(bool) bison_update_set_array_end(struct bison_insert_array_state *state_in);

NG5_EXPORT(struct bison_insert *) bison_update_set_column_begin(struct bison_revise *context, const char *path,
        struct bison_insert_column_state *state_out, enum bison_field_type type, u64 column_capacity);

NG5_EXPORT(bool) bison_update_set_column_end(struct bison_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_update_set_null_compiled(struct bison_revise *context, const struct bison_dot_path *path);

NG5_EXPORT(bool) bison_update_set_true_compiled(struct bison_revise *context, const struct bison_dot_path *path);

NG5_EXPORT(bool) bison_update_set_false_compiled(struct bison_revise *context, const struct bison_dot_path *path);

NG5_EXPORT(bool) bison_update_set_u8_compiled(struct bison_revise *context, const struct bison_dot_path *path, u8 value);

NG5_EXPORT(bool) bison_update_set_u16_compiled(struct bison_revise *context, const struct bison_dot_path *path, u16 value);

NG5_EXPORT(bool) bison_update_set_u32_compiled(struct bison_revise *context, const struct bison_dot_path *path, u32 value);

NG5_EXPORT(bool) bison_update_set_u64_compiled(struct bison_revise *context, const struct bison_dot_path *path, u64 value);

NG5_EXPORT(bool) bison_update_set_i8_compiled(struct bison_revise *context, const struct bison_dot_path *path, i8 value);

NG5_EXPORT(bool) bison_update_set_i16_compiled(struct bison_revise *context, const struct bison_dot_path *path, i16 value);

NG5_EXPORT(bool) bison_update_set_i32_compiled(struct bison_revise *context, const struct bison_dot_path *path, i32 value);

NG5_EXPORT(bool) bison_update_set_i64_compiled(struct bison_revise *context, const struct bison_dot_path *path, i64 value);

NG5_EXPORT(bool) bison_update_set_float_compiled(struct bison_revise *context, const struct bison_dot_path *path, float value);

NG5_EXPORT(bool) bison_update_set_unsigned_compiled(struct bison_revise *context, const struct bison_dot_path *path, u64 value);

NG5_EXPORT(bool) bison_update_set_signed_compiled(struct bison_revise *context, const struct bison_dot_path *path, i64 value);

NG5_EXPORT(bool) bison_update_set_string_compiled(struct bison_revise *context, const struct bison_dot_path *path, const char *value);

NG5_EXPORT(bool) bison_update_set_binary_compiled(struct bison_revise *context, const struct bison_dot_path *path, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type);

NG5_EXPORT(struct bison_insert *) bison_update_set_array_begin_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        struct bison_insert_array_state *state_out, u64 array_capacity);

NG5_EXPORT(bool) bison_update_set_array_end_compiled(struct bison_insert_array_state *state_in);

NG5_EXPORT(struct bison_insert *) bison_update_set_column_begin_compiled(struct bison_revise *context, const struct bison_dot_path *path,
        struct bison_insert_column_state *state_out, enum bison_field_type type, u64 column_capacity);

NG5_EXPORT(bool) bison_update_set_column_end_compiled(struct bison_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_update_one_set_null(const char *dot_path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_true(const char *dot_path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_false(const char *dot_path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_u8(const char *dot_path, struct bison *rev_doc, struct bison *doc, u8 value);

NG5_EXPORT(bool) bison_update_one_set_u16(const char *dot_path, struct bison *rev_doc, struct bison *doc, u16 value);

NG5_EXPORT(bool) bison_update_one_set_u32(const char *dot_path, struct bison *rev_doc, struct bison *doc, u32 value);

NG5_EXPORT(bool) bison_update_one_set_u64(const char *dot_path, struct bison *rev_doc, struct bison *doc, u64 value);

NG5_EXPORT(bool) bison_update_one_set_i8(const char *dot_path, struct bison *rev_doc, struct bison *doc, i8 value);

NG5_EXPORT(bool) bison_update_one_set_i16(const char *dot_path, struct bison *rev_doc, struct bison *doc, i16 value);

NG5_EXPORT(bool) bison_update_one_set_i32(const char *dot_path, struct bison *rev_doc, struct bison *doc, i32 value);

NG5_EXPORT(bool) bison_update_one_set_i64(const char *dot_path, struct bison *rev_doc, struct bison *doc, i64 value);

NG5_EXPORT(bool) bison_update_one_set_float(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        float value);

NG5_EXPORT(bool) bison_update_one_set_unsigned(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        u64 value);

NG5_EXPORT(bool) bison_update_one_set_signed(const char *dot_path, struct bison *rev_doc, struct bison *doc, i64 value);

NG5_EXPORT(bool) bison_update_one_set_string(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        const char *value);

NG5_EXPORT(bool) bison_update_one_set_binary(const char *dot_path, struct bison *rev_doc, struct bison *doc,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type);

NG5_EXPORT(struct bison_insert *) bison_update_one_set_array_begin(struct bison_insert_array_state *state_out,
        const char *dot_path, struct bison *rev_doc, struct bison *doc, u64 array_capacity);

NG5_EXPORT(bool) bison_update_one_set_array_end(struct bison_insert_array_state *state_in);

NG5_EXPORT(struct bison_insert *) bison_update_one_set_column_begin(struct bison_insert_column_state *state_out,
        const char *dot_path, struct bison *rev_doc, struct bison *doc, enum bison_field_type type,
        u64 column_capacity);

NG5_EXPORT(bool) bison_update_one_set_column_end(struct bison_insert_column_state *state_in);

// ---------------------------------------------------------------------------------------------------------------------

NG5_EXPORT(bool) bison_update_one_set_null_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_true_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_false_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_update_one_set_u8_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u8 value);

NG5_EXPORT(bool) bison_update_one_set_u16_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u16 value);

NG5_EXPORT(bool) bison_update_one_set_u32_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u32 value);

NG5_EXPORT(bool) bison_update_one_set_u64_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u64 value);

NG5_EXPORT(bool) bison_update_one_set_i8_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, i8 value);

NG5_EXPORT(bool) bison_update_one_set_i16_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, i16 value);

NG5_EXPORT(bool) bison_update_one_set_i32_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, i32 value);

NG5_EXPORT(bool) bison_update_one_set_i64_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, i64 value);

NG5_EXPORT(bool) bison_update_one_set_float_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc,
        float value);

NG5_EXPORT(bool) bison_update_one_set_unsigned_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc,
        u64 value);

NG5_EXPORT(bool) bison_update_one_set_signed_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, i64 value);

NG5_EXPORT(bool) bison_update_one_set_string_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc,
        const char *value);

NG5_EXPORT(bool) bison_update_one_set_binary_compiled(const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc,
        const void *value, size_t nbytes, const char *file_ext, const char *user_type);

NG5_EXPORT(struct bison_insert *) bison_update_one_set_array_begin_compiled(struct bison_insert_array_state *state_out,
        const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, u64 array_capacity);

NG5_EXPORT(bool) bison_update_one_set_array_end_compiled(struct bison_insert_array_state *state_in);

NG5_EXPORT(struct bison_insert *) bison_update_one_set_column_begin_compiled(struct bison_insert_column_state *state_out,
        const struct bison_dot_path *path, struct bison *rev_doc, struct bison *doc, enum bison_field_type type,
        u64 column_capacity);

NG5_EXPORT(bool) bison_update_one_set_column_end_compiled(struct bison_insert_column_state *state_in);


NG5_END_DECL

#endif
