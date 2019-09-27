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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <inttypes.h>

#include <jakson/stdx/jak_unique_id.h>
#include <jakson/archive/jak_encode_async.h>
#include <jakson/archive/jak_pack.h>
#include <jakson/archive/jak_archive_strid_it.h>
#include <jakson/archive/jak_archive_int.h>
#include <jakson/archive/jak_archive_query.h>
#include <jakson/archive/jak_archive_cache.h>
#include <jakson/jak_archive.h>
#include <jakson/archive/jak_encode_sync.h>
#include <jakson/jak_stdinc.h>
#include <jakson/memfile/jak_memblock.h>
#include <jakson/memfile/jak_memfile.h>
#include <jakson/archive/jak_huffman.h>
#include <jakson/jak_archive.h>

#define WRITE_PRIMITIVE_VALUES(memfile, values_vec, type)                                                              \
{                                                                                                                      \
    type *values = JAK_VECTOR_ALL(values_vec, type);                                                                \
    jak_memfile_write(memfile, values, values_vec->num_elems * sizeof(type));                                       \
}

#define WRITE_ARRAY_VALUES(memfile, values_vec, type)                                                                  \
{                                                                                                                      \
    for (jak_u32 i = 0; i < values_vec->num_elems; i++) {                                                             \
        jak_vector ofType(type) *nested_values = JAK_VECTOR_GET(values_vec, i, jak_vector);                     \
        WRITE_PRIMITIVE_VALUES(memfile, nested_values, type);                                                          \
    }                                                                                                                  \
}

#define PRINT_SIMPLE_PROPS(file, memfile, offset, nesting_level, value_type, type_string, format_string)               \
{                                                                                                                      \
    jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);             \
    jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop_header->num_entries *          \
                                   sizeof(jak_archive_field_sid_t));                                                        \
    value_type *values = (value_type *) JAK_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(value_type));   \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nesting_level)                                                                                         \
    fprintf(file, "[marker: %c (" type_string ")] [num_entries: %d] [", entryMarker, prop_header->num_entries);        \
    for (jak_u32 i = 0; i < prop_header->num_entries; i++) {                                                          \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");                      \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
    for (jak_u32 i = 0; i < prop_header->num_entries; i++) {                                                          \
      fprintf(file, "value: "format_string"%s", values[i], i + 1 < prop_header->num_entries ? ", " : "");              \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

#define PRINT_ARRAY_PROPS(memfile, offset, nesting_level, entryMarker, type, type_string, format_string)               \
{                                                                                                                      \
    jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile, jak_prop_header);             \
                                                                                                                       \
    jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile, prop_header->num_entries *          \
                                        sizeof(jak_archive_field_sid_t));                                                   \
    jak_u32 *array_lengths;                                                                                           \
                                                                                                                       \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nesting_level)                                                                                         \
    fprintf(file, "[marker: %c ("type_string")] [num_entries: %d] [", entryMarker, prop_header->num_entries);          \
                                                                                                                       \
    for (jak_u32 i = 0; i < prop_header->num_entries; i++) {                                                          \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");                      \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    array_lengths = (jak_u32 *) JAK_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(jak_u32));            \
                                                                                                                       \
    for (jak_u32 i = 0; i < prop_header->num_entries; i++) {                                                          \
        fprintf(file, "num_entries: %d%s", array_lengths[i], i + 1 < prop_header->num_entries ? ", " : "");            \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    for (jak_u32 array_idx = 0; array_idx < prop_header->num_entries; array_idx++) {                                  \
        type *values = (type *) JAK_MEMFILE_READ(memfile, array_lengths[array_idx] * sizeof(type));                 \
        fprintf(file, "[");                                                                                            \
        for (jak_u32 i = 0; i < array_lengths[array_idx]; i++) {                                                      \
            fprintf(file, "value: "format_string"%s", values[i], i + 1 < array_lengths[array_idx] ? ", " : "");        \
        }                                                                                                              \
        fprintf(file, "]%s", array_idx + 1 < prop_header->num_entries ? ", " : "");                                    \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "]\n");                                                                                              \
}

#define INTENT_LINE(nesting_level)                                                                                     \
{                                                                                                                      \
    for (unsigned nest_level = 0; nest_level < nesting_level; nest_level++) {                                          \
        fprintf(file, "   ");                                                                                          \
    }                                                                                                                  \
}

