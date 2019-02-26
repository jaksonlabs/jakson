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


void slicesort2(Hash keyHashColumn[], const char * keyColumn[], const char * stringIdColumn[], size_t arraySize);
void slicesort3(Hash keyHashColumn[], size_t length, Hash mapping[]);
size_t removeDuplicates(Hash keyHashColumn[], size_t arraySize, size_t duplicates[], Hash keyTargetColumn[]);

NG5_END_DECL

#endif