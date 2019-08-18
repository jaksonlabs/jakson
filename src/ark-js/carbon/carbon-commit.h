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

#ifndef CARBON_COMMIT_HASH_H
#define CARBON_COMMIT_HASH_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/shared/stdx/string.h>

ARK_BEGIN_DECL

bool carbon_commit_hash_create(struct memfile *file);

bool carbon_commit_hash_skip(struct memfile *file);

bool carbon_commit_hash_read(u64 *commit_hash, struct memfile *file);

bool carbon_commit_hash_peek(u64 *commit_hash, struct memfile *file);

bool carbon_commit_hash_update(struct memfile *file, const char *base, u64 len);

bool carbon_commit_hash_compute(u64 *commit_hash, const void *base, u64 len);

const char *carbon_commit_hash_to_str(struct string *dst, u64 commit_hash);

bool carbon_commit_hash_append_to_str(struct string *dst, u64 commit_hash);

u64 carbon_commit_hash_from_str(const char *commit_str, struct err *err);

ARK_END_DECL

#endif
