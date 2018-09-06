// file: ng5_bitset.h

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

#ifndef NG5_BITSET
#define NG5_BITSET

#include <ng5_common.h>
#include <stdx/ng5_vector.h>

typedef struct ng5_bitset_t
{
    ng5_vector_t of_type(uint64_t) data;
    uint16_t                       num_bits;
} ng5_bitset_t;

int ng5_bitset_create(ng5_bitset_t *bitset, uint16_t num_bits);

int ng5_bitset_drop(ng5_bitset_t *bitset);

size_t ng5_bitset_num_bits(const ng5_bitset_t *bitset);

int ng5_bitset_clear(ng5_bitset_t *bitset);

int ng5_bitset_set(ng5_bitset_t *bitset, uint16_t bit_pos, bool on);

bool ng5_bitset_get(ng5_bitset_t *bitset, uint16_t bit_pos);

#endif