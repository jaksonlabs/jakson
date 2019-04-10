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

#ifndef error_H
#define error_H

#include <stdint.h>

#include "shared/common.h"
#include "types.h"

CARBON_BEGIN_DECL

#define CARBON_ERR_NOERR 0                    /** No error */
#define CARBON_ERR_NULLPTR 1                  /** Null pointer detected */
#define CARBON_ERR_NOTIMPL 2                  /** Function not implemented */
#define CARBON_ERR_OUTOFBOUNDS 3              /** Index is out of bounds */
#define CARBON_ERR_MALLOCERR 4                /** Memory allocation failed */
#define CARBON_ERR_ILLEGALARG 5               /** Illegal arguments */
#define CARBON_ERR_INTERNALERR 6              /** Internal error */
#define CARBON_ERR_ILLEGALIMPL 7              /** Illegal implementation */
#define CARBON_ERR_NOTFOUND 8                 /** Not found */
#define CARBON_ERR_NIL 9                      /** Element not in list */
#define CARBON_ERR_ARRAYOFARRAYS 10           /** Array index out of bounds */
#define CARBON_ERR_ARRAYOFMIXEDTYPES 11       /** Illegal JSON array: mixed types */
#define CARBON_ERR_FOPEN_FAILED 12            /** Reading from file failed */
#define CARBON_ERR_IO 13                      /** I/O error */
#define CARBON_ERR_FORMATVERERR 14            /** Unsupported archive format version */
#define CARBON_ERR_CORRUPTED 15               /** Archive file is corrupted */
#define CARBON_ERR_NOCARBONSTREAM 16          /** Stream is not a carbon archive */
#define CARBON_ERR_NOBITMODE 17               /** Not in bit writing mode */
#define CARBON_ERR_NOTIMPLEMENTED 18          /** Function is not yet implemented */
#define CARBON_ERR_NOTYPE 19                  /** Unsupported type found */
#define CARBON_ERR_NOCOMPRESSOR 20            /** Unsupported compressor strategy requested */
#define CARBON_ERR_NOVALUESTR 21              /** No string representation for type available */
#define CARBON_ERR_MARKERMAPPING 22           /** Marker type cannot be mapped to value type */
#define CARBON_ERR_PARSETYPE 23               /** Parsing stopped; unknown data type requested */
#define CARBON_ERR_NOJSONTOKEN 24             /** Unknown token during parsing JSON detected */
#define CARBON_ERR_NOJSONNUMBERT 25           /** Unknown value type for number in JSON property */
#define CARBON_ERR_NOARCHIVEFILE 26           /** Stream is not a valid archive file */
#define CARBON_ERR_UNSUPFINDSTRAT 27          /** Unsupported strategy requested for key lookup */
#define CARBON_ERR_ERRINTERNAL 28             /** Internal error */
#define CARBON_ERR_HUFFERR 29                 /** No huffman code table entry found for character */
#define CARBON_ERR_MEMSTATE 30                /** Memory file was opened as read-only but requested a modification */
#define CARBON_ERR_JSONTYPE 31                /** Unable to import json file: unsupported type */
#define CARBON_ERR_WRITEPROT 32               /** Mode set to read-only but modification was requested */
#define CARBON_ERR_READOUTOFBOUNDS 33         /** Read outside of memory range bounds */
#define CARBON_ERR_SLOTBROKEN 34              /** Slot management broken */
#define CARBON_ERR_THREADOOOBJIDS 35          /** Thread run out of object ids: start another one */
#define CARBON_ERR_JSONPARSEERR 36            /** JSON parsing error */
#define CARBON_ERR_BULKCREATEFAILED 37        /** Document insertion bulk creation failed */
#define CARBON_ERR_FOPENWRITE 38              /** File cannot be opened for writing */
#define CARBON_ERR_WRITEARCHIVE 39            /** Archive cannot be serialized into file */
#define CARBON_ERR_ARCHIVEOPEN 40             /** Archive cannot be deserialized form file */
#define CARBON_ERR_FREAD_FAILED 41            /** Unable to read from file */
#define CARBON_ERR_SCAN_FAILED 42             /** Unable to perform full scan in archive file */
#define CARBON_ERR_DECOMPRESSFAILED 43        /** String decompression from archive failed */
#define CARBON_ERR_ITERATORNOTCLOSED 44       /** Closing iterator failed */
#define CARBON_ERR_HARDCOPYFAILED 45          /** Unable to construct a hard copy of the source object */
#define CARBON_ERR_REALLOCERR 46              /** Memory reallocation failed */
#define CARBON_ERR_PREDEVAL_FAILED 47         /** Predicate evaluation failed */
#define CARBON_ERR_INITFAILED 48              /** Initialization failed */
#define CARBON_ERR_DROPFAILED 49              /** Resource release failed: potentially a memory leak occurred */
#define CARBON_ERR_OPPFAILED 50               /** Operation failed */
#define CARBON_ERR_REHASH_NOROLLBACK 51       /** Rehashing hash table failed; rollback is not performed */
#define CARBON_ERR_MEMFILEOPEN_FAILED 52      /** Unable to open memory file */
#define CARBON_ERR_VITEROPEN_FAILED 53        /** Value iterator cannot be initialized */
#define CARBON_ERR_MEMFILESKIP_FAILED 54      /** Memfile cannot skip desired amount of bytes */
#define CARBON_ERR_MEMFILESEEK_FAILED 55      /** Unable to seek in memory file */
#define CARBON_ERR_ITER_NOOBJ 56              /** Unable to get value: type is not non-array object */
#define CARBON_ERR_ITER_NOBOOL 57             /** Unable to get value: type is not non-array boolean */
#define CARBON_ERR_ITER_NOINT8 58             /** Unable to get value: type is not non-array int8 */
#define CARBON_ERR_ITER_NOINT16 59            /** Unable to get value: type is not non-array int16 */
#define CARBON_ERR_ITER_NOINT32 60            /** Unable to get value: type is not non-array int32 */
#define CARBON_ERR_ITER_NOINT64 61            /** Unable to get value: type is not non-array int64 */
#define CARBON_ERR_ITER_NOUINT8 62            /** Unable to get value: type is not non-array uint8 */
#define CARBON_ERR_ITER_NOUINT16 63           /** Unable to get value: type is not non-array uint16 */
#define CARBON_ERR_ITER_NOUINT32 64           /** Unable to get value: type is not non-array uint32 */
#define CARBON_ERR_ITER_NOUINT64 65           /** Unable to get value: type is not non-array uint64 */
#define CARBON_ERR_ITER_NONUMBER 66           /** Unable to get value: type is not non-array number */
#define CARBON_ERR_ITER_NOSTRING 67           /** Unable to get value: type is not non-array string */
#define CARBON_ERR_ITER_OBJECT_NEEDED 68      /** Illegal state: iteration over object issued, but collection found */
#define CARBON_ERR_ITER_COLLECTION_NEEDED 69  /** Illegal state: iteration over collection issued, but object found */
#define CARBON_ERR_TYPEMISMATCH 70            /** Type mismatch */
#define CARBON_ERR_INDEXCORRUPTED_OFFSET 71   /** Index is corrupted: requested offset is outside file bounds */
#define CARBON_ERR_TMP_FOPENWRITE 72          /** Temporary file cannot be opened for writing */
#define CARBON_ERR_FWRITE_FAILED 73           /** Unable to write to file */
#define CARBON_ERR_HASTABLE_DESERIALERR 74    /** Unable to deserialize hash table from file */
#define CARBON_ERR_UNKNOWN_DIC_TYPE 75        /** Unknown string dictionary implementation requested */

