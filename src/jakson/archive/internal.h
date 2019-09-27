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

#ifndef INTERNALS_ARCHIVE_H
#define INTERNALS_ARCHIVE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/mem/file.h>
#include <jakson/types.h>
#include <jakson/stdx/unique_id.h>
#include <jakson/archive/pack.h>

BEGIN_DECL

typedef struct __attribute__((packed)) archive_header {
        char magic[9];
        u8 version;
        offset_t root_object_header_offset;
        offset_t string_id_to_offset_index_offset;
} archive_header;

typedef struct __attribute__((packed)) record_header {
        char marker;
        u8 flags;
        u64 record_size;
} record_header;

typedef struct __attribute__((packed)) object_header {
        char marker;
        unique_id_t oid;
        u32 flags;
} object_header;

typedef struct __attribute__((packed)) prop_header {
        char marker;
        u32 num_entries;
} prop_header;

typedef union __attribute__((packed)) string_tab_flags {
        struct {
                u8 compressor_none
                        : 1;
                u8 compressed_huffman
                        : 1;
        } bits;
        u8 value;
} string_tab_flags_u;

typedef struct __attribute__((packed)) string_table_header {
        char marker;
        u32 num_entries;
        u8 flags;
        offset_t first_entry;
        offset_t compressor_extra_size;
} string_table_header;

typedef struct __attribute__((packed)) object_array_header {
        char marker;
        u8 num_entries;
} object_array_header;

typedef struct __attribute__((packed)) column_group_header {
        char marker;
        u32 num_columns;
        u32 num_objects;
} column_group_header;

typedef struct __attribute__((packed)) column_header {
        char marker;
        archive_field_sid_t column_name;
        char value_type;
        u32 num_entries;
} column_header;

typedef union object_flags {
        struct {
                u32 has_null_props
                        : 1;
                u32 has_bool_props
                        : 1;
                u32 has_int8_props
                        : 1;
                u32 has_int16_props
                        : 1;
                u32 has_int32_props
                        : 1;
                u32 has_int64_props
                        : 1;
                u32 has_uint8_props
                        : 1;
                u32 has_uint16_props
                        : 1;
                u32 has_uint32_props
                        : 1;
                u32 has_uint64_props
                        : 1;
                u32 has_float_props
                        : 1;
                u32 has_string_props
                        : 1;
                u32 has_object_props
                        : 1;
                u32 has_null_array_props
                        : 1;
                u32 has_bool_array_props
                        : 1;
                u32 has_int8_array_props
                        : 1;
                u32 has_int16_array_props
                        : 1;
                u32 has_int32_array_props
                        : 1;
                u32 has_int64_array_props
                        : 1;
                u32 has_uint8_array_props
                        : 1;
                u32 has_uint16_array_props
                        : 1;
                u32 has_uint32_array_props
                        : 1;
                u32 has_uint64_array_props
                        : 1;
                u32 has_float_array_props
                        : 1;
                u32 has_string_array_props
                        : 1;
                u32 has_object_array_props
                        : 1;
                u32 RESERVED_27
                        : 1;
                u32 RESERVED_28
                        : 1;
                u32 RESERVED_29
                        : 1;
                u32 RESERVED_30
                        : 1;
                u32 RESERVED_31
                        : 1;
                u32 RESERVED_32
                        : 1;
        } bits;
        u32 value;
} object_flags_u;

typedef struct archive_prop_offs {
        offset_t nulls;
        offset_t bools;
        offset_t int8s;
        offset_t int16s;
        offset_t int32s;
        offset_t int64s;
        offset_t uint8s;
        offset_t uint16s;
        offset_t uint32s;
        offset_t uint64s;
        offset_t floats;
        offset_t strings;
        offset_t objects;
        offset_t null_arrays;
        offset_t bool_arrays;
        offset_t int8_arrays;
        offset_t int16_arrays;
        offset_t int32_arrays;
        offset_t int64_arrays;
        offset_t uint8_arrays;
        offset_t uint16_arrays;
        offset_t uint32_arrays;
        offset_t uint64_arrays;
        offset_t float_arrays;
        offset_t string_arrays;
        offset_t object_arrays;
} archive_prop_offs;

