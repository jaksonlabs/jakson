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

#ifndef QUERY_H
#define QUERY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/archive.h>
#include <jakson/archive/strid_it.h>
#include <jakson/archive/pred.h>
#include <jakson/std/hash/table.h>

BEGIN_DECL

typedef struct query {
        archive *archive;
        archive_io_context *context;
        err err;
} archive_query;

bool query_create(query *query, archive *archive);
bool query_drop(query *query);
bool query_scan_strids(strid_iter *it, query *query);
bool query_create_index_string_id_to_offset(struct sid_to_offset **index, query *query);
void query_drop_index_string_id_to_offset(struct sid_to_offset *index);
bool query_index_id_to_offset_serialize(FILE *file, err *err, struct sid_to_offset *index);
bool query_index_id_to_offset_deserialize(struct sid_to_offset **index, err *err, const char *file_path, offset_t offset);
char *query_fetch_string_by_id(query *query, archive_field_sid_t id);
char *query_fetch_string_by_id_nocache(query *query, archive_field_sid_t id);
char **query_fetch_strings_by_offset(query *query, offset_t *offs, u32 *strlens, size_t num_offs);
archive_field_sid_t *query_find_ids(size_t *num_found, query *query, const string_pred *pred, void *capture, i64 limit);

END_DECL

#endif