static const char *const _carbon_err_str[] = {
    "No error",
    "Null pointer detected",
    "Function not implemented",
    "Index is out of bounds",
    "Memory allocation failed",
    "Illegal arguments",
    "Internal error",
    "Illegal implementation",
    "Not found",
    "Element not in list",
    "Array index out of bounds",
    "Illegal JSON array: mixed types",
    "Reading from file failed",
    "I/O error",
    "Unsupported archive format version",
    "Archive file is corrupted",
    "Stream is not a types archive",
    "Not in bit writing mode",
    "Function is not yet implemented",
    "Unsupported type found",
    "Unsupported pack strategy requested",
    "No string representation for type available",
    "Marker type cannot be mapped to value type",
    "Parsing stopped; unknown data type requested",
    "Unknown token during parsing JSON detected",
    "Unknown value type for number in JSON property",
    "Stream is not a valid archive file",
    "Unsupported strategy requested for key lookup",
    "Internal error",
    "No huffman code table entry found for character",
    "Memory file was opened as read-only but requested a modification",
    "Unable to import json file: unsupported type",
    "Mode set to read-only but modification was requested",
    "Read outside of memory range bounds",
    "Slot management broken",
    "Thread run out of object ids: start another one",
    "JSON parsing error",
    "Document insertion bulk creation failed",
    "File cannot be opened for writing",
    "Archive cannot be serialized into file",
    "Archive cannot be deserialized form file",
    "Unable to read from file",
    "Unable to perform full scan in archive file",
    "String decompression from archive failed",
    "Closing iterator failed",
    "Unable to construct a hard copy of the source object",
    "Memory reallocation failed",
    "Predicate evaluation failed",
    "Initialization failed",
    "Resource release failed: potentially a memory leak occurred",
    "Operation failed",
    "Rehashing hash table failed; rollback is not performed",
    "Unable to open memory file",
    "Value iterator cannot be initialized",
    "Memfile cannot skip desired amount of bytes",
    "Unable to seek in memory file",
    "Unable to get value: type is not non-array object",
    "Unable to get value: type is not non-array boolean",
    "Unable to get value: type is not non-array int8",
    "Unable to get value: type is not non-array int16",
    "Unable to get value: type is not non-array int32",
    "Unable to get value: type is not non-array int64",
    "Unable to get value: type is not non-array uint8",
    "Unable to get value: type is not non-array uint16",
    "Unable to get value: type is not non-array uint32",
    "Unable to get value: type is not non-array uint64",
    "Unable to get value: type is not non-array number",
    "Unable to get value: type is not non-array string",
    "Illegal state: iteration over object issued, but collection found",
    "Illegal state: iteration over collection issued, but object found",
    "Type mismatch",
    "Index is corrupted: requested offset is outside file bounds",
    "Temporary file cannot be opened for writing",
    "Unable to write to file",
    "Unable to deserialize hash table from file",
    "Unknown string dictionary implementation requested"
};