typedef struct fixed_prop {
        prop_header *header;
        const archive_field_sid_t *keys;
        const void *values;
} fixed_prop;

typedef struct table_prop {
        prop_header *header;
        const archive_field_sid_t *keys;
        const offset_t *group_offs;
} table_prop;

typedef struct var_prop {
        prop_header *header;
        const archive_field_sid_t *keys;
        const offset_t *offsets;
        const void *values;
} var_prop;

typedef struct array_prop {
        prop_header *header;
        const archive_field_sid_t *keys;
        const u32 *lengths;
        offset_t values_begin;
} array_prop;

typedef struct null_prop {
        prop_header *header;
        const archive_field_sid_t *keys;
} null_prop;

typedef enum archive_marker {
        MARKER_TYPE_OBJECT_BEGIN = 0,
        MARKER_TYPE_OBJECT_END = 1,
        MARKER_TYPE_PROP_NULL = 2,
        MARKER_TYPE_PROP_BOOLEAN = 3,
        MARKER_TYPE_PROP_INT8 = 4,
        MARKER_TYPE_PROP_INT16 = 5,
        MARKER_TYPE_PROP_INT32 = 6,
        MARKER_TYPE_PROP_INT64 = 7,
        MARKER_TYPE_PROP_UINT8 = 8,
        MARKER_TYPE_PROP_UINT16 = 9,
        MARKER_TYPE_PROP_UINT32 = 10,
        MARKER_TYPE_PROP_UINT64 = 11,
        MARKER_TYPE_PROP_REAL = 12,
        MARKER_TYPE_PROP_TEXT = 13,
        MARKER_TYPE_PROP_OBJECT = 14,
        MARKER_TYPE_PROP_NULL_ARRAY = 15,
        MARKER_TYPE_PROP_BOOLEAN_ARRAY = 16,
        MARKER_TYPE_PROP_INT8_ARRAY = 17,
        MARKER_TYPE_PROP_INT16_ARRAY = 18,
        MARKER_TYPE_PROP_INT32_ARRAY = 19,
        MARKER_TYPE_PROP_INT64_ARRAY = 20,
        MARKER_TYPE_PROP_UINT8_ARRAY = 21,
        MARKER_TYPE_PROP_UINT16_ARRAY = 22,
        MARKER_TYPE_PROP_UINT32_ARRAY = 23,
        MARKER_TYPE_PROP_UINT64_ARRAY = 24,
        MARKER_TYPE_PROP_REAL_ARRAY = 25,
        MARKER_TYPE_PROP_TEXT_ARRAY = 26,
        MARKER_TYPE_PROP_OBJECT_ARRAY = 27,
        MARKER_TYPE_EMBEDDED_STR_DIC = 28,
        MARKER_TYPE_EMBEDDED_UNCOMP_STR = 29,
        MARKER_TYPE_COLUMN_GROUP = 30,
        MARKER_TYPE_COLUMN = 31,
        MARKER_TYPE_HUFFMAN_DIC_ENTRY = 32,
        MARKER_TYPE_RECORD_HEADER = 33,
} archive_marker_e;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static archive_header this_file_header = {.version = CARBON_ARCHIVE_VERSION, .root_object_header_offset = 0};

