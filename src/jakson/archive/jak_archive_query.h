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

#ifndef JAK_QUERY_H
#define JAK_QUERY_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include <jakson/jak_archive.h>
#include <jakson/archive/jak_archive_strid_it.h>
#include <jakson/archive/jak_archive_pred.h>
#include <jakson/std/hash/jak_hash_table.h>

JAK_BEGIN_DECL

typedef struct jak_archive_query {
        jak_archive *archive;
        jak_archive_io_context *context;
        jak_error err;
} jak_archive_query;

JAK_DEFINE_GET_ERROR_FUNCTION(query, jak_archive_query, query)

bool jak_query_create(jak_archive_query *query, jak_archive *archive);
bool jak_query_drop(jak_archive_query *query);
bool jak_query_scan_strids(jak_strid_iter *it, jak_archive_query *query);
bool jak_query_create_index_jak_string_id_to_offset(struct jak_sid_to_offset **index, jak_archive_query *query);
void jak_query_drop_index_jak_string_id_to_offset(struct jak_sid_to_offset *index);
bool jak_query_index_id_to_offset_serialize(FILE *file, jak_error *err, struct jak_sid_to_offset *index);
bool jak_query_index_id_to_offset_deserialize(struct jak_sid_to_offset **index, jak_error *err, const char *file_path, jak_offset_t offset);
char *jak_query_fetch_jak_string_by_id(jak_archive_query *query, jak_archive_field_sid_t id);
char *jak_query_fetch_jak_string_by_id_nocache(jak_archive_query *query, jak_archive_field_sid_t id);
char **jak_query_fetch_strings_by_offset(jak_archive_query *query, jak_offset_t *offs, jak_u32 *strlens, size_t num_offs);
jak_archive_field_sid_t *jak_query_find_ids(size_t *num_found, jak_archive_query *query, const jak_string_pred *pred, void *capture, jak_i64 limit);

JAK_END_DECL

#endif
