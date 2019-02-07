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

#include "carbon/carbon-memblock.h"
#include "carbon/carbon-memfile.h"
#include "carbon/carbon-huffman.h"
#include "carbon/carbon-archive.h"

#define CABIN_FILE_MAGIC                    "MP/CARBON"
#define CABIN_FILE_VERSION                  1

#define  MARKER_SYMBOL_OBJECT_BEGIN        '{'
#define  MARKER_SYMBOL_OBJECT_END          '}'
#define  MARKER_SYMBOL_PROP_NULL           'n'
#define  MARKER_SYMBOL_PROP_BOOLEAN        'b'
#define  MARKER_SYMBOL_PROP_INT8           'c'
#define  MARKER_SYMBOL_PROP_INT16          's'
#define  MARKER_SYMBOL_PROP_INT32          'i'
#define  MARKER_SYMBOL_PROP_INT64          'l'
#define  MARKER_SYMBOL_PROP_UINT8          'r'
#define  MARKER_SYMBOL_PROP_UINT16         'h'
#define  MARKER_SYMBOL_PROP_UINT32         'e'
#define  MARKER_SYMBOL_PROP_UINT64         'g'
#define  MARKER_SYMBOL_PROP_REAL           'f'
#define  MARKER_SYMBOL_PROP_TEXT           't'
#define  MARKER_SYMBOL_PROP_OBJECT         'o'
#define  MARKER_SYMBOL_PROP_NULL_ARRAY     'N'
#define  MARKER_SYMBOL_PROP_BOOLEAN_ARRAY  'B'
#define  MARKER_SYMBOL_PROP_INT8_ARRAY     'C'
#define  MARKER_SYMBOL_PROP_INT16_ARRAY    'S'
#define  MARKER_SYMBOL_PROP_INT32_ARRAY    'I'
#define  MARKER_SYMBOL_PROP_INT64_ARRAY    'L'
#define  MARKER_SYMBOL_PROP_UINT8_ARRAY    'R'
#define  MARKER_SYMBOL_PROP_UINT16_ARRAY   'H'
#define  MARKER_SYMBOL_PROP_UINT32_ARRAY   'E'
#define  MARKER_SYMBOL_PROP_UINT64_ARRAY   'G'
#define  MARKER_SYMBOL_PROP_REAL_ARRAY     'F'
#define  MARKER_SYMBOL_PROP_TEXT_ARRAY     'T'
#define  MARKER_SYMBOL_PROP_OBJECT_ARRAY   'O'
#define  MARKER_SYMBOL_EMBEDDED_STR_DIC    'D'
#define  MARKER_SYMBOL_EMBEDDED_STR        '-'
#define  MARKER_SYMBOL_COLUMN_GROUP        'X'
#define  MARKER_SYMBOL_COLUMN              'x'
#define  MARKER_SYMBOL_HUFFMAN_DIC_ENTRY   'd'
#define  MARKER_SYMBOL_RECORD_HEADER       'r'

#define WRITE_PRIMITIVE_VALUES(memfile, values_vec, type)                                                               \
{                                                                                                                      \
    type *values = VECTOR_ALL(values_vec, type);                                                                        \
    carbon_memfile_write(memfile, values, values_vec->num_elems * sizeof(type));                                                 \
}

#define WRITE_ARRAY_VALUES(memfile, values_vec, type)                                                                   \
{                                                                                                                      \
    for (uint32_t i = 0; i < values_vec->num_elems; i++) {                                                               \
        carbon_vec_t ofType(type) *nested_values = VECTOR_GET(values_vec, i, carbon_vec_t);                            \
        WRITE_PRIMITIVE_VALUES(memfile, nested_values, type);                                                           \
    }                                                                                                                  \
}

#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(num_pairs, obj, bit_flag_name, offset_name)                                    \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR)                                                   \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bit_flag_name) {                                                                           \
        assert(obj->props.offset_name != 0);                                                                      \
        carbon_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        ewmbedded_fixed_prop_t prop;                                                                                        \
        embedded_fixed_props_read(&prop, &obj->file);                                                                  \
        reset_cabin_object_mem_file(obj);                                                                                  \
        CARBON_OPTIONAL_SET(num_pairs, prop.header->num_entries);                                                               \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(num_pairs, 0);                                                                                     \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

#define PRINT_SIMPLE_PROPS(file, memfile, offset, nesting_level, value_type, type_string, format_string)                   \
{                                                                                                                      \
    struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);                         \
    carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));                    \
    value_type *values = (value_type *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(value_type));               \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nesting_level)                                                                                          \
    fprintf(file, "[marker: %c (" type_string ")] [num_entries: %d] [", entryMarker, prop_header->num_entries);            \
    for (uint32_t i = 0; i < prop_header->num_entries; i++) {                                                            \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");                              \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
    for (uint32_t i = 0; i < prop_header->num_entries; i++) {                                                            \
      fprintf(file, "value: "format_string"%s", values[i], i + 1 < prop_header->num_entries ? ", " : "");                 \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

#define PRINT_ARRAY_PROPS(memfile, offset, nesting_level, entryMarker, type, type_string, format_string)                  \
{                                                                                                                      \
    struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);                         \
                                                                                                                       \
    carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));                    \
    uint32_t *array_lengths;                                                                                            \
                                                                                                                       \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nesting_level)                                                                                          \
    fprintf(file, "[marker: %c ("type_string")] [num_entries: %d] [", entryMarker, prop_header->num_entries);              \
                                                                                                                       \
    for (uint32_t i = 0; i < prop_header->num_entries; i++) {                                                            \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");                              \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    array_lengths = (uint32_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(uint32_t));                      \
                                                                                                                       \
    for (uint32_t i = 0; i < prop_header->num_entries; i++) {                                                            \
        fprintf(file, "num_entries: %d%s", array_lengths[i], i + 1 < prop_header->num_entries ? ", " : "");                \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    for (uint32_t array_idx = 0; array_idx < prop_header->num_entries; array_idx++) {                                       \
        type *values = (type *) CARBON_MEMFILE_READ(memfile, array_lengths[array_idx] * sizeof(type));                          \
        fprintf(file, "[");                                                                                            \
        for (uint32_t i = 0; i < array_lengths[array_idx]; i++) {                                                        \
            fprintf(file, "value: "format_string"%s", values[i], i + 1 < array_lengths[array_idx] ? ", " : "");           \
        }                                                                                                              \
        fprintf(file, "]%s", array_idx + 1 < prop_header->num_entries ? ", " : "");                                       \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "]\n");                                                                                              \
}

#define INTENT_LINE(nesting_level)                                                                                      \
{                                                                                                                      \
    for (unsigned nest_level = 0; nest_level < nesting_level; nest_level++) {                                              \
        fprintf(file, "   ");                                                                                          \
    }                                                                                                                  \
}

#define PRINT_VALUE_ARRAY(type, memfile, header, format_string)                                                         \
{                                                                                                                      \
    uint32_t num_elements = *CARBON_MEMFILE_READ_TYPE(memfile, uint32_t);                                                      \
    const type *values = (const type *) CARBON_MEMFILE_READ(memfile, num_elements * sizeof(type));                             \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                        \
    INTENT_LINE(nesting_level);                                                                                         \
    fprintf(file, "   [num_elements: %d] [values: [", num_elements);                                                     \
    for (size_t i = 0; i < num_elements; i++) {                                                                         \
        fprintf(file, "value: "format_string"%s", values[i], i + 1 < num_elements ? ", " : "");                          \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

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

struct
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

struct
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

struct __attribute__((packed)) carbon_file_header
{
    char magic[9];
    uint8_t version;
    carbon_off_t root_object_header_offset;
};

struct __attribute__((packed)) record_header
{
    char marker;
    uint8_t flags;
    uint64_t record_size;
};

struct carbon_file_header this_carbon_file_header = {
    .magic = CABIN_FILE_MAGIC,
    .version = CABIN_FILE_VERSION,
    .root_object_header_offset = 0
};

struct __attribute__((packed)) object_header
{
    char marker;
    uint32_t flags;
};

struct __attribute__((packed)) simple_prop_header
{
    char marker;
    uint32_t num_entries;
};

struct __attribute__((packed)) array_prop_header
{
    char marker;
    uint32_t num_entries;
};

union __attribute__((packed)) carbon_archive_dic_flags
{
    struct {
        uint8_t is_compressed           : 1;
        uint8_t compressed_with_huffman : 1;
    } bits;
    uint8_t value;
};

struct __attribute__((packed)) embedded_dic_header
{
    char marker;
    uint32_t num_entries;
    uint8_t flags;
};

struct __attribute__((packed)) embedded_string
{
    char marker;
    uint64_t strlen;
};

struct __attribute__((packed)) object_array_header
{
    char marker;
    uint8_t num_entries;
};

struct __attribute__((packed)) column_group_header
{
    char marker;
    uint32_t num_columns;
};

struct __attribute__((packed)) column_header
{
    char marker;
    carbon_string_id_t column_name;
    char value_type;
    uint32_t num_entries;
};

static carbon_off_t skip_record_header(carbon_memfile_t *memfile);
static void update_record_header(carbon_memfile_t *memfile,
                               carbon_off_t root_object_header_offset,
                               carbon_columndoc_t *model,
                               uint64_t record_size);
static bool serialize_columndoc(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memfile, carbon_columndoc_obj_t *columndoc, carbon_off_t root_object_header_offset);
static carbon_archive_object_flags_t *get_flags(carbon_archive_object_flags_t *flags, carbon_columndoc_obj_t *columndoc);
static void update_carbon_file_header(carbon_memfile_t *memfile, carbon_off_t root_object_header_offset);
static void skip_cabin_file_header(carbon_memfile_t *memfile);
static bool serialize_string_dic(carbon_memfile_t *memfile, carbon_err_t *err, const carbon_doc_bulk_t *context, carbon_archive_compressor_type_e compressor);
static bool print_archive_from_memfile(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile);

static void compressor_none_set_header_flags(union carbon_archive_dic_flags *flags);
static bool compressor_none_accepts(const union carbon_archive_dic_flags *flags);
static void compressor_none_write_dictionary(carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                                const carbon_vec_t ofType(carbon_string_id_t) *string_ids);
static void compressor_none_dump_dictionary(FILE *file, carbon_memfile_t *memfile);
static void compressor_none_create(carbon_archive_compressor_t *strategy);

static void compressor_huffman_set_header_flags(union carbon_archive_dic_flags *flags);
static bool compressor_huffman_accepts(const union carbon_archive_dic_flags *flags);
static void compressor_huffman_write_dictionary(carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                                   const carbon_vec_t ofType(carbon_string_id_t) *string_ids);
static void compressor_huffman_dump_dictionary(FILE *file, carbon_memfile_t *memfile);
static void compressor_huffman_create(carbon_archive_compressor_t *strategy);

struct
{
    carbon_archive_compressor_type_e type;
    void (*create)(carbon_archive_compressor_t *strategy);
    bool (*accepts)(const union carbon_archive_dic_flags *flags);
} compressor_strategy_register[] =
    {
        { .type = CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE,
            .create = compressor_none_create,
            .accepts = compressor_none_accepts },

        { .type = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN,
            .create = compressor_huffman_create,
            .accepts = compressor_huffman_accepts }
    };


bool carbon_archive_from_model(carbon_memblock_t **stream,
                               carbon_err_t *err,
                               carbon_columndoc_t *model,
                               carbon_archive_compressor_type_e compressor)
{
    CARBON_NON_NULL_OR_ERROR(model)
    CARBON_NON_NULL_OR_ERROR(stream)
    CARBON_NON_NULL_OR_ERROR(err)

    carbon_memblock_create(stream, 1024);
    carbon_memfile_t memfile;
    carbon_memfile_open(&memfile, *stream, CARBON_MEMFILE_MODE_READWRITE);

    skip_cabin_file_header(&memfile);
    if(!serialize_string_dic(&memfile, err, model->bulk, compressor)) {
        return false;
    }
    carbon_off_t record_header_offset = skip_record_header(&memfile);
    update_carbon_file_header(&memfile, record_header_offset);
    carbon_off_t root_object_header_offset = CARBON_MEMFILE_TELL(&memfile);
    if(!serialize_columndoc(NULL, err, &memfile, &model->columndoc, root_object_header_offset)) {
        return false;
    }
    uint64_t record_size = CARBON_MEMFILE_TELL(&memfile) - (record_header_offset + sizeof(struct record_header));
    update_record_header(&memfile, record_header_offset, model, record_size);

    carbon_memfile_shrink(&memfile);
    return true;
}

bool carbon_archive_drop(carbon_memblock_t *stream)
{
    carbon_memblock_drop(stream);
    return true;
}

bool carbon_archive_write(FILE *file, const carbon_memblock_t *stream)
{
    return carbon_memblock_write_to_file(file, stream);
}

bool carbon_archive_load(carbon_memblock_t **stream, FILE *file)
{
    long start = ftell(file);
    fseek(file, 0, SEEK_END);
    long end = ftell(file);
    fseek(file, start, SEEK_SET);
    long fileSize = (end - start);

    return carbon_memblock_from_file(stream, file, fileSize);
}

bool carbon_archive_print(FILE *file, carbon_err_t *err, carbon_memblock_t *stream)
{
    carbon_memfile_t memfile;
    carbon_memfile_open(&memfile, stream, CARBON_MEMFILE_MODE_READONLY);
    if (carbon_memfile_size(&memfile) < sizeof(struct carbon_file_header) + sizeof(struct embedded_dic_header) + sizeof(struct object_header)) {
        CARBON_ERROR(err, CARBON_ERR_NOCARBONSTREAM);
        return false;
    } else {
        return print_archive_from_memfile(file, err, &memfile);
    }
}

static bool compressor_strategy_by_type(carbon_err_t *err, carbon_archive_compressor_t *strategy, carbon_archive_compressor_type_e type)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(compressor_strategy_register); i++) {
        if (compressor_strategy_register[i].type == type) {
            compressor_strategy_register[i].create(strategy);
            assert (strategy->tag == type);
            assert (strategy->dump_dic);
            assert (strategy->set_flags);
            assert (strategy->serialize_dic);
            return true;
        }
    }
    CARBON_ERROR(err, CARBON_ERR_NOCOMPRESSOR)
    return false;
}

