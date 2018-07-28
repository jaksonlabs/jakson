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


#ifndef _NG5_HASH
#define _NG5_HASH

#include <common.h>

typedef size_t hash_t;

#define JENKINS_MIX(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c >> 13); \
    b -= c; b -= a; b ^= (a << 8); \
    c -= a; c -= b; c ^= (b >> 13); \
    a -= b; a -= c; a ^= (c >> 12); \
    b -= c; b -= a; b ^= (a << 16); \
    c -= a; c -= b; c ^= (b >> 5); \
    a -= b; a -= c; a ^= (c >> 3); \
    b -= c; b -= a; b ^= (a << 10); \
    c -= a; c -= b; c ^= (b >> 15); \
}

inline static hash_t jenkins_hash(size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    unsigned a, b;
    unsigned c = 0;
    unsigned char *k = (unsigned char *) key;

    a = b = 0x9e3779b9;

    while (key_size >= 12) {
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));
        JENKINS_MIX(a, b, c);
        k += 12;
        key_size -= 12;
    }

    c += key_size;

    switch (key_size) {
    case 11: c += ((unsigned)k[10] << 24); break;
    case 10: c += ((unsigned)k[9] << 16); break;
    case 9: c += ((unsigned)k[8] << 8); break;
    case 8: b += ((unsigned)k[7] << 24); break;
    case 7: b += ((unsigned)k[6] << 16); break;
    case 6: b += ((unsigned)k[5] << 8); break;
    case 5: b += k[4]; break;
    case 4: a += ((unsigned)k[3] << 24); break;
    case 3: a += ((unsigned)k[2] << 16); break;
    case 2: a += ((unsigned)k[1] << 8); break;
    case 1: a += k[0]; break;
    }
    JENKINS_MIX(a, b, c);
    return c;
}

#endif