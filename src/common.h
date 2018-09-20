// file: common.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NG5_COMMON
#define NG5_COMMON

#ifdef __cplusplus
#define NG5_BEGIN_DECL  extern "C" {
#define NG5_END_DECL    }
#else
#define NG5_BEGIN_DECL
#define NG5_END_DECL
#endif

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "config_static.h"
#include "status.h"

#define UNUSED(x)   (void)(x)

#define ofType(x) /* a convenience way to write types for generic containers; no effect than just a visual one */

#define optional /* parameters to functions marked with this tag can be NULL and will be ignored */

#define max(a, b)                                                                                                      \
    ((b) > (a) ? (b) : (a))

#define min(a, b)                                                                                                      \
    ((a) < (b) ? (a) : (b))

#define CAST(type, name, src)                                                                                          \
      type name = (type) src

#define BRANCH_LIKELY(x)                                                                                               \
    __builtin_expect((x), 1)
#define BRANCH_UNLIKELY(x)                                                                                             \
    __builtin_expect((x), 0)

#define PANIC(msg)                                                                                                     \
{                                                                                                                      \
    fprintf(stderr, "%s", msg);                                                                                        \
    abort();                                                                                                           \
}

#define PANIC_WARGS(msg, ...)                                                                                          \
{                                                                                                                      \
    fprintf(stderr, msg, __VA_ARGS__);                                                                                 \
    abort();                                                                                                           \
}

#define PANIC_IF(expr, msg)                                                                                            \
{                                                                                                                      \
    if (BRANCH_UNLIKELY((expr))) {                                                                                     \
        PANIC(msg)                                                                                                     \
    };                                                                                                                 \
}

#define PANIC_IF_WARGS(expr, msg, ...)                                                                                 \
{                                                                                                                      \
    if (BRANCH_UNLIKELY((expr))) {                                                                                     \
        PANIC_WARGS(msg, __VA_ARGS__)                                                                                  \
    };                                                                                                                 \
}

#ifndef NDEBUG
#define CHECK_TAG(is, expected)                                                                                        \
{                                                                                                                      \
    if (is != expected) {                                                                                              \
        return STATUS_ILLEGALIMPL;                                                                                     \
    }                                                                                                                  \
}
#else
#define CHECK_TAG(is, expected) { }
#endif

#ifndef NDEBUG
#define CHECK_NON_NULL(x)                                                                                              \
{                                                                                                                      \
    if (!x) {                                                                                                          \
        return STATUS_NULLPTR;                                                                                         \
    }                                                                                                                  \
}
#else
#define CHECK_NON_NULL(x) { }
#endif

#ifndef NDEBUG
#define CHECK_LARGER_ONE(x)                                                                                            \
{                                                                                                                      \
    if (x <= 1) {                                                                                                      \
        return STATUS_ILLEGALARG;                                                                                      \
    }                                                                                                                  \
}
#else
#define CHECK_LARGER_ONE(x) { }
#endif

#define PREFETCH_READ(adr)                                                                                             \
    __builtin_prefetch(adr, 0, 3)

#define PREFETCH_WRITE(adr)                                                                                            \
    __builtin_prefetch(adr, 1, 3)

#define CHECK_SUCCESS(x)                                                                                               \
{                                                                                                                      \
    if (BRANCH_UNLIKELY(x != STATUS_OK)) {                                                                             \
        return x;                                                                                                      \
    }                                                                                                                  \
}

#define NOT_YET_IMPLEMENTED                                                                                            \
{                                                                                                                      \
    abort();                                                                                                           \
};

#define FORCE_INLINE                                                                                                   \
    __attribute__((always_inline))

#define UNUSED_FUNCTION                                                                                                \
    __attribute__((unused))

#define OPTIONAL_SET(x, y)                                                                                             \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    }

#define OPTIONAL_SET_OR_ELSE(x, y, stmt)                                                                               \
    if (x) {                                                                                                           \
        *x = y;                                                                                                        \
    } else { stmt; }

#endif

#ifdef NLOG_TRACE
#define TRACE(tag, msg, ...) { }
#else
#define TRACE(tag, msg, ...)                                                                                           \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [TRACE   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#ifdef NLOG_INFO
#define INFO(tag, msg, ...) { }
#else
#define INFO(tag, msg, ...)                                                                                            \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [INFO    : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#endif

#ifndef NDEBUG
#define DEBUG(tag, msg, ...)                                                                                           \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [DEBUG   : %-10s] %s\n", getpid(), tag, msg);                                              \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}
#else
#define DEBUG(tag, msg, ...)                                                                                           \
{ }
#endif

#define ZERO_MEMORY(dst, len)                                                                                          \
    memset((void *) dst, 0, len);

#define WARN(tag, msg, ...)                                                                                            \
{                                                                                                                      \
    char buffer[1024];                                                                                                 \
    sprintf(buffer, "--%d-- [WARNING: %-10s] %s\n", getpid(), tag, msg);                                               \
    fprintf(stderr, buffer, __VA_ARGS__);                                                                              \
    fflush(stderr);                                                                                                    \
}

#ifndef thread_local
#define thread_local __thread
#endif

#define ARRAY_LENGTH_OF(x)                                                                                             \
    sizeof(x)/sizeof(x[0])

typedef size_t StringId;

typedef char Byte;