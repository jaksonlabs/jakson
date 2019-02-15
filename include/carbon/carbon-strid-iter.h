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

#ifndef CARBON_STRID_ITER_H
#define CARBON_STRID_ITER_H

#include "carbon-common.h"
#include "carbon-types.h"
#include "carbon-archive.h"

CARBON_BEGIN_DECL

typedef struct carbon_strid_info
{
    carbon_string_id_t id;
    uint32_t           strlen;
    carbon_off_t       offset;
} carbon_strid_info_t;

typedef struct carbon_strid_iter
{
    FILE *disk_file;
    bool is_open;
    carbon_off_t disk_offset;
    carbon_strid_info_t vector[10000];
} carbon_strid_iter_t;

CARBON_EXPORT(bool)
carbon_strid_iter_open(carbon_strid_iter_t *it, carbon_err_t *err, carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_strid_iter_next(bool *success, carbon_strid_info_t **info, carbon_err_t *err, size_t *info_length, carbon_strid_iter_t *it);

CARBON_EXPORT(bool)
carbon_strid_iter_close(carbon_strid_iter_t *it);

CARBON_END_DECL

#endif
