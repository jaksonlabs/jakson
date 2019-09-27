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

#ifndef CARBON_UPDATE_H
#define CARBON_UPDATE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/mem/block.h>
#include <jakson/mem/file.h>
#include <jakson/std/spinlock.h>
#include <jakson/carbon.h>
#include <jakson/carbon/dot.h>
#include <jakson/carbon/path.h>
#include <jakson/carbon/internal.h>

BEGIN_DECL

typedef struct carbon_update {
        carbon_revise *context;
        carbon_path_evaluator path_evaluater;
        const carbon_dot_path *path;
        err err;
        bool is_found;
} carbon_update;

bool carbon_update_set_null(carbon_revise *context, const char *path);
bool carbon_update_set_true(carbon_revise *context, const char *path);
bool carbon_update_set_false(carbon_revise *context, const char *path);
bool carbon_update_set_u8(carbon_revise *context, const char *path, u8 value);
bool carbon_update_set_u16(carbon_revise *context, const char *path, u16 value);
bool carbon_update_set_u32(carbon_revise *context, const char *path, u32 value);
bool carbon_update_set_u64(carbon_revise *context, const char *path, u64 value);
bool carbon_update_set_i8(carbon_revise *context, const char *path, i8 value);
bool carbon_update_set_i16(carbon_revise *context, const char *path, i16 value);
bool carbon_update_set_i32(carbon_revise *context, const char *path, i32 value);
bool carbon_update_set_i64(carbon_revise *context, const char *path, i64 value);
bool carbon_update_set_float(carbon_revise *context, const char *path, float value);
bool carbon_update_set_unsigned(carbon_revise *context, const char *path, u64 value);
bool carbon_update_set_signed(carbon_revise *context, const char *path, i64 value);
bool carbon_update_set_string(carbon_revise *context, const char *path, const char *value);
bool carbon_update_set_binary(carbon_revise *context, const char *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

carbon_insert *carbon_update_set_array_begin(carbon_revise *context, const char *path, carbon_insert_array_state *state_out, u64 array_capacity);
bool carbon_update_set_array_end(carbon_insert_array_state *state_in);

carbon_insert *carbon_update_set_column_begin(carbon_revise *context, const char *path, carbon_insert_column_state *state_out, carbon_field_type_e type, u64 column_capacity);
bool carbon_update_set_column_end(carbon_insert_column_state *state_in);

bool carbon_update_set_null_compiled(carbon_revise *context, const carbon_dot_path *path);
bool carbon_update_set_true_compiled(carbon_revise *context, const carbon_dot_path *path);
bool carbon_update_set_false_compiled(carbon_revise *context, const carbon_dot_path *path);

bool carbon_update_set_u8_compiled(carbon_revise *context, const carbon_dot_path *path, u8 value);
bool carbon_update_set_u16_compiled(carbon_revise *context, const carbon_dot_path *path, u16 value);
bool carbon_update_set_u32_compiled(carbon_revise *context, const carbon_dot_path *path, u32 value);
bool carbon_update_set_u64_compiled(carbon_revise *context, const carbon_dot_path *path, u64 value);
bool carbon_update_set_i8_compiled(carbon_revise *context, const carbon_dot_path *path, i8 value);
bool carbon_update_set_i16_compiled(carbon_revise *context, const carbon_dot_path *path, i16 value);
bool carbon_update_set_i32_compiled(carbon_revise *context, const carbon_dot_path *path, i32 value);
bool carbon_update_set_i64_compiled(carbon_revise *context, const carbon_dot_path *path, i64 value);
bool carbon_update_set_float_compiled(carbon_revise *context, const carbon_dot_path *path, float value);
bool carbon_update_set_unsigned_compiled(carbon_revise *context, const carbon_dot_path *path, u64 value);
bool carbon_update_set_signed_compiled(carbon_revise *context, const carbon_dot_path *path, i64 value);
bool carbon_update_set_string_compiled(carbon_revise *context, const carbon_dot_path *path, const char *value);
bool carbon_update_set_binary_compiled(carbon_revise *context, const carbon_dot_path *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
carbon_insert * carbon_update_set_array_begin_compiled(carbon_revise *context, const carbon_dot_path *path, carbon_insert_array_state *state_out, u64 array_capacity);
bool carbon_update_set_array_end_compiled(carbon_insert_array_state *state_in);
carbon_insert *carbon_update_set_column_begin_compiled(carbon_revise *context, const carbon_dot_path *path, carbon_insert_column_state *state_out, carbon_field_type_e type, u64 column_capacity);
bool carbon_update_set_column_end_compiled(carbon_insert_column_state *state_in);

bool carbon_update_one_set_null(const char *dot_path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_true(const char *dot_path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_false(const char *dot_path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_u8(const char *dot_path, carbon *rev_doc, carbon *doc, u8 value);
bool carbon_update_one_set_u16(const char *dot_path, carbon *rev_doc, carbon *doc, u16 value);
bool carbon_update_one_set_u32(const char *dot_path, carbon *rev_doc, carbon *doc, u32 value);
bool carbon_update_one_set_u64(const char *dot_path, carbon *rev_doc, carbon *doc, u64 value);
bool carbon_update_one_set_i8(const char *dot_path, carbon *rev_doc, carbon *doc, i8 value);
bool carbon_update_one_set_i16(const char *dot_path, carbon *rev_doc, carbon *doc, i16 value);
bool carbon_update_one_set_i32(const char *dot_path, carbon *rev_doc, carbon *doc, i32 value);
bool carbon_update_one_set_i64(const char *dot_path, carbon *rev_doc, carbon *doc, i64 value);
bool carbon_update_one_set_float(const char *dot_path, carbon *rev_doc, carbon *doc, float value);
bool carbon_update_one_set_unsigned(const char *dot_path, carbon *rev_doc, carbon *doc, u64 value);
bool carbon_update_one_set_signed(const char *dot_path, carbon *rev_doc, carbon *doc, i64 value);
bool carbon_update_one_set_string(const char *dot_path, carbon *rev_doc, carbon *doc, const char *value);
bool carbon_update_one_set_binary(const char *dot_path, carbon *rev_doc, carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
carbon_insert *carbon_update_one_set_array_begin(carbon_insert_array_state *state_out, const char *dot_path, carbon *rev_doc, carbon *doc, u64 array_capacity);
bool carbon_update_one_set_array_end(carbon_insert_array_state *state_in);

carbon_insert *carbon_update_one_set_column_begin(carbon_insert_column_state *state_out, const char *dot_path, carbon *rev_doc, carbon *doc, carbon_field_type_e type, u64 column_capacity);
bool carbon_update_one_set_column_end(carbon_insert_column_state *state_in);

bool carbon_update_one_set_null_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_true_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_false_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc);
bool carbon_update_one_set_u8_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u8 value);
bool carbon_update_one_set_u16_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u16 value);
bool carbon_update_one_set_u32_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u32 value);
bool carbon_update_one_set_u64_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u64 value);
bool carbon_update_one_set_i8_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, i8 value);
bool carbon_update_one_set_i16_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, i16 value);
bool carbon_update_one_set_i32_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, i32 value);
bool carbon_update_one_set_i64_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, i64 value);
bool carbon_update_one_set_float_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, float value);
bool carbon_update_one_set_unsigned_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u64 value);
bool carbon_update_one_set_signed_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, i64 value);
bool carbon_update_one_set_string_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, const char *value);
bool carbon_update_one_set_binary_compiled(const carbon_dot_path *path, carbon *rev_doc, carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
carbon_insert *carbon_update_one_set_array_begin_compiled(carbon_insert_array_state *state_out, const carbon_dot_path *path, carbon *rev_doc, carbon *doc, u64 array_capacity);
bool carbon_update_one_set_array_end_compiled(carbon_insert_array_state *state_in);

carbon_insert *carbon_update_one_set_column_begin_compiled(carbon_insert_column_state *state_out, const carbon_dot_path *path, carbon *rev_doc, carbon *doc, carbon_field_type_e type, u64 column_capacity);
bool carbon_update_one_set_column_end_compiled(carbon_insert_column_state *state_in);

END_DECL

#endif
