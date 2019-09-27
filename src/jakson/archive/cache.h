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

#ifndef CACHE_H
#define CACHE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/types.h>
#include <jakson/archive/query.h>

BEGIN_DECL

typedef struct sid_cache_stats {
        size_t num_hits;
        size_t num_misses;
        size_t num_evicted;
} sid_cache_stats;

bool string_id_cache_create_lru(struct string_cache **cache, archive *archive);
bool string_id_cache_create_lru_ex(struct string_cache **cache, archive *archive, size_t capacity);
bool string_id_cache_get_error(err *err, const struct string_cache *cache);
bool string_id_cache_get_size(size_t *size, const struct string_cache *cache);
char *string_id_cache_get(struct string_cache *cache, archive_field_sid_t id);
bool string_id_cache_get_statistics(sid_cache_stats *statistics, struct string_cache *cache);
bool string_id_cache_reset_statistics(struct string_cache *cache);
bool string_id_cache_drop(struct string_cache *cache);

END_DECL

#endif
