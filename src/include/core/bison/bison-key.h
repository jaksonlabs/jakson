/**
 * Copyright 2019 Marcus Pinnecke
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

#ifndef BISON_KEY_H
#define BISON_KEY_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/bison/bison.h"
#include "core/mem/file.h"

NG5_BEGIN_DECL

NG5_EXPORT(bool) bison_key_create(struct memfile *file, enum bison_primary_key_type type, struct err *err);

NG5_EXPORT(bool) bison_key_skip(enum bison_primary_key_type *out, struct memfile *file);

NG5_EXPORT(bool) bison_key_read_type(enum bison_primary_key_type *out, struct memfile *file);

NG5_EXPORT(bool) bison_key_write_unsigned(struct memfile *file, u64 key);

NG5_EXPORT(bool) bison_key_write_signed(struct memfile *file, i64 key);

NG5_EXPORT(bool) bison_key_write_string(struct memfile *file, const char *key);

NG5_EXPORT(bool) bison_key_update_string(struct memfile *file, const char *key);

NG5_EXPORT(const void *) bison_key_read(u64 *len, enum bison_primary_key_type *out, struct memfile *file);

NG5_END_DECL

#endif
