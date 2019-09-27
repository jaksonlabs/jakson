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

#include <jakson/archive/internal.h>
#include <jakson/archive/pred.h>
#include <jakson/archive/cache.h>
#include <jakson/archive/query.h>

struct sid_to_offset_arg {
        offset_t offset;
        u32 strlen;
};

struct sid_to_offset {
        hashtable ofMapping(archive_field_sid_t, struct sid_to_offset_arg) mapping;
        FILE *disk_file;
        size_t disk_file_size;

};

#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(num_pairs, obj, bit_flag_name, offset_name)                                \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        ERROR_PRINT_AND_DIE(ERR_NULLPTR)                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_name) {                                                                               \
        JAK_ASSERT(obj->props.offset_name != 0);                                                                           \
        memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        fixed_prop prop;                                                                                      \
        int_embedded_fixed_props_read(&prop, &obj->file);                                                       \
        archive_int_reset_carbon_object_mem_file(obj);                                                                   \
        OPTIONAL_SET(num_pairs, prop.header->num_entries);                                                      \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        OPTIONAL_SET(num_pairs, 0);                                                                             \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

bool query_create(query *query, archive *archive)
{
        ERROR_IF_NULL(query)
        ERROR_IF_NULL(archive)
        query->archive = archive;
        query->context = archive_io_context_create(archive);
        error_init(&query->err);
        return query->context != NULL;
}

bool query_drop(query *query)
{
        ERROR_IF_NULL(query)
        return io_context_drop(query->context);
}

bool query_scan_strids(strid_iter *it, query *query)
{
        ERROR_IF_NULL(it)
        ERROR_IF_NULL(query)
        return strid_iter_open(it, &query->err, query->archive);
}

static bool
index_string_id_to_offset_open_file(struct sid_to_offset *index, err *err, const char *file)
{
        index->disk_file = fopen(file, "r");
        if (!index->disk_file) {
                ERROR(err, ERR_FOPEN_FAILED)
                return false;
        } else {
                fseek(index->disk_file, 0, SEEK_END);
                index->disk_file_size = ftell(index->disk_file);
                fseek(index->disk_file, 0, SEEK_SET);
                return true;
        }
}

bool query_create_index_string_id_to_offset(struct sid_to_offset **index, query *query)
{
        ERROR_IF_NULL(index)
        ERROR_IF_NULL(query)

        strid_iter strid_iter;
        strid_info *info;
        size_t vector_len;
        bool status;
        bool success;
        u32 capacity;
        archive_info archive_info;
        archive_get_info(&archive_info, query->archive);
        capacity = archive_info.num_embeddded_strings;

        struct sid_to_offset *result = MALLOC(sizeof(struct sid_to_offset));
        hashtable_create(&result->mapping,
                         &query->err,
                         sizeof(archive_field_sid_t),
                         sizeof(struct sid_to_offset_arg),
                         capacity);

        if (!index_string_id_to_offset_open_file(result, &query->err, query->archive->disk_file_path)) {
                return false;
        }

        status = query_scan_strids(&strid_iter, query);

        if (status) {
                while (strid_iter_next(&success, &info, &query->err, &vector_len, &strid_iter)) {
                        for (size_t i = 0; i < vector_len; i++) {
                                struct sid_to_offset_arg arg = {.offset = info[i].offset, .strlen = info[i].strlen};
                                hashtable_insert_or_update(&result->mapping, &info[i].id, &arg, 1);
                        }
                }
                *index = result;
                strid_iter_close(&strid_iter);
                return true;
        } else {
                ERROR(&query->err, ERR_SCAN_FAILED);
                return false;
        }

}

void query_drop_index_string_id_to_offset(struct sid_to_offset *index)
{
        if (index) {
                hashtable_drop(&index->mapping);
                fclose(index->disk_file);
                free(index);
        }
}

bool query_index_id_to_offset_serialize(FILE *file, err *err, struct sid_to_offset *index)
{
        UNUSED(file);
        UNUSED(err);
        UNUSED(index);
        return hashtable_serialize(file, &index->mapping);
}

bool query_index_id_to_offset_deserialize(struct sid_to_offset **index, err *err,
                                              const char *file_path, offset_t offset)
{
        ERROR_IF_NULL(index)
        ERROR_IF_NULL(err)
        ERROR_IF_NULL(file_path)
        ERROR_IF_NULL(offset)

        struct sid_to_offset *result = MALLOC(sizeof(struct sid_to_offset));
        if (!result) {
                ERROR(err, ERR_MALLOCERR);
                return false;
        }

        if (!index_string_id_to_offset_open_file(result, err, file_path)) {
                return false;
        }

        FILE *index_reader_file = fopen(file_path, "r");
        if (!index_reader_file) {
                ERROR(err, ERR_FOPEN_FAILED)
                return false;
        } else {
                fseek(index_reader_file, 0, SEEK_END);
                offset_t file_size = ftell(index_reader_file);

                if (offset >= file_size) {
                        ERROR(err, ERR_INTERNALERR)
                        return false;
                }

                fseek(index_reader_file, offset, SEEK_SET);

                if (!hashtable_deserialize(&result->mapping, err, index_reader_file)) {
                        ERROR(err, ERR_HASTABLE_DESERIALERR);
                        fclose(index_reader_file);
                        *index = NULL;
                        return false;
                }

                fclose(index_reader_file);
                *index = result;
                return true;
        }
}

