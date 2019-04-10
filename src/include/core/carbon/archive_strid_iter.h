/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef NG5_STRID_ITER_H
#define NG5_STRID_ITER_H

#include "shared/common.h"
#include "shared/types.h"
#include "archive.h"

NG5_BEGIN_DECL

typedef struct carbon_strid_info
{
    carbon_string_id_t id;
    u32           strlen;
    offset_t       offset;
} carbon_strid_info_t;

typedef struct carbon_strid_iter
{
    FILE *disk_file;
    bool is_open;
    offset_t disk_offset;
    carbon_strid_info_t vector[100000];
} carbon_strid_iter_t;

NG5_EXPORT(bool)
carbon_strid_iter_open(carbon_strid_iter_t *it, struct err *err, struct archive *archive);

NG5_EXPORT(bool)
carbon_strid_iter_next(bool *success, carbon_strid_info_t **info, struct err *err, size_t *info_length, carbon_strid_iter_t *it);

NG5_EXPORT(bool)
carbon_strid_iter_close(carbon_strid_iter_t *it);

NG5_END_DECL

#endif
