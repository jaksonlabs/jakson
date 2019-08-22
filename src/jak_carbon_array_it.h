/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in carbon
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

#ifndef JAK_CARBON_ARRAY_IT_H
#define JAK_CARBON_ARRAY_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_vector.h>
#include <jak_spinlock.h>
#include <jak_carbon.h>
#include <jak_carbon_field.h>

JAK_BEGIN_DECL

struct jak_carbon; /* forwarded from carbon.h */
struct jak_carbon_insert; /* forwarded from carbon-literal-inserter.h */
struct jak_carbon_column_it; /* forwarded from carbon-column-it.h */

struct field_access {
    enum carbon_field_type it_field_type;

    const void *it_field_data;
    jak_u64 it_field_len;

    const char *it_mime_type;
    jak_u64 it_mime_type_strlen;

    bool nested_array_it_is_created;
    bool nested_array_it_accessed;

    bool nested_object_it_is_created;
    bool nested_object_it_accessed;

    bool nested_column_it_is_created;

    struct jak_carbon_array_it *nested_array_it;
    struct jak_carbon_column_it *nested_column_it;
    struct jak_carbon_object_it *nested_object_it;
};

struct jak_carbon_array_it {
    struct jak_memfile memfile;
    jak_offset_t payload_start;
    struct spinlock lock;
    struct jak_error err;

    /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
    jak_i64 mod_size;
    bool array_end_reached;

    struct vector ofType(jak_offset_t) history;
    struct field_access field_access;
    jak_offset_t field_offset;
};

JAK_DEFINE_ERROR_GETTER(jak_carbon_array_it);

#define DECLARE_IN_PLACE_UPDATE_FUNCTION(type_name)                                                                    \
bool carbon_array_it_update_in_place_##type_name(struct jak_carbon_array_it *it, jak_##type_name value);

DECLARE_IN_PLACE_UPDATE_FUNCTION(u8)

DECLARE_IN_PLACE_UPDATE_FUNCTION(u16)

DECLARE_IN_PLACE_UPDATE_FUNCTION(u32)

DECLARE_IN_PLACE_UPDATE_FUNCTION(u64)

DECLARE_IN_PLACE_UPDATE_FUNCTION(i8)

DECLARE_IN_PLACE_UPDATE_FUNCTION(i16)

DECLARE_IN_PLACE_UPDATE_FUNCTION(i32)

DECLARE_IN_PLACE_UPDATE_FUNCTION(i64)

DECLARE_IN_PLACE_UPDATE_FUNCTION(float)

bool carbon_array_it_update_in_place_true(struct jak_carbon_array_it *it);

bool carbon_array_it_update_in_place_false(struct jak_carbon_array_it *it);

bool carbon_array_it_update_in_place_null(struct jak_carbon_array_it *it);

/**
 * Constructs a new array iterator in a carbon document, where <code>payload_start</code> is a memory offset
 * that starts with the first (potentially empty) array entry. If there is some data before the array contents
 * (e.g., a header), <code>payload_start</code> must not include this data.
 */
bool carbon_array_it_create(struct jak_carbon_array_it *it, struct jak_memfile *memfile, struct jak_error *err,
                            jak_offset_t payload_start);

bool carbon_array_it_copy(struct jak_carbon_array_it *dst, struct jak_carbon_array_it *src);

bool carbon_array_it_clone(struct jak_carbon_array_it *dst, struct jak_carbon_array_it *src);

bool carbon_array_it_readonly(struct jak_carbon_array_it *it);

bool carbon_array_it_length(jak_u64 *len, struct jak_carbon_array_it *it);

bool carbon_array_it_is_empty(struct jak_carbon_array_it *it);

/**
 * Drops the iterator.
 */
bool carbon_array_it_drop(struct jak_carbon_array_it *it);

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_array_it_unlock</code> is required for unlocking.
 */
bool carbon_array_it_lock(struct jak_carbon_array_it *it);

/**
 * Unlocks the iterator
 */
bool carbon_array_it_unlock(struct jak_carbon_array_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
bool carbon_array_it_rewind(struct jak_carbon_array_it *it);

/**
 * Positions the iterator to the slot after the current element, potentially pointing to next element.
 * The function returns true, if the slot is non-empty, and false otherwise.
 */
bool carbon_array_it_next(struct jak_carbon_array_it *it);

bool carbon_array_it_has_next(struct jak_carbon_array_it *it);

bool carbon_array_it_is_unit(struct jak_carbon_array_it *it);

bool carbon_array_it_prev(struct jak_carbon_array_it *it);

jak_offset_t carbon_array_it_memfilepos(struct jak_carbon_array_it *it);

jak_offset_t carbon_array_it_tell(struct jak_carbon_array_it *it);

bool carbon_int_array_it_offset(jak_offset_t *off, struct jak_carbon_array_it *it);

bool carbon_array_it_fast_forward(struct jak_carbon_array_it *it);

bool carbon_array_it_field_type(enum carbon_field_type *type, struct jak_carbon_array_it *it);

bool carbon_array_it_u8_value(jak_u8 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_u16_value(jak_u16 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_u32_value(jak_u32 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_u64_value(jak_u64 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_i8_value(jak_i8 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_i16_value(jak_i16 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_i32_value(jak_i32 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_i64_value(jak_i64 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_float_value(bool *is_null_in, float *value, struct jak_carbon_array_it *it);

bool carbon_array_it_signed_value(bool *is_null_in, jak_i64 *value, struct jak_carbon_array_it *it);

bool carbon_array_it_unsigned_value(bool *is_null_in, jak_u64 *value, struct jak_carbon_array_it *it);

const char *carbon_array_it_string_value(jak_u64 *strlen, struct jak_carbon_array_it *it);

bool carbon_array_it_binary_value(struct jak_carbon_binary *out, struct jak_carbon_array_it *it);

struct jak_carbon_array_it *carbon_array_it_array_value(struct jak_carbon_array_it *it_in);

struct jak_carbon_object_it *carbon_array_it_object_value(struct jak_carbon_array_it *it_in);

struct jak_carbon_column_it *carbon_array_it_column_value(struct jak_carbon_array_it *it_in);

/**
 * Inserts a new element at the current position of the iterator.
 */
bool carbon_array_it_insert_begin(struct jak_carbon_insert *inserter, struct jak_carbon_array_it *it);

bool carbon_array_it_insert_end(struct jak_carbon_insert *inserter);

bool carbon_array_it_remove(struct jak_carbon_array_it *it);


//bool carbon_array_it_update_in_place_u8(struct jak_carbon_array_it *it, jak_u8 value);

JAK_END_DECL

#endif
