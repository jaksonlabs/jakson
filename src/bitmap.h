// file: bitmap.h

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

#ifndef NG5_BITMAP
#define NG5_BITMAP

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "vector.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct Bitmap
{
    Vector ofType(uint64_t) data;
    uint16_t numBits;
} Bitmap;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int BitmapCreate(Bitmap *bitmap, uint16_t numBits);

int BitmapDrop(Bitmap *bitmap);

size_t BitmapNumBits(const Bitmap *bitmap);

int BitmapClear(Bitmap *bitmap);

int BitmapSet(Bitmap *bitmap, uint16_t bitPosition, bool on);

bool BitmapGet(Bitmap *bitmap, uint16_t bitPosition);

NG5_END_DECL

#endif