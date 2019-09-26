/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file is for internal usage only; do not call these functions from outside
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

#ifndef JAK_CARBON_INT_H
#define JAK_CARBON_INT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memfile.h>
#include <jak_uintvar_stream.h>
#include <jak_json.h>
#include <jak_carbon_int.h>
#include <jak_carbon_containers.h>
#include <jak_carbon_field.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_abstract.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_insert {
        jak_carbon_container_e context_type;
        union {
                jak_carbon_array_it *array;
                jak_carbon_column_it *column;
                jak_carbon_object_it *object;
        } context;

        jak_memfile memfile;
        jak_offset_t position;
        jak_error err;
} jak_carbon_insert;

typedef struct jak_carbon_insert_array_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_array_it *nested_array;
        jak_carbon_insert nested_inserter;
        jak_offset_t array_begin, array_end;
} jak_carbon_insert_array_state;

typedef struct jak_carbon_insert_object_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_object_it *it;
        jak_carbon_insert inserter;
        jak_offset_t object_begin, object_end;
} jak_carbon_insert_object_state;

typedef struct jak_carbon_insert_column_state {
        jak_carbon_insert *parent_inserter;
        jak_carbon_field_type_e type;
        jak_carbon_column_it *nested_column;
        jak_carbon_insert nested_inserter;
        jak_offset_t column_begin, column_end;
} jak_carbon_insert_column_state;

bool jak_carbon_int_insert_object(jak_memfile *memfile, carbon_map_derivable_e derivation, size_t nbytes);
bool jak_carbon_int_insert_array(jak_memfile *memfile, carbon_list_derivable_e derivation, size_t nbytes);
bool jak_carbon_int_insert_column(jak_memfile *jak_memfile_in, jak_error *err_in, carbon_list_derivable_e derivation, jak_carbon_column_type_e type, size_t capactity);

/**
 * Returns the number of bytes required to store a field type including its type marker in a byte sequence.
 */
size_t jak_carbon_int_get_type_size_encoded(jak_carbon_field_type_e type);

/**
 * Returns the number of bytes required to store a field value of a particular type exclusing its type marker.
 */
size_t jak_carbon_int_get_type_value_size(jak_carbon_field_type_e type);

bool jak_carbon_int_array_it_next(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);
bool jak_carbon_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);
bool jak_carbon_int_array_it_field_type_read(jak_carbon_array_it *it);
bool jak_carbon_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_array_it *it);

bool jak_carbon_int_object_it_next(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it);
bool jak_carbon_int_object_it_refresh(bool *is_empty_slot, bool *is_object_end, jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_key_access(jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_value_skip(jak_carbon_object_it *it);
bool jak_carbon_int_object_it_prop_skip(jak_carbon_object_it *it);
bool jak_carbon_int_object_skip_contents(bool *is_empty_slot, bool *is_array_end, jak_carbon_object_it *it);
bool jak_carbon_int_field_data_access(jak_memfile *file, jak_error *err, jak_field_access *field_access);

jak_offset_t jak_carbon_int_column_get_payload_off(jak_carbon_column_it *it);
jak_offset_t jak_carbon_int_payload_after_header(jak_carbon *doc);

jak_u64 jak_carbon_int_header_get_commit_hash(jak_carbon *doc);

void jak_carbon_int_history_push(jak_vector ofType(jak_offset_t) *vec, jak_offset_t off);
void jak_carbon_int_history_clear(jak_vector ofType(jak_offset_t) *vec);
jak_offset_t jak_carbon_int_history_pop(jak_vector ofType(jak_offset_t) *vec);
jak_offset_t jak_carbon_int_history_peek(jak_vector ofType(jak_offset_t) *vec);
bool jak_carbon_int_history_has(jak_vector ofType(jak_offset_t) *vec);

bool jak_carbon_int_field_access_create(jak_field_access *field);
bool jak_carbon_int_field_access_clone(jak_field_access *dst, jak_field_access *src);
bool jak_carbon_int_field_access_drop(jak_field_access *field);
bool jak_carbon_int_field_auto_close(jak_field_access *it);
bool jak_carbon_int_field_access_object_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_array_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_column_it_opened(jak_field_access *field);
bool jak_carbon_int_field_access_field_type(jak_carbon_field_type_e *type, jak_field_access *field);
bool jak_carbon_int_field_access_u8_value(jak_u8 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u16_value(jak_u16 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u32_value(jak_u32 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_u64_value(jak_u64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i8_value(jak_i8 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i16_value(jak_i16 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i32_value(jak_i32 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_i64_value(jak_i64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_float_value(bool *is_null_in, float *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_signed_value(bool *is_null_in, jak_i64 *value, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_unsigned_value(bool *is_null_in, jak_u64 *value, jak_field_access *field, jak_error *err);
const char *jak_carbon_int_field_access_jak_string_value(jak_u64 *strlen, jak_field_access *field, jak_error *err);
bool jak_carbon_int_field_access_binary_value(jak_carbon_binary *out, jak_field_access *field, jak_error *err);
jak_carbon_array_it *jak_carbon_int_field_access_array_value(jak_field_access *field, jak_error *err);
jak_carbon_object_it *jak_carbon_int_field_access_object_value(jak_field_access *field, jak_error *err);
jak_carbon_column_it *jak_carbon_int_field_access_column_value(jak_field_access *field, jak_error *err);

void jak_carbon_int_auto_close_nested_array_it(jak_field_access *field);
void jak_carbon_int_auto_close_nested_object_it(jak_field_access *field);
void jak_carbon_int_auto_close_nested_column_it(jak_field_access *field);

bool jak_carbon_int_field_remove(jak_memfile *memfile, jak_error *err, jak_carbon_field_type_e type);

/**
 * For <code>mode</code>, see <code>jak_carbon_create_begin</code>
 */
bool jak_carbon_int_from_json(jak_carbon *doc, const jak_json *data, jak_carbon_key_e key_type, const void *primary_key, int mode);

JAK_END_DECL

#endif
