// file: misc.h

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "stdlib.h"
#include "alloc.h"

NG5_BEGIN_DECL

#ifndef NG5_MISC
#define NG5_MISC

typedef bool (*CompareLessEqFunction)(const void *lhs, const void *rhs);

typedef bool (*CompareEqFunction)(const void *lhs, const void *rhs);

typedef bool (*CompareLessFunction)(const void *lhs, const void *rhs);

#define NG5_QSORT_INDICES_SWAP(x, y)                                                                                   \
{                                                                                                                      \
    size_t *a = x;                                                                                                     \
    size_t *b = y;                                                                                                     \
    size_t tmp = *a;                                                                                                   \
    *a = *b;                                                                                                           \
    *b = tmp;                                                                                                          \
}

#define NG5_QSORT_INDICIES_PARTITION(indices, base, width, comp, l, h)                                                 \
({                                                                                                                     \
    const void   *x       = base + indices[h] * width;                                                                 \
    int64_t        i       = (l - 1);                                                                                  \
                                                                                                                       \
    for (int64_t j = l; j <= h - 1; j++)                                                                               \
    {                                                                                                                  \
        if (comp(base + indices[j] * width, x))                                                                        \
        {                                                                                                              \
            i++;                                                                                                       \
            NG5_QSORT_INDICES_SWAP (indices + i, indices + j);                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    NG5_QSORT_INDICES_SWAP (indices + (i + 1), indices + h);                                                           \
    (i + 1);                                                                                                           \
})

NG5_LIB_FUNCTION
static int QSortIndicies(size_t *indices, const void *base, size_t width, CompareLessEqFunction comp, size_t numElems,
                         Allocator *alloc)
{
    CHECK_NON_NULL(base);
    CHECK_NON_NULL(alloc);

    int64_t h      = numElems - 1;
    int64_t *stack = AllocatorMalloc(alloc, (h + 1) *sizeof(int64_t));
    int64_t top   = -1;
    int64_t l      = 0;

    stack[ ++top ] = l;
    stack[ ++top ] = h;

    while ( top >= 0 )
    {
        h = stack[ top-- ];
        l = stack[ top-- ];

        int64_t p = NG5_QSORT_INDICIES_PARTITION(indices, base, width, comp, l, h );

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

    CHECK_SUCCESS(AllocatorFree(alloc, stack));
    return STATUS_OK;
}

NG5_LIB_FUNCTION
static size_t BinarySearchIndicies(const size_t *indicies, const void *base, size_t width, size_t numElem,
                                   const void *neelde, CompareEqFunction compEq, CompareLessFunction compLess)
{
    size_t l = 0;
    size_t r = numElem - 1;
    while (l <= r && r < SIZE_MAX)
    {
        size_t m = l + (r-l)/2;

        // Check if x is present at mid
        if (compEq(base + indicies[m] * width, neelde))
            return m;

        // If x greater, ignore left half
        if (compLess(base + indicies[m] * width, neelde))
            l = m + 1;

            // If x is smaller, ignore right half
        else
            r = m - 1;
    }

    // if we reach here, then element was
    // not present
    return numElem;
}

NG5_LIB_FUNCTION
static size_t GetMinimum(const size_t *elements, size_t numElements)
{
    size_t min = (size_t) -1;
    while (numElements--) {
        min = min < *elements ? min : *elements;
        elements++;
    }
    return min;
}

NG5_LIB_FUNCTION
static size_t GetMaximum(const size_t *elements, size_t numElements)
{
    size_t max = 0;
    while (numElements--) {
        max = max > *elements ? max : *elements;
        elements++;
    }
    return max;
}

NG5_LIB_FUNCTION
static double GetSum(const size_t *elements, size_t numElements)
{
    double sum = 0;
    while (numElements--) {
        sum += *elements;
        elements++;
    }
    return sum;
}

NG5_LIB_FUNCTION
static double GetAvg(const size_t *elements, size_t numElements)
{
    return GetSum(elements, numElements) / (double) numElements;
}

NG5_END_DECL

#endif