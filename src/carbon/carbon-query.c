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

#include <carbon/carbon-int-archive.h>
#include <carbon/carbon-string-pred.h>
#include "carbon/carbon-query.h"


#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(num_pairs, obj, bit_flag_name, offset_name)                                \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR)                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_name) {                                                                               \
        assert(obj->props.offset_name != 0);                                                                           \
        carbon_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        carbon_fixed_prop_t prop;                                                                                      \
        carbon_int_embedded_fixed_props_read(&prop, &obj->file);                                                       \
        carbon_int_reset_cabin_object_mem_file(obj);                                                                   \
        CARBON_OPTIONAL_SET(num_pairs, prop.header->num_entries);                                                      \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(num_pairs, 0);                                                                             \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

static carbon_off_t object_init(carbon_archive_object_t *obj, carbon_memblock_t *memBlock, carbon_off_t objectHeaderOffset, carbon_archive_record_table_t *context)
{
    carbon_memfile_open(&obj->file, memBlock, CARBON_MEMFILE_MODE_READONLY);
    carbon_memfile_seek(&obj->file, objectHeaderOffset);
    assert (*CARBON_MEMFILE_PEEK(&obj->file, char) == MARKER_SYMBOL_OBJECT_BEGIN);
    carbon_object_header_t *header = CARBON_MEMFILE_READ_TYPE(&obj->file, carbon_object_header_t);
    carbon_archive_object_flags_t flags = {
        .value = header->flags
    };
    carbon_error_init(&obj->err);
    obj->context = context;
    obj->flags.value = header->flags;
    carbon_int_read_prop_offsets(&obj->props, &obj->file, &flags);
    obj->self = objectHeaderOffset;
    carbon_off_t read_length = CARBON_MEMFILE_TELL(&obj->file) - objectHeaderOffset;
    carbon_memfile_seek(&obj->file, objectHeaderOffset);
    return read_length;
}

CARBON_EXPORT(bool)
carbon_query_create(carbon_query_t *query, carbon_archive_t *archive)
{
    CARBON_NON_NULL_OR_ERROR(query)
    CARBON_NON_NULL_OR_ERROR(archive)
    query->archive = archive;
    query->context = carbon_archive_io_context_create(archive);
    carbon_error_init(&query->err);
    return query->context != NULL;
}

CARBON_EXPORT(bool)
carbon_query_drop(carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(query)
    return carbon_io_context_drop(query->context);
}

CARBON_EXPORT(bool)
carbon_query_scan_strids(carbon_strid_iter_t *it, carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(it)
    CARBON_NON_NULL_OR_ERROR(query)
    return carbon_strid_iter_open(it, &query->err, query->archive);
}


CARBON_EXPORT(char *)
carbon_query_fetch_string_by_id(carbon_query_t *query, carbon_string_id_t id)
{
    assert(query);

    carbon_strid_iter_t  strid_iter;
    carbon_strid_info_t *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;

    status = carbon_query_scan_strids(&strid_iter, query);

    if (status) {
        while (carbon_strid_iter_next(&success, &info, &query->err, &vector_len, &strid_iter)) {
            for (size_t i = 0; i < vector_len; i++) {
                if (info[i].id == id) {
                    char *result = malloc(info[i].strlen + 1);
                    memset(result, 0, info[i].strlen + 1);

                    fseek(strid_iter.disk_file, info[i].offset, SEEK_SET);

                    bool decode_result = carbon_compressor_decode(&query->err, &query->archive->string_table.compressor,
                                                                  result, info[i].strlen, strid_iter.disk_file);

                    bool close_iter_result = carbon_strid_iter_close(&strid_iter);

                    if (!decode_result || !close_iter_result) {
                        free (result);
                        CARBON_ERROR(&query->err, !decode_result ? CARBON_ERR_DECOMPRESSFAILED :
                                                    CARBON_ERR_ITERATORNOTCLOSED);
                        return NULL;
                    } else {
                        return result;
                    }
                }
            }
        }
        CARBON_ERROR(&query->err, CARBON_ERR_NOTFOUND);
        return NULL;
    } else {
        CARBON_ERROR(&query->err, CARBON_ERR_SCAN_FAILED);
        return NULL;
    }
}