#define PRINT_VALUE_ARRAY(type, memfile, header, format_string)                                                        \
{                                                                                                                      \
    jak_u32 num_elements = *JAK_MEMFILE_READ_TYPE(memfile, jak_u32);                                              \
    const type *values = (const type *) JAK_MEMFILE_READ(memfile, num_elements * sizeof(type));                     \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nesting_level);                                                                                        \
    fprintf(file, "   [num_elements: %d] [values: [", num_elements);                                                   \
    for (size_t i = 0; i < num_elements; i++) {                                                                        \
        fprintf(file, "value: "format_string"%s", values[i], i + 1 < num_elements ? ", " : "");                        \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

static jak_offset_t skip_record_header(jak_memfile *memfile);

static void
update_record_header(jak_memfile *memfile, jak_offset_t root_object_header_offset, jak_column_doc *model,
                     jak_u64 record_size);

static bool __serialize(jak_offset_t *offset, jak_error *err, jak_memfile *memfile,
                        jak_column_doc_obj *columndoc,
                        jak_offset_t root_object_header_offset);

static jak_object_flags_u *get_flags(jak_object_flags_u *flags, jak_column_doc_obj *columndoc);

static void update_file_header(jak_memfile *memfile, jak_offset_t root_object_header_offset);

static void skip_file_header(jak_memfile *memfile);

static bool serialize_jak_string_dic(jak_memfile *memfile, jak_error *err, const jak_doc_bulk *context,
                                 jak_packer_e compressor);

static bool print_archive_from_memfile(FILE *file, jak_error *err, jak_memfile *memfile);

bool jak_archive_from_json(jak_archive *out, const char *file, jak_error *err, const char *json_string,
                           jak_packer_e compressor, jak_str_dict_tag_e dictionary,
                           size_t num_async_dic_threads,
                           bool read_optimized,
                           bool bake_jak_string_id_index, jak_archive_callback *callback)
{
        JAK_ERROR_IF_NULL(out);
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(err);
        JAK_ERROR_IF_NULL(json_string);

        JAK_OPTIONAL_CALL(callback, begin_create_from_json);

        jak_memblock *stream;
        FILE *out_file;

        if (!jak_archive_stream_from_json(&stream,
                                          err,
                                          json_string,
                                          compressor,
                                          dictionary,
                                          num_async_dic_threads,
                                          read_optimized,
                                          bake_jak_string_id_index,
                                          callback)) {
                return false;
        }

        JAK_OPTIONAL_CALL(callback, begin_write_archive_file_to_disk);

        if ((out_file = fopen(file, "w")) == NULL) {
                JAK_ERROR(err, JAK_ERR_FOPENWRITE);
                jak_memblock_drop(stream);
                return false;
        }

        if (!jak_archive_write(out_file, stream)) {
                JAK_ERROR(err, JAK_ERR_WRITEARCHIVE);
                fclose(out_file);
                jak_memblock_drop(stream);
                return false;
        }

        fclose(out_file);

        JAK_OPTIONAL_CALL(callback, end_write_archive_file_to_disk);

        JAK_OPTIONAL_CALL(callback, begin_load_archive);

        if (!jak_archive_open(out, file)) {
                JAK_ERROR(err, JAK_ERR_ARCHIVEOPEN);
                return false;
        }

        JAK_OPTIONAL_CALL(callback, end_load_archive);

        jak_memblock_drop(stream);

        JAK_OPTIONAL_CALL(callback, end_create_from_json);

        return true;
}

bool jak_archive_stream_from_json(jak_memblock **stream, jak_error *err, const char *json_string,
                                  jak_packer_e compressor, jak_str_dict_tag_e dictionary,
                                  size_t num_async_dic_threads,
                                  bool read_optimized,
                                  bool bake_id_index, jak_archive_callback *callback)
{
        JAK_ERROR_IF_NULL(stream);
        JAK_ERROR_IF_NULL(err);
        JAK_ERROR_IF_NULL(json_string);

        jak_string_dict dic;
        jak_json_parser parser;
        jak_json_err error_desc;
        jak_doc_bulk bulk;
        jak_doc_entries *partition;
        jak_column_doc *columndoc;
        jak_json jak_json;

        JAK_OPTIONAL_CALL(callback, begin_jak_archive_stream_from_json)

        JAK_OPTIONAL_CALL(callback, begin_setup_jak_string_dict_ionary);
        if (dictionary == JAK_SYNC) {
                jak_encode_sync_create(&dic, 1000, 1000, 1000, 0, NULL);
        } else if (dictionary == JAK_ASYNC) {
                jak_encode_async_create(&dic, 1000, 1000, 1000, num_async_dic_threads, NULL);
        } else {
                JAK_ERROR(err, JAK_ERR_UNKNOWN_DIC_TYPE);
        }

        JAK_OPTIONAL_CALL(callback, end_setup_jak_string_dict_ionary);

        JAK_OPTIONAL_CALL(callback, begin_parse_json);
        jak_json_parser_create(&parser);
        if (!(jak_json_parse(&jak_json, &error_desc, &parser, json_string))) {
                char buffer[2048];
                if (error_desc.token) {
                        sprintf(buffer,
                                "%s. Token %s was found in line %u column %u",
                                error_desc.msg,
                                error_desc.token_type_str,
                                error_desc.token->line,
                                error_desc.token->column);
                        JAK_ERROR_WDETAILS(err, JAK_ERR_JSONPARSEERR, &buffer[0]);
                } else {
                        sprintf(buffer, "%s", error_desc.msg);
                        JAK_ERROR_WDETAILS(err, JAK_ERR_JSONPARSEERR, &buffer[0]);
                }
                return false;
        }
        JAK_OPTIONAL_CALL(callback, end_parse_json);

        JAK_OPTIONAL_CALL(callback, begin_test_json);
        if (!jak_json_test(err, &jak_json)) {
                return false;
        }
        JAK_OPTIONAL_CALL(callback, end_test_json);

        JAK_OPTIONAL_CALL(callback, begin_import_json);
        if (!jak_doc_bulk_create(&bulk, &dic)) {
                JAK_ERROR(err, JAK_ERR_BULKCREATEFAILED);
                return false;
        }

        partition = jak_doc_bulk_new_entries(&bulk);
        jak_doc_bulk_add_json(partition, &jak_json);

        jak_json_drop(&jak_json);

        jak_doc_bulk_shrink(&bulk);

        columndoc = jak_doc_entries_columndoc(&bulk, partition, read_optimized);

        if (!jak_archive_from_model(stream, err, columndoc, compressor, bake_id_index, callback)) {
                return false;
        }

        JAK_OPTIONAL_CALL(callback, end_import_json);

        JAK_OPTIONAL_CALL(callback, begin_cleanup);
        jak_string_dict_drop(&dic);
        jak_doc_bulk_drop(&bulk);
        jak_doc_entries_drop(partition);
        jak_columndoc_free(columndoc);
        free(columndoc);
        JAK_OPTIONAL_CALL(callback, end_cleanup);

        JAK_OPTIONAL_CALL(callback, end_jak_archive_stream_from_json)

        return true;
}

static bool run_jak_string_id_baking(jak_error *err, jak_memblock **stream)
{
        jak_archive archive;
        char tmp_file_name[512];
        jak_uid_t rand_part;
        jak_unique_id_create(&rand_part);
        sprintf(tmp_file_name, "/tmp/jakson-tool-temp-%"
                               PRIu64
                               ".jakson-tool", rand_part);
        FILE *tmp_file;

        if ((tmp_file = fopen(tmp_file_name, "w")) == NULL) {
                JAK_ERROR(err, JAK_ERR_TMP_FOPENWRITE);
                return false;
        }

        if (!jak_archive_write(tmp_file, *stream)) {
                JAK_ERROR(err, JAK_ERR_WRITEARCHIVE);
                fclose(tmp_file);
                remove(tmp_file_name);
                return false;
        }

        fflush(tmp_file);
        fclose(tmp_file);

        if (!jak_archive_open(&archive, tmp_file_name)) {
                JAK_ERROR(err, JAK_ERR_ARCHIVEOPEN);
                return false;
        }

        bool has_index;
        jak_archive_has_query_index_jak_string_id_to_offset(&has_index, &archive);
        if (has_index) {
                JAK_ERROR(err, JAK_ERR_INTERNALERR);
                remove(tmp_file_name);
                return false;
        }

        struct jak_sid_to_offset *index;
        jak_archive_query query;
        jak_query_create(&query, &archive);
        jak_query_create_index_jak_string_id_to_offset(&index, &query);
        jak_query_drop(&query);
        jak_archive_close(&archive);

        if ((tmp_file = fopen(tmp_file_name, "rb+")) == NULL) {
                JAK_ERROR(err, JAK_ERR_TMP_FOPENWRITE);
                return false;
        }

        fseek(tmp_file, 0, SEEK_END);
        jak_offset_t index_pos = ftell(tmp_file);
        jak_query_index_id_to_offset_serialize(tmp_file, err, index);
        jak_offset_t file_length = ftell(tmp_file);
        fseek(tmp_file, 0, SEEK_SET);

        jak_archive_header header;
        size_t nread = fread(&header, sizeof(jak_archive_header), 1, tmp_file);
        JAK_ERROR_IF(nread != 1, err, JAK_ERR_FREAD_FAILED);
        header.jak_string_id_to_offset_index_offset = index_pos;
        fseek(tmp_file, 0, SEEK_SET);
        int nwrite = fwrite(&header, sizeof(jak_archive_header), 1, tmp_file);
        JAK_ERROR_IF(nwrite != 1, err, JAK_ERR_FWRITE_FAILED);
        fseek(tmp_file, 0, SEEK_SET);

        jak_query_drop_index_jak_string_id_to_offset(index);

        jak_memblock_drop(*stream);
        jak_memblock_from_file(stream, tmp_file, file_length);

        remove(tmp_file_name);

        return true;
}

bool jak_archive_from_model(jak_memblock **stream, jak_error *err, jak_column_doc *model,
                            jak_packer_e compressor,
                            bool bake_jak_string_id_index, jak_archive_callback *callback)
{
        JAK_ERROR_IF_NULL(model)
        JAK_ERROR_IF_NULL(stream)
        JAK_ERROR_IF_NULL(err)

        JAK_OPTIONAL_CALL(callback, begin_create_from_model)

        jak_memblock_create(stream, 1024 * 1024 * 1024);
        jak_memfile memfile;
        jak_memfile_open(&memfile, *stream, JAK_READ_WRITE);

        JAK_OPTIONAL_CALL(callback, begin_write_jak_string_table);
        skip_file_header(&memfile);
        if (!serialize_jak_string_dic(&memfile, err, model->bulk, compressor)) {
                return false;
        }
        JAK_OPTIONAL_CALL(callback, end_write_jak_string_table);

        JAK_OPTIONAL_CALL(callback, begin_write_record_table);
        jak_offset_t record_header_offset = skip_record_header(&memfile);
        update_file_header(&memfile, record_header_offset);
        jak_offset_t root_object_header_offset = jak_memfile_tell(&memfile);
        if (!__serialize(NULL, err, &memfile, &model->columndoc, root_object_header_offset)) {
                return false;
        }
        jak_u64 record_size = jak_memfile_tell(&memfile) - (record_header_offset + sizeof(jak_record_header));
        update_record_header(&memfile, record_header_offset, model, record_size);
        JAK_OPTIONAL_CALL(callback, end_write_record_table);

        jak_memfile_shrink(&memfile);

        if (bake_jak_string_id_index) {
                /* create string id to offset index, and append it to the CARBON file */
                JAK_OPTIONAL_CALL(callback, begin_jak_string_id_index_baking);
                if (!run_jak_string_id_baking(err, stream)) {
                        return false;
                }
                JAK_OPTIONAL_CALL(callback, end_jak_string_id_index_baking);
        } else {
                JAK_OPTIONAL_CALL(callback, skip_jak_string_id_index_baking);
        }

        JAK_OPTIONAL_CALL(callback, end_create_from_model)

        return true;
}

jak_archive_io_context *jak_archive_io_context_create(jak_archive *archive)
{
        JAK_ERROR_IF_NULL(archive);
        jak_archive_io_context *context;
        if (jak_io_context_create(&context, &archive->err, archive->disk_file_path)) {
                return context;
        } else {
                JAK_ERROR(&archive->err, JAK_ERR_IO)
                return NULL;
        }
}

bool jak_archive_write(FILE *file, const jak_memblock *stream)
{
        return jak_memblock_write_to_file(file, stream);
}

bool jak_archive_load(jak_memblock **stream, FILE *file)
{
        long start = ftell(file);
        fseek(file, 0, SEEK_END);
        long end = ftell(file);
        fseek(file, start, SEEK_SET);
        long fileSize = (end - start);

        return jak_memblock_from_file(stream, file, fileSize);
}

bool jak_archive_print(FILE *file, jak_error *err, jak_memblock *stream)
{
        jak_memfile memfile;
        jak_memfile_open(&memfile, stream, JAK_READ_ONLY);
        if (jak_memfile_size(&memfile)
            < sizeof(jak_archive_header) + sizeof(jak_string_table_header) +
              sizeof(jak_object_header)) {
                JAK_ERROR(err, JAK_ERR_NOCARBONSTREAM);
                return false;
        } else {
                return print_archive_from_memfile(file, err, &memfile);
        }
}

bool _jak_archive_print_object(FILE *file, jak_error *err, jak_memfile *memfile, unsigned nesting_level);

static jak_u32 flags_to_int32(jak_object_flags_u *flags)
{
        return *((jak_i32 *) flags);
}

static const char *array_value_type_to_string(jak_error *err, jak_archive_field_e type)
{
        switch (type) {
                case JAK_FIELD_NULL:
                        return "Null Array";
                case JAK_FIELD_BOOLEAN:
                        return "Boolean Array";
                case JAK_FIELD_INT8:
                        return "Int8 Array";
                case JAK_FIELD_INT16:
                        return "Int16 Array";
                case JAK_FIELD_INT32:
                        return "Int32 Array";
                case JAK_FIELD_INT64:
                        return "Int64 Array";
                case JAK_FIELD_UINT8:
                        return "UInt8 Array";
                case JAK_FIELD_UINT16:
                        return "UInt16 Array";
                case JAK_FIELD_UINT32:
                        return "UInt32 Array";
                case JAK_FIELD_UINT64:
                        return "UInt64 Array";
                case JAK_FIELD_FLOAT:
                        return "UIntFloat Array";
                case JAK_FIELD_STRING:
                        return "Text Array";
                case JAK_FIELD_OBJECT:
                        return "Object Array";
                default: {
                        JAK_ERROR(err, JAK_ERR_NOVALUESTR)
                        return NULL;
                }
        }
}

static void
write_primitive_key_column(jak_memfile *memfile, jak_vector ofType(jak_archive_field_sid_t) *keys)
{
        jak_archive_field_sid_t *jak_string_ids = JAK_VECTOR_ALL(keys, jak_archive_field_sid_t);
        jak_memfile_write(memfile, jak_string_ids, keys->num_elems * sizeof(jak_archive_field_sid_t));
}

static jak_offset_t skip_var_value_offset_column(jak_memfile *memfile, size_t num_keys)
{
        jak_offset_t result = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, num_keys * sizeof(jak_offset_t));
        return result;
}

static void write_var_value_offset_column(jak_memfile *file, jak_offset_t where, jak_offset_t after,
                                          const jak_offset_t *values,
                                          size_t n)
{
        jak_memfile_seek(file, where);
        jak_memfile_write(file, values, n * sizeof(jak_offset_t));
        jak_memfile_seek(file, after);
}

static bool
write_primitive_fixed_value_column(jak_memfile *memfile, jak_error *err, jak_archive_field_e type,
                                   jak_vector ofType(T) *values_vec)
{
        JAK_ASSERT (type != JAK_FIELD_OBJECT); /** use 'write_primitive_var_value_column' instead */

        switch (type) {
                case JAK_FIELD_NULL:
                        break;
                case JAK_FIELD_BOOLEAN: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_boolean_t);
                        break;
                case JAK_FIELD_INT8: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_i8_t);
                        break;
                case JAK_FIELD_INT16: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_i16_t);
                        break;
                case JAK_FIELD_INT32: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_i32_t);
                        break;
                case JAK_FIELD_INT64: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_i64_t);
                        break;
                case JAK_FIELD_UINT8: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_u8_t);
                        break;
                case JAK_FIELD_UINT16: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_u16_t);
                        break;
                case JAK_FIELD_UINT32: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_u32_t);
                        break;
                case JAK_FIELD_UINT64: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_FLOAT: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_number_t);
                        break;
                case JAK_FIELD_STRING: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_archive_field_sid_t);
                        break;
                default: JAK_ERROR(err, JAK_ERR_NOTYPE);
                        return false;
        }
        return true;
}

static jak_offset_t *__write_primitive_column(jak_memfile *memfile, jak_error *err,
                                              jak_vector ofType(jak_column_doc_obj) *values_vec,
                                              jak_offset_t root_offset)
{
        jak_offset_t *result = JAK_MALLOC(values_vec->num_elems * sizeof(jak_offset_t));
        jak_column_doc_obj *mapped = JAK_VECTOR_ALL(values_vec, jak_column_doc_obj);
        for (jak_u32 i = 0; i < values_vec->num_elems; i++) {
                jak_column_doc_obj *obj = mapped + i;
                result[i] = jak_memfile_tell(memfile) - root_offset;
                if (!__serialize(NULL, err, memfile, obj, root_offset)) {
                        return NULL;
                }
        }
        return result;
}

static bool __write_array_len_column(jak_error *err, jak_memfile *memfile, jak_archive_field_e type,
                                     jak_vector ofType(...) *values)
{
        switch (type) {
                case JAK_FIELD_NULL:
                        break;
                case JAK_FIELD_BOOLEAN:
                case JAK_FIELD_INT8:
                case JAK_FIELD_INT16:
                case JAK_FIELD_INT32:
                case JAK_FIELD_INT64:
                case JAK_FIELD_UINT8:
                case JAK_FIELD_UINT16:
                case JAK_FIELD_UINT32:
                case JAK_FIELD_UINT64:
                case JAK_FIELD_FLOAT:
                case JAK_FIELD_STRING:
                        for (jak_u32 i = 0; i < values->num_elems; i++) {
                                jak_vector *arrays = JAK_VECTOR_GET(values, i, jak_vector);
                                jak_memfile_write(memfile, &arrays->num_elems, sizeof(jak_u32));
                        }
                        break;
                case JAK_FIELD_OBJECT: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_ILLEGALIMPL)
                        return false;
                        break;
                default: JAK_ERROR(err, JAK_ERR_NOTYPE);
                        return false;
        }
        return true;
}

