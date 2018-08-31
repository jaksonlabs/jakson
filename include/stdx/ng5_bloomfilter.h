// file: ng5_bloomfilter.h

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

#include <ng5_common.h>
#include "ng5_bitset.h"

typedef ng5_bitset_t ng5_bloomfilter_t;

int ng5_bloomfilter_create(ng5_bloomfilter_t *filter, size_t size);

int ng5_bloomfilter_drop(ng5_bloomfilter_t *filter);

int ng5_bloomfilter_clear(ng5_bloomfilter_t *filter);

#define ng5_bloomfilter_test_and_set(filter, key, key_size) \
({                                                          \
    size_t nbits = ng5_bitset_num_bits(filter);             \
    size_t b0 = hash_additive(key_size, key) % nbits;       \
    size_t b1 = hash__xor(key_size, key) % nbits;           \
    size_t b2 = hash_rot(key_size, key) % nbits;            \
    size_t b3 = hash_sax(key_size, key) % nbits;            \
    bool b0set = ng5_bitset_get(filter, b0);                \
    bool b1set = ng5_bitset_get(filter, b1);                \
    bool b2set = ng5_bitset_get(filter, b2);                \
    bool b3set = ng5_bitset_get(filter, b3);                \
    ng5_bitset_set(filter, b0, true);                       \
    ng5_bitset_set(filter, b1, true);                       \
    ng5_bitset_set(filter, b2, true);                       \
    ng5_bitset_set(filter, b3, true);                       \
    (b0set && b1set && b2set && b3set);                     \
})

#endif