static bool compressor_strategy_by_flags(carbon_archive_compressor_t *strategy, const union carbon_archive_dic_flags *flags)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(compressor_strategy_register); i++) {
        if (compressor_strategy_register[i].accepts(flags)) {
            compressor_strategy_register[i].create(strategy);
            return true;
        }
    }
    return false;
}

static void compressor_none_set_header_flags(union carbon_archive_dic_flags *flags)
{
    flags->bits.is_compressed = 0;
}

static bool compressor_none_accepts(const union carbon_archive_dic_flags *flags)
{
    return !flags->bits.is_compressed;
}

static void compressor_none_write_dictionary(carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                                const carbon_vec_t ofType(carbon_string_id_t) *string_ids)
{
    for (size_t i = 0; i < strings->num_elems; i++) {
        carbon_string_id_t *string_id_t = VECTOR_GET(string_ids, i, carbon_string_id_t);
        const char *string = *VECTOR_GET(strings, i, const char *);
        size_t string_length = strlen(string);

        struct embedded_string embedded_string = {
            .marker = marker_symbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol,
            .strlen = string_length
        };

        carbon_memfile_write(memfile, &embedded_string, sizeof(struct embedded_string));
        carbon_memfile_write(memfile, string_id_t, sizeof(carbon_string_id_t));
        carbon_memfile_write(memfile, string, string_length);
    }
}

static void compressor_none_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    while ((*CARBON_MEMFILE_PEEK(memfile, char)) == marker_symbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol) {
        unsigned offset = CARBON_MEMFILE_TELL(memfile);
        struct embedded_string *embedded_string = CARBON_MEMFILE_READ_TYPE(memfile, struct embedded_string);
        carbon_string_id_t *string_id = CARBON_MEMFILE_READ_TYPE(memfile, carbon_string_id_t);
        const char *string = CARBON_MEMFILE_READ(memfile, embedded_string->strlen);
        char *printableString = malloc(embedded_string->strlen + 1);
        memcpy(printableString, string, embedded_string->strlen);
        printableString[embedded_string->strlen] = '\0';

        fprintf(file, "0x%04x ", offset);
        fprintf(file, "   [marker: %c] [string_length: %"PRIu64"] [string_id: %"PRIu64"] [string: '%s']\n",
                embedded_string->marker,
                embedded_string->strlen, *string_id, printableString);

        free(printableString);
    }
}

static void compressor_none_create(carbon_archive_compressor_t *strategy)
{
    strategy->tag = CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE;
    strategy->set_flags = compressor_none_set_header_flags;
    strategy->serialize_dic = compressor_none_write_dictionary;
    strategy->dump_dic = compressor_none_dump_dictionary;
}

static void compressor_huffman_set_header_flags(union carbon_archive_dic_flags *flags)
{
    flags->bits.is_compressed = 1;
    flags->bits.compressed_with_huffman = 1;
}

static bool compressor_huffman_accepts(const union carbon_archive_dic_flags *flags)
{
    return flags->bits.is_compressed && flags->bits.compressed_with_huffman;
}

static void compressor_huffman_write_dictionary(carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                                   const carbon_vec_t ofType(carbon_string_id_t) *string_ids)
{
    carbon_huffman_t *dic;

    carbon_huffman_create(&dic, strings);
    carbon_huffman_serialize_dic(memfile, dic, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);
    carbon_huffman_encode(memfile, dic, MARKER_SYMBOL_EMBEDDED_STR, string_ids, strings);
    carbon_huffman_drop(dic);
}

static void huffman_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    while ((*CARBON_MEMFILE_PEEK(memfile, char)) == MARKER_SYMBOL_HUFFMAN_DIC_ENTRY) {
        carbon_huffman_entry_info_t entry_info;
        carbon_off_t offset;
        carbon_memfile_tell(&offset, memfile);
        carbon_huffman_read_dic_entry(&entry_info, memfile, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);
        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [letter: '%c'] [nbytes_prefix: %d] [code: ",
                MARKER_SYMBOL_HUFFMAN_DIC_ENTRY, entry_info.letter,
                entry_info.nbytes_prefix);
        if (entry_info.nbytes_prefix > 0) {
            for (uint16_t i = 0; i < entry_info.nbytes_prefix; i++) {
                carbon_bitmap_print_bits_in_char(file, entry_info.prefix_code[i]);
                fprintf(file, "%s", i + 1 < entry_info.nbytes_prefix ? ", " : "");
            }
        } else {
            fprintf(file, "0b00000000");
        }

        fprintf(file, "]\n");
    }
}

static void huffman_dump_string_table(FILE *file, carbon_memfile_t *memfile)
{
    char marker;
    while ((marker = *CARBON_MEMFILE_PEEK(memfile, char)) == MARKER_SYMBOL_EMBEDDED_STR) {
        carbon_huffman_encoded_str_info_t info;
        carbon_off_t offset;
        carbon_memfile_tell(&offset, memfile);
        carbon_huffman_read_string(&info, memfile, MARKER_SYMBOL_EMBEDDED_STR);
        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [string_id: '%"PRIu64"'] [string_length: '%d'] [nbytes_encoded: %d] [bytes: ",
                marker, info.string_id, info.str_length, info.nbytes_encoded);
        for (size_t i = 0; i < info.nbytes_encoded; i++) {
            char byte = info.encoded_bytes[i];
            carbon_bitmap_print_bits_in_char(file, byte);
            fprintf(file, "%s", i + 1 < info.nbytes_encoded ? "," : "");
        }
        fprintf(file, "]\n");
    }
}

static void compressor_huffman_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    huffman_dump_dictionary(file, memfile);
    huffman_dump_string_table(file, memfile);
}

static void compressor_huffman_create(carbon_archive_compressor_t *strategy)
{
    strategy->tag = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN;
    strategy->set_flags = compressor_huffman_set_header_flags;
    strategy->serialize_dic = compressor_huffman_write_dictionary;
    strategy->dump_dic = compressor_huffman_dump_dictionary;
}

bool print_object(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile, unsigned nesting_level);

static uint32_t flags_to_int32(carbon_archive_object_flags_t *flags)
{
    return *((int32_t *) flags);
}

static const char *array_value_type_to_string(carbon_err_t *err, carbon_field_type_e type)
{
    switch (type) {
        case carbon_field_type_null:    return "Null Array";
        case carbon_field_type_bool:    return "Boolean Array";
        case carbon_field_type_int8:    return "Int8 Array";
        case carbon_field_type_int16:   return "Int16 Array";
        case carbon_field_type_int32:   return "Int32 Array";
        case carbon_field_type_int64:   return "Int64 Array";
        case carbon_field_type_uint8:   return "UInt8 Array";
        case carbon_field_type_uint16:  return "UInt16 Array";
        case carbon_field_type_uint32:  return "UInt32 Array";
        case carbon_field_type_uint64:  return "UInt64 Array";
        case carbon_field_type_float:   return "UIntFloat Array";
        case carbon_field_type_string:  return "Text Array";
        case carbon_field_type_object:  return "Object Array";
        default: {
            CARBON_ERROR(err, CARBON_ERR_NOVALUESTR)
            return NULL;
        }
    }
}

static carbon_field_type_e value_type_symbol_to_value_type(char symbol)
{
    switch (symbol) {
        case MARKER_SYMBOL_PROP_NULL:
        case MARKER_SYMBOL_PROP_NULL_ARRAY:
            return carbon_field_type_null;
        case MARKER_SYMBOL_PROP_BOOLEAN:
        case MARKER_SYMBOL_PROP_BOOLEAN_ARRAY:
            return carbon_field_type_bool;
        case MARKER_SYMBOL_PROP_INT8:
        case MARKER_SYMBOL_PROP_INT8_ARRAY:
            return carbon_field_type_int8;
        case MARKER_SYMBOL_PROP_INT16:
        case MARKER_SYMBOL_PROP_INT16_ARRAY:
            return carbon_field_type_int16;
        case MARKER_SYMBOL_PROP_INT32:
        case MARKER_SYMBOL_PROP_INT32_ARRAY:
            return carbon_field_type_int32;
        case MARKER_SYMBOL_PROP_INT64:
        case MARKER_SYMBOL_PROP_INT64_ARRAY:
            return carbon_field_type_int64;
        case MARKER_SYMBOL_PROP_UINT8:
        case MARKER_SYMBOL_PROP_UINT8_ARRAY:
            return carbon_field_type_uint8;
        case MARKER_SYMBOL_PROP_UINT16:
        case MARKER_SYMBOL_PROP_UINT16_ARRAY:
            return carbon_field_type_uint16;
        case MARKER_SYMBOL_PROP_UINT32:
        case MARKER_SYMBOL_PROP_UINT32_ARRAY:
            return carbon_field_type_uint32;
        case MARKER_SYMBOL_PROP_UINT64:
        case MARKER_SYMBOL_PROP_UINT64_ARRAY:
            return carbon_field_type_uint64;
        case MARKER_SYMBOL_PROP_REAL:
        case MARKER_SYMBOL_PROP_REAL_ARRAY:
            return carbon_field_type_float;
        case MARKER_SYMBOL_PROP_TEXT:
        case MARKER_SYMBOL_PROP_TEXT_ARRAY:
            return carbon_field_type_string;
        case MARKER_SYMBOL_PROP_OBJECT:
        case MARKER_SYMBOL_PROP_OBJECT_ARRAY:
            return carbon_field_type_object;
        default: {
            CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_MARKERMAPPING);
        }
    }
}

