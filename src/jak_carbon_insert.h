/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef JAK_CARBON_INSERT_H
#define JAK_CARBON_INSERT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_int.h>

JAK_BEGIN_DECL

bool jak_carbon_int_insert_create_for_array(jak_carbon_insert *inserter, jak_carbon_array_it *context);
bool jak_carbon_int_insert_create_for_column(jak_carbon_insert *inserter, jak_carbon_column_it *context);
bool jak_carbon_int_insert_create_for_object(jak_carbon_insert *inserter, struct jak_carbon_object_it *context);

bool jak_carbon_insert_null(jak_carbon_insert *inserter);
bool jak_carbon_insert_true(jak_carbon_insert *inserter);
bool jak_carbon_insert_false(jak_carbon_insert *inserter);
bool jak_carbon_insert_u8(jak_carbon_insert *inserter, jak_u8 value);
bool jak_carbon_insert_u16(jak_carbon_insert *inserter, jak_u16 value);
bool jak_carbon_insert_u32(jak_carbon_insert *inserter, jak_u32 value);
bool jak_carbon_insert_u64(jak_carbon_insert *inserter, jak_u64 value);
bool jak_carbon_insert_i8(jak_carbon_insert *inserter, jak_i8 value);
bool jak_carbon_insert_i16(jak_carbon_insert *inserter, jak_i16 value);
bool jak_carbon_insert_i32(jak_carbon_insert *inserter, jak_i32 value);
bool jak_carbon_insert_i64(jak_carbon_insert *inserter, jak_i64 value);
bool jak_carbon_insert_unsigned(jak_carbon_insert *inserter, jak_u64 value);
bool jak_carbon_insert_signed(jak_carbon_insert *inserter, jak_i64 value);
bool jak_carbon_insert_float(jak_carbon_insert *inserter, float value);
bool jak_carbon_insert_string(jak_carbon_insert *inserter, const char *value);
bool jak_carbon_insert_nchar(jak_carbon_insert *inserter, const char *value, jak_u64 value_len);
/**
 * Inserts a user-defined binary string <code>value</code> of <code>nbytes</code> bytes along with a (mime) type annotation.
 * The type annotation is automatically found if <code>file_ext</code> is non-null and known to the system. If it is
 * not known or null, the non-empty <code>user_type</code> string is used to encode the mime annotation. In case
 * <code>user_type</code> is null (or empty) and <code>file_ext</code> is null (or not known), the mime type is set to
 * <code>application/octet-stream</code>, which encodes arbitrary binary data.
 */
bool jak_carbon_insert_binary(jak_carbon_insert *inserter, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_insert_object_begin(jak_carbon_insert_object_state *out, jak_carbon_insert *inserter, jak_u64 object_capacity);
bool jak_carbon_insert_object_end(jak_carbon_insert_object_state *state);

jak_carbon_insert *jak_carbon_insert_array_begin(jak_carbon_insert_array_state *state_out, jak_carbon_insert *inserter_in, jak_u64 array_capacity);
bool jak_carbon_insert_array_end(jak_carbon_insert_array_state *state_in);

jak_carbon_insert *jak_carbon_insert_column_begin(jak_carbon_insert_column_state *state_out, jak_carbon_insert *inserter_in, jak_carbon_column_type_e type, jak_u64 column_capacity);
bool jak_carbon_insert_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_insert_prop_null(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_true(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_false(jak_carbon_insert *inserter, const char *key);
bool jak_carbon_insert_prop_u8(jak_carbon_insert *inserter, const char *key, jak_u8 value);
bool jak_carbon_insert_prop_u16(jak_carbon_insert *inserter, const char *key, jak_u16 value);
bool jak_carbon_insert_prop_u32(jak_carbon_insert *inserter, const char *key, jak_u32 value);
bool jak_carbon_insert_prop_u64(jak_carbon_insert *inserter, const char *key, jak_u64 value);
bool jak_carbon_insert_prop_i8(jak_carbon_insert *inserter, const char *key, jak_i8 value);
bool jak_carbon_insert_prop_i16(jak_carbon_insert *inserter, const char *key, jak_i16 value);
bool jak_carbon_insert_prop_i32(jak_carbon_insert *inserter, const char *key, jak_i32 value);
bool jak_carbon_insert_prop_i64(jak_carbon_insert *inserter, const char *key, jak_i64 value);
bool jak_carbon_insert_prop_unsigned(jak_carbon_insert *inserter, const char *key, jak_u64 value);
bool jak_carbon_insert_prop_signed(jak_carbon_insert *inserter, const char *key, jak_i64 value);
bool jak_carbon_insert_prop_float(jak_carbon_insert *inserter, const char *key, float value);
bool jak_carbon_insert_prop_string(jak_carbon_insert *inserter, const char *key, const char *value);
bool jak_carbon_insert_prop_nchar(jak_carbon_insert *inserter, const char *key, const char *value, jak_u64 value_len);
bool jak_carbon_insert_prop_binary(jak_carbon_insert *inserter, const char *key, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

jak_carbon_insert *jak_carbon_insert_prop_object_begin(jak_carbon_insert_object_state *out, jak_carbon_insert *inserter, const char *key, jak_u64 object_capacity);
jak_u64 jak_carbon_insert_prop_object_end(jak_carbon_insert_object_state *state);

jak_carbon_insert *jak_carbon_insert_prop_array_begin(jak_carbon_insert_array_state *state, jak_carbon_insert *inserter, const char *key, jak_u64 array_capacity);
jak_u64 jak_carbon_insert_prop_array_end(jak_carbon_insert_array_state *state);

jak_carbon_insert *jak_carbon_insert_prop_column_begin(jak_carbon_insert_column_state *state_out, jak_carbon_insert *inserter_in, const char *key, jak_carbon_column_type_e type, jak_u64 column_capacity);
jak_u64 jak_carbon_insert_prop_column_end(jak_carbon_insert_column_state *state_in);

bool jak_carbon_insert_drop(jak_carbon_insert *inserter);

JAK_END_DECL

#endif
