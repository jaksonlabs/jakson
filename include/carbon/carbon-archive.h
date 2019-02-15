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

#ifndef CARBON_ARCHIVE_H
#define CARBON_ARCHIVE_H

#include "carbon-common.h"
#include "carbon-memblock.h"
#include "carbon-memfile.h"
#include "carbon-compressor.h"
#include "carbon-columndoc.h"
#include "carbon-io-context.h"
#include "carbon-int-archive.h"

CARBON_BEGIN_DECL

typedef struct carbon_query carbon_query_t; /* forwarded from 'carbon-query.h' */

typedef union carbon_archive_dic_flags carbon_archive_dic_flags_t;

typedef struct carbon_archive
{
    carbon_archive_info_t         info;
    char                         *diskFilePath;
    carbon_archive_string_table_t string_table;
    carbon_archive_record_table_t record_table;
    carbon_err_t                  err;
} carbon_archive_t;


CARBON_EXPORT(bool)
carbon_archive_from_json(carbon_archive_t *out,
                         const char *file,
                         carbon_err_t *err,
                         const char *json_string,
                         carbon_compressor_type_e compressor,
                         bool read_optimized);

CARBON_EXPORT(bool)
carbon_archive_stream_from_json(carbon_memblock_t **stream,
                                carbon_err_t *err,
                                const char *json_string,
                                carbon_compressor_type_e compressor,
                                bool read_optimized);

CARBON_EXPORT(bool)
carbon_archive_from_model(carbon_memblock_t **stream,
                          carbon_err_t *err,
                          carbon_columndoc_t *model,
                          carbon_compressor_type_e compressor);


CARBON_EXPORT(bool)
carbon_archive_write(FILE *file, const carbon_memblock_t *stream);

CARBON_EXPORT(bool)
carbon_archive_load(carbon_memblock_t **stream, FILE *file);

CARBON_EXPORT(bool)
carbon_archive_print(FILE *file, carbon_err_t *err, carbon_memblock_t *stream);

CARBON_EXPORT(bool)
carbon_archive_open(carbon_archive_t *out, const char *file_path);

CARBON_EXPORT(bool)
carbon_archive_get_info(carbon_archive_info_t *info, const struct carbon_archive *archive);

CARBON_DEFINE_GET_ERROR_FUNCTION(archive, carbon_archive_t, archive);

CARBON_EXPORT(bool)
carbon_archive_close(carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_archive_query(carbon_query_t *query, carbon_archive_t *archive);



/**
 * Creates a new <code>carbon_io_context_t</code> to access the archives underlying file for unsafe operations.
 *
 * An unsafe operation directly seeks randomly in the underlying file. To avoid creation of multiple file
 * descriptors while at the same time allow to access unsafe operations in a multi-threading environment, an
 * <code>carbon_io_context_t</code> is used. Roughly, such a context is a regular FILE that is protected by a lock.
 *
 * @param archive The archive
 * @return a heap-allocated instance of <code>carbon_io_context_t</code>, or NULL if not successful
 */
CARBON_EXPORT(carbon_io_context_t *)
carbon_archive_io_context_create(carbon_archive_t *archive);



CARBON_END_DECL

#endif