static void write_primitive_key_column(carbon_memfile_t *memfile, carbon_vec_t ofType(carbon_string_id_t) *keys)
{
    carbon_string_id_t *string_ids = VECTOR_ALL(keys, carbon_string_id_t);
    carbon_memfile_write(memfile, string_ids, keys->num_elems * sizeof(carbon_string_id_t));
}

static carbon_off_t skip_var_value_offset_column(carbon_memfile_t *memfile, size_t num_keys)
{
    carbon_off_t result = CARBON_MEMFILE_TELL(memfile);
    carbon_memfile_skip(memfile, num_keys * sizeof(carbon_off_t));
    return result;
}

static void write_var_value_offset_column(carbon_memfile_t *file, carbon_off_t where, carbon_off_t after, const carbon_off_t *values, size_t n)
{
    carbon_memfile_seek(file, where);
    carbon_memfile_write(file, values, n * sizeof(carbon_off_t));
    carbon_memfile_seek(file, after);
}

static bool write_primitive_fixed_value_column(carbon_memfile_t *memfile,
                                           carbon_err_t *err,
                                           carbon_field_type_e type,
                                           carbon_vec_t ofType(T) *values_vec)
{
    assert (type != carbon_field_type_object); /** use 'write_primitive_var_value_column' instead */

    switch (type) {
    case carbon_field_type_null:
        break;
    case carbon_field_type_bool:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_bool_t);
        break;
    case carbon_field_type_int8:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_int8_t);
        break;
    case carbon_field_type_int16:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_int16_t);
        break;
    case carbon_field_type_int32:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_int32_t);
        break;
    case carbon_field_type_int64:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_int64_t);
        break;
    case carbon_field_type_uint8:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_uint8_t);
        break;
    case carbon_field_type_uint16:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_uint16_t);
        break;
    case carbon_field_type_uint32:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_uin32_t);
        break;
    case carbon_field_type_uint64:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_uin64_t);
        break;
    case carbon_field_type_float:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_float_t);
        break;
    case carbon_field_type_string:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, carbon_string_id_t);
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static carbon_off_t *write_primitive_var_value_column(carbon_memfile_t *memfile,
                                                      carbon_err_t *err,
                                                      carbon_vec_t ofType(carbon_columndoc_obj_t) *values_vec,
                                                      carbon_off_t root_object_header_offset)
{
    carbon_off_t *result = malloc(values_vec->num_elems * sizeof(carbon_off_t));
    carbon_columndoc_obj_t *mapped_objects = VECTOR_ALL(values_vec, carbon_columndoc_obj_t);
    for (uint32_t i = 0; i < values_vec->num_elems; i++) {
        carbon_columndoc_obj_t *mapped_object = mapped_objects + i;
        result[i] = CARBON_MEMFILE_TELL(memfile) - root_object_header_offset;
        if (!serialize_columndoc(NULL, err, memfile, mapped_object, root_object_header_offset)) {
            return NULL;
        }
    }
    return result;
}

static bool write_array_lengths_column(carbon_err_t *err, carbon_memfile_t *memfile, carbon_field_type_e type, carbon_vec_t ofType(...) *values_vec)
{
    switch (type) {
    case carbon_field_type_null:
        break;
    case carbon_field_type_bool:
    case carbon_field_type_int8:
    case carbon_field_type_int16:
    case carbon_field_type_int32:
    case carbon_field_type_int64:
    case carbon_field_type_uint8:
    case carbon_field_type_uint16:
    case carbon_field_type_uint32:
    case carbon_field_type_uint64:
    case carbon_field_type_float:
    case carbon_field_type_string:
        for (uint32_t i = 0; i < values_vec->num_elems; i++) {
            carbon_vec_t *nested_arrays = VECTOR_GET(values_vec, i, carbon_vec_t);
            carbon_memfile_write(memfile, &nested_arrays->num_elems, sizeof(uint32_t));
        }
        break;
    case carbon_field_type_object:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_ILLEGALIMPL)
        return false;
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static bool write_array_value_column(carbon_memfile_t *memfile, carbon_err_t *err, carbon_field_type_e type, carbon_vec_t ofType(...) *values_vec)
{

    switch (type) {
    case carbon_field_type_null:
        WRITE_PRIMITIVE_VALUES(memfile, values_vec, uint32_t);
        break;
    case carbon_field_type_bool:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_bool_t);
        break;
    case carbon_field_type_int8:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_int8_t);
        break;
    case carbon_field_type_int16:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_int16_t);
        break;
    case carbon_field_type_int32:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_int32_t);
        break;
    case carbon_field_type_int64:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_int64_t);
        break;
    case carbon_field_type_uint8:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_uin64_t);
        break;
    case carbon_field_type_uint16:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_uint16_t);
        break;
    case carbon_field_type_uint32:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_uin32_t);
        break;
    case carbon_field_type_uint64:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_uin64_t);
        break;
    case carbon_field_type_float:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_float_t);
        break;
    case carbon_field_type_string:
        WRITE_ARRAY_VALUES(memfile, values_vec, carbon_string_id_t);
        break;
    case carbon_field_type_object:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NOTIMPL)
        return false;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;
    }
    return true;
}

static bool write_array_prop(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memfile, carbon_vec_t ofType(carbon_string_id_t) *keys, carbon_field_type_e type,
                             carbon_vec_t ofType(...) *values, carbon_off_t root_object_header_offset)
{
    assert(keys->num_elems == values->num_elems);

    if (keys->num_elems > 0) {
        struct array_prop_header header = {
            .marker = marker_symbols[value_array_marker_mapping[type].marker].symbol,
            .num_entries = keys->num_elems
        };
        carbon_off_t prop_ofOffset = CARBON_MEMFILE_TELL(memfile);
        carbon_memfile_write(memfile, &header, sizeof(struct array_prop_header));

        write_primitive_key_column(memfile, keys);
        if(!write_array_lengths_column(err, memfile, type, values)) {
            return false;
        }
        if(!write_array_value_column(memfile, err, type, values)) {
            return false;
        }
        *offset = (prop_ofOffset - root_object_header_offset);
    } else {
        *offset = 0;
    }
    return true;
}

