// file: simd.c

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "simd.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------
/*#ifndef NG5_SIMD_GET_COMPARE_OP
#define NG5_SIMD_GET_COMPARE_OP (compareElementCount) \
({                                                      \
    switch (compareElementCount) {                      \
        case 8:                                         \
            _mm256_cmpeq_epi8;                          \
        case 16:                                        \
            _mm256_cmpeq_epi16;                         \
        case 32:                                        \
            _mm256_cmpeq_epi32;                         \
        case 64:                                        \
            ;                         \
    }                                                       \
)}                                                   \

#endif */

// TODO: Futher investigation, can size_t be smaller?
#ifndef NG5_SIMD_LOAD
#define NG5_SIMD_LOAD _mm256_load_si256
#endif 

#ifndef NG5_SIMD_COMPARE_EQUALS
#define NG5_SIMD_COMPARE_EQUALS _mm256_cmpeq_epi64
#endif


// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------
inline static size_t* replicateSearchValue(size_t searchValue) {
    size_t* searchValueArray = malloc(NG5_SIMD_COMPARE_ELEMENT_COUNT * NG5_SIMD_SIZEOF_SIZET);
    uint8_t i = 0;

    for(i = 0; i < NG5_SIMD_COMPARE_ELEMENT_COUNT; ++i) {
        searchValueArray[0] = searchValue;
    }

    return searchValueArray;
}

// Returns position of the only set bit in 'n' 
/*nline static int findPosition(unsigned n) { 
  
    unsigned i = 1, pos = 1; 
  
    // Iterate through bits of n till we find a set bit 
    // i&n will be non-zero only when 'i' and 'n' have a set bit 
    // at same position 
    while (!(i & n)) 
    { 
        // Unset current bit and set the next bit in 'i' 
        i = i << 1; 
  
        // increment position 
        ++pos; 
    } 
  
    return pos; 
} */

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

inline int SIMDScanPrepare(SIMDScanOperation* scanOperation) {
    size_t* replicatedSearchValue = replicateSearchValue(scanOperation->searchValue);
    scanOperation->replicatedSearchValue = (__m256i *)replicatedSearchValue;
    scanOperation->currentIndex = 0;
    scanOperation->matchIndex = 0;
    scanOperation->endReached = 0;

    // TODO: Success/Error?
    return 1;
}

inline int SIMDScanExecuteSingleOperation(SIMDScanOperation* scanOperation) {

    // Check for end of list    
    size_t restElements = NG5_SIMD_COMPARE_ELEMENT_COUNT - (scanOperation->elementCount - scanOperation->currentIndex);

    // If there is a positive rest of elements, that are smaller than the simd size
    // Move the pointer back to compare the correct amount of elements
    // Compares the last x < NG5_SIMD_COMPARE_ELEMENT_COUNT twice, but the other solution
    // Would be to fill up the remaining slots with some static value like MAXINT
    size_t* currentSearchData = restElements > 0 ? 
        scanOperation->data + scanOperation->currentIndex - restElements
        : scanOperation->data + scanOperation->currentIndex;

    __m256i simdSearchData = NG5_SIMD_LOAD((__m256i *)currentSearchData);
    __m256i simdSearchValue = NG5_SIMD_LOAD(scanOperation->replicatedSearchValue);
    __m256i compareResult = NG5_SIMD_COMPARE_EQUALS(simdSearchData, simdSearchValue);
    
    // Check if the result is empty
    if(!_mm256_testz_si256(compareResult, compareResult)) {
        unsigned bitmask = _mm256_movemask_epi8(compareResult);
        scanOperation->matchIndex = scanOperation->currentIndex + _bit_scan_forward(bitmask);
    }
    scanOperation->currentIndex += NG5_SIMD_COMPARE_ELEMENT_COUNT;

    return (scanOperation->matchIndex != 0);
}

inline int SIMDScanFree(SIMDScanOperation* scanOperation) {
    free(scanOperation->replicatedSearchValue);
    free(scanOperation);

    return 1;
}