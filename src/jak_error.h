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

#ifndef JAK_ERROR_H
#define JAK_ERROR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdint.h>

#include <jak_stdinc.h>
#include <jak_types.h>

JAK_BEGIN_DECL

#define JAK_ERR_NOERR 0                    /** No JAK_ERROR */
#define JAK_ERR_NULLPTR 1                  /** Null pointer detected */
#define JAK_ERR_NOTIMPL 2                  /** Function not implemented */
#define JAK_ERR_OUTOFBOUNDS 3              /** Index is out of bounds */
#define JAK_ERR_MALLOCERR 4                /** Memory allocation failed */
#define JAK_ERR_ILLEGALARG 5               /** Illegal arguments */
#define JAK_ERR_INTERNALERR 6              /** Internal JAK_ERROR */
#define JAK_ERR_ILLEGALIMPL 7              /** Illegal implementation */
#define JAK_ERR_NOTFOUND 8                 /** Not found */
#define JAK_ERR_NIL 9                      /** Element not in list */
#define JAK_ERR_ARRAYOFARRAYS 10           /** Array index out of bounds */
#define JAK_ERR_ARRAYOFMIXEDTYPES 11       /** Illegal JSON array: mixed types */
#define JAK_ERR_FOPEN_FAILED 12            /** Reading from file failed */
#define JAK_ERR_IO 13                      /** I/O JAK_ERROR */
#define JAK_ERR_FORMATVERERR 14            /** Unsupported archive format version */
#define JAK_ERR_CORRUPTED 15               /** Format is corrupted */
#define JAK_ERR_NOCARBONSTREAM 16          /** Stream is not a carbon archive */
#define JAK_ERR_NOBITMODE 17               /** Not in bit writing mode */
#define JAK_ERR_NOTIMPLEMENTED 18          /** Function is not yet implemented */
#define JAK_ERR_NOTYPE 19                  /** Unsupported type found */
#define JAK_ERR_NOCOMPRESSOR 20            /** Unsupported compressor strategy requested */
#define JAK_ERR_NOVALUESTR 21              /** No string representation for type available */
#define JAK_ERR_MARKERMAPPING 22           /** Marker type cannot be mapped to value type */
#define JAK_ERR_PARSETYPE 23               /** Parsing stopped; unknown data type requested */
#define JAK_ERR_NOJSONTOKEN 24             /** Unknown token during parsing JSON detected */
#define JAK_ERR_NOJSONNUMBERT 25           /** Unknown value type for number in JSON property */
#define JAK_ERR_NOARCHIVEFILE 26           /** Stream is not a valid archive file */
#define JAK_ERR_UNSUPFINDSTRAT 27          /** Unsupported strategy requested for key lookup */
#define JAK_ERR_ERRINTERNAL 28             /** Internal JAK_ERROR */
#define JAK_ERR_HUFFERR 29                 /** No huffman code table entry found for character */
#define JAK_ERR_MEMSTATE 30                /** Memory file was opened as read-only but requested a modification */
#define JAK_ERR_JSONTYPE 31                /** Unable to import json file: unsupported type */
#define JAK_ERR_WRITEPROT 32               /** Mode set to read-only but modification was requested */
#define JAK_ERR_READOUTOFBOUNDS 33         /** Read outside of memory range bounds */
#define JAK_ERR_SLOTBROKEN 34              /** Slot management broken */
#define JAK_ERR_THREADOOOBJIDS 35          /** Thread run out of object ids: start another one */
#define JAK_ERR_JSONPARSEERR 36            /** JSON parsing JAK_ERROR */
#define JAK_ERR_BULKCREATEFAILED 37        /** Document insertion bulk creation failed */
#define JAK_ERR_FOPENWRITE 38              /** File cannot be opened for writing */
#define JAK_ERR_WRITEARCHIVE 39            /** Archive cannot be serialized into file */
#define JAK_ERR_ARCHIVEOPEN 40             /** Archive cannot be deserialized form file */
#define JAK_ERR_FREAD_FAILED 41            /** Unable to read from file */
#define JAK_ERR_SCAN_FAILED 42             /** Unable to perform full scan in archive file */
#define JAK_ERR_DECOMPRESSFAILED 43        /** String decompression from archive failed */
#define JAK_ERR_ITERATORNOTCLOSED 44       /** Closing iterator failed */
#define JAK_ERR_HARDCOPYFAILED 45          /** Unable to construct a hard copy of the source object */
#define JAK_ERR_REALLOCERR 46              /** Memory reallocation failed */
#define JAK_ERR_PREDEVAL_FAILED 47         /** Predicate evaluation failed */
#define JAK_ERR_INITFAILED 48              /** Initialization failed */
#define JAK_ERR_DROPFAILED 49              /** Resource release failed: potentially a memory leak occurred */
#define JAK_ERR_OPPFAILED 50               /** Operation failed */
#define JAK_ERR_REHASH_NOROLLBACK 51       /** Rehashing hash table failed; rollback is not performed */
#define JAK_ERR_MEMFILEOPEN_FAILED 52      /** Unable to open memory file */
#define JAK_ERR_VITEROPEN_FAILED 53        /** Value iterator cannot be initialized */
#define JAK_ERR_MEMFILESKIP_FAILED 54      /** Memfile cannot skip desired amount of bytes */
#define JAK_ERR_MEMFILESEEK_FAILED 55      /** Unable to seek in memory file */
#define JAK_ERR_ITER_NOOBJ 56              /** Unable to get value: type is not non-array object */
#define JAK_ERR_ITER_NOBOOL 57             /** Unable to get value: type is not non-array boolean */
#define JAK_ERR_ITER_NOINT8 58             /** Unable to get value: type is not non-array int8 */
#define JAK_ERR_ITER_NOINT16 59            /** Unable to get value: type is not non-array int16 */
#define JAK_ERR_ITER_NOINT32 60            /** Unable to get value: type is not non-array int32 */
#define JAK_ERR_ITER_NOINT64 61            /** Unable to get value: type is not non-array int64 */
#define JAK_ERR_ITER_NOUINT8 62            /** Unable to get value: type is not non-array uint8 */
#define JAK_ERR_ITER_NOUINT16 63           /** Unable to get value: type is not non-array uint16 */
#define JAK_ERR_ITER_NOUINT32 64           /** Unable to get value: type is not non-array uint32 */
#define JAK_ERR_ITER_NOUINT64 65           /** Unable to get value: type is not non-array uint64 */
#define JAK_ERR_ITER_NONUMBER 66           /** Unable to get value: type is not non-array number */
#define JAK_ERR_ITER_NOSTRING 67           /** Unable to get value: type is not non-array string */
#define JAK_ERR_ITER_OBJECT_NEEDED 68      /** Illegal state: iteration over object issued, but collection found */
#define JAK_ERR_ITER_COLLECTION_NEEDED 69  /** Illegal state: iteration over collection issued, but object found */
#define JAK_ERR_TYPEMISMATCH 70            /** Type mismatch detected */
#define JAK_ERR_INDEXCORRUPTED_OFFSET 71   /** Index is corrupted: requested offset is outside file bounds */
#define JAK_ERR_TMP_FOPENWRITE 72          /** Temporary file cannot be opened for writing */
#define JAK_ERR_FWRITE_FAILED 73           /** Unable to write to file */
#define JAK_ERR_HASTABLE_DESERIALERR 74    /** Unable to deserialize hash table from file */
#define JAK_ERR_UNKNOWN_DIC_TYPE 75        /** Unknown string dictionary implementation requested */
#define JAK_ERR_STACK_OVERFLOW 76          /** Stack overflow */
#define JAK_ERR_STACK_UNDERFLOW 77         /** Stack underflow */
#define JAK_ERR_OUTDATED 78                /** Object was modified but is out of date */
#define JAK_ERR_NOTREADABLE 79             /** Object is currently being updated; no read allowed */
#define JAK_ERR_ILLEGALOP 80               /** Illegal operation */
#define JAK_ERR_BADTYPE 81                 /** Unsupported type */
#define JAK_ERR_UNSUPPCONTAINER 82         /** Unsupported container for data type */
#define JAK_ERR_INSERT_TOO_DANGEROUS 83    /** Adding integers with this function will perform an auto casting to
                                             * the smallest type required to store the integer value. Since you push
                                             * integers with this function into an column container that is bound
                                             * to a specific type, any insertion function call will fail once the
                                             * integer value requires a larger (or smaller) type than the fist value
                                             * added to the container. Use '*_insert_X' instead, where X is jak_u8, jak_u16,...
                                             * , jak_u32 resp. jak_i8, jak_i16,..., jak_i32. */
