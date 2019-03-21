/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CARBON_CACHE_H
#define CARBON_CACHE_H

#include "carbon-common.h"
#include "carbon-types.h"
#include "carbon-query.h"

CARBON_BEGIN_DECL

typedef struct carbon_string_id_cache carbon_string_id_cache_t;

typedef struct
{
    size_t num_hits;
    size_t num_misses;
    size_t num_evicted;
} carbon_string_id_cache_statistics_t;

CARBON_EXPORT(bool)
carbon_string_id_cache_create_LRU(carbon_string_id_cache_t **cache, carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_string_id_cache_create_LRU_ex(carbon_string_id_cache_t **cache, carbon_archive_t *archive, size_t capacity);

CARBON_EXPORT(bool)
carbon_string_id_cache_get_error(carbon_err_t *err, const carbon_string_id_cache_t *cache);

CARBON_EXPORT(bool)
carbon_string_id_cache_get_size(size_t *size, const carbon_string_id_cache_t *cache);

CARBON_EXPORT(char *)
carbon_string_id_cache_get(carbon_string_id_cache_t *cache, carbon_string_id_t id);

CARBON_EXPORT(bool)
carbon_string_id_cache_get_statistics(carbon_string_id_cache_statistics_t *statistics, carbon_string_id_cache_t *cache);

CARBON_EXPORT(bool)
carbon_string_id_cache_reset_statistics(carbon_string_id_cache_t *cache);

CARBON_EXPORT(bool)
carbon_string_id_cache_drop(carbon_string_id_cache_t *cache);

CARBON_END_DECL

#endif
