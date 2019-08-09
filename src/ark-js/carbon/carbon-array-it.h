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

#ifndef CARBON_ARRAY_IT_H
#define CARBON_ARRAY_IT_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-field.h>

ARK_BEGIN_DECL

struct carbon; /* forwarded from carbon.h */
struct carbon_insert; /* forwarded from carbon-literal-inserter.h */
struct carbon_column_it; /* forwarded from carbon-column-it.h */

struct field_access
{
        enum carbon_field_type it_field_type;

        const void *it_field_data;
        u64 it_field_len;

        const char *it_mime_type;
        u64 it_mime_type_strlen;

        bool nested_array_it_is_created;
        bool nested_array_it_accessed;

        bool nested_object_it_is_created;
        bool nested_object_it_accessed;

        bool nested_column_it_is_created;

        struct carbon_array_it *nested_array_it;
        struct carbon_column_it *nested_column_it;
        struct carbon_object_it *nested_object_it;
};

struct carbon_array_it
{
        struct memfile memfile;
        offset_t payload_start;
        struct spinlock lock;
        struct err err;

        /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        i64 mod_size;
        bool array_end_reached;

        struct vector ofType(offset_t) history;
        struct field_access field_access;
};

ARK_DEFINE_ERROR_GETTER(carbon_array_it);

#define DECLARE_IN_PLACE_UPDATE_FUNCTION(type_name)                                                                    \
ARK_EXPORT(bool) carbon_array_it_update_in_place_##type_name(struct carbon_array_it *it, type_name value);

DECLARE_IN_PLACE_UPDATE_FUNCTION(u8)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u16)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u32)
DECLARE_IN_PLACE_UPDATE_FUNCTION(u64)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i8)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i16)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i32)
DECLARE_IN_PLACE_UPDATE_FUNCTION(i64)
DECLARE_IN_PLACE_UPDATE_FUNCTION(float)

ARK_EXPORT(bool) carbon_array_it_update_in_place_true(struct carbon_array_it *it);
ARK_EXPORT(bool) carbon_array_it_update_in_place_false(struct carbon_array_it *it);
ARK_EXPORT(bool) carbon_array_it_update_in_place_null(struct carbon_array_it *it);

/**
 * Constructs a new array iterator in a carbon document, where <code>payload_start</code> is a memory offset
 * that starts with the first (potentially empty) array entry. If there is some data before the array contents
 * (e.g., a header), <code>payload_start</code> must not include this data.
 */
ARK_EXPORT(bool) carbon_array_it_create(struct carbon_array_it *it, struct memfile *memfile, struct err *err,
        offset_t payload_start);

ARK_EXPORT(bool) carbon_array_it_copy(struct carbon_array_it *dst, struct carbon_array_it *src);

ARK_EXPORT(bool) carbon_array_it_clone(struct carbon_array_it *dst, struct carbon_array_it *src);

ARK_EXPORT(bool) carbon_array_it_readonly(struct carbon_array_it *it);

/**
 * Drops the iterator.
 */
ARK_EXPORT(bool) carbon_array_it_drop(struct carbon_array_it *it);

/**
 * Locks the iterator with a spinlock. A call to <code>carbon_array_it_unlock</code> is required for unlocking.
 */
ARK_EXPORT(bool) carbon_array_it_lock(struct carbon_array_it *it);

/**
 * Unlocks the iterator
 */
ARK_EXPORT(bool) carbon_array_it_unlock(struct carbon_array_it *it);

/**
 * Positions the iterator at the beginning of this array.
 */
ARK_EXPORT(bool) carbon_array_it_rewind(struct carbon_array_it *it);

/**
 * Positions the iterator to the slot after the current element, potentially pointing to next element.
 * The function returns true, if the slot is non-empty, and false otherwise.
 */
ARK_EXPORT(bool) carbon_array_it_next(struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_prev(struct carbon_array_it *it);

ARK_EXPORT(offset_t) carbon_array_it_tell(struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_int_array_it_offset(offset_t *off, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_fast_forward(struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_field_type(enum carbon_field_type *type, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_u8_value(u8 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_u16_value(u16 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_u32_value(u32 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_u64_value(u64 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_i8_value(i8 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_i16_value(i16 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_i32_value(i32 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_i64_value(i64 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_float_value(bool *is_null_in, float *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_signed_value(bool *is_null_in, i64 *value, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_unsigned_value(bool *is_null_in, u64 *value, struct carbon_array_it *it);

ARK_EXPORT(const char *) carbon_array_it_string_value(u64 *strlen, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_binary_value(struct carbon_binary *out, struct carbon_array_it *it);

ARK_EXPORT(struct carbon_array_it *) carbon_array_it_array_value(struct carbon_array_it *it_in);

ARK_EXPORT(struct carbon_object_it *) carbon_array_it_object_value(struct carbon_array_it *it_in);

ARK_EXPORT(struct carbon_column_it *) carbon_array_it_column_value(struct carbon_array_it *it_in);

/**
 * Inserts a new element at the current position of the iterator.
 */
ARK_EXPORT(bool) carbon_array_it_insert_begin(struct carbon_insert *inserter, struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_array_it_insert_end(struct carbon_insert *inserter);

ARK_EXPORT(bool) carbon_array_it_remove(struct carbon_array_it *it);


//ARK_EXPORT(bool) carbon_array_it_update_in_place_u8(struct carbon_array_it *it, u8 value);

ARK_END_DECL

#endif
