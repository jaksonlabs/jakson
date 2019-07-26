/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef BISON_INT_H
#define BISON_INT_H

#include "shared/common.h"
#include "core/mem/file.h"
#include "stdx/varuint.h"
#include "core/bison/bison-int.h"
#include "core/bison/bison-field.h"

NG5_BEGIN_DECL

struct bison_insert
{
        enum bison_container_type context_type;
        union {
                struct bison_array_it *array;
                struct bison_column_it *column;
        } context;

        struct memfile memfile;
        offset_t position;
        struct err err;
};

struct bison_insert_array_state
{
        struct bison_insert *parent_inserter;

        struct bison_array_it *nested_array;
        struct bison_insert nested_inserter;
};

struct bison_insert_column_state
{
        struct bison_insert *parent_inserter;

        enum bison_field_type type;
        struct bison_column_it *nested_column;
        struct bison_insert nested_inserter;

};

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes);

NG5_EXPORT(bool) bison_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum bison_field_type type, size_t capactity);

/**
 * Returns the number of bytes required to store a field type including its type marker in a byte sequence.
 */
NG5_EXPORT(size_t) bison_int_get_type_size_encoded(enum bison_field_type type);

/**
 * Returns the number of bytes required to store a field value of a particular type exclusing its type marker.
 */
NG5_EXPORT(size_t) bison_int_get_type_value_size(enum bison_field_type type);

NG5_EXPORT(bool) bison_int_array_it_next(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_skip_contents(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_refresh(bool *is_empty_slot, bool *is_array_end, struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_field_type_read(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_field_data_access(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_field_skip(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_array(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_column(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_binary(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_custom_binary(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_string(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_float(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_8(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_16(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_32(struct bison_array_it *it);

NG5_EXPORT(bool) bison_int_array_it_skip_64(struct bison_array_it *it);

NG5_EXPORT(offset_t) bison_int_column_get_payload_off(struct bison_column_it *it);

NG5_EXPORT(offset_t) bison_int_payload_after_header(struct bison *doc);

NG5_EXPORT(u64) bison_int_header_get_rev(struct bison *doc);


NG5_END_DECL

#endif
