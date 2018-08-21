// file: stdinc.h

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

#ifndef _NG5_STDINC
#define _NG5_STDINC

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>

#include "status.h"

#define unused(x)   (void)(x)

#define of_type(x) /* a convenience way to write types for generic containers; no effect than just a visual one */

#define optional /* parameters to functions marked with this tag can be NULL and will be ignored */

#define max(a,b)                  \
    ((b) > (a) ? (b) : (a))

#define cast(type, name, src)     \
      type name = (type) src

#define likely(x)                 \
    __builtin_expect((x), 1)
#define unlikely(x)               \
    __builtin_expect((x), 0)

#define panic(msg)                \
{                                 \
    fprintf(stderr, "%s", msg);   \
    abort();                      \
}

#define panic_if(expr, msg)                   \
{                                             \
    if (unlikely((expr))) {                   \
        panic(msg)                            \
    };                                        \
}

#ifndef NDEBUG
#define check_tag(is, expected)    \
{                                  \
    if (is != expected) {          \
        return STATUS_ILLEGALIMPL; \
    }                              \
}
#else
#define check_tag(is, expected) { }
#endif

#ifndef NDEBUG
#define check_non_null(x)         \
{                                 \
    if (!x) {                     \
        return STATUS_NULLPTR;    \
    }                             \
}
#else
#define check_non_null(x) { }
#endif

#ifndef NDEBUG
#define check_larger_one(x)          \
{                                    \
    if (x <= 1) {                    \
        return STATUS_ILLEGALARG;    \
    }                                \
}
#else
#define check_larger_one(x) { }
#endif

#define prefetch_read(adr)          \
    __builtin_prefetch(adr, 0, 3)

#define prefetch_write(adr)         \
    __builtin_prefetch(adr, 1, 3)

#define check_success(x)            \
{                                   \
    if (unlikely(x != STATUS_OK)) { \
        return x;                   \
    }                               \
}

#define NOT_YET_IMPLEMENTED         \
{                                   \
    abort();                        \
};

#define force_inline                \
    __attribute__((always_inline))

#define unused_fn                   \
    __attribute__((unused))

#define optional_set(x, y)          \
    if (x) {                        \
        *x = y;                     \
    }

#define optional_set_else(x, y, stmt)   \
    if (x) {                            \
        *x = y;                         \
    } else { stmt; }

#endif

#ifndef NDEBUG
#define debug(msg, ...)                 \
{                                       \
    fprintf(stderr, msg, __VA_ARGS__);  \
    fprintf(stderr, "\n");              \
    fflush(stderr);                     \
}
#else
#define debug(msg, ...)                 \
{ }
#endif


#ifndef thread_local
#define thread_local __thread
#endif

typedef size_t string_id_t;