CARBON_EXPORT(char **)
carbon_query_fetch_strings_by_offset(carbon_query_t *query, carbon_off_t *offs, uint32_t *strlens, size_t num_offs)
{
    assert(query);
    assert(offs);
    assert(strlens);

    FILE *file;

    if (num_offs == 0)
    {
        return NULL;
    }

    char **result = malloc(num_offs * sizeof(char*));
    if (!result) {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        return NULL;
    }
    for (size_t i = 0; i < num_offs; i++)
    {
        if ((result[i] = malloc((strlens[i] + 1) * sizeof(char))) == NULL) {
            for (size_t k = 0; k < i; k++) {
                free(result[k]);
            }
            free(result);
            return NULL;
        }
        memset(result[i], 0, (strlens[i] + 1) * sizeof(char));
    }

    if (!result)
    {
        CARBON_ERROR(carbon_io_context_get_error(query->context), CARBON_ERR_MALLOCERR);
        return NULL;
    } else {
        if (!(file = carbon_io_context_lock(query->context)))
        {
            carbon_error_cpy(&query->err, carbon_io_context_get_error(query->context));
            goto cleanup_and_error;
        }

        for (size_t i = 0; i < num_offs; i++)
        {
            fseek(file, offs[i], SEEK_SET);
            if (!carbon_compressor_decode(&query->err, &query->archive->string_table.compressor,
                                          result[i], strlens[i], file))
            {
                carbon_io_context_unlock(query->context);
                goto cleanup_and_error;
            }
        }
        carbon_io_context_unlock(query->context);
        return result;
    }

cleanup_and_error:
    for (size_t i = 0; i < num_offs; i++) {
        free(result[i]);
    }
    free(result);
    return NULL;
}

