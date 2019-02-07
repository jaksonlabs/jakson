/*
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

// ---------------------------------------------------------------------------------------------------------------------
//
//
//
//  C A B I N   F I L E   B Y T E   S T R E A M
//
//
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define WRITE_PRIMITIVE_VALUES(memFile, valuesVec, type)                                                               \
{                                                                                                                      \
    type *values = VECTOR_ALL(valuesVec, type);                                                                        \
    carbon_memfile_write(memFile, values, valuesVec->numElems * sizeof(type));                                                 \
}

#define WRITE_ARRAY_VALUES(memFile, valuesVec, type)                                                                   \
{                                                                                                                      \
    for (uint32_t i = 0; i < valuesVec->numElems; i++) {                                                               \
        carbon_vec_t ofType(type) *nestedValues = VECTOR_GET(valuesVec, i, carbon_vec_t);                            \
        WRITE_PRIMITIVE_VALUES(memFile, nestedValues, type);                                                           \
    }                                                                                                                  \
}

#define OBJECT_GET_KEYS_TO_FIX_TYPE_GENERIC(numPairs, obj, bitFlagName, offsetName)                                    \
{                                                                                                                      \
    if (!obj) {                                                                                                        \
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NULLPTR)                                                   \
    }                                                                                                                  \
                                                                                                                       \
    if (obj->flags.bits.bitFlagName) {                                                                           \
        assert(obj->props.offsetName != 0);                                                                      \
        carbon_memfile_seek(&obj->file, obj->props.offsetName);                                                       \
        EmbeddedFixedProp prop;                                                                                        \
        embeddedFixedPropsRead(&prop, &obj->file);                                                                  \
        resetCabinObjectMemFile(obj);                                                                                  \
        CARBON_OPTIONAL_SET(numPairs, prop.header->numEntries);                                                               \
        return prop.keys;                                                                                              \
    } else {                                                                                                           \
        CARBON_OPTIONAL_SET(numPairs, 0);                                                                                     \
        return NULL;                                                                                                   \
    }                                                                                                                  \
}

#define PRINT_SIMPLE_PROPS(file, memFile, offset, nestingLevel, valueType, typeString, formatString)                   \
{                                                                                                                      \
    struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);                         \
    carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));                    \
    valueType *values = (valueType *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(valueType));               \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nestingLevel)                                                                                          \
    fprintf(file, "[marker: %c (" typeString ")] [numEntries: %d] [", entryMarker, propHeader->numEntries);            \
    for (uint32_t i = 0; i < propHeader->numEntries; i++) {                                                            \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");                              \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
    for (uint32_t i = 0; i < propHeader->numEntries; i++) {                                                            \
      fprintf(file, "value: "formatString"%s", values[i], i + 1 < propHeader->numEntries ? ", " : "");                 \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

#define PRINT_ARRAY_PROPS(memFile, offset, nestingLevel, entryMarker, type, typeString, formatString)                  \
{                                                                                                                      \
    struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);                         \
                                                                                                                       \
    carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));                    \
    uint32_t *arrayLengths;                                                                                            \
                                                                                                                       \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                       \
    INTENT_LINE(nestingLevel)                                                                                          \
    fprintf(file, "[marker: %c ("typeString")] [numEntries: %d] [", entryMarker, propHeader->numEntries);              \
                                                                                                                       \
    for (uint32_t i = 0; i < propHeader->numEntries; i++) {                                                            \
        fprintf(file, "key: %"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");                              \
    }                                                                                                                  \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    arrayLengths = (uint32_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(uint32_t));                      \
                                                                                                                       \
    for (uint32_t i = 0; i < propHeader->numEntries; i++) {                                                            \
        fprintf(file, "numEntries: %d%s", arrayLengths[i], i + 1 < propHeader->numEntries ? ", " : "");                \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "] [");                                                                                              \
                                                                                                                       \
    for (uint32_t arrayIdx = 0; arrayIdx < propHeader->numEntries; arrayIdx++) {                                       \
        type *values = (type *) CARBON_MEMFILE_READ(memFile, arrayLengths[arrayIdx] * sizeof(type));                          \
        fprintf(file, "[");                                                                                            \
        for (uint32_t i = 0; i < arrayLengths[arrayIdx]; i++) {                                                        \
            fprintf(file, "value: "formatString"%s", values[i], i + 1 < arrayLengths[arrayIdx] ? ", " : "");           \
        }                                                                                                              \
        fprintf(file, "]%s", arrayIdx + 1 < propHeader->numEntries ? ", " : "");                                       \
    }                                                                                                                  \
                                                                                                                       \
    fprintf(file, "]\n");                                                                                              \
}

#define INTENT_LINE(nestingLevel)                                                                                      \
{                                                                                                                      \
    for (unsigned nestLevel = 0; nestLevel < nestingLevel; nestLevel++) {                                              \
        fprintf(file, "   ");                                                                                          \
    }                                                                                                                  \
}

#define PRINT_VALUE_ARRAY(type, memFile, header, formatString)                                                         \
{                                                                                                                      \
    uint32_t numElements = *CARBON_MEMFILE_READ_TYPE(memFile, uint32_t);                                                      \
    const type *values = (const type *) CARBON_MEMFILE_READ(memFile, numElements * sizeof(type));                             \
    fprintf(file, "0x%04x ", (unsigned) offset);                                                                        \
    INTENT_LINE(nestingLevel);                                                                                         \
    fprintf(file, "   [numElements: %d] [values: [", numElements);                                                     \
    for (size_t i = 0; i < numElements; i++) {                                                                         \
        fprintf(file, "value: "formatString"%s", values[i], i + 1 < numElements ? ", " : "");                          \
    }                                                                                                                  \
    fprintf(file, "]\n");                                                                                              \
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

enum MarkerType
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

struct MarkerEntry
{
    enum MarkerType type;
    char symbol;
} markerSymbols [] = {
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
    carbon_field_type_e valueType;
    enum MarkerType marker;
} valueArrayMarkerMapping [] = {
    { carbon_field_type_null,    MARKER_TYPE_PROP_NULL_ARRAY },
    { carbon_field_type_bool, MARKER_TYPE_PROP_BOOLEAN_ARRAY },
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
    { carbon_field_type_bool, MARKER_TYPE_PROP_BOOLEAN },
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

struct __attribute__((packed)) CabinFileHeader
{
    char magic[9];
    uint8_t version;
    carbon_off_t rootObjectHeaderOffset;
};

struct __attribute__((packed)) RecordHeader
{
    char marker;
    uint8_t flags;
    uint64_t recordSize;
};

struct CabinFileHeader ThisCabinFileHeader = {
    .magic = CABIN_FILE_MAGIC,
    .version = CABIN_FILE_VERSION,
    .rootObjectHeaderOffset = 0
};

struct __attribute__((packed)) ObjectHeader
{
    char marker;
    uint32_t flags;
};

struct __attribute__((packed)) SimplePropHeader
{
    char marker;
    uint32_t numEntries;
};

struct __attribute__((packed)) ArrayPropHeader
{
    char marker;
    uint32_t numEntries;
};

union __attribute__((packed)) carbon_archive_dic_flags
{
    struct {
        uint8_t isCompressed          : 1;
        uint8_t compressedWithHuffman : 1;
    } bits;
    uint8_t value;
};

struct __attribute__((packed)) EmbeddedDicHeader
{
    char marker;
    uint32_t numEntries;
    uint8_t flags;
};

struct __attribute__((packed)) EmbeddedString
{
    char marker;
    uint64_t strlen;
};

struct __attribute__((packed)) ObjectArrayHeader
{
    char marker;
    uint8_t numEntries;
};

struct __attribute__((packed)) ColumnGroupHeader
{
    char marker;
    uint32_t numColumns;
};

struct __attribute__((packed)) ColumnHeader
{
    char marker;
    carbon_string_id_t columnName;
    char valueType;
    uint32_t numEntries;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static carbon_off_t skipRecordHeader(carbon_memfile_t *memFile);
static void updateRecordHeader(carbon_memfile_t *memFile,
                               carbon_off_t rootObjectHeaderOffset,
                               carbon_columndoc_t *model,
                               uint64_t recordSize);
static bool serializeObjectMetaModel(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memFile, carbon_columndoc_obj_t *objMetaModel, carbon_off_t rootObjectHeaderOffset);
static carbon_archive_object_flags_t *getFlags(carbon_archive_object_flags_t *flags, carbon_columndoc_obj_t *objMetaModel);
static void updateCabinFileHeader(carbon_memfile_t *memFile, carbon_off_t rootObjectHeaderOffset);
static void skipCabinFileHeader(carbon_memfile_t *memFile);
static bool serializeStringDic(carbon_memfile_t *memFile, carbon_err_t *err, const carbon_doc_bulk_t *context, carbon_archive_compressor_type_e compressor);
static bool dumpPrint(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

bool carbon_archive_from_model(carbon_memblock_t **stream,
                               carbon_err_t *err,
                               carbon_columndoc_t *model,
                               carbon_archive_compressor_type_e compressor)
{
    CARBON_NON_NULL_OR_ERROR(model)
    CARBON_NON_NULL_OR_ERROR(stream)
    CARBON_NON_NULL_OR_ERROR(err)

    carbon_memblock_create(stream, 1024);
    carbon_memfile_t memFile;
    carbon_memfile_open(&memFile, *stream, CARBON_MEMFILE_MODE_READWRITE);

    skipCabinFileHeader(&memFile);
    if(!serializeStringDic(&memFile, err, model->bulk, compressor)) {
        return false;
    }
    carbon_off_t recordHeaderOffset = skipRecordHeader(&memFile);
    updateCabinFileHeader(&memFile, recordHeaderOffset);
    carbon_off_t rootObjectHeaderOffset = CARBON_MEMFILE_TELL(&memFile);
    if(!serializeObjectMetaModel(NULL, err, &memFile, &model->columndoc, rootObjectHeaderOffset)) {
        return false;
    }
    uint64_t recordSize = CARBON_MEMFILE_TELL(&memFile) - (recordHeaderOffset + sizeof(struct RecordHeader));
    updateRecordHeader(&memFile, recordHeaderOffset, model, recordSize);

    carbon_memfile_shrink(&memFile);
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
    carbon_memfile_t memFile;
    carbon_memfile_open(&memFile, stream, CARBON_MEMFILE_MODE_READONLY);
    if (carbon_memfile_size(&memFile) < sizeof(struct CabinFileHeader) + sizeof(struct EmbeddedDicHeader) + sizeof(struct ObjectHeader)) {
        CARBON_ERROR(err, CARBON_ERR_NOCARBONSTREAM);
        return false;
    } else {
        return dumpPrint(file, err, &memFile);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//
//
//
//  S T R I N G   C O M P R E S S O R
//
//
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O M P R E S S O R   B I N D I N G S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  N O   C O M P R E S S I O N
// ---------------------------------------------------------------------------------------------------------------------
static void stringCompressorNoneSetHeaderFlags(union carbon_archive_dic_flags *flags);
static bool stringCompressorNoneAccepts(const union carbon_archive_dic_flags *flags);
static void stringCompressorNoneWriteDictionary(carbon_memfile_t *memFile, const carbon_vec_t ofType (const char *) *strings,
                                                const carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts);
static void stringCompressorNoneDumpDictionary(FILE *file, carbon_memfile_t *memFile);
static void stringCompressorNoneCreate(carbon_archive_compressor_t *strategy);

// ---------------------------------------------------------------------------------------------------------------------
//  H U F F M A N   C O M P R E S S I O N
// ---------------------------------------------------------------------------------------------------------------------
static void stringCompressorHuffmanSetHeaderFlags(union carbon_archive_dic_flags *flags);
static bool stringCompressorHuffmanAccepts(const union carbon_archive_dic_flags *flags);
static void stringCompressorHuffmanWriteDictionary(carbon_memfile_t *memFile, const carbon_vec_t ofType (const char *) *strings,
                                                   const carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts);
static void stringCompressorHuffmanDumpDictionary(FILE *file, carbon_memfile_t *memFile);
static void stringCompressorHuffmanCreate(carbon_archive_compressor_t *strategy);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I M P L E M E N T A T I O N   R E G I S T E R
//
// ---------------------------------------------------------------------------------------------------------------------

struct
{
    carbon_archive_compressor_type_e type;
    void (*create)(carbon_archive_compressor_t *strategy);
    bool (*accepts)(const union carbon_archive_dic_flags *flags);
} CompressorStrategyRegister[] =
    {
        { .type = CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE,
            .create = stringCompressorNoneCreate,
            .accepts = stringCompressorNoneAccepts },

        { .type = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN,
            .create = stringCompressorHuffmanCreate,
            .accepts = stringCompressorHuffmanAccepts }
    };

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static bool compressorStrategyByType(carbon_err_t *err, carbon_archive_compressor_t *strategy, carbon_archive_compressor_type_e type)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(CompressorStrategyRegister); i++) {
        if (CompressorStrategyRegister[i].type == type) {
            CompressorStrategyRegister[i].create(strategy);
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

static bool compressorStrategyByFlags(carbon_archive_compressor_t *strategy, const union carbon_archive_dic_flags *flags)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(CompressorStrategyRegister); i++) {
        if (CompressorStrategyRegister[i].accepts(flags)) {
            CompressorStrategyRegister[i].create(strategy);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
//  N O   C O M P R E S S I O N
// ---------------------------------------------------------------------------------------------------------------------

static void stringCompressorNoneSetHeaderFlags(union carbon_archive_dic_flags *flags)
{
    flags->bits.isCompressed = 0;
}

static bool stringCompressorNoneAccepts(const union carbon_archive_dic_flags *flags)
{
    return !flags->bits.isCompressed;
}

static void stringCompressorNoneWriteDictionary(carbon_memfile_t *memFile, const carbon_vec_t ofType (const char *) *strings,
                                                const carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts)
{
    for (size_t i = 0; i < strings->numElems; i++) {
        carbon_string_id_t *string_id_t = VECTOR_GET(carbon_string_id_ts, i, carbon_string_id_t);
        const char *string = *VECTOR_GET(strings, i, const char *);
        size_t stringLength = strlen(string);

        struct EmbeddedString embeddedString = {
            .marker = markerSymbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol,
            .strlen = stringLength
        };

        carbon_memfile_write(memFile, &embeddedString, sizeof(struct EmbeddedString));
        carbon_memfile_write(memFile, string_id_t, sizeof(carbon_string_id_t));
        carbon_memfile_write(memFile, string, stringLength);
    }
}

static void stringCompressorNoneDumpDictionary(FILE *file, carbon_memfile_t *memFile)
{
    while ((*CARBON_MEMFILE_PEEK(memFile, char)) == markerSymbols[MARKER_TYPE_EMBEDDED_UNCOMP_STR].symbol) {
        unsigned offset = CARBON_MEMFILE_TELL(memFile);
        struct EmbeddedString *embeddedString = CARBON_MEMFILE_READ_TYPE(memFile, struct EmbeddedString);
        carbon_string_id_t *string_id = CARBON_MEMFILE_READ_TYPE(memFile, carbon_string_id_t);
        const char *string = CARBON_MEMFILE_READ(memFile, embeddedString->strlen);
        char *printableString = malloc(embeddedString->strlen + 1);
        memcpy(printableString, string, embeddedString->strlen);
        printableString[embeddedString->strlen] = '\0';

        fprintf(file, "0x%04x ", offset);
        fprintf(file, "   [marker: %c] [stringLength: %"PRIu64"] [string_id: %"PRIu64"] [string: '%s']\n",
                embeddedString->marker,
                embeddedString->strlen, *string_id, printableString);

        free(printableString);
    }
}

static void stringCompressorNoneCreate(carbon_archive_compressor_t *strategy)
{
    strategy->tag = CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE;
    strategy->set_flags = stringCompressorNoneSetHeaderFlags;
    strategy->serialize_dic = stringCompressorNoneWriteDictionary;
    strategy->dump_dic = stringCompressorNoneDumpDictionary;
}

// ---------------------------------------------------------------------------------------------------------------------
//  H U F F M A N   C O M P R E S S I O N
// ---------------------------------------------------------------------------------------------------------------------

static void stringCompressorHuffmanSetHeaderFlags(union carbon_archive_dic_flags *flags)
{
    flags->bits.isCompressed = 1;
    flags->bits.compressedWithHuffman = 1;
}

static bool stringCompressorHuffmanAccepts(const union carbon_archive_dic_flags *flags)
{
    return flags->bits.isCompressed && flags->bits.compressedWithHuffman;
}

static void stringCompressorHuffmanWriteDictionary(carbon_memfile_t *memFile, const carbon_vec_t ofType (const char *) *strings,
                                                   const carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts)
{
    carbon_huffman_t *dic;

    carbon_huffman_create(&dic, strings);
    carbon_huffman_serialize_dic(memFile, dic, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);
    carbon_huffman_encode(memFile, dic, MARKER_SYMBOL_EMBEDDED_STR, carbon_string_id_ts, strings);
    carbon_huffman_drop(dic);
}

static void huffmanDumpDictionary(FILE *file, carbon_memfile_t *memFile)
{
    while ((*CARBON_MEMFILE_PEEK(memFile, char)) == MARKER_SYMBOL_HUFFMAN_DIC_ENTRY) {
        carbon_huffman_entry_info_t entryInfo;
        carbon_off_t offset;
        carbon_memfile_tell(&offset, memFile);
        carbon_huffman_read_dic_entry(&entryInfo, memFile, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);
        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [letter: '%c'] [nbytes_prefix: %d] [code: ",
                MARKER_SYMBOL_HUFFMAN_DIC_ENTRY, entryInfo.letter,
                entryInfo.nbytes_prefix);
        if (entryInfo.nbytes_prefix > 0) {
            for (uint16_t i = 0; i < entryInfo.nbytes_prefix; i++) {
                carbon_bitmap_print_bits_in_char(file, entryInfo.prefix_code[i]);
                fprintf(file, "%s", i + 1 < entryInfo.nbytes_prefix ? ", " : "");
            }
        } else {
            fprintf(file, "0b00000000");
        }

        fprintf(file, "]\n");
    }
}

static void huffmanDumpStringTable(FILE *file, carbon_memfile_t *memFile)
{
    char marker;
    while ((marker = *CARBON_MEMFILE_PEEK(memFile, char)) == MARKER_SYMBOL_EMBEDDED_STR) {
        carbon_huffman_encoded_str_info_t info;
        carbon_off_t offset;
        carbon_memfile_tell(&offset, memFile);
        carbon_huffman_read_string(&info, memFile, MARKER_SYMBOL_EMBEDDED_STR);
        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [string_id: '%"PRIu64"'] [stringLength: '%d'] [nbytes_encoded: %d] [bytes: ",
                marker, info.string_id, info.str_length, info.nbytes_encoded);
        for (size_t i = 0; i < info.nbytes_encoded; i++) {
            char byte = info.encoded_bytes[i];
            carbon_bitmap_print_bits_in_char(file, byte);
            fprintf(file, "%s", i + 1 < info.nbytes_encoded ? "," : "");
        }
        fprintf(file, "]\n");
    }
}

static void stringCompressorHuffmanDumpDictionary(FILE *file, carbon_memfile_t *memFile)
{
    huffmanDumpDictionary(file, memFile);
    huffmanDumpStringTable(file, memFile);
}

static void stringCompressorHuffmanCreate(carbon_archive_compressor_t *strategy)
{
    strategy->tag = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN;
    strategy->set_flags = stringCompressorHuffmanSetHeaderFlags;
    strategy->serialize_dic = stringCompressorHuffmanWriteDictionary;
    strategy->dump_dic = stringCompressorHuffmanDumpDictionary;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

bool dumpPrintObject(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile, unsigned nestingLevel);

static uint32_t flagsToInt32(carbon_archive_object_flags_t *flags)
{
    return *((int32_t *) flags);
}

static const char *arrayValueTypeToString(carbon_err_t *err, carbon_field_type_e type)
{
    switch (type) {
        case carbon_field_type_null:    return "Null Array";
        case carbon_field_type_bool: return "Boolean Array";
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

static carbon_field_type_e valueTypeSymbolToValueType(char symbol)
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

static void writePrimitiveKeyColumn(carbon_memfile_t *memFile, carbon_vec_t ofType(carbon_string_id_t) *keys)
{
    carbon_string_id_t *carbon_string_id_ts = VECTOR_ALL(keys, carbon_string_id_t);
    carbon_memfile_write(memFile, carbon_string_id_ts, keys->numElems * sizeof(carbon_string_id_t));
}

static carbon_off_t skipVarValueOffsetColumn(carbon_memfile_t *memFile, size_t numKeys)
{
    carbon_off_t result = CARBON_MEMFILE_TELL(memFile);
    carbon_memfile_skip(memFile, numKeys * sizeof(carbon_off_t));
    return result;
}

static void writeVarValueOffsetColumn(carbon_memfile_t *file, carbon_off_t where, carbon_off_t after, const carbon_off_t *values, size_t n)
{
    carbon_memfile_seek(file, where);
    carbon_memfile_write(file, values, n * sizeof(carbon_off_t));
    carbon_memfile_seek(file, after);
}

static bool writePrimitiveFixedValueColumn(carbon_memfile_t *memFile,
                                           carbon_err_t *err,
                                           carbon_field_type_e type,
                                           carbon_vec_t ofType(T) *valuesVec)
{
    assert (type != carbon_field_type_object); /* use 'writePrimitiveVarValueColumn' instead */

    switch (type) {
    case carbon_field_type_null:
        break;
    case carbon_field_type_bool:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_bool_t);
        break;
    case carbon_field_type_int8:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_int8_t);
        break;
    case carbon_field_type_int16:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_int16_t);
        break;
    case carbon_field_type_int32:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_int32_t);
        break;
    case carbon_field_type_int64:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_int64_t);
        break;
    case carbon_field_type_uint8:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_uint8_t);
        break;
    case carbon_field_type_uint16:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_uint16_t);
        break;
    case carbon_field_type_uint32:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_uin32_t);
        break;
    case carbon_field_type_uint64:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_uin64_t);
        break;
    case carbon_field_type_float:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_float_t);
        break;
    case carbon_field_type_string:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, carbon_string_id_t);
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static carbon_off_t *writePrimitiveVarValueColumn(carbon_memfile_t *memFile,
                                                  carbon_err_t *err,
                                         carbon_vec_t ofType(carbon_columndoc_obj_t) *valuesVec,
                                         carbon_off_t rootObjectHeaderOffset)
{
    carbon_off_t *result = malloc(valuesVec->numElems * sizeof(carbon_off_t));
    carbon_columndoc_obj_t *mappedObjects = VECTOR_ALL(valuesVec, carbon_columndoc_obj_t);
    for (uint32_t i = 0; i < valuesVec->numElems; i++) {
        carbon_columndoc_obj_t *mappedObject = mappedObjects + i;
        result[i] = CARBON_MEMFILE_TELL(memFile) - rootObjectHeaderOffset;
        if (!serializeObjectMetaModel(NULL, err, memFile, mappedObject, rootObjectHeaderOffset)) {
            return NULL;
        }
    }
    return result;
}

