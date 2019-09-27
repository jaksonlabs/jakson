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

#ifndef ERROR_H
#define ERROR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <stdint.h>

#include <jakson/forwdecl.h>
#include <jakson/stdinc.h>
#include <jakson/types.h>

BEGIN_DECL

typedef u16 error_code;

extern _Thread_local err global_error;

#define ERR_NOERR 0                    /** No ERROR */
#define ERR_NULLPTR 1                  /** Null pointer detected */
#define ERR_NOTIMPL 2                  /** Function not implemented */
#define ERR_OUTOFBOUNDS 3              /** Index is out of bounds */
#define ERR_MALLOCERR 4                /** Memory allocation failed */
#define ERR_ILLEGALARG 5               /** Illegal arguments */
#define ERR_INTERNALERR 6              /** Internal ERROR */
#define ERR_ILLEGALIMPL 7              /** Illegal implementation */
#define ERR_NOTFOUND 8                 /** Not found */
#define ERR_NIL 9                      /** Element not in list */
#define ERR_ARRAYOFARRAYS 10           /** Array index out of bounds */
#define ERR_ARRAYOFMIXEDTYPES 11       /** Illegal JSON array: mixed types */
#define ERR_FOPEN_FAILED 12            /** Reading from file failed */
#define ERR_IO 13                      /** I/O ERROR */
#define ERR_FORMATVERERR 14            /** Unsupported archive format version */
#define ERR_CORRUPTED 15               /** Format is corrupted */
#define ERR_NOCARBONSTREAM 16          /** Stream is not a carbon archive */
#define ERR_NOBITMODE 17               /** Not in bit writing mode */
#define ERR_NOTIMPLEMENTED 18          /** Function is not yet implemented */
#define ERR_NOTYPE 19                  /** Unsupported type found */
#define ERR_NOCOMPRESSOR 20            /** Unsupported compressor strategy requested */
#define ERR_NOVALUESTR 21              /** No string representation for type available */
#define ERR_MARKERMAPPING 22           /** Marker type cannot be mapped to value type */
#define ERR_PARSETYPE 23               /** Parsing stopped; unknown data type requested */
#define ERR_NOJSONTOKEN 24             /** Unknown token during parsing JSON detected */
#define ERR_NOJSONNUMBERT 25           /** Unknown value type for number in JSON property */
#define ERR_NOARCHIVEFILE 26           /** Stream is not a valid archive file */
#define ERR_UNSUPFINDSTRAT 27          /** Unsupported strategy requested for key lookup */
#define ERR_ERRINTERNAL 28             /** Internal ERROR */
#define ERR_HUFFERR 29                 /** No huffman code table entry found for character */
#define ERR_MEMSTATE 30                /** Memory file was opened as read-only but requested a modification */
#define ERR_JSONTYPE 31                /** Unable to import json file: unsupported type */
#define ERR_WRITEPROT 32               /** Mode set to read-only but modification was requested */
#define ERR_READOUTOFBOUNDS 33         /** Read outside of memory range bounds */
#define ERR_SLOTBROKEN 34              /** Slot management broken */
#define ERR_THREADOOOBJIDS 35          /** Thread run out of object ids: start another one */
#define ERR_JSONPARSEERR 36            /** JSON parsing ERROR */
#define ERR_BULKCREATEFAILED 37        /** Document insertion bulk creation failed */
#define ERR_FOPENWRITE 38              /** File cannot be opened for writing */
#define ERR_WRITEARCHIVE 39            /** Archive cannot be serialized into file */
#define ERR_ARCHIVEOPEN 40             /** Archive cannot be deserialized form file */
#define ERR_FREAD_FAILED 41            /** Unable to read from file */
#define ERR_SCAN_FAILED 42             /** Unable to perform full scan in archive file */
#define ERR_DECOMPRESSFAILED 43        /** String decompression from archive failed */
#define ERR_ITERATORNOTCLOSED 44       /** Closing iterator failed */
#define ERR_HARDCOPYFAILED 45          /** Unable to construct a hard copy of the source object */
#define ERR_REALLOCERR 46              /** Memory reallocation failed */
#define ERR_PREDEVAL_FAILED 47         /** Predicate evaluation failed */
#define ERR_INITFAILED 48              /** Initialization failed */
#define ERR_DROPFAILED 49              /** Resource release failed: potentially a memory leak occurred */
#define ERR_OPPFAILED 50               /** Operation failed */
#define ERR_REHASH_NOROLLBACK 51       /** Rehashing hash table failed; rollback is not performed */
#define ERR_MEMFILEOPEN_FAILED 52      /** Unable to open memory file */
#define ERR_VITEROPEN_FAILED 53        /** Value iterator cannot be initialized */
#define ERR_MEMFILESKIP_FAILED 54      /** Memfile cannot skip desired amount of bytes */
#define ERR_MEMFILESEEK_FAILED 55      /** Unable to seek in memory file */
#define ERR_ITER_NOOBJ 56              /** Unable to get value: type is not non-array object */
#define ERR_ITER_NOBOOL 57             /** Unable to get value: type is not non-array boolean */
#define ERR_ITER_NOINT8 58             /** Unable to get value: type is not non-array int8 */
#define ERR_ITER_NOINT16 59            /** Unable to get value: type is not non-array int16 */
#define ERR_ITER_NOINT32 60            /** Unable to get value: type is not non-array int32 */
#define ERR_ITER_NOINT64 61            /** Unable to get value: type is not non-array int64 */
#define ERR_ITER_NOUINT8 62            /** Unable to get value: type is not non-array uint8 */
#define ERR_ITER_NOUINT16 63           /** Unable to get value: type is not non-array uint16 */
#define ERR_ITER_NOUINT32 64           /** Unable to get value: type is not non-array uint32 */
#define ERR_ITER_NOUINT64 65           /** Unable to get value: type is not non-array uint64 */
#define ERR_ITER_NONUMBER 66           /** Unable to get value: type is not non-array number */
#define ERR_ITER_NOSTRING 67           /** Unable to get value: type is not non-array string */
#define ERR_ITER_OBJECT_NEEDED 68      /** Illegal state: iteration over object issued, but collection found */
#define ERR_ITER_COLLECTION_NEEDED 69  /** Illegal state: iteration over collection issued, but object found */
#define ERR_TYPEMISMATCH 70            /** Type mismatch detected */
#define ERR_INDEXCORRUPTED_OFFSET 71   /** Index is corrupted: requested offset is outside file bounds */
#define ERR_TMP_FOPENWRITE 72          /** Temporary file cannot be opened for writing */
#define ERR_FWRITE_FAILED 73           /** Unable to write to file */
#define ERR_HASTABLE_DESERIALERR 74    /** Unable to deserialize hash table from file */
#define ERR_UNKNOWN_DIC_TYPE 75        /** Unknown string dictionary implementation requested */
#define ERR_STACK_OVERFLOW 76          /** Stack overflow */
#define ERR_STACK_UNDERFLOW 77         /** Stack underflow */
#define ERR_OUTDATED 78                /** Object was modified but is out of date */
#define ERR_NOTREADABLE 79             /** Object is currently being updated; no read allowed */
#define ERR_ILLEGALOP 80               /** Illegal operation */
#define ERR_BADTYPE 81                 /** Unsupported type */
#define ERR_UNSUPPCONTAINER 82         /** Unsupported container for data type */
#define ERR_INSERT_TOO_DANGEROUS 83    /** Adding integers with this function will perform an auto casting to
                                             * the smallest type required to store the integer value. Since you push
                                             * integers with this function into an column container that is bound
                                             * to a specific type, any insertion function call will fail once the
                                             * integer value requires a larger (or smaller) type than the fist value
                                             * added to the container. Use '*_insert_X' instead, where X is u8, u16,...
                                             * , u32 resp. i8, i16,..., i32. */
