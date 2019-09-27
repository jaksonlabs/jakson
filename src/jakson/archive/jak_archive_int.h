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

#ifndef JAK_INTERNALS_ARCHIVE_H
#define JAK_INTERNALS_ARCHIVE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/memfile/jak_memfile.h>
#include <jakson/types.h>
#include <jakson/stdx/jak_unique_id.h>
#include <jakson/archive/jak_pack.h>

JAK_BEGIN_DECL

typedef struct __attribute__((packed)) jak_archive_header {
        char magic[9];
        jak_u8 version;
        jak_offset_t root_object_header_offset;
        jak_offset_t jak_string_id_to_offset_index_offset;
} jak_archive_header;

typedef struct __attribute__((packed)) jak_record_header {
        char marker;
        jak_u8 flags;
        jak_u64 record_size;
} jak_record_header;

typedef struct __attribute__((packed)) jak_object_header {
        char marker;
        jak_uid_t oid;
        jak_u32 flags;
} jak_object_header;

typedef struct __attribute__((packed)) jak_prop_header {
        char marker;
        jak_u32 num_entries;
} jak_prop_header;

typedef union __attribute__((packed)) jak_string_tab_flags {
        struct {
                jak_u8 compressor_none
                        : 1;
                jak_u8 compressed_huffman
                        : 1;
        } bits;
        jak_u8 value;
} jak_string_tab_flags_u;

typedef struct __attribute__((packed)) jak_string_table_header {
        char marker;
        jak_u32 num_entries;
        jak_u8 flags;
        jak_offset_t first_entry;
        jak_offset_t compressor_extra_size;
} jak_string_table_header;

typedef struct __attribute__((packed)) jak_object_array_header {
        char marker;
        jak_u8 num_entries;
} jak_object_array_header;

typedef struct __attribute__((packed)) jak_column_group_header {
        char marker;
        jak_u32 num_columns;
        jak_u32 num_objects;
} jak_column_group_header;

typedef struct __attribute__((packed)) jak_column_header {
        char marker;
        jak_archive_field_sid_t column_name;
        char value_type;
        jak_u32 num_entries;
} jak_column_header;

typedef union jak_object_flags {
        struct {
                jak_u32 has_null_props
                        : 1;
                jak_u32 has_bool_props
                        : 1;
                jak_u32 has_int8_props
                        : 1;
                jak_u32 has_int16_props
                        : 1;
                jak_u32 has_int32_props
                        : 1;
                jak_u32 has_int64_props
                        : 1;
                jak_u32 has_uint8_props
                        : 1;
                jak_u32 has_uint16_props
                        : 1;
                jak_u32 has_uint32_props
                        : 1;
                jak_u32 has_uint64_props
                        : 1;
                jak_u32 has_float_props
                        : 1;
                jak_u32 has_jak_string_props
                        : 1;
                jak_u32 has_object_props
                        : 1;
                jak_u32 has_null_array_props
                        : 1;
                jak_u32 has_bool_array_props
                        : 1;
                jak_u32 has_int8_array_props
                        : 1;
                jak_u32 has_int16_array_props
                        : 1;
                jak_u32 has_int32_array_props
                        : 1;
                jak_u32 has_int64_array_props
                        : 1;
                jak_u32 has_uint8_array_props
                        : 1;
                jak_u32 has_uint16_array_props
                        : 1;
                jak_u32 has_uint32_array_props
                        : 1;
                jak_u32 has_uint64_array_props
                        : 1;
                jak_u32 has_float_array_props
                        : 1;
                jak_u32 has_jak_string_array_props
                        : 1;
                jak_u32 has_object_array_props
                        : 1;
                jak_u32 RESERVED_27
                        : 1;
                jak_u32 RESERVED_28
                        : 1;
                jak_u32 RESERVED_29
                        : 1;
                jak_u32 RESERVED_30
                        : 1;
                jak_u32 RESERVED_31
                        : 1;
                jak_u32 RESERVED_32
                        : 1;
        } bits;
        jak_u32 value;
} jak_object_flags_u;

