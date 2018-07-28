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
#include <assert.h>
#include <stdatomic.h>

#include "status.h"

#define unused(x)   (void)(x)

#define panic(msg)                \
{                                 \
    fprintf(stderr, "%s", msg);   \
    abort();                      \
}

#define check_non_null(x)         \
{                                 \
    if (!x) {                     \
        return STATUS_NULLPTR;    \
    }                             \
}

#endif