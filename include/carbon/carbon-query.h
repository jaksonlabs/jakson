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




//
//
//
//CARBON_EXPORT(bool)
//carbon_archive_record(carbon_archive_object_t *root, carbon_query_t *query);
//
//CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
//carbon_archive_object_keys_to_type(CARBON_NULLABLE size_t *npairs, carbon_basic_type_e type, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(CARBON_NULLABLE const carbon_string_id_t *)
//carbon_archive_object_keys_to_array(CARBON_NULLABLE size_t *npairs, carbon_basic_type_e type, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(bool)
//carbon_archive_object_values_object(carbon_archive_object_t *out, size_t idx, carbon_archive_object_t *props);
//
//CARBON_EXPORT(const carbon_int8_t *)
//carbon_archive_object_values_int8(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int16_t *)
//carbon_archive_object_values_int16s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int32_t *)
//carbon_archive_object_values_int32(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int64_t *)
//carbon_archive_object_values_int64s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint8_t *)
//carbon_archive_object_values_uint8s(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint16_t *)
//carbon_archive_object_values_uin16(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint32_t *)
//carbon_archive_object_values_uint32(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint64_t *)
//carbon_archive_object_values_uint64(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_bool_t *)
//carbon_archive_object_values_bool(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_float_t *)
//carbon_archive_object_values_float(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_string_id_t *)
//carbon_archive_object_values_strings(CARBON_NULLABLE size_t *npairs, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int8_t *)
//carbon_archive_object_values_int8_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int16_t *)
//carbon_archive_object_values_int16_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int32_t *)
//carbon_archive_object_values_int32_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_int64_t *)
//carbon_archive_object_values_int64_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint8_t *)
//carbon_archive_object_values_uint8_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint16_t *)
//carbon_archive_object_values_uint16_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint32_t *)
//carbon_archive_object_values_uint32_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_uint64_t *)
//carbon_archive_object_values_uint64_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_bool_t *)
//carbon_archive_object_values_bool_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_float_t *)
//carbon_archive_object_values_float_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(const carbon_string_id_t *)
//carbon_archive_object_values_string_arrays(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(bool)
//carbon_archive_object_values_null_array_lengths(uint32_t *length, size_t idx, carbon_archive_object_t *obj);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_open(carbon_archive_table_t *out, carbon_archive_object_t *obj);
//
//CARBON_DEFINE_GET_ERROR_FUNCTION(carbon_archive_table, carbon_archive_table_t, table);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_column_group(carbon_column_group_t *group, size_t idx, carbon_archive_table_t *table);
//
//CARBON_DEFINE_GET_ERROR_FUNCTION(archive_column_group, carbon_column_group_t, group);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_column(carbon_column_t *column, size_t idx, carbon_column_group_t *group);
//
//CARBON_DEFINE_GET_ERROR_FUNCTION(archive_column, carbon_column_t, column);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_field_get(carbon_field_t *field, size_t idx, carbon_column_t *column);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_field_type(carbon_field_type_e *type, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_int8_t *)
//carbon_archive_table_field_get_int8_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_int16_t *)
//carbon_archive_table_field_get_int16_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_int32_t *)
//carbon_archive_table_field_get_int32_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_int64_t *)
//carbon_archive_table_field_get_int64_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_uint8_t *)
//carbon_archive_table_field_get_uint8_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_uint16_t *)
//carbon_archive_table_field_get_uint16_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_uint32_t *)
//carbon_archive_table_field_get_uint32_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_uint64_t *)
//carbon_archive_table_field_get_uint64_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_bool_t *)
//carbon_archive_table_field_get_bool_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_float_t *)
//carbon_archive_table_field_get_float_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(const carbon_string_id_t *)
//carbon_archive_table_field_get_string_array(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_field_get_null_array_lengths(uint32_t *length, const carbon_field_t *field);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_field_object_cursor_open(carbon_object_cursor_t *cursor, carbon_field_t *field);
//
//CARBON_EXPORT(bool)
//carbon_archive_table_field_object_cursor_next(carbon_archive_object_t **obj, carbon_object_cursor_t *cursor);



CARBON_END_DECL

#endif
