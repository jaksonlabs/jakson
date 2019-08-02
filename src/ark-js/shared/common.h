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

#ifndef ARK_COMMON_H
#define ARK_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <assert.h>
#include <inttypes.h>

#ifdef __cplusplus
#define ARK_BEGIN_DECL  extern "C" {
#define ARK_END_DECL    }
#else
#define ARK_BEGIN_DECL
#define ARK_END_DECL
#endif

#ifndef ARK_EXPORT
#ifndef ARK_STATIC
#ifdef _WIN32
#define ARK_EXPORT(x) __declspec(dllimport) x
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ARK_EXPORT(x) __attribute__((visibility("default"))) x
#else
#define ARK_EXPORT(x) x
#endif
#else
#define ARK_EXPORT(x) x
#endif
#endif

#define ARK_QUERY_LIMIT_NONE -1
#define ARK_QUERY_LIMIT_1     1

#define ARK_ARRAY_LENGTH(x)                                                                                            \
    sizeof(x)/sizeof(x[0])

typedef uint64_t offset_t;
typedef int64_t signed_offset_t;

typedef unsigned char u_char;

typedef enum field_type {
        FIELD_NULL = 0,
        FIELD_BOOLEAN = 1,
        FIELD_INT8 = 2,
        FIELD_INT16 = 3,
        FIELD_INT32 = 4,
        FIELD_INT64 = 5,
        FIELD_UINT8 = 6,
        FIELD_UINT16 = 7,
        FIELD_UINT32 = 8,
        FIELD_UINT64 = 9,
        FIELD_FLOAT = 10,
        FIELD_STRING = 11,
        FIELD_OBJECT = 12
} field_e;

enum access_mode {
        READ_WRITE,
        READ_ONLY
};

#define ark_func_unused __attribute__((unused))

ark_func_unused static const char *basic_type_to_json_type_str(enum field_type t)
{
        switch (t) {
        case FIELD_INT8:
        case FIELD_INT16:
        case FIELD_INT32:
        case FIELD_INT64:
        case FIELD_UINT8:
        case FIELD_UINT16:
        case FIELD_UINT32:
        case FIELD_UINT64:
                return "integer";
        case FIELD_FLOAT:
                return "float";
        case FIELD_STRING:
                return "string";
        case FIELD_BOOLEAN:
                return "boolean";
        case FIELD_NULL:
                return "null";
        case FIELD_OBJECT:
                return "object";
        default:
                return "(unknown)";
        }
}

ark_func_unused static const char *basic_type_to_system_type_str(enum field_type t)
{
        switch (t) {
        case FIELD_INT8:
                return "int8";
        case FIELD_INT16:
                return "int16";
        case FIELD_INT32:
                return "int32";
        case FIELD_INT64:
                return "int64";
        case FIELD_UINT8:
                return "uint8";
        case FIELD_UINT16:
                return "uint16";
        case FIELD_UINT32:
                return "uint32";
        case FIELD_UINT64:
                return "uint64";
        case FIELD_FLOAT:
                return "float32";
        case FIELD_STRING:
                return "string64";
        case FIELD_BOOLEAN:
                return "bool8";
        case FIELD_NULL:
                return "void";
        case FIELD_OBJECT:
                return "variable";
        default:
                return "(unknown)";
        }
}

#define ARK_NOT_IMPLEMENTED                                                                                            \
{                                                                                                                      \
    struct err err;                                                                                                    \
    error_init(&err);                                                                                                  \
    error(&err, ARK_ERR_NOTIMPLEMENTED)                                                                                \
    error_print_and_abort(&err);                                                                                       \
    return false;                                                                                                      \
};

#ifndef NDEBUG
#define ark_check_tag(is, expected)                                                                                 \
{                                                                                                                      \
    if (is != expected) {                                                                                              \
        error_print(ARK_ERR_ERRINTERNAL)                                                                     \
        return false;                                                                                                  \
    }                                                                                                                  \
}
#else
#define ark_check_tag(is, expected) { }
#endif

#if !defined(ARK_LOG_TRACE) || defined(NDEBUG)
#define ark_trace(tag, msg, ...) { }
#else
#define ark_trace(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [TRACE   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(ARK_LOG_INFO) || defined(NDEBUG)
#define ark_info(tag, msg, ...) { }
#else
#define ark_info(tag, msg, ...)                                                                                     \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [INFO    : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(ARK_LOG_DEBUG) || defined(NDEBUG)
#define ark_debug(tag, msg, ...)                                                                                       \
{ }
#else
#define ark_debug(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [DEBUG   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#if !defined(ARK_LOG_WARN) || defined(NDEBUG)
#define ark_warn(tag, msg, ...) { }
#else
#define ark_warn(tag, msg, ...)                                                                                     \
    {                                                                                                                  \
        char buffer[1024];                                                                                             \
        sprintf(buffer, "--%d-- [WARNING: %-10s] %s\n", getpid(), tag, msg);                                           \
        fprintf(stderr, buffer, __VA_ARGS__);                                                                          \
        fflush(stderr);                                                                                                \
    }