static char *fetch_string_from_file(bool *decode_success, FILE *disk_file, size_t offset, size_t string_len,
                                    err *err, archive *archive)
{
        char *result = MALLOC(string_len + 1);
        memset(result, 0, string_len + 1);

        fseek(disk_file, offset, SEEK_SET);

        bool decode_result = pack_decode(err, &archive->string_table.compressor, result, string_len, disk_file);

        *decode_success = decode_result;
        return result;
}

static char *fetch_string_by_id_via_scan(query *query, archive_field_sid_t id)
{
        JAK_ASSERT(query);

        strid_iter strid_iter;
        strid_info *info;
        size_t vector_len;
        bool status;
        bool success;

        status = query_scan_strids(&strid_iter, query);

        if (status) {
                while (strid_iter_next(&success, &info, &query->err, &vector_len, &strid_iter)) {
                        for (size_t i = 0; i < vector_len; i++) {
                                if (info[i].id == id) {
                                        bool decode_result;
                                        char *result = fetch_string_from_file(&decode_result,
                                                                              strid_iter.disk_file,
                                                                              info[i].offset,
                                                                              info[i].strlen,
                                                                              &query->err,
                                                                              query->archive);

                                        bool close_iter_result = strid_iter_close(&strid_iter);

                                        if (!success || !close_iter_result) {
                                                if (result) {
                                                        free(result);
                                                }
                                                ERROR(&query->err,
                                                      !decode_result ? ERR_DECOMPRESSFAILED
                                                                     : ERR_ITERATORNOTCLOSED);
                                                return NULL;
                                        } else {
                                                return result;
                                        }
                                }
                        }
                }
                strid_iter_close(&strid_iter);
                ERROR(&query->err, ERR_NOTFOUND);
                return NULL;
        } else {
                ERROR(&query->err, ERR_SCAN_FAILED);
                return NULL;
        }
}

static char *fetch_string_by_id_via_index(query *query, struct sid_to_offset *index,
                                          archive_field_sid_t id)
{
        const struct sid_to_offset_arg *args = hashtable_get_value(&index->mapping, &id);
        if (args) {
                if (args->offset < index->disk_file_size) {
                        bool decode_result;
                        char *result = fetch_string_from_file(&decode_result,
                                                              index->disk_file,
                                                              args->offset,
                                                              args->strlen,
                                                              &query->err,
                                                              query->archive);
                        if (decode_result) {
                                return result;
                        } else {
                                ERROR(&query->err, ERR_DECOMPRESSFAILED);
                                return NULL;
                        }

                } else {
                        ERROR(&query->err, ERR_INDEXCORRUPTED_OFFSET);
                        return NULL;
                }
        } else {
                ERROR(&query->err, ERR_NOTFOUND);
                return NULL;
        }
}

char *query_fetch_string_by_id(query *query, archive_field_sid_t id)
{
        JAK_ASSERT(query);

        bool has_cache = false;
        archive_hash_query_string_id_cache(&has_cache, query->archive);
        if (has_cache) {
                return string_id_cache_get(query->archive->string_id_cache, id);
        } else {
                return query_fetch_string_by_id_nocache(query, id);
        }
}

char *query_fetch_string_by_id_nocache(query *query, archive_field_sid_t id)
{
        bool has_index = false;
        archive_has_query_index_string_id_to_offset(&has_index, query->archive);
        if (has_index) {
                return fetch_string_by_id_via_index(query, query->archive->query_index_string_id_to_offset, id);
        } else {
                return fetch_string_by_id_via_scan(query, id);
        }
}

char **query_fetch_strings_by_offset(query *query, offset_t *offs, u32 *strlens,
                                         size_t num_offs)
{
        JAK_ASSERT(query);
        JAK_ASSERT(offs);
        JAK_ASSERT(strlens);

        FILE *file;

        if (num_offs == 0) {
                return NULL;
        }

        char **result = MALLOC(num_offs * sizeof(char *));
        if (!result) {
                ERROR(&query->err, ERR_MALLOCERR);
                return NULL;
        }
        for (size_t i = 0; i < num_offs; i++) {
                if ((result[i] = MALLOC((strlens[i] + 1) * sizeof(char))) == NULL) {
                        for (size_t k = 0; k < i; k++) {
                                free(result[k]);
                        }
                        free(result);
                        return NULL;
                }
                memset(result[i], 0, (strlens[i] + 1) * sizeof(char));
        }

        if (!result) {
                ERROR(io_context_get_error(query->context), ERR_MALLOCERR);
                return NULL;
        } else {
                if (!(file = io_context_lock_and_access(query->context))) {
                        error_cpy(&query->err, io_context_get_error(query->context));
                        goto cleanup_and_error;
                }

                for (size_t i = 0; i < num_offs; i++) {
                        fseek(file, offs[i], SEEK_SET);
                        if (!pack_decode(&query->err,
                                         &query->archive->string_table.compressor,
                                         result[i],
                                         strlens[i],
                                         file)) {
                                io_context_unlock(query->context);
                                goto cleanup_and_error;
                        }
                }
                io_context_unlock(query->context);
                return result;
        }

        cleanup_and_error:
        for (size_t i = 0; i < num_offs; i++) {
                free(result[i]);
        }
        free(result);
        return NULL;
}