static bool write_array_props(carbon_memfile_t *memfile, carbon_err_t *err, carbon_columndoc_obj_t *columndoc, carbon_archive_prop_offs_t *offsets,
                            carbon_off_t root_object_header_offset)
{
     if (!write_array_prop(&offsets->null_arrays, err, memfile, &columndoc->null_array_prop_keys, carbon_field_type_null,
                                                        &columndoc->null_array_prop_vals, root_object_header_offset)) {
         return false;
     }
    if (!write_array_prop(&offsets->bool_arrays, err, memfile, &columndoc->bool_array_prop_keys, carbon_field_type_bool,
                                                           &columndoc->bool_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->int8_arrays, err, memfile, &columndoc->int8_array_prop_keys, carbon_field_type_int8,
                                                        &columndoc->int8_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->int16_arrays, err, memfile, &columndoc->int16_array_prop_keys, carbon_field_type_int16,
                                                         &columndoc->int16_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->int32_arrays, err, memfile, &columndoc->int32_array_prop_keys, carbon_field_type_int32,
                                                         &columndoc->int32_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->int64_arrays, err, memfile, &columndoc->int64_array_prop_keys, carbon_field_type_int64,
                                                         &columndoc->int64_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->uint8_arrays, err, memfile, &columndoc->uint8_array_prop_keys, carbon_field_type_uint8,
                                                    &columndoc->uint8_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->uint16_arrays, err, memfile, &columndoc->uint16_array_prop_keys, carbon_field_type_uint16,
                                                     &columndoc->uint16_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->uint32_arrays, err, memfile, &columndoc->uint32_array_prop_keys, carbon_field_type_uint32,
                                                     &columndoc->uint32_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->uint64_arrays, err, memfile, &columndoc->uint64_array_prop_keys, carbon_field_type_uint64,
                                                     &columndoc->uin64_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->float_arrays, err, memfile, &columndoc->float_array_prop_keys, carbon_field_type_float,
                                                        &columndoc->float_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    if (!write_array_prop(&offsets->string_arrays, err, memfile, &columndoc->string_array_prop_keys, carbon_field_type_string,
                                                          &columndoc->string_array_prop_vals, root_object_header_offset)) {
        return false;
    }
    return true;
}

/** Fixed-length property lists; value position can be determined by size of value and position of key in key column.
 * In contrast, variable-length property list require an additional offset column (see 'write_var_props') */
static bool write_fixed_props(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memfile,
                              carbon_vec_t ofType(carbon_string_id_t) *keys,
                              carbon_field_type_e type,
                              carbon_vec_t ofType(T) *values)
{
    assert(!values || keys->num_elems == values->num_elems);
    assert(type != carbon_field_type_object); /** use 'write_var_props' instead */

    if (keys->num_elems > 0) {
        struct simple_prop_header header = {
                .marker = marker_symbols[valueMarkerMapping[type].marker].symbol,
                .num_entries = keys->num_elems
        };

        carbon_off_t prop_ofOffset = CARBON_MEMFILE_TELL(memfile);
        carbon_memfile_write(memfile, &header, sizeof(struct simple_prop_header));

        write_primitive_key_column(memfile, keys);
        if(!write_primitive_fixed_value_column(memfile, err, type, values)) {
            return false;
        }
        *offset = prop_ofOffset;
    } else {
        *offset = 0;
    }
    return true;
}

/** Variable-length property lists; value position cannot be determined by position of key in key column, since single
 * value has unknown size. Hence, a dedicated offset column is added to these properties allowing to seek directly
 * to a particular property. Due to the move of strings (i.e., variable-length values) to a dedicated string table,
 * the only variable-length value for properties are "JSON objects".
 * In contrast, fixed-length property list doesn't require an additional offset column (see 'write_fixed_props') */
static bool write_var_props(carbon_off_t *offset,
                            carbon_err_t *err,
                            carbon_memfile_t *memfile,
                            carbon_vec_t ofType(carbon_string_id_t) *keys,
                            carbon_vec_t ofType(carbon_columndoc_obj_t) *objects,
                            carbon_off_t root_object_header_offset)
{
    assert(!objects || keys->num_elems == objects->num_elems);

    if (keys->num_elems > 0) {
        struct simple_prop_header header = {
            .marker = MARKER_SYMBOL_PROP_OBJECT,
            .num_entries = keys->num_elems
        };

        carbon_off_t prop_ofOffset = CARBON_MEMFILE_TELL(memfile);
        carbon_memfile_write(memfile, &header, sizeof(struct simple_prop_header));

        write_primitive_key_column(memfile, keys);
        carbon_off_t value_offset = skip_var_value_offset_column(memfile, keys->num_elems);
        carbon_off_t *value_offsets = write_primitive_var_value_column(memfile, err, objects, root_object_header_offset);
        if (!value_offsets) {
            return false;
        }

        carbon_off_t last = CARBON_MEMFILE_TELL(memfile);
        write_var_value_offset_column(memfile, value_offset, last, value_offsets, keys->num_elems);
        free(value_offsets);
        *offset = prop_ofOffset;
    } else {
        *offset = 0;
    }
    return true;
}

static bool write_primitive_props(carbon_memfile_t *memfile, carbon_err_t *err, carbon_columndoc_obj_t *columndoc, carbon_archive_prop_offs_t *offsets,
                                carbon_off_t root_object_header_offset)
{
     if (!write_fixed_props(&offsets->nulls, err, memfile, &columndoc->null_prop_keys, carbon_field_type_null,
                                                    NULL)) {
         return false;
     }
    if (!write_fixed_props(&offsets->bools, err, memfile, &columndoc->bool_prop_keys, carbon_field_type_bool,
                                                       &columndoc->bool_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->int8s, err, memfile, &columndoc->int8_prop_keys, carbon_field_type_int8,
                                                    &columndoc->int8_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->int16s, err, memfile, &columndoc->int16_prop_keys, carbon_field_type_int16,
                                                     &columndoc->int16_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->int32s, err, memfile, &columndoc->int32_prop_keys, carbon_field_type_int32,
                                                     &columndoc->int32_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->int64s, err, memfile, &columndoc->int64_prop_keys, carbon_field_type_int64,
                                                     &columndoc->int64_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->uint8s, err, memfile, &columndoc->uint8_prop_keys, carbon_field_type_uint8,
                                                     &columndoc->uint8_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->uint16s, err, memfile, &columndoc->uint16_prop_keys, carbon_field_type_uint16,
                                                      &columndoc->uint16_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->uint32s, err, memfile, &columndoc->uin32_prop_keys, carbon_field_type_uint32,
                                                      &columndoc->uint32_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->uint64s, err, memfile, &columndoc->uint64_prop_keys, carbon_field_type_uint64,
                                                      &columndoc->uint64_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->floats, err, memfile, &columndoc->float_prop_keys, carbon_field_type_float,
                                                    &columndoc->float_prop_vals)) {
        return false;
    }
    if (!write_fixed_props(&offsets->strings, err, memfile, &columndoc->string_prop_keys, carbon_field_type_string,
                                                      &columndoc->string_prop_vals)) {
        return false;
    }
    if (!write_var_props(&offsets->objects, err, memfile, &columndoc->obj_prop_keys,
                                                    &columndoc->obj_prop_vals, root_object_header_offset)) {
        return false;
    }

    offsets->nulls -= root_object_header_offset;
    offsets->bools -= root_object_header_offset;
    offsets->int8s -= root_object_header_offset;
    offsets->int16s -= root_object_header_offset;
    offsets->int32s -= root_object_header_offset;
    offsets->int64s -= root_object_header_offset;
    offsets->uint8s -= root_object_header_offset;
    offsets->uint16s -= root_object_header_offset;
    offsets->uint32s -= root_object_header_offset;
    offsets->uint64s -= root_object_header_offset;
    offsets->floats -= root_object_header_offset;
    offsets->strings -= root_object_header_offset;
    offsets->objects -= root_object_header_offset;
    return true;
}

static bool write_column_entry(carbon_memfile_t *memfile, carbon_err_t *err, carbon_field_type_e type, carbon_vec_t ofType(<T>) *column, carbon_off_t root_object_header_offset)
{
    carbon_memfile_write(memfile, &column->num_elems, sizeof(uint32_t));
    switch (type) {
        case carbon_field_type_null:carbon_memfile_write(memfile, column->base, column->num_elems * sizeof(uint32_t));
            break;
        case carbon_field_type_bool:
        case carbon_field_type_int8:
        case carbon_field_type_int16:
        case carbon_field_type_int32:
        case carbon_field_type_int64:
        case carbon_field_type_uint8:
        case carbon_field_type_uint16:
        case carbon_field_type_uint32:
        case carbon_field_type_uint64:
        case carbon_field_type_float:
        case carbon_field_type_string:carbon_memfile_write(memfile, column->base, column->num_elems * GET_TYPE_SIZE(type));
            break;
        case carbon_field_type_object: {
            carbon_off_t preObjectNext = 0;
            for (size_t i = 0; i < column->num_elems; i++) {
                carbon_columndoc_obj_t *object = VECTOR_GET(column, i, carbon_columndoc_obj_t);
                if (CARBON_BRANCH_LIKELY(preObjectNext != 0)) {
                    carbon_off_t continuePos = CARBON_MEMFILE_TELL(memfile);
                    carbon_off_t relativeContinuePos = continuePos - root_object_header_offset;
                    carbon_memfile_seek(memfile, preObjectNext);
                    carbon_memfile_write(memfile, &relativeContinuePos, sizeof(carbon_off_t));
                    carbon_memfile_seek(memfile, continuePos);
                }
                 if (!serialize_columndoc(&preObjectNext, err, memfile, object, root_object_header_offset)) {
                     return false;
                 }
            }
        } break;
        default:
            CARBON_ERROR(err, CARBON_ERR_NOTYPE)
            return false;
    }
    return true;
}

static carbon_field_type_e get_value_type_of_char(char c)
{
    size_t len = sizeof(value_array_marker_mapping)/ sizeof(value_array_marker_mapping[0]);
    for (size_t i = 0; i < len; i++) {
        if (marker_symbols[value_array_marker_mapping[i].marker].symbol == c) {
            return value_array_marker_mapping[i].value_type;
        }
    }
    return carbon_field_type_null;
}

static bool write_column(carbon_memfile_t *memfile, carbon_err_t *err, carbon_columndoc_column_t *column, carbon_off_t root_object_header_offset)
{
    assert(column->array_positions.num_elems == column->values.num_elems);

    struct column_header header = {
        .marker = marker_symbols[MARKER_TYPE_COLUMN].symbol,
        .column_name = column->key_name,
        .value_type = marker_symbols[value_array_marker_mapping[column->type].marker].symbol,
        .num_entries = column->values.num_elems
    };

    carbon_memfile_write(memfile, &header, sizeof(struct column_header));

    /** skip offset column to value entry points */
    carbon_off_t value_entry_offsets = CARBON_MEMFILE_TELL(memfile);
    carbon_memfile_skip(memfile, column->values.num_elems * sizeof(carbon_off_t));

    carbon_memfile_write(memfile, column->array_positions.base, column->array_positions.num_elems * sizeof(uint32_t));

    for (size_t i = 0; i < column->values.num_elems; i++) {
        carbon_vec_t ofType(<T>) *column_data = VECTOR_GET(&column->values, i, carbon_vec_t);
        carbon_off_t column_entry_offset = CARBON_MEMFILE_TELL(memfile);
        carbon_off_t relative_entry_offset = column_entry_offset - root_object_header_offset;
        carbon_memfile_seek(memfile, value_entry_offsets + i * sizeof(carbon_off_t));
        carbon_memfile_write(memfile, &relative_entry_offset, sizeof(carbon_off_t));
        carbon_memfile_seek(memfile, column_entry_offset);
        if (!write_column_entry(memfile, err, column->type, column_data, root_object_header_offset)) {
            return false;
        }
    }
    return true;
}

static bool write_object_array_props(carbon_memfile_t *memfile, carbon_err_t *err, carbon_vec_t ofType(carbon_columndoc_columngroup_t) *object_key_columns,
                                  carbon_archive_prop_offs_t *offsets, carbon_off_t root_object_header_offset)
{
    if (object_key_columns->num_elems > 0) {
        struct object_array_header header = {
            .marker = marker_symbols[MARKER_TYPE_PROP_OBJECT_ARRAY].symbol,
            .num_entries = object_key_columns->num_elems
        };

        offsets->object_arrays = CARBON_MEMFILE_TELL(memfile) - root_object_header_offset;
        carbon_memfile_write(memfile, &header, sizeof(struct object_array_header));

        for (size_t i = 0; i < object_key_columns->num_elems; i++) {
            carbon_columndoc_columngroup_t *column_group = VECTOR_GET(object_key_columns, i, carbon_columndoc_columngroup_t);
            carbon_memfile_write(memfile, &column_group->key, sizeof(carbon_string_id_t));
        }

        // skip offset column to column groups
        carbon_off_t column_offsets = CARBON_MEMFILE_TELL(memfile);
        carbon_memfile_skip(memfile, object_key_columns->num_elems * sizeof(carbon_off_t));

        for (size_t i = 0; i < object_key_columns->num_elems; i++) {
            carbon_columndoc_columngroup_t *column_group = VECTOR_GET(object_key_columns, i, carbon_columndoc_columngroup_t);

            struct column_group_header column_group_header = {
                .marker = marker_symbols[MARKER_TYPE_COLUMN_GROUP].symbol,
                .num_columns = column_group->columns.num_elems
            };

            carbon_off_t this_column_offfset_relative = CARBON_MEMFILE_TELL(memfile) - root_object_header_offset;
            carbon_memfile_write(memfile, &column_group_header, sizeof(struct column_group_header));
            carbon_off_t continue_write = CARBON_MEMFILE_TELL(memfile);
            carbon_memfile_seek(memfile, column_offsets + i * sizeof(carbon_off_t));
            carbon_memfile_write(memfile, &this_column_offfset_relative, sizeof(carbon_off_t));
            carbon_memfile_seek(memfile, continue_write);

            carbon_off_t offset_column_to_columns = continue_write;
            carbon_memfile_skip(memfile, column_group->columns.num_elems * sizeof(carbon_off_t));

            for (size_t k = 0; k < column_group->columns.num_elems; k++) {
                carbon_columndoc_column_t *column = VECTOR_GET(&column_group->columns, k, carbon_columndoc_column_t);
                carbon_off_t continue_write = CARBON_MEMFILE_TELL(memfile);
                carbon_off_t column_off = continue_write - root_object_header_offset;
                carbon_memfile_seek(memfile, offset_column_to_columns + k * sizeof(carbon_off_t));
                carbon_memfile_write(memfile, &column_off, sizeof(carbon_off_t));
                carbon_memfile_seek(memfile, continue_write);
                if(!write_column(memfile, err, column, root_object_header_offset)) {
                    return false;
                }
            }

        }
    } else {
        offsets->object_arrays = 0;
    }

    return true;
}

static carbon_off_t skip_record_header(carbon_memfile_t *memfile)
{
    carbon_off_t offset = CARBON_MEMFILE_TELL(memfile);
    carbon_memfile_skip(memfile, sizeof(struct record_header));
    return offset;
}

static void update_record_header(carbon_memfile_t *memfile,
                               carbon_off_t root_object_header_offset,
                               carbon_columndoc_t *model,
                               uint64_t record_size)
{
    carbon_archive_record_flags_t flags = {
        .bits.is_sorted = model->read_optimized
    };
    struct record_header header = {
        .marker = MARKER_SYMBOL_RECORD_HEADER,
        .flags = flags.value,
        .record_size = record_size
    };
    carbon_off_t offset;
    carbon_memfile_tell(&offset, memfile);
    carbon_memfile_seek(memfile, root_object_header_offset);
    carbon_memfile_write(memfile, &header, sizeof(struct record_header));
    carbon_memfile_seek(memfile, offset);
}

static void propOffsetsWrite(carbon_memfile_t *memfile, const carbon_archive_object_flags_t *flags, carbon_archive_prop_offs_t *prop_offsets)
{
    if (flags->bits.has_null_props) {
        carbon_memfile_write(memfile, &prop_offsets->nulls, sizeof(carbon_off_t));
    }
    if (flags->bits.has_bool_props) {
        carbon_memfile_write(memfile, &prop_offsets->bools, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int8_props) {
        carbon_memfile_write(memfile, &prop_offsets->int8s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int16_props) {
        carbon_memfile_write(memfile, &prop_offsets->int16s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int32_props) {
        carbon_memfile_write(memfile, &prop_offsets->int32s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int64_props) {
        carbon_memfile_write(memfile, &prop_offsets->int64s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint8_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint8s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint16_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint16s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint32_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint32s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint64_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint64s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_float_props) {
        carbon_memfile_write(memfile, &prop_offsets->floats, sizeof(carbon_off_t));
    }
    if (flags->bits.has_string_props) {
        carbon_memfile_write(memfile, &prop_offsets->strings, sizeof(carbon_off_t));
    }
    if (flags->bits.has_object_props) {
        carbon_memfile_write(memfile, &prop_offsets->objects, sizeof(carbon_off_t));
    }
    if (flags->bits.has_null_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->null_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_bool_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->bool_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int8_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->int8_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int16_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->int16_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int32_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->int32_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int64_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->int64_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint8_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint8_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint16_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint16_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint32_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint32_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint64_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->uint64_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_float_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->float_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_string_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->string_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_object_array_props) {
        carbon_memfile_write(memfile, &prop_offsets->object_arrays, sizeof(carbon_off_t));
    }
}

static void prop_offsets_skip_write(carbon_memfile_t *memfile, const carbon_archive_object_flags_t *flags)
{
    unsigned num_skip_offset_bytes = 0;
    if (flags->bits.has_null_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_bool_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int8_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int16_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int32_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int64_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint8_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint16_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint32_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint64_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_float_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_string_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_object_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_null_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_bool_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int8_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int16_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int32_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_int64_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint8_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint16_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint32_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_uint64_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_float_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_string_array_props) {
        num_skip_offset_bytes++;
    }
    if (flags->bits.has_object_array_props) {
        num_skip_offset_bytes++;
    }

    carbon_memfile_skip(memfile, num_skip_offset_bytes * sizeof(carbon_off_t));
}

static bool serialize_columndoc(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memfile, carbon_columndoc_obj_t *columndoc, carbon_off_t root_object_header_offset)
{
    carbon_archive_object_flags_t flags;
    carbon_archive_prop_offs_t prop_offsets;
    get_flags(&flags, columndoc);

    carbon_off_t header_offset = CARBON_MEMFILE_TELL(memfile);
    carbon_memfile_skip(memfile, sizeof(struct object_header));

    prop_offsets_skip_write(memfile, &flags);
    carbon_off_t next_offset = CARBON_MEMFILE_TELL(memfile);
    carbon_off_t default_next_nil = 0;
    carbon_memfile_write(memfile, &default_next_nil, sizeof(carbon_off_t));

    if (!write_primitive_props(memfile, err, columndoc, &prop_offsets, root_object_header_offset)) {
        return false;
    }
    if (!write_array_props(memfile, err, columndoc, &prop_offsets, root_object_header_offset)) {
        return false;
    }
    if (!write_object_array_props(memfile, err, &columndoc->obj_array_props, &prop_offsets, root_object_header_offset)) {
        return false;
    }

    carbon_memfile_write(memfile, &marker_symbols[MARKER_TYPE_OBJECT_END].symbol, 1);


    carbon_off_t object_end_offset = CARBON_MEMFILE_TELL(memfile);
    carbon_memfile_seek(memfile, header_offset);

    struct object_header header = {
        .marker = marker_symbols[MARKER_TYPE_OBJECT_BEGIN].symbol,
        .flags = flags_to_int32(&flags),
    };

    carbon_memfile_write(memfile, &header, sizeof(struct object_header));

    propOffsetsWrite(memfile, &flags, &prop_offsets);

    carbon_memfile_seek(memfile, object_end_offset);
    CARBON_OPTIONAL_SET(offset, next_offset);
    return true;
}

static char *embedded_dic_flags_to_string(const union carbon_archive_dic_flags *flags)
{
    size_t max = 2048;
    char *string = malloc(max + 1);
    size_t length = 0;

    if (flags->value == 0) {
        strcpy(string, " uncompressed");
        length = strlen(string);
        assert(length <= max);
    } else {
        if (flags->bits.is_compressed) {
            strcpy(string + length, " compressed");
            length = strlen(string);
            assert(length <= max);
        }

        if (flags->bits.compressed_with_huffman) {
            strcpy(string + length, " huffman");
            length = strlen(string);
            assert(length <= max);
        }
    }
    string[length] = '\0';
    return string;
}

static char *record_header_flags_to_string(const carbon_archive_record_flags_t *flags)
{
    size_t max = 2048;
    char *string = malloc(max + 1);
    size_t length = 0;

    if (flags->value == 0) {
        strcpy(string, " none");
        length = strlen(string);
        assert(length <= max);
    } else {
        if (flags->bits.is_sorted) {
            strcpy(string + length, " sorted");
            length = strlen(string);
            assert(length <= max);
        }
    }
    string[length] = '\0';
    return string;
}

static bool validate_embedded_dic_header_flags(carbon_err_t *err, const union carbon_archive_dic_flags *flags)
{
    if (flags->value != 0) {
        if (!flags->bits.is_compressed) {
            CARBON_ERROR(err, CARBON_ERR_CORRUPTED)
            return false;
        }
    }
    return true;
}

static bool serialize_string_dic(carbon_memfile_t *memfile, carbon_err_t *err, const carbon_doc_bulk_t *context, carbon_archive_compressor_type_e compressor)
{
    union carbon_archive_dic_flags flags;
    carbon_archive_compressor_t strategy;
    struct embedded_dic_header header;

    carbon_vec_t ofType (const char *) *strings;
    carbon_vec_t ofType(carbon_string_id_t) *string_ids;

    carbon_doc_bulk_get_dic_conetnts(&strings, &string_ids, context);

    assert(strings->num_elems == string_ids->num_elems);

    flags.value = 0;
    if(!compressor_strategy_by_type(err, &strategy, compressor)) {
        return false;
    }
    strategy.set_flags(&flags);
    if (!validate_embedded_dic_header_flags(err, &flags)) {
        return false;
    }

    header = (struct embedded_dic_header) {
        .marker = marker_symbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol,
        .flags = flags.value,
        .num_entries = strings->num_elems
    };

    carbon_memfile_write(memfile, &header, sizeof(struct embedded_dic_header));

    strategy.serialize_dic(memfile, strings, string_ids);

    carbon_vec_drop(strings);
    carbon_vec_drop(string_ids);
    free(strings);
    free(string_ids);
    return true;
}

static void skip_cabin_file_header(carbon_memfile_t *memfile)
{
    carbon_memfile_skip(memfile, sizeof(struct carbon_file_header));
}

static void update_carbon_file_header(carbon_memfile_t *memfile, carbon_off_t record_header_offset)
{
    carbon_off_t current_pos;
    carbon_memfile_tell(&current_pos, memfile);
    carbon_memfile_seek(memfile, 0);
    this_carbon_file_header.root_object_header_offset = record_header_offset;
    carbon_memfile_write(memfile, &this_carbon_file_header, sizeof(struct carbon_file_header));
    carbon_memfile_seek(memfile, current_pos);
}

static bool print_column_form_memfile(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile, unsigned nesting_level)
{
    carbon_off_t offset;
    carbon_memfile_tell(&offset, memfile);
    struct column_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct column_header);
    if (header->marker != MARKER_SYMBOL_COLUMN) {
        char buffer[256];
        sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_COLUMN, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }
    fprintf(file, "0x%04x ", (unsigned) offset);
    INTENT_LINE(nesting_level);
    const char *type_name = array_value_type_to_string(err, value_type_symbol_to_value_type(header->value_type));
    if (!type_name) {
        return false;
    }

    fprintf(file, "[marker: %c (Column)] [column_name: '%"PRIu64"'] [value_type: %c (%s)] [nentries: %d] [", header->marker,
            header->column_name, header->value_type, type_name, header->num_entries);

    for (size_t i = 0; i < header->num_entries; i++) {
        carbon_off_t entryOff = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
        fprintf(file, "offset: 0x%04x%s", (unsigned) entryOff, i + 1 < header->num_entries ? ", " : "");
    }

    uint32_t *positions = (uint32_t *) CARBON_MEMFILE_READ(memfile, header->num_entries * sizeof(uint32_t));
    fprintf(file, "] [positions: [");
    for (size_t i = 0; i < header->num_entries; i++) {
        fprintf(file, "%d%s", positions[i], i + 1 < header->num_entries ? ", " : "");
    }
    fprintf(file, "]]\n");

    carbon_field_type_e data_type = value_type_symbol_to_value_type(header->value_type);

    //fprintf(file, "[");
    for (size_t i = 0; i < header->num_entries; i++) {
        switch (data_type) {
            case carbon_field_type_null: {
                PRINT_VALUE_ARRAY(uint32_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_bool: {
                PRINT_VALUE_ARRAY(carbon_bool_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_int8: {
                PRINT_VALUE_ARRAY(carbon_int8_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_int16: {
                PRINT_VALUE_ARRAY(carbon_int16_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_int32: {
                PRINT_VALUE_ARRAY(carbon_int32_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_int64: {
                PRINT_VALUE_ARRAY(carbon_int64_t, memfile, header, "%"PRIi64);
            }
                break;
            case carbon_field_type_uint8: {
                PRINT_VALUE_ARRAY(carbon_uint8_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_uint16: {
                PRINT_VALUE_ARRAY(carbon_uint16_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_uint32: {
                PRINT_VALUE_ARRAY(carbon_uin32_t, memfile, header, "%d");
            }
                break;
            case carbon_field_type_uint64: {
                PRINT_VALUE_ARRAY(carbon_uin64_t, memfile, header, "%"PRIu64);
            }
                break;
            case carbon_field_type_float: {
                PRINT_VALUE_ARRAY(carbon_float_t, memfile, header, "%f");
            }
                break;
            case carbon_field_type_string: {
                PRINT_VALUE_ARRAY(carbon_string_id_t, memfile, header, "%"PRIu64"");
            }
                break;
            case carbon_field_type_object: {
                uint32_t num_elements = *CARBON_MEMFILE_READ_TYPE(memfile, uint32_t);
                INTENT_LINE(nesting_level);
                fprintf(file, "   [num_elements: %d] [values: [\n", num_elements);
                for (size_t i = 0; i < num_elements; i++) {
                    if (!print_object(file, err, memfile, nesting_level + 2)) {
                        return false;
                    }
                }
                INTENT_LINE(nesting_level);
                fprintf(file, "   ]\n");
            } break;
            default:
                CARBON_ERROR(err, CARBON_ERR_NOTYPE)
                return false;
        }
    }
    return true;
}

static bool print_object_array_from_memfile(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile, unsigned nesting_level)
{
    unsigned offset = (unsigned) CARBON_MEMFILE_TELL(memfile);
    struct object_array_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct object_array_header);
    if (header->marker != MARKER_SYMBOL_PROP_OBJECT_ARRAY) {
        char buffer[256];
        sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_PROP_OBJECT_ARRAY, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nesting_level);
    fprintf(file, "[marker: %c (Object Array)] [nentries: %d] [", header->marker, header->num_entries);

    for (size_t i = 0; i < header->num_entries; i++) {
        carbon_string_id_t string_id = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_string_id_t);
        fprintf(file, "key: %"PRIu64"%s", string_id, i + 1 < header->num_entries ? ", " : "");
    }
    fprintf(file, "] [");
    for (size_t i = 0; i < header->num_entries; i++) {
        carbon_off_t columnGroupOffset = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
        fprintf(file, "offset: 0x%04x%s", (unsigned) columnGroupOffset, i + 1 < header->num_entries ? ", " : "");
    }

    fprintf(file, "]\n");
    nesting_level++;

    for (size_t i = 0; i < header->num_entries; i++) {
        offset = CARBON_MEMFILE_TELL(memfile);
        struct column_group_header *column_group_header = CARBON_MEMFILE_READ_TYPE(memfile, struct column_group_header);
        if (column_group_header->marker != MARKER_SYMBOL_COLUMN_GROUP) {
            char buffer[256];
            sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_COLUMN_GROUP, column_group_header->marker);
            CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
            return false;
        }
        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nesting_level);
        fprintf(file, "[marker: %c (Column Group)] [num_columns: %d] [", column_group_header->marker, column_group_header->num_columns);
        for (size_t k = 0; k < column_group_header->num_columns; k++) {
            carbon_off_t column_off = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
            fprintf(file, "offset: 0x%04x%s", (unsigned) column_off, k + 1 < column_group_header->num_columns ? ", " : "");
        }
        fprintf(file, "]\n");

        for (size_t k = 0; k < column_group_header->num_columns; k++) {
            if(!print_column_form_memfile(file, err, memfile, nesting_level + 1)) {
                return false;
            }
        }

        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nesting_level);
        fprintf(file, "]\n");
    }
    return true;
}

static void print_prop_offsets(FILE *file, const carbon_archive_object_flags_t *flags, const carbon_archive_prop_offs_t *prop_offsets)
{
    if (flags->bits.has_null_props) {
        fprintf(file, " nulls: 0x%04x", (unsigned) prop_offsets->nulls);
    }
    if (flags->bits.has_bool_props) {
        fprintf(file, " bools: 0x%04x", (unsigned) prop_offsets->bools);
    }
    if (flags->bits.has_int8_props) {
        fprintf(file, " int8s: 0x%04x", (unsigned) prop_offsets->int8s);
    }
    if (flags->bits.has_int16_props) {
        fprintf(file, " int16s: 0x%04x", (unsigned) prop_offsets->int16s);
    }
    if (flags->bits.has_int32_props) {
        fprintf(file, " int32s: 0x%04x", (unsigned) prop_offsets->int32s);
    }
    if (flags->bits.has_int64_props) {
        fprintf(file, " int64s: 0x%04x", (unsigned) prop_offsets->int64s);
    }
    if (flags->bits.has_uint8_props) {
        fprintf(file, " uint8s: 0x%04x", (unsigned) prop_offsets->uint8s);
    }
    if (flags->bits.has_uint16_props) {
        fprintf(file, " uint16s: 0x%04x", (unsigned) prop_offsets->uint16s);
    }
    if (flags->bits.has_uint32_props) {
        fprintf(file, " uint32s: 0x%04x", (unsigned) prop_offsets->uint32s);
    }
    if (flags->bits.has_uint64_props) {
        fprintf(file, " uint64s: 0x%04x", (unsigned) prop_offsets->uint64s);
    }
    if (flags->bits.has_float_props) {
        fprintf(file, " floats: 0x%04x", (unsigned) prop_offsets->floats);
    }
    if (flags->bits.has_string_props) {
        fprintf(file, " texts: 0x%04x", (unsigned) prop_offsets->strings);
    }
    if (flags->bits.has_object_props) {
        fprintf(file, " objects: 0x%04x", (unsigned) prop_offsets->objects);
    }
    if (flags->bits.has_null_array_props) {
        fprintf(file, " nullArrays: 0x%04x", (unsigned) prop_offsets->null_arrays);
    }
    if (flags->bits.has_bool_array_props) {
        fprintf(file, " boolArrays: 0x%04x", (unsigned) prop_offsets->bool_arrays);
    }
    if (flags->bits.has_int8_array_props) {
        fprintf(file, " int8Arrays: 0x%04x", (unsigned) prop_offsets->int8_arrays);
    }
    if (flags->bits.has_int16_array_props) {
        fprintf(file, " int16Arrays: 0x%04x", (unsigned) prop_offsets->int16_arrays);
    }
    if (flags->bits.has_int32_array_props) {
        fprintf(file, " int32Arrays: 0x%04x", (unsigned) prop_offsets->int32_arrays);
    }
    if (flags->bits.has_int64_array_props) {
        fprintf(file, " int16Arrays: 0x%04x", (unsigned) prop_offsets->int64_arrays);
    }
    if (flags->bits.has_uint8_array_props) {
        fprintf(file, " uint8Arrays: 0x%04x", (unsigned) prop_offsets->uint8_arrays);
    }
    if (flags->bits.has_uint16_array_props) {
        fprintf(file, " uint16Arrays: 0x%04x", (unsigned) prop_offsets->uint16_arrays);
    }
    if (flags->bits.has_uint32_array_props) {
        fprintf(file, " uint32Arrays: 0x%04x", (unsigned) prop_offsets->uint32_arrays);
    }
    if (flags->bits.has_uint64_array_props) {
        fprintf(file, " uint64Arrays: 0x%04x", (unsigned) prop_offsets->uint64_arrays);
    }
    if (flags->bits.has_float_array_props) {
        fprintf(file, " floatArrays: 0x%04x", (unsigned) prop_offsets->float_arrays);
    }
    if (flags->bits.has_string_array_props) {
        fprintf(file, " textArrays: 0x%04x", (unsigned) prop_offsets->string_arrays);
    }
    if (flags->bits.has_object_array_props) {
        fprintf(file, " objectArrays: 0x%04x", (unsigned) prop_offsets->object_arrays);
    }
}

static void read_prop_offsets(carbon_archive_prop_offs_t *prop_offsets, carbon_memfile_t *memfile, const carbon_archive_object_flags_t *flags)
{
    if (flags->bits.has_null_props) {
        prop_offsets->nulls = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_bool_props) {
        prop_offsets->bools = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int8_props) {
        prop_offsets->int8s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int16_props) {
        prop_offsets->int16s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int32_props) {
        prop_offsets->int32s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int64_props) {
        prop_offsets->int64s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint8_props) {
        prop_offsets->uint8s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint16_props) {
        prop_offsets->uint16s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint32_props) {
        prop_offsets->uint32s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint64_props) {
        prop_offsets->uint64s = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_float_props) {
        prop_offsets->floats = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_string_props) {
        prop_offsets->strings = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_object_props) {
        prop_offsets->objects = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_null_array_props) {
        prop_offsets->null_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_bool_array_props) {
        prop_offsets->bool_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int8_array_props) {
        prop_offsets->int8_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int16_array_props) {
        prop_offsets->int16_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int32_array_props) {
        prop_offsets->int32_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_int64_array_props) {
        prop_offsets->int64_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint8_array_props) {
        prop_offsets->uint8_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint16_array_props) {
        prop_offsets->uint16_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint32_array_props) {
        prop_offsets->uint32_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_uint64_array_props) {
        prop_offsets->uint64_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_float_array_props) {
        prop_offsets->float_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_string_array_props) {
        prop_offsets->string_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
    if (flags->bits.has_object_array_props) {
        prop_offsets->object_arrays = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);
    }
}

typedef struct
{
    struct simple_prop_header *header;
    const carbon_string_id_t *keys;
    const void *values;
} ewmbedded_fixed_prop_t;

typedef struct
{
    struct simple_prop_header *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *groupOffs;
} embedded_table_prop_t;

typedef struct
{
    struct simple_prop_header *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *offsets;
    const void *values;
} embedded_var_prop_t;

typedef struct
{
    struct simple_prop_header *header;
    const carbon_string_id_t *keys;
    const uint32_t *lengths;
    carbon_off_t values_begin;
} embedded_array_prop_t;

typedef struct
{
    struct simple_prop_header *header;
    const carbon_string_id_t *keys;
} embedded_null_prop_t;

static void embedded_fixed_props_read(ewmbedded_fixed_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

static void embedded_var_props_read(embedded_var_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->offsets = (carbon_off_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_off_t));
    prop->values = carbon_memfile_peek(memfile, 1);
}

static void embedded_null_props_read(embedded_null_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
}

static void embedded_array_props_read(embedded_array_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->lengths = (uint32_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(uint32_t));
    prop->values_begin = CARBON_MEMFILE_TELL(memfile);
}

static void embedded_table_props_read(embedded_table_prop_t *prop, carbon_memfile_t *memfile) {
    prop->header->marker = *CARBON_MEMFILE_READ_TYPE(memfile, char);
    prop->header->num_entries = *CARBON_MEMFILE_READ_TYPE(memfile, uint8_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_string_id_t));
    prop->groupOffs = (carbon_off_t *) CARBON_MEMFILE_READ(memfile, prop->header->num_entries * sizeof(carbon_off_t));
}

bool print_object(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile, unsigned nesting_level)
{
    unsigned offset = (unsigned) CARBON_MEMFILE_TELL(memfile);
    struct object_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct object_header);

    carbon_archive_prop_offs_t prop_offsets;
    carbon_archive_object_flags_t flags = {
        .value = header->flags
    };

    read_prop_offsets(&prop_offsets, memfile, &flags);
    carbon_off_t nextObjectOrNil = *CARBON_MEMFILE_READ_TYPE(memfile, carbon_off_t);

    if (header->marker != MARKER_SYMBOL_OBJECT_BEGIN) {
        char buffer[256];
        sprintf(buffer, "Parsing error: expected object marker [{] but found [%c]\"", header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nesting_level);
    nesting_level++;
    fprintf(file, "[marker: %c (BeginObject)] [flags: %u] [propertyOffsets: [", header->marker, header->flags);
    print_prop_offsets(file, &flags, &prop_offsets);
    fprintf(file, " ] [next: 0x%04x] \n", (unsigned) nextObjectOrNil);

    bool continue_read = true;
    while (continue_read) {
        offset = CARBON_MEMFILE_TELL(memfile);
        char entryMarker = *CARBON_MEMFILE_PEEK(memfile, char);

        switch (entryMarker) {
            case MARKER_SYMBOL_PROP_NULL: {
                struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level)
                fprintf(file, "[marker: %c (null)] [nentries: %d] [", entryMarker, prop_header->num_entries);

                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");
                }
                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_BOOLEAN: {
                struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);
                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));
                carbon_bool_t *values = (carbon_bool_t *) CARBON_MEMFILE_READ(memfile,
                                                                prop_header->num_entries * sizeof(carbon_bool_t));
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level)
                fprintf(file, "[marker: %c (boolean)] [nentries: %d] [", entryMarker, prop_header->num_entries);
                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");
                }
                fprintf(file, "] [");
                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "%s%s", values[i] ? "true" : "false", i + 1 < prop_header->num_entries ? ", " : "");
                }
                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_INT8:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_int8_t, "Int8", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT16:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_int16_t, "Int16", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT32:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_int32_t, "Int32", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT64:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_int64_t, "Int64", "%"PRIi64);
                break;
            case MARKER_SYMBOL_PROP_UINT8:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_uint8_t, "UInt8", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT16:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_uint16_t, "UInt16", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT32:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_uin32_t, "UInt32", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT64:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_uin64_t, "UInt64", "%"PRIu64);
                break;
            case MARKER_SYMBOL_PROP_REAL:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_float_t , "Float", "%f");
                break;
            case MARKER_SYMBOL_PROP_TEXT:
                PRINT_SIMPLE_PROPS(file, memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, carbon_string_id_t, "Text", "%"PRIu64"");
                break;
            case MARKER_SYMBOL_PROP_OBJECT: {
                embedded_var_prop_t prop;
                embedded_var_props_read(&prop, memfile);
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level)
                fprintf(file, "[marker: %c (Object)] [nentries: %d] [", entryMarker, prop.header->num_entries);
                for (uint32_t i = 0; i < prop.header->num_entries; i++) {
                    fprintf(file, "key: %"PRIu64"%s", prop.keys[i], i + 1 < prop.header->num_entries ? ", " : "");
                }
                fprintf(file, "] [");
                for (uint32_t i = 0; i < prop.header->num_entries; i++) {
                    fprintf(file, "offsets: 0x%04x%s", (unsigned) prop.offsets[i], i + 1 < prop.header->num_entries ? ", " : "");
                }
                fprintf(file, "] [\n");

                char nextEntryMarker;
                do {
                    if (!print_object(file, err, memfile, nesting_level + 1)) {
                        return false;
                    }
                    nextEntryMarker = *CARBON_MEMFILE_PEEK(memfile, char);
                } while (nextEntryMarker == MARKER_SYMBOL_OBJECT_BEGIN);

            } break;
            case MARKER_SYMBOL_PROP_NULL_ARRAY: {
                struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);

                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));
                uint32_t *nullArrayLengths;

                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level)
                fprintf(file, "[marker: %c (Null Array)] [nentries: %d] [", entryMarker, prop_header->num_entries);

                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");
                }
                fprintf(file, "] [");

                nullArrayLengths = (uint32_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(uint32_t));

                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "nentries: %d%s", nullArrayLengths[i], i + 1 < prop_header->num_entries ? ", " : "");
                }

                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_BOOLEAN_ARRAY: {
                struct simple_prop_header *prop_header = CARBON_MEMFILE_READ_TYPE(memfile, struct simple_prop_header);

                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(carbon_string_id_t));
                uint32_t *array_lengths;

                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nesting_level)
                fprintf(file, "[marker: %c (Boolean Array)] [nentries: %d] [", entryMarker, prop_header->num_entries);

                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < prop_header->num_entries ? ", " : "");
                }
                fprintf(file, "] [");

                array_lengths = (uint32_t *) CARBON_MEMFILE_READ(memfile, prop_header->num_entries * sizeof(uint32_t));

                for (uint32_t i = 0; i < prop_header->num_entries; i++) {
                    fprintf(file, "arrayLength: %d%s", array_lengths[i], i + 1 < prop_header->num_entries ? ", " : "");
                }

                fprintf(file, "] [");

                for (uint32_t array_idx = 0; array_idx < prop_header->num_entries; array_idx++) {
                    carbon_bool_t *values = (carbon_bool_t *) CARBON_MEMFILE_READ(memfile,
                                                                   array_lengths[array_idx] * sizeof(carbon_bool_t));
                    fprintf(file, "[");
                    for (uint32_t i = 0; i < array_lengths[array_idx]; i++) {
                        fprintf(file, "value: %s%s", values[i] ? "true" : "false", i + 1 < array_lengths[array_idx] ? ", " : "");
                    }
                    fprintf(file, "]%s", array_idx + 1 < prop_header->num_entries ? ", " : "");
                }

                fprintf(file, "]\n");
            } break;
                break;
            case MARKER_SYMBOL_PROP_INT8_ARRAY: {
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_int8_t, "Int8 Array", "%d");
             } break;
            case MARKER_SYMBOL_PROP_INT16_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_int16_t, "Int16 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT32_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_int32_t, "Int32 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT64_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_int64_t, "Int64 Array", "%"PRIi64);
                break;
            case MARKER_SYMBOL_PROP_UINT8_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_uint8_t, "UInt8 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT16_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_uint16_t, "UInt16 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT32_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_uin32_t, "UInt32 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT64_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_uin64_t, "UInt64 Array", "%"PRIu64);
                break;
            case MARKER_SYMBOL_PROP_REAL_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_float_t, "Float Array", "%f");
                break;
            case MARKER_SYMBOL_PROP_TEXT_ARRAY:
                PRINT_ARRAY_PROPS(memfile, CARBON_MEMFILE_TELL(memfile), nesting_level, entryMarker, carbon_string_id_t, "Text Array", "%"PRIu64"");
                break;
            case MARKER_SYMBOL_PROP_OBJECT_ARRAY:
                if(!print_object_array_from_memfile(file, err, memfile, nesting_level)) {
                    return false;
                }
                break;
            case MARKER_SYMBOL_OBJECT_END:
                continue_read = false;
                break;
            default: {
                char buffer[256];
                sprintf(buffer, "Parsing error: unexpected marker [%c] was detected in file %p", entryMarker, memfile);
                CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
                return false;
            }
        }
    }

    offset = CARBON_MEMFILE_TELL(memfile);
    char end_marker = *CARBON_MEMFILE_READ_TYPE(memfile, char);
    assert (end_marker == MARKER_SYMBOL_OBJECT_END);
    nesting_level--;
    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nesting_level);
    fprintf(file, "[marker: %c (EndObject)]\n", end_marker);
    return true;
}

