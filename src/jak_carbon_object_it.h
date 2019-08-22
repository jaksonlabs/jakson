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

#ifndef JAK_CARBON_OBJECT_IT_H
#define JAK_CARBON_OBJECT_IT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_memfile.h>
#include <jak_carbon_field.h>
#include <jak_carbon_array_it.h>

JAK_BEGIN_DECL

struct jak_carbon_object_it
{
    struct memfile memfile;
    struct err err;

    offset_t object_contents_off;
    bool object_end_reached;

    struct vector ofType(offset_t) history;

    struct {
        struct {
            offset_t offset;
            const char *name;
            u64 name_len;
        } key;
        struct {
            offset_t offset;
            struct field_access data;
        } value;
    } field;

    struct spinlock lock;
    /* in case of modifications (updates, inserts, deletes), the number of bytes that are added resp. removed */
    i64 mod_size;
};

bool carbon_object_it_create(struct jak_carbon_object_it *it, struct memfile *memfile, struct err *err,
                             offset_t payload_start);

bool carbon_object_it_copy(struct jak_carbon_object_it *dst, struct jak_carbon_object_it *src);

bool carbon_object_it_clone(struct jak_carbon_object_it *dst, struct jak_carbon_object_it *src);

bool carbon_object_it_drop(struct jak_carbon_object_it *it);

bool carbon_object_it_rewind(struct jak_carbon_object_it *it);

bool carbon_object_it_next(struct jak_carbon_object_it *it);

bool carbon_object_it_has_next(struct jak_carbon_object_it *it);

bool carbon_object_it_prev(struct jak_carbon_object_it *it);

offset_t carbon_object_it_memfile_pos(struct jak_carbon_object_it *it);

bool carbon_object_it_tell(offset_t *key_off, offset_t *value_off, struct jak_carbon_object_it *it);

const char *carbon_object_it_prop_name(u64 *key_len, struct jak_carbon_object_it *it);

bool carbon_object_it_remove(struct jak_carbon_object_it *it);

bool carbon_object_it_prop_type(enum carbon_field_type *type, struct jak_carbon_object_it *it);

bool carbon_object_it_insert_begin(struct jak_carbon_insert *inserter, struct jak_carbon_object_it *it);

bool carbon_object_it_insert_end(struct jak_carbon_insert *inserter);

bool carbon_object_it_lock(struct jak_carbon_object_it *it);

bool carbon_object_it_unlock(struct jak_carbon_object_it *it);

bool carbon_object_it_fast_forward(struct jak_carbon_object_it *it);

bool carbon_object_it_u8_value(u8 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_u16_value(u16 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_u32_value(u32 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_u64_value(u64 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_i8_value(i8 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_i16_value(i16 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_i32_value(i32 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_i64_value(i64 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_float_value(bool *is_null_in, float *value, struct jak_carbon_object_it *it);

bool carbon_object_it_signed_value(bool *is_null_in, i64 *value, struct jak_carbon_object_it *it);

bool carbon_object_it_unsigned_value(bool *is_null_in, u64 *value, struct jak_carbon_object_it *it);

const char *carbon_object_it_string_value(u64 *strlen, struct jak_carbon_object_it *it);

bool carbon_object_it_binary_value(struct jak_carbon_binary *out, struct jak_carbon_object_it *it);

struct jak_carbon_array_it *carbon_object_it_array_value(struct jak_carbon_object_it *it_in);

struct jak_carbon_object_it *carbon_object_it_object_value(struct jak_carbon_object_it *it_in);

struct jak_carbon_column_it *carbon_object_it_column_value(struct jak_carbon_object_it *it_in);


JAK_END_DECL

#endif