static bool writeArrayLengthsColumn(carbon_err_t *err, carbon_memfile_t *memFile, carbon_field_type_e type, carbon_vec_t ofType(...) *valuesVec)
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
        for (uint32_t i = 0; i < valuesVec->numElems; i++) {
            carbon_vec_t *nestedArrays = VECTOR_GET(valuesVec, i, carbon_vec_t);
            carbon_memfile_write(memFile, &nestedArrays->numElems, sizeof(uint32_t));
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

static bool writeArrayValueColumn(carbon_memfile_t *memFile, carbon_err_t *err, carbon_field_type_e type, carbon_vec_t ofType(...) *valuesVec)
{

    switch (type) {
    case carbon_field_type_null:
        WRITE_PRIMITIVE_VALUES(memFile, valuesVec, uint32_t);
        break;
    case carbon_field_type_bool:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_bool_t);
        break;
    case carbon_field_type_int8:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_int8_t);
        break;
    case carbon_field_type_int16:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_int16_t);
        break;
    case carbon_field_type_int32:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_int32_t);
        break;
    case carbon_field_type_int64:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_int64_t);
        break;
    case carbon_field_type_uint8:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_uin64_t);
        break;
    case carbon_field_type_uint16:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_uint16_t);
        break;
    case carbon_field_type_uint32:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_uin32_t);
        break;
    case carbon_field_type_uint64:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_uin64_t);
        break;
    case carbon_field_type_float:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_float_t);
        break;
    case carbon_field_type_string:
        WRITE_ARRAY_VALUES(memFile, valuesVec, carbon_string_id_t);
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

