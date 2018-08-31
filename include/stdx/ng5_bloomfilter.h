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

bool ng5_bloomfilter_test_and_set(ng5_bloomfilter_t *filter, const void *key, size_t key_size);

#endif