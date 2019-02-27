/**
 * Copyright 2018 Marcus Pinnecke
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

#include <carbon/carbon-error.h>
#include <carbon/carbon-string-id-cache.h>
#include "carbon/carbon-string-id-cache.h"

typedef struct cache_entry cache_entry_t;

typedef struct cache_entry
{
    cache_entry_t *prev, *next;
    carbon_string_id_t id;
    char *string;
} cache_entry_t;

typedef struct
{
    cache_entry_t *most_recent;
    cache_entry_t *lest_recent;
    cache_entry_t entries[1024];
} lru_list_t;

typedef struct carbon_string_id_cache
{
    carbon_vec_t ofType(lru_list_t) list_entries;
    carbon_string_id_cache_statistics_t statistics;
    carbon_query_t *query;
    carbon_err_t err;
} carbon_string_id_cache_t;

static void
init_list(lru_list_t *list)
{
    size_t num_entries = sizeof(list->entries)/sizeof(list->entries[0]);
    list->most_recent = list->entries + 0;
    list->lest_recent = list->entries + num_entries - 1;
    for (size_t i = 0; i < num_entries; i++) {
        cache_entry_t *entry = list->entries + i;
        entry->prev = i == 0 ? NULL : &list->entries[i - 1];
        entry->next = i + 1 < num_entries ? &list->entries[i + 1] : NULL;
    }
}

CARBON_EXPORT(bool)
carbon_string_id_cache_create_LRU(carbon_string_id_cache_t **cache, carbon_query_t *query)
{
    CARBON_NON_NULL_OR_ERROR(cache)
    CARBON_NON_NULL_OR_ERROR(query)

    uint32_t              capacity;
    carbon_archive_info_t archive_info;
    carbon_archive_get_info(&archive_info, query->archive);
    capacity = archive_info.num_embeddded_strings;

    carbon_string_id_cache_t *result = malloc(sizeof(carbon_string_id_cache_t));

    size_t num_buckets = CARBON_MAX(1, capacity / sizeof(lru_list_t));
    carbon_vec_create(&result->list_entries, NULL, sizeof(lru_list_t), num_buckets);
    for (size_t i = 0; i < num_buckets; i++) {
        lru_list_t *list = VECTOR_NEW_AND_GET(&result->list_entries, lru_list_t);
        CARBON_ZERO_MEMORY(list, sizeof(lru_list_t));
        init_list(list);
    }

    carbon_error_init(&result->err);
    result->query = query;
    carbon_string_id_cache_reset_statistics(result);
    *cache = result;

    return true;
}

CARBON_EXPORT(bool)
carbon_string_id_cache_get_error(carbon_err_t *err, const carbon_string_id_cache_t *cache)
{
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(cache)
    *err = cache->err;
    return true;
}

static void
make_most_recent(lru_list_t *list, cache_entry_t *entry)
{
    if (list->most_recent == entry) {
        return;
    } else {
        if (entry->prev) {
            entry->prev->next = entry->next;
        }
        if (entry->next) {
            entry->next->prev = entry->next;
        } else {
            list->lest_recent = entry->prev;
        }
        list->most_recent->prev = entry;
        entry->next = list->most_recent;
        list->most_recent = entry;
    }
}

CARBON_EXPORT(char *)
carbon_string_id_cache_get(carbon_string_id_cache_t *cache, carbon_string_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(cache)
    carbon_hash_t id_hash = CARBON_HASH_BERNSTEIN(sizeof(carbon_string_id_t), &id);
    size_t bucket_pos = id_hash % cache->list_entries.num_elems;
    lru_list_t *list = CARBON_VECTOR_GET(&cache->list_entries, bucket_pos, lru_list_t);
    cache_entry_t *cursor = list->most_recent;
    while (cursor != NULL) {
        if (id == cursor->id) {
            make_most_recent(list, cursor);
            cache->statistics.num_hits++;
            return strdup(cursor->string);
        }
        cursor = cursor->next;
    }
    char *result = carbon_query_fetch_string_by_id_nocache(cache->query, id);
    assert(result);
    if (list->lest_recent->string != NULL) {
        cache->statistics.num_evicted++;
    }
    list->lest_recent->string = result;
    list->lest_recent->id = id;
    make_most_recent(list, list->lest_recent);
    cache->statistics.num_misses++;
    return strdup(result);
}

CARBON_EXPORT(bool)
carbon_string_id_cache_get_statistics(carbon_string_id_cache_statistics_t *statistics, carbon_string_id_cache_t *cache)
{
    CARBON_NON_NULL_OR_ERROR(statistics);
    CARBON_NON_NULL_OR_ERROR(cache);
    *statistics = cache->statistics;
    return true;
}

CARBON_EXPORT(bool)
carbon_string_id_cache_reset_statistics(carbon_string_id_cache_t *cache)
{
    CARBON_NON_NULL_OR_ERROR(cache);
    CARBON_ZERO_MEMORY(&cache->statistics, sizeof(carbon_string_id_cache_statistics_t));
    return true;
}

CARBON_EXPORT(bool)
carbon_string_id_cache_drop(carbon_string_id_cache_t *cache)
{
    CARBON_NON_NULL_OR_ERROR(cache);
    for (size_t i = 0; i < cache->list_entries.num_elems; i++) {
        lru_list_t *entry = CARBON_VECTOR_GET(&cache->list_entries, i, lru_list_t);
        for (size_t k = 0; k < sizeof(entry->entries)/ sizeof(entry->entries[0]); k++)
        {
            cache_entry_t *it = &entry->entries[k];
            if (it->string) {
                free(it->string);
            }
        }
    }
    carbon_vec_drop(&cache->list_entries);
    return true;
}


