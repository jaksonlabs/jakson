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

#ifndef CARBON_INSERT_H
#define CARBON_INSERT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/mem/block.h>
#include <jakson/mem/file.h>
#include <jakson/std/spinlock.h>
#include <jakson/carbon.h>
#include <jakson/carbon/internal.h>

BEGIN_DECL

fn_result carbon_int_insert_create_for_array(carbon_insert *inserter, carbon_array_it *context);
bool carbon_int_insert_create_for_column(carbon_insert *inserter, carbon_column_it *context);
bool carbon_int_insert_create_for_object(carbon_insert *inserter, carbon_object_it *context);

bool carbon_insert_null(carbon_insert *inserter);
bool carbon_insert_true(carbon_insert *inserter);
bool carbon_insert_false(carbon_insert *inserter);
bool carbon_insert_u8(carbon_insert *inserter, u8 value);
bool carbon_insert_u16(carbon_insert *inserter, u16 value);
bool carbon_insert_u32(carbon_insert *inserter, u32 value);
bool carbon_insert_u64(carbon_insert *inserter, u64 value);
bool carbon_insert_i8(carbon_insert *inserter, i8 value);
bool carbon_insert_i16(carbon_insert *inserter, i16 value);
bool carbon_insert_i32(carbon_insert *inserter, i32 value);
bool carbon_insert_i64(carbon_insert *inserter, i64 value);
bool carbon_insert_unsigned(carbon_insert *inserter, u64 value);
bool carbon_insert_signed(carbon_insert *inserter, i64 value);
bool carbon_insert_float(carbon_insert *inserter, float value);
bool carbon_insert_string(carbon_insert *inserter, const char *value);
bool carbon_insert_nchar(carbon_insert *inserter, const char *value, u64 value_len);
/**
 * Inserts a user-defined binary string <code>value</code> of <code>nbytes</code> bytes along with a (mime) type annotation.
 * The type annotation is automatically found if <code>file_ext</code> is non-null and known to the system. If it is
 * not known or null, the non-empty <code>user_type</code> string is used to encode the mime annotation. In case
 * <code>user_type</code> is null (or empty) and <code>file_ext</code> is null (or not known), the mime type is set to
 * <code>application/octet-stream</code>, which encodes arbitrary binary data.
 */
bool carbon_insert_binary(carbon_insert *inserter, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

carbon_insert *carbon_insert_object_begin(carbon_insert_object_state *out, carbon_insert *inserter, u64 object_capacity);
bool carbon_insert_object_end(carbon_insert_object_state *state);

carbon_insert *carbon_insert_object_map_begin(carbon_insert_object_state *out, carbon_insert *inserter,
                                              carbon_map_derivable_e derivation, u64 object_capacity);
bool carbon_insert_object_map_end(carbon_insert_object_state *state);

carbon_insert *carbon_insert_array_begin(carbon_insert_array_state *state_out, carbon_insert *inserter_in, u64 array_capacity);
bool carbon_insert_array_end(carbon_insert_array_state *state_in);

carbon_insert *carbon_insert_array_list_begin(carbon_insert_array_state *state_out, carbon_insert *inserter_in, carbon_list_derivable_e derivation, u64 array_capacity);
bool carbon_insert_array_list_end(carbon_insert_array_state *state_in);

carbon_insert *carbon_insert_column_begin(carbon_insert_column_state *state_out, carbon_insert *inserter_in, carbon_column_type_e type, u64 column_capacity);
bool carbon_insert_column_end(carbon_insert_column_state *state_in);

carbon_insert *carbon_insert_column_list_begin(carbon_insert_column_state *state_out, carbon_insert *inserter_in, carbon_list_derivable_e derivation, carbon_column_type_e type, u64 column_capacity);
bool carbon_insert_column_list_end(carbon_insert_column_state *state_in);

bool carbon_insert_prop_null(carbon_insert *inserter, const char *key);
bool carbon_insert_prop_true(carbon_insert *inserter, const char *key);
bool carbon_insert_prop_false(carbon_insert *inserter, const char *key);
bool carbon_insert_prop_u8(carbon_insert *inserter, const char *key, u8 value);
bool carbon_insert_prop_u16(carbon_insert *inserter, const char *key, u16 value);
bool carbon_insert_prop_u32(carbon_insert *inserter, const char *key, u32 value);
bool carbon_insert_prop_u64(carbon_insert *inserter, const char *key, u64 value);
bool carbon_insert_prop_i8(carbon_insert *inserter, const char *key, i8 value);
bool carbon_insert_prop_i16(carbon_insert *inserter, const char *key, i16 value);
bool carbon_insert_prop_i32(carbon_insert *inserter, const char *key, i32 value);
bool carbon_insert_prop_i64(carbon_insert *inserter, const char *key, i64 value);
bool carbon_insert_prop_unsigned(carbon_insert *inserter, const char *key, u64 value);
bool carbon_insert_prop_signed(carbon_insert *inserter, const char *key, i64 value);
bool carbon_insert_prop_float(carbon_insert *inserter, const char *key, float value);
bool carbon_insert_prop_string(carbon_insert *inserter, const char *key, const char *value);
bool carbon_insert_prop_nchar(carbon_insert *inserter, const char *key, const char *value, u64 value_len);
bool carbon_insert_prop_binary(carbon_insert *inserter, const char *key, const void *value, size_t nbytes, const char *file_ext, const char *user_type);

carbon_insert *carbon_insert_prop_object_begin(carbon_insert_object_state *out, carbon_insert *inserter, const char *key, u64 object_capacity);
u64 carbon_insert_prop_object_end(carbon_insert_object_state *state);

carbon_insert *carbon_insert_prop_map_begin(carbon_insert_object_state *out, carbon_insert *inserter, carbon_map_derivable_e derivation, const char *key, u64 object_capacity);
u64 carbon_insert_prop_map_end(carbon_insert_object_state *state);

carbon_insert *carbon_insert_prop_array_begin(carbon_insert_array_state *state, carbon_insert *inserter, const char *key, u64 array_capacity);
u64 carbon_insert_prop_array_end(carbon_insert_array_state *state);

carbon_insert *carbon_insert_prop_column_begin(carbon_insert_column_state *state_out, carbon_insert *inserter_in, const char *key, carbon_column_type_e type, u64 column_capacity);
u64 carbon_insert_prop_column_end(carbon_insert_column_state *state_in);

fn_result carbon_insert_drop(carbon_insert *inserter);

END_DECL

#endif
