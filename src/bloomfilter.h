// file: bloomfilter.h

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

#ifndef NG5_BLOOMFILTER
#define NG5_BLOOMFILTER

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "bitmap.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef Bitmap Bloomfilter;

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define BLOOMFILTER_SET(filter, key, keySize)              \
({                                                         \
    size_t nbits = BitmapNumBits(filter);                  \
    size_t b0 = HashAdditive(keySize, key) % nbits;       \
    size_t b1 = HashXor(keySize, key) % nbits;           \
    size_t b2 = HashRot(keySize, key) % nbits;            \
    size_t b3 = HashSax(keySize, key) % nbits;            \
    BitmapSet(filter, b0, true);                           \
    BitmapSet(filter, b1, true);                           \
    BitmapSet(filter, b2, true);                           \
    BitmapSet(filter, b3, true);                           \
})

#define BLOOMFILTER_TEST(filter, key, keySize)             \
({                                                         \
    size_t nbits = BitmapNumBits(filter);                  \
    size_t b0 = HashAdditive(keySize, key) % nbits;       \
    size_t b1 = HashXor(keySize, key) % nbits;           \
    size_t b2 = HashRot(keySize, key) % nbits;            \
    size_t b3 = HashSax(keySize, key) % nbits;            \
    bool b0set = BitmapGet(filter, b0);                    \
    bool b1set = BitmapGet(filter, b1);                    \
    bool b2set = BitmapGet(filter, b2);                    \
    bool b3set = BitmapGet(filter, b3);                    \
    (b0set && b1set && b2set && b3set);                    \
})

#define BLOOMFILTER_TEST_AND_SET(filter, key, keySize)     \
({                                                         \
    size_t nbits = BitmapNumBits(filter);                  \
    size_t b0 = HashAdditive(keySize, key) % nbits;       \
    size_t b1 = HashXor(keySize, key) % nbits;           \
    size_t b2 = HashRot(keySize, key) % nbits;            \
    size_t b3 = HashSax(keySize, key) % nbits;            \
    bool b0set = BitmapGet(filter, b0);                    \
    bool b1set = BitmapGet(filter, b1);                    \
    bool b2set = BitmapGet(filter, b2);                    \
    bool b3set = BitmapGet(filter, b3);                    \
    BitmapSet(filter, b0, true);                           \
    BitmapSet(filter, b1, true);                           \
    BitmapSet(filter, b2, true);                           \
    BitmapSet(filter, b3, true);                           \
    (b0set && b1set && b2set && b3set);                    \
})

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int BloomfilterCreate(Bloomfilter *filter, size_t size);

int BloomfilterDrop(Bloomfilter *filter);

int BloomfilterClear(Bloomfilter *filter);

size_t BloomfilterNumBits(Bloomfilter *filter);

unsigned BloomfilterNumHashs();

NG5_END_DECL

#endif