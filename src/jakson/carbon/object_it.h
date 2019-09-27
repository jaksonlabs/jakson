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

#ifndef CARBON_OBJECT_IT_H
#define CARBON_OBJECT_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/mem/file.h>
#include <jakson/carbon/field.h>
#include <jakson/carbon/array_it.h>

BEGIN_DECL

typedef struct carbon_object_it {
        memfile memfile;
        err err;

        offset_t object_contents_off;
        bool object_end_reached;

        vector ofType(offset_t) history;

        struct {
                struct {
                        offset_t offset;
                        const char *name;
                        u64 name_len;
                } key;
                struct {
                        offset_t offset;
                        field_access data;
                } value;
        } field;

        spinlock lock;
        /** in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
        i64 mod_size;
} carbon_object_it;

bool carbon_object_it_create(carbon_object_it *it, memfile *memfile, err *err, offset_t payload_start);
bool carbon_object_it_copy(carbon_object_it *dst, carbon_object_it *src);
bool carbon_object_it_clone(carbon_object_it *dst, carbon_object_it *src);
bool carbon_object_it_drop(carbon_object_it *it);

bool carbon_object_it_rewind(carbon_object_it *it);
bool carbon_object_it_next(carbon_object_it *it);
bool carbon_object_it_has_next(carbon_object_it *it);
bool carbon_object_it_fast_forward(carbon_object_it *it);
bool carbon_object_it_prev(carbon_object_it *it);

offset_t carbon_object_it_memfile_pos(carbon_object_it *it);
bool carbon_object_it_tell(offset_t *key_off, offset_t *value_off, carbon_object_it *it);

const char *carbon_object_it_prop_name(u64 *key_len, carbon_object_it *it);
bool carbon_object_it_remove(carbon_object_it *it);
bool carbon_object_it_prop_type(carbon_field_type_e *type, carbon_object_it *it);

bool carbon_object_it_insert_begin(carbon_insert *inserter, carbon_object_it *it);
bool carbon_object_it_insert_end(carbon_insert *inserter);

bool carbon_object_it_lock(carbon_object_it *it);
bool carbon_object_it_unlock(carbon_object_it *it);

bool carbon_object_it_u8_value(u8 *value, carbon_object_it *it);
bool carbon_object_it_u16_value(u16 *value, carbon_object_it *it);
bool carbon_object_it_u32_value(u32 *value, carbon_object_it *it);
bool carbon_object_it_u64_value(u64 *value, carbon_object_it *it);
bool carbon_object_it_i8_value(i8 *value, carbon_object_it *it);
bool carbon_object_it_i16_value(i16 *value, carbon_object_it *it);
bool carbon_object_it_i32_value(i32 *value, carbon_object_it *it);
bool carbon_object_it_i64_value(i64 *value, carbon_object_it *it);
bool carbon_object_it_float_value(bool *is_null_in, float *value, carbon_object_it *it);
bool carbon_object_it_signed_value(bool *is_null_in, i64 *value, carbon_object_it *it);
bool carbon_object_it_unsigned_value(bool *is_null_in, u64 *value, carbon_object_it *it);
const char *carbon_object_it_string_value(u64 *strlen, carbon_object_it *it);
bool carbon_object_it_binary_value(carbon_binary *out, carbon_object_it *it);
carbon_array_it *carbon_object_it_array_value(carbon_object_it *it_in);
carbon_object_it *carbon_object_it_object_value(carbon_object_it *it_in);
carbon_column_it *carbon_object_it_column_value(carbon_object_it *it_in);

END_DECL

#endif