static bool is_valid_carbon_file(const struct carbon_file_header *header)
{
    if (CARBON_ARRAY_LENGTH(header->magic) != strlen(CABIN_FILE_MAGIC)) {
        return false;
    } else {
        for (size_t i = 0; i < CARBON_ARRAY_LENGTH(header->magic); i++) {
            if (header->magic[i] != CABIN_FILE_MAGIC[i]) {
                return false;
            }
        }
        if (header->version != CABIN_FILE_VERSION) {
            return false;
        }
        if (header->root_object_header_offset == 0) {
            return false;
        }
        return true;
    }
}

static void dumpPrintRecordHeader(FILE *file, carbon_memfile_t *memfile)
{
    unsigned offset = CARBON_MEMFILE_TELL(memfile);
    struct record_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct record_header);
    carbon_archive_record_flags_t flags;
    flags.value = header->flags;
    char *flagsString = record_header_flags_to_string(&flags);
    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[marker: %c] [flags: %s] [record_size: 0x%04x]\n",
            header->marker, flagsString, (unsigned) header->record_size);
    free(flagsString);
}

static bool dumpPrintCabinHeader(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile)
{
    unsigned offset = CARBON_MEMFILE_TELL(memfile);
    assert(carbon_memfile_size(memfile) > sizeof(struct carbon_file_header));
    struct carbon_file_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct carbon_file_header);
    if (!is_valid_carbon_file(header)) {
        CARBON_ERROR(err, CARBON_ERR_NOARCHIVEFILE)
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[magic: " CABIN_FILE_MAGIC "] [version: %d] [recordOffset: 0x%04x]\n",
            header->version, (unsigned) header->root_object_header_offset);
    return true;
}

