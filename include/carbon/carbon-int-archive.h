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

#ifndef CARBON_INTERNALS_ARCHIVE_H
#define CARBON_INTERNALS_ARCHIVE_H

#include "carbon-common.h"
#include "carbon-memfile.h"
#include "carbon-types.h"
#include "carbon-oid.h"
#include "carbon-compressor.h"

CARBON_BEGIN_DECL


typedef struct __attribute__((packed))
{
    char magic[9];
    uint8_t version;
    carbon_off_t root_object_header_offset;
    carbon_off_t string_id_to_offset_index_offset;
} carbon_file_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    uint8_t flags;
    uint64_t record_size;
} carbon_record_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    carbon_object_id_t oid;
    uint32_t flags;
} carbon_object_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    uint32_t num_entries;
} carbon_prop_header_t;

typedef union __attribute__((packed)) carbon_archive_dic_flags
{
    struct {
        uint8_t compressor_none           : 1;
        uint8_t compressed_huffman        : 1;
    } bits;
    uint8_t value;
} carbon_archive_dic_flags_t;

typedef struct __attribute__((packed))
{
    char marker;
    uint32_t num_entries;
    uint8_t flags;
    carbon_off_t first_entry;
} carbon_string_table_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    uint8_t num_entries;
} carbon_object_array_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    uint32_t num_columns;
    uint32_t num_objects;
} carbon_column_group_header_t;

typedef struct __attribute__((packed))
{
    char marker;
    carbon_string_id_t column_name;
    char value_type;
    uint32_t num_entries;
} carbon_column_header_t;

typedef union
{
    struct {
        uint32_t has_null_props            : 1;
        uint32_t has_bool_props            : 1;
        uint32_t has_int8_props            : 1;
        uint32_t has_int16_props           : 1;
        uint32_t has_int32_props           : 1;
        uint32_t has_int64_props           : 1;
        uint32_t has_uint8_props           : 1;
        uint32_t has_uint16_props          : 1;
        uint32_t has_uint32_props          : 1;
        uint32_t has_uint64_props          : 1;
        uint32_t has_float_props           : 1;
        uint32_t has_string_props          : 1;
        uint32_t has_object_props          : 1;
        uint32_t has_null_array_props      : 1;
        uint32_t has_bool_array_props      : 1;
        uint32_t has_int8_array_props      : 1;
        uint32_t has_int16_array_props     : 1;
        uint32_t has_int32_array_props     : 1;
        uint32_t has_int64_array_props     : 1;
        uint32_t has_uint8_array_props     : 1;
        uint32_t has_uint16_array_props    : 1;
        uint32_t has_uint32_array_props    : 1;
        uint32_t has_uint64_array_props    : 1;
        uint32_t has_float_array_props     : 1;
        uint32_t has_string_array_props    : 1;
        uint32_t has_object_array_props    : 1;

        uint32_t RESERVED_27               : 1;
        uint32_t RESERVED_28               : 1;
        uint32_t RESERVED_29               : 1;
        uint32_t RESERVED_30               : 1;
        uint32_t RESERVED_31               : 1;
        uint32_t RESERVED_32               : 1;
    } bits;
    uint32_t value;
} carbon_archive_object_flags_t;


typedef struct {
    carbon_off_t nulls;
    carbon_off_t bools;
    carbon_off_t int8s;
    carbon_off_t int16s;
    carbon_off_t int32s;
    carbon_off_t int64s;
    carbon_off_t uint8s;
    carbon_off_t uint16s;
    carbon_off_t uint32s;
    carbon_off_t uint64s;
    carbon_off_t floats;
    carbon_off_t strings;
    carbon_off_t objects;
    carbon_off_t null_arrays;
    carbon_off_t bool_arrays;
    carbon_off_t int8_arrays;
    carbon_off_t int16_arrays;
    carbon_off_t int32_arrays;
    carbon_off_t int64_arrays;
    carbon_off_t uint8_arrays;
    carbon_off_t uint16_arrays;
    carbon_off_t uint32_arrays;
    carbon_off_t uint64_arrays;
    carbon_off_t float_arrays;
    carbon_off_t string_arrays;
    carbon_off_t object_arrays;
} carbon_archive_prop_offs_t;