static bool write_array_value_column(jak_memfile *memfile, jak_error *err, jak_archive_field_e type,
                                     jak_vector ofType(...) *values_vec)
{

        switch (type) {
                case JAK_FIELD_NULL: WRITE_PRIMITIVE_VALUES(memfile, values_vec, jak_u32);
                        break;
                case JAK_FIELD_BOOLEAN: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_boolean_t);
                        break;
                case JAK_FIELD_INT8: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_i8_t);
                        break;
                case JAK_FIELD_INT16: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_i16_t);
                        break;
                case JAK_FIELD_INT32: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_i32_t);
                        break;
                case JAK_FIELD_INT64: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_i64_t);
                        break;
                case JAK_FIELD_UINT8: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_UINT16: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_u16_t);
                        break;
                case JAK_FIELD_UINT32: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_u32_t);
                        break;
                case JAK_FIELD_UINT64: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_u64_t);
                        break;
                case JAK_FIELD_FLOAT: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_number_t);
                        break;
                case JAK_FIELD_STRING: WRITE_ARRAY_VALUES(memfile, values_vec, jak_archive_field_sid_t);
                        break;
                case JAK_FIELD_OBJECT: JAK_ERROR_PRINT_AND_DIE(JAK_ERR_NOTIMPL)
                        return false;
                default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                        return false;
        }
        return true;
}

static bool write_array_prop(jak_offset_t *offset, jak_error *err, jak_memfile *memfile,
                             jak_vector ofType(jak_archive_field_sid_t) *keys, jak_archive_field_e type,
                             jak_vector ofType(...) *values,
                             jak_offset_t root_object_header_offset)
{
        JAK_ASSERT(keys->num_elems == values->num_elems);

        if (keys->num_elems > 0) {
                jak_prop_header header =
                        {.marker = jak_global_marker_symbols[jak_global_value_array_marker_mapping[type].marker].symbol, .num_entries = keys
                                ->num_elems};
                jak_offset_t prop_ofOffset = jak_memfile_tell(memfile);
                jak_memfile_write(memfile, &header, sizeof(jak_prop_header));

                write_primitive_key_column(memfile, keys);
                if (!__write_array_len_column(err, memfile, type, values)) {
                        return false;
                }
                if (!write_array_value_column(memfile, err, type, values)) {
                        return false;
                }
                *offset = (prop_ofOffset - root_object_header_offset);
        } else {
                *offset = 0;
        }
        return true;
}

static bool write_array_props(jak_memfile *memfile, jak_error *err, jak_column_doc_obj *columndoc,
                              jak_archive_prop_offs *offsets, jak_offset_t root_object_header_offset)
{
        if (!write_array_prop(&offsets->null_arrays,
                              err,
                              memfile,
                              &columndoc->null_array_prop_keys,
                              JAK_FIELD_NULL,
                              &columndoc->null_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->bool_arrays,
                              err,
                              memfile,
                              &columndoc->bool_array_prop_keys,
                              JAK_FIELD_BOOLEAN,
                              &columndoc->bool_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->int8_arrays,
                              err,
                              memfile,
                              &columndoc->int8_array_prop_keys,
                              JAK_FIELD_INT8,
                              &columndoc->int8_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->int16_arrays,
                              err,
                              memfile,
                              &columndoc->int16_array_prop_keys,
                              JAK_FIELD_INT16,
                              &columndoc->int16_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->int32_arrays,
                              err,
                              memfile,
                              &columndoc->int32_array_prop_keys,
                              JAK_FIELD_INT32,
                              &columndoc->int32_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->int64_arrays,
                              err,
                              memfile,
                              &columndoc->int64_array_prop_keys,
                              JAK_FIELD_INT64,
                              &columndoc->int64_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->uint8_arrays,
                              err,
                              memfile,
                              &columndoc->uint8_array_prop_keys,
                              JAK_FIELD_UINT8,
                              &columndoc->uint8_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->uint16_arrays,
                              err,
                              memfile,
                              &columndoc->uint16_array_prop_keys,
                              JAK_FIELD_UINT16,
                              &columndoc->uint16_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->uint32_arrays,
                              err,
                              memfile,
                              &columndoc->uint32_array_prop_keys,
                              JAK_FIELD_UINT32,
                              &columndoc->uint32_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->uint64_arrays,
                              err,
                              memfile,
                              &columndoc->uint64_array_prop_keys,
                              JAK_FIELD_UINT64,
                              &columndoc->ui64_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->float_arrays,
                              err,
                              memfile,
                              &columndoc->float_array_prop_keys,
                              JAK_FIELD_FLOAT,
                              &columndoc->float_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        if (!write_array_prop(&offsets->jak_string_arrays,
                              err,
                              memfile,
                              &columndoc->jak_string_array_prop_keys,
                              JAK_FIELD_STRING,
                              &columndoc->jak_string_array_prop_vals,
                              root_object_header_offset)) {
                return false;
        }
        return true;
}

/** Fixed-length property lists; value position can be determined by size of value and position of key in key column.
 * In contrast, variable-length property list require an additional offset column (see 'write_var_props') */
static bool write_fixed_props(jak_offset_t *offset, jak_error *err, jak_memfile *memfile,
                              jak_vector ofType(jak_archive_field_sid_t) *keys, jak_archive_field_e type,
                              jak_vector ofType(T) *values)
{
        JAK_ASSERT(!values || keys->num_elems == values->num_elems);
        JAK_ASSERT(type != JAK_FIELD_OBJECT); /** use 'write_var_props' instead */

        if (keys->num_elems > 0) {
                jak_prop_header header =
                        {.marker = jak_global_marker_symbols[valueMarkerMapping[type].marker].symbol, .num_entries = keys
                                ->num_elems};

                jak_offset_t prop_ofOffset = jak_memfile_tell(memfile);
                jak_memfile_write(memfile, &header, sizeof(jak_prop_header));

                write_primitive_key_column(memfile, keys);
                if (!write_primitive_fixed_value_column(memfile, err, type, values)) {
                        return false;
                }
                *offset = prop_ofOffset;
        } else {
                *offset = 0;
        }
        return true;
}

/** Variable-length property lists; value position cannot be determined by position of key in key column, since single
 * value has unknown size. Hence, a dedicated offset column is added to these properties allowing to seek directly
 * to a particular property. Due to the move of strings (i.e., variable-length values) to a dedicated string table,
 * the only variable-length value for properties are "JSON objects".
 * In contrast, fixed-length property list doesn't require an additional offset column (see 'write_fixed_props') */
static bool write_var_props(jak_offset_t *offset, jak_error *err, jak_memfile *memfile,
                            jak_vector ofType(jak_archive_field_sid_t) *keys,
                            jak_vector ofType(jak_column_doc_obj) *objects,
                            jak_offset_t root_object_header_offset)
{
        JAK_ASSERT(!objects || keys->num_elems == objects->num_elems);

        if (keys->num_elems > 0) {
                jak_prop_header header = {.marker = JAK_MARKER_SYMBOL_PROP_OBJECT, .num_entries = keys->num_elems};

                jak_offset_t prop_ofOffset = jak_memfile_tell(memfile);
                jak_memfile_write(memfile, &header, sizeof(jak_prop_header));

                write_primitive_key_column(memfile, keys);
                jak_offset_t value_offset = skip_var_value_offset_column(memfile, keys->num_elems);
                jak_offset_t *value_offsets = __write_primitive_column(memfile, err, objects,
                                                                       root_object_header_offset);
                if (!value_offsets) {
                        return false;
                }

                jak_offset_t last = jak_memfile_tell(memfile);
                write_var_value_offset_column(memfile, value_offset, last, value_offsets, keys->num_elems);
                free(value_offsets);
                *offset = prop_ofOffset;
        } else {
                *offset = 0;
        }
        return true;
}

static bool
write_primitive_props(jak_memfile *memfile, jak_error *err, jak_column_doc_obj *columndoc,
                      jak_archive_prop_offs *offsets, jak_offset_t root_object_header_offset)
{
        if (!write_fixed_props(&offsets->nulls, err, memfile, &columndoc->null_prop_keys, JAK_FIELD_NULL, NULL)) {
                return false;
        }
        if (!write_fixed_props(&offsets->bools,
                               err,
                               memfile,
                               &columndoc->bool_prop_keys,
                               JAK_FIELD_BOOLEAN,
                               &columndoc->bool_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->int8s,
                               err,
                               memfile,
                               &columndoc->int8_prop_keys,
                               JAK_FIELD_INT8,
                               &columndoc->int8_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->int16s,
                               err,
                               memfile,
                               &columndoc->int16_prop_keys,
                               JAK_FIELD_INT16,
                               &columndoc->int16_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->int32s,
                               err,
                               memfile,
                               &columndoc->int32_prop_keys,
                               JAK_FIELD_INT32,
                               &columndoc->int32_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->int64s,
                               err,
                               memfile,
                               &columndoc->int64_prop_keys,
                               JAK_FIELD_INT64,
                               &columndoc->int64_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->uint8s,
                               err,
                               memfile,
                               &columndoc->uint8_prop_keys,
                               JAK_FIELD_UINT8,
                               &columndoc->uint8_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->uint16s,
                               err,
                               memfile,
                               &columndoc->uint16_prop_keys,
                               JAK_FIELD_UINT16,
                               &columndoc->uint16_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->uint32s,
                               err,
                               memfile,
                               &columndoc->uin32_prop_keys,
                               JAK_FIELD_UINT32,
                               &columndoc->uint32_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->uint64s,
                               err,
                               memfile,
                               &columndoc->uint64_prop_keys,
                               JAK_FIELD_UINT64,
                               &columndoc->uint64_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->floats,
                               err,
                               memfile,
                               &columndoc->float_prop_keys,
                               JAK_FIELD_FLOAT,
                               &columndoc->float_prop_vals)) {
                return false;
        }
        if (!write_fixed_props(&offsets->strings,
                               err,
                               memfile,
                               &columndoc->jak_string_prop_keys,
                               JAK_FIELD_STRING,
                               &columndoc->jak_string_prop_vals)) {
                return false;
        }
        if (!write_var_props(&offsets->objects,
                             err,
                             memfile,
                             &columndoc->obj_prop_keys,
                             &columndoc->obj_prop_vals,
                             root_object_header_offset)) {
                return false;
        }

        offsets->nulls -= root_object_header_offset;
        offsets->bools -= root_object_header_offset;
        offsets->int8s -= root_object_header_offset;
        offsets->int16s -= root_object_header_offset;
        offsets->int32s -= root_object_header_offset;
        offsets->int64s -= root_object_header_offset;
        offsets->uint8s -= root_object_header_offset;
        offsets->uint16s -= root_object_header_offset;
        offsets->uint32s -= root_object_header_offset;
        offsets->uint64s -= root_object_header_offset;
        offsets->floats -= root_object_header_offset;
        offsets->strings -= root_object_header_offset;
        offsets->objects -= root_object_header_offset;
        return true;
}

static bool write_column_entry(jak_memfile *memfile, jak_error *err, jak_archive_field_e type,
                               jak_vector ofType(<T>) *column, jak_offset_t root_object_header_offset)
{
        jak_memfile_write(memfile, &column->num_elems, sizeof(jak_u32));
        switch (type) {
                case JAK_FIELD_NULL:
                        jak_memfile_write(memfile, column->base, column->num_elems * sizeof(jak_u32));
                        break;
                case JAK_FIELD_BOOLEAN:
                case JAK_FIELD_INT8:
                case JAK_FIELD_INT16:
                case JAK_FIELD_INT32:
                case JAK_FIELD_INT64:
                case JAK_FIELD_UINT8:
                case JAK_FIELD_UINT16:
                case JAK_FIELD_UINT32:
                case JAK_FIELD_UINT64:
                case JAK_FIELD_FLOAT:
                case JAK_FIELD_STRING:
                        jak_memfile_write(memfile, column->base, column->num_elems * GET_TYPE_SIZE(type));
                        break;
                case JAK_FIELD_OBJECT: {
                        jak_offset_t preObjectNext = 0;
                        for (size_t i = 0; i < column->num_elems; i++) {
                                jak_column_doc_obj *object = JAK_VECTOR_GET(column, i, jak_column_doc_obj);
                                if (JAK_LIKELY(preObjectNext != 0)) {
                                        jak_offset_t continuePos = jak_memfile_tell(memfile);
                                        jak_offset_t relativeContinuePos = continuePos - root_object_header_offset;
                                        jak_memfile_seek(memfile, preObjectNext);
                                        jak_memfile_write(memfile, &relativeContinuePos, sizeof(jak_offset_t));
                                        jak_memfile_seek(memfile, continuePos);
                                }
                                if (!__serialize(&preObjectNext, err, memfile, object, root_object_header_offset)) {
                                        return false;
                                }
                        }
                }
                        break;
                default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                        return false;
        }
        return true;
}

static bool write_column(jak_memfile *memfile, jak_error *err, jak_column_doc_column *column,
                         jak_offset_t root_object_header_offset)
{
        JAK_ASSERT(column->array_positions.num_elems == column->values.num_elems);

        jak_column_header header = {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_COLUMN].symbol, .column_name = column
                ->key_name, .value_type = jak_global_marker_symbols[jak_global_value_array_marker_mapping[column->type].marker]
                .symbol, .num_entries = column->values.num_elems};

        jak_memfile_write(memfile, &header, sizeof(jak_column_header));

        /** skip offset column to value entry points */
        jak_offset_t value_entry_offsets = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, column->values.num_elems * sizeof(jak_offset_t));

        jak_memfile_write(memfile, column->array_positions.base, column->array_positions.num_elems * sizeof(jak_u32));

        for (size_t i = 0; i < column->values.num_elems; i++) {
                jak_vector ofType(<T>) *column_data = JAK_VECTOR_GET(&column->values, i, jak_vector);
                jak_offset_t column_entry_offset = jak_memfile_tell(memfile);
                jak_offset_t relative_entry_offset = column_entry_offset - root_object_header_offset;
                jak_memfile_seek(memfile, value_entry_offsets + i * sizeof(jak_offset_t));
                jak_memfile_write(memfile, &relative_entry_offset, sizeof(jak_offset_t));
                jak_memfile_seek(memfile, column_entry_offset);
                if (!write_column_entry(memfile, err, column->type, column_data, root_object_header_offset)) {
                        return false;
                }
        }
        return true;
}

