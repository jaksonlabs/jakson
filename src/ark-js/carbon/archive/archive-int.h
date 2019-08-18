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

#ifndef ARK_INTERNALS_ARCHIVE_H
#define ARK_INTERNALS_ARCHIVE_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/mem/file.h>
#include <ark-js/shared/types.h>
#include <ark-js/shared/stdx/global_id.h>
#include <ark-js/carbon/coding/pack.h>

ARK_BEGIN_DECL


struct __attribute__((packed)) archive_header {
    char magic[9];
    u8 version;
    offset_t root_object_header_offset;
    offset_t string_id_to_offset_index_offset;
};

struct __attribute__((packed)) record_header {
    char marker;
    u8 flags;
    u64 record_size;
};

struct __attribute__((packed)) object_header {
    char marker;
    global_id_t oid;
    u32 flags;
};

struct __attribute__((packed)) prop_header {
    char marker;
    u32 num_entries;
};

union __attribute__((packed)) string_tab_flags {
    struct {
        u8 compressor_none
                : 1;
        u8 compressed_huffman
                : 1;
    } bits;
    u8 value;
};

struct __attribute__((packed)) string_table_header {
    char marker;
    u32 num_entries;
    u8 flags;
    offset_t first_entry;
    offset_t compressor_extra_size;
};

struct __attribute__((packed)) object_array_header {
    char marker;
    u8 num_entries;
};

struct __attribute__((packed)) column_group_header {
    char marker;
    u32 num_columns;
    u32 num_objects;
};

struct __attribute__((packed)) column_header {
    char marker;
    field_sid_t column_name;
    char value_type;
    u32 num_entries;
};

union object_flags {
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
};

struct archive_prop_offs {
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
};

struct fixed_prop {
    struct prop_header *header;
    const field_sid_t *keys;
    const void *values;
};

struct table_prop {
    struct prop_header *header;
    const field_sid_t *keys;
    const offset_t *groupOffs;
};

struct var_prop {
    struct prop_header *header;
    const field_sid_t *keys;
    const offset_t *offsets;
    const void *values;
};

struct array_prop {
    struct prop_header *header;
    const field_sid_t *keys;
    const u32 *lengths;
    offset_t values_begin;
};

struct null_prop {
    struct prop_header *header;
    const field_sid_t *keys;
};

enum marker_type {
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
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static struct archive_header this_file_header = {.version = CARBON_ARCHIVE_VERSION, .root_object_header_offset = 0};

static struct {
    enum marker_type type;
    char symbol;
} marker_symbols[] =
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
    field_e value_type;
    enum marker_type marker;
} value_array_marker_mapping[] =
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

struct record_flags {
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
};

struct string_table {
    struct packer compressor;
    offset_t first_entry_off;
    u32 num_embeddded_strings;
};

struct record_table {
    struct record_flags flags;
    struct memblock *recordDataBase;
};

struct archive_info {
    size_t string_table_size;
    size_t record_table_size;
    size_t string_id_index_size;
    u32 num_embeddded_strings;
};

struct __attribute__((packed)) string_entry_header {
    char marker;
    offset_t next_entry_off;
    field_sid_t string_id;
    u32 string_len;
};

void int_read_prop_offsets(struct archive_prop_offs *prop_offsets, struct memfile *memfile,
                           const union object_flags *flags);

void int_embedded_fixed_props_read(struct fixed_prop *prop, struct memfile *memfile);

void int_embedded_var_props_read(struct var_prop *prop, struct memfile *memfile);

void int_embedded_null_props_read(struct null_prop *prop, struct memfile *memfile);

void int_embedded_array_props_read(struct array_prop *prop, struct memfile *memfile);

void int_embedded_table_props_read(struct table_prop *prop, struct memfile *memfile);

field_e int_get_value_type_of_char(char c);

field_e int_marker_to_field_type(char symbol);

ARK_END_DECL

#endif
