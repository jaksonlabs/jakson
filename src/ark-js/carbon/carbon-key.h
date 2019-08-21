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

#ifndef CARBON_KEY_H
#define CARBON_KEY_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/carbon/carbon.h>
#include <ark-js/shared/mem/file.h>

ARK_BEGIN_DECL

bool carbon_key_create(struct memfile *file, enum carbon_key_type type, struct err *err);

bool carbon_key_skip(enum carbon_key_type *out, struct memfile *file);

bool carbon_key_read_type(enum carbon_key_type *out, struct memfile *file);

bool carbon_key_write_unsigned(struct memfile *file, u64 key);

bool carbon_key_write_signed(struct memfile *file, i64 key);

bool carbon_key_write_string(struct memfile *file, const char *key);

bool carbon_key_update_string(struct memfile *file, const char *key);

bool carbon_key_update_string_wnchar(struct memfile *file, const char *key, size_t length);

const void *carbon_key_read(u64 *len, enum carbon_key_type *out, struct memfile *file);

ARK_END_DECL

#endif
