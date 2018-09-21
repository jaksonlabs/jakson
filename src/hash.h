// file: hash.h

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


#ifndef NG5_HASH
#define NG5_HASH

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"

NG5_BEGIN_DECL

typedef size_t Hash;

// ---------------------------------------------------------------------------------------------------------------------
// J E N K I N S   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define JENKINS_MIX(a, b, c)                                                                                           \
{                                                                                                                      \
    a -= b; a -= c; a ^= (c >> 13);                                                                                    \
    b -= c; b -= a; b ^= (a << 8);                                                                                     \
    c -= a; c -= b; c ^= (b >> 13);                                                                                    \
    a -= b; a -= c; a ^= (c >> 12);                                                                                    \
    b -= c; b -= a; b ^= (a << 16);                                                                                    \
    c -= a; c -= b; c ^= (b >> 5);                                                                                     \
    a -= b; a -= c; a ^= (c >> 3);                                                                                     \
    b -= c; b -= a; b ^= (a << 10);                                                                                    \
    c -= a; c -= b; c ^= (b >> 15);                                                                                    \
}

/* implements: Hash hash_jenkins(size_t keySize, const void *key) */
#define HashJenkins(keySizeIn, key)                                                                                    \
({                                                                                                                     \
    size_t keySize = keySizeIn;                                                                                        \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    unsigned a, b;                                                                                                     \
    unsigned c = 0;                                                                                                    \
    unsigned char *k = (unsigned char *) key;                                                                          \
                                                                                                                       \
    a = b = 0x9e3779b9;                                                                                                \
                                                                                                                       \
    while (keySize >= 12) {                                                                                            \
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));                         \
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));                         \
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));                       \
        JENKINS_MIX(a, b, c);                                                                                          \
        k += 12;                                                                                                       \
        keySize -= 12;                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    c += keySize;                                                                                                      \
                                                                                                                       \
    switch (keySize) {                                                                                                 \
        case 11: c += ((unsigned)k[10] << 24); break;                                                                  \
        case 10: c += ((unsigned)k[9] << 16); break;                                                                   \
        case 9: c += ((unsigned)k[8] << 8); break;                                                                     \
        case 8: b += ((unsigned)k[7] << 24); break;                                                                    \
        case 7: b += ((unsigned)k[6] << 16); break;                                                                    \
        case 6: b += ((unsigned)k[5] << 8); break;                                                                     \
        case 5: b += k[4]; break;                                                                                      \
        case 4: a += ((unsigned)k[3] << 24); break;                                                                    \
        case 3: a += ((unsigned)k[2] << 16); break;                                                                    \
        case 2: a += ((unsigned)k[1] << 8); break;                                                                     \
        case 1: a += k[0]; break;                                                                                      \
    }                                                                                                                  \
    JENKINS_MIX(a, b, c);                                                                                              \
    c;                                                                                                                 \
})

// ---------------------------------------------------------------------------------------------------------------------
// I D E N T I T Y   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashIdentity(keySize, key)                                                                                     \
({                                                                                                                     \
    assert (keySize == sizeof(Hash) && (key != NULL));                                                                 \
    *((Hash *)key);                                                                                                    \
})

// ---------------------------------------------------------------------------------------------------------------------
// A D D I T I V E   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashAdditive(keySize, key)                                                                                     \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash += ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// X O R   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashXor(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash ^= ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// R O T A T I O N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashRot(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash ^= (hash << 4) ^ (hash >> 28) ^ ((unsigned char* )key)[i];                                                \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// B E R N S T E I N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashBernstein(keySize, key)                                                                                    \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// M O D I F I E D   B E R N S T E I N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashBernstein2(keySize, key)                                                                                   \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash ^= 33 * hash ^ ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// S H I F T - A D D - X O R   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashSax(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash ^= (hash << 5) + (hash >> 2) + ((unsigned char* )key)[i];                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// F N V   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashFnv(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 2166136261;                                                                                            \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash = (hash * 16777619) ^ ((unsigned char* )key)[i];                                                          \
    }                                                                                                                  \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// O N E - A T - A - T I M E   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashOat(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0;                                                                                                     \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash += ((unsigned char* )key)[i];                                                                             \
        hash += (hash << 10);                                                                                          \
        hash ^= (hash >> 6);                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    hash += (hash << 3);                                                                                               \
    hash ^= (hash >> 11);                                                                                              \
    hash += (hash << 15);                                                                                              \
                                                                                                                       \
    hash;                                                                                                              \
})

// ---------------------------------------------------------------------------------------------------------------------
// E L F   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define HashElf(keySize, key)                                                                                          \
({                                                                                                                     \
    assert ((key != NULL) && (keySize > 0));                                                                           \
                                                                                                                       \
    Hash hash = 0, g;                                                                                                  \
    for (size_t i = 0; i < keySize; i++) {                                                                             \
        hash = (hash << 4) + ((unsigned char* )key)[i];                                                                \
        if ((g = hash & 0xf0000000L) != 0) {                                                                           \
            hash ^= g >> 24;                                                                                           \
        }                                                                                                              \
        hash &= ~g;                                                                                                    \
    }                                                                                                                  \
    hash;                                                                                                              \
})

NG5_END_DECL

#endif