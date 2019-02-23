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

#include "carbon-common.h"
#include "carbon-archive.h"
#include "carbon-strid-iter.h"
#include "carbon-string-pred.h"

CARBON_BEGIN_DECL

typedef struct carbon_query
{
    carbon_archive_t    *archive;
    carbon_io_context_t *context;
    carbon_err_t         err;
} carbon_query_t;

CARBON_DEFINE_GET_ERROR_FUNCTION(query, carbon_query_t, query)

CARBON_EXPORT(bool)
carbon_query_create(carbon_query_t *query, carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_query_drop(carbon_query_t *query);

CARBON_EXPORT(bool)
carbon_query_scan_strids(carbon_strid_iter_t *it, carbon_query_t *query);

CARBON_EXPORT(char *)
carbon_query_fetch_string_by_id(carbon_query_t *query, carbon_string_id_t id);

CARBON_EXPORT(char **)
carbon_query_fetch_strings_by_offset(carbon_query_t *query, carbon_off_t *offs, uint32_t *strlens, size_t num_offs);

CARBON_EXPORT(carbon_string_id_t *)
carbon_query_find_ids(size_t *num_found, carbon_query_t *query, const carbon_string_pred_t *pred,
                      void *capture, int64_t limit);

CARBON_END_DECL

#endif
