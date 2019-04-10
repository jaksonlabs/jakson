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

#ifndef NG5_ARCHIVE_H
#define NG5_ARCHIVE_H

#include "shared/common.h"
#include "core/mem/block.h"
#include "core/mem/file.h"
#include "core/pack/pack.h"
#include "json/columndoc.h"
#include "archive_io.h"
#include "archive_int.h"

NG5_BEGIN_DECL

struct archive_query; /* forwarded from 'types-query.h' */

struct archive
{
        struct archive_info info;
        char *diskFilePath;
        struct string_table string_table;
        carbon_archive_record_table_t record_table;
        struct err err;

        struct sid_to_offset *query_index_string_id_to_offset;
        struct string_cache *string_id_cache;

        struct archive_query *default_query;
};

typedef struct
{
        void (*begin_create_from_model)();
        void (*end_create_from_model)();
        void (*begin_create_from_json)();
        void (*end_create_from_json)();
        void (*begin_archive_stream_from_json)();
        void (*end_archive_stream_from_json)();
        void (*begin_write_archive_file_to_disk)();
        void (*end_write_archive_file_to_disk)();
        void (*begin_load_archive)();
        void (*end_load_archive)();
        void (*begin_setup_string_dictionary)();
        void (*end_setup_string_dictionary)();
        void (*begin_parse_json)();
        void (*end_parse_json)();
        void (*begin_test_json)();
        void (*end_test_json)();
        void (*begin_import_json)();
        void (*end_import_json)();
        void (*begin_cleanup)();
        void (*end_cleanup)();
        void (*begin_write_string_table)();
        void (*end_write_string_table)();
        void (*begin_write_record_table)();
        void (*end_write_record_table)();
        void (*skip_string_id_index_baking)();
        void (*begin_string_id_index_baking)();
        void (*end_string_id_index_baking)();
} carbon_archive_callback_t;

NG5_EXPORT(bool) carbon_archive_from_json(struct archive *out, const char *file, struct err *err,
        const char *json_string, carbon_compressor_type_e compressor, carbon_strdic_type_e dictionary,
        size_t num_async_dic_threads, bool read_optimized, bool bake_string_id_index,
        carbon_archive_callback_t *callback);

NG5_EXPORT(bool) carbon_archive_stream_from_json(carbon_memblock_t **stream, struct err *err, const char *json_string,
        carbon_compressor_type_e compressor, carbon_strdic_type_e dictionary, size_t num_async_dic_threads,
        bool read_optimized, bool bake_id_index, carbon_archive_callback_t *callback);

NG5_EXPORT(bool) carbon_archive_from_model(carbon_memblock_t **stream, struct err *err, carbon_columndoc_t *model,
        carbon_compressor_type_e compressor, bool bake_string_id_index, carbon_archive_callback_t *callback);

NG5_EXPORT(bool) carbon_archive_write(FILE *file, const carbon_memblock_t *stream);

NG5_EXPORT(bool) carbon_archive_load(carbon_memblock_t **stream, FILE *file);

NG5_EXPORT(bool) carbon_archive_print(FILE *file, struct err *err, carbon_memblock_t *stream);

NG5_EXPORT(bool) carbon_archive_open(struct archive *out, const char *file_path);

NG5_EXPORT(bool) carbon_archive_get_info(struct archive_info *info, const struct archive *archive);

NG5_DEFINE_GET_ERROR_FUNCTION(archive, struct archive, archive);

NG5_EXPORT(bool) carbon_archive_close(struct archive *archive);

NG5_EXPORT(bool) carbon_archive_drop_indexes(struct archive *archive);

NG5_EXPORT(bool) carbon_archive_query(struct archive_query *query, struct archive *archive);

NG5_EXPORT(bool) carbon_archive_has_query_index_string_id_to_offset(bool *state, struct archive *archive);

NG5_EXPORT(bool) carbon_archive_hash_query_string_id_cache(bool *has_cache, struct archive *archive);

NG5_EXPORT(bool) carbon_archive_drop_query_string_id_cache(struct archive *archive);

NG5_EXPORT(struct string_cache *)carbon_archive_get_query_string_id_cache(struct archive *archive);

NG5_EXPORT(struct archive_query *)carbon_archive_query_default(struct archive *archive);



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
NG5_EXPORT(carbon_io_context_t *)carbon_archive_io_context_create(struct archive *archive);

NG5_END_DECL

#endif