CARBON_EXPORT(carbon_string_id_t *)
carbon_query_find_ids(size_t *num_found, carbon_query_t *query, const carbon_string_pred_t *pred,
                      void *capture, int64_t limit)
{
    if (CARBON_UNLIKELY(carbon_string_pred_validate(&query->err, pred) == false)) {
        return NULL;
    }
    int64_t              pred_limit;
    carbon_string_pred_get_limit(&pred_limit, pred);
    pred_limit = pred_limit < 0 ? limit : CARBON_MIN(pred_limit, limit);

    carbon_strid_iter_t  it;
    carbon_strid_info_t *info              = NULL;
    size_t               info_len          = 0;
    size_t               step_len          = 0;
    carbon_off_t        *str_offs          = NULL;
    uint32_t            *str_lens          = NULL;
    size_t              *idxs_matching     = NULL;
    size_t               num_matching      = 0;
    void                *tmp               = NULL;
    size_t               str_cap           = 1024;
    carbon_string_id_t  *step_ids          = NULL;
    carbon_string_id_t  *result_ids        = NULL;
    size_t               result_len        = 0;
    size_t               result_cap        = pred_limit < 0 ? str_cap : pred_limit;
    bool                 success           = false;

    if (CARBON_UNLIKELY(pred_limit == 0))
    {
        *num_found = 0;
        return NULL;
    }

    if (CARBON_UNLIKELY(!num_found || !query || !pred))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_NULLPTR);
        return NULL;
    }

    if (CARBON_UNLIKELY((step_ids = malloc(str_cap * sizeof(carbon_string_id_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        return NULL;
    }

    if (CARBON_UNLIKELY((str_offs = malloc(str_cap * sizeof(carbon_off_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY((str_lens = malloc(str_cap * sizeof(uint32_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY((idxs_matching = malloc(str_cap * sizeof(size_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        free(str_lens);
        goto cleanup_result_and_error;
        return NULL;
    }

    if (CARBON_UNLIKELY(carbon_query_scan_strids(&it, query) == false))
    {
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        goto cleanup_result_and_error;
    }

    if (CARBON_UNLIKELY((result_ids = malloc(result_cap * sizeof(carbon_string_id_t))) == NULL))
    {
        CARBON_ERROR(&query->err, CARBON_ERR_MALLOCERR);
        free(str_offs);
        free(str_lens);
        free(idxs_matching);
        carbon_strid_iter_close(&it);
        goto cleanup_result_and_error;
        return NULL;
    }

    while (carbon_strid_iter_next(&success, &info, &query->err, &info_len, &it))
    {
        if (CARBON_UNLIKELY(info_len > str_cap))
        {
            str_cap = (info_len + 1) * 1.7f;
            if (CARBON_UNLIKELY((tmp = realloc(str_offs, str_cap * sizeof(carbon_off_t))) == NULL))
            {
                goto realloc_error;
            } else {
                str_offs = tmp;
            }
            if (CARBON_UNLIKELY((tmp = realloc(str_lens, str_cap * sizeof(uint32_t))) == NULL))
            {
                goto realloc_error;
            } else {
                str_lens = tmp;
            }
            if (CARBON_UNLIKELY((tmp = realloc(idxs_matching, str_cap * sizeof(size_t))) == NULL))
            {
                goto realloc_error;
            } else {
                idxs_matching = tmp;
            }
        }
        assert(info_len <= str_cap);
        for (step_len = 0; step_len < info_len; step_len++)
        {
            assert(step_len < str_cap);
            str_offs[step_len] = info[step_len].offset;
            str_lens[step_len] = info[step_len].strlen;
        }

        char **strings = carbon_query_fetch_strings_by_offset(query, str_offs, str_lens, step_len); // TODO: buffer + cleanup buffer

        if (CARBON_UNLIKELY(carbon_string_pred_eval(pred, idxs_matching, &num_matching,
                                                    strings, step_len, capture) == false))
        {
            CARBON_ERROR(&query->err, CARBON_ERR_PREDEVAL_FAILED);
            carbon_strid_iter_close(&it);
            goto cleanup_intermediate;
        }

        for (size_t i = 0; i < step_len; i++) {
            free(strings[i]);
        }
        free(strings);

        for (size_t i = 0; i < num_matching; i++)
        {
            assert (idxs_matching[i] < info_len);
            result_ids[result_len++] =  info[idxs_matching[i]].id;
            if (pred_limit > 0 && result_len == (size_t) pred_limit) {
                goto stop_search_and_return;
            }
            if (CARBON_UNLIKELY(result_len > result_cap))
            {
                result_cap = (result_len + 1) * 1.7f;
                if (CARBON_UNLIKELY((tmp = realloc(result_ids, result_cap * sizeof(carbon_string_id_t))) == NULL))
                {
                    carbon_strid_iter_close(&it);
                    goto cleanup_intermediate;
                } else {
                    result_ids = tmp;
                }
            }
        }
    }

stop_search_and_return:
    if (CARBON_UNLIKELY(success == false)) {
        carbon_strid_iter_close(&it);
        goto cleanup_intermediate;
    }

    *num_found = result_len;
    return result_ids;

realloc_error:
    CARBON_ERROR(&query->err, CARBON_ERR_REALLOCERR);

cleanup_intermediate:
    free(str_offs);
    free(str_lens);
    free(idxs_matching);
    free(result_ids);

cleanup_result_and_error:
    free (step_ids);
    return NULL;
}


CARBON_EXPORT(bool)
carbon_archive_record(carbon_archive_object_t *root, carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(root)
    CARBON_NON_NULL_OR_ERROR(query)
    object_init(root, query->archive->record_table.recordDataBase, 0, &query->archive->record_table);
    return true;
}

CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
carbon_archive_object_keys_to_type(CARBON_NULLABLE size_t *npairs, carbon_type_e type, carbon_archive_object_t *obj)
{
    switch (type) {
    case CARBON_TYPE_INT8:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int8_props, int8s);
    case CARBON_TYPE_INT16:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int16_props, int16s);
    case CARBON_TYPE_INT32:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int32_props, int32s);
    case CARBON_TYPE_INT64:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int64_props, int64s);
    case CARBON_TYPE_UINT8:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint8_props, uint8s);
    case CARBON_TYPE_UINT16:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint16_props, uint16s);
    case CARBON_TYPE_UINT32:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint32_props, uint32s);
    case CARBON_TYPE_UINT64:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint64_props, uint64s);
    case CARBON_TYPE_FLOAT:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_float_props, floats);
    case CARBON_TYPE_STRING:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_string_props, strings);
    case CARBON_TYPE_BOOL:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_bool_props, bools);
    case CARBON_TYPE_VOID: {
        if (obj->flags.bits.has_null_props) {
            assert(obj->props.nulls != 0);
            carbon_memfile_seek(&obj->file, obj->props.nulls);
            carbon_null_prop_t prop;
            carbon_int_embedded_null_props_read(&prop, &obj->file);
            carbon_int_reset_cabin_object_mem_file(obj);
            CARBON_OPTIONAL_SET(npairs, prop.header->num_entries);
            return prop.keys;
        } else {
            CARBON_OPTIONAL_SET(npairs, 0);
            return NULL;
        }
    }
    case CARBON_TYPE_OBJECT: {
        if (obj->flags.bits.has_object_props) {
            assert(obj->props.objects != 0);
            carbon_memfile_seek(&obj->file, obj->props.objects);
            carbon_var_prop_t objectProp;
            carbon_int_embedded_var_props_read(&objectProp, &obj->file);
            carbon_int_reset_cabin_object_mem_file(obj);
            CARBON_OPTIONAL_SET(npairs, objectProp.header->num_entries);
            return objectProp.keys;
        } else {
            CARBON_OPTIONAL_SET(npairs, 0);
            return NULL;
        }
    }
    default:
        CARBON_ERROR(&obj->err, CARBON_ERR_NOTYPE)
        return NULL;
    }
}

CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
carbon_archive_object_keys_to_array(CARBON_NULLABLE size_t *npairs, carbon_type_e type, carbon_archive_object_t *obj)
{
    switch (type) {
    case CARBON_TYPE_INT8:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int8_array_props, int8_arrays);
    case CARBON_TYPE_INT16:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int16_array_props, int16_arrays);
    case CARBON_TYPE_INT32:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int32_array_props, int32_arrays);
    case CARBON_TYPE_INT64:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_int64_array_props, int64_arrays);
    case CARBON_TYPE_UINT8:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint8_array_props, uint8_arrays);
    case CARBON_TYPE_UINT16:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint16_array_props, uint16_arrays);
    case CARBON_TYPE_UINT32:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint32_array_props, uint32_arrays);
    case CARBON_TYPE_UINT64:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_uint64_array_props, uint64_arrays);
    case CARBON_TYPE_FLOAT:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_float_array_props, float_arrays);
    case CARBON_TYPE_STRING:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_string_array_props, string_arrays);
    case CARBON_TYPE_BOOL:
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_bool_array_props, bool_arrays);
    case CARBON_TYPE_VOID: {
        OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(npairs, obj, has_null_array_props, null_arrays);
    }
    case CARBON_TYPE_OBJECT: {
        CARBON_ERROR(&obj->err, CARBON_ERR_ERRINTERNAL) /** wrong usage: use table get function instead */
        return NULL;
    }
    default:
        CARBON_ERROR(&obj->err, CARBON_ERR_NOTYPE)
        return NULL;
    }
}