typedef struct
{
    carbon_prop_header_t *header;
    const carbon_string_id_t *keys;
    const void *values;
} carbon_fixed_prop_t;

typedef struct
{
    carbon_prop_header_t *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *groupOffs;
} carbon_table_prop_t;

typedef struct
{
    carbon_prop_header_t *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *offsets;
    const void *values;
} carbon_var_prop_t;

typedef struct
{
    carbon_prop_header_t *header;
    const carbon_string_id_t *keys;
    const uint32_t *lengths;
    carbon_off_t values_begin;
} carbon_array_prop_t;

typedef struct
{
    carbon_prop_header_t *header;
    const carbon_string_id_t *keys;
} carbon_null_prop_t;

enum marker_type
{
    MARKER_TYPE_OBJECT_BEGIN            =  0,
    MARKER_TYPE_OBJECT_END              =  1,
    MARKER_TYPE_PROP_NULL               =  2,
    MARKER_TYPE_PROP_BOOLEAN            =  3,
    MARKER_TYPE_PROP_INT8               =  4,
    MARKER_TYPE_PROP_INT16              =  5,
    MARKER_TYPE_PROP_INT32              =  6,
    MARKER_TYPE_PROP_INT64              =  7,
    MARKER_TYPE_PROP_UINT8              =  8,
    MARKER_TYPE_PROP_UINT16             =  9,
    MARKER_TYPE_PROP_UINT32             = 10,
    MARKER_TYPE_PROP_UINT64             = 11,
    MARKER_TYPE_PROP_REAL               = 12,
    MARKER_TYPE_PROP_TEXT               = 13,
    MARKER_TYPE_PROP_OBJECT             = 14,
    MARKER_TYPE_PROP_NULL_ARRAY         = 15,
    MARKER_TYPE_PROP_BOOLEAN_ARRAY      = 16,
    MARKER_TYPE_PROP_INT8_ARRAY         = 17,
    MARKER_TYPE_PROP_INT16_ARRAY        = 18,
    MARKER_TYPE_PROP_INT32_ARRAY        = 19,
    MARKER_TYPE_PROP_INT64_ARRAY        = 20,
    MARKER_TYPE_PROP_UINT8_ARRAY        = 21,
    MARKER_TYPE_PROP_UINT16_ARRAY       = 22,
    MARKER_TYPE_PROP_UINT32_ARRAY       = 23,
    MARKER_TYPE_PROP_UINT64_ARRAY       = 24,
    MARKER_TYPE_PROP_REAL_ARRAY         = 25,
    MARKER_TYPE_PROP_TEXT_ARRAY         = 26,
    MARKER_TYPE_PROP_OBJECT_ARRAY       = 27,
    MARKER_TYPE_EMBEDDED_STR_DIC        = 28,
    MARKER_TYPE_EMBEDDED_UNCOMP_STR     = 29,
    MARKER_TYPE_COLUMN_GROUP            = 30,
    MARKER_TYPE_COLUMN                  = 31,
    MARKER_TYPE_HUFFMAN_DIC_ENTRY       = 32,
    MARKER_TYPE_RECORD_HEADER           = 33,
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static carbon_file_header_t this_carbon_file_header = {
    .version = CABIN_FILE_VERSION,
    .root_object_header_offset = 0
};

static struct
{
    enum marker_type type;
    char symbol;
} marker_symbols [] = {
    { MARKER_TYPE_OBJECT_BEGIN,        MARKER_SYMBOL_OBJECT_BEGIN },
    { MARKER_TYPE_OBJECT_END,          MARKER_SYMBOL_OBJECT_END },
    { MARKER_TYPE_PROP_NULL,           MARKER_SYMBOL_PROP_NULL },
    { MARKER_TYPE_PROP_BOOLEAN,        MARKER_SYMBOL_PROP_BOOLEAN },
    { MARKER_TYPE_PROP_INT8,           MARKER_SYMBOL_PROP_INT8 },
    { MARKER_TYPE_PROP_INT16,          MARKER_SYMBOL_PROP_INT16 },
    { MARKER_TYPE_PROP_INT32,          MARKER_SYMBOL_PROP_INT32 },
    { MARKER_TYPE_PROP_INT64,          MARKER_SYMBOL_PROP_INT64 },
    { MARKER_TYPE_PROP_UINT8,          MARKER_SYMBOL_PROP_UINT8 },
    { MARKER_TYPE_PROP_UINT16,         MARKER_SYMBOL_PROP_UINT16 },
    { MARKER_TYPE_PROP_UINT32,         MARKER_SYMBOL_PROP_UINT32 },
    { MARKER_TYPE_PROP_UINT64,         MARKER_SYMBOL_PROP_UINT64 },
    { MARKER_TYPE_PROP_REAL,           MARKER_SYMBOL_PROP_REAL },
    { MARKER_TYPE_PROP_TEXT,           MARKER_SYMBOL_PROP_TEXT },
    { MARKER_TYPE_PROP_OBJECT,         MARKER_SYMBOL_PROP_OBJECT },
    { MARKER_TYPE_PROP_NULL_ARRAY,     MARKER_SYMBOL_PROP_NULL_ARRAY },
    { MARKER_TYPE_PROP_BOOLEAN_ARRAY,  MARKER_SYMBOL_PROP_BOOLEAN_ARRAY },
    { MARKER_TYPE_PROP_INT8_ARRAY,     MARKER_SYMBOL_PROP_INT8_ARRAY },
    { MARKER_TYPE_PROP_INT16_ARRAY,    MARKER_SYMBOL_PROP_INT16_ARRAY },
    { MARKER_TYPE_PROP_INT32_ARRAY,    MARKER_SYMBOL_PROP_INT32_ARRAY },
    { MARKER_TYPE_PROP_INT64_ARRAY,    MARKER_SYMBOL_PROP_INT64_ARRAY },
    { MARKER_TYPE_PROP_UINT8_ARRAY,    MARKER_SYMBOL_PROP_UINT8_ARRAY },
    { MARKER_TYPE_PROP_UINT16_ARRAY,   MARKER_SYMBOL_PROP_UINT16_ARRAY },
    { MARKER_TYPE_PROP_UINT32_ARRAY,   MARKER_SYMBOL_PROP_UINT32_ARRAY },
    { MARKER_TYPE_PROP_UINT64_ARRAY,   MARKER_SYMBOL_PROP_UINT64_ARRAY },
    { MARKER_TYPE_PROP_REAL_ARRAY,     MARKER_SYMBOL_PROP_REAL_ARRAY },
    { MARKER_TYPE_PROP_TEXT_ARRAY,     MARKER_SYMBOL_PROP_TEXT_ARRAY },
    { MARKER_TYPE_PROP_OBJECT_ARRAY,   MARKER_SYMBOL_PROP_OBJECT_ARRAY },
    { MARKER_TYPE_EMBEDDED_STR_DIC,    MARKER_SYMBOL_EMBEDDED_STR_DIC },
    { MARKER_TYPE_EMBEDDED_UNCOMP_STR, MARKER_SYMBOL_EMBEDDED_STR },
    { MARKER_TYPE_COLUMN_GROUP,        MARKER_SYMBOL_COLUMN_GROUP },
    { MARKER_TYPE_COLUMN,              MARKER_SYMBOL_COLUMN },
    { MARKER_TYPE_HUFFMAN_DIC_ENTRY,   MARKER_SYMBOL_HUFFMAN_DIC_ENTRY },
    { MARKER_TYPE_RECORD_HEADER,       MARKER_SYMBOL_RECORD_HEADER }
};

static struct
{
    carbon_field_type_e value_type;
    enum marker_type marker;
} value_array_marker_mapping [] = {
    { carbon_field_type_null,    MARKER_TYPE_PROP_NULL_ARRAY },
    { carbon_field_type_bool,    MARKER_TYPE_PROP_BOOLEAN_ARRAY },
    { carbon_field_type_int8,    MARKER_TYPE_PROP_INT8_ARRAY },
    { carbon_field_type_int16,   MARKER_TYPE_PROP_INT16_ARRAY },
    { carbon_field_type_int32,   MARKER_TYPE_PROP_INT32_ARRAY },
    { carbon_field_type_int64,   MARKER_TYPE_PROP_INT64_ARRAY },
    { carbon_field_type_uint8,   MARKER_TYPE_PROP_UINT8_ARRAY },
    { carbon_field_type_uint16,  MARKER_TYPE_PROP_UINT16_ARRAY },
    { carbon_field_type_uint32,  MARKER_TYPE_PROP_UINT32_ARRAY },
    { carbon_field_type_uint64,  MARKER_TYPE_PROP_UINT64_ARRAY },
    { carbon_field_type_float,   MARKER_TYPE_PROP_REAL_ARRAY },
    { carbon_field_type_string,  MARKER_TYPE_PROP_TEXT_ARRAY },
    { carbon_field_type_object,  MARKER_TYPE_PROP_OBJECT_ARRAY }
}, valueMarkerMapping [] = {
    { carbon_field_type_null,    MARKER_TYPE_PROP_NULL },
    { carbon_field_type_bool,    MARKER_TYPE_PROP_BOOLEAN },
    { carbon_field_type_int8,    MARKER_TYPE_PROP_INT8 },
    { carbon_field_type_int16,   MARKER_TYPE_PROP_INT16 },
    { carbon_field_type_int32,   MARKER_TYPE_PROP_INT32 },
    { carbon_field_type_int64,   MARKER_TYPE_PROP_INT64 },
    { carbon_field_type_uint8,   MARKER_TYPE_PROP_UINT8 },
    { carbon_field_type_uint16,  MARKER_TYPE_PROP_UINT16 },
    { carbon_field_type_uint32,  MARKER_TYPE_PROP_UINT32 },
    { carbon_field_type_uint64,  MARKER_TYPE_PROP_UINT64 },
    { carbon_field_type_float,   MARKER_TYPE_PROP_REAL },
    { carbon_field_type_string,  MARKER_TYPE_PROP_TEXT },
    { carbon_field_type_object,  MARKER_TYPE_PROP_OBJECT }
};

#pragma GCC diagnostic pop

typedef struct
{
    struct {
        uint8_t is_sorted                 : 1;

        uint8_t RESERVED_2                : 1;
        uint8_t RESERVED_3                : 1;
        uint8_t RESERVED_4                : 1;
        uint8_t RESERVED_5                : 1;
        uint8_t RESERVED_6                : 1;
        uint8_t RESERVED_7                : 1;
        uint8_t RESERVED_8                : 1;
    } bits;
    uint8_t value;
} carbon_archive_record_flags_t;

typedef struct
{
    carbon_compressor_t     compressor;
    carbon_off_t            first_entry_off;
} carbon_archive_string_table_t;

typedef struct
{
    carbon_archive_record_flags_t   flags;
    carbon_memblock_t              *recordDataBase;
} carbon_archive_record_table_t;

typedef struct
{
    size_t string_table_size;
    size_t record_table_size;
} carbon_archive_info_t;


typedef struct __attribute__((packed))
{
    char                          marker;
    carbon_off_t                  next_entry_off;
    carbon_string_id_t            string_id;
    uint32_t                      string_len;
} carbon_string_entry_header_t;

void
carbon_int_read_prop_offsets(carbon_archive_prop_offs_t *prop_offsets,
                             carbon_memfile_t *memfile,
                             const carbon_archive_object_flags_t *flags);

void
carbon_int_embedded_fixed_props_read(carbon_fixed_prop_t *prop, carbon_memfile_t *memfile);

void
carbon_int_embedded_var_props_read(carbon_var_prop_t *prop, carbon_memfile_t *memfile);

void
carbon_int_embedded_null_props_read(carbon_null_prop_t *prop, carbon_memfile_t *memfile);

void
carbon_int_embedded_array_props_read(carbon_array_prop_t *prop, carbon_memfile_t *memfile);

void
carbon_int_embedded_table_props_read(carbon_table_prop_t *prop, carbon_memfile_t *memfile);

carbon_field_type_e
carbon_int_get_value_type_of_char(char c);

carbon_field_type_e
carbon_int_marker_to_field_type(char symbol);

carbon_basic_type_e
carbon_int_field_type_to_basic_type(carbon_field_type_e type);

CARBON_END_DECL

#endif
