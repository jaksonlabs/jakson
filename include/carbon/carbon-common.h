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

#ifndef CARBON_COMMON_H
#define CARBON_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
#define CARBON_BEGIN_DECL  extern "C" {
#define CARBON_END_DECL    }
#else
#define CARBON_BEGIN_DECL
#define CARBON_END_DECL
#endif

#ifndef CARBON_EXPORT
#ifndef CARBON_STATIC
#ifdef _WIN32
#define CARBON_EXPORT(x) __declspec(dllimport) x
#elif defined(__GNUC__) && __GNUC__ >= 4
#define CARBON_EXPORT(x) __attribute__((visibility("default"))) x
#else
#define CARBON_EXPORT(x) x
#endif
#else
#define CARBON_EXPORT(x) x
#endif
#endif

#define CARBON_ARRAY_LENGTH(x)                                                                                         \
    sizeof(x)/sizeof(x[0])

typedef char            carbon_byte_t;

typedef size_t          carbon_off_t;

typedef unsigned char   u_char;

typedef enum carbon_type
{
    CARBON_TYPE_INT8,
    CARBON_TYPE_INT16,
    CARBON_TYPE_INT32,
    CARBON_TYPE_INT64,
    CARBON_TYPE_UINT8,
    CARBON_TYPE_UINT16,
    CARBON_TYPE_UINT32,
    CARBON_TYPE_UINT64,
    CARBON_TYPE_FLOAT,
    CARBON_TYPE_STRING,
    CARBON_TYPE_BOOL,
    CARBON_TYPE_VOID,
    CARBON_TYPE_OBJECT,
} carbon_type_e;

#define CARBON_NOT_IMPLEMENTED                                                                                         \
{                                                                                                                      \
    carbon_err_t err;                                                                                                  \
    carbon_error_init(&err);                                                                                           \
    CARBON_ERROR(&err, CARBON_ERR_NOTIMPLEMENTED)                                                                      \
    carbon_error_print_and_abort(&err);                                                                                \
    return false;                                                                                                      \
};

#ifndef NDEBUG
#define CARBON_CHECK_TAG(is, expected)                                                                                 \
{                                                                                                                      \
    if (is != expected) {                                                                                              \
        CARBON_PRINT_ERROR(STATUS_ILLEGALIMPL)                                                                         \
        return false;                                                                                                  \
    }                                                                                                                  \
}
#else
#define CARBON_CHECK_TAG(is, expected) { }
#endif


#ifdef NLOG_TRACE
#define CARBON_TRACE(tag, msg, ...) { }
#else
#define CARBON_TRACE(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [TRACE   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#ifdef NLOG_INFO
#define CARBON_INFO(tag, msg, ...) { }
#else
#define CARBON_INFO(tag, msg, ...)                                                                                     \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [INFO    : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#ifndef NDEBUG
#define CARBON_DEBUG(tag, msg, ...)                                                                                    \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [DEBUG   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#else
#define CARBON_DEBUG(tag, msg, ...)                                                                                    \
{ }
#endif

#define CARBON_ZERO_MEMORY(dst, len)                                                                                   \
    memset((void *) dst, 0, len);

#ifdef NLOG_WARN
    #define CARBON_WARN(tag, msg, ...) { }
#else
    #define CARBON_WARN(tag, msg, ...)                                                                                 \
    {                                                                                                                  \
        char buffer[1024];                                                                                             \
        sprintf(buffer, "--%d-- [WARNING: %-10s] %s\n", getpid(), tag, msg);                                           \
        fprintf(stderr, buffer, __VA_ARGS__);                                                                          \
        fflush(stderr);                                                                                                \
    }
#endif

#define CARBON_CAST(type, name, src)                                                                                   \
      type name = (type) src

#define CARBON_UNUSED(x)   (void)(x)

#define CARBON_FUNC_UNUSED __attribute__((unused))

#define ofType(x) /** a convenience way to write types for generic containers; no effect than just a visual one */

#define CARBON_NULLABLE /** parameters to functions marked with this tag can be NULL and will be ignored; is attached
                    to a return value (typically a pointer), the this means the function may return NULL. */

#define CARBON_MAX(a, b)                                                                                               \
    ((b) > (a) ? (b) : (a))

#define CARBON_MIN(a, b)                                                                                               \
    ((a) < (b) ? (a) : (b))

#define CARBON_NON_NULL_OR_ERROR(x)                                                                                    \
{                                                                                                                      \
    if (!(x)) {                                                                                                        \
        carbon_err_t err;                                                                                              \
        carbon_error_init(&err);                                                                                       \
        CARBON_ERROR(&err, CARBON_ERR_NULLPTR);                                                                        \
        carbon_error_print(&err);                                                                                      \
        return false;                                                                                                  \
    }                                                                                                                  \
}


#define CARBON_CHECK_SUCCESS(x)                                                                                        \
{                                                                                                                      \
    if (CARBON_BRANCH_UNLIKELY(!x)) {                                                                                  \
        return x;                                                                                                      \
    }                                                                                                                  \
}

#define CARBON_BRANCH_LIKELY(x)                                                                                        \
    __builtin_expect((x), 1)
#define CARBON_BRANCH_UNLIKELY(x)                                                                                      \
    __builtin_expect((x), 0)

#define CARBON_PREFETCH_READ(adr)                                                                                      \
    __builtin_prefetch(adr, 0, 3)

#define CARBON_PREFETCH_WRITE(adr)                                                                                     \
    __builtin_prefetch(adr, 1, 3)

#define CARBON_FORWARD_STRUCT_DECL(x) struct x;

#define CARBON_NUM_BITS(x)             (sizeof(x) * 8)
#define CARBON_SET_BIT(n)              ( ((uint32_t) 1) << (n) )
#define CARBON_FIELD_SET(x, mask)      ( x |=  (mask) )
#define CARBON_FIELD_CLEAR(x, mask)    ( x &= ~(mask) )

#define CARBON_OPTIONAL_SET(x, y)                                                                                      \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    }

#define CARBON_OPTIONAL_SET_OR_ELSE(x, y, stmt)                                                                        \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    } else { stmt; }

bool GlobalEnableConsoleOutput;

#define CARBON_CONSOLE_OUTPUT_ON()                                                                                     \
    GlobalEnableConsoleOutput = true;

#define CARBON_CONSOLE_OUTPUT_OFF()                                                                                    \
    GlobalEnableConsoleOutput = false;

#define CARBON_CONSOLE_WRITE(file, msg, ...)                                                                           \
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

#define CARBON_CONSOLE_WRITE_ENDL(file)                                                                                \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        fprintf(file, "\n");                                                                                           \
    }                                                                                                                  \
}

#define CARBON_CONSOLE_WRITE_CONT(file, msg, ...)                                                                      \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        fprintf(file, msg, __VA_ARGS__);                                                                               \
    }                                                                                                                  \
}

#define CARBON_CONSOLE_WRITELN(file, msg, ...)                                                                         \
{                                                                                                                      \
    if (GlobalEnableConsoleOutput) {                                                                                   \
        CARBON_CONSOLE_WRITE(file, msg, __VA_ARGS__)                                                                   \
        CARBON_CONSOLE_WRITE_ENDL(file)                                                                                \
    }                                                                                                                  \
}

#endif