bool carbon_archive_table_open(carbon_archive_table_t *out, carbon_archive_object_t *obj)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(obj);

    if (obj->flags.bits.has_object_array_props) {
        assert(obj->props.object_arrays != 0);
        carbon_memfile_seek(&obj->file, obj->props.object_arrays);
        carbon_table_prop_t prop;
        carbon_int_embedded_table_props_read(&prop, &obj->file);
        carbon_int_reset_cabin_object_mem_file(obj);
        out->ngroups = prop.header->num_entries;
        out->keys = prop.keys;
        out->groups_offsets = prop.groupOffs;
        out->context = obj;
        carbon_error_init(&out->err);
        return true;
    } else {
        CARBON_ERROR(&obj->err, CARBON_ERR_NOTFOUND);
        return false;
    }
}

bool carbon_archive_object_values_object(carbon_archive_object_t *out, size_t idx,
                                         carbon_archive_object_t *props)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(props);
    if (!props->flags.bits.has_object_props) {
        CARBON_ERROR(&props->err, CARBON_ERR_NOTFOUND);
        return false;
    } else {
        assert(props->props.objects != 0);
        carbon_memfile_seek(&props->file, props->props.objects);
        carbon_var_prop_t objectProp;
        carbon_int_embedded_var_props_read(&objectProp, &props->file);
        if (idx > objectProp.header->num_entries) {
            carbon_int_reset_cabin_object_mem_file(props);
            CARBON_ERROR(&props->err, CARBON_ERR_NOTFOUND);
            return false;
        } else {
            object_init(out, props->context->recordDataBase, objectProp.offsets[idx], props->context);
            return true;
        }
    }
}

