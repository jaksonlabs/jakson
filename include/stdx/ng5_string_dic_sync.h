// file: string_dic_naive.h

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

#ifndef NG5_STRING_DIC_NAIVE
#define NG5_STRING_DIC_NAIVE

#include <stdx/ng5_string_dic.h>

int string_dic_create_sync(struct string_dic* dic, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads, const ng5_allocator_t* alloc);

#endif