#define CARBON_ERRSTR_ILLEGAL_CODE "illegal error code"

static const int _carbon_nerr_str = CARBON_ARRAY_LENGTH(_carbon_err_str);

struct err
{
    int          code;
    const char  *file;
    u32     line;
    char        *details;
};

CARBON_EXPORT(bool)
carbon_error_init(struct err *err);

CARBON_EXPORT(bool)
carbon_error_cpy(struct err *dst, const struct err *src);

CARBON_EXPORT(bool)
carbon_error_drop(struct err *err);

CARBON_EXPORT(bool)
carbon_error_set(struct err *err, int code, const char *file, u32 line);

CARBON_EXPORT(bool)
carbon_error_set_wdetails(struct err *err, int code, const char *file, u32 line, const char *details);

CARBON_EXPORT(bool)
carbon_error_str(const char **errstr, const char **file, u32 *line, bool *details, const char **detailsstr,
                 const struct err *err);

CARBON_EXPORT(bool)
carbon_error_print(const struct err *err);

CARBON_EXPORT(bool)
carbon_error_print_and_abort(const struct err *err);

#define error_OCCURRED(x)                   ((x)->err.code != CARBON_ERR_NOERR)

#define error(err, code)                     error_IF (true, err, code)
#define error_IF(expr, err, code)            { if (expr) { carbon_error_set(err, code, __FILE__, __LINE__); } }
#define error_IF_AND_RETURN(expr, err, code, retval) \
                                                    { if (expr) { carbon_error_set(err, code, __FILE__, __LINE__);     \
                                                                  return retval; } }
#define error_WDETAILS(err, code, msg)       carbon_error_set_wdetails(err, code, __FILE__, __LINE__, msg);

#define CARBON_PRINT_ERROR(code)                    CARBON_PRINT_ERROR_IF(true, code)
#define carbon_print_error_and_die(code)            { CARBON_PRINT_ERROR(code); abort(); }
#define CARBON_PRINT_ERROR_AND_DIE_IF(expr, code)   { if(expr) { carbon_print_error_and_die(code) } }
#define CARBON_PRINT_ERROR_IF(expr, code)                                                                              \
{                                                                                                                      \
    if (expr) {                                                                                                        \
        struct err err;                                                                                              \
        carbon_error_init(&err);                                                                                       \
        error(&err, code);                                                                                      \
        carbon_error_print(&err);                                                                                      \
    }                                                                                                                  \
}

#define CARBON_DEFINE_GET_ERROR_FUNCTION(type_name, type, arg)                                                         \
CARBON_FUNC_UNUSED static bool                                                                                         \
carbon_##type_name##_get_error(struct err *err, const type *arg)                                                     \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(err)                                                                                      \
    CARBON_NON_NULL_OR_ERROR(arg)                                                                                      \
    carbon_error_cpy(err, &arg->err);                                                                                  \
    return true;                                                                                                       \
}

CARBON_END_DECL

#endif