#endif

#define CARBON_ARCHIVE_MAGIC                "MP/CARBON"
#define CARBON_ARCHIVE_VERSION               1

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
#define  MARKER_SYMBOL_HASHTABLE_HEADER    '#'
#define  MARKER_SYMBOL_VECTOR_HEADER       '|'

#define ark_zero_memory(dst, len)                                                                                      \
    memset((void *) dst, 0, len);

#define ark_cast(type, name, src)                                                                                      \
      type name = (type) src

#define unused(x)   (void)(x);

#define ARK_BUILT_IN(x)   ark_func_unused x

#define ofType(x) /** a convenience way to write types for generic containers; no effect than just a visual one */
#define ofMapping(x, y) /** a convenience way to write types for generic containers; no effect than just a visual one */

#define ark_optional_call(x, func, ...) if((x) && (x)->func) { (x)->func(__VA_ARGS__); }

#define ark_max(a, b)                                                                                                  \
    ((b) > (a) ? (b) : (a))

#define ark_min(a, b)                                                                                                  \
    ((a) < (b) ? (a) : (b))

#define error_if_null(x)                                                                                               \
{                                                                                                                      \
    if (!(x)) {                                                                                                        \
        struct err err;                                                                                                \
        error_init(&err);                                                                                              \
        error(&err, ARK_ERR_NULLPTR);                                                                                  \
        error_print_to_stderr(&err);                                                                                   \
        return false;                                                                                                  \
    }                                                                                                                  \
}

#define ark_check_success(x)                                                                                           \
{                                                                                                                      \
    if (unlikely(!x)) {                                                                                                \
        return x;                                                                                                      \
    }                                                                                                                  \
}

#define ark_success_or_jump(expr, label)                                                                               \
{                                                                                                                      \
    if (unlikely(!expr)) {                                                                                             \
        goto label;                                                                                                    \
    }                                                                                                                  \
}

#define likely(x)                                                                                                      \
    __builtin_expect((x), 1)
#define unlikely(x)                                                                                                    \
    __builtin_expect((x), 0)

#define prefetch_read(adr)                                                                                             \
    __builtin_prefetch(adr, 0, 3)

#define prefetch_write(adr)                                                                                            \
    __builtin_prefetch(adr, 1, 3)

#define ARK_FORWARD_STRUCT_DECL(x) struct x;

#define ark_bit_num_of(x)             (sizeof(x) * 8)
#define ark_set_bit(n)                ( ((u32) 1) << (n) )
#define ark_set_bits(x, mask)         ( x |=  (mask) )
#define ark_unset_bits(x, mask)       ( x &= ~(mask) )
#define ark_are_bits_set(mask, bit)   (((bit) & mask ) == (bit))

#define ark_implemented_or_error(err, x, func)                                                                         \
    ark_optional(x->func == NULL, error(err, ARK_ERR_NOTIMPLEMENTED))

#define ark_optional(expr, stmt)                                                                                       \
    if (expr) { stmt; }

#define ark_optional_set(x, y)                                                                                         \
     ark_optional(x, *x = y)

#define ark_optional_set_or_else(x, y, stmt)                                                                           \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    } else { stmt; }

bool GlobalEnableConsoleOutput;

#define ARK_CONSOLE_OUTPUT_ON()                                                                                        \
    GlobalEnableConsoleOutput = true;

#define ARK_CONSOLE_OUTPUT_OFF()                                                                                       \
    GlobalEnableConsoleOutput = false;

#define ARK_CONSOLE_WRITE(file, msg, ...)                                                                              \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        pid_t pid = getpid();                                                                                          \
        char timeBuffer[2048];                                                                                         \
        char formatBuffer[2048];                                                                                       \
        time_t now = time (0);                                                                                         \
        fflush(file);                                                                                                  \
        strftime (timeBuffer, 2048, "%Y-%m-%d %H:%M:%S", localtime (&now));                                            \
        sprintf (formatBuffer, msg, __VA_ARGS__);                                                                      \
        fprintf(file, "[%d] %s   %-70s", pid, timeBuffer, formatBuffer);                                               \
        fflush(file);                                                                                                  \
    }                                                                                                                  \
}

#define ARK_CONSOLE_WRITE_ENDL(file)                                                                                   \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        fprintf(file, "\n");                                                                                           \
    }                                                                                                                  \
}

#define ARK_CONSOLE_WRITE_CONT(file, msg, ...)                                                                         \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        fprintf(file, msg, __VA_ARGS__);                                                                               \
    }                                                                                                                  \
}

#define ARK_CONSOLE_WRITELN(file, msg, ...)                                                                            \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        ARK_CONSOLE_WRITE(file, msg, __VA_ARGS__)                                                                      \
        ARK_CONSOLE_WRITE_ENDL(file)                                                                                   \
        fflush(file);                                                                                                  \
    }                                                                                                                  \
}

#endif