static bool writeArrayProp(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memFile, carbon_vec_t ofType(carbon_string_id_t) *keys, carbon_field_type_e type,
                             carbon_vec_t ofType(...) *values, carbon_off_t rootObjectHeaderOffset)
{
    assert(keys->numElems == values->numElems);

    if (keys->numElems > 0) {
        struct ArrayPropHeader header = {
            .marker = markerSymbols[valueArrayMarkerMapping[type].marker].symbol,
            .numEntries = keys->numElems
        };
        carbon_off_t propOffset = CARBON_MEMFILE_TELL(memFile);
        carbon_memfile_write(memFile, &header, sizeof(struct ArrayPropHeader));

        writePrimitiveKeyColumn(memFile, keys);
        if(!writeArrayLengthsColumn(err, memFile, type, values)) {
            return false;
        }
        if(!writeArrayValueColumn(memFile, err, type, values)) {
            return false;
        }
        *offset = (propOffset - rootObjectHeaderOffset);
    } else {
        *offset = 0;
    }
    return true;
}

static bool writeArrayProps(carbon_memfile_t *memFile, carbon_err_t *err, carbon_columndoc_obj_t *objMetaModel, carbon_archive_prop_offs_t *offsets,
                            carbon_off_t rootObjectHeaderOffset)
{
     if (!writeArrayProp(&offsets->null_arrays, err, memFile, &objMetaModel->null_array_prop_keys, carbon_field_type_null,
                                                        &objMetaModel->null_array_prop_vals, rootObjectHeaderOffset)) {
         return false;
     }
    if (!writeArrayProp(&offsets->bool_arrays, err, memFile, &objMetaModel->bool_array_prop_keys, carbon_field_type_bool,
                                                           &objMetaModel->bool_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->int8_arrays, err, memFile, &objMetaModel->int8_array_prop_keys, carbon_field_type_int8,
                                                        &objMetaModel->int8_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->int16_arrays, err, memFile, &objMetaModel->int16_array_prop_keys, carbon_field_type_int16,
                                                         &objMetaModel->int16_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->int32_arrays, err, memFile, &objMetaModel->int32_array_prop_keys, carbon_field_type_int32,
                                                         &objMetaModel->int32_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->int64_arrays, err, memFile, &objMetaModel->int64_array_prop_keys, carbon_field_type_int64,
                                                         &objMetaModel->int64_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->uint8_arrays, err, memFile, &objMetaModel->uint8_array_prop_keys, carbon_field_type_uint8,
                                                    &objMetaModel->uint8_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->uint16_arrays, err, memFile, &objMetaModel->uint16_array_prop_keys, carbon_field_type_uint16,
                                                     &objMetaModel->uint16_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->uint32_arrays, err, memFile, &objMetaModel->uint32_array_prop_keys, carbon_field_type_uint32,
                                                     &objMetaModel->uint32_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->uint64_arrays, err, memFile, &objMetaModel->uint64_array_prop_keys, carbon_field_type_uint64,
                                                     &objMetaModel->uin64_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->float_arrays, err, memFile, &objMetaModel->float_array_prop_keys, carbon_field_type_float,
                                                        &objMetaModel->float_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProp(&offsets->string_arrays, err, memFile, &objMetaModel->string_array_prop_keys, carbon_field_type_string,
                                                          &objMetaModel->string_array_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }
    return true;
}

/* Fixed-length property lists; value position can be determined by size of value and position of key in key column.
 * In contrast, variable-length property list require an additional offset column (see 'writeVarProps') */
static bool writeFixedProps(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memFile,
                              carbon_vec_t ofType(carbon_string_id_t) *keys,
                              carbon_field_type_e type,
                              carbon_vec_t ofType(T) *values)
{
    assert(!values || keys->numElems == values->numElems);
    assert(type != carbon_field_type_object); /* use 'writeVarProps' instead */

    if (keys->numElems > 0) {
        struct SimplePropHeader header = {
                .marker = markerSymbols[valueMarkerMapping[type].marker].symbol,
                .numEntries = keys->numElems
        };

        carbon_off_t propOffset = CARBON_MEMFILE_TELL(memFile);
        carbon_memfile_write(memFile, &header, sizeof(struct SimplePropHeader));

        writePrimitiveKeyColumn(memFile, keys);
        if(!writePrimitiveFixedValueColumn(memFile, err, type, values)) {
            return false;
        }
        *offset = propOffset;
    } else {
        *offset = 0;
    }
    return true;
}

/* Variable-length property lists; value position cannot be determined by position of key in key column, since single
 * value has unknown size. Hence, a dedicated offset column is added to these properties allowing to seek directly
 * to a particular property. Due to the move of strings (i.e., variable-length values) to a dedicated string table,
 * the only variable-length value for properties are "JSON objects".
 * In contrast, fixed-length property list doesn't require an additional offset column (see 'writeFixedProps') */
static bool writeVarProps(carbon_off_t *offset,
                          carbon_err_t *err,
                          carbon_memfile_t *memFile,
                          carbon_vec_t ofType(carbon_string_id_t) *keys,
                          carbon_vec_t ofType(carbon_columndoc_obj_t) *objects,
                          carbon_off_t rootObjectHeaderOffset)
{
    assert(!objects || keys->numElems == objects->numElems);

    if (keys->numElems > 0) {
        struct SimplePropHeader header = {
            .marker = MARKER_SYMBOL_PROP_OBJECT,
            .numEntries = keys->numElems
        };

        carbon_off_t propOffset = CARBON_MEMFILE_TELL(memFile);
        carbon_memfile_write(memFile, &header, sizeof(struct SimplePropHeader));

        writePrimitiveKeyColumn(memFile, keys);
        carbon_off_t valueOffset = skipVarValueOffsetColumn(memFile, keys->numElems);
        carbon_off_t *valueOffsets = writePrimitiveVarValueColumn(memFile, err, objects, rootObjectHeaderOffset);
        if (!valueOffsets) {
            return false;
        }

        carbon_off_t last = CARBON_MEMFILE_TELL(memFile);
        writeVarValueOffsetColumn(memFile, valueOffset, last, valueOffsets, keys->numElems);
        free(valueOffsets);
        *offset = propOffset;
    } else {
        *offset = 0;
    }
    return true;
}

static bool writePrimitiveProps(carbon_memfile_t *memFile, carbon_err_t *err, carbon_columndoc_obj_t *objMetaModel, carbon_archive_prop_offs_t *offsets,
                                carbon_off_t rootObjectHeaderOffset)
{
     if (!writeFixedProps(&offsets->nulls, err, memFile, &objMetaModel->null_prop_keys, carbon_field_type_null,
                                                    NULL)) {
         return false;
     }
    if (!writeFixedProps(&offsets->bools, err, memFile, &objMetaModel->bool_prop_keys, carbon_field_type_bool,
                                                       &objMetaModel->bool_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->int8s, err, memFile, &objMetaModel->int8_prop_keys, carbon_field_type_int8,
                                                    &objMetaModel->int8_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->int16s, err, memFile, &objMetaModel->int16_prop_keys, carbon_field_type_int16,
                                                     &objMetaModel->int16_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->int32s, err, memFile, &objMetaModel->int32_prop_keys, carbon_field_type_int32,
                                                     &objMetaModel->int32_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->int64s, err, memFile, &objMetaModel->int64_prop_keys, carbon_field_type_int64,
                                                     &objMetaModel->int64_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->uint8s, err, memFile, &objMetaModel->uint8_prop_keys, carbon_field_type_uint8,
                                                     &objMetaModel->uint8_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->uint16s, err, memFile, &objMetaModel->uint16_prop_keys, carbon_field_type_uint16,
                                                      &objMetaModel->uint16_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->uint32s, err, memFile, &objMetaModel->uin32_prop_keys, carbon_field_type_uint32,
                                                      &objMetaModel->uint32_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->uint64s, err, memFile, &objMetaModel->uint64_prop_keys, carbon_field_type_uint64,
                                                      &objMetaModel->uint64_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->floats, err, memFile, &objMetaModel->float_prop_keys, carbon_field_type_float,
                                                    &objMetaModel->float_prop_vals)) {
        return false;
    }
    if (!writeFixedProps(&offsets->strings, err, memFile, &objMetaModel->string_prop_keys, carbon_field_type_string,
                                                      &objMetaModel->string_prop_vals)) {
        return false;
    }
    if (!writeVarProps(&offsets->objects, err, memFile, &objMetaModel->obj_prop_keys,
                                                    &objMetaModel->obj_prop_vals, rootObjectHeaderOffset)) {
        return false;
    }

    offsets->nulls -= rootObjectHeaderOffset;
    offsets->bools -= rootObjectHeaderOffset;
    offsets->int8s -= rootObjectHeaderOffset;
    offsets->int16s -= rootObjectHeaderOffset;
    offsets->int32s -= rootObjectHeaderOffset;
    offsets->int64s -= rootObjectHeaderOffset;
    offsets->uint8s -= rootObjectHeaderOffset;
    offsets->uint16s -= rootObjectHeaderOffset;
    offsets->uint32s -= rootObjectHeaderOffset;
    offsets->uint64s -= rootObjectHeaderOffset;
    offsets->floats -= rootObjectHeaderOffset;
    offsets->strings -= rootObjectHeaderOffset;
    offsets->objects -= rootObjectHeaderOffset;
    return true;
}

static bool writeColumnEntry(carbon_memfile_t *memFile, carbon_err_t *err, carbon_field_type_e type, carbon_vec_t ofType(<T>) *column, carbon_off_t rootObjectHeaderOffset)
{
    carbon_memfile_write(memFile, &column->numElems, sizeof(uint32_t));
    switch (type) {
        case carbon_field_type_null:carbon_memfile_write(memFile, column->base, column->numElems * sizeof(uint32_t));
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
        case carbon_field_type_string:carbon_memfile_write(memFile, column->base, column->numElems * GET_TYPE_SIZE(type));
            break;
        case carbon_field_type_object: {
            carbon_off_t preObjectNext = 0;
            for (size_t i = 0; i < column->numElems; i++) {
                carbon_columndoc_obj_t *object = VECTOR_GET(column, i, carbon_columndoc_obj_t);
                if (CARBON_BRANCH_LIKELY(preObjectNext != 0)) {
                    carbon_off_t continuePos = CARBON_MEMFILE_TELL(memFile);
                    carbon_off_t relativeContinuePos = continuePos - rootObjectHeaderOffset;
                    carbon_memfile_seek(memFile, preObjectNext);
                    carbon_memfile_write(memFile, &relativeContinuePos, sizeof(carbon_off_t));
                    carbon_memfile_seek(memFile, continuePos);
                }
                 if (!serializeObjectMetaModel(&preObjectNext, err, memFile, object, rootObjectHeaderOffset)) {
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

static carbon_field_type_e getValueTypeOfChar(char c)
{
    size_t len = sizeof(valueArrayMarkerMapping)/ sizeof(valueArrayMarkerMapping[0]);
    for (size_t i = 0; i < len; i++) {
        if (markerSymbols[valueArrayMarkerMapping[i].marker].symbol == c) {
            return valueArrayMarkerMapping[i].valueType;
        }
    }
    return carbon_field_type_null;
}

static bool writeColumn(carbon_memfile_t *memFile, carbon_err_t *err, carbon_columndoc_column_t *column, carbon_off_t rootObjectHeaderOffset)
{
    assert(column->array_positions.numElems == column->values.numElems);

    struct ColumnHeader header = {
        .marker = markerSymbols[MARKER_TYPE_COLUMN].symbol,
        .columnName = column->key_name,
        .valueType = markerSymbols[valueArrayMarkerMapping[column->type].marker].symbol,
        .numEntries = column->values.numElems
    };

    carbon_memfile_write(memFile, &header, sizeof(struct ColumnHeader));

    /* skip offset column to value entry points */
    carbon_off_t valueEntryOffsets = CARBON_MEMFILE_TELL(memFile);
    carbon_memfile_skip(memFile, column->values.numElems * sizeof(carbon_off_t));

    carbon_memfile_write(memFile, column->array_positions.base, column->array_positions.numElems * sizeof(uint32_t));

    for (size_t i = 0; i < column->values.numElems; i++) {
        carbon_vec_t ofType(<T>) *columnData = VECTOR_GET(&column->values, i, carbon_vec_t);
        carbon_off_t columnEntryOffset = CARBON_MEMFILE_TELL(memFile);
        carbon_off_t relativeEntryOffset = columnEntryOffset - rootObjectHeaderOffset;
        carbon_memfile_seek(memFile, valueEntryOffsets + i * sizeof(carbon_off_t));
        carbon_memfile_write(memFile, &relativeEntryOffset, sizeof(carbon_off_t));
        carbon_memfile_seek(memFile, columnEntryOffset);
        if (!writeColumnEntry(memFile, err, column->type, columnData, rootObjectHeaderOffset)) {
            return false;
        }
    }
    return true;
}

static bool writeObjectArrayProps(carbon_memfile_t *memFile, carbon_err_t *err, carbon_vec_t ofType(carbon_columndoc_columngroup_t) *objectKeyColumns,
                                  carbon_archive_prop_offs_t *offsets, carbon_off_t rootObjectHeaderOffset)
{
    if (objectKeyColumns->numElems > 0) {
        struct ObjectArrayHeader header = {
            .marker = markerSymbols[MARKER_TYPE_PROP_OBJECT_ARRAY].symbol,
            .numEntries = objectKeyColumns->numElems
        };

        offsets->object_arrays = CARBON_MEMFILE_TELL(memFile) - rootObjectHeaderOffset;
        carbon_memfile_write(memFile, &header, sizeof(struct ObjectArrayHeader));

        for (size_t i = 0; i < objectKeyColumns->numElems; i++) {
            carbon_columndoc_columngroup_t *columnGroup = VECTOR_GET(objectKeyColumns, i, carbon_columndoc_columngroup_t);
            carbon_memfile_write(memFile, &columnGroup->key, sizeof(carbon_string_id_t));
        }

        // skip offset column to column groups
        carbon_off_t columnOffsets = CARBON_MEMFILE_TELL(memFile);
        carbon_memfile_skip(memFile, objectKeyColumns->numElems * sizeof(carbon_off_t));

        for (size_t i = 0; i < objectKeyColumns->numElems; i++) {
            carbon_columndoc_columngroup_t *columnGroup = VECTOR_GET(objectKeyColumns, i, carbon_columndoc_columngroup_t);

            struct ColumnGroupHeader columnGroupHeader = {
                .marker = markerSymbols[MARKER_TYPE_COLUMN_GROUP].symbol,
                .numColumns = columnGroup->columns.numElems
            };

            carbon_off_t thisColumnOffsetRelative = CARBON_MEMFILE_TELL(memFile) - rootObjectHeaderOffset;
            carbon_memfile_write(memFile, &columnGroupHeader, sizeof(struct ColumnGroupHeader));
            carbon_off_t continueWrite = CARBON_MEMFILE_TELL(memFile);
            carbon_memfile_seek(memFile, columnOffsets + i * sizeof(carbon_off_t));
            carbon_memfile_write(memFile, &thisColumnOffsetRelative, sizeof(carbon_off_t));
            carbon_memfile_seek(memFile, continueWrite);

            carbon_off_t offsetColumnToColumns = continueWrite;
            carbon_memfile_skip(memFile, columnGroup->columns.numElems * sizeof(carbon_off_t));

            for (size_t k = 0; k < columnGroup->columns.numElems; k++) {
                carbon_columndoc_column_t *column = VECTOR_GET(&columnGroup->columns, k, carbon_columndoc_column_t);
                carbon_off_t continueWrite = CARBON_MEMFILE_TELL(memFile);
                carbon_off_t columnOff = continueWrite - rootObjectHeaderOffset;
                carbon_memfile_seek(memFile, offsetColumnToColumns + k * sizeof(carbon_off_t));
                carbon_memfile_write(memFile, &columnOff, sizeof(carbon_off_t));
                carbon_memfile_seek(memFile, continueWrite);
                if(!writeColumn(memFile, err, column, rootObjectHeaderOffset)) {
                    return false;
                }
            }

        }
    } else {
        offsets->object_arrays = 0;
    }

    return true;
}

static carbon_off_t skipRecordHeader(carbon_memfile_t *memFile)
{
    carbon_off_t offset = CARBON_MEMFILE_TELL(memFile);
    carbon_memfile_skip(memFile, sizeof(struct RecordHeader));
    return offset;
}

static void updateRecordHeader(carbon_memfile_t *memFile,
                               carbon_off_t rootObjectHeaderOffset,
                               carbon_columndoc_t *model,
                               uint64_t recordSize)
{
    carbon_archive_record_flags_t flags = {
        .bits.is_sorted = model->read_optimized
    };
    struct RecordHeader header = {
        .marker = MARKER_SYMBOL_RECORD_HEADER,
        .flags = flags.value,
        .recordSize = recordSize
    };
    carbon_off_t offset;
    carbon_memfile_tell(&offset, memFile);
    carbon_memfile_seek(memFile, rootObjectHeaderOffset);
    carbon_memfile_write(memFile, &header, sizeof(struct RecordHeader));
    carbon_memfile_seek(memFile, offset);
}

static void propOffsetsWrite(carbon_memfile_t *memFile, const carbon_archive_object_flags_t *flags, carbon_archive_prop_offs_t *propOffsets)
{
    if (flags->bits.has_null_props) {
        carbon_memfile_write(memFile, &propOffsets->nulls, sizeof(carbon_off_t));
    }
    if (flags->bits.has_bool_props) {
        carbon_memfile_write(memFile, &propOffsets->bools, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int8_props) {
        carbon_memfile_write(memFile, &propOffsets->int8s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int16_props) {
        carbon_memfile_write(memFile, &propOffsets->int16s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int32_props) {
        carbon_memfile_write(memFile, &propOffsets->int32s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int64_props) {
        carbon_memfile_write(memFile, &propOffsets->int64s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint8_props) {
        carbon_memfile_write(memFile, &propOffsets->uint8s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint16_props) {
        carbon_memfile_write(memFile, &propOffsets->uint16s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint32_props) {
        carbon_memfile_write(memFile, &propOffsets->uint32s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint64_props) {
        carbon_memfile_write(memFile, &propOffsets->uint64s, sizeof(carbon_off_t));
    }
    if (flags->bits.has_float_props) {
        carbon_memfile_write(memFile, &propOffsets->floats, sizeof(carbon_off_t));
    }
    if (flags->bits.has_string_props) {
        carbon_memfile_write(memFile, &propOffsets->strings, sizeof(carbon_off_t));
    }
    if (flags->bits.has_object_props) {
        carbon_memfile_write(memFile, &propOffsets->objects, sizeof(carbon_off_t));
    }
    if (flags->bits.has_null_array_props) {
        carbon_memfile_write(memFile, &propOffsets->null_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_bool_array_props) {
        carbon_memfile_write(memFile, &propOffsets->bool_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int8_array_props) {
        carbon_memfile_write(memFile, &propOffsets->int8_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int16_array_props) {
        carbon_memfile_write(memFile, &propOffsets->int16_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int32_array_props) {
        carbon_memfile_write(memFile, &propOffsets->int32_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_int64_array_props) {
        carbon_memfile_write(memFile, &propOffsets->int64_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint8_array_props) {
        carbon_memfile_write(memFile, &propOffsets->uint8_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint16_array_props) {
        carbon_memfile_write(memFile, &propOffsets->uint16_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint32_array_props) {
        carbon_memfile_write(memFile, &propOffsets->uint32_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_uint64_array_props) {
        carbon_memfile_write(memFile, &propOffsets->uint64_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_float_array_props) {
        carbon_memfile_write(memFile, &propOffsets->float_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_string_array_props) {
        carbon_memfile_write(memFile, &propOffsets->string_arrays, sizeof(carbon_off_t));
    }
    if (flags->bits.has_object_array_props) {
        carbon_memfile_write(memFile, &propOffsets->object_arrays, sizeof(carbon_off_t));
    }
}

static void propOffsetsSkipWrite(carbon_memfile_t *memFile, const carbon_archive_object_flags_t *flags)
{
    unsigned numSkipOffsetBytes = 0;
    if (flags->bits.has_null_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_bool_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int8_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int16_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int32_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int64_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint8_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint16_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint32_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint64_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_float_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_string_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_object_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_null_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_bool_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int8_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int16_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int32_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_int64_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint8_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint16_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint32_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_uint64_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_float_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_string_array_props) {
        numSkipOffsetBytes++;
    }
    if (flags->bits.has_object_array_props) {
        numSkipOffsetBytes++;
    }

    carbon_memfile_skip(memFile, numSkipOffsetBytes * sizeof(carbon_off_t));
}

static bool serializeObjectMetaModel(carbon_off_t *offset, carbon_err_t *err, carbon_memfile_t *memFile, carbon_columndoc_obj_t *objMetaModel, carbon_off_t rootObjectHeaderOffset)
{
    carbon_archive_object_flags_t flags;
    carbon_archive_prop_offs_t propOffsets;
    getFlags(&flags, objMetaModel);

    carbon_off_t headerOffset = CARBON_MEMFILE_TELL(memFile);
    carbon_memfile_skip(memFile, sizeof(struct ObjectHeader));

    propOffsetsSkipWrite(memFile, &flags);
    carbon_off_t nextOffset = CARBON_MEMFILE_TELL(memFile);
    carbon_off_t defaultNextNil = 0;
    carbon_memfile_write(memFile, &defaultNextNil, sizeof(carbon_off_t));

    if (!writePrimitiveProps(memFile, err, objMetaModel, &propOffsets, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeArrayProps(memFile, err, objMetaModel, &propOffsets, rootObjectHeaderOffset)) {
        return false;
    }
    if (!writeObjectArrayProps(memFile, err, &objMetaModel->obj_array_props, &propOffsets, rootObjectHeaderOffset)) {
        return false;
    }

    carbon_memfile_write(memFile, &markerSymbols[MARKER_TYPE_OBJECT_END].symbol, 1);


    carbon_off_t objectEndOffset = CARBON_MEMFILE_TELL(memFile);
    carbon_memfile_seek(memFile, headerOffset);

    struct ObjectHeader header = {
        .marker = markerSymbols[MARKER_TYPE_OBJECT_BEGIN].symbol,
        .flags = flagsToInt32(&flags),
    };

    carbon_memfile_write(memFile, &header, sizeof(struct ObjectHeader));

    propOffsetsWrite(memFile, &flags, &propOffsets);

    carbon_memfile_seek(memFile, objectEndOffset);
    CARBON_OPTIONAL_SET(offset, nextOffset);
    return true;
}

static char *embeddedDicFlagsToString(const union carbon_archive_dic_flags *flags)
{
    size_t max = 2048;
    char *string = malloc(max + 1);
    size_t length = 0;

    if (flags->value == 0) {
        strcpy(string, " uncompressed");
        length = strlen(string);
        assert(length <= max);
    } else {
        if (flags->bits.isCompressed) {
            strcpy(string + length, " compressed");
            length = strlen(string);
            assert(length <= max);
        }

        if (flags->bits.compressedWithHuffman) {
            strcpy(string + length, " huffman");
            length = strlen(string);
            assert(length <= max);
        }
    }
    string[length] = '\0';
    return string;
}

static char *recordHeaderFlagsToString(const carbon_archive_record_flags_t *flags)
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

static bool validateEmbeddedDicHeaderFlags(carbon_err_t *err, const union carbon_archive_dic_flags *flags)
{
    if (flags->value != 0) {
        if (!flags->bits.isCompressed) {
            CARBON_ERROR(err, CARBON_ERR_CORRUPTED)
            return false;
        }
    }
    return true;
}

static bool serializeStringDic(carbon_memfile_t *memFile, carbon_err_t *err, const carbon_doc_bulk_t *context, carbon_archive_compressor_type_e compressor)
{
    union carbon_archive_dic_flags flags;
    carbon_archive_compressor_t strategy;
    struct EmbeddedDicHeader header;

    carbon_vec_t ofType (const char *) *strings;
    carbon_vec_t ofType(carbon_string_id_t) *carbon_string_id_ts;

    carbon_doc_bulk_get_dic_conetnts(&strings, &carbon_string_id_ts, context);

    assert(strings->numElems == carbon_string_id_ts->numElems);

    flags.value = 0;
    if(!compressorStrategyByType(err, &strategy, compressor)) {
        return false;
    }
    strategy.set_flags(&flags);
    if (!validateEmbeddedDicHeaderFlags(err, &flags)) {
        return false;
    }

    header = (struct EmbeddedDicHeader) {
        .marker = markerSymbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol,
        .flags = flags.value,
        .numEntries = strings->numElems
    };

    carbon_memfile_write(memFile, &header, sizeof(struct EmbeddedDicHeader));

    strategy.serialize_dic(memFile, strings, carbon_string_id_ts);

    VectorDrop(strings);
    VectorDrop(carbon_string_id_ts);
    free(strings);
    free(carbon_string_id_ts);
    return true;
}

static void skipCabinFileHeader(carbon_memfile_t *memFile)
{
    carbon_memfile_skip(memFile, sizeof(struct CabinFileHeader));
}

static void updateCabinFileHeader(carbon_memfile_t *memFile, carbon_off_t recordHeaderOffset)
{
    carbon_off_t currentPos;
    carbon_memfile_tell(&currentPos, memFile);
    carbon_memfile_seek(memFile, 0);
    ThisCabinFileHeader.rootObjectHeaderOffset = recordHeaderOffset;
    carbon_memfile_write(memFile, &ThisCabinFileHeader, sizeof(struct CabinFileHeader));
    carbon_memfile_seek(memFile, currentPos);
}

static bool dumpColumn(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile, unsigned nestingLevel)
{
    carbon_off_t offset;
    carbon_memfile_tell(&offset, memFile);
    struct ColumnHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct ColumnHeader);
    if (header->marker != MARKER_SYMBOL_COLUMN) {
        char buffer[256];
        sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_COLUMN, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }
    fprintf(file, "0x%04x ", (unsigned) offset);
    INTENT_LINE(nestingLevel);
    const char *type_name = arrayValueTypeToString(err, valueTypeSymbolToValueType(header->valueType));
    if (!type_name) {
        return false;
    }

    fprintf(file, "[marker: %c (Column)] [columnName: '%"PRIu64"'] [valueType: %c (%s)] [nentries: %d] [", header->marker,
            header->columnName, header->valueType, type_name, header->numEntries);

    for (size_t i = 0; i < header->numEntries; i++) {
        carbon_off_t entryOff = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
        fprintf(file, "offset: 0x%04x%s", (unsigned) entryOff, i + 1 < header->numEntries ? ", " : "");
    }

    uint32_t *positions = (uint32_t *) CARBON_MEMFILE_READ(memFile, header->numEntries * sizeof(uint32_t));
    fprintf(file, "] [positions: [");
    for (size_t i = 0; i < header->numEntries; i++) {
        fprintf(file, "%d%s", positions[i], i + 1 < header->numEntries ? ", " : "");
    }
    fprintf(file, "]]\n");

    carbon_field_type_e dataType = valueTypeSymbolToValueType(header->valueType);

    //fprintf(file, "[");
    for (size_t i = 0; i < header->numEntries; i++) {
        switch (dataType) {
            case carbon_field_type_null: {
                PRINT_VALUE_ARRAY(uint32_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_bool: {
                PRINT_VALUE_ARRAY(carbon_bool_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_int8: {
                PRINT_VALUE_ARRAY(carbon_int8_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_int16: {
                PRINT_VALUE_ARRAY(carbon_int16_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_int32: {
                PRINT_VALUE_ARRAY(carbon_int32_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_int64: {
                PRINT_VALUE_ARRAY(carbon_int64_t, memFile, header, "%"PRIi64);
            }
                break;
            case carbon_field_type_uint8: {
                PRINT_VALUE_ARRAY(carbon_uint8_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_uint16: {
                PRINT_VALUE_ARRAY(carbon_uint16_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_uint32: {
                PRINT_VALUE_ARRAY(carbon_uin32_t, memFile, header, "%d");
            }
                break;
            case carbon_field_type_uint64: {
                PRINT_VALUE_ARRAY(carbon_uin64_t, memFile, header, "%"PRIu64);
            }
                break;
            case carbon_field_type_float: {
                PRINT_VALUE_ARRAY(carbon_float_t, memFile, header, "%f");
            }
                break;
            case carbon_field_type_string: {
                PRINT_VALUE_ARRAY(carbon_string_id_t, memFile, header, "%"PRIu64"");
            }
                break;
            case carbon_field_type_object: {
                uint32_t numElements = *CARBON_MEMFILE_READ_TYPE(memFile, uint32_t);
                INTENT_LINE(nestingLevel);
                fprintf(file, "   [numElements: %d] [values: [\n", numElements);
                for (size_t i = 0; i < numElements; i++) {
                    if (!dumpPrintObject(file, err, memFile, nestingLevel + 2)) {
                        return false;
                    }
                }
                INTENT_LINE(nestingLevel);
                fprintf(file, "   ]\n");
            } break;
            default:
                CARBON_ERROR(err, CARBON_ERR_NOTYPE)
                return false;
        }
    }
    return true;
}

static bool dumpObjectArray(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile, unsigned nestingLevel)
{
    unsigned offset = (unsigned) CARBON_MEMFILE_TELL(memFile);
    struct ObjectArrayHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct ObjectArrayHeader);
    if (header->marker != MARKER_SYMBOL_PROP_OBJECT_ARRAY) {
        char buffer[256];
        sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_PROP_OBJECT_ARRAY, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nestingLevel);
    fprintf(file, "[marker: %c (Object Array)] [nentries: %d] [", header->marker, header->numEntries);

    for (size_t i = 0; i < header->numEntries; i++) {
        carbon_string_id_t string_id = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_string_id_t);
        fprintf(file, "key: %"PRIu64"%s", string_id, i + 1 < header->numEntries ? ", " : "");
    }
    fprintf(file, "] [");
    for (size_t i = 0; i < header->numEntries; i++) {
        carbon_off_t columnGroupOffset = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
        fprintf(file, "offset: 0x%04x%s", (unsigned) columnGroupOffset, i + 1 < header->numEntries ? ", " : "");
    }

    fprintf(file, "]\n");
    nestingLevel++;

    for (size_t i = 0; i < header->numEntries; i++) {
        offset = CARBON_MEMFILE_TELL(memFile);
        struct ColumnGroupHeader *columnGroupHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct ColumnGroupHeader);
        if (columnGroupHeader->marker != MARKER_SYMBOL_COLUMN_GROUP) {
            char buffer[256];
            sprintf(buffer, "expected marker [%c] but found [%c]", MARKER_SYMBOL_COLUMN_GROUP, columnGroupHeader->marker);
            CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
            return false;
        }
        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nestingLevel);
        fprintf(file, "[marker: %c (Column Group)] [numColumns: %d] [", columnGroupHeader->marker, columnGroupHeader->numColumns);
        for (size_t k = 0; k < columnGroupHeader->numColumns; k++) {
            carbon_off_t columnOff = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
            fprintf(file, "offset: 0x%04x%s", (unsigned) columnOff, k + 1 < columnGroupHeader->numColumns ? ", " : "");
        }
        fprintf(file, "]\n");

        for (size_t k = 0; k < columnGroupHeader->numColumns; k++) {
            if(!dumpColumn(file, err, memFile, nestingLevel + 1)) {
                return false;
            }
        }

        fprintf(file, "0x%04x ", offset);
        INTENT_LINE(nestingLevel);
        fprintf(file, "]\n");
    }
    return true;
}

static void propertyOffsetsPrint(FILE *file, const carbon_archive_object_flags_t *flags, const carbon_archive_prop_offs_t *propOffsets)
{
    if (flags->bits.has_null_props) {
        fprintf(file, " nulls: 0x%04x", (unsigned) propOffsets->nulls);
    }
    if (flags->bits.has_bool_props) {
        fprintf(file, " bools: 0x%04x", (unsigned) propOffsets->bools);
    }
    if (flags->bits.has_int8_props) {
        fprintf(file, " int8s: 0x%04x", (unsigned) propOffsets->int8s);
    }
    if (flags->bits.has_int16_props) {
        fprintf(file, " int16s: 0x%04x", (unsigned) propOffsets->int16s);
    }
    if (flags->bits.has_int32_props) {
        fprintf(file, " int32s: 0x%04x", (unsigned) propOffsets->int32s);
    }
    if (flags->bits.has_int64_props) {
        fprintf(file, " int64s: 0x%04x", (unsigned) propOffsets->int64s);
    }
    if (flags->bits.has_uint8_props) {
        fprintf(file, " uint8s: 0x%04x", (unsigned) propOffsets->uint8s);
    }
    if (flags->bits.has_uint16_props) {
        fprintf(file, " uint16s: 0x%04x", (unsigned) propOffsets->uint16s);
    }
    if (flags->bits.has_uint32_props) {
        fprintf(file, " uint32s: 0x%04x", (unsigned) propOffsets->uint32s);
    }
    if (flags->bits.has_uint64_props) {
        fprintf(file, " uint64s: 0x%04x", (unsigned) propOffsets->uint64s);
    }
    if (flags->bits.has_float_props) {
        fprintf(file, " floats: 0x%04x", (unsigned) propOffsets->floats);
    }
    if (flags->bits.has_string_props) {
        fprintf(file, " texts: 0x%04x", (unsigned) propOffsets->strings);
    }
    if (flags->bits.has_object_props) {
        fprintf(file, " objects: 0x%04x", (unsigned) propOffsets->objects);
    }
    if (flags->bits.has_null_array_props) {
        fprintf(file, " nullArrays: 0x%04x", (unsigned) propOffsets->null_arrays);
    }
    if (flags->bits.has_bool_array_props) {
        fprintf(file, " boolArrays: 0x%04x", (unsigned) propOffsets->bool_arrays);
    }
    if (flags->bits.has_int8_array_props) {
        fprintf(file, " int8Arrays: 0x%04x", (unsigned) propOffsets->int8_arrays);
    }
    if (flags->bits.has_int16_array_props) {
        fprintf(file, " int16Arrays: 0x%04x", (unsigned) propOffsets->int16_arrays);
    }
    if (flags->bits.has_int32_array_props) {
        fprintf(file, " int32Arrays: 0x%04x", (unsigned) propOffsets->int32_arrays);
    }
    if (flags->bits.has_int64_array_props) {
        fprintf(file, " int16Arrays: 0x%04x", (unsigned) propOffsets->int64_arrays);
    }
    if (flags->bits.has_uint8_array_props) {
        fprintf(file, " uint8Arrays: 0x%04x", (unsigned) propOffsets->uint8_arrays);
    }
    if (flags->bits.has_uint16_array_props) {
        fprintf(file, " uint16Arrays: 0x%04x", (unsigned) propOffsets->uint16_arrays);
    }
    if (flags->bits.has_uint32_array_props) {
        fprintf(file, " uint32Arrays: 0x%04x", (unsigned) propOffsets->uint32_arrays);
    }
    if (flags->bits.has_uint64_array_props) {
        fprintf(file, " uint64Arrays: 0x%04x", (unsigned) propOffsets->uint64_arrays);
    }
    if (flags->bits.has_float_array_props) {
        fprintf(file, " floatArrays: 0x%04x", (unsigned) propOffsets->float_arrays);
    }
    if (flags->bits.has_string_array_props) {
        fprintf(file, " textArrays: 0x%04x", (unsigned) propOffsets->string_arrays);
    }
    if (flags->bits.has_object_array_props) {
        fprintf(file, " objectArrays: 0x%04x", (unsigned) propOffsets->object_arrays);
    }
}

static void readPropertyOffsets(carbon_archive_prop_offs_t *propOffsets, carbon_memfile_t *memFile, const carbon_archive_object_flags_t *flags)
{
    if (flags->bits.has_null_props) {
        propOffsets->nulls = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_bool_props) {
        propOffsets->bools = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int8_props) {
        propOffsets->int8s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int16_props) {
        propOffsets->int16s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int32_props) {
        propOffsets->int32s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int64_props) {
        propOffsets->int64s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint8_props) {
        propOffsets->uint8s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint16_props) {
        propOffsets->uint16s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint32_props) {
        propOffsets->uint32s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint64_props) {
        propOffsets->uint64s = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_float_props) {
        propOffsets->floats = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_string_props) {
        propOffsets->strings = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_object_props) {
        propOffsets->objects = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_null_array_props) {
        propOffsets->null_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_bool_array_props) {
        propOffsets->bool_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int8_array_props) {
        propOffsets->int8_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int16_array_props) {
        propOffsets->int16_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int32_array_props) {
        propOffsets->int32_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_int64_array_props) {
        propOffsets->int64_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint8_array_props) {
        propOffsets->uint8_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint16_array_props) {
        propOffsets->uint16_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint32_array_props) {
        propOffsets->uint32_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_uint64_array_props) {
        propOffsets->uint64_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_float_array_props) {
        propOffsets->float_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_string_array_props) {
        propOffsets->string_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
    if (flags->bits.has_object_array_props) {
        propOffsets->object_arrays = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);
    }
}

typedef struct
{
    struct SimplePropHeader *header;
    const carbon_string_id_t *keys;
    const void *values;
} EmbeddedFixedProp;

typedef struct
{
    struct SimplePropHeader *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *groupOffs;
} EmbeddedTableProp;

typedef struct
{
    struct SimplePropHeader *header;
    const carbon_string_id_t *keys;
    const carbon_off_t *offsets;
    const void *values;
} EmbeddedVarProp;

typedef struct
{
    struct SimplePropHeader *header;
    const carbon_string_id_t *keys;
    const uint32_t *lengths;
    carbon_off_t valuesBegin;
} EmbeddedArrayProp;

typedef struct
{
    struct SimplePropHeader *header;
    const carbon_string_id_t *keys;
} EmbeddedNullProp;

static void embeddedFixedPropsRead(EmbeddedFixedProp *prop, carbon_memfile_t *memFile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_string_id_t));
    prop->values = carbon_memfile_peek(memFile, 1);
}

static void embeddedVarPropsRead(EmbeddedVarProp *prop, carbon_memfile_t *memFile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_string_id_t));
    prop->offsets = (carbon_off_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_off_t));
    prop->values = carbon_memfile_peek(memFile, 1);
}

static void embeddedNullPropsRead(EmbeddedNullProp *prop, carbon_memfile_t *memFile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_string_id_t));
}

static void embeddedArrayPropsRead(EmbeddedArrayProp *prop, carbon_memfile_t *memFile) {
    prop->header = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_string_id_t));
    prop->lengths = (uint32_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(uint32_t));
    prop->valuesBegin = CARBON_MEMFILE_TELL(memFile);
}

static void embeddedTablePropsRead(EmbeddedTableProp *prop, carbon_memfile_t *memFile) {
    prop->header->marker = *CARBON_MEMFILE_READ_TYPE(memFile, char);
    prop->header->numEntries = *CARBON_MEMFILE_READ_TYPE(memFile, uint8_t);
    prop->keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_string_id_t));
    prop->groupOffs = (carbon_off_t *) CARBON_MEMFILE_READ(memFile, prop->header->numEntries * sizeof(carbon_off_t));
}

bool dumpPrintObject(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile, unsigned nestingLevel)
{
    unsigned offset = (unsigned) CARBON_MEMFILE_TELL(memFile);
    struct ObjectHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct ObjectHeader);

    carbon_archive_prop_offs_t propOffsets;
    carbon_archive_object_flags_t flags = {
        .value = header->flags
    };

    readPropertyOffsets(&propOffsets, memFile, &flags);
    carbon_off_t nextObjectOrNil = *CARBON_MEMFILE_READ_TYPE(memFile, carbon_off_t);

    if (header->marker != MARKER_SYMBOL_OBJECT_BEGIN) {
        char buffer[256];
        sprintf(buffer, "Parsing error: expected object marker [{] but found [%c]\"", header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nestingLevel);
    nestingLevel++;
    fprintf(file, "[marker: %c (BeginObject)] [flags: %u] [propertyOffsets: [", header->marker, header->flags);
    propertyOffsetsPrint(file, &flags, &propOffsets);
    fprintf(file, " ] [next: 0x%04x] \n", (unsigned) nextObjectOrNil);

    bool continueRead = true;
    while (continueRead) {
        offset = CARBON_MEMFILE_TELL(memFile);
        char entryMarker = *CARBON_MEMFILE_PEEK(memFile, char);

        switch (entryMarker) {
            case MARKER_SYMBOL_PROP_NULL: {
                struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nestingLevel)
                fprintf(file, "[marker: %c (null)] [nentries: %d] [", entryMarker, propHeader->numEntries);

                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");
                }
                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_BOOLEAN: {
                struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);
                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));
                carbon_bool_t *values = (carbon_bool_t *) CARBON_MEMFILE_READ(memFile,
                                                                propHeader->numEntries * sizeof(carbon_bool_t));
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nestingLevel)
                fprintf(file, "[marker: %c (boolean)] [nentries: %d] [", entryMarker, propHeader->numEntries);
                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");
                }
                fprintf(file, "] [");
                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "%s%s", values[i] ? "true" : "false", i + 1 < propHeader->numEntries ? ", " : "");
                }
                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_INT8:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_int8_t, "Int8", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT16:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_int16_t, "Int16", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT32:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_int32_t, "Int32", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT64:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_int64_t, "Int64", "%"PRIi64);
                break;
            case MARKER_SYMBOL_PROP_UINT8:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_uint8_t, "UInt8", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT16:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_uint16_t, "UInt16", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT32:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_uin32_t, "UInt32", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT64:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_uin64_t, "UInt64", "%"PRIu64);
                break;
            case MARKER_SYMBOL_PROP_REAL:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_float_t , "Float", "%f");
                break;
            case MARKER_SYMBOL_PROP_TEXT:
                PRINT_SIMPLE_PROPS(file, memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, carbon_string_id_t, "Text", "%"PRIu64"");
                break;
            case MARKER_SYMBOL_PROP_OBJECT: {
                EmbeddedVarProp prop;
                embeddedVarPropsRead(&prop, memFile);
                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nestingLevel)
                fprintf(file, "[marker: %c (Object)] [nentries: %d] [", entryMarker, prop.header->numEntries);
                for (uint32_t i = 0; i < prop.header->numEntries; i++) {
                    fprintf(file, "key: %"PRIu64"%s", prop.keys[i], i + 1 < prop.header->numEntries ? ", " : "");
                }
                fprintf(file, "] [");
                for (uint32_t i = 0; i < prop.header->numEntries; i++) {
                    fprintf(file, "offsets: 0x%04x%s", (unsigned) prop.offsets[i], i + 1 < prop.header->numEntries ? ", " : "");
                }
                fprintf(file, "] [\n");

                char nextEntryMarker;
                do {
                    if (!dumpPrintObject(file, err, memFile, nestingLevel + 1)) {
                        return false;
                    }
                    nextEntryMarker = *CARBON_MEMFILE_PEEK(memFile, char);
                } while (nextEntryMarker == MARKER_SYMBOL_OBJECT_BEGIN);

            } break;
            case MARKER_SYMBOL_PROP_NULL_ARRAY: {
                struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);

                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));
                uint32_t *nullArrayLengths;

                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nestingLevel)
                fprintf(file, "[marker: %c (Null Array)] [nentries: %d] [", entryMarker, propHeader->numEntries);

                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");
                }
                fprintf(file, "] [");

                nullArrayLengths = (uint32_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(uint32_t));

                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "nentries: %d%s", nullArrayLengths[i], i + 1 < propHeader->numEntries ? ", " : "");
                }

                fprintf(file, "]\n");
            } break;
            case MARKER_SYMBOL_PROP_BOOLEAN_ARRAY: {
                struct SimplePropHeader *propHeader = CARBON_MEMFILE_READ_TYPE(memFile, struct SimplePropHeader);

                carbon_string_id_t *keys = (carbon_string_id_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(carbon_string_id_t));
                uint32_t *arrayLengths;

                fprintf(file, "0x%04x ", offset);
                INTENT_LINE(nestingLevel)
                fprintf(file, "[marker: %c (Boolean Array)] [nentries: %d] [", entryMarker, propHeader->numEntries);

                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "%"PRIu64"%s", keys[i], i + 1 < propHeader->numEntries ? ", " : "");
                }
                fprintf(file, "] [");

                arrayLengths = (uint32_t *) CARBON_MEMFILE_READ(memFile, propHeader->numEntries * sizeof(uint32_t));

                for (uint32_t i = 0; i < propHeader->numEntries; i++) {
                    fprintf(file, "arrayLength: %d%s", arrayLengths[i], i + 1 < propHeader->numEntries ? ", " : "");
                }

                fprintf(file, "] [");

                for (uint32_t arrayIdx = 0; arrayIdx < propHeader->numEntries; arrayIdx++) {
                    carbon_bool_t *values = (carbon_bool_t *) CARBON_MEMFILE_READ(memFile,
                                                                   arrayLengths[arrayIdx] * sizeof(carbon_bool_t));
                    fprintf(file, "[");
                    for (uint32_t i = 0; i < arrayLengths[arrayIdx]; i++) {
                        fprintf(file, "value: %s%s", values[i] ? "true" : "false", i + 1 < arrayLengths[arrayIdx] ? ", " : "");
                    }
                    fprintf(file, "]%s", arrayIdx + 1 < propHeader->numEntries ? ", " : "");
                }

                fprintf(file, "]\n");
            } break;
                break;
            case MARKER_SYMBOL_PROP_INT8_ARRAY: {
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_int8_t, "Int8 Array", "%d");
             } break;
            case MARKER_SYMBOL_PROP_INT16_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_int16_t, "Int16 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT32_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_int32_t, "Int32 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_INT64_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_int64_t, "Int64 Array", "%"PRIi64);
                break;
            case MARKER_SYMBOL_PROP_UINT8_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_uint8_t, "UInt8 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT16_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_uint16_t, "UInt16 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT32_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_uin32_t, "UInt32 Array", "%d");
                break;
            case MARKER_SYMBOL_PROP_UINT64_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_uin64_t, "UInt64 Array", "%"PRIu64);
                break;
            case MARKER_SYMBOL_PROP_REAL_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_float_t, "Float Array", "%f");
                break;
            case MARKER_SYMBOL_PROP_TEXT_ARRAY:
                PRINT_ARRAY_PROPS(memFile, CARBON_MEMFILE_TELL(memFile), nestingLevel, entryMarker, carbon_string_id_t, "Text Array", "%"PRIu64"");
                break;
            case MARKER_SYMBOL_PROP_OBJECT_ARRAY:
                if(!dumpObjectArray(file, err, memFile, nestingLevel)) {
                    return false;
                }
                break;
            case MARKER_SYMBOL_OBJECT_END:
                continueRead = false;
                break;
            default: {
                char buffer[256];
                sprintf(buffer, "Parsing error: unexpected marker [%c] was detected in file %p", entryMarker, memFile);
                CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
                return false;
            }
        }
    }

    offset = CARBON_MEMFILE_TELL(memFile);
    char endMarker = *CARBON_MEMFILE_READ_TYPE(memFile, char);
    assert (endMarker == MARKER_SYMBOL_OBJECT_END);
    nestingLevel--;
    fprintf(file, "0x%04x ", offset);
    INTENT_LINE(nestingLevel);
    fprintf(file, "[marker: %c (EndObject)]\n", endMarker);
    return true;
}

