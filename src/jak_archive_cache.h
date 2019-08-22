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

#ifndef JAK_CACHE_H
#define JAK_CACHE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_types.h>
#include <jak_archive_query.h>

JAK_BEGIN_DECL

struct jak_sid_cache_stats {
    size_t num_hits;
    size_t num_misses;
    size_t num_evicted;
};

bool jak_string_id_cache_create_lru(struct jak_string_cache **cache, struct jak_archive *archive);

bool jak_string_id_cache_create_lru_ex(struct jak_string_cache **cache, struct jak_archive *archive, size_t capacity);

bool jak_string_id_cache_get_error(struct jak_error *err, const struct jak_string_cache *cache);

bool jak_string_id_cache_get_size(size_t *size, const struct jak_string_cache *cache);

char *jak_string_id_cache_get(struct jak_string_cache *cache, jak_archive_field_sid_t id);

bool jak_string_id_cache_get_statistics(struct jak_sid_cache_stats *statistics, struct jak_string_cache *cache);

bool jak_string_id_cache_reset_statistics(struct jak_string_cache *cache);

bool jak_string_id_cache_drop(struct jak_string_cache *cache);

JAK_END_DECL

#endif
