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

#ifndef JAK_STRID_ITER_H
#define JAK_STRID_ITER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include <jakson/jak_types.h>
#include <jakson/jak_archive.h>

JAK_BEGIN_DECL

typedef struct jak_strid_info {
        jak_archive_field_sid_t id;
        jak_u32 strlen;
        jak_offset_t offset;
} jak_strid_info;

typedef struct jak_strid_iter {
        FILE *disk_file;
        bool is_open;
        jak_offset_t disk_offset;
        jak_strid_info vector[100000];
} jak_strid_iter;

bool jak_strid_iter_open(jak_strid_iter *it, jak_error *err, jak_archive *archive);
bool jak_strid_iter_next(bool *success, jak_strid_info **info, jak_error *err, size_t *info_length, jak_strid_iter *it);
bool jak_strid_iter_close(jak_strid_iter *it);

JAK_END_DECL

#endif