static bool dumpEmbeddedDic(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile)
{
    carbon_archive_compressor_t strategy;
    union carbon_archive_dic_flags flags;

    unsigned offset = CARBON_MEMFILE_TELL(memfile);
    struct embedded_dic_header *header = CARBON_MEMFILE_READ_TYPE(memfile, struct embedded_dic_header);
    if (header->marker != marker_symbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
        char buffer[256];
        sprintf(buffer, "expected [%c] marker, but found [%c]", marker_symbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }
    flags.value = header->flags;

    char *flagsStr = embedded_dic_flags_to_string(&flags);
    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[marker: %c] [nentries: %d] [flags:%s]\n", header->marker,
            header->num_entries, flagsStr);
    free(flagsStr);

    if (compressor_strategy_by_flags(&strategy, &flags) != true) {
        CARBON_ERROR(err, CARBON_ERR_NOCOMPRESSOR);
        return false;
    }
    strategy.dump_dic(file, memfile);
    return true;
}

static bool print_archive_from_memfile(FILE *file, carbon_err_t *err, carbon_memfile_t *memfile)
{
    if (!dumpPrintCabinHeader(file, err, memfile)) {
        return false;
    }
    if (!dumpEmbeddedDic(file, err, memfile)) {
        return false;
    }
    dumpPrintRecordHeader(file, memfile);
    if (!print_object(file, err, memfile, 0)) {
        return false;
    }
    return true;
}

