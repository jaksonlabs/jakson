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

#ifndef JAK_FORWDECL_H
#define JAK_FORWDECL_H

typedef struct jak_allocator jak_allocator;

typedef struct jak_archive jak_archive;
typedef struct jak_archive_callback jak_archive_callback;
typedef struct jak_sid_cache_stats jak_sid_cache_stats;
typedef struct jak_archive_header jak_archive_header;
typedef struct jak_record_header jak_record_header;
typedef struct jak_object_header jak_object_header;
typedef struct jak_prop_header jak_prop_header;
typedef union jak_string_tab_flags jak_string_tab_flags_u;
typedef struct jak_string_table_header jak_string_table_header;
typedef struct jak_object_array_header jak_object_array_header;
typedef struct jak_column_group_header jak_column_group_header;
typedef struct jak_column_header jak_column_header;
typedef union jak_object_flags jak_object_flags_u;
typedef struct jak_archive_prop_offs jak_archive_prop_offs;
typedef struct jak_fixed_prop jak_fixed_prop;
typedef struct jak_table_prop jak_table_prop;
typedef struct jak_var_prop jak_var_prop;
typedef struct jak_array_prop jak_array_prop;
typedef struct jak_null_prop jak_null_prop;
typedef struct jak_record_flags jak_record_flags;
typedef struct jak_string_table jak_string_table;
typedef struct jak_record_table jak_record_table;
typedef struct jak_archive_info jak_archive_info;
typedef struct jak_string_entry_header jak_string_entry_header;
typedef struct jak_archive_io_context jak_archive_io_context;
typedef struct jak_archive_object jak_archive_object;
typedef struct jak_collection_iter_state jak_collection_iter_state;
typedef struct jak_archive_value_vector jak_archive_value_vector;
typedef struct jak_prop_iter jak_prop_iter;
typedef struct jak_independent_iter_state jak_independent_iter_state;
typedef struct jak_column_object_iter jak_column_object_iter;
typedef struct jak_string_pred jak_string_pred;
typedef struct jak_archive_query jak_archive_query;
typedef struct jak_strid_info jak_strid_info;
typedef struct jak_strid_iter jak_strid_iter;
typedef struct jak_path_entry jak_path_entry;
typedef struct jak_archive_visitor_desc jak_archive_visitor_desc;
typedef struct jak_archive_visitor jak_archive_visitor;

typedef struct jak_async_func_proxy jak_async_func_proxy;
typedef struct jak_filter_arg jak_filter_arg;
typedef struct jak_map_args jak_map_args;
typedef struct jak_gather_scatter_args jak_gather_scatter_args;

typedef struct jak_bitmap jak_bitmap;

typedef struct jak_carbon jak_carbon;
typedef struct jak_carbon_insert jak_carbon_insert;
typedef struct jak_carbon_new jak_carbon_new;
typedef struct jak_field_access jak_field_access;
typedef struct jak_carbon_array_it jak_carbon_array_it;

#endif