static bool isValidCabinFile(const struct CabinFileHeader *header)
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
        if (header->rootObjectHeaderOffset == 0) {
            return false;
        }
        return true;
    }
}

static void dumpPrintRecordHeader(FILE *file, carbon_memfile_t *memFile)
{
    unsigned offset = CARBON_MEMFILE_TELL(memFile);
    struct RecordHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct RecordHeader);
    carbon_archive_record_flags_t flags;
    flags.value = header->flags;
    char *flagsString = recordHeaderFlagsToString(&flags);
    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[marker: %c] [flags: %s] [recordSize: 0x%04x]\n",
            header->marker, flagsString, (unsigned) header->recordSize);
    free(flagsString);
}

static bool dumpPrintCabinHeader(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile)
{
    unsigned offset = CARBON_MEMFILE_TELL(memFile);
    assert(carbon_memfile_size(memFile) > sizeof(struct CabinFileHeader));
    struct CabinFileHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct CabinFileHeader);
    if (!isValidCabinFile(header)) {
        CARBON_ERROR(err, CARBON_ERR_NOARCHIVEFILE)
        return false;
    }

    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[magic: " CABIN_FILE_MAGIC "] [version: %d] [recordOffset: 0x%04x]\n",
            header->version, (unsigned) header->rootObjectHeaderOffset);
    return true;
}