#define OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, bit_flag_prop_name, offset_prop_name, T)                            \
({                                                                                                                     \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR);                                                                \
    }                                                                                                                  \
                                                                                                                       \
    const void *result = NULL;                                                                                         \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_prop_name) {                                                                          \
        assert(obj->props.offset_prop_name != 0);                                                                      \
        carbon_memfile_seek(&obj->file, obj->props.offset_prop_name);                                                  \
        carbon_fixed_prop_t prop;                                                                                      \
        carbon_int_embedded_fixed_props_read(&prop, &obj->file);                                                       \
        carbon_int_reset_cabin_object_mem_file(obj);                                                                   \
        CARBON_OPTIONAL_SET(npairs, prop.header->num_entries);                                                         \
        result = prop.values;                                                                                          \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(npairs, 0);                                                                                \
    }                                                                                                                  \
    (const T *) result;                                                                                                \
})

const carbon_int8_t *carbon_archive_object_values_int8(CARBON_NULLABLE
                                                       size_t *npairs,
                                                       carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_int8_props, int8s, carbon_int8_t);
}

const carbon_int16_t *carbon_archive_object_values_int16s(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_int16_props, int16s, carbon_int16_t);
}

const carbon_int32_t *carbon_archive_object_values_int32(CARBON_NULLABLE
                                                         size_t *npairs,
                                                         carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_int32_props, int32s, carbon_int32_t);
}

const carbon_int64_t *carbon_archive_object_values_int64s(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_int64_props, int64s, carbon_int64_t);
}

const carbon_uint8_t *carbon_archive_object_values_uint8s(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_uint8_props, uint8s, carbon_uint8_t);
}

const carbon_uint16_t *carbon_archive_object_values_uin16(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_uint16_props, uint16s, carbon_uint16_t);
}

const carbon_uin32_t *carbon_archive_object_values_uint32(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_uint32_props, uint32s, carbon_uin32_t);
}

const carbon_uin64_t *carbon_archive_object_values_uint64(CARBON_NULLABLE
                                                          size_t *npairs,
                                                          carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_uint64_props, uint64s, carbon_uin64_t);
}

const carbon_bool_t *carbon_archive_object_values_bool(CARBON_NULLABLE
                                                       size_t *npairs,
                                                       carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_bool_props, bools, carbon_bool_t);
}

