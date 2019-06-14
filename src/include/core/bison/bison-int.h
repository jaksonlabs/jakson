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

NG5_BEGIN_DECL

struct bison_insert; /* forwarded from 'bison-insert.h' */

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes);

NG5_EXPORT(bool) bison_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum bison_field_type type, size_t capactity);

NG5_EXPORT(bool) bison_int_ensure_space(struct memfile *memfile, u64 nbytes);

/**
 * Returns the number of bytes required to store a field type including its type marker in a byte sequence.
 */
NG5_EXPORT(size_t) bison_int_get_type_size_encoded(enum bison_field_type type);

/**
 * Returns the number of bytes required to store a field value of a particular type exclusing its type marker.
 */
NG5_EXPORT(size_t) bison_int_get_type_value_size(enum bison_field_type type);

NG5_END_DECL

#endif