#define ERR_PARSE_DOT_EXPECTED 84       /** parsing ERROR: dot ('.') expected */
#define ERR_PARSE_ENTRY_EXPECTED 85     /** parsing ERROR: key name or array index expected */
#define ERR_PARSE_UNKNOWN_TOKEN 86      /** parsing ERROR: unknown token */
#define ERR_DOT_PATH_PARSERR 87         /** dot-notated path could not be parsed */
#define ERR_ILLEGALSTATE 88             /** Illegal state */
#define ERR_UNSUPPORTEDTYPE 89          /** Unsupported data type */
#define ERR_FAILED 90                   /** Operation failed */
#define ERR_CLEANUP 91                  /** Cleanup operation failed; potentially a memory leak occurred */
#define ERR_DOT_PATH_COMPILEERR 92      /** dot-notated path could not be compiled */
#define ERR_NONUMBER 93                 /** not a number */
#define ERR_BUFFERTOOTINY 94            /** buffer capacity exceeded */
#define ERR_TAILINGJUNK 95              /** tailing junk was detected in a stream */
#define ERR_NOTINDEXED 96               /** not indexed */
#define ERR_NOERR_RESULT_BOOLEAN 97     /** No ERROR, and result is boolean */
#define ERR_NOERR_RESULT_INT 98         /** No ERROR, and result is 63bit int */
#define ERR_NOERR_RESULT_UINT 99        /** No ERROR, and result is 63bit unsigned int */
#define ERR_NOERR_RESULT_PTR 100        /** No ERROR, and result is 63bit pointer */
#define ERR_PERMISSIONS 101             /** Permissions error */