static bool dumpEmbeddedDic(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile)
{
    carbon_archive_compressor_t strategy;
    union carbon_archive_dic_flags flags;

    unsigned offset = CARBON_MEMFILE_TELL(memFile);
    struct EmbeddedDicHeader *header = CARBON_MEMFILE_READ_TYPE(memFile, struct EmbeddedDicHeader);
    if (header->marker != markerSymbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
        char buffer[256];
        sprintf(buffer, "expected [%c] marker, but found [%c]", markerSymbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol, header->marker);
        CARBON_ERROR_WDETAILS(err, CARBON_ERR_CORRUPTED, buffer);
        return false;
    }
    flags.value = header->flags;

    char *flagsStr = embeddedDicFlagsToString(&flags);
    fprintf(file, "0x%04x ", offset);
    fprintf(file, "[marker: %c] [nentries: %d] [flags:%s]\n", header->marker,
            header->numEntries, flagsStr);
    free(flagsStr);

    if (compressorStrategyByFlags(&strategy, &flags) != true) {
        CARBON_ERROR(err, CARBON_ERR_NOCOMPRESSOR);
        return false;
    }
    strategy.dump_dic(file, memFile);
    return true;
}

static bool dumpPrint(FILE *file, carbon_err_t *err, carbon_memfile_t *memFile)
{
    if (!dumpPrintCabinHeader(file, err, memFile)) {
        return false;
    }
    if (!dumpEmbeddedDic(file, err, memFile)) {
        return false;
    }
    dumpPrintRecordHeader(file, memFile);
    if (!dumpPrintObject(file, err, memFile, 0)) {
        return false;
    }
    return true;
}