static carbon_archive_object_flags_t *get_flags(carbon_archive_object_flags_t *flags, carbon_columndoc_obj_t *columndoc) {
    CARBON_ZERO_MEMORY(flags, sizeof(carbon_archive_object_flags_t));
    flags->bits.has_null_props         = (columndoc->null_prop_keys.num_elems > 0);
    flags->bits.has_bool_props      = (columndoc->bool_prop_keys.num_elems > 0);
    flags->bits.has_int8_props         = (columndoc->int8_prop_keys.num_elems > 0);
    flags->bits.has_int16_props        = (columndoc->int16_prop_keys.num_elems > 0);
    flags->bits.has_int32_props        = (columndoc->int32_prop_keys.num_elems > 0);
    flags->bits.has_int64_props        = (columndoc->int64_prop_keys.num_elems > 0);
    flags->bits.has_uint8_props        = (columndoc->uint8_prop_keys.num_elems > 0);
    flags->bits.has_uint16_props       = (columndoc->uint16_prop_keys.num_elems > 0);
    flags->bits.has_uint32_props       = (columndoc->uin32_prop_keys.num_elems > 0);
    flags->bits.has_uint64_props       = (columndoc->uint64_prop_keys.num_elems > 0);
    flags->bits.has_float_props         = (columndoc->float_prop_keys.num_elems > 0);
    flags->bits.has_string_props       = (columndoc->string_prop_keys.num_elems > 0);
    flags->bits.has_object_props       = (columndoc->obj_prop_keys.num_elems > 0);
    flags->bits.has_null_array_props    = (columndoc->null_array_prop_keys.num_elems > 0);
    flags->bits.has_bool_array_props = (columndoc->bool_array_prop_keys.num_elems > 0);
    flags->bits.has_int8_array_props    = (columndoc->int8_array_prop_keys.num_elems > 0);
    flags->bits.has_int16_array_props   = (columndoc->int16_array_prop_keys.num_elems > 0);
    flags->bits.has_int32_array_props   = (columndoc->int32_array_prop_keys.num_elems > 0);
    flags->bits.has_int64_array_props   = (columndoc->int64_array_prop_keys.num_elems > 0);
    flags->bits.has_uint8_array_props   = (columndoc->uint8_array_prop_keys.num_elems > 0);
    flags->bits.has_uint16_array_props  = (columndoc->uint16_array_prop_keys.num_elems > 0);
    flags->bits.has_uint32_array_props  = (columndoc->uint32_array_prop_keys.num_elems > 0);
    flags->bits.has_uint64_array_props  = (columndoc->uint64_array_prop_keys.num_elems > 0);
    flags->bits.has_float_array_props    = (columndoc->float_array_prop_keys.num_elems > 0);
    flags->bits.has_string_array_props  = (columndoc->string_array_prop_keys.num_elems > 0);
    flags->bits.has_object_array_props  = (columndoc->obj_array_props.num_elems > 0);
    assert(flags->value != 0);
    return flags;
}

static bool initDecompressor(carbon_archive_record_table_t *file);
static bool readRecord(carbon_archive_record_table_t *file, carbon_off_t record_header_offset);
static void resetStringDicDiskFileCursor(carbon_archive_record_table_t *file);
static void convertObjectToModel(carbon_doc_t *model, carbon_archive_object_t *obj);