const carbon_float_t *carbon_archive_object_values_float(CARBON_NULLABLE
                                                         size_t *npairs,
                                                         carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_float_props, floats, carbon_float_t);
}

const carbon_string_id_t *carbon_archive_object_values_strings(CARBON_NULLABLE
                                                               size_t *npairs,
                                                               carbon_archive_object_t *obj)
{
    return OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, has_string_props, strings, carbon_string_id_t);
}

#define OBJECT_GET_ARRAY_LENGTHS_GENERIC(err, length, obj, bit_fiel_name, offset_name, idx, prop)                      \
({                                                                                                                     \
    int status;                                                                                                        \
                                                                                                                       \
    if (obj->flags.bits.bit_fiel_name) {                                                                               \
        assert(obj->props.offset_name != 0);                                                                           \
        carbon_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        carbon_int_embedded_array_props_read(&prop, &obj->file);                                                       \
        carbon_int_reset_cabin_object_mem_file(obj);                                                                   \
        if (CARBON_UNLIKELY(idx >= prop.header->num_entries)) {                                                 \
            *length = 0;                                                                                               \
            CARBON_ERROR(err, CARBON_ERR_OUTOFBOUNDS);                                                                 \
            status = false;                                                                                            \
        } else {                                                                                                       \
            *length = prop.lengths[idx];                                                                               \
            status = true;                                                                                             \
        }                                                                                                              \
    } else {                                                                                                           \
        *length = 0;                                                                                                   \
        CARBON_ERROR(err, CARBON_ERR_NOTFOUND);                                                                        \
        status = false;                                                                                                \
    }                                                                                                                  \
    status;                                                                                                            \
})

#define OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, T)                                                     \
({                                                                                                                     \
    const void *result = NULL;                                                                                         \
    if (status == true) {                                                                                              \
        size_t skip_size = 0;                                                                                          \
        for (size_t i = 0; i < idx; i++) {                                                                             \
            skip_size += prop.lengths[i] * sizeof(T);                                                                  \
        }                                                                                                              \
        carbon_memfile_seek(&obj->file, prop.values_begin + skip_size);                                                \
        result = carbon_memfile_peek(&obj->file, 1);                                                                   \
        carbon_int_reset_cabin_object_mem_file(obj);                                                                   \
    }                                                                                                                  \
    (const T*) result;                                                                                                 \
})

const carbon_int8_t *carbon_archive_object_values_int8_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int8_array_props,
                                                  int8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int8_t);
}

const carbon_int16_t *carbon_archive_object_values_int16_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int16_array_props,
                                                  int16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int16_t);
}

const carbon_int32_t *carbon_archive_object_values_int32_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int32_array_props,
                                                  int32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int32_t);
}

const carbon_int64_t *carbon_archive_object_values_int64_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int64_array_props,
                                                  int64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int64_t);
}

const carbon_uint8_t *carbon_archive_object_values_uint8_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint8_array_props,
                                                  uint8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint8_t);
}

const carbon_uint16_t *carbon_archive_object_values_uint16_arrays(uint32_t *length,
                                                                  size_t idx,
                                                                  carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint16_array_props,
                                                  uint16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint16_t);
}

const carbon_uin32_t *carbon_archive_object_values_uint32_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint32_array_props,
                                                  uint32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin32_t);
}

const carbon_uin64_t *carbon_archive_object_values_uint64_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint64_array_props,
                                                  uint64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin64_t);
}

const carbon_bool_t *carbon_archive_object_values_bool_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_bool_array_props,
                                                  bool_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_bool_t);
}

const carbon_float_t *carbon_archive_object_values_float_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_float_array_props,
                                                  float_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_float_t);
}

const carbon_string_id_t *carbon_archive_object_values_string_arrays(uint32_t *length,
                                                                     size_t idx,
                                                                     carbon_archive_object_t *obj)
{
    carbon_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_string_array_props,
                                                  string_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_string_id_t);
}