typedef struct jak_archive_prop_offs {
        jak_offset_t nulls;
        jak_offset_t bools;
        jak_offset_t int8s;
        jak_offset_t int16s;
        jak_offset_t int32s;
        jak_offset_t int64s;
        jak_offset_t uint8s;
        jak_offset_t uint16s;
        jak_offset_t uint32s;
        jak_offset_t uint64s;
        jak_offset_t floats;
        jak_offset_t strings;
        jak_offset_t objects;
        jak_offset_t null_arrays;
        jak_offset_t bool_arrays;
        jak_offset_t int8_arrays;
        jak_offset_t int16_arrays;
        jak_offset_t int32_arrays;
        jak_offset_t int64_arrays;
        jak_offset_t uint8_arrays;
        jak_offset_t uint16_arrays;
        jak_offset_t uint32_arrays;
        jak_offset_t uint64_arrays;
        jak_offset_t float_arrays;
        jak_offset_t jak_string_arrays;
        jak_offset_t object_arrays;
} jak_archive_prop_offs;

typedef struct jak_fixed_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const void *values;
} jak_fixed_prop;

typedef struct jak_table_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_offset_t *group_offs;
} jak_table_prop;

typedef struct jak_var_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_offset_t *offsets;
        const void *values;
} jak_var_prop;

typedef struct jak_array_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
        const jak_u32 *lengths;
        jak_offset_t values_begin;
} jak_array_prop;

typedef struct jak_null_prop {
        jak_prop_header *header;
        const jak_archive_field_sid_t *keys;
} jak_null_prop;

typedef enum jak_archive_marker {
        JAK_MARKER_TYPE_OBJECT_BEGIN = 0,
        JAK_MARKER_TYPE_OBJECT_END = 1,
        JAK_MARKER_TYPE_PROP_NULL = 2,
        JAK_MARKER_TYPE_PROP_BOOLEAN = 3,
        JAK_MARKER_TYPE_PROP_INT8 = 4,
        JAK_MARKER_TYPE_PROP_INT16 = 5,
        JAK_MARKER_TYPE_PROP_INT32 = 6,
        JAK_MARKER_TYPE_PROP_INT64 = 7,
        JAK_MARKER_TYPE_PROP_UINT8 = 8,
        JAK_MARKER_TYPE_PROP_UINT16 = 9,
        JAK_MARKER_TYPE_PROP_UINT32 = 10,
        JAK_MARKER_TYPE_PROP_UINT64 = 11,
        JAK_MARKER_TYPE_PROP_REAL = 12,
        JAK_MARKER_TYPE_PROP_TEXT = 13,
        JAK_MARKER_TYPE_PROP_OBJECT = 14,
        JAK_MARKER_TYPE_PROP_NULL_ARRAY = 15,
        JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY = 16,
        JAK_MARKER_TYPE_PROP_INT8_ARRAY = 17,
        JAK_MARKER_TYPE_PROP_INT16_ARRAY = 18,
        JAK_MARKER_TYPE_PROP_INT32_ARRAY = 19,
        JAK_MARKER_TYPE_PROP_INT64_ARRAY = 20,
        JAK_MARKER_TYPE_PROP_UINT8_ARRAY = 21,
        JAK_MARKER_TYPE_PROP_UINT16_ARRAY = 22,
        JAK_MARKER_TYPE_PROP_UINT32_ARRAY = 23,
        JAK_MARKER_TYPE_PROP_UINT64_ARRAY = 24,
        JAK_MARKER_TYPE_PROP_REAL_ARRAY = 25,
        JAK_MARKER_TYPE_PROP_TEXT_ARRAY = 26,
        JAK_MARKER_TYPE_PROP_OBJECT_ARRAY = 27,
        JAK_MARKER_TYPE_EMBEDDED_STR_DIC = 28,
        JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR = 29,
        JAK_MARKER_TYPE_COLUMN_GROUP = 30,
        JAK_MARKER_TYPE_COLUMN = 31,
        JAK_MARKER_TYPE_HUFFMAN_DIC_ENTRY = 32,
        JAK_MARKER_TYPE_RECORD_HEADER = 33,
} jak_archive_marker_e;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static jak_archive_header this_file_header = {.version = JAK_CARBON_ARCHIVE_VERSION, .root_object_header_offset = 0};

