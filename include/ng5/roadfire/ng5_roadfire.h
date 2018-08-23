// file: roadfire.h

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

#ifndef NG5_ROADFIRE
#define NG5_ROADFIRE

#include <ng5/ng5_storage_engine.h>

struct roadfire_conf {
    /* initial reserved number of unique strings stored in the string dictionary */
    size_t string_dic_capacity;

    /* number of hash buckets for string lookup in string dictionary */
    size_t string_dic_lookup_num_buckets;

    /* reserved number of objects per bucket in string dictionary */
    size_t string_dic_bucket_capacity;

    /* number of threads used inside the string dictionary */
    size_t string_dic_nthreads;

    /* result handle register initial number of reserved handles */
    size_t result_register_reserve_num_handles;

};

extern struct roadfire_conf roadfire_conf_default;

int storage_engine_roadfire_create(struct storage_engine *out, optional struct roadfire_conf *conf,
        const ng5_allocator_t *alloc);

#endif
