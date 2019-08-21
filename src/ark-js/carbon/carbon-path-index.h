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

#include <ark-js/shared/common.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/shared/mem/block.h>
#include <ark-js/shared/error.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-find.h>
#include <ark-js/carbon/carbon-dot.h>

ARK_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

struct carbon_path_index
{
    struct memblock *memblock;
    struct memfile memfile;
    struct err err;
};

struct carbon_path_index_it
{
    struct carbon *doc;
    struct memfile memfile;
    struct err err;

    enum carbon_container_type container_type;
    u64 pos;
};

enum path_index_node_type { PATH_ROOT, PATH_INDEX_PROP_KEY, PATH_INDEX_ARRAY_INDEX, PATH_INDEX_COLUMN_INDEX };

// ---------------------------------------------------------------------------------------------------------------------
//  construction and deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_create(struct carbon_path_index *index, struct carbon *doc);
bool carbon_path_index_drop(struct carbon_path_index *index);

// ---------------------------------------------------------------------------------------------------------------------
//  index data access and meta information
// ---------------------------------------------------------------------------------------------------------------------

const void *carbon_path_index_raw_data(u64 *size, struct carbon_path_index *index);
bool carbon_path_index_commit_hash(u64 *commit_hash, struct carbon_path_index *index);
bool carbon_path_index_key_type(enum carbon_key_type *key_type, struct carbon_path_index *index);
bool carbon_path_index_key_unsigned_value(u64 *key, struct carbon_path_index *index);
bool carbon_path_index_key_signed_value(i64 *key, struct carbon_path_index *index);
const char *carbon_path_index_key_string_value(u64 *str_len, struct carbon_path_index *index);
bool carbon_path_index_indexes_doc(struct carbon_path_index *index, struct carbon *doc);

ARK_DEFINE_ERROR_GETTER(carbon_path_index);

// ---------------------------------------------------------------------------------------------------------------------
//  index access and type information
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_open(struct carbon_path_index_it *it, struct carbon_path_index *index, struct carbon *doc);
bool carbon_path_index_it_type(enum carbon_container_type *type, struct carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  array and column container functions
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_list_length(u64 *key_len, struct carbon_path_index_it *it);
bool carbon_path_index_it_list_goto(u64 pos, struct carbon_path_index_it *it);
bool carbon_path_index_it_list_pos(u64 *pos, struct carbon_path_index_it *it);
bool carbon_path_index_it_list_can_enter(struct carbon_path_index_it *it);
bool carbon_path_index_it_list_enter(struct carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  object container functions
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_obj_num_props(u64 *num_props, struct carbon_path_index_it *it);
bool carbon_path_index_it_obj_goto(const char *key_name, struct carbon_path_index_it *it);
const char *carbon_path_index_it_key_name(u64 *name_len, struct carbon_path_index_it *it);
bool carbon_path_index_it_obj_can_enter(struct carbon_path_index_it *it);
bool carbon_path_index_it_obj_enter(struct carbon_path_index_it *it);

// ---------------------------------------------------------------------------------------------------------------------
//  field access
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_it_field_type(enum carbon_field_type *type, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_u8_value(u8 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_u16_value(u16 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_u32_value(u32 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_u64_value(u64 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_i8_value(i8 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_i16_value(i16 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_i32_value(i32 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_i64_value(i64 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_float_value(bool *is_null_in, float *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_signed_value(bool *is_null_in, i64 *value, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_unsigned_value(bool *is_null_in, u64 *value, struct carbon_path_index_it *it);
const char *carbon_path_index_it_field_string_value(u64 *strlen, struct carbon_path_index_it *it);
bool carbon_path_index_it_field_binary_value(struct carbon_binary *out, struct carbon_array_it *it);
bool carbon_path_index_it_field_array_value(struct carbon_array_it *it_out, struct carbon_path_index_it *it_in);
bool carbon_path_index_it_field_object_value(struct carbon_object_it *it_out, struct carbon_path_index_it *it_in);
bool carbon_path_index_it_field_column_value(struct carbon_column_it *it_out, struct carbon_path_index_it *it_in);

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_path_index_hexdump(FILE *file, struct carbon_path_index *index);
bool carbon_path_index_to_carbon(struct carbon *doc, struct carbon_path_index *index);
const char*carbon_path_index_to_str(struct string *str, struct carbon_path_index *index);
bool carbon_path_index_print(FILE *file, struct carbon_path_index *index);

ARK_END_DECL

#endif