static struct {
        jak_archive_marker_e type;
        char symbol;
} jak_global_marker_symbols[] =
        {{JAK_MARKER_TYPE_OBJECT_BEGIN,        JAK_MARKER_SYMBOL_OBJECT_BEGIN},
         {JAK_MARKER_TYPE_OBJECT_END,          JAK_MARKER_SYMBOL_OBJECT_END},
         {JAK_MARKER_TYPE_PROP_NULL,           JAK_MARKER_SYMBOL_PROP_NULL},
         {JAK_MARKER_TYPE_PROP_BOOLEAN,        JAK_MARKER_SYMBOL_PROP_BOOLEAN},
         {JAK_MARKER_TYPE_PROP_INT8,           JAK_MARKER_SYMBOL_PROP_INT8},
         {JAK_MARKER_TYPE_PROP_INT16,          JAK_MARKER_SYMBOL_PROP_INT16},
         {JAK_MARKER_TYPE_PROP_INT32,          JAK_MARKER_SYMBOL_PROP_INT32},
         {JAK_MARKER_TYPE_PROP_INT64,          JAK_MARKER_SYMBOL_PROP_INT64},
         {JAK_MARKER_TYPE_PROP_UINT8,          JAK_MARKER_SYMBOL_PROP_UINT8},
         {JAK_MARKER_TYPE_PROP_UINT16,         JAK_MARKER_SYMBOL_PROP_UINT16},
         {JAK_MARKER_TYPE_PROP_UINT32,         JAK_MARKER_SYMBOL_PROP_UINT32},
         {JAK_MARKER_TYPE_PROP_UINT64,         JAK_MARKER_SYMBOL_PROP_UINT64},
         {JAK_MARKER_TYPE_PROP_REAL,           JAK_MARKER_SYMBOL_PROP_REAL},
         {JAK_MARKER_TYPE_PROP_TEXT,           JAK_MARKER_SYMBOL_PROP_TEXT},
         {JAK_MARKER_TYPE_PROP_OBJECT,         JAK_MARKER_SYMBOL_PROP_OBJECT},
         {JAK_MARKER_TYPE_PROP_NULL_ARRAY,     JAK_MARKER_SYMBOL_PROP_NULL_ARRAY},
         {JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY,  JAK_MARKER_SYMBOL_PROP_BOOLEAN_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT8_ARRAY,     JAK_MARKER_SYMBOL_PROP_INT8_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT16_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT16_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT32_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT32_ARRAY},
         {JAK_MARKER_TYPE_PROP_INT64_ARRAY,    JAK_MARKER_SYMBOL_PROP_INT64_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT8_ARRAY,    JAK_MARKER_SYMBOL_PROP_UINT8_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT16_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT16_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT32_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT32_ARRAY},
         {JAK_MARKER_TYPE_PROP_UINT64_ARRAY,   JAK_MARKER_SYMBOL_PROP_UINT64_ARRAY},
         {JAK_MARKER_TYPE_PROP_REAL_ARRAY,     JAK_MARKER_SYMBOL_PROP_REAL_ARRAY},
         {JAK_MARKER_TYPE_PROP_TEXT_ARRAY,     JAK_MARKER_SYMBOL_PROP_TEXT_ARRAY},
         {JAK_MARKER_TYPE_PROP_OBJECT_ARRAY,   JAK_MARKER_SYMBOL_PROP_OBJECT_ARRAY},
         {JAK_MARKER_TYPE_EMBEDDED_STR_DIC,    JAK_MARKER_SYMBOL_EMBEDDED_STR_DIC},
         {JAK_MARKER_TYPE_EMBEDDED_UNCOMP_STR, JAK_MARKER_SYMBOL_EMBEDDED_STR},
         {JAK_MARKER_TYPE_COLUMN_GROUP,        JAK_MARKER_SYMBOL_COLUMN_GROUP},
         {JAK_MARKER_TYPE_COLUMN,              JAK_MARKER_SYMBOL_COLUMN},
         {JAK_MARKER_TYPE_HUFFMAN_DIC_ENTRY,   JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY},
         {JAK_MARKER_TYPE_RECORD_HEADER,       JAK_MARKER_SYMBOL_RECORD_HEADER}};