static bool write_object_array_props(jak_memfile *memfile, jak_error *err,
                                     jak_vector ofType(jak_column_doc_group) *object_key_columns,
                                     jak_archive_prop_offs *offsets,
                                     jak_offset_t root_object_header_offset)
{
        if (object_key_columns->num_elems > 0) {
                jak_object_array_header header = {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_PROP_OBJECT_ARRAY]
                        .symbol, .num_entries = object_key_columns->num_elems};

                offsets->object_arrays = jak_memfile_tell(memfile) - root_object_header_offset;
                jak_memfile_write(memfile, &header, sizeof(jak_object_array_header));

                for (size_t i = 0; i < object_key_columns->num_elems; i++) {
                        jak_column_doc_group *column_group = JAK_VECTOR_GET(object_key_columns, i,
                                                                            jak_column_doc_group);
                        jak_memfile_write(memfile, &column_group->key, sizeof(jak_archive_field_sid_t));
                }

                // skip offset column to column groups
                jak_offset_t column_offsets = jak_memfile_tell(memfile);
                jak_memfile_skip(memfile, object_key_columns->num_elems * sizeof(jak_offset_t));

                for (size_t i = 0; i < object_key_columns->num_elems; i++) {
                        jak_column_doc_group *column_group = JAK_VECTOR_GET(object_key_columns, i,
                                                                            jak_column_doc_group);
                        jak_offset_t this_column_offset_relative = jak_memfile_tell(memfile) - root_object_header_offset;

                        /* write an object-id for each position number */
                        size_t max_pos = 0;
                        for (size_t k = 0; k < column_group->columns.num_elems; k++) {
                                jak_column_doc_column
                                        *column = JAK_VECTOR_GET(&column_group->columns, k, jak_column_doc_column);
                                const jak_u32 *array_pos = JAK_VECTOR_ALL(&column->array_positions, jak_u32);
                                for (size_t m = 0; m < column->array_positions.num_elems; m++) {
                                        max_pos = JAK_MAX(max_pos, array_pos[m]);
                                }
                        }
                        jak_column_group_header jak_column_group_header =
                                {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_COLUMN_GROUP].symbol, .num_columns = column_group
                                        ->columns.num_elems, .num_objects = max_pos + 1};
                        jak_memfile_write(memfile, &jak_column_group_header, sizeof(jak_column_group_header));

                        for (size_t i = 0; i < jak_column_group_header.num_objects; i++) {
                                jak_uid_t oid;
                                if (!jak_unique_id_create(&oid)) {
                                        JAK_ERROR(err, JAK_ERR_THREADOOOBJIDS);
                                        return false;
                                }
                                jak_memfile_write(memfile, &oid, sizeof(jak_uid_t));
                        }

                        jak_offset_t continue_write = jak_memfile_tell(memfile);
                        jak_memfile_seek(memfile, column_offsets + i * sizeof(jak_offset_t));
                        jak_memfile_write(memfile, &this_column_offset_relative, sizeof(jak_offset_t));
                        jak_memfile_seek(memfile, continue_write);

                        jak_offset_t offset_column_to_columns = continue_write;
                        jak_memfile_skip(memfile, column_group->columns.num_elems * sizeof(jak_offset_t));

                        for (size_t k = 0; k < column_group->columns.num_elems; k++) {
                                jak_column_doc_column
                                        *column = JAK_VECTOR_GET(&column_group->columns, k, jak_column_doc_column);
                                jak_offset_t continue_write = jak_memfile_tell(memfile);
                                jak_offset_t column_off = continue_write - root_object_header_offset;
                                jak_memfile_seek(memfile, offset_column_to_columns + k * sizeof(jak_offset_t));
                                jak_memfile_write(memfile, &column_off, sizeof(jak_offset_t));
                                jak_memfile_seek(memfile, continue_write);
                                if (!write_column(memfile, err, column, root_object_header_offset)) {
                                        return false;
                                }
                        }

                }
        } else {
                offsets->object_arrays = 0;
        }

        return true;
}

static jak_offset_t skip_record_header(jak_memfile *memfile)
{
        jak_offset_t offset = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, sizeof(jak_record_header));
        return offset;
}

static void
update_record_header(jak_memfile *memfile, jak_offset_t root_object_header_offset, jak_column_doc *model,
                     jak_u64 record_size)
{
        jak_record_flags flags = {.bits.is_sorted = model->read_optimized};
        jak_record_header
                header = {.marker = JAK_MARKER_SYMBOL_RECORD_HEADER, .flags = flags.value, .record_size = record_size};
        jak_offset_t offset;
        jak_memfile_get_offset(&offset, memfile);
        jak_memfile_seek(memfile, root_object_header_offset);
        jak_memfile_write(memfile, &header, sizeof(jak_record_header));
        jak_memfile_seek(memfile, offset);
}