bool carbon_archive_object_values_null_array_lengths(uint32_t *length, size_t idx, carbon_archive_object_t *obj)
{
    CARBON_NON_NULL_OR_ERROR(length);
    CARBON_NON_NULL_OR_ERROR(obj);
    carbon_array_prop_t prop;
    return OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_null_array_props,
                                            null_arrays, idx, prop);
}

bool carbon_archive_table_column_group(carbon_column_group_t *group, size_t idx, carbon_archive_table_t *table)
{
    CARBON_NON_NULL_OR_ERROR(group);
    CARBON_NON_NULL_OR_ERROR(table);
    CARBON_NON_NULL_OR_ERROR(table->context);

    if (CARBON_UNLIKELY(idx >= table->ngroups)) {
        CARBON_ERROR(&table->err, CARBON_ERR_OUTOFBOUNDS);
        return false;
    } else {
        carbon_off_t group_off = table->groups_offsets[idx];
        carbon_off_t last = CARBON_MEMFILE_TELL(&table->context->file);
        carbon_memfile_seek(&table->context->file, group_off);
        carbon_column_group_header_t *column_group_header = CARBON_MEMFILE_READ_TYPE(&table->context->file,
        carbon_column_group_header_t);
        group->ncolumns = column_group_header->num_columns;
        group->column_offsets = (const carbon_off_t *) carbon_memfile_peek(&table->context->file, sizeof(carbon_off_t));
        group->context = table->context;
        carbon_error_init(&group->err);

        carbon_memfile_seek(&table->context->file, last);

        return true;
    }
}

bool carbon_archive_table_column(carbon_column_t *column, size_t idx, carbon_column_group_t *group)
{
    CARBON_NON_NULL_OR_ERROR(column);
    CARBON_NON_NULL_OR_ERROR(group);
    if (CARBON_UNLIKELY(idx >= group->ncolumns)) {
        CARBON_ERROR(&group->err, CARBON_ERR_OUTOFBOUNDS);
        return false;
    } else {
        carbon_off_t last = CARBON_MEMFILE_TELL(&group->context->file);
        carbon_off_t column_off = group->column_offsets[idx];
        carbon_memfile_seek(&group->context->file, column_off);
        const carbon_column_header_t *header = CARBON_MEMFILE_READ_TYPE(&group->context->file, carbon_column_header_t);
        column->nelems = header->num_entries;
        column->type = carbon_int_get_value_type_of_char(header->value_type);
        column->entry_offsets = (const carbon_off_t *) carbon_memfile_peek(&group->context->file, sizeof(carbon_off_t));
        carbon_memfile_skip(&group->context->file, column->nelems * sizeof(carbon_off_t));
        column->position_list = (const uint32_t*) carbon_memfile_peek(&group->context->file, sizeof(uint32_t));
        column->context = group->context;
        carbon_error_init(&column->err);
        carbon_memfile_seek(&group->context->file, last);
        return true;
    }
}

bool carbon_archive_table_field_type(carbon_field_type_e *type, const carbon_field_t *field)
{
    CARBON_NON_NULL_OR_ERROR(type);
    CARBON_NON_NULL_OR_ERROR(field);
    *type = field->type;
    return true;
}

bool carbon_archive_table_field_get(carbon_field_t *field, size_t idx, carbon_column_t *column)
{
    CARBON_NON_NULL_OR_ERROR(field);
    CARBON_NON_NULL_OR_ERROR(column);
    if (CARBON_UNLIKELY(idx >= column->nelems)) {
        CARBON_ERROR(&column->err, CARBON_ERR_OUTOFBOUNDS);
    } else {
        carbon_off_t last = CARBON_MEMFILE_TELL(&column->context->file);
        carbon_off_t entry_off = column->entry_offsets[idx];
        carbon_memfile_seek(&column->context->file, entry_off);
        field->nentries = *CARBON_MEMFILE_READ_TYPE(&column->context->file, uint32_t);
        field->data = carbon_memfile_peek(&column->context->file, 1);
        field->type = column->type;
        field->context = column->context;
        field->data_offset = entry_off + sizeof(uint32_t);
        carbon_memfile_seek(&column->context->file, last);
        return true;
    }
    return true;
}

