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

#ifndef JAK_ARCHIVE_H
#define JAK_ARCHIVE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_memblock.h>
#include <jak_memfile.h>
#include <jak_pack.h>
#include <jak_column_doc.h>
#include <jak_archive_io.h>
#include <jak_archive_int.h>

JAK_BEGIN_DECL

typedef struct jak_archive {
        jak_archive_info info;
        char *disk_file_path;
        jak_string_table string_table;
        jak_record_table record_table;
        jak_error err;
        struct jak_sid_to_offset *query_index_string_id_to_offset;
        struct jak_string_cache *string_id_cache;
        jak_archive_query *default_query;
} jak_archive;

typedef struct jak_archive_callback {
        void (*begin_create_from_model)();
        void (*end_create_from_model)();
        void (*begin_create_from_json)();
        void (*end_create_from_json)();
        void (*begin_jak_archive_stream_from_json)();
        void (*end_jak_archive_stream_from_json)();
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
} jak_archive_callback;

JAK_DEFINE_GET_ERROR_FUNCTION(archive, jak_archive, archive);

bool jak_archive_from_json(jak_archive *out, const char *file, jak_error *err, const char *json_string, jak_packer_e compressor, enum jak_str_dict_tag dictionary, size_t num_async_dic_threads, bool read_optimized, bool bake_string_id_index, jak_archive_callback *callback);
bool jak_archive_stream_from_json(jak_memblock **stream, jak_error *err, const char *json_string, jak_packer_e compressor, enum jak_str_dict_tag dictionary, size_t num_async_dic_threads, bool read_optimized, bool bake_id_index, jak_archive_callback *callback);
bool jak_archive_from_model(jak_memblock **stream, jak_error *err, jak_column_doc *model, jak_packer_e compressor, bool bake_string_id_index, jak_archive_callback *callback);
bool jak_archive_write(FILE *file, const jak_memblock *stream);
bool jak_archive_load(jak_memblock **stream, FILE *file);
bool jak_archive_print(FILE *file, jak_error *err, jak_memblock *stream);
bool jak_archive_open(jak_archive *out, const char *file_path);
bool jak_archive_get_info(jak_archive_info *info, const jak_archive *archive);
bool jak_archive_close(jak_archive *archive);
bool jak_archive_drop_indexes(jak_archive *archive);
bool jak_archive_query_run(jak_archive_query *query, jak_archive *archive);
bool jak_archive_has_query_index_string_id_to_offset(bool *state, jak_archive *archive);
bool jak_archive_hash_query_string_id_cache(bool *has_cache, jak_archive *archive);
bool jak_archive_drop_query_string_id_cache(jak_archive *archive);
struct jak_string_cache *jak_archive_get_query_string_id_cache(jak_archive *archive);
jak_archive_query *jak_archive_query_default(jak_archive *archive);

/**
 * Creates a new <code>jak_archive_io_context</code> to access the archives underlying file for unsafe operations.
 *
 * An unsafe operation directly seeks randomly in the underlying file. To avoid creation of multiple file
 * descriptors while at the same time allow to access unsafe operations in a multi-threading environment, an
 * <code>jak_archive_io_context</code> is used. Roughly, such a context is a regular FILE that is protected by a lock.
 *
 * @param archive The archive
 * @return a heap-allocated instance of <code>jak_archive_io_context</code>, or NULL if not successful
 */
jak_archive_io_context *jak_archive_io_context_create(jak_archive *archive);

JAK_END_DECL

#endif