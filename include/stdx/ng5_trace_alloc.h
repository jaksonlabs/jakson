// file: ng5_trace_alloc.h

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

#ifndef NG5_TRACE_ALLOC
#define NG5_TRACE_ALLOC

#include <stdx/ng5_allocator.h>

NG5_BEGIN_DECL

/**
 * Returns standard c-lib allocator (malloc, realloc, free) that collects some statistics
 * for inspection purposes. Generally, this implementation is slow and should not be used
 * in productive mode
 *
 * @param alloc must be non-null
 * @return STATUS_OK in case of non-null parameter alloc, STATUS_NULLPTR otherwise
 */
int allocator_trace(ng5_allocator_t *alloc);

NG5_END_DECL

#endif