#define FIELD_GET_VALUE_ARRAY_GENERIC(length, field, expectedType, T)                                                  \
({                                                                                                                     \
    assert(length);                                                                                                    \
    assert(field);                                                                                                     \
    CARBON_PRINT_ERROR_AND_DIE_IF(field->type != expectedType, CARBON_ERR_ERRINTERNAL)                                 \
    *length = field->nentries;                                                                                         \
    (const T *) field->data;                                                                                           \
})

const carbon_int8_t *carbon_archive_table_field_get_int8_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_int8, carbon_int8_t);
}

const carbon_int16_t *carbon_archive_table_field_get_int16_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_int16, carbon_int16_t);
}

const carbon_int32_t *carbon_archive_table_field_get_int32_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_int32, carbon_int32_t);
}

const carbon_int64_t *carbon_archive_table_field_get_int64_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_int64, carbon_int64_t);
}

const carbon_uint8_t *carbon_archive_table_field_get_uint8_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_uint8, carbon_uint8_t);
}

const carbon_uint16_t *carbon_archive_table_field_get_uint16_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_uint16, carbon_uint16_t);
}

const carbon_uin32_t *carbon_archive_table_field_get_uint32_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_uint32, carbon_uin32_t);
}

const carbon_uin64_t *carbon_archive_table_field_get_uint64_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_uint64, carbon_uin64_t);
}

const carbon_bool_t *carbon_archive_table_field_get_bool_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_bool, carbon_bool_t);
}

const carbon_float_t *carbon_archive_table_field_get_float_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_float, carbon_float_t);
}

const carbon_string_id_t *carbon_archive_table_field_get_string_array(uint32_t *length, const carbon_field_t *field)
{
    return FIELD_GET_VALUE_ARRAY_GENERIC(length, field, carbon_field_type_string, carbon_string_id_t);
}

bool carbon_archive_table_field_get_null_array_lengths(uint32_t *length, const carbon_field_t *field)
{
    assert(length);
    assert(field);
    /** array does not carbon_parallel_map_exec to array of type NULL */
    CARBON_PRINT_ERROR_AND_DIE_IF(field->type != carbon_field_type_null, CARBON_ERR_ERRINTERNAL)
        *length = *(uint32_t *) field->data;
    return true;
}

bool carbon_archive_table_field_object_cursor_open(carbon_object_cursor_t *cursor, carbon_field_t *field)
{
    CARBON_NON_NULL_OR_ERROR(cursor);
    CARBON_NON_NULL_OR_ERROR(field);
    cursor->field = field;
    cursor->current_idx = 0;
    cursor->max_idx = field->nentries;
    cursor->mem_block = field->context->file.memblock;
    return true;
}

bool carbon_archive_table_field_object_cursor_next(carbon_archive_object_t **obj, carbon_object_cursor_t *cursor)
{
    CARBON_NON_NULL_OR_ERROR(obj);
    CARBON_NON_NULL_OR_ERROR(cursor);
    if (cursor->current_idx < cursor->max_idx) {
        carbon_off_t read_length = object_init(&cursor->obj, cursor->mem_block, cursor->field->data_offset,
                                               cursor->field->context->context);
        cursor->field->data_offset += read_length;
        carbon_memfile_t file;
        carbon_memfile_open(&file, cursor->mem_block, CARBON_MEMFILE_MODE_READONLY);
        carbon_memfile_seek(&file, cursor->field->data_offset);
        cursor->field->data_offset = *CARBON_MEMFILE_READ_TYPE(&file, carbon_off_t);
        cursor->current_idx++;
        *obj = &cursor->obj;
        return true;
    } else {
        return false;
    }
}

