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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_hash_bern.h>
#include <jak_error.h>
#include <jak_archive_cache.h>

struct cache_entry {
    struct cache_entry *prev, *next;
    jak_archive_field_sid_t id;
    char *string;
};

struct lru_list {
    struct cache_entry *most_recent;
    struct cache_entry *lest_recent;
    struct cache_entry entries[1024];
};

struct jak_string_cache {
    struct jak_vector ofType(struct lru_list) list_entries;
    struct jak_sid_cache_stats statistics;
    struct jak_archive_query query;
    struct jak_error err;
    size_t capacity;
};

static void init_list(struct lru_list *list)
{
        size_t num_entries = sizeof(list->entries) / sizeof(list->entries[0]);
        list->most_recent = list->entries + 0;
        list->lest_recent = list->entries + num_entries - 1;
        for (size_t i = 0; i < num_entries; i++) {
                struct cache_entry *entry = list->entries + i;
                entry->prev = i == 0 ? NULL : &list->entries[i - 1];
                entry->next = i + 1 < num_entries ? &list->entries[i + 1] : NULL;
        }
}

bool jak_string_id_cache_create_lru(struct jak_string_cache **cache, struct jak_archive *archive)
{
        struct jak_archive_info archive_info;
        jak_archive_get_info(&archive_info, archive);
        jak_u32 capacity = archive_info.num_embeddded_strings * 0.25f;
        return jak_string_id_cache_create_lru_ex(cache, archive, capacity);
}

bool jak_string_id_cache_create_lru_ex(struct jak_string_cache **cache, struct jak_archive *archive, size_t capacity)
{
        JAK_ERROR_IF_NULL(cache)
        JAK_ERROR_IF_NULL(archive)

        struct jak_string_cache *result = JAK_MALLOC(sizeof(struct jak_string_cache));

        jak_query_create(&result->query, archive);
        result->capacity = capacity;

        size_t num_buckets = JAK_max(1, capacity);
        vec_create(&result->list_entries, NULL, sizeof(struct lru_list), num_buckets);
        for (size_t i = 0; i < num_buckets; i++) {
                struct lru_list *list = vec_new_and_get(&result->list_entries, struct lru_list);
                JAK_zero_memory(list, sizeof(struct lru_list));
                init_list(list);
        }

        error_init(&result->err);
        jak_string_id_cache_reset_statistics(result);
        *cache = result;

        return true;
}

bool jak_string_id_cache_get_error(struct jak_error *err, const struct jak_string_cache *cache)
{
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(cache)
        *err = cache->err;
        return true;
}

bool jak_string_id_cache_get_size(size_t *size, const struct jak_string_cache *cache)
{
        JAK_ERROR_IF_NULL(size)
        JAK_ERROR_IF_NULL(cache)
        *size = cache->capacity;
        return true;
}

static void make_most_recent(struct lru_list *list, struct cache_entry *entry)
{
        if (list->most_recent == entry) {
                return;
        } else {
                if (entry->prev) {
                        entry->prev->next = entry->next;
                }
                if (entry->next) {
                        entry->next->prev = entry->prev;
                } else {
                        list->lest_recent = entry->prev;
                }
                list->most_recent->prev = entry;
                entry->next = list->most_recent;
                list->most_recent = entry;
        }
}

char *jak_string_id_cache_get(struct jak_string_cache *cache, jak_archive_field_sid_t id)
{
        JAK_ERROR_IF_NULL(cache)
        hash32_t id_hash = JAK_HASH_BERNSTEIN(sizeof(jak_archive_field_sid_t), &id);
        size_t bucket_pos = id_hash % cache->list_entries.num_elems;
        struct lru_list *list = vec_get(&cache->list_entries, bucket_pos, struct lru_list);
        struct cache_entry *cursor = list->most_recent;
        while (cursor != NULL) {
                if (id == cursor->id) {
                        make_most_recent(list, cursor);
                        cache->statistics.num_hits++;
                        return strdup(cursor->string);
                }
                cursor = cursor->next;
        }
        char *result = jak_query_fetch_string_by_id_nocache(&cache->query, id);
        JAK_ASSERT(result);
        if (list->lest_recent->string != NULL) {
                cache->statistics.num_evicted++;
        }
        list->lest_recent->string = result;
        list->lest_recent->id = id;
        make_most_recent(list, list->lest_recent);
        cache->statistics.num_misses++;
        return strdup(result);
}

bool jak_string_id_cache_get_statistics(struct jak_sid_cache_stats *statistics, struct jak_string_cache *cache)
{
        JAK_ERROR_IF_NULL(statistics);
        JAK_ERROR_IF_NULL(cache);
        *statistics = cache->statistics;
        return true;
}

bool jak_string_id_cache_reset_statistics(struct jak_string_cache *cache)
{
        JAK_ERROR_IF_NULL(cache);
        JAK_zero_memory(&cache->statistics, sizeof(struct jak_sid_cache_stats));
        return true;
}

bool jak_string_id_cache_drop(struct jak_string_cache *cache)
{
        JAK_ERROR_IF_NULL(cache);
        for (size_t i = 0; i < cache->list_entries.num_elems; i++) {
                struct lru_list *entry = vec_get(&cache->list_entries, i, struct lru_list);
                for (size_t k = 0; k < sizeof(entry->entries) / sizeof(entry->entries[0]); k++) {
                        struct cache_entry *it = &entry->entries[k];
                        if (it->string) {
                                free(it->string);
                        }
                }
        }
        vec_drop(&cache->list_entries);
        return true;
}