static struct {
        jak_archive_field_e value_type;
        jak_archive_marker_e marker;
} jak_global_value_array_marker_mapping[] =
        {{JAK_FIELD_NULL,    JAK_MARKER_TYPE_PROP_NULL_ARRAY},
         {JAK_FIELD_BOOLEAN, JAK_MARKER_TYPE_PROP_BOOLEAN_ARRAY},
         {JAK_FIELD_INT8,    JAK_MARKER_TYPE_PROP_INT8_ARRAY},
         {JAK_FIELD_INT16,   JAK_MARKER_TYPE_PROP_INT16_ARRAY},
         {JAK_FIELD_INT32,   JAK_MARKER_TYPE_PROP_INT32_ARRAY},
         {JAK_FIELD_INT64,   JAK_MARKER_TYPE_PROP_INT64_ARRAY},
         {JAK_FIELD_UINT8,   JAK_MARKER_TYPE_PROP_UINT8_ARRAY},
         {JAK_FIELD_UINT16,  JAK_MARKER_TYPE_PROP_UINT16_ARRAY},
         {JAK_FIELD_UINT32,  JAK_MARKER_TYPE_PROP_UINT32_ARRAY},
         {JAK_FIELD_UINT64,  JAK_MARKER_TYPE_PROP_UINT64_ARRAY},
         {JAK_FIELD_FLOAT,   JAK_MARKER_TYPE_PROP_REAL_ARRAY},
         {JAK_FIELD_STRING,  JAK_MARKER_TYPE_PROP_TEXT_ARRAY},
         {JAK_FIELD_OBJECT,  JAK_MARKER_TYPE_PROP_OBJECT_ARRAY}}, valueMarkerMapping[] =
        {{JAK_FIELD_NULL,    JAK_MARKER_TYPE_PROP_NULL},
         {JAK_FIELD_BOOLEAN, JAK_MARKER_TYPE_PROP_BOOLEAN},
         {JAK_FIELD_INT8,    JAK_MARKER_TYPE_PROP_INT8},
         {JAK_FIELD_INT16,   JAK_MARKER_TYPE_PROP_INT16},
         {JAK_FIELD_INT32,   JAK_MARKER_TYPE_PROP_INT32},
         {JAK_FIELD_INT64,   JAK_MARKER_TYPE_PROP_INT64},
         {JAK_FIELD_UINT8,   JAK_MARKER_TYPE_PROP_UINT8},
         {JAK_FIELD_UINT16,  JAK_MARKER_TYPE_PROP_UINT16},
         {JAK_FIELD_UINT32,  JAK_MARKER_TYPE_PROP_UINT32},
         {JAK_FIELD_UINT64,  JAK_MARKER_TYPE_PROP_UINT64},
         {JAK_FIELD_FLOAT,   JAK_MARKER_TYPE_PROP_REAL},
         {JAK_FIELD_STRING,  JAK_MARKER_TYPE_PROP_TEXT},
         {JAK_FIELD_OBJECT,  JAK_MARKER_TYPE_PROP_OBJECT}};

#pragma GCC diagnostic pop

typedef struct jak_record_flags {
        struct {
                jak_u8 is_sorted
                        : 1;
                jak_u8 RESERVED_2
                        : 1;
                jak_u8 RESERVED_3
                        : 1;
                jak_u8 RESERVED_4
                        : 1;
                jak_u8 RESERVED_5
                        : 1;
                jak_u8 RESERVED_6
                        : 1;
                jak_u8 RESERVED_7
                        : 1;
                jak_u8 RESERVED_8
                        : 1;
        } bits;
        jak_u8 value;
} jak_record_flags;

typedef struct jak_string_table {
        jak_packer compressor;
        jak_offset_t first_entry_off;
        jak_u32 num_embeddded_strings;
} jak_string_table;

typedef struct jak_record_table {
        jak_record_flags flags;
        jak_memblock *record_db;
} jak_record_table;

typedef struct jak_archive_info {
        size_t jak_string_table_size;
        size_t record_table_size;
        size_t jak_string_id_index_size;
        jak_u32 num_embeddded_strings;
} jak_archive_info;

typedef struct __attribute__((packed)) jak_string_entry_header {
        char marker;
        jak_offset_t next_entry_off;
        jak_archive_field_sid_t jak_string_id;
        jak_u32 jak_string_len;
} jak_string_entry_header;

void jak_int_read_prop_offsets(jak_archive_prop_offs *prop_offsets, jak_memfile *memfile, const jak_object_flags_u *flags);
void jak_int_embedded_fixed_props_read(jak_fixed_prop *prop, jak_memfile *memfile);
void jak_int_embedded_var_props_read(jak_var_prop *prop, jak_memfile *memfile);
void jak_int_embedded_null_props_read(jak_null_prop *prop, jak_memfile *memfile);
void jak_int_embedded_array_props_read(jak_array_prop *prop, jak_memfile *memfile);
void jak_int_embedded_table_props_read(jak_table_prop *prop, jak_memfile *memfile);
jak_archive_field_e jak_int_get_value_type_of_char(char c);
jak_archive_field_e jak_int_marker_to_field_type(char symbol);

JAK_END_DECL

#endif