static carbon_archive_object_flags_t *getFlags(carbon_archive_object_flags_t *flags, carbon_columndoc_obj_t *objMetaModel) {
    CARBON_ZERO_MEMORY(flags, sizeof(carbon_archive_object_flags_t));
    flags->bits.has_null_props         = (objMetaModel->null_prop_keys.numElems > 0);
    flags->bits.has_bool_props      = (objMetaModel->bool_prop_keys.numElems > 0);
    flags->bits.has_int8_props         = (objMetaModel->int8_prop_keys.numElems > 0);
    flags->bits.has_int16_props        = (objMetaModel->int16_prop_keys.numElems > 0);
    flags->bits.has_int32_props        = (objMetaModel->int32_prop_keys.numElems > 0);
    flags->bits.has_int64_props        = (objMetaModel->int64_prop_keys.numElems > 0);
    flags->bits.has_uint8_props        = (objMetaModel->uint8_prop_keys.numElems > 0);
    flags->bits.has_uint16_props       = (objMetaModel->uint16_prop_keys.numElems > 0);
    flags->bits.has_uint32_props       = (objMetaModel->uin32_prop_keys.numElems > 0);
    flags->bits.has_uint64_props       = (objMetaModel->uint64_prop_keys.numElems > 0);
    flags->bits.has_float_props         = (objMetaModel->float_prop_keys.numElems > 0);
    flags->bits.has_string_props       = (objMetaModel->string_prop_keys.numElems > 0);
    flags->bits.has_object_props       = (objMetaModel->obj_prop_keys.numElems > 0);
    flags->bits.has_null_array_props    = (objMetaModel->null_array_prop_keys.numElems > 0);
    flags->bits.has_bool_array_props = (objMetaModel->bool_array_prop_keys.numElems > 0);
    flags->bits.has_int8_array_props    = (objMetaModel->int8_array_prop_keys.numElems > 0);
    flags->bits.has_int16_array_props   = (objMetaModel->int16_array_prop_keys.numElems > 0);
    flags->bits.has_int32_array_props   = (objMetaModel->int32_array_prop_keys.numElems > 0);
    flags->bits.has_int64_array_props   = (objMetaModel->int64_array_prop_keys.numElems > 0);
    flags->bits.has_uint8_array_props   = (objMetaModel->uint8_array_prop_keys.numElems > 0);
    flags->bits.has_uint16_array_props  = (objMetaModel->uint16_array_prop_keys.numElems > 0);
    flags->bits.has_uint32_array_props  = (objMetaModel->uint32_array_prop_keys.numElems > 0);
    flags->bits.has_uint64_array_props  = (objMetaModel->uint64_array_prop_keys.numElems > 0);
    flags->bits.has_float_array_props    = (objMetaModel->float_array_prop_keys.numElems > 0);
    flags->bits.has_string_array_props  = (objMetaModel->string_array_prop_keys.numElems > 0);
    flags->bits.has_object_array_props  = (objMetaModel->obj_array_props.numElems > 0);
    assert(flags->value != 0);
    return flags;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//
//
//  C A B I N   F I L E
//
//
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static bool initDecompressor(carbon_archive_record_table_t *file);
static bool readRecord(carbon_archive_record_table_t *file, carbon_off_t recordHeaderOffset);
static void resetStringDicDiskFileCursor(carbon_archive_record_table_t *file);
static void convertObjectToModel(carbon_doc_t *model, carbon_archive_object_t *obj);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

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
        struct CabinFileHeader header;
        size_t nread = fread(&header, sizeof(struct CabinFileHeader), 1, out->record_table.diskFile);
        if (nread != 1) {
            fclose(out->record_table.diskFile);
            CARBON_PRINT_ERROR(CARBON_ERR_IO);
            return false;
        } else {
            if (!isValidCabinFile(&header)) {
                CARBON_PRINT_ERROR(CARBON_ERR_FORMATVERERR);
                return false;
            } else {
                if ((status = initDecompressor(&out->record_table)) != true) {
                    return status;
                }
                if ((status = readRecord(&out->record_table, header.rootObjectHeaderOffset)) != true) {
                    return status;
                }

                fseek(out->record_table.diskFile, sizeof(struct CabinFileHeader), SEEK_SET);
                carbon_off_t stringDicStart = ftell(out->record_table.diskFile);
                fseek(out->record_table.diskFile, 0, SEEK_END);
                carbon_off_t fileEnd = ftell(out->record_table.diskFile);
                fseek(out->record_table.diskFile, stringDicStart, SEEK_SET);
                carbon_error_init(&out->err);
                out->info.string_table_size = header.rootObjectHeaderOffset - stringDicStart;
                out->info.record_table_size = fileEnd - header.rootObjectHeaderOffset;
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
    struct ObjectHeader *header = CARBON_MEMFILE_READ_TYPE(&obj->file, struct ObjectHeader);
    carbon_archive_object_flags_t flags = {
        .value = header->flags
    };
    carbon_error_init(&obj->err);
    obj->context = context;
    obj->flags.value = header->flags;
    readPropertyOffsets(&obj->props, &obj->file, &flags);
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

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static bool initDecompressor(carbon_archive_record_table_t *file)
{
    assert(file->diskFile);

    struct EmbeddedDicHeader header;
    union carbon_archive_dic_flags flags;

    fread(&header, sizeof(struct EmbeddedDicHeader), 1, file->diskFile);
    if (header.marker != markerSymbols[MARKER_TYPE_EMBEDDED_STR_DIC].symbol) {
        CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
        return false;
    }

    flags.value = header.flags;

    if (compressorStrategyByFlags(&file->strategy, &flags) != true) {
        return false;
    }

    return true;
}

static bool readRecord(carbon_archive_record_table_t *file, carbon_off_t recordHeaderOffset)
{
    carbon_err_t err;
    fseek(file->diskFile, recordHeaderOffset, SEEK_SET);
    struct RecordHeader header;
    if (fread(&header, sizeof(struct RecordHeader), 1, file->diskFile) != 1) {
        CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
        return false;
    } else {
        file->flags.value = header.flags;
        bool status = carbon_memblock_from_file(&file->recordDataBase, file->diskFile, header.recordSize);
        if (!status) {
            carbon_memblock_get_error(&err, file->recordDataBase);
            carbon_error_cpy(&file->err, &err);
            return false;
        }

        carbon_memfile_t memFile;
        if (carbon_memfile_open(&memFile, file->recordDataBase, CARBON_MEMFILE_MODE_READONLY) != true) {
            CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
            status = false;
        }
        if (*CARBON_MEMFILE_PEEK(&memFile, char) != MARKER_SYMBOL_OBJECT_BEGIN) {
            CARBON_ERROR(&file->err, CARBON_ERR_CORRUPTED);
            status = false;
        }
        return true;
    }
}

static void resetStringDicDiskFileCursor(carbon_archive_record_table_t *file)
{
    fseek(file->diskFile, sizeof(struct CabinFileHeader) + sizeof(struct EmbeddedDicHeader), SEEK_SET);
}

CARBON_FUNC_UNUSED
static void convertObjectToModel(carbon_doc_t *model, carbon_archive_object_t *obj)
{
//    carbon_doc_t
    CARBON_UNUSED(model);
    CARBON_UNUSED(obj);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//
//
//  C A B I N   O B J E C T
//
//
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static void resetCabinObjectMemFile(carbon_archive_object_t *object)
{
    carbon_memfile_seek(&object->file, object->self);
}

CARBON_FUNC_UNUSED
static void getObjectProperties(carbon_archive_object_t *object)
{
    if (object->flags.bits.has_object_props) {
        assert(object->props.objects != 0);
        carbon_memfile_seek(&object->file, object->props.objects);
        resetCabinObjectMemFile(object);
    } else {

    }
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  R E C O R D   T A B L E   I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

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
            EmbeddedNullProp prop;
            embeddedNullPropsRead(&prop, &obj->file);
            resetCabinObjectMemFile(obj);
            CARBON_OPTIONAL_SET(npairs, prop.header->numEntries);
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
            EmbeddedVarProp objectProp;
            embeddedVarPropsRead(&objectProp, &obj->file);
            resetCabinObjectMemFile(obj);
            CARBON_OPTIONAL_SET(npairs, objectProp.header->numEntries);
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
        CARBON_ERROR(&obj->err, CARBON_ERR_ERRINTERNAL) /* wrong usage: use table get function instead */
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
        EmbeddedTableProp prop;
        embeddedTablePropsRead(&prop, &obj->file);
        resetCabinObjectMemFile(obj);
        out->ngroups = prop.header->numEntries;
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
        EmbeddedVarProp objectProp;
        embeddedVarPropsRead(&objectProp, &props->file);
        if (idx > objectProp.header->numEntries) {
            resetCabinObjectMemFile(props);
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
        EmbeddedFixedProp prop;                                                                                        \
        embeddedFixedPropsRead(&prop, &obj->file);                                                                  \
        resetCabinObjectMemFile(obj);                                                                                  \
        CARBON_OPTIONAL_SET(npairs, prop.header->numEntries);                                                               \
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

#define OBJECT_GET_ARRAY_LENGTHS_GENERIC(err, length, obj, bitfielName, offsetName, idx, prop)                              \
({                                                                                                                     \
    int status;                                                                                                        \
                                                                                                                       \
    if (obj->flags.bits.bitfielName) {                                                                           \
        assert(obj->props.offsetName != 0);                                                                      \
        carbon_memfile_seek(&obj->file, obj->props.offsetName);                                                       \
        embeddedArrayPropsRead(&prop, &obj->file);                                                                  \
        resetCabinObjectMemFile(obj);                                                                                  \
        if (CARBON_BRANCH_UNLIKELY(idx >= prop.header->numEntries)) {                                                         \
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
        carbon_memfile_seek(&obj->file, prop.valuesBegin + skipSize);                                                       \
        result = carbon_memfile_peek(&obj->file, 1);                                                                        \
        resetCabinObjectMemFile(obj);                                                                                  \
    }                                                                                                                  \
    (const T*) result;                                                                                              \
})

const carbon_int8_t *carbon_archive_object_values_int8_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int8_array_props,
                                                  int8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int8_t);
}

const carbon_int16_t *carbon_archive_object_values_int16_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int16_array_props,
                                                  int16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int16_t);
}

const carbon_int32_t *carbon_archive_object_values_int32_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int32_array_props,
                                                  int32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int32_t);
}

const carbon_int64_t *carbon_archive_object_values_int64_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_int64_array_props,
                                                  int64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_int64_t);
}

