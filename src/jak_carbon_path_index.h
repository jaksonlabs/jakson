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

#ifndef JAK_CARBON_PATH_INDEX_H
#define JAK_CARBON_PATH_INDEX_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memfile.h>
#include <jak_memblock.h>
#include <jak_error.h>
#include <jak_carbon.h>
#include <jak_carbon_find.h>
#include <jak_carbon_dot.h>

JAK_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

typedef struct jak_carbon_path_index {
        jak_memblock *memblock;
        struct jak_memfile memfile;
        jak_error err;
} jak_carbon_path_index;

typedef struct jak_carbon_path_index_it {
        jak_carbon *doc;
        struct jak_memfile memfile;
        jak_error err;

        jak_carbon_container_e container_type;
        jak_u64 pos;
} jak_carbon_path_index_it;

typedef enum jak_path_index_node {
        JAK_PATH_ROOT, JAK_PATH_INDEX_PROP_KEY, JAK_PATH_INDEX_ARRAY_INDEX, JAK_PATH_INDEX_COLUMN_INDEX
} jak_path_index_node_e;

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_create(jak_carbon_path_index *index, jak_carbon *doc);
bool jak_carbon_path_index_drop(jak_carbon_path_index *index);

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *jak_carbon_path_index_raw_data(jak_u64 *size, jak_carbon_path_index *index);
bool jak_carbon_path_index_commit_hash(jak_u64 *commit_hash, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_type(jak_carbon_key_e *key_type, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_unsigned_value(jak_u64 *key, jak_carbon_path_index *index);
bool jak_carbon_path_index_key_signed_value(jak_i64 *key, jak_carbon_path_index *index);
const char *jak_carbon_path_index_key_string_value(jak_u64 *str_len, jak_carbon_path_index *index);
bool jak_carbon_path_index_indexes_doc(jak_carbon_path_index *index, jak_carbon *doc);

JAK_DEFINE_ERROR_GETTER(jak_carbon_path_index);

// ---------------------------------------------------------------------------------------------------------------------
//  index access and type information
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_open(jak_carbon_path_index_it *it, jak_carbon_path_index *index, jak_carbon *doc);
bool jak_carbon_path_index_it_type(jak_carbon_container_e *type, jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  array and column container functions
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_list_length(jak_u64 *key_len, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_goto(jak_u64 pos, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_pos(jak_u64 *pos, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_can_enter(jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_list_enter(jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  object container functions
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_obj_num_props(jak_u64 *num_props, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_goto(const char *key_name, jak_carbon_path_index_it *it);
const char *jak_carbon_path_index_it_key_name(jak_u64 *name_len, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_can_enter(jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_obj_enter(jak_carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  field access
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_it_field_type(jak_carbon_field_type_e *type, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u8_value(jak_u8 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u16_value(jak_u16 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u32_value(jak_u32 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_u64_value(jak_u64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i8_value(jak_i8 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i16_value(jak_i16 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i32_value(jak_i32 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_i64_value(jak_i64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_float_value(bool *is_null_in, float *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_signed_value(bool *is_null_in, jak_i64 *value, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_unsigned_value(bool *is_null_in, jak_u64 *value, jak_carbon_path_index_it *it);
const char *jak_carbon_path_index_it_field_string_value(jak_u64 *strlen, jak_carbon_path_index_it *it);
bool jak_carbon_path_index_it_field_binary_value(jak_carbon_binary *out, jak_carbon_array_it *it);
bool jak_carbon_path_index_it_field_array_value(jak_carbon_array_it *it_out, jak_carbon_path_index_it *it_in);
bool jak_carbon_path_index_it_field_object_value(jak_carbon_object_it *it_out, jak_carbon_path_index_it *it_in);
bool jak_carbon_path_index_it_field_column_value(jak_carbon_column_it *it_out, jak_carbon_path_index_it *it_in);

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool jak_carbon_path_index_hexdump(FILE *file, jak_carbon_path_index *index);
bool jak_carbon_path_index_to_carbon(jak_carbon *doc, jak_carbon_path_index *index);
const char *jak_carbon_path_index_to_str(struct jak_string *str, jak_carbon_path_index *index);
bool jak_carbon_path_index_print(FILE *file, jak_carbon_path_index *index);

JAK_END_DECL

#endif