archive_field_sid_t *query_find_ids(size_t *num_found, query *query,
                                            const string_pred *pred, void *capture, i64 limit)
{
        if (UNLIKELY(string_pred_validate(&query->err, pred) == false)) {
                return NULL;
        }
        i64 pred_limit;
        string_pred_get_limit(&pred_limit, pred);
        pred_limit = pred_limit < 0 ? limit : JAK_MIN(pred_limit, limit);

        strid_iter it;
        strid_info *info = NULL;
        size_t info_len = 0;
        size_t step_len = 0;
        offset_t *str_offs = NULL;
        u32 *str_lens = NULL;
        size_t *idxs_matching = NULL;
        size_t num_matching = 0;
        void *tmp = NULL;
        size_t str_cap = 1024;
        archive_field_sid_t *step_ids = NULL;
        archive_field_sid_t *result_ids = NULL;
        size_t result_len = 0;
        size_t result_cap = pred_limit < 0 ? str_cap : (size_t) pred_limit;
        bool success = false;

        if (UNLIKELY(pred_limit == 0)) {
                *num_found = 0;
                return NULL;
        }

        if (UNLIKELY(!num_found || !query || !pred)) {
                ERROR(&query->err, ERR_NULLPTR);
                return NULL;
        }

        if (UNLIKELY((step_ids = MALLOC(str_cap * sizeof(archive_field_sid_t))) == NULL)) {
                ERROR(&query->err, ERR_MALLOCERR);
                return NULL;
        }

        if (UNLIKELY((str_offs = MALLOC(str_cap * sizeof(offset_t))) == NULL)) {
                ERROR(&query->err, ERR_MALLOCERR);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (UNLIKELY((str_lens = MALLOC(str_cap * sizeof(u32))) == NULL)) {
                ERROR(&query->err, ERR_MALLOCERR);
                free(str_offs);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (UNLIKELY((idxs_matching = MALLOC(str_cap * sizeof(size_t))) == NULL)) {
                ERROR(&query->err, ERR_MALLOCERR);
                free(str_offs);
                free(str_lens);
                goto cleanup_result_and_error;
                return NULL;
        }

        if (UNLIKELY(query_scan_strids(&it, query) == false)) {
                free(str_offs);
                free(str_lens);
                free(idxs_matching);
                goto cleanup_result_and_error;
        }

        if (UNLIKELY((result_ids = MALLOC(result_cap * sizeof(archive_field_sid_t))) == NULL)) {
                ERROR(&query->err, ERR_MALLOCERR);
                free(str_offs);
                free(str_lens);
                free(idxs_matching);
                strid_iter_close(&it);
                goto cleanup_result_and_error;
                return NULL;
        }

        while (strid_iter_next(&success, &info, &query->err, &info_len, &it)) {
                if (UNLIKELY(info_len > str_cap)) {
                        str_cap = (info_len + 1) * 1.7f;
                        if (UNLIKELY((tmp = realloc(str_offs, str_cap * sizeof(offset_t))) == NULL)) {
                                goto realloc_error;
                        } else {
                                str_offs = tmp;
                        }
                        if (UNLIKELY((tmp = realloc(str_lens, str_cap * sizeof(u32))) == NULL)) {
                                goto realloc_error;
                        } else {
                                str_lens = tmp;
                        }
                        if (UNLIKELY((tmp = realloc(idxs_matching, str_cap * sizeof(size_t))) == NULL)) {
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

                char **strings = query_fetch_strings_by_offset(query,
                                                                   str_offs,
                                                                   str_lens,
                                                                   step_len); // TODO: buffer + cleanup buffer

                if (UNLIKELY(
                        string_pred_eval(pred, idxs_matching, &num_matching, strings, step_len, capture) ==
                        false)) {
                        ERROR(&query->err, ERR_PREDEVAL_FAILED);
                        strid_iter_close(&it);
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
                        if (UNLIKELY(result_len > result_cap)) {
                                result_cap = (result_len + 1) * 1.7f;
                                if (UNLIKELY(
                                        (tmp = realloc(result_ids, result_cap * sizeof(archive_field_sid_t))) ==
                                        NULL)) {
                                        strid_iter_close(&it);
                                        goto cleanup_intermediate;
                                } else {
                                        result_ids = tmp;
                                }
                        }
                }
        }

        stop_search_and_return:
        if (UNLIKELY(success == false)) {
                strid_iter_close(&it);
                goto cleanup_intermediate;
        }

        *num_found = result_len;
        return result_ids;

        realloc_error:
        ERROR(&query->err, ERR_REALLOCERR);

        cleanup_intermediate:
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        free(result_ids);

        cleanup_result_and_error:
        free(step_ids);
        return NULL;
}