const carbon_uint8_t *carbon_archive_object_values_uint8_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint8_array_props,
                                                  uint8_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint8_t);
}

const carbon_uint16_t *carbon_archive_object_values_uint16_arrays(uint32_t *length,
                                                                  size_t idx,
                                                                  carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint16_array_props,
                                                  uint16_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uint16_t);
}

const carbon_uin32_t *carbon_archive_object_values_uint32_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint32_array_props,
                                                  uint32_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin32_t);
}

const carbon_uin64_t *carbon_archive_object_values_uint64_arrays(uint32_t *length,
                                                                 size_t idx,
                                                                 carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_uint64_array_props,
                                                  uint64_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_uin64_t);
}

const carbon_bool_t *carbon_archive_object_values_bool_arrays(uint32_t *length,
                                                              size_t idx,
                                                              carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_bool_array_props,
                                                  bool_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_bool_t);
}

const carbon_float_t *carbon_archive_object_values_float_arrays(uint32_t *length,
                                                                size_t idx,
                                                                carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_float_array_props,
                                                  float_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_float_t);
}

const carbon_string_id_t *carbon_archive_object_values_string_arrays(uint32_t *length,
                                                                     size_t idx,
                                                                     carbon_archive_object_t *obj)
{
    EmbeddedArrayProp prop;
    int status = OBJECT_GET_ARRAY_LENGTHS_GENERIC(&obj->err, length, obj, has_string_array_props,
                                                  string_arrays, idx, prop);
    return OBJECT_GET_ARRAY_VALUES_GENERIC(status, idx, prop, obj, carbon_string_id_t);
}

