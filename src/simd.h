// file: simd.h

/**
 *  Copyright (C) 2018 Marten Wallewein, Marcus Pinnecke
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

#ifndef NG5_SIMD
#define NG5_SIMD

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "immintrin.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E   F O R W A R D I N G
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct SIMDScanOperation SIMDScanOperation;

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef NG5_SIMD_SIZEOF_SIZET
#define NG5_SIMD_SIZEOF_SIZET sizeof(size_t)
#endif

#ifndef NG5_SIMD_COMPARE_ELEMENT_COUNT
#define NG5_SIMD_COMPARE_ELEMENT_COUNT sizeof(__m256i) / NG5_SIMD_SIZEOF_SIZET
#endif

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct SIMDScanOperation {
    uint32_t matchIndex;
    uint32_t currentIndex;
    size_t* data;
    size_t searchValue;
    __m256i* replicatedSearchValue;
    bool endReached;
    size_t elementCount;
} SIMDScanOperation;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int SIMDScanPrepare(SIMDScanOperation* scanOperation);

int SIMDScanFree(SIMDScanOperation* scanOperation);

int SIMDScanExecuteSingleOperation(SIMDScanOperation* scanOperation);

NG5_END_DECL

#endif