#define JAK_ERR_PARSE_DOT_EXPECTED 84       /** parsing JAK_ERROR: dot ('.') expected */
#define JAK_ERR_PARSE_ENTRY_EXPECTED 85     /** parsing JAK_ERROR: key name or array index expected */
#define JAK_ERR_PARSE_UNKNOWN_TOKEN 86      /** parsing JAK_ERROR: unknown token */
#define JAK_ERR_DOT_PATH_PARSERR 87         /** dot-notated path could not be parsed */
#define JAK_ERR_ILLEGALSTATE 88             /** Illegal state */
#define JAK_ERR_UNSUPPORTEDTYPE 89          /** Unsupported data type */
#define JAK_ERR_FAILED 90                   /** Operation failed */
#define JAK_ERR_CLEANUP 91                  /** Cleanup operation failed; potentially a memory leak occurred */
#define JAK_ERR_DOT_PATH_COMPILEERR 92      /** dot-notated path could not be compiled */
#define JAK_ERR_NONUMBER 93                 /** not a number */
#define JAK_ERR_BUFFERTOOTINY 94            /** buffer capacity exceeded */
#define JAK_ERR_TAILINGJUNK 95              /** tailing junk was detected in a stream */
#define JAK_ERR_NOTINDEXED 96               /** not indexed */

static const char *const jak_global_err_str[] =
        {"No JAK_ERROR", "Null pointer detected", "Function not implemented", "Index is out of bounds",
         "Memory allocation failed", "Illegal arguments", "Internal JAK_ERROR", "Illegal implementation", "Not found",
         "Element not in list", "Array index out of bounds", "Illegal JSON array: mixed types",
         "Reading from file failed", "I/O JAK_ERROR", "Unsupported archive format version", "Format is corrupted",
         "Stream is not a types archive", "Not in bit writing mode", "Function is not yet implemented",
         "Unsupported type found", "Unsupported pack strategy requested", "No string representation for type available",
         "Marker type cannot be mapped to value type", "Parsing stopped; unknown data type requested",
         "Unknown token during parsing JSON detected", "Unknown value type for number in JSON property",
         "Stream is not a valid archive file", "Unsupported strategy requested for key lookup", "Internal JAK_ERROR",
         "No huffman code table entry found for character",
         "Memory file was opened as read-only but requested a modification",
         "Unable to import json file: unsupported type", "Mode set to read-only but modification was requested",
         "Read outside of memory range bounds", "Slot management broken",
         "Thread run out of object ids: start another one", "JSON parsing JAK_ERROR",
         "Document insertion bulk creation failed", "File cannot be opened for writing",
         "Archive cannot be serialized into file", "Archive cannot be deserialized form file",
         "Unable to read from file", "Unable to perform full scan in archive file",
         "String decompression from archive failed", "Closing iterator failed",
         "Unable to construct a hard copy of the source object", "Memory reallocation failed",
         "Predicate evaluation failed", "Initialization failed",
         "Resource release failed: potentially a memory leak occurred", "Operation failed",
         "Rehashing hash table failed; rollback is not performed", "Unable to open memory file",
         "Value iterator cannot be initialized", "Memfile cannot skip desired amount of bytes",
         "Unable to seek in memory file", "Unable to get value: type is not non-array object",
         "Unable to get value: type is not non-array boolean", "Unable to get value: type is not non-array int8",
         "Unable to get value: type is not non-array int16", "Unable to get value: type is not non-array int32",
         "Unable to get value: type is not non-array int64", "Unable to get value: type is not non-array uint8",
         "Unable to get value: type is not non-array uint16", "Unable to get value: type is not non-array uint32",
         "Unable to get value: type is not non-array uint64", "Unable to get value: type is not non-array number",
         "Unable to get value: type is not non-array string",
         "Illegal state: iteration over object issued, but collection found",
         "Illegal state: iteration over collection issued, but object found", "Type mismatch detected",
         "Index is corrupted: requested offset is outside file bounds", "Temporary file cannot be opened for writing",
         "Unable to write to file", "Unable to deserialize hash table from file",
         "Unknown string dictionary implementation requested", "Stack overflow", "Stack underflow",
         "Object was modified but is out of date", "Object is currently being updated; no read allowed",
         "Illegal operation", "Unsupported type", "Unsupported container for data type",
         "Adding integers with this function will perform an auto casting to the smallest type required to store "
         "the integer value. Since you push integers with this function into an column container that is bound "
         "to a specific type, any insertion function call will fail once the integer value requires a larger "
         "(or smaller) type than the fist value added to the container. Use '*_insert_X' instead, where X is "
         "jak_u8, jak_u16,..., jak_u32 resp. jak_i8, jak_i16,..., jak_i32. ", "parsing JAK_ERROR dot ('.') expected",
         "parsing JAK_ERROR key name or array index expected", "parsing JAK_ERROR: unknown token",
         "dot-notated path could not be parsed", "Illegal state", "Unsupported data type", "Operation failed",
         "Cleanup operation failed; potentially a memory leak occurred", "dot-notated path could not be compiled",
         "not a number", "buffer capacity exceeded", "tailing junk was detected in a stream", "not indexed"};

#define JAK_ERRSTR_ILLEGAL_CODE "illegal JAK_ERROR code"

static const int jak_global_nerr_str = JAK_ARRAY_LENGTH(jak_global_err_str);

typedef struct jak_error {
        int code;
        const char *file;
        jak_u32 line;
        char *details;
} jak_error;

bool jak_error_init(jak_error *err);

bool jak_error_cpy(jak_error *dst, const jak_error *src);

bool jak_error_drop(jak_error *err);

bool jak_error_set(jak_error *err, int code, const char *file, jak_u32 line);

bool jak_error_set_wdetails(jak_error *err, int code, const char *file, jak_u32 line, const char *details);

bool jak_error_set_no_abort(jak_error *err, int code, const char *file, jak_u32 line);

bool jak_error_set_wdetails_no_abort(jak_error *err, int code, const char *file, jak_u32 line, const char *details);

bool jak_error_str(const char **errstr, const char **file, jak_u32 *line, bool *details, const char **detailsstr,
               const jak_error *err);

bool jak_error_print_to_stderr(const jak_error *err);

bool jak_error_print_and_abort(const jak_error *err);

#define JAK_ERROR_OCCURED(x)                   ((x)->err.code != JAK_ERR_NOERR)

#define JAK_SUCCESS_ELSE_RETURN(expr, err, code, retval)                                                                   \
{                                                                                                                      \
        bool result = expr;                                                                                            \
        JAK_ERROR_IF(!(result), err, code);                                                                                \
        if (!(result)) { return retval; }                                                                              \
}

#define JAK_SUCCESS_ELSE_NULL(expr, err)           JAK_SUCCESS_ELSE_RETURN(expr, err, JAK_ERR_FAILED, NULL)
#define JAK_SUCCESS_ELSE_FAIL(expr, err)           JAK_SUCCESS_ELSE_RETURN(expr, err, JAK_ERR_FAILED, false)


#define JAK_ERROR(err, code)                     JAK_ERROR_IF (true, err, code)
#define JAK_ERROR_NO_ABORT(err, code)            JAK_ERROR_IF (true, err, code)
#define JAK_ERROR_IF(expr, err, code)            { if (expr) { jak_error_set(err, code, __FILE__, __LINE__); } }
#define JAK_ERROR_IF_AND_RETURN(expr, err, code, retval) \
                                                    { if (expr) { jak_error_set(err, code, __FILE__, __LINE__);            \
                                                                  return retval; } }

#define JAK_ERROR_IF_WDETAILS(expr, err, code, msg)            { if (expr) { JAK_ERROR_WDETAILS(err, code, msg); } }
#define JAK_ERROR_WDETAILS(err, code, msg)                     jak_error_set_wdetails(err, code, __FILE__, __LINE__, msg);

#define JAK_ERROR_PRINT(code)                    JAK_ERROR_PRINT_IF(true, code)
#define JAK_ERROR_PRINT_AND_DIE(code)            { JAK_ERROR_PRINT(code); abort(); }
#define JAK_ERROR_PRINT_AND_DIE_IF(expr, code)   { if(expr) { JAK_ERROR_PRINT_AND_DIE(code) } }
#define JAK_ERROR_PRINT_IF(expr, code)                                                                                     \
{                                                                                                                      \
    if (expr) {                                                                                                        \
        jak_error err;                                                                                                \
        jak_error_init(&err);                                                                                              \
        JAK_ERROR(&err, code);                                                                                             \
        jak_error_print_to_stderr(&err);                                                                                   \
    }                                                                                                                  \
}

#define JAK_DEFINE_ERROR_GETTER(type_tag_name)  JAK_DEFINE_GET_ERROR_FUNCTION(type_tag_name, struct type_tag_name, e)

#define JAK_DEFINE_GET_ERROR_FUNCTION(type_name, type, arg)                                                            \
JAK_FUNC_UNUSED static bool                                                                                            \
jak_##type_name##_get_error(jak_error *err, const type *arg)                                                                \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(err)                                                                                                 \
    JAK_ERROR_IF_NULL(arg)                                                                                                 \
    jak_error_cpy(err, &arg->err);                                                                                         \
    return true;                                                                                                       \
}

JAK_END_DECL

#endif