bool carbon_archive_open(carbon_archive_t *out,
                        const char *file_path)
{
    CARBON_UNUSED(out);
    CARBON_UNUSED(file_path);

    int status;

    out->record_table.diskFile = fopen(file_path, "r");
    if (!out->record_table.diskFile) {
        CARBON_PRINT_ERROR(CARBON_ERR_FOPEN_FAILED);
        return false;
    } else {
        struct carbon_file_header header;
        size_t nread = fread(&header, sizeof(struct carbon_file_header), 1, out->record_table.diskFile);
        if (nread != 1) {
            fclose(out->record_table.diskFile);
            CARBON_PRINT_ERROR(CARBON_ERR_IO);
            return false;
        } else {
            if (!is_valid_carbon_file(&header)) {
                CARBON_PRINT_ERROR(CARBON_ERR_FORMATVERERR);
                return false;
            } else {
                if ((status = initDecompressor(&out->record_table)) != true) {
                    return status;
                }
                if ((status = readRecord(&out->record_table, header.root_object_header_offset)) != true) {
                    return status;
                }

                fseek(out->record_table.diskFile, sizeof(struct carbon_file_header), SEEK_SET);
                carbon_off_t stringDicStart = ftell(out->record_table.diskFile);
                fseek(out->record_table.diskFile, 0, SEEK_END);
                carbon_off_t fileEnd = ftell(out->record_table.diskFile);
                fseek(out->record_table.diskFile, stringDicStart, SEEK_SET);
                carbon_error_init(&out->err);
                out->info.string_table_size = header.root_object_header_offset - stringDicStart;
                out->info.record_table_size = fileEnd - header.root_object_header_offset;
                carbon_error_init(&out->err);

                resetStringDicDiskFileCursor(&out->record_table);
            }
        }
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_archive_get_info(carbon_archive_info_t *info, const struct carbon_archive *archive)
{
    CARBON_NON_NULL_OR_ERROR(info);
    CARBON_NON_NULL_OR_ERROR(archive);
    *info = archive->info;
    return true;
}

static carbon_off_t objectSetup(carbon_archive_object_t *obj, carbon_memblock_t *memBlock, carbon_off_t objectHeaderOffset, carbon_archive_record_table_t *context)
{
    carbon_memfile_open(&obj->file, memBlock, CARBON_MEMFILE_MODE_READONLY);
    carbon_memfile_seek(&obj->file, objectHeaderOffset);
    assert (*CARBON_MEMFILE_PEEK(&obj->file, char) == MARKER_SYMBOL_OBJECT_BEGIN);
    struct object_header *header = CARBON_MEMFILE_READ_TYPE(&obj->file, struct object_header);
    carbon_archive_object_flags_t flags = {
        .value = header->flags
    };
    carbon_error_init(&obj->err);
    obj->context = context;
    obj->flags.value = header->flags;
    read_prop_offsets(&obj->props, &obj->file, &flags);
    obj->self = objectHeaderOffset;
    carbon_off_t readLength = CARBON_MEMFILE_TELL(&obj->file) - objectHeaderOffset;
    carbon_memfile_seek(&obj->file, objectHeaderOffset);
    return readLength;
}

bool carbon_archive_close(carbon_archive_t *archive)
{
    CARBON_NON_NULL_OR_ERROR(archive);
    fclose(archive->record_table.diskFile);
    carbon_memblock_drop(archive->record_table.recordDataBase);
    return true;
}

static bool initDecompressor(carbon_archive_record_table_t *file)
{
    assert(file->diskFile);

    struct embedded_dic_header header;
    union carbon_archive_dic_flags flags;

    fread(&header, sizeof(struct embedded_dic_header), 1, file->diskFile);
    if (header.marker != marker_symbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
        CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
        return false;
    }

    flags.value = header.flags;

    if (compressor_strategy_by_flags(&file->strategy, &flags) != true) {
        return false;
    }

    return true;
}

static bool readRecord(carbon_archive_record_table_t *file, carbon_off_t record_header_offset)
{
    carbon_err_t err;
    fseek(file->diskFile, record_header_offset, SEEK_SET);
    struct record_header header;
    if (fread(&header, sizeof(struct record_header), 1, file->diskFile) != 1) {
        CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
        return false;
    } else {
        file->flags.value = header.flags;
        bool status = carbon_memblock_from_file(&file->recordDataBase, file->diskFile, header.record_size);
        if (!status) {
            carbon_memblock_get_error(&err, file->recordDataBase);
            carbon_error_cpy(&file->err, &err);
            return false;
        }

        carbon_memfile_t memfile;
        if (carbon_memfile_open(&memfile, file->recordDataBase, CARBON_MEMFILE_MODE_READONLY) != true) {
            CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
            status = false;
        }
        if (*CARBON_MEMFILE_PEEK(&memfile, char) != MARKER_SYMBOL_OBJECT_BEGIN) {
            CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
            status = false;
        }
        return true;
    }
}

static void resetStringDicDiskFileCursor(carbon_archive_record_table_t *file)
{
    fseek(file->diskFile, sizeof(struct carbon_file_header) + sizeof(struct embedded_dic_header), SEEK_SET);
}

CARBON_FUNC_UNUSED
static void convertObjectToModel(carbon_doc_t *model, carbon_archive_object_t *obj)
{
//    carbon_doc_t
    CARBON_UNUSED(model);
    CARBON_UNUSED(obj);
}

static void reset_cabin_object_mem_file(carbon_archive_object_t *object)
{
    carbon_memfile_seek(&object->file, object->self);
}

CARBON_FUNC_UNUSED
static void getObjectProperties(carbon_archive_object_t *object)
{
    if (object->flags.bits.has_object_props) {
        assert(object->props.objects != 0);
        carbon_memfile_seek(&object->file, object->props.objects);
        reset_cabin_object_mem_file(object);
    } else {

    }
}

CARBON_EXPORT(bool)
carbon_archive_record(carbon_archive_object_t *root, carbon_archive_t *archive)
{
    CARBON_NON_NULL_OR_ERROR(root)
    CARBON_NON_NULL_OR_ERROR(archive)
    objectSetup(root, archive->record_table.recordDataBase, 0, &archive->record_table);
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
            embedded_null_prop_t prop;
            embedded_null_props_read(&prop, &obj->file);
            reset_cabin_object_mem_file(obj);
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
            embedded_var_prop_t objectProp;
            embedded_var_props_read(&objectProp, &obj->file);
            reset_cabin_object_mem_file(obj);
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
        embedded_table_prop_t prop;
        embedded_table_props_read(&prop, &obj->file);
        reset_cabin_object_mem_file(obj);
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

CARBON_EXPORT(bool)
carbon_archive_table_get_error(carbon_err_t *out, carbon_archive_table_t *table)
{
    CARBON_NON_NULL_OR_ERROR(out)
    CARBON_NON_NULL_OR_ERROR(table)
    carbon_error_cpy(out, &table->err);
    return true;
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
        embedded_var_prop_t objectProp;
        embedded_var_props_read(&objectProp, &props->file);
        if (idx > objectProp.header->num_entries) {
            reset_cabin_object_mem_file(props);
            CARBON_ERROR(&props->err, CARBON_ERR_NOTFOUND);
            return false;
        } else {
            objectSetup(out, props->context->recordDataBase, objectProp.offsets[idx], props->context);
            return true;
        }
    }
}

#define OBJECT_GET_VALUES_OF_FIX_TYPE_GENERIC(obj, bitflagPropName, offsetPropName, T)                              \
({                                                                                                                     \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR);                                                \
    }                                                                                                                  \
                                                                                                                       \
    const void *result = NULL;                                                                                         \
                                                                                                                       \
    if (obj->flags.bits.bitflagPropName) {                                                                       \
        assert(obj->props.offsetPropName != 0);                                                                  \
        carbon_memfile_seek(&obj->file, obj->props.offsetPropName);                                                   \
        ewmbedded_fixed_prop_t prop;                                                                                        \
        embedded_fixed_props_read(&prop, &obj->file);                                                                  \
        reset_cabin_object_mem_file(obj);                                                                                  \
        CARBON_OPTIONAL_SET(npairs, prop.header->num_entries);                                                               \
        result = prop.values;                                                                                          \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(npairs, 0);                                                                                     \
    }                                                                                                                  \
    (const T *) result;                                                                                             \
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

#define OBJECT_GET_ARRAY_LENGTHS_GENERIC(err, length, obj, bitfielName, offset_name, idx, prop)                              \
({                                                                                                                     \
    int status;                                                                                                        \
                                                                                                                       \
    if (obj->flags.bits.bitfielName) {                                                                           \
        assert(obj->props.offset_name != 0);                                                                      \
        carbon_memfile_seek(&obj->file, obj->props.offset_name);                                                       \
        embedded_array_props_read(&prop, &obj->file);                                                                  \
        reset_cabin_object_mem_file(obj);                                                                                  \
        if (CARBON_BRANCH_UNLIKELY(idx >= prop.header->num_entries)) {                                                         \
            *length = 0;                                                                                               \
            CARBON_ERROR(err, CARBON_ERR_OUTOFBOUNDS);                                                   \
            status = false;                                                                               \
        } else {                                                                                                       \
            *length = prop.lengths[idx];                                                                               \
            status = true;                                                                                        \
        }                                                                                                              \
    } else {                                                                                                           \
        *length = 0;                                                                                                   \
        CARBON_ERROR(err, CARBON_ERR_NOTFOUND);                                                      \
        status = false;                                                                                      \
    }                                                                                                                  \
    status;                                                                                                            \
})

#define OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, T)                                                  \
({                                                                                                                     \
    const void *result = NULL;                                                                                         \
    if (status == true) {                                                                                         \
        size_t skipSize = 0;                                                                                           \
        for (size_t i = 0; i < idx; i++) {                                                                             \
            skipSize += prop.lengths[i] * sizeof(T);                                                                \
        }                                                                                                              \
        carbon_memfile_seek(&obj->file, prop.values_begin + skipSize);                                                       \
        result = carbon_memfile_peek(&obj->file, 1);                                                                        \
        reset_cabin_object_mem_file(obj);                                                                                  \
    }                                                                                                                  \
    (const T*) result;                                                                                              \
})

const carbon_int8_t *carbon_archive_object_values_int8_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int8_array_props,
                                                  int8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int8_t);
}

const carbon_int16_t *carbon_archive_object_values_int16_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int16_array_props,
                                                  int16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int16_t);
}

const carbon_int32_t *carbon_archive_object_values_int32_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int32_array_props,
                                                  int32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int32_t);
}

const carbon_int64_t *carbon_archive_object_values_int64_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int64_array_props,
                                                  int64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int64_t);
}

const carbon_uint8_t *carbon_archive_object_values_uint8_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint8_array_props,
                                                  uint8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint8_t);
}

const carbon_uint16_t *carbon_archive_object_values_uint16_arrays(uint32_t *length,
                                                                  size_t idx,
                                                                  carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint16_array_props,
                                                  uint16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint16_t);
}

const carbon_uin32_t *carbon_archive_object_values_uint32_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint32_array_props,
                                                  uint32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin32_t);
}

const carbon_uin64_t *carbon_archive_object_values_uint64_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint64_array_props,
                                                  uint64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin64_t);
}

const carbon_bool_t *carbon_archive_object_values_bool_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_bool_array_props,
                                                  bool_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_bool_t);
}

const carbon_float_t *carbon_archive_object_values_float_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_float_array_props,
                                                  float_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_float_t);
}

const carbon_string_id_t *carbon_archive_object_values_string_arrays(uint32_t *length,
                                                                     size_t idx,
                                                                     carbon_archive_object_t *obj)
{
    embedded_array_prop_t prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_string_array_props,
                                                  string_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_string_id_t);
}

bool carbon_archive_object_values_null_array_lengths(uint32_t *length, size_t idx, carbon_archive_object_t *obj)
{
    CARBON_NON_NULL_OR_ERROR(length);
    CARBON_NON_NULL_OR_ERROR(obj);
    embedded_array_prop_t prop;
    return OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_null_array_props,
                                            null_arrays, idx, prop);
}

bool carbon_archive_table_column_group(carbon_column_group_t *group, size_t idx, carbon_archive_table_t *table)
{
    CARBON_NON_NULL_OR_ERROR(group);
    CARBON_NON_NULL_OR_ERROR(table);
    CARBON_NON_NULL_OR_ERROR(table->context);

    if (CARBON_BRANCH_UNLIKELY(idx >= table->ngroups)) {
        CARBON_ERROR(&table->err, CARBON_ERR_OUTOFBOUNDS);
        return false;
    } else {
        carbon_off_t groupOff = table->groups_offsets[idx];
        carbon_off_t last = CARBON_MEMFILE_TELL(&table->context->file);
        carbon_memfile_seek(&table->context->file, groupOff);
        struct column_group_header *column_group_header = CARBON_MEMFILE_READ_TYPE(&table->context->file,
                                                                        struct column_group_header);
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
    if (CARBON_BRANCH_UNLIKELY(idx >= group->ncolumns)) {
        CARBON_ERROR(&group->err, CARBON_ERR_OUTOFBOUNDS);
        return false;
    } else {
        carbon_off_t last = CARBON_MEMFILE_TELL(&group->context->file);
        carbon_off_t column_off = group->column_offsets[idx];
        carbon_memfile_seek(&group->context->file, column_off);
        const struct column_header *header = CARBON_MEMFILE_READ_TYPE(&group->context->file, struct column_header);
        column->nelems = header->num_entries;
        column->type = get_value_type_of_char(header->value_type);
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
    if (CARBON_BRANCH_UNLIKELY(idx >= column->nelems)) {
        CARBON_ERROR(&column->err, CARBON_ERR_OUTOFBOUNDS);
    } else {
        carbon_off_t last = CARBON_MEMFILE_TELL(&column->context->file);
        carbon_off_t entryOff = column->entry_offsets[idx];
        carbon_memfile_seek(&column->context->file, entryOff);
        field->nentries = *CARBON_MEMFILE_READ_TYPE(&column->context->file, uint32_t);
        field->data = carbon_memfile_peek(&column->context->file, 1);
        field->type = column->type;
        field->context = column->context;
        field->data_offset = entryOff + sizeof(uint32_t);
        carbon_memfile_seek(&column->context->file, last);
        return true;
    }
    return true;
}

#define FIELD_GET_VALUE_ARRAY_GENERIC(length, field, expectedType, T)                                               \
({                                                                                                                     \
    assert(length);                                                                                                    \
    assert(field);                                                                                                     \
    CARBON_PRINT_ERROR_AND_DIE_IF(field->type != expectedType, CARBON_ERR_ERRINTERNAL)                                                                    \
    *length = field->nentries;                                                                                       \
    (const T *) field->data;                                                                                        \
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
    /** array does not map to array of type NULL */
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
        carbon_off_t readLength = objectSetup(&cursor->obj, cursor->mem_block, cursor->field->data_offset,
                                        cursor->field->context->context);
        cursor->field->data_offset += readLength;
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











