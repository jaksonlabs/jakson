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

#ifndef JAK_CARBON_UPDATE_H
#define JAK_CARBON_UPDATE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_path.h>
#include <jak_carbon_int.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_update {
        jak_carbon_revise *context;
        jak_carbon_path_evaluator path_evaluater;
        const jak_carbon_dot_path *path;
        jak_error err;
        bool is_found;
} jak_carbon_update;

JAK_DEFINE_ERROR_GETTER(jak_carbon_update)

bool jak_carbon_update_set_null(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_true(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_false(jak_carbon_revise *context, const char *path);
bool jak_carbon_update_set_u8(jak_carbon_revise *context, const char *path, jak_u8 value);
bool jak_carbon_update_set_u16(jak_carbon_revise *context, const char *path, jak_u16 value);
bool jak_carbon_update_set_u32(jak_carbon_revise *context, const char *path, jak_u32 value);
bool jak_carbon_update_set_u64(jak_carbon_revise *context, const char *path, jak_u64 value);
bool jak_carbon_update_set_i8(jak_carbon_revise *context, const char *path, jak_i8 value);
bool jak_carbon_update_set_i16(jak_carbon_revise *context, const char *path, jak_i16 value);
bool jak_carbon_update_set_i32(jak_carbon_revise *context, const char *path, jak_i32 value);
bool jak_carbon_update_set_i64(jak_carbon_revise *context, const char *path, jak_i64 value);
bool jak_carbon_update_set_float(jak_carbon_revise *context, const char *path, float value);
bool jak_carbon_update_set_unsigned(jak_carbon_revise *context, const char *path, jak_u64 value);
bool jak_carbon_update_set_signed(jak_carbon_revise *context, const char *path, jak_i64 value);
bool jak_carbon_update_set_string(jak_carbon_revise *context, const char *path, const char *value);
bool jak_carbon_update_set_binary(jak_carbon_revise *context, const char *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_update_set_array_begin(jak_carbon_revise *context, const char *path, jak_carbon_insert_array_state *state_out, jak_u64 array_capacity);
bool jak_carbon_update_set_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *jak_carbon_update_set_column_begin(jak_carbon_revise *context, const char *path, jak_carbon_insert_column_state *state_out, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool jak_carbon_update_set_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_update_set_null_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);
bool jak_carbon_update_set_true_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);
bool jak_carbon_update_set_false_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path);

bool jak_carbon_update_set_u8_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u8 value);
bool jak_carbon_update_set_u16_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u16 value);
bool jak_carbon_update_set_u32_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u32 value);
bool jak_carbon_update_set_u64_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u64 value);
bool jak_carbon_update_set_i8_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i8 value);
bool jak_carbon_update_set_i16_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i16 value);
bool jak_carbon_update_set_i32_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i32 value);
bool jak_carbon_update_set_i64_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i64 value);
bool jak_carbon_update_set_float_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, float value);
bool jak_carbon_update_set_unsigned_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_u64 value);
bool jak_carbon_update_set_signed_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_i64 value);
bool jak_carbon_update_set_string_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, const char *value);
bool jak_carbon_update_set_binary_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert * jak_carbon_update_set_array_begin_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_carbon_insert_array_state *state_out, jak_u64 array_capacity);
bool jak_carbon_update_set_array_end_compiled(jak_carbon_insert_array_state *state_in);
jak_carbon_insert *jak_carbon_update_set_column_begin_compiled(jak_carbon_revise *context, const jak_carbon_dot_path *path, jak_carbon_insert_column_state *state_out, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool jak_carbon_update_set_column_end_compiled(jak_carbon_insert_column_state *state_in);

bool carbon_update_one_set_null(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_true(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_false(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_u8(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u8 value);
bool carbon_update_one_set_u16(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u16 value);
bool carbon_update_one_set_u32(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u32 value);
bool carbon_update_one_set_u64(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_i8(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i8 value);
bool carbon_update_one_set_i16(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i16 value);
bool carbon_update_one_set_i32(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i32 value);
bool carbon_update_one_set_i64(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_float(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, float value);
bool carbon_update_one_set_unsigned(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_signed(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_string(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, const char *value);
bool carbon_update_one_set_binary(const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert *carbon_update_one_set_array_begin(jak_carbon_insert_array_state *state_out, const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 array_capacity);
bool carbon_update_one_set_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *carbon_update_one_set_column_begin(jak_carbon_insert_column_state *state_out, const char *dot_path, jak_carbon *rev_doc, jak_carbon *doc, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool carbon_update_one_set_column_end(jak_carbon_insert_column_state *state_in);

bool carbon_update_one_set_null_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_true_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_false_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc);
bool carbon_update_one_set_u8_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u8 value);
bool carbon_update_one_set_u16_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u16 value);
bool carbon_update_one_set_u32_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u32 value);
bool carbon_update_one_set_u64_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_i8_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i8 value);
bool carbon_update_one_set_i16_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i16 value);
bool carbon_update_one_set_i32_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i32 value);
bool carbon_update_one_set_i64_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_float_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, float value);
bool carbon_update_one_set_unsigned_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 value);
bool carbon_update_one_set_signed_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_i64 value);
bool carbon_update_one_set_string_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, const char *value);
bool carbon_update_one_set_binary_compiled(const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, const void *value, size_t nbytes, const char *file_ext, const char *user_type);
jak_carbon_insert *carbon_update_one_set_array_begin_compiled(jak_carbon_insert_array_state *state_out, const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_u64 array_capacity);
bool carbon_update_one_set_array_end_compiled(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *carbon_update_one_set_column_begin_compiled(jak_carbon_insert_column_state *state_out, const jak_carbon_dot_path *path, jak_carbon *rev_doc, jak_carbon *doc, jak_carbon_field_type_e type, jak_u64 column_capacity);
bool carbon_update_one_set_column_end_compiled(jak_carbon_insert_column_state *state_in);

JAK_END_DECL

#endif
