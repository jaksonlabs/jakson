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

#ifndef CARBON_QUERY_H
#define CARBON_QUERY_H

#include "shared/common.h"
#include "archive.h"
#include "strid_iter.h"
#include "string-pred.h"
#include "stdx/hash_table.h"

CARBON_BEGIN_DECL

typedef struct carbon_query
{
    carbon_archive_t    *archive;
    carbon_io_context_t *context;
    struct err         err;
} carbon_query_t;

typedef struct carbon_query_index_id_to_offset carbon_query_index_id_to_offset_t;

CARBON_DEFINE_GET_ERROR_FUNCTION(query, carbon_query_t, query)

CARBON_EXPORT(bool)
carbon_query_create(carbon_query_t *query, carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_query_drop(carbon_query_t *query);

CARBON_EXPORT(bool)
carbon_query_scan_strids(carbon_strid_iter_t *it, carbon_query_t *query);

CARBON_EXPORT(bool)
carbon_query_create_index_string_id_to_offset(carbon_query_index_id_to_offset_t **index,
                                              carbon_query_t *query);

CARBON_EXPORT(void)
carbon_query_drop_index_string_id_to_offset(carbon_query_index_id_to_offset_t *index);

CARBON_EXPORT(bool)
carbon_query_index_id_to_offset_serialize(FILE *file, struct err *err, carbon_query_index_id_to_offset_t *index);

CARBON_EXPORT(bool)
carbon_query_index_id_to_offset_deserialize(carbon_query_index_id_to_offset_t **index, struct err *err, const char *file_path, offset_t offset);

CARBON_EXPORT(char *)
carbon_query_fetch_string_by_id(carbon_query_t *query, carbon_string_id_t id);

CARBON_EXPORT(char *)
carbon_query_fetch_string_by_id_nocache(carbon_query_t *query, carbon_string_id_t id);

CARBON_EXPORT(char **)
carbon_query_fetch_strings_by_offset(carbon_query_t *query, offset_t *offs, u32 *strlens, size_t num_offs);

CARBON_EXPORT(carbon_string_id_t *)
carbon_query_find_ids(size_t *num_found, carbon_query_t *query, const carbon_string_pred_t *pred,
                      void *capture, i64 limit);

CARBON_END_DECL

#endif
