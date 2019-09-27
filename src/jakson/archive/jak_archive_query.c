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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/archive/jak_archive_int.h>
#include <jakson/archive/jak_archive_pred.h>
#include <jakson/archive/jak_archive_cache.h>
#include <jakson/archive/jak_archive_query.h>

struct jak_sid_to_offset_arg {
        jak_offset_t offset;
        jak_u32 strlen;
};

struct jak_sid_to_offset {
        jak_hashtable ofMapping(jak_archive_field_sid_t, struct jak_sid_to_offset_arg) mapping;
        FILE *disk_file;
        size_t disk_file_size;

};

#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(num_pairs, obj, bit_flag_name, offset_name)                                \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        JAK_ERROR_PRINT_AND_DIE(JAK_ERR_NULLPTR)                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_name) {                                                                               \
        JAK_ASSERT(obj->props.offset_name != 0);                                                                           \
        jak_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        jak_fixed_prop prop;                                                                                      \
        jak_int_embedded_fixed_props_read(&prop, &obj->file);                                                       \
        jak_archive_int_reset_carbon_object_mem_file(obj);                                                                   \
        JAK_OPTIONAL_SET(num_pairs, prop.header->num_entries);                                                      \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        JAK_OPTIONAL_SET(num_pairs, 0);                                                                             \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

bool jak_query_create(jak_archive_query *query, jak_archive *archive)
{
        JAK_ERROR_IF_NULL(query)
        JAK_ERROR_IF_NULL(archive)
        query->archive = archive;
        query->context = jak_archive_io_context_create(archive);
        jak_error_init(&query->err);
        return query->context != NULL;
}

bool jak_query_drop(jak_archive_query *query)
{
        JAK_ERROR_IF_NULL(query)
        return jak_io_context_drop(query->context);
}

bool jak_query_scan_strids(jak_strid_iter *it, jak_archive_query *query)
{
        JAK_ERROR_IF_NULL(it)
        JAK_ERROR_IF_NULL(query)
        return jak_strid_iter_open(it, &query->err, query->archive);
}

static bool
index_jak_string_id_to_offset_open_file(struct jak_sid_to_offset *index, jak_error *err, const char *file)
{
        index->disk_file = fopen(file, "r");
        if (!index->disk_file) {
                JAK_ERROR(err, JAK_ERR_FOPEN_FAILED)
                return false;
        } else {
                fseek(index->disk_file, 0, SEEK_END);
                index->disk_file_size = ftell(index->disk_file);
                fseek(index->disk_file, 0, SEEK_SET);
                return true;
        }
}

bool jak_query_create_index_jak_string_id_to_offset(struct jak_sid_to_offset **index, jak_archive_query *query)
{
        JAK_ERROR_IF_NULL(index)
        JAK_ERROR_IF_NULL(query)

        jak_strid_iter strid_iter;
        jak_strid_info *info;
        size_t jak_vector_len;
        bool status;
        bool success;
        jak_u32 capacity;
        jak_archive_info archive_info;
        jak_archive_get_info(&archive_info, query->archive);
        capacity = archive_info.num_embeddded_strings;

        struct jak_sid_to_offset *result = JAK_MALLOC(sizeof(struct jak_sid_to_offset));
        jak_hashtable_create(&result->mapping,
                         &query->err,
                         sizeof(jak_archive_field_sid_t),
                         sizeof(struct jak_sid_to_offset_arg),
                         capacity);

        if (!index_jak_string_id_to_offset_open_file(result, &query->err, query->archive->disk_file_path)) {
                return false;
        }

        status = jak_query_scan_strids(&strid_iter, query);

        if (status) {
                while (jak_strid_iter_next(&success, &info, &query->err, &jak_vector_len, &strid_iter)) {
                        for (size_t i = 0; i < jak_vector_len; i++) {
                                struct jak_sid_to_offset_arg arg = {.offset = info[i].offset, .strlen = info[i].strlen};
                                jak_hashtable_insert_or_update(&result->mapping, &info[i].id, &arg, 1);
                        }
                }
                *index = result;
                jak_strid_iter_close(&strid_iter);
                return true;
        } else {
                JAK_ERROR(&query->err, JAK_ERR_SCAN_FAILED);
                return false;
        }

}

void jak_query_drop_index_jak_string_id_to_offset(struct jak_sid_to_offset *index)
{
        if (index) {
                jak_hashtable_drop(&index->mapping);
                fclose(index->disk_file);
                free(index);
        }
}

