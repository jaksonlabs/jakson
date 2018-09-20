// file: bloomfilter.c

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

#include "bloomfilter.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int BloomfilterCreate(Bloomfilter *filter, size_t size)
{
    return BitmapCreate(filter, size);
}

int BloomfilterDrop(Bloomfilter *filter)
{
    return BitmapDrop(filter);
}

int BloomfilterClear(Bloomfilter *filter)
{
    return BitmapClear(filter);
}

size_t BloomfilterNumBits(Bloomfilter *filter)
{
    return BitmapNumBits(filter);
}

unsigned BloomfilterNumHashs()
{
    return 4;
}