static void propOffsetsWrite(jak_memfile *memfile, const jak_object_flags_u *flags,
                             jak_archive_prop_offs *prop_offsets)
{
        if (flags->bits.has_null_props) {
                jak_memfile_write(memfile, &prop_offsets->nulls, sizeof(jak_offset_t));
        }
        if (flags->bits.has_bool_props) {
                jak_memfile_write(memfile, &prop_offsets->bools, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int8_props) {
                jak_memfile_write(memfile, &prop_offsets->int8s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int16_props) {
                jak_memfile_write(memfile, &prop_offsets->int16s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int32_props) {
                jak_memfile_write(memfile, &prop_offsets->int32s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int64_props) {
                jak_memfile_write(memfile, &prop_offsets->int64s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint8_props) {
                jak_memfile_write(memfile, &prop_offsets->uint8s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint16_props) {
                jak_memfile_write(memfile, &prop_offsets->uint16s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint32_props) {
                jak_memfile_write(memfile, &prop_offsets->uint32s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint64_props) {
                jak_memfile_write(memfile, &prop_offsets->uint64s, sizeof(jak_offset_t));
        }
        if (flags->bits.has_float_props) {
                jak_memfile_write(memfile, &prop_offsets->floats, sizeof(jak_offset_t));
        }
        if (flags->bits.has_jak_string_props) {
                jak_memfile_write(memfile, &prop_offsets->strings, sizeof(jak_offset_t));
        }
        if (flags->bits.has_object_props) {
                jak_memfile_write(memfile, &prop_offsets->objects, sizeof(jak_offset_t));
        }
        if (flags->bits.has_null_array_props) {
                jak_memfile_write(memfile, &prop_offsets->null_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_bool_array_props) {
                jak_memfile_write(memfile, &prop_offsets->bool_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int8_array_props) {
                jak_memfile_write(memfile, &prop_offsets->int8_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int16_array_props) {
                jak_memfile_write(memfile, &prop_offsets->int16_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int32_array_props) {
                jak_memfile_write(memfile, &prop_offsets->int32_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_int64_array_props) {
                jak_memfile_write(memfile, &prop_offsets->int64_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint8_array_props) {
                jak_memfile_write(memfile, &prop_offsets->uint8_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint16_array_props) {
                jak_memfile_write(memfile, &prop_offsets->uint16_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint32_array_props) {
                jak_memfile_write(memfile, &prop_offsets->uint32_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_uint64_array_props) {
                jak_memfile_write(memfile, &prop_offsets->uint64_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_float_array_props) {
                jak_memfile_write(memfile, &prop_offsets->float_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_jak_string_array_props) {
                jak_memfile_write(memfile, &prop_offsets->jak_string_arrays, sizeof(jak_offset_t));
        }
        if (flags->bits.has_object_array_props) {
                jak_memfile_write(memfile, &prop_offsets->object_arrays, sizeof(jak_offset_t));
        }
}

static void prop_offsets_skip_write(jak_memfile *memfile, const jak_object_flags_u *flags)
{
        unsigned num_skip_offset_bytes = 0;
        if (flags->bits.has_null_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_bool_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int8_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int16_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int32_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int64_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint8_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint16_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint32_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint64_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_float_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_jak_string_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_object_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_null_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_bool_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int8_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int16_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int32_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_int64_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint8_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint16_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint32_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_uint64_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_float_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_jak_string_array_props) {
                num_skip_offset_bytes++;
        }
        if (flags->bits.has_object_array_props) {
                num_skip_offset_bytes++;
        }

        jak_memfile_skip(memfile, num_skip_offset_bytes * sizeof(jak_offset_t));
}

static bool __serialize(jak_offset_t *offset, jak_error *err, jak_memfile *memfile,
                        jak_column_doc_obj *columndoc,
                        jak_offset_t root_object_header_offset)
{
        jak_object_flags_u flags;
        jak_archive_prop_offs prop_offsets;
        get_flags(&flags, columndoc);

        jak_offset_t header_offset = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, sizeof(jak_object_header));

        prop_offsets_skip_write(memfile, &flags);
        jak_offset_t next_offset = jak_memfile_tell(memfile);
        jak_offset_t default_next_nil = 0;
        jak_memfile_write(memfile, &default_next_nil, sizeof(jak_offset_t));

        if (!write_primitive_props(memfile, err, columndoc, &prop_offsets, root_object_header_offset)) {
                return false;
        }
        if (!write_array_props(memfile, err, columndoc, &prop_offsets, root_object_header_offset)) {
                return false;
        }
        if (!write_object_array_props(memfile,
                                      err,
                                      &columndoc->obj_array_props,
                                      &prop_offsets,
                                      root_object_header_offset)) {
                return false;
        }

        jak_memfile_write(memfile, &jak_global_marker_symbols[JAK_MARKER_TYPE_OBJECT_END].symbol, 1);

        jak_offset_t object_end_offset = jak_memfile_tell(memfile);
        jak_memfile_seek(memfile, header_offset);

        jak_uid_t oid;
        if (!jak_unique_id_create(&oid)) {
                JAK_ERROR(err, JAK_ERR_THREADOOOBJIDS);
                return false;
        }

        jak_object_header header =
                {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_OBJECT_BEGIN].symbol, .oid = oid, .flags = flags_to_int32(
                        &flags),

                };

        jak_memfile_write(memfile, &header, sizeof(jak_object_header));

        propOffsetsWrite(memfile, &flags, &prop_offsets);

        jak_memfile_seek(memfile, object_end_offset);
        JAK_OPTIONAL_SET(offset, next_offset);
        return true;
}

static char *embedded_dic_flags_to_string(const jak_string_tab_flags_u *flags)
{
        size_t max = 2048;
        char *string = JAK_MALLOC(max + 1);
        size_t length = 0;

        if (flags->value == 0) {
                strcpy(string, " uncompressed");
                length = strlen(string);
                JAK_ASSERT(length <= max);
        } else {

                for (size_t i = 0; i < JAK_ARRAY_LENGTH(jak_global_pack_strategy_register); i++) {
                        if (flags->value & jak_global_pack_strategy_register[i].flag_bit) {
                                strcpy(string + length, jak_global_pack_strategy_register[i].name);
                                length = strlen(string);
                                strcpy(string + length, " ");
                                length = strlen(string);
                        }
                }
        }
        string[length] = '\0';
        return string;
}

static char *record_header_flags_to_string(const jak_record_flags *flags)
{
        size_t max = 2048;
        char *string = JAK_MALLOC(max + 1);
        size_t length = 0;

        if (flags->value == 0) {
                strcpy(string, " none");
                length = strlen(string);
                JAK_ASSERT(length <= max);
        } else {
                if (flags->bits.is_sorted) {
                        strcpy(string + length, " sorted");
                        length = strlen(string);
                        JAK_ASSERT(length <= max);
                }
        }
        string[length] = '\0';
        return string;
}

static bool serialize_jak_string_dic(jak_memfile *memfile, jak_error *err, const jak_doc_bulk *context,
                                 jak_packer_e compressor)
{
        jak_string_tab_flags_u flags;
        jak_packer strategy;
        jak_string_table_header header;

        jak_vector ofType (const char *) *strings;
        jak_vector ofType(jak_archive_field_sid_t) *jak_string_ids;

        jak_doc_bulk_get_dic_contents(&strings, &jak_string_ids, context);

        JAK_ASSERT(strings->num_elems == jak_string_ids->num_elems);

        flags.value = 0;
        if (!jak_pack_by_type(err, &strategy, compressor)) {
                return false;
        }
        jak_u8 flag_bit = jak_pack_flagbit_by_type(compressor);
        JAK_SET_BITS(flags.value, flag_bit);

        jak_offset_t header_pos = jak_memfile_tell(memfile);
        jak_memfile_skip(memfile, sizeof(jak_string_table_header));

        jak_offset_t extra_begin_off = jak_memfile_tell(memfile);
        jak_pack_write_extra(err, &strategy, memfile, strings);
        jak_offset_t extra_end_off = jak_memfile_tell(memfile);

        header = (jak_string_table_header) {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_STR_DIC]
                .symbol, .flags = flags.value, .num_entries = strings
                ->num_elems, .first_entry = jak_memfile_tell(memfile), .compressor_extra_size = (extra_end_off
                                                                                             - extra_begin_off)};

        for (size_t i = 0; i < strings->num_elems; i++) {
                jak_archive_field_sid_t id = *JAK_VECTOR_GET(jak_string_ids, i, jak_archive_field_sid_t);
                const char *string = *JAK_VECTOR_GET(strings, i, char *);

                jak_string_entry_header header = {.marker = jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR]
                        .symbol, .next_entry_off = 0, .jak_string_id = id, .jak_string_len = strlen(string)};

                jak_offset_t header_pos_off = jak_memfile_tell(memfile);
                jak_memfile_skip(memfile, sizeof(jak_string_entry_header));

                if (!jak_pack_encode(err, &strategy, memfile, string)) {
                        JAK_ERROR_PRINT(err.code);
                        return false;
                }
                jak_offset_t continue_off = jak_memfile_tell(memfile);
                jak_memfile_seek(memfile, header_pos_off);
                header.next_entry_off = i + 1 < strings->num_elems ? continue_off : 0;
                jak_memfile_write(memfile, &header, sizeof(jak_string_entry_header));
                jak_memfile_seek(memfile, continue_off);
        }

        jak_offset_t continue_pos = jak_memfile_tell(memfile);
        jak_memfile_seek(memfile, header_pos);
        jak_memfile_write(memfile, &header, sizeof(jak_string_table_header));
        jak_memfile_seek(memfile, continue_pos);

        jak_vector_drop(strings);
        jak_vector_drop(jak_string_ids);
        free(strings);
        free(jak_string_ids);

        return jak_pack_drop(err, &strategy);
}

static void skip_file_header(jak_memfile *memfile)
{
        jak_memfile_skip(memfile, sizeof(jak_archive_header));
}

static void update_file_header(jak_memfile *memfile, jak_offset_t record_header_offset)
{
        jak_offset_t current_pos;
        jak_memfile_get_offset(&current_pos, memfile);
        jak_memfile_seek(memfile, 0);
        memcpy(&this_file_header.magic, JAK_CARBON_ARCHIVE_MAGIC, strlen(JAK_CARBON_ARCHIVE_MAGIC));
        this_file_header.root_object_header_offset = record_header_offset;
        this_file_header.jak_string_id_to_offset_index_offset = 0;
        jak_memfile_write(memfile, &this_file_header, sizeof(jak_archive_header));
        jak_memfile_seek(memfile, current_pos);
}

static bool
print_column_form_memfile(FILE *file, jak_error *err, jak_memfile *memfile, unsigned nesting_level)
{
        jak_offset_t offset;
        jak_memfile_get_offset(&offset, memfile);
        jak_column_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_column_header);
        if (header->marker != JAK_MARKER_SYMBOL_COLUMN) {
                char buffer[256];
                sprintf(buffer, "expected marker [%c] but found [%c]", JAK_MARKER_SYMBOL_COLUMN, header->marker);
                JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                return false;
        }
        fprintf(file, "0x%04x ", (unsigned) offset);
        INTENT_LINE(nesting_level);
        const char *type_name = array_value_type_to_string(err, jak_int_marker_to_field_type(header->value_type));
        if (!type_name) {
                return false;
        }

        fprintf(file,
                "[marker: %c (Column)] [column_name: '%"PRIu64"'] [value_type: %c (%s)] [nentries: %d] [",
                header->marker,
                header->column_name,
                header->value_type,
                type_name,
                header->num_entries);

        for (size_t i = 0; i < header->num_entries; i++) {
                jak_offset_t entry_off = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
                fprintf(file, "offset: 0x%04x%s", (unsigned) entry_off, i + 1 < header->num_entries ? ", " : "");
        }

        jak_u32 *positions = (jak_u32 *) JAK_MEMFILE_READ(memfile, header->num_entries * sizeof(jak_u32));
        fprintf(file, "] [positions: [");
        for (size_t i = 0; i < header->num_entries; i++) {
                fprintf(file, "%d%s", positions[i], i + 1 < header->num_entries ? ", " : "");
        }

        fprintf(file, "]]\n");

        jak_archive_field_e data_type = jak_int_marker_to_field_type(header->value_type);

        //fprintf(file, "[");
        for (size_t i = 0; i < header->num_entries; i++) {
                switch (data_type) {
                        case JAK_FIELD_NULL: {
                                PRINT_VALUE_ARRAY(jak_u32, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_BOOLEAN: {
                                PRINT_VALUE_ARRAY(jak_archive_field_boolean_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_INT8: {
                                PRINT_VALUE_ARRAY(jak_archive_field_i8_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_INT16: {
                                PRINT_VALUE_ARRAY(jak_archive_field_i16_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_INT32: {
                                PRINT_VALUE_ARRAY(jak_archive_field_i32_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_INT64: {
                                PRINT_VALUE_ARRAY(jak_archive_field_i64_t, memfile, header, "%"
                                        PRIi64);
                        }
                                break;
                        case JAK_FIELD_UINT8: {
                                PRINT_VALUE_ARRAY(jak_archive_field_u8_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_UINT16: {
                                PRINT_VALUE_ARRAY(jak_archive_field_u16_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_UINT32: {
                                PRINT_VALUE_ARRAY(jak_archive_field_u32_t, memfile, header, "%d");
                        }
                                break;
                        case JAK_FIELD_UINT64: {
                                PRINT_VALUE_ARRAY(jak_archive_field_u64_t, memfile, header, "%"
                                        PRIu64);
                        }
                                break;
                        case JAK_FIELD_FLOAT: {
                                PRINT_VALUE_ARRAY(jak_archive_field_number_t, memfile, header, "%f");
                        }
                                break;
                        case JAK_FIELD_STRING: {
                                PRINT_VALUE_ARRAY(jak_archive_field_sid_t, memfile, header, "%"
                                        PRIu64
                                        "");
                        }
                                break;
                        case JAK_FIELD_OBJECT: {
                                jak_u32 num_elements = *JAK_MEMFILE_READ_TYPE(memfile, jak_u32);
                                INTENT_LINE(nesting_level);
                                fprintf(file, "   [num_elements: %d] [values: [\n", num_elements);
                                for (size_t i = 0; i < num_elements; i++) {
                                        if (!_jak_archive_print_object(file, err, memfile, nesting_level + 2)) {
                                                return false;
                                        }
                                }
                                INTENT_LINE(nesting_level);
                                fprintf(file, "   ]\n");
                        }
                                break;
                        default: JAK_ERROR(err, JAK_ERR_NOTYPE)
                                return false;
                }
        }
        return true;
}

static bool _jak_archive_print_object_array_from_memfile(FILE *file, jak_error *err, jak_memfile *memfile,
                                            unsigned nesting_level)
{
        unsigned offset = (unsigned) jak_memfile_tell(memfile);
        jak_object_array_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_object_array_header);
        if (header->marker != JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY) {
                char buffer[256];
                sprintf(buffer, "expected marker [%c] but found [%c]", JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY,
                        header->marker);
                JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                return false;
        }

        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nesting_level);
        fprintf(file, "[marker: %c (Object Array)] [nentries: %d] [", header->marker, header->num_entries);

        for (size_t i = 0; i < header->num_entries; i++) {
                jak_archive_field_sid_t jak_string_id = *JAK_MEMFILE_READ_TYPE(memfile, jak_archive_field_sid_t);
                fprintf(file, "key: %"PRIu64"%s", jak_string_id, i + 1 < header->num_entries ? ", " : "");
        }
        fprintf(file, "] [");
        for (size_t i = 0; i < header->num_entries; i++) {
                jak_offset_t columnGroupOffset = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
                fprintf(file,
                        "offset: 0x%04x%s",
                        (unsigned) columnGroupOffset,
                        i + 1 < header->num_entries ? ", " : "");
        }

        fprintf(file, "]\n");
        nesting_level++;

        for (size_t i = 0; i < header->num_entries; i++) {
                offset = jak_memfile_tell(memfile);
                jak_column_group_header
                        *column_group_header = JAK_MEMFILE_READ_TYPE(memfile, jak_column_group_header);
                if (column_group_header->marker != JAK_MARKER_SYMBOL_COLUMN_GROUP) {
                        char buffer[256];
                        sprintf(buffer,
                                "expected marker [%c] but found [%c]",
                                JAK_MARKER_SYMBOL_COLUMN_GROUP,
                                column_group_header->marker);
                        JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                        return false;
                }
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level);
                fprintf(file,
                        "[marker: %c (Column Group)] [num_columns: %d] [num_objects: %d] [object_ids: ",
                        column_group_header->marker,
                        column_group_header->num_columns,
                        column_group_header->num_objects);
                const jak_uid_t
                        *oids = JAK_MEMFILE_READ_TYPE_LIST(memfile, jak_uid_t,
                                                           column_group_header->num_objects);
                for (size_t k = 0; k < column_group_header->num_objects; k++) {
                        fprintf(file, "%"PRIu64"%s", oids[k], k + 1 < column_group_header->num_objects ? ", " : "");
                }
                fprintf(file, "] [offsets: ");
                for (size_t k = 0; k < column_group_header->num_columns; k++) {
                        jak_offset_t column_off = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);
                        fprintf(file,
                                "0x%04x%s",
                                (unsigned) column_off,
                                k + 1 < column_group_header->num_columns ? ", " : "");
                }

                fprintf(file, "]\n");

                for (size_t k = 0; k < column_group_header->num_columns; k++) {
                        if (!print_column_form_memfile(file, err, memfile, nesting_level + 1)) {
                                return false;
                        }
                }

                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level);
                fprintf(file, "]\n");
        }
        return true;
}

static void print_prop_offsets(FILE *file, const jak_object_flags_u *flags,
                               const jak_archive_prop_offs *prop_offsets)
{
        if (flags->bits.has_null_props) {
                fprintf(file, " nulls: 0x%04x", (unsigned) prop_offsets->nulls);
        }
        if (flags->bits.has_bool_props) {
                fprintf(file, " bools: 0x%04x", (unsigned) prop_offsets->bools);
        }
        if (flags->bits.has_int8_props) {
                fprintf(file, " int8s: 0x%04x", (unsigned) prop_offsets->int8s);
        }
        if (flags->bits.has_int16_props) {
                fprintf(file, " int16s: 0x%04x", (unsigned) prop_offsets->int16s);
        }
        if (flags->bits.has_int32_props) {
                fprintf(file, " int32s: 0x%04x", (unsigned) prop_offsets->int32s);
        }
        if (flags->bits.has_int64_props) {
                fprintf(file, " int64s: 0x%04x", (unsigned) prop_offsets->int64s);
        }
        if (flags->bits.has_uint8_props) {
                fprintf(file, " uint8s: 0x%04x", (unsigned) prop_offsets->uint8s);
        }
        if (flags->bits.has_uint16_props) {
                fprintf(file, " uint16s: 0x%04x", (unsigned) prop_offsets->uint16s);
        }
        if (flags->bits.has_uint32_props) {
                fprintf(file, " uint32s: 0x%04x", (unsigned) prop_offsets->uint32s);
        }
        if (flags->bits.has_uint64_props) {
                fprintf(file, " uint64s: 0x%04x", (unsigned) prop_offsets->uint64s);
        }
        if (flags->bits.has_float_props) {
                fprintf(file, " floats: 0x%04x", (unsigned) prop_offsets->floats);
        }
        if (flags->bits.has_jak_string_props) {
                fprintf(file, " texts: 0x%04x", (unsigned) prop_offsets->strings);
        }
        if (flags->bits.has_object_props) {
                fprintf(file, " objects: 0x%04x", (unsigned) prop_offsets->objects);
        }
        if (flags->bits.has_null_array_props) {
                fprintf(file, " nullArrays: 0x%04x", (unsigned) prop_offsets->null_arrays);
        }
        if (flags->bits.has_bool_array_props) {
                fprintf(file, " boolArrays: 0x%04x", (unsigned) prop_offsets->bool_arrays);
        }
        if (flags->bits.has_int8_array_props) {
                fprintf(file, " int8Arrays: 0x%04x", (unsigned) prop_offsets->int8_arrays);
        }
        if (flags->bits.has_int16_array_props) {
                fprintf(file, " int16Arrays: 0x%04x", (unsigned) prop_offsets->int16_arrays);
        }
        if (flags->bits.has_int32_array_props) {
                fprintf(file, " int32Arrays: 0x%04x", (unsigned) prop_offsets->int32_arrays);
        }
        if (flags->bits.has_int64_array_props) {
                fprintf(file, " int16Arrays: 0x%04x", (unsigned) prop_offsets->int64_arrays);
        }
        if (flags->bits.has_uint8_array_props) {
                fprintf(file, " uint8Arrays: 0x%04x", (unsigned) prop_offsets->uint8_arrays);
        }
        if (flags->bits.has_uint16_array_props) {
                fprintf(file, " uint16Arrays: 0x%04x", (unsigned) prop_offsets->uint16_arrays);
        }
        if (flags->bits.has_uint32_array_props) {
                fprintf(file, " uint32Arrays: 0x%04x", (unsigned) prop_offsets->uint32_arrays);
        }
        if (flags->bits.has_uint64_array_props) {
                fprintf(file, " uint64Arrays: 0x%04x", (unsigned) prop_offsets->uint64_arrays);
        }
        if (flags->bits.has_float_array_props) {
                fprintf(file, " floatArrays: 0x%04x", (unsigned) prop_offsets->float_arrays);
        }
        if (flags->bits.has_jak_string_array_props) {
                fprintf(file, " textArrays: 0x%04x", (unsigned) prop_offsets->jak_string_arrays);
        }
        if (flags->bits.has_object_array_props) {
                fprintf(file, " objectArrays: 0x%04x", (unsigned) prop_offsets->object_arrays);
        }
}

bool _jak_archive_print_object(FILE *file, jak_error *err, jak_memfile *memfile, unsigned nesting_level)
{
        unsigned offset = (unsigned) jak_memfile_tell(memfile);
        jak_object_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_object_header);

        jak_archive_prop_offs prop_offsets;
        jak_object_flags_u flags = {.value = header->flags};

        jak_int_read_prop_offsets(&prop_offsets, memfile, &flags);
        jak_offset_t nextObjectOrNil = *JAK_MEMFILE_READ_TYPE(memfile, jak_offset_t);

        if (header->marker != JAK_MARKER_SYMBOL_OBJECT_BEGIN) {
                char buffer[256];
                sprintf(buffer, "Parsing JAK_ERROR: expected object marker [{] but found [%c]\"", header->marker);
                JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                return false;
        }

        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nesting_level);
        nesting_level++;
        fprintf(file,
                "[marker: %c (BeginObject)] [object-id: %"PRIu64"] [flags: %u] [propertyOffsets: [",
                header->marker,
                header->oid,
                header->flags);
        print_prop_offsets(file, &flags, &prop_offsets);
        fprintf(file, " ] [next: 0x%04x] \n", (unsigned) nextObjectOrNil);

        bool continue_read = true;
        while (continue_read) {
                offset = jak_memfile_tell(memfile);
                char entryMarker = *JAK_MEMFILE_PEEK(memfile, char);

                switch (entryMarker) {
                        case JAK_MARKER_SYMBOL_PROP_NULL: {
                                jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile,
                                                                                                jak_prop_header);
                                jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile,
                                                                                                             prop_header->num_entries *
                                                                                                             sizeof(jak_archive_field_sid_t));
                                fprintf(file, "0x%04x ", offset);
                                INTENT_LINE(nesting_level)
                                fprintf(file, "[marker: %c (null)] [nentries: %d] [", entryMarker,
                                        prop_header->num_entries);

                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file, "%"PRIu64"%s", keys[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }
                                fprintf(file, "]\n");
                        }
                                break;
                        case JAK_MARKER_SYMBOL_PROP_BOOLEAN: {
                                jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile,
                                                                                                jak_prop_header);
                                jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile,
                                                                                                             prop_header->num_entries *
                                                                                                             sizeof(jak_archive_field_sid_t));
                                jak_archive_field_boolean_t *values = (jak_archive_field_boolean_t *) JAK_MEMFILE_READ(
                                        memfile,
                                        prop_header->num_entries *
                                        sizeof(jak_archive_field_boolean_t));
                                fprintf(file, "0x%04x ", offset);
                                INTENT_LINE(nesting_level)
                                fprintf(file, "[marker: %c (boolean)] [nentries: %d] [", entryMarker,
                                        prop_header->num_entries);
                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file, "%"PRIu64"%s", keys[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }
                                fprintf(file, "] [");
                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file,
                                                "%s%s",
                                                values[i] ? "true" : "false",
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }
                                fprintf(file, "]\n");
                        }
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT8: PRINT_SIMPLE_PROPS(file,
                                                                             memfile,
                                                                             jak_memfile_tell(memfile),
                                                                             nesting_level,
                                                                             jak_archive_field_i8_t,
                                                                             "Int8",
                                                                             "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT16: PRINT_SIMPLE_PROPS(file,
                                                                              memfile,
                                                                              jak_memfile_tell(memfile),
                                                                              nesting_level,
                                                                              jak_archive_field_i16_t,
                                                                              "Int16",
                                                                              "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT32: PRINT_SIMPLE_PROPS(file,
                                                                              memfile,
                                                                              jak_memfile_tell(memfile),
                                                                              nesting_level,
                                                                              jak_archive_field_i32_t,
                                                                              "Int32",
                                                                              "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT64: PRINT_SIMPLE_PROPS(file,
                                                                              memfile,
                                                                              jak_memfile_tell(memfile),
                                                                              nesting_level,
                                                                              jak_archive_field_i64_t,
                                                                              "Int64",
                                                                              "%"
                                                                                      PRIi64);
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT8: PRINT_SIMPLE_PROPS(file,
                                                                              memfile,
                                                                              jak_memfile_tell(memfile),
                                                                              nesting_level,
                                                                              jak_archive_field_u8_t,
                                                                              "UInt8",
                                                                              "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT16: PRINT_SIMPLE_PROPS(file,
                                                                               memfile,
                                                                               jak_memfile_tell(memfile),
                                                                               nesting_level,
                                                                               jak_archive_field_u16_t,
                                                                               "UInt16",
                                                                               "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT32: PRINT_SIMPLE_PROPS(file,
                                                                               memfile,
                                                                               jak_memfile_tell(memfile),
                                                                               nesting_level,
                                                                               jak_archive_field_u32_t,
                                                                               "UInt32",
                                                                               "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT64: PRINT_SIMPLE_PROPS(file,
                                                                               memfile,
                                                                               jak_memfile_tell(memfile),
                                                                               nesting_level,
                                                                               jak_archive_field_u64_t,
                                                                               "UInt64",
                                                                               "%"
                                                                                       PRIu64);
                                break;
                        case JAK_MARKER_SYMBOL_PROP_REAL: PRINT_SIMPLE_PROPS(file,
                                                                             memfile,
                                                                             jak_memfile_tell(memfile),
                                                                             nesting_level,
                                                                             jak_archive_field_number_t,
                                                                             "Float",
                                                                             "%f");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_TEXT: PRINT_SIMPLE_PROPS(file,
                                                                             memfile,
                                                                             jak_memfile_tell(memfile),
                                                                             nesting_level,
                                                                             jak_archive_field_sid_t,
                                                                             "Text",
                                                                             "%"
                                                                                     PRIu64
                                                                                     "");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_OBJECT: {
                                jak_var_prop prop;
                                jak_int_embedded_var_props_read(&prop, memfile);
                                fprintf(file, "0x%04x ", offset);
                                INTENT_LINE(nesting_level)
                                fprintf(file, "[marker: %c (Object)] [nentries: %d] [", entryMarker,
                                        prop.header->num_entries);
                                for (jak_u32 i = 0; i < prop.header->num_entries; i++) {
                                        fprintf(file,
                                                "key: %"PRIu64"%s",
                                                prop.keys[i],
                                                i + 1 < prop.header->num_entries ? ", " : "");
                                }
                                fprintf(file, "] [");
                                for (jak_u32 i = 0; i < prop.header->num_entries; i++) {
                                        fprintf(file,
                                                "offsets: 0x%04x%s",
                                                (unsigned) prop.offsets[i],
                                                i + 1 < prop.header->num_entries ? ", " : "");
                                }
                                fprintf(file, "] [\n");

                                char nextEntryMarker;
                                do {
                                        if (!_jak_archive_print_object(file, err, memfile, nesting_level + 1)) {
                                                return false;
                                        }
                                        nextEntryMarker = *JAK_MEMFILE_PEEK(memfile, char);
                                } while (nextEntryMarker == JAK_MARKER_SYMBOL_OBJECT_BEGIN);

                        }
                                break;
                        case JAK_MARKER_SYMBOL_PROP_NULL_ARRAY: {
                                jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile,
                                                                                                jak_prop_header);

                                jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile,
                                                                                                             prop_header->num_entries *
                                                                                                             sizeof(jak_archive_field_sid_t));
                                jak_u32 *nullArrayLengths;

                                fprintf(file, "0x%04x ", offset);
                                INTENT_LINE(nesting_level)
                                fprintf(file,
                                        "[marker: %c (Null Array)] [nentries: %d] [",
                                        entryMarker,
                                        prop_header->num_entries);

                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file, "%"PRIu64"%s", keys[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }
                                fprintf(file, "] [");

                                nullArrayLengths = (jak_u32 *) JAK_MEMFILE_READ(memfile,
                                                                                prop_header->num_entries *
                                                                                sizeof(jak_u32));

                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file,
                                                "nentries: %d%s",
                                                nullArrayLengths[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }

                                fprintf(file, "]\n");
                        }
                                break;
                        case JAK_MARKER_SYMBOL_PROP_BOOLEAN_ARRAY: {
                                jak_prop_header *prop_header = JAK_MEMFILE_READ_TYPE(memfile,
                                                                                                jak_prop_header);

                                jak_archive_field_sid_t *keys = (jak_archive_field_sid_t *) JAK_MEMFILE_READ(memfile,
                                                                                                             prop_header->num_entries *
                                                                                                             sizeof(jak_archive_field_sid_t));
                                jak_u32 *array_lengths;

                                fprintf(file, "0x%04x ", offset);
                                INTENT_LINE(nesting_level)
                                fprintf(file,
                                        "[marker: %c (Boolean Array)] [nentries: %d] [",
                                        entryMarker,
                                        prop_header->num_entries);

                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file, "%"PRIu64"%s", keys[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }
                                fprintf(file, "] [");

                                array_lengths = (jak_u32 *) JAK_MEMFILE_READ(memfile,
                                                                             prop_header->num_entries *
                                                                             sizeof(jak_u32));

                                for (jak_u32 i = 0; i < prop_header->num_entries; i++) {
                                        fprintf(file,
                                                "arrayLength: %d%s",
                                                array_lengths[i],
                                                i + 1 < prop_header->num_entries ? ", " : "");
                                }

                                fprintf(file, "] [");

                                for (jak_u32 array_idx = 0; array_idx < prop_header->num_entries; array_idx++) {
                                        jak_archive_field_boolean_t *values = (jak_archive_field_boolean_t *) JAK_MEMFILE_READ(
                                                memfile,
                                                array_lengths[array_idx] *
                                                sizeof(jak_archive_field_boolean_t));
                                        fprintf(file, "[");
                                        for (jak_u32 i = 0; i < array_lengths[array_idx]; i++) {
                                                fprintf(file,
                                                        "value: %s%s",
                                                        values[i] ? "true" : "false",
                                                        i + 1 < array_lengths[array_idx] ? ", " : "");
                                        }
                                        fprintf(file, "]%s", array_idx + 1 < prop_header->num_entries ? ", " : "");
                                }

                                fprintf(file, "]\n");
                        }
                                break;
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT8_ARRAY: {
                                PRINT_ARRAY_PROPS(memfile,
                                                  jak_memfile_tell(memfile),
                                                  nesting_level,
                                                  entryMarker,
                                                  jak_archive_field_i8_t,
                                                  "Int8 Array",
                                                  "%d");
                        }
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT16_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                   jak_memfile_tell(memfile),
                                                                                   nesting_level,
                                                                                   entryMarker,
                                                                                   jak_archive_field_i16_t,
                                                                                   "Int16 Array",
                                                                                   "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT32_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                   jak_memfile_tell(memfile),
                                                                                   nesting_level,
                                                                                   entryMarker,
                                                                                   jak_archive_field_i32_t,
                                                                                   "Int32 Array",
                                                                                   "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_INT64_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                   jak_memfile_tell(memfile),
                                                                                   nesting_level,
                                                                                   entryMarker,
                                                                                   jak_archive_field_i64_t,
                                                                                   "Int64 Array",
                                                                                   "%"
                                                                                           PRIi64);
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT8_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                   jak_memfile_tell(memfile),
                                                                                   nesting_level,
                                                                                   entryMarker,
                                                                                   jak_archive_field_u8_t,
                                                                                   "UInt8 Array",
                                                                                   "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT16_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                    jak_memfile_tell(memfile),
                                                                                    nesting_level,
                                                                                    entryMarker,
                                                                                    jak_archive_field_u16_t,
                                                                                    "UInt16 Array",
                                                                                    "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT32_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                    jak_memfile_tell(memfile),
                                                                                    nesting_level,
                                                                                    entryMarker,
                                                                                    jak_archive_field_u32_t,
                                                                                    "UInt32 Array",
                                                                                    "%d");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_UINT64_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                    jak_memfile_tell(memfile),
                                                                                    nesting_level,
                                                                                    entryMarker,
                                                                                    jak_archive_field_u64_t,
                                                                                    "UInt64 Array",
                                                                                    "%"
                                                                                            PRIu64);
                                break;
                        case JAK_MARKER_SYMBOL_PROP_REAL_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                  jak_memfile_tell(memfile),
                                                                                  nesting_level,
                                                                                  entryMarker,
                                                                                  jak_archive_field_number_t,
                                                                                  "Float Array",
                                                                                  "%f");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_TEXT_ARRAY: PRINT_ARRAY_PROPS(memfile,
                                                                                  jak_memfile_tell(memfile),
                                                                                  nesting_level,
                                                                                  entryMarker,
                                                                                  jak_archive_field_sid_t,
                                                                                  "Text Array",
                                                                                  "%"
                                                                                          PRIu64
                                                                                          "");
                                break;
                        case JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY:
                                if (!_jak_archive_print_object_array_from_memfile(file, err, memfile, nesting_level)) {
                                        return false;
                                }
                                break;
                        case JAK_MARKER_SYMBOL_OBJECT_END:
                                continue_read = false;
                                break;
                        default: {
                                char buffer[256];
                                sprintf(buffer,
                                        "Parsing JAK_ERROR: unexpected marker [%c] was detected in file %p",
                                        entryMarker,
                                        memfile);
                                JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                                return false;
                        }
                }
        }

        offset = jak_memfile_tell(memfile);
        char end_marker = *JAK_MEMFILE_READ_TYPE(memfile, char);
        JAK_ASSERT (end_marker == JAK_MARKER_SYMBOL_OBJECT_END);
        nesting_level--;
        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nesting_level);
        fprintf(file, "[marker: %c (EndObject)]\n", end_marker);
        return true;
}

static bool is_valid_file(const jak_archive_header *header)
{
        if (JAK_ARRAY_LENGTH(header->magic) != strlen(JAK_CARBON_ARCHIVE_MAGIC)) {
                return false;
        } else {
                for (size_t i = 0; i < JAK_ARRAY_LENGTH(header->magic); i++) {
                        if (header->magic[i] != JAK_CARBON_ARCHIVE_MAGIC[i]) {
                                return false;
                        }
                }
                if (header->version != JAK_CARBON_ARCHIVE_VERSION) {
                        return false;
                }
                if (header->root_object_header_offset == 0) {
                        return false;
                }
                return true;
        }
}

static void print_record_header_from_memfile(FILE *file, jak_memfile *memfile)
{
        unsigned offset = jak_memfile_tell(memfile);
        jak_record_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_record_header);
        jak_record_flags flags;
        memset(&flags, 0, sizeof(jak_record_flags));
        flags.value = header->flags;
        char *flags_string = record_header_flags_to_string(&flags);
        fprintf(file, "0x%04x ", offset);
        fprintf(file,
                "[marker: %c] [flags: %s] [record_size: 0x%04x]\n",
                header->marker,
                flags_string,
                (unsigned) header->record_size);
        free(flags_string);
}

static bool print_header_from_memfile(FILE *file, jak_error *err, jak_memfile *memfile)
{
        unsigned offset = jak_memfile_tell(memfile);
        JAK_ASSERT(jak_memfile_size(memfile) > sizeof(jak_archive_header));
        jak_archive_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_archive_header);
        if (!is_valid_file(header)) {
                JAK_ERROR(err, JAK_ERR_NOARCHIVEFILE)
                return false;
        }

        fprintf(file, "0x%04x ", offset);
        fprintf(file,
                "[magic: " JAK_CARBON_ARCHIVE_MAGIC "] [version: %d] [recordOffset: 0x%04x] [string-id-offset-index: 0x%04x]\n",
                header->version,
                (unsigned) header->root_object_header_offset,
                (unsigned) header->jak_string_id_to_offset_index_offset);
        return true;
}

static bool print_embedded_dic_from_memfile(FILE *file, jak_error *err, jak_memfile *memfile)
{
        jak_packer strategy;
        jak_string_tab_flags_u flags;

        unsigned offset = jak_memfile_tell(memfile);
        jak_string_table_header *header = JAK_MEMFILE_READ_TYPE(memfile, jak_string_table_header);
        if (header->marker != jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
                char buffer[256];
                sprintf(buffer,
                        "expected [%c] marker, but found [%c]",
                        jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_STR_DIC].symbol,
                        header->marker);
                JAK_ERROR_WDETAILS(err, JAK_ERR_CORRUPTED, buffer);
                return false;
        }
        flags.value = header->flags;

        char *flagsStr = embedded_dic_flags_to_string(&flags);
        fprintf(file, "0x%04x ", offset);
        fprintf(file,
                "[marker: %c] [nentries: %d] [flags: %s] [first-entry-off: 0x%04x] [extra-size: %" PRIu64 "]\n",
                header->marker,
                header->num_entries,
                flagsStr,
                (unsigned) header->first_entry,
                header->compressor_extra_size);
        free(flagsStr);

        if (jak_pack_by_flags(&strategy, flags.value) != true) {
                JAK_ERROR(err, JAK_ERR_NOCOMPRESSOR);
                return false;
        }

        jak_pack_print_extra(err, &strategy, file, memfile);

        while ((*JAK_MEMFILE_PEEK(memfile, char)) == jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol) {
                unsigned offset = jak_memfile_tell(memfile);
                jak_string_entry_header header = *JAK_MEMFILE_READ_TYPE(memfile, jak_string_entry_header);
                fprintf(file,
                        "0x%04x    [marker: %c] [next-entry-off: 0x%04zx] [string-id: %"PRIu64"] [string-length: %"PRIu32"]",
                        offset,
                        header.marker,
                        (size_t) header.next_entry_off,
                        header.jak_string_id,
                        header.jak_string_len);
                jak_pack_print_encoded(err, &strategy, file, memfile, header.jak_string_len);
                fprintf(file, "\n");
        }

        return jak_pack_drop(err, &strategy);
}

static bool print_archive_from_memfile(FILE *file, jak_error *err, jak_memfile *memfile)
{
        if (!print_header_from_memfile(file, err, memfile)) {
                return false;
        }
        if (!print_embedded_dic_from_memfile(file, err, memfile)) {
                return false;
        }
        print_record_header_from_memfile(file, memfile);
        if (!_jak_archive_print_object(file, err, memfile, 0)) {
                return false;
        }
        return true;
}

static jak_object_flags_u *get_flags(jak_object_flags_u *flags, jak_column_doc_obj *columndoc)
{
        JAK_ZERO_MEMORY(flags, sizeof(jak_object_flags_u));
        flags->bits.has_null_props = (columndoc->null_prop_keys.num_elems > 0);
        flags->bits.has_bool_props = (columndoc->bool_prop_keys.num_elems > 0);
        flags->bits.has_int8_props = (columndoc->int8_prop_keys.num_elems > 0);
        flags->bits.has_int16_props = (columndoc->int16_prop_keys.num_elems > 0);
        flags->bits.has_int32_props = (columndoc->int32_prop_keys.num_elems > 0);
        flags->bits.has_int64_props = (columndoc->int64_prop_keys.num_elems > 0);
        flags->bits.has_uint8_props = (columndoc->uint8_prop_keys.num_elems > 0);
        flags->bits.has_uint16_props = (columndoc->uint16_prop_keys.num_elems > 0);
        flags->bits.has_uint32_props = (columndoc->uin32_prop_keys.num_elems > 0);
        flags->bits.has_uint64_props = (columndoc->uint64_prop_keys.num_elems > 0);
        flags->bits.has_float_props = (columndoc->float_prop_keys.num_elems > 0);
        flags->bits.has_jak_string_props = (columndoc->jak_string_prop_keys.num_elems > 0);
        flags->bits.has_object_props = (columndoc->obj_prop_keys.num_elems > 0);
        flags->bits.has_null_array_props = (columndoc->null_array_prop_keys.num_elems > 0);
        flags->bits.has_bool_array_props = (columndoc->bool_array_prop_keys.num_elems > 0);
        flags->bits.has_int8_array_props = (columndoc->int8_array_prop_keys.num_elems > 0);
        flags->bits.has_int16_array_props = (columndoc->int16_array_prop_keys.num_elems > 0);
        flags->bits.has_int32_array_props = (columndoc->int32_array_prop_keys.num_elems > 0);
        flags->bits.has_int64_array_props = (columndoc->int64_array_prop_keys.num_elems > 0);
        flags->bits.has_uint8_array_props = (columndoc->uint8_array_prop_keys.num_elems > 0);
        flags->bits.has_uint16_array_props = (columndoc->uint16_array_prop_keys.num_elems > 0);
        flags->bits.has_uint32_array_props = (columndoc->uint32_array_prop_keys.num_elems > 0);
        flags->bits.has_uint64_array_props = (columndoc->uint64_array_prop_keys.num_elems > 0);
        flags->bits.has_float_array_props = (columndoc->float_array_prop_keys.num_elems > 0);
        flags->bits.has_jak_string_array_props = (columndoc->jak_string_array_prop_keys.num_elems > 0);
        flags->bits.has_object_array_props = (columndoc->obj_array_props.num_elems > 0);
        //JAK_ASSERT(flags->value != 0);
        return flags;
}

static bool init_decompressor(jak_packer *strategy, jak_u8 flags);

static bool read_stringtable(jak_string_table *table, jak_error *err, FILE *disk_file);

static bool read_record(jak_record_header *header_read, jak_archive *archive, FILE *disk_file,
                        jak_offset_t record_header_offset);

static bool read_jak_string_id_to_offset_index(jak_error *err, jak_archive *archive, const char *file_path,
                                           jak_offset_t jak_string_id_to_offset_index_offset);

bool jak_archive_open(jak_archive *out, const char *file_path)
{
        int status;
        FILE *disk_file;

        jak_error_init(&out->err);
        out->disk_file_path = strdup(file_path);
        disk_file = fopen(out->disk_file_path, "r");
        if (!disk_file) {
                jak_string sb;
                char cwd[PATH_MAX];
                jak_string_create(&sb);

                jak_string_add(&sb, "File '");
                jak_string_add(&sb, file_path);
                jak_string_add(&sb, "' not found in current working directory ('");
                jak_string_add(&sb, getcwd(cwd, sizeof(cwd)));
                jak_string_add(&sb, "')");
                JAK_ERROR_WDETAILS(&out->err, JAK_ERR_FOPEN_FAILED, jak_string_cstr(&sb));
                jak_string_drop(&sb);
                return false;
        } else {
                jak_archive_header header;
                size_t nread = fread(&header, sizeof(jak_archive_header), 1, disk_file);
                if (nread != 1) {
                        fclose(disk_file);
                        JAK_ERROR_PRINT(JAK_ERR_IO);
                        return false;
                } else {
                        if (!is_valid_file(&header)) {
                                JAK_ERROR_PRINT(JAK_ERR_FORMATVERERR);
                                return false;
                        } else {
                                out->query_index_jak_string_id_to_offset = NULL;
                                out->jak_string_id_cache = NULL;

                                jak_record_header jak_record_header;

                                if ((status = read_stringtable(&out->jak_string_table, &out->err, disk_file)) != true) {
                                        return status;
                                }
                                if ((status = read_record(&jak_record_header,
                                                          out,
                                                          disk_file,
                                                          header.root_object_header_offset)) != true) {
                                        return status;
                                }

                                if (header.jak_string_id_to_offset_index_offset != 0) {
                                        jak_error err;
                                        if ((status = read_jak_string_id_to_offset_index(&err,
                                                                                     out,
                                                                                     file_path,
                                                                                     header.jak_string_id_to_offset_index_offset)) !=
                                            true) {
                                                JAK_ERROR_PRINT(err.code);
                                                return status;
                                        }
                                }

                                fseek(disk_file, sizeof(jak_archive_header), SEEK_SET);

                                jak_offset_t data_start = ftell(disk_file);
                                fseek(disk_file, 0, SEEK_END);
                                jak_offset_t file_size = ftell(disk_file);

                                fclose(disk_file);

                                size_t jak_string_table_size = header.root_object_header_offset - data_start;
                                size_t record_table_size = jak_record_header.record_size;
                                size_t jak_string_id_index = file_size - header.jak_string_id_to_offset_index_offset;

                                out->info.jak_string_table_size = jak_string_table_size;
                                out->info.record_table_size = record_table_size;
                                out->info.num_embeddded_strings = out->jak_string_table.num_embeddded_strings;
                                out->info.jak_string_id_index_size = jak_string_id_index;
                                out->default_query = JAK_MALLOC(sizeof(jak_archive_query));
                                jak_query_create(out->default_query, out);

                        }
                }
        }

        return true;
}

bool jak_archive_get_info(jak_archive_info *info, const jak_archive *archive)
{
        JAK_ERROR_IF_NULL(info);
        JAK_ERROR_IF_NULL(archive);
        *info = archive->info;
        return true;
}

bool jak_archive_close(jak_archive *archive)
{
        JAK_ERROR_IF_NULL(archive);
        jak_archive_drop_indexes(archive);
        jak_archive_drop_query_jak_string_id_cache(archive);
        free(archive->disk_file_path);
        jak_memblock_drop(archive->record_table.record_db);
        jak_query_drop(archive->default_query);
        free(archive->default_query);
        return true;
}

bool jak_archive_drop_indexes(jak_archive *archive)
{
        if (archive->query_index_jak_string_id_to_offset) {
                jak_query_drop_index_jak_string_id_to_offset(archive->query_index_jak_string_id_to_offset);
                archive->query_index_jak_string_id_to_offset = NULL;
        }
        return true;
}

bool jak_archive_query_run(jak_archive_query *query, jak_archive *archive)
{
        if (jak_query_create(query, archive)) {
                bool has_index = false;
                jak_archive_has_query_index_jak_string_id_to_offset(&has_index, archive);
                if (!has_index) {
                        jak_query_create_index_jak_string_id_to_offset(&archive->query_index_jak_string_id_to_offset, query);
                }
                bool has_cache = false;
                jak_archive_hash_query_jak_string_id_cache(&has_cache, archive);
                if (!has_cache) {
                        jak_string_id_cache_create_lru(&archive->jak_string_id_cache, archive);
                }
                return true;
        } else {
                return false;
        }
}

bool jak_archive_has_query_index_jak_string_id_to_offset(bool *state, jak_archive *archive)
{
        JAK_ERROR_IF_NULL(state)
        JAK_ERROR_IF_NULL(archive)
        *state = (archive->query_index_jak_string_id_to_offset != NULL);
        return true;
}

bool jak_archive_hash_query_jak_string_id_cache(bool *has_cache, jak_archive *archive)
{
        JAK_ERROR_IF_NULL(has_cache)
        JAK_ERROR_IF_NULL(archive)
        *has_cache = archive->jak_string_id_cache != NULL;
        return true;
}

bool jak_archive_drop_query_jak_string_id_cache(jak_archive *archive)
{
        JAK_ERROR_IF_NULL(archive)
        if (archive->jak_string_id_cache) {
                jak_string_id_cache_drop(archive->jak_string_id_cache);
                archive->jak_string_id_cache = NULL;
        }
        return true;
}

struct jak_string_cache *jak_archive_get_query_jak_string_id_cache(jak_archive *archive)
{
        return archive->jak_string_id_cache;
}

jak_archive_query *jak_archive_query_default(jak_archive *archive)
{
        return archive ? archive->default_query : NULL;
}

static bool init_decompressor(jak_packer *strategy, jak_u8 flags)
{
        if (jak_pack_by_flags(strategy, flags) != true) {
                return false;
        }
        return true;
}

static bool read_stringtable(jak_string_table *table, jak_error *err, FILE *disk_file)
{
        JAK_ASSERT(disk_file);

        jak_string_table_header header;
        jak_string_tab_flags_u flags;

        size_t num_read = fread(&header, sizeof(jak_string_table_header), 1, disk_file);
        if (num_read != 1) {
                JAK_ERROR(err, JAK_ERR_IO);
                return false;
        }
        if (header.marker != jak_global_marker_symbols[JAK_MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
                JAK_ERROR(err, JAK_ERR_CORRUPTED);
                return false;
        }

        flags.value = header.flags;
        table->first_entry_off = header.first_entry;
        table->num_embeddded_strings = header.num_entries;

        if ((init_decompressor(&table->compressor, flags.value)) != true) {
                return false;
        }
        if ((jak_pack_read_extra(err, &table->compressor, disk_file, header.compressor_extra_size)) != true) {
                return false;
        }
        return true;
}

static bool read_record(jak_record_header *header_read, jak_archive *archive, FILE *disk_file,
                        jak_offset_t record_header_offset)
{
        jak_error err;
        fseek(disk_file, record_header_offset, SEEK_SET);
        jak_record_header header;
        if (fread(&header, sizeof(jak_record_header), 1, disk_file) != 1) {
                JAK_ERROR(&archive->err, JAK_ERR_CORRUPTED);
                return false;
        } else {
                archive->record_table.flags.value = header.flags;
                bool status = jak_memblock_from_file(&archive->record_table.record_db, disk_file, header.record_size);
                if (!status) {
                        jak_memblock_get_error(&err, archive->record_table.record_db);
                        jak_error_cpy(&archive->err, &err);
                        return false;
                }

                jak_memfile memfile;
                if (jak_memfile_open(&memfile, archive->record_table.record_db, JAK_READ_ONLY) != true) {
                        JAK_ERROR(&archive->err, JAK_ERR_CORRUPTED);
                        status = false;
                }
                if (*JAK_MEMFILE_PEEK(&memfile, char) != JAK_MARKER_SYMBOL_OBJECT_BEGIN) {
                        JAK_ERROR(&archive->err, JAK_ERR_CORRUPTED);
                        status = false;
                }

                *header_read = header;
                return true;
        }
}

static bool read_jak_string_id_to_offset_index(jak_error *err, jak_archive *archive, const char *file_path,
                                           jak_offset_t jak_string_id_to_offset_index_offset)
{
        return jak_query_index_id_to_offset_deserialize(&archive->query_index_jak_string_id_to_offset,
                                                        err,
                                                        file_path,
                                                        jak_string_id_to_offset_index_offset);
}







