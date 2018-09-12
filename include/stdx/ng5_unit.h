// file: unit.h

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

#ifndef NG5_UNIT
#define NG5_UNIT

#include <ng5_status.h>

NG5_BEGIN_DECL

/**
 * A floating point number between 0 and 1
 */
typedef float unit_t;

#define check_unit(x)                       \
{                                           \
    if (unlikely(x < 0 || x > 1)) {         \
        return STATUS_UNIT_OUTOFBOUNDS;     \
    }                                       \
}

NG5_END_DECL

#endif
