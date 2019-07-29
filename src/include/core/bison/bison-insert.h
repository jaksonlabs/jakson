/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef BISON_LITERAL_INSERTER_H
#define BISON_LITERAL_INSERTER_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/async/spin.h"
#include "core/bison/bison.h"
#include "core/bison/bison-int.h"

NG5_BEGIN_DECL

struct bison_array_it; /* forwarded from bison-array-it.h */
struct bison_column_it; /* forwarded from bison-column-it.h */

NG5_EXPORT(bool) bison_int_insert_create_for_array(struct bison_insert *inserter, struct bison_array_it *context);

NG5_EXPORT(bool) bison_int_insert_create_for_column(struct bison_insert *inserter, struct bison_column_it *context);

NG5_EXPORT(bool) bison_int_insert_create_for_object(struct bison_insert *inserter, struct bison_object_it *context);

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_true(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_false(struct bison_insert *inserter);

NG5_EXPORT(bool) bison_insert_u8(struct bison_insert *inserter, u8 value);

NG5_EXPORT(bool) bison_insert_u16(struct bison_insert *inserter, u16 value);

NG5_EXPORT(bool) bison_insert_u32(struct bison_insert *inserter, u32 value);

NG5_EXPORT(bool) bison_insert_u64(struct bison_insert *inserter, u64 value);

NG5_EXPORT(bool) bison_insert_i8(struct bison_insert *inserter, i8 value);

NG5_EXPORT(bool) bison_insert_i16(struct bison_insert *inserter, i16 value);

NG5_EXPORT(bool) bison_insert_i32(struct bison_insert *inserter, i32 value);

NG5_EXPORT(bool) bison_insert_i64(struct bison_insert *inserter, i64 value);

NG5_EXPORT(bool) bison_insert_unsigned(struct bison_insert *inserter, u64 value);

NG5_EXPORT(bool) bison_insert_signed(struct bison_insert *inserter, i64 value);

NG5_EXPORT(bool) bison_insert_float(struct bison_insert *inserter, float value);

NG5_EXPORT(bool) bison_insert_string(struct bison_insert *inserter, const char *value);

/**
 * Inserts a user-defined binary string <code>value</code> of <code>nbytes</code> bytes along with a (mime) type annotation.
 * The type annotation is automatically found if <code>file_ext</code> is non-null and known to the system. If it is
 * not known or null, the non-empty <code>user_type</code> string is used to encode the mime annotation. In case
 * <code>user_type</code> is null (or empty) and <code>file_ext</code> is null (or not known), the mime type is set to
 * <code>application/octet-stream</code>, which encodes arbitrary binary data.
 */
NG5_EXPORT(bool) bison_insert_binary(struct bison_insert *inserter, const void *value, size_t nbytes,
        const char *file_ext, const char *user_type);

NG5_EXPORT(struct bison_insert *) bison_insert_object_begin(struct bison_insert_object_state *out,
        struct bison_insert *inserter, u64 object_capacity);

NG5_EXPORT(bool) bison_insert_object_end(struct bison_insert_object_state *state);

NG5_EXPORT(struct bison_insert *) bison_insert_array_begin(struct bison_insert_array_state *state_out,
        struct bison_insert *inserter_in, u64 array_capacity);

NG5_EXPORT(bool) bison_insert_array_end(struct bison_insert_array_state *state_in);

NG5_EXPORT(struct bison_insert *) bison_insert_column_begin(struct bison_insert_column_state *state_out,
        struct bison_insert *inserter_in, enum bison_field_type type, u64 column_capacity);

NG5_EXPORT(bool) bison_insert_column_end(struct bison_insert_column_state *state_in);

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter);

NG5_END_DECL

#endif