bool jak_query_index_id_to_offset_serialize(FILE *file, jak_error *err, struct jak_sid_to_offset *index)
{
        JAK_UNUSED(file);
        JAK_UNUSED(err);
        JAK_UNUSED(index);
        return jak_hashtable_serialize(file, &index->mapping);
}

bool jak_query_index_id_to_offset_deserialize(struct jak_sid_to_offset **index, jak_error *err,
                                              const char *file_path, jak_offset_t offset)
{
        JAK_ERROR_IF_NULL(index)
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(file_path)
        JAK_ERROR_IF_NULL(offset)

        struct jak_sid_to_offset *result = JAK_MALLOC(sizeof(struct jak_sid_to_offset));
        if (!result) {
                JAK_ERROR(err, JAK_ERR_MALLOCERR);
                return false;
        }

        if (!index_jak_string_id_to_offset_open_file(result, err, file_path)) {
                return false;
        }

        FILE *index_reader_file = fopen(file_path, "r");
        if (!index_reader_file) {
                JAK_ERROR(err, JAK_ERR_FOPEN_FAILED)
                return false;
        } else {
                fseek(index_reader_file, 0, SEEK_END);
                jak_offset_t file_size = ftell(index_reader_file);

                if (offset >= file_size) {
                        JAK_ERROR(err, JAK_ERR_INTERNALERR)
                        return false;
                }

                fseek(index_reader_file, offset, SEEK_SET);

                if (!jak_hashtable_deserialize(&result->mapping, err, index_reader_file)) {
                        JAK_ERROR(err, JAK_ERR_HASTABLE_DESERIALERR);
                        fclose(index_reader_file);
                        *index = NULL;
                        return false;
                }

                fclose(index_reader_file);
                *index = result;
                return true;
        }
}

static char *fetch_jak_string_from_file(bool *decode_success, FILE *disk_file, size_t offset, size_t jak_string_len,
                                    jak_error *err, jak_archive *archive)
{
        char *result = JAK_MALLOC(jak_string_len + 1);
        memset(result, 0, jak_string_len + 1);

        fseek(disk_file, offset, SEEK_SET);

        bool decode_result = jak_pack_decode(err, &archive->jak_string_table.compressor, result, jak_string_len, disk_file);

        *decode_success = decode_result;
        return result;
}

static char *fetch_jak_string_by_id_via_scan(jak_archive_query *query, jak_archive_field_sid_t id)
{
        JAK_ASSERT(query);

        jak_strid_iter strid_iter;
        jak_strid_info *info;
        size_t jak_vector_len;
        bool status;
        bool success;

        status = jak_query_scan_strids(&strid_iter, query);

        if (status) {
                while (jak_strid_iter_next(&success, &info, &query->err, &jak_vector_len, &strid_iter)) {
                        for (size_t i = 0; i < jak_vector_len; i++) {
                                if (info[i].id == id) {
                                        bool decode_result;
                                        char *result = fetch_jak_string_from_file(&decode_result,
                                                                              strid_iter.disk_file,
                                                                              info[i].offset,
                                                                              info[i].strlen,
                                                                              &query->err,
                                                                              query->archive);

                                        bool close_iter_result = jak_strid_iter_close(&strid_iter);

                                        if (!success || !close_iter_result) {
                                                if (result) {
                                                        free(result);
                                                }
                                                JAK_ERROR(&query->err,
                                                      !decode_result ? JAK_ERR_DECOMPRESSFAILED
                                                                     : JAK_ERR_ITERATORNOTCLOSED);
                                                return NULL;
                                        } else {
                                                return result;
                                        }
                                }
                        }
                }
                jak_strid_iter_close(&strid_iter);
                JAK_ERROR(&query->err, JAK_ERR_NOTFOUND);
                return NULL;
        } else {
                JAK_ERROR(&query->err, JAK_ERR_SCAN_FAILED);
                return NULL;
        }
}

static char *fetch_jak_string_by_id_via_index(jak_archive_query *query, struct jak_sid_to_offset *index,
                                          jak_archive_field_sid_t id)
{
        const struct jak_sid_to_offset_arg *args = jak_hashtable_get_value(&index->mapping, &id);
        if (args) {
                if (args->offset < index->disk_file_size) {
                        bool decode_result;
                        char *result = fetch_jak_string_from_file(&decode_result,
                                                              index->disk_file,
                                                              args->offset,
                                                              args->strlen,
                                                              &query->err,
                                                              query->archive);
                        if (decode_result) {
                                return result;
                        } else {
                                JAK_ERROR(&query->err, JAK_ERR_DECOMPRESSFAILED);
                                return NULL;
                        }

                } else {
                        JAK_ERROR(&query->err, JAK_ERR_INDEXCORRUPTED_OFFSET);
                        return NULL;
                }
        } else {
                JAK_ERROR(&query->err, JAK_ERR_NOTFOUND);
                return NULL;
        }
}

