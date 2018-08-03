// file: sort.h

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

#include <common.h>
#include <stdlib.h>

#ifndef NG5_SORT
#define NG5_SORT

typedef bool (*comp_less_eq)(const void *lhs, const void *rhs);

typedef bool (*comp_eq)(const void *lhs, const void *rhs);

typedef bool (*comp_less)(const void *lhs, const void *rhs);

#define qsort_indices_swap(x, y)                                                        \
{                                                                                       \
    size_t *a = x;                                                                      \
    size_t *b = y;                                                                      \
    size_t tmp = *a;                                                                    \
    *a = *b;                                                                            \
    *b = tmp;                                                                           \
}

#define qsort_indicies_partition(indices, base, width, comp, l, h)                      \
({                                                                                      \
    const void   *x       = base + indices[h] * width;                                  \
    int64_t        i       = (l - 1);                                                   \
                                                                                        \
    for (int64_t j = l; j <= h - 1; j++)                                                \
    {                                                                                   \
        if (comp(base + indices[j] * width, x))                                         \
        {                                                                               \
            i++;                                                                        \
            qsort_indices_swap (indices + i, indices + j);                              \
        }                                                                               \
    }                                                                                   \
    qsort_indices_swap (indices + (i + 1), indices + h);                                \
    (i + 1);                                                                            \
})

unused_fn
static int qsort_indicies(size_t *indices, const void *base, size_t width, comp_less_eq comp, size_t num_elems,
                          struct allocator *alloc)
{
    check_non_null(base);
    check_non_null(alloc);

    int64_t h      = num_elems - 1;
    int64_t *stack = allocator_malloc(alloc, (h + 1) *sizeof(int64_t));
    int64_t top   = -1;
    int64_t l      = 0;

    stack[ ++top ] = l;
    stack[ ++top ] = h;

    while ( top >= 0 )
    {
        h = stack[ top-- ];
        l = stack[ top-- ];

        int64_t p = qsort_indicies_partition(indices, base, width, comp, l, h );

        if ( p - 1 > l ) {
            stack[ ++top ] = l;
            stack[ ++top ] = p - 1;
        }

        if ( p + 1 < h )
        {
            stack[ ++top ] = p + 1;
            stack[ ++top ] = h;
        }
    }

    check_success(allocator_free(alloc, stack));
    return STATUS_OK;
}

unused_fn
static size_t binary_search_indicies(const size_t *indicies, const void *base, size_t width, size_t num_elem, const void *neelde, comp_eq comp_1, comp_less comp_2)
{
    size_t l = 0;
    size_t r = num_elem - 1;
    while (l <= r && r < SIZE_MAX)
    {
        size_t m = l + (r-l)/2;

        // Check if x is present at mid
        if (comp_1(base + indicies[m] * width, neelde))
            return m;

        // If x greater, ignore left half
        if (comp_2(base + indicies[m] * width, neelde))
            l = m + 1;

            // If x is smaller, ignore right half
        else
            r = m - 1;
    }

    // if we reach here, then element was
    // not present
    return num_elem;
}

#endif