bool carbon_archive_object_values_null_array_lengths(uint32_t *length, size_t idx, carbon_archive_object_t *obj)
{
    CARBON_NON_NULL_OR_ERROR(length);
    CARBON_NON_NULL_OR_ERROR(obj);
    EmbeddedArrayProp prop;
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
        struct ColumnGroupHeader *columnGroupHeader = CARBON_MEMFILE_READ_TYPE(&table->context->file,
                                                                        struct ColumnGroupHeader);
        group->ncolumns = columnGroupHeader->numColumns;
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
        carbon_off_t columnOff = group->column_offsets[idx];
        carbon_memfile_seek(&group->context->file, columnOff);
        const struct ColumnHeader *header = CARBON_MEMFILE_READ_TYPE(&group->context->file, struct ColumnHeader);
        column->nelems = header->numEntries;
        column->type = getValueTypeOfChar(header->valueType);
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
    /* array does not map to array of type NULL */
    CARBON_PRINT_ERROR_AND_DIE_IF(field->type != carbon_field_type_null, CARBON_ERR_ERRINTERNAL)
    *length = *(uint32_t *) field->data;
    return true;
}

bool carbon_archive_table_field_object_cursor_open(carbon_object_cursor_t *cursor, carbon_field_t *field)
{
    CARBON_NON_NULL_OR_ERROR(cursor);
    CARBON_NON_NULL_OR_ERROR(field);
    cursor->field = field;
    cursor->currentIdx = 0;
    cursor->maxIdx = field->nentries;
    cursor->memBlock = field->context->file.memblock;
    return true;
}

bool carbon_archive_table_field_object_cursor_next(carbon_archive_object_t **obj, carbon_object_cursor_t *cursor)
{
    CARBON_NON_NULL_OR_ERROR(obj);
    CARBON_NON_NULL_OR_ERROR(cursor);
    if (cursor->currentIdx < cursor->maxIdx) {
        carbon_off_t readLength = objectSetup(&cursor->obj, cursor->memBlock, cursor->field->data_offset,
                                        cursor->field->context->context);
        cursor->field->data_offset += readLength;
        carbon_memfile_t file;
        carbon_memfile_open(&file, cursor->memBlock, CARBON_MEMFILE_MODE_READONLY);
        carbon_memfile_seek(&file, cursor->field->data_offset);
        cursor->field->data_offset = *CARBON_MEMFILE_READ_TYPE(&file, carbon_off_t);
        cursor->currentIdx++;
        *obj = &cursor->obj;
        return true;
    } else {
        return false;
    }
}