static struct {
        archive_marker_e type;
        char symbol;
} global_marker_symbols[] =
        {{MARKER_TYPE_OBJECT_BEGIN,        MARKER_SYMBOL_OBJECT_BEGIN},
         {MARKER_TYPE_OBJECT_END,          MARKER_SYMBOL_OBJECT_END},
         {MARKER_TYPE_PROP_NULL,           MARKER_SYMBOL_PROP_NULL},
         {MARKER_TYPE_PROP_BOOLEAN,        MARKER_SYMBOL_PROP_BOOLEAN},
         {MARKER_TYPE_PROP_INT8,           MARKER_SYMBOL_PROP_INT8},
         {MARKER_TYPE_PROP_INT16,          MARKER_SYMBOL_PROP_INT16},
         {MARKER_TYPE_PROP_INT32,          MARKER_SYMBOL_PROP_INT32},
         {MARKER_TYPE_PROP_INT64,          MARKER_SYMBOL_PROP_INT64},
         {MARKER_TYPE_PROP_UINT8,          MARKER_SYMBOL_PROP_UINT8},
         {MARKER_TYPE_PROP_UINT16,         MARKER_SYMBOL_PROP_UINT16},
         {MARKER_TYPE_PROP_UINT32,         MARKER_SYMBOL_PROP_UINT32},
         {MARKER_TYPE_PROP_UINT64,         MARKER_SYMBOL_PROP_UINT64},
         {MARKER_TYPE_PROP_REAL,           MARKER_SYMBOL_PROP_REAL},
         {MARKER_TYPE_PROP_TEXT,           MARKER_SYMBOL_PROP_TEXT},
         {MARKER_TYPE_PROP_OBJECT,         MARKER_SYMBOL_PROP_OBJECT},
         {MARKER_TYPE_PROP_NULL_ARRAY,     MARKER_SYMBOL_PROP_NULL_ARRAY},
         {MARKER_TYPE_PROP_BOOLEAN_ARRAY,  MARKER_SYMBOL_PROP_BOOLEAN_ARRAY},
         {MARKER_TYPE_PROP_INT8_ARRAY,     MARKER_SYMBOL_PROP_INT8_ARRAY},
         {MARKER_TYPE_PROP_INT16_ARRAY,    MARKER_SYMBOL_PROP_INT16_ARRAY},
         {MARKER_TYPE_PROP_INT32_ARRAY,    MARKER_SYMBOL_PROP_INT32_ARRAY},
         {MARKER_TYPE_PROP_INT64_ARRAY,    MARKER_SYMBOL_PROP_INT64_ARRAY},
         {MARKER_TYPE_PROP_UINT8_ARRAY,    MARKER_SYMBOL_PROP_UINT8_ARRAY},
         {MARKER_TYPE_PROP_UINT16_ARRAY,   MARKER_SYMBOL_PROP_UINT16_ARRAY},
         {MARKER_TYPE_PROP_UINT32_ARRAY,   MARKER_SYMBOL_PROP_UINT32_ARRAY},
         {MARKER_TYPE_PROP_UINT64_ARRAY,   MARKER_SYMBOL_PROP_UINT64_ARRAY},
         {MARKER_TYPE_PROP_REAL_ARRAY,     MARKER_SYMBOL_PROP_REAL_ARRAY},
         {MARKER_TYPE_PROP_TEXT_ARRAY,     MARKER_SYMBOL_PROP_TEXT_ARRAY},
         {MARKER_TYPE_PROP_OBJECT_ARRAY,   MARKER_SYMBOL_PROP_OBJECT_ARRAY},
         {MARKER_TYPE_EMBEDDED_STR_DIC,    MARKER_SYMBOL_EMBEDDED_STR_DIC},
         {MARKER_TYPE_EMBEDDED_UNCOMP_STR, MARKER_SYMBOL_EMBEDDED_STR},
         {MARKER_TYPE_COLUMN_GROUP,        MARKER_SYMBOL_COLUMN_GROUP},
         {MARKER_TYPE_COLUMN,              MARKER_SYMBOL_COLUMN},
         {MARKER_TYPE_HUFFMAN_DIC_ENTRY,   MARKER_SYMBOL_HUFFMAN_DIC_ENTRY},
         {MARKER_TYPE_RECORD_HEADER,       MARKER_SYMBOL_RECORD_HEADER}};

