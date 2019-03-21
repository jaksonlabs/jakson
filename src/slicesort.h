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

#ifndef NG5_SLICESORT
#define NG5_SLICESORT

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------
#include "hash.h"
#include "common.h"
#include <limits.h>

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E   F O R W A R D I N G
//
// ---------------------------------------------------------------------------------------------------------------------

// typedef struct SIMDScanOperation SIMDScanOperation;

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------


/**
 * Simple slice sort algorithms that saves moved hashes to mapping array
 * @param keyHashColumn hashes to sort
 * @param length number of hashes to sort
 * @param mapping memorization where the hashes were moved to later find relating stringIds
 */
void selectionSort(Hash arr[], size_t length, Hash mapping[]);


/**
 * Removes the duplicates out of the keyHashColumn array and memorizes duplicates were, save state into extra mapping
 * Optimization: do it in place!
 * @param keyHashColumn hashes to check for duplicates
 * @param arraySize number of hashes
 * @param duplicates array containing the number of duplicates of a hash at the specific index
 * @param keyTargetColumn Target array of new hashes without duplicates
 * @param mapping Mapping to memorize changes
 * @return Number of distinct hashes
 */
size_t removeDuplicates2(Hash keyHashColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[], Hash mapping[]);

/**
 * Recursive Implementation of slice_linearize
 */
void slice_linearizeImpl(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[],
                         Hash duplicates[], size_t factor,
                         size_t level, size_t index, Hash newMapping[], Hash newDuplicates[]);

/**
 * Linearizes the duplicate free hash array into a k-ary search tree
 * Optimization: do it in place!
 * @param compressedColumn Hashes without duplicates
 * @param targetColumn New Array containing the linearized version
 * @param low startIndex
 * @param high endIndex
 * @param mapping mapping of duplicates
 * @param duplicates array containing the duplicates
 * @param factor number of childnodes in the tree: In our case always 5
 * @param newMapping New Mapping for the linearized tree to find key/stringId column
 * @param newDuplicates Changed duplicates array
 */
void slice_linearize(Hash compressedColumn[], Hash targetColumn[], size_t low, size_t high, Hash mapping[],
                     Hash duplicates[], size_t factor, Hash newMapping[], Hash newDuplicates[]);

/**
 * Fills up the compressedHashes array with huge numbers to build up a complete tree
 * @param compressedHashes
 * @param currentLength
 * @param maxLength
 */
void fillUpCompressedArray(Hash compressedHashes[], size_t currentLength, size_t maxLength);

NG5_END_DECL

#endif
