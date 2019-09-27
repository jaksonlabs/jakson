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

#ifndef CARBON_PATH_INDEX_H
#define CARBON_PATH_INDEX_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/mem/file.h>
#include <jakson/mem/block.h>
#include <jakson/error.h>
#include <jakson/carbon.h>
#include <jakson/carbon/find.h>
#include <jakson/carbon/dot.h>

BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

typedef struct carbon_path_index {
        memblock *memblock;
        memfile memfile;
        err err;
} carbon_path_index;

typedef struct carbon_path_index_it {
        carbon *doc;
        memfile memfile;
        err err;

        carbon_container_e container_type;
        u64 pos;
} carbon_path_index_it;

typedef enum {
        PATH_ROOT, PATH_INDEX_PROP_KEY, PATH_INDEX_ARRAY_INDEX, PATH_INDEX_COLUMN_INDEX
} path_index_node_e;

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_create(carbon_path_index *index, carbon *doc);
bool carbon_path_index_drop(carbon_path_index *index);

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *carbon_path_index_raw_data(u64 *size, carbon_path_index *index);
bool carbon_path_index_commit_hash(u64 *commit_hash, carbon_path_index *index);
bool carbon_path_index_key_type(carbon_key_e *key_type, carbon_path_index *index);
bool carbon_path_index_key_unsigned_value(u64 *key, carbon_path_index *index);
bool carbon_path_index_key_signed_value(i64 *key, carbon_path_index *index);
const char *carbon_path_index_key_string_value(u64 *str_len, carbon_path_index *index);
bool carbon_path_index_indexes_doc(carbon_path_index *index, carbon *doc);

// ---------------------------------------------------------------------------------------------------------------------
//  index access and type information
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_open(carbon_path_index_it *it, carbon_path_index *index, carbon *doc);
bool carbon_path_index_it_type(carbon_container_e *type, carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  array and column container functions
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_list_length(u64 *key_len, carbon_path_index_it *it);
bool carbon_path_index_it_list_goto(u64 pos, carbon_path_index_it *it);
bool carbon_path_index_it_list_pos(u64 *pos, carbon_path_index_it *it);
bool carbon_path_index_it_list_can_enter(carbon_path_index_it *it);
bool carbon_path_index_it_list_enter(carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  object container functions
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_obj_num_props(u64 *num_props, carbon_path_index_it *it);
bool carbon_path_index_it_obj_goto(const char *key_name, carbon_path_index_it *it);
const char *carbon_path_index_it_key_name(u64 *name_len, carbon_path_index_it *it);
bool carbon_path_index_it_obj_can_enter(carbon_path_index_it *it);
bool carbon_path_index_it_obj_enter(carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  field access
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_field_type(carbon_field_type_e *type, carbon_path_index_it *it);
bool carbon_path_index_it_field_u8_value(u8 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_u16_value(u16 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_u32_value(u32 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_u64_value(u64 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_i8_value(i8 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_i16_value(i16 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_i32_value(i32 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_i64_value(i64 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_float_value(bool *is_null_in, float *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_signed_value(bool *is_null_in, i64 *value, carbon_path_index_it *it);
bool carbon_path_index_it_field_unsigned_value(bool *is_null_in, u64 *value, carbon_path_index_it *it);
const char *carbon_path_index_it_field_string_value(u64 *strlen, carbon_path_index_it *it);
bool carbon_path_index_it_field_binary_value(carbon_binary *out, carbon_array_it *it);
bool carbon_path_index_it_field_array_value(carbon_array_it *it_out, carbon_path_index_it *it_in);
bool carbon_path_index_it_field_object_value(carbon_object_it *it_out, carbon_path_index_it *it_in);
bool carbon_path_index_it_field_column_value(carbon_column_it *it_out, carbon_path_index_it *it_in);

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_hexdump(FILE *file, carbon_path_index *index);
fn_result carbon_path_index_to_carbon(carbon *doc, carbon_path_index *index);
const char *carbon_path_index_to_str(string_buffer *str, carbon_path_index *index);
bool carbon_path_index_print(FILE *file, carbon_path_index *index);

END_DECL

#endif