static const char *const global_err_str[] =
        {"No error", "Null pointer detected", "Function not implemented", "Index is out of bounds",
         "Memory allocation failed", "Illegal arguments", "Internal ERROR", "Illegal implementation", "Not found",
         "Element not in list", "Array index out of bounds", "Illegal JSON array: mixed types",
         "Reading from file failed", "I/O ERROR", "Unsupported archive format version", "Format is corrupted",
         "Stream is not a types archive", "Not in bit writing mode", "Function is not yet implemented",
         "Unsupported type found", "Unsupported pack strategy requested", "No string_buffer representation for type available",
         "Marker type cannot be mapped to value type", "Parsing stopped; unknown data type requested",
         "Unknown token during parsing JSON detected", "Unknown value type for number in JSON property",
         "Stream is not a valid archive file", "Unsupported strategy requested for key lookup", "Internal ERROR",
         "No huffman code table entry found for character",
         "Memory file was opened as read-only but requested a modification",
         "Unable to import json file: unsupported type", "Mode set to read-only but modification was requested",
         "Read outside of memory range bounds", "Slot management broken",
         "Thread run out of object ids: start another one", "JSON parsing ERROR",
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
         "Unable to get value: type is not non-array string_buffer",
         "Illegal state: iteration over object issued, but collection found",
         "Illegal state: iteration over collection issued, but object found", "Type mismatch detected",
         "Index is corrupted: requested offset is outside file bounds", "Temporary file cannot be opened for writing",
         "Unable to write to file", "Unable to deserialize hash table from file",
         "Unknown string_buffer dictionary implementation requested", "Stack overflow", "Stack underflow",
         "Object was modified but is out of date", "Object is currently being updated; no read allowed",
         "Illegal operation", "Unsupported type", "Unsupported container for data type",
         "Adding integers with this function will perform an auto casting to the smallest type required to store "
         "the integer value. Since you push integers with this function into an column container that is bound "
         "to a specific type, any insertion function call will fail once the integer value requires a larger "
         "(or smaller) type than the fist value added to the container. Use '*_insert_X' instead, where X is "
         "u8, u16,..., u32 resp. i8, i16,..., i32. ", "parsing ERROR dot ('.') expected",
         "parsing ERROR key name or array index expected", "parsing ERROR: unknown token",
         "dot-notated path could not be parsed", "Illegal state", "Unsupported data type", "Operation failed",
         "Cleanup operation failed; potentially a memory leak occurred", "dot-notated path could not be compiled",
         "not a number", "buffer capacity exceeded", "tailing junk was detected in a stream", "not indexed",
         "result is boolean", "result is 63bit int", "result is 63bit unsigned int", "result is 63bit pointer",
         "Permissions error"
        };

#define ERRSTR_ILLEGAL_CODE "illegal ERROR code"

static const int global_nerr_str = ARRAY_LENGTH(global_err_str);

typedef struct err {
        int code;
        const char *file;
        u32 line;
        char *details;
} err;

bool error_init(err *err);

bool error_cpy(err *dst, const err *src);

bool error_drop(err *err);

bool error_set(err *err, int code, const char *file, u32 line);

bool error_set_wdetails(err *err, int code, const char *file, u32 line, const char *details);

bool error_set_no_abort(err *err, int code, const char *file, u32 line);

bool error_set_wdetails_no_abort(err *err, int code, const char *file, u32 line, const char *details);

bool error_str(const char **errstr, const char **file, u32 *line, bool *details, const char **detailsstr,
               const err *err);

bool error_print_to_stderr(const err *err);

bool error_print_and_abort(const err *err);

#define ERROR_OCCURRED(x)                   ((x)->err.code != ERR_NOERR)

#define SUCCESS_ELSE_RETURN(expr, err, code, retval)                                                                   \
{                                                                                                                      \
        bool result = expr;                                                                                            \
        ERROR_IF(!(result), err, code);                                                                                \
        if (!(result)) { return retval; }                                                                              \
}

#define SUCCESS_ELSE_NULL(expr, err)           SUCCESS_ELSE_RETURN(expr, err, ERR_FAILED, NULL)
#define SUCCESS_ELSE_FAIL(expr, err)           SUCCESS_ELSE_RETURN(expr, err, ERR_FAILED, false)


#define ERROR(err, code)                     ERROR_IF (true, err, code)
#define ERROR_NO_ABORT(err, code)            ERROR_IF (true, err, code)
#define ERROR_IF(expr, err, code)            { if (expr) { error_set(err, code, __FILE__, __LINE__); } }
#define ERROR_IF_AND_RETURN(expr, err, code, retval) \
                                                    { if (expr) { error_set(err, code, __FILE__, __LINE__);            \
                                                                  return retval; } }

#define ERROR_IF_WDETAILS(expr, err, code, msg)            { if (expr) { ERROR_WDETAILS(err, code, msg); } }
#define ERROR_WDETAILS(err, code, msg)                     error_set_wdetails(err, code, __FILE__, __LINE__, msg);

#define ERROR_PRINT(code)                    ERROR_PRINT_IF(true, code)
#define ERROR_PRINT_AND_DIE(code)            { ERROR_PRINT(code); abort(); }
#define ERROR_PRINT_AND_DIE_IF(expr, code)   { if(expr) { ERROR_PRINT_AND_DIE(code) } }
#define ERROR_PRINT_IF(expr, code)                                                                                     \
{                                                                                                                      \
    if (expr) {                                                                                                        \
        struct err err;                                                                                                \
        error_init(&err);                                                                                              \
        ERROR(&err, code);                                                                                             \
        error_print_to_stderr(&err);                                                                                   \
    }                                                                                                                  \
}


END_DECL

#endif