char *jak_query_fetch_jak_string_by_id(jak_archive_query *query, jak_archive_field_sid_t id)
{
        JAK_ASSERT(query);

        bool has_cache = false;
        jak_archive_hash_query_jak_string_id_cache(&has_cache, query->archive);
        if (has_cache) {
                return jak_string_id_cache_get(query->archive->jak_string_id_cache, id);
        } else {
                return jak_query_fetch_jak_string_by_id_nocache(query, id);
        }
}

char *jak_query_fetch_jak_string_by_id_nocache(jak_archive_query *query, jak_archive_field_sid_t id)
{
        bool has_index = false;
        jak_archive_has_query_index_jak_string_id_to_offset(&has_index, query->archive);
        if (has_index) {
                return fetch_jak_string_by_id_via_index(query, query->archive->query_index_jak_string_id_to_offset, id);
        } else {
                return fetch_jak_string_by_id_via_scan(query, id);
        }
}

char **jak_query_fetch_strings_by_offset(jak_archive_query *query, jak_offset_t *offs, jak_u32 *strlens,
                                         size_t num_offs)
{
        JAK_ASSERT(query);
        JAK_ASSERT(offs);
        JAK_ASSERT(strlens);

        FILE *file;

        if (num_offs == 0) {
                return NULL;
        }

        char **result = JAK_MALLOC(num_offs * sizeof(char *));
        if (!result) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                return NULL;
        }
        for (size_t i = 0; i < num_offs; i++) {
                if ((result[i] = JAK_MALLOC((strlens[i] + 1) * sizeof(char))) == NULL) {
                        for (size_t k = 0; k < i; k++) {
                                free(result[k]);
                        }
                        free(result);
                        return NULL;
                }
                memset(result[i], 0, (strlens[i] + 1) * sizeof(char));
        }

        if (!result) {
                JAK_ERROR(jak_io_context_get_error(query->context), JAK_ERR_MALLOCERR);
                return NULL;
        } else {
                if (!(file = jak_io_context_lock_and_access(query->context))) {
                        jak_error_cpy(&query->err, jak_io_context_get_error(query->context));
                        goto cleanup_and_error;
                }

                for (size_t i = 0; i < num_offs; i++) {
                        fseek(file, offs[i], SEEK_SET);
                        if (!jak_pack_decode(&query->err,
                                         &query->archive->jak_string_table.compressor,
                                         result[i],
                                         strlens[i],
                                         file)) {
                                jak_io_context_unlock(query->context);
                                goto cleanup_and_error;
                        }
                }
                jak_io_context_unlock(query->context);
                return result;
        }

        cleanup_and_error:
        for (size_t i = 0; i < num_offs; i++) {
                free(result[i]);
        }
        free(result);
        return NULL;
}

