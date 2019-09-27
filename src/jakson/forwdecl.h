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

#ifndef FORWDECL_H
#define FORWDECL_H

typedef struct allocator allocator;

typedef struct archive archive;
typedef struct archive_callback archive_callback;
typedef struct sid_cache_stats sid_cache_stats;
typedef struct archive_header archive_header;
typedef struct record_header record_header;
typedef struct object_header object_header;
typedef struct prop_header prop_header;
typedef union string_tab_flags string_tab_flags_u;
typedef struct string_table_header string_table_header;
typedef struct object_array_header object_array_header;
typedef struct column_group_header column_group_header;
typedef struct column_header column_header;
typedef union object_flags object_flags_u;
typedef struct archive_prop_offs archive_prop_offs;
typedef struct fixed_prop fixed_prop;
typedef struct table_prop table_prop;
typedef struct var_prop var_prop;
typedef struct array_prop array_prop;
typedef struct null_prop null_prop;
typedef struct record_flags record_flags;
typedef struct string_table string_table;
typedef struct record_table record_table;
typedef struct archive_info archive_info;
typedef struct string_entry_header string_entry_header;
typedef struct archive_io_context archive_io_context;
typedef struct archive_object archive_object;
typedef struct collection_iter_state collection_iter_state;
typedef struct archive_value_vector archive_value_vector;
typedef struct prop_iter prop_iter;
typedef struct independent_iter_state independent_iter_state;
typedef struct column_object_iter column_object_iter;
typedef struct string_pred string_pred;
typedef struct query query;
typedef struct strid_info strid_info;
typedef struct strid_iter strid_iter;
typedef struct path_entry path_entry;
typedef struct archive_visitor_desc archive_visitor_desc;
typedef struct visitor visitor;
typedef struct column_doc_column column_doc_column;
typedef struct column_doc_group column_doc_group;
typedef struct column_doc_obj column_doc_obj;
typedef struct column_doc column_doc;
typedef struct doc_entries doc_entries;
typedef struct doc_bulk doc_bulk;
typedef struct doc doc;
typedef struct doc_obj doc_obj;
typedef union encoded_doc_value encoded_doc_value_u;
typedef struct encoded_doc_prop_header encoded_doc_prop_header;
typedef struct encoded_doc_prop encoded_doc_prop;
typedef struct encoded_doc_prop_array encoded_doc_prop_array;
typedef struct encoded_doc encoded_doc;
typedef struct encoded_doc_list encoded_doc_list;

typedef struct err err;
typedef struct fn_result fn_result;

typedef struct async_func_proxy async_func_proxy;
typedef struct filter_arg filter_arg;
typedef struct map_args map_args;
typedef struct gather_scatter_args gather_scatter_args;

typedef struct bitmap bitmap;

typedef struct carbon carbon;
typedef struct carbon_insert carbon_insert;
typedef struct carbon_new carbon_new;
typedef struct field_access field_access;
typedef struct carbon_array_it carbon_array_it;
typedef struct carbon_column_it carbon_column_it;
typedef struct carbon_dot_node carbon_dot_node;
typedef struct carbon_dot_path carbon_dot_path;
typedef struct carbon_find carbon_find;
typedef struct carbon_insert_array_state carbon_insert_array_state;
typedef struct carbon_insert_object_state carbon_insert_object_state;
typedef struct carbon_insert_column_state carbon_insert_column_state;
typedef struct carbon_object_it carbon_object_it;
typedef struct carbon_path_evaluator carbon_path_evaluator;
typedef struct carbon_path_index carbon_path_index;
typedef struct carbon_path_index_it carbon_path_index_it;
typedef struct carbon_printer carbon_printer;
typedef struct carbon_revise carbon_revise;
typedef struct carbon_binary carbon_binary;
typedef struct carbon_update carbon_update;
typedef struct packer packer;

typedef struct hashset_bucket hashset_bucket;
typedef struct hashset hashset;
typedef struct hashtable_bucket hashtable_bucket;
typedef struct hashtable hashtable;
typedef struct huffman huffman;
typedef struct pack_huffman_entry pack_huffman_entry;
typedef struct pack_huffman_info pack_huffman_info;
typedef struct pack_huffman_str_info pack_huffman_str_info;

typedef struct json json;
typedef struct json_token json_token;
typedef struct json_err json_err;
typedef struct json_tokenizer json_tokenizer;
typedef struct json_parser json_parser;
typedef struct json json;
typedef struct json_node_value json_node_value;
typedef struct json_object json_object;
typedef struct json_element json_element;
typedef struct json_string json_string;
typedef struct json_prop json_prop;
typedef struct json_members json_members;
typedef struct json_elements json_elements;
typedef struct json_array json_array;
typedef struct json_number json_number;

typedef struct memblock memblock;
typedef struct memfile memfile;

typedef struct command_opt command_opt;
typedef struct command_opt_group command_opt_group;
typedef struct command_opt_mgr command_opt_mgr;

typedef struct priority_queue_element_info priority_queue_element_info;
typedef struct priority_queue priority_queue;

typedef struct slice slice;
typedef struct hash_bounds hash_bounds;
typedef struct slice_descriptor slice_descriptor;
typedef struct slice_list slice_list;
typedef struct slice_handle slice_handle;

typedef struct spinlock spinlock;

typedef struct vector vector;

typedef struct str_hash str_hash;
typedef struct str_hash_counters str_hash_counters;

typedef struct string_buffer string_buffer;

typedef struct string_dict string_dict;

typedef struct thread_task thread_task;
typedef struct task_state task_state;
typedef struct task_handle task_handle;
typedef struct thread_pool thread_pool;
typedef struct thread_info thread_info;
typedef struct thread_pool_stats thread_pool_stats;
typedef struct thread_stats thread_stats;
typedef struct task_stats task_stats;

#endif