static struct {
        archive_field_e value_type;
        archive_marker_e marker;
} global_value_array_marker_mapping[] =
        {{FIELD_NULL,    MARKER_TYPE_PROP_NULL_ARRAY},
         {FIELD_BOOLEAN, MARKER_TYPE_PROP_BOOLEAN_ARRAY},
         {FIELD_INT8,    MARKER_TYPE_PROP_INT8_ARRAY},
         {FIELD_INT16,   MARKER_TYPE_PROP_INT16_ARRAY},
         {FIELD_INT32,   MARKER_TYPE_PROP_INT32_ARRAY},
         {FIELD_INT64,   MARKER_TYPE_PROP_INT64_ARRAY},
         {FIELD_UINT8,   MARKER_TYPE_PROP_UINT8_ARRAY},
         {FIELD_UINT16,  MARKER_TYPE_PROP_UINT16_ARRAY},
         {FIELD_UINT32,  MARKER_TYPE_PROP_UINT32_ARRAY},
         {FIELD_UINT64,  MARKER_TYPE_PROP_UINT64_ARRAY},
         {FIELD_FLOAT,   MARKER_TYPE_PROP_REAL_ARRAY},
         {FIELD_STRING,  MARKER_TYPE_PROP_TEXT_ARRAY},
         {FIELD_OBJECT,  MARKER_TYPE_PROP_OBJECT_ARRAY}}, valueMarkerMapping[] =
        {{FIELD_NULL,    MARKER_TYPE_PROP_NULL},
         {FIELD_BOOLEAN, MARKER_TYPE_PROP_BOOLEAN},
         {FIELD_INT8,    MARKER_TYPE_PROP_INT8},
         {FIELD_INT16,   MARKER_TYPE_PROP_INT16},
         {FIELD_INT32,   MARKER_TYPE_PROP_INT32},
         {FIELD_INT64,   MARKER_TYPE_PROP_INT64},
         {FIELD_UINT8,   MARKER_TYPE_PROP_UINT8},
         {FIELD_UINT16,  MARKER_TYPE_PROP_UINT16},
         {FIELD_UINT32,  MARKER_TYPE_PROP_UINT32},
         {FIELD_UINT64,  MARKER_TYPE_PROP_UINT64},
         {FIELD_FLOAT,   MARKER_TYPE_PROP_REAL},
         {FIELD_STRING,  MARKER_TYPE_PROP_TEXT},
         {FIELD_OBJECT,  MARKER_TYPE_PROP_OBJECT}};

#pragma GCC diagnostic pop

typedef struct record_flags {
        struct {
                u8 is_sorted
                        : 1;
                u8 RESERVED_2
                        : 1;
                u8 RESERVED_3
                        : 1;
                u8 RESERVED_4
                        : 1;
                u8 RESERVED_5
                        : 1;
                u8 RESERVED_6
                        : 1;
                u8 RESERVED_7
                        : 1;
                u8 RESERVED_8
                        : 1;
        } bits;
        u8 value;
} record_flags;

typedef struct string_table {
        packer compressor;
        offset_t first_entry_off;
        u32 num_embeddded_strings;
} string_table;

typedef struct record_table {
        record_flags flags;
        memblock *record_db;
} record_table;

typedef struct archive_info {
        size_t string_table_size;
        size_t record_table_size;
        size_t string_id_index_size;
        u32 num_embeddded_strings;
} archive_info;

typedef struct __attribute__((packed)) string_entry_header {
        char marker;
        offset_t next_entry_off;
        archive_field_sid_t string_id;
        u32 string_len;
} string_entry_header;

void int_read_prop_offsets(archive_prop_offs *prop_offsets, memfile *memfile, const object_flags_u *flags);
void int_embedded_fixed_props_read(fixed_prop *prop, memfile *memfile);
void int_embedded_var_props_read(var_prop *prop, memfile *memfile);
void int_embedded_null_props_read(null_prop *prop, memfile *memfile);
void int_embedded_array_props_read(array_prop *prop, memfile *memfile);
void int_embedded_table_props_read(table_prop *prop, memfile *memfile);
archive_field_e int_get_value_type_of_char(char c);
archive_field_e int_marker_to_field_type(char symbol);

END_DECL

#endif