jak_archive_field_sid_t *jak_query_find_ids(size_t *num_found, jak_archive_query *query,
                                            const jak_string_pred *pred, void *capture, jak_i64 limit)
{
        if (JAK_UNLIKELY(jak_string_pred_validate(&query->err, pred) == false)) {
                return NULL;
        }
        jak_i64 pred_limit;
        jak_string_pred_get_limit(&pred_limit, pred);
        pred_limit = pred_limit < 0 ? limit : JAK_MIN(pred_limit, limit);

        jak_strid_iter it;
        jak_strid_info *info = NULL;
        size_t info_len = 0;
        size_t step_len = 0;
        jak_offset_t *str_offs = NULL;
        jak_u32 *str_lens = NULL;
        size_t *idxs_matching = NULL;
        size_t num_matching = 0;
        void *tmp = NULL;
        size_t str_cap = 1024;
        jak_archive_field_sid_t *step_ids = NULL;
        jak_archive_field_sid_t *result_ids = NULL;
        size_t result_len = 0;
        size_t result_cap = pred_limit < 0 ? str_cap : (size_t) pred_limit;
        bool success = false;

        if (JAK_UNLIKELY(pred_limit == 0)) {
                *num_found = 0;
                return NULL;
        }

        if (JAK_UNLIKELY(!num_found || !query || !pred)) {
                JAK_ERROR(&query->err, JAK_ERR_NULLPTR);
                return NULL;
        }

        if (JAK_UNLIKELY((step_ids = JAK_MALLOC(str_cap * sizeof(jak_archive_field_sid_t))) == NULL)) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                return NULL;
        }

        if (JAK_UNLIKELY((str_offs = JAK_MALLOC(str_cap * sizeof(jak_offset_t))) == NULL)) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (JAK_UNLIKELY((str_lens = JAK_MALLOC(str_cap * sizeof(jak_u32))) == NULL)) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                free(str_offs);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (JAK_UNLIKELY((idxs_matching = JAK_MALLOC(str_cap * sizeof(size_t))) == NULL)) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                free(str_offs);
                free(str_lens);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (JAK_UNLIKELY(jak_query_scan_strids(&it, query) == false)) {
                free(str_offs);
                free(str_lens);
                free(idxs_matching);
                goto cleanup_result_and_error;
        }

        if (JAK_UNLIKELY((result_ids = JAK_MALLOC(result_cap * sizeof(jak_archive_field_sid_t))) == NULL)) {
                JAK_ERROR(&query->err, JAK_ERR_MALLOCERR);
                free(str_offs);
                free(str_lens);
                free(idxs_matching);
                jak_strid_iter_close(&it);
                goto cleanup_result_and_error;
                return NULL;
        }

        while (jak_strid_iter_next(&success, &info, &query->err, &info_len, &it)) {
                if (JAK_UNLIKELY(info_len > str_cap)) {
                        str_cap = (info_len + 1) * 1.7f;
                        if (JAK_UNLIKELY((tmp = realloc(str_offs, str_cap * sizeof(jak_offset_t))) == NULL)) {
                                goto realloc_error;
                        } else {
                                str_offs = tmp;
                        }
                        if (JAK_UNLIKELY((tmp = realloc(str_lens, str_cap * sizeof(jak_u32))) == NULL)) {
                                goto realloc_error;
                        } else {
                                str_lens = tmp;
                        }
                        if (JAK_UNLIKELY((tmp = realloc(idxs_matching, str_cap * sizeof(size_t))) == NULL)) {
                                goto realloc_error;
                        } else {
                                idxs_matching = tmp;
                        }
                }
                JAK_ASSERT(info_len <= str_cap);
                for (step_len = 0; step_len < info_len; step_len++) {
                        JAK_ASSERT(step_len < str_cap);
                        str_offs[step_len] = info[step_len].offset;
                        str_lens[step_len] = info[step_len].strlen;
                }

                char **strings = jak_query_fetch_strings_by_offset(query,
                                                                   str_offs,
                                                                   str_lens,
                                                                   step_len); // TODO: buffer + cleanup buffer

                if (JAK_UNLIKELY(
                        jak_string_pred_eval(pred, idxs_matching, &num_matching, strings, step_len, capture) ==
                        false)) {
                        JAK_ERROR(&query->err, JAK_ERR_PREDEVAL_FAILED);
                        jak_strid_iter_close(&it);
                        goto cleanup_intermediate;
                }

                for (size_t i = 0; i < step_len; i++) {
                        free(strings[i]);
                }
                free(strings);

                for (size_t i = 0; i < num_matching; i++) {
                        JAK_ASSERT (idxs_matching[i] < info_len);
                        result_ids[result_len++] = info[idxs_matching[i]].id;
                        if (pred_limit > 0 && result_len == (size_t) pred_limit) {
                                goto stop_search_and_return;
                        }
                        if (JAK_UNLIKELY(result_len > result_cap)) {
                                result_cap = (result_len + 1) * 1.7f;
                                if (JAK_UNLIKELY(
                                        (tmp = realloc(result_ids, result_cap * sizeof(jak_archive_field_sid_t))) ==
                                        NULL)) {
                                        jak_strid_iter_close(&it);
                                        goto cleanup_intermediate;
                                } else {
                                        result_ids = tmp;
                                }
                        }
                }
        }

        stop_search_and_return:
        if (JAK_UNLIKELY(success == false)) {
                jak_strid_iter_close(&it);
                goto cleanup_intermediate;
        }

        *num_found = result_len;
        return result_ids;

        realloc_error:
        JAK_ERROR(&query->err, JAK_ERR_REALLOCERR);

        cleanup_intermediate:
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        free(result_ids);

        cleanup_result_and_error:
        free(step_ids);
        return NULL;
}