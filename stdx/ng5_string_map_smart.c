// file: simple_bsearch.c

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include <stdx/ng5_string_map_smart.h>
#include <stdx/ng5_spinlock.h>
#include <stdlib.h>
#include <stdx/ng5_algorithm.h>
#include <stdx/ng5_trace_alloc.h>
#include <stdx/ng5_time.h>
#include <stdx/ng5_bloomfilter.h>

#define get_hashcode(key)      hash_bernstein(strlen(key), key)
#define get_hashcode_2(key)    hash_additive(strlen(key), key)

#define SMART_MAP_TAG "smart-map"

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------


struct simple_bucket_entry {
  const char    *str;
  string_id_t    value;
  hash_t         key_hash_2;
};

struct simple_bucket {
  struct spinlock                            spinlock;
  ng5_vector_t of_type(simple_bucket_entry) entries;
  ng5_vector_t of_type(uint32_t)             freelist;  /* OPTIMIZATION: in "string map", freelist uses 32bit instead of 64bit since that's enough (i.e., maps are used by several threads which means: the number of total managble strings scale with the number of threads */

  /* used to minimize lookup times in case the same string is queried more than once in a direct sequence */
  uint32_t                                    cache_idx;

  /* number of entries actually occupied with data inside this bucket (since entries num == entries cap all times) */
  uint32_t                                     num_entries;

  /* bloomfilter to skip searching in case of that a key definitively was never added */
  ng5_bloomfilter_t                            bloomfilter;
};

struct simple_extra {
  ng5_vector_t of_type(simple_bucket) buckets;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

static int smart_drop(struct string_map* self);
static int smart_put_safe_bulk(struct string_map* self, char* const* keys, const string_id_t* values, size_t num_pairs);
static int smart_put_fast_bulk(struct string_map* self, char* const* keys, const string_id_t* values, size_t num_pairs);
static int smart_put_safe_exact(struct string_map* self, const char *key, string_id_t value);
static int smart_put_fast_exact(struct string_map* self, const char *key, string_id_t value);
static int smart_get_safe(struct string_map* self, string_id_t** out, bool** found_mask, size_t* num_not_found,
        char* const* keys, size_t num_keys);
static int smart_get_safe_exact(struct string_map* self, string_id_t* out, bool* found_mask, const char* key);
static int smart_get_fast(struct string_map* self, string_id_t** out, char* const* keys, size_t num_keys);
static int smart_update_key_fast(struct string_map* self, const string_id_t* values, char* const* keys,
        size_t num_keys);
static int smart_remove(struct string_map* self, char* const* keys, size_t num_keys);
static int smart_free(struct string_map* self, void* ptr);

static int smart_map_insert_bulk(ng5_vector_t of_type(simple_bucket)* buckets, char* const* restrict keys,
        const string_id_t* restrict values, size_t* restrict bucket_idxs, size_t num_pairs, ng5_allocator_t* alloc,
        struct string_map_counters* counter);
static int smart_map_insert_exact(ng5_vector_t of_type(simple_bucket)* buckets, const char * restrict key,
        string_id_t value, size_t bucket_idx, ng5_allocator_t* alloc, struct string_map_counters* counter);
static int simple_map_fetch_bulk(ng5_vector_t of_type(simple_bucket)* buckets, string_id_t* values_out,
        bool* key_found_mask,
        size_t* num_keys_not_found, size_t* bucket_idxs, char* const* keys, size_t num_keys,
        ng5_allocator_t* alloc, struct string_map_counters* counter);
static int simple_map_fetch_single(ng5_vector_t of_type(simple_bucket)* buckets, string_id_t* value_out,
        bool* key_found, const size_t bucket_idx, const char* key, struct string_map_counters* counter);

static int smart_extra_create(struct string_map* self, float grow_factor, size_t num_buckets, size_t cap_buckets);
static struct simple_extra *smart_extra_get(struct string_map* self);
static int smart_bucket_create(struct simple_bucket* buckets, size_t num_buckets, size_t bucket_cap,
        float grow_factor, ng5_allocator_t* alloc);
static int smart_bucket_drop(struct simple_bucket* buckets, size_t num_buckets, ng5_allocator_t* alloc);
static void smart_bucket_lock(struct simple_bucket* bucket) force_inline;
static void smart_bucket_unlock(struct simple_bucket* bucket) force_inline;
static int smart_bucket_insert(struct simple_bucket* bucket, const char* restrict key, string_id_t value,
        ng5_allocator_t* alloc, struct string_map_counters* counter) force_inline;
static void smart_bucket_freelist_push(struct simple_bucket* bucket, uint32_t idx);
static uint32_t smart_bucket_freelist_pop(struct simple_bucket* bucket, ng5_allocator_t* alloc) force_inline;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

int string_hashtable_create_scan1_cache(struct string_map* map, const ng5_allocator_t* alloc, size_t num_buckets,
        size_t cap_buckets, float bucket_grow_factor)
{
    check_success(allocator_this_or_default(&map->allocator, alloc));

    num_buckets  = num_buckets  < 1 ? 1 : num_buckets;
    cap_buckets  = cap_buckets  < 1 ? 1 : cap_buckets;

    map->tag              = STRING_ID_MAP_SMART;
    map->drop             = smart_drop;
    map->put_safe_bulk    = smart_put_safe_bulk;
    map->put_fast_bulk    = smart_put_fast_bulk;
    map->put_safe_exact    = smart_put_safe_exact;
    map->put_fast_exact    = smart_put_fast_exact;
    map->get_safe_bulk    = smart_get_safe;
    map->get_fast         = smart_get_fast;
    map->update_key_fast  = smart_update_key_fast;
    map->remove           = smart_remove;
    map->free             = smart_free;
    map->get_safe_exact   = smart_get_safe_exact;

    string_lookup_reset_counters(map);
    check_success(smart_extra_create(map, bucket_grow_factor, num_buckets, cap_buckets));
    return STATUS_OK;
}

static int smart_drop(struct string_map* self)
{
    assert(self->tag == STRING_ID_MAP_SMART);
    struct simple_extra *extra = smart_extra_get(self);
    struct simple_bucket *data = (struct simple_bucket *) ng5_vector_data(&extra->buckets);
    check_success(smart_bucket_drop(data, extra->buckets.cap_elems, &self->allocator));
    ng5_vector_drop(&extra->buckets);
    allocator_free(&self->allocator, self->extra);
    return STATUS_OK;
}

static int smart_put_safe_bulk(struct string_map* self, char* const* keys, const string_id_t* values, size_t num_pairs)
{
    assert(self->tag == STRING_ID_MAP_SMART);
    struct simple_extra *extra = smart_extra_get(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_pairs * sizeof(size_t));

    prefetch_write(bucket_idxs);

    for (size_t i = 0; i < num_pairs; i++) {
        const char *key        = keys[i];
        hash_t      hash       = get_hashcode(key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    prefetch_read(bucket_idxs);
    prefetch_read(keys);
    prefetch_read(values);

    check_success(smart_map_insert_bulk(&extra->buckets, keys, values, bucket_idxs, num_pairs, &self->allocator,
            &self->counters));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static int smart_put_safe_exact(struct string_map* self, const char *key, string_id_t value)
{
    assert(self->tag == STRING_ID_MAP_SMART);
    struct simple_extra *extra = smart_extra_get(self);

    hash_t      hash       = get_hashcode(key);
    size_t      bucket_idx = hash % extra->buckets.cap_elems;

    prefetch_read(key);

    check_success(smart_map_insert_exact(&extra->buckets, key, value, bucket_idx, &self->allocator,
            &self->counters));

    return STATUS_OK;
}

static int smart_put_fast_exact(struct string_map* self, const char *key, string_id_t value)
{
    return smart_put_safe_exact(self, key, value);
}

static int smart_put_fast_bulk(struct string_map* self, char* const* keys, const string_id_t* values, size_t num_pairs)
{
    return smart_put_safe_bulk(self, keys, values, num_pairs);
}

#define simple_bucket_find_entry_by_key(counter, bucket, key)                                                          \
({                                                                                                                     \
    hash_t key_hash_2   = get_hashcode_2(key);                                                                         \
    uint32_t return_value = bucket->entries.cap_elems;                                                                 \
                                                                                                                       \
    struct simple_bucket_entry* data = (struct simple_bucket_entry*) ng5_vector_data(&bucket->entries);                \
                                                                                                                       \
    if (likely(bucket->cache_idx!=(uint32_t) -1)) {                                                                    \
        struct simple_bucket_entry* cache = data + bucket->cache_idx;                                                  \
        if (cache->key_hash_2 == key_hash_2 && strcmp(cache->str, key) == 0) {                                         \
            counter->num_bucket_cache_search_hit++;                                                                    \
            return_value = bucket->cache_idx;                                                                          \
            goto exit_find_macro;                                                                                      \
        }                                                                                                              \
        counter->num_bucket_cache_search_miss++;                                                                       \
        prefetch_read(data);                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    size_t i = 0;                                                                                                      \
    for (; i<bucket->entries.cap_elems; i++) {                                                                         \
        if (unlikely(data[i].key_hash_2 == key_hash_2 && strcmp(data[i].str, key) == 0)) {                             \
            counter->num_bucket_search_hit++;                                                                          \
            bucket->cache_idx = i;                                                                                     \
            return_value = i;                                                                                          \
            break;                                                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
    counter->num_bucket_search_miss++;                                                                                 \
exit_find_macro:                                                                                                       \
    return_value;                                                                                                      \
})

static int simple_map_fetch_bulk(ng5_vector_t of_type(simple_bucket)* buckets, string_id_t* values_out,
        bool* key_found_mask,
        size_t* num_keys_not_found, size_t* bucket_idxs, char* const* keys, size_t num_keys,
        ng5_allocator_t* alloc, struct string_map_counters* counter)
{
    unused(alloc);

    size_t num_not_found = 0;
    struct simple_bucket *data = (struct simple_bucket *) ng5_vector_data(buckets);

    prefetch_write(values_out);

    for (size_t i = 0; i < num_keys; i++) {
        struct simple_bucket       *bucket     = data + bucket_idxs[i];

      //  smart_bucket_lock(bucket);

        const char* key = keys[i];
        /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        uint32_t needle_pos = bucket->num_entries > 0 ? simple_bucket_find_entry_by_key(counter, bucket, key) : bucket->entries.cap_elems;

        bool found = needle_pos < bucket->entries.cap_elems;
        num_not_found += found ? 0 : 1;
        key_found_mask[i] = found;
        values_out[i] = found ? (((struct simple_bucket_entry*) bucket->entries.base)+needle_pos)->value : -1;

        //  smart_bucket_unlock(bucket);
    }

    *num_keys_not_found = num_not_found;
    return STATUS_OK;
}

static int simple_map_fetch_single(ng5_vector_t of_type(simple_bucket)* buckets, string_id_t* value_out,
        bool* key_found, const size_t bucket_idx, const char* key, struct string_map_counters* counter)
{
    struct simple_bucket *data = (struct simple_bucket *) ng5_vector_data(buckets);

    prefetch_write(value_out);
    prefetch_write(key_found);

    struct simple_bucket       *bucket     = data + bucket_idx;

    smart_bucket_lock(bucket);

    /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    uint32_t needle_pos = bucket->num_entries > 0 ? simple_bucket_find_entry_by_key(counter, bucket, key) : bucket->entries.cap_elems;

    *key_found = needle_pos < bucket->entries.cap_elems;
    *value_out = (*key_found) ? (((struct simple_bucket_entry*) bucket->entries.base)+needle_pos)->value : -1;

    smart_bucket_unlock(bucket);

    return STATUS_OK;
}

static int smart_get_safe(struct string_map* self, string_id_t** out, bool** found_mask, size_t* num_not_found,
        char* const* keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SMART);

    timestamp_t begin = time_current_time_ms();
    trace(SMART_MAP_TAG, "'get_safe' function invoked for %zu strings", num_keys)

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    check_success(allocator_trace(&hashtable_alloc));
#else
    check_success(allocator_this_or_default(&hashtable_alloc, &self->allocator));
#endif

    struct simple_extra *extra          = smart_extra_get(self);
    size_t              *bucket_idxs    = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    string_id_t         *values_out     = allocator_malloc(&self->allocator, num_keys * sizeof(string_id_t));
    bool                *found_mask_out = allocator_malloc(&self->allocator, num_keys * sizeof(bool));

    assert(bucket_idxs != NULL);
    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    for (register size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = get_hashcode(key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
        prefetch_read((struct simple_bucket *) ng5_vector_data(&extra->buckets) + bucket_idxs[i]);
    }
    trace(SMART_MAP_TAG, "'get_safe' function invoke fetch...for %zu strings", num_keys)
    check_success(simple_map_fetch_bulk(&extra->buckets, values_out, found_mask_out, num_not_found, bucket_idxs,
            keys, num_keys, &self->allocator, &self->counters));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    trace(SMART_MAP_TAG, "'get_safe' function invok fetch: done for %zu strings", num_keys)

    assert(values_out != NULL);
    assert(found_mask_out != NULL);

    *out = values_out;
    *found_mask = found_mask_out;

    timestamp_t end = time_current_time_ms();
    unused(begin);
    unused(end);
    trace(SMART_MAP_TAG, "'get_safe' function done: %f seconds spent here", (end-begin)/1000.0f)

    return STATUS_OK;
}

static int smart_get_safe_exact(struct string_map* self, string_id_t* out, bool* found_mask, const char* key)
{
    assert(self->tag == STRING_ID_MAP_SMART);

    ng5_allocator_t hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    check_success(allocator_trace(&hashtable_alloc));
#else
    check_success(allocator_this_or_default(&hashtable_alloc, &self->allocator));
#endif

    struct simple_extra *extra          = smart_extra_get(self);

    hash_t      hash       = get_hashcode(key);
    size_t      bucket_idx = hash % extra->buckets.cap_elems;
    prefetch_read((struct simple_bucket *) ng5_vector_data(&extra->buckets) + bucket_idx);

    check_success(simple_map_fetch_single(&extra->buckets, out, found_mask, bucket_idx, key, &self->counters));

    return STATUS_OK;
}

static int smart_get_fast(struct string_map* self, string_id_t** out, char* const* keys, size_t num_keys)
{
    bool* found_mask;
    size_t num_not_found;
    int status = smart_get_safe(self, out, &found_mask, &num_not_found, keys, num_keys);
    smart_free(self, found_mask);
    return status;
}

static int smart_update_key_fast(struct string_map* self, const string_id_t* values, char* const* keys,
        size_t num_keys)
{
    unused(self);
    unused(values);
    unused(keys);
    unused(num_keys);
    return STATUS_NOTIMPL;
}

static int simple_map_remove(struct simple_extra *extra, size_t *bucket_idxs, char *const *keys, size_t num_keys,
        ng5_allocator_t *alloc, struct string_map_counters *counter)
{
    unused(alloc);

    struct simple_bucket *data = (struct simple_bucket *) ng5_vector_data(&extra->buckets);

    for (register size_t i = 0; i < num_keys; i++) {
        struct simple_bucket* bucket     = data + bucket_idxs[i];
        const char           *key        = keys[i];

        smart_bucket_lock(bucket);

        /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        uint32_t needle_pos = bucket->num_entries > 0 ? simple_bucket_find_entry_by_key(counter, bucket, key) : bucket->entries.cap_elems;
        if (likely(needle_pos < bucket->entries.cap_elems)) {
            struct simple_bucket_entry* entry =
                    (struct simple_bucket_entry*) ng5_vector_data(&bucket->entries)+needle_pos;
            assert (entry->key_hash_2 != 0);
            entry->key_hash_2 = 0;
            smart_bucket_freelist_push(bucket, needle_pos);
        }

        smart_bucket_unlock(bucket);
    }
    return STATUS_OK;
}

static int smart_remove(struct string_map* self, char* const* keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SMART);

    struct simple_extra *extra = smart_extra_get(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (register size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = get_hashcode(key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_remove(extra, bucket_idxs, keys, num_keys, &self->allocator, &self->counters));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static int smart_free(struct string_map* self, void* ptr)
{
    assert(self->tag == STRING_ID_MAP_SMART);
    check_success(allocator_free(&self->allocator, ptr));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

unused_fn
static int smart_extra_create(struct string_map* self, float grow_factor, size_t num_buckets, size_t cap_buckets)
{
    if ((self->extra = allocator_malloc(&self->allocator, sizeof(struct simple_extra))) != NULL) {
        struct simple_extra *extra = smart_extra_get(self);
        ng5_vector_create(&extra->buckets, &self->allocator, sizeof(struct simple_bucket), num_buckets);

        /* Optimization: notify the kernel that the list of buckets are accessed randomly (since hash based access)*/
        ng5_vector_advise(&extra->buckets, MADV_RANDOM | MADV_WILLNEED);


        struct simple_bucket *data = (struct simple_bucket *) ng5_vector_data(&extra->buckets);
        check_success(smart_bucket_create(data, num_buckets, cap_buckets, grow_factor, &self->allocator));
        return STATUS_OK;
    } else {
        return STATUS_MALLOCERR;
    }
}

unused_fn
static struct simple_extra *smart_extra_get(struct string_map* self)
{
    assert (self->tag == STRING_ID_MAP_SMART);
    return (struct simple_extra *)(self->extra);
}

unused_fn
static int smart_bucket_create(struct simple_bucket* buckets, size_t num_buckets, size_t bucket_cap,
        float grow_factor, ng5_allocator_t* alloc)
{
    check_non_null(buckets);

    struct simple_bucket_entry entry = {
            .str        = NULL,
            .value      = 0,
            .key_hash_2 = 0
    };

    // TODO: parallize this!
    while (num_buckets--) {
        struct simple_bucket *bucket = buckets++;
        bucket->cache_idx   = (uint32_t) -1;
        bucket->num_entries = 0;
        spinlock_create(&bucket->spinlock);

        ng5_vector_create(&bucket->entries, alloc, sizeof(struct simple_bucket_entry), bucket_cap);
        ng5_vector_create(&bucket->freelist, alloc, sizeof(uint32_t), bucket_cap);

        ng5_vector_set_growfactor(&bucket->entries, grow_factor);
        ng5_vector_set_growfactor(&bucket->freelist, grow_factor);
        ng5_vector_repreat_push(&bucket->entries, &entry, bucket_cap);

        ng5_bloomfilter_create(&bucket->bloomfilter, 256);
    }

    return STATUS_OK;
}

static int smart_bucket_drop(struct simple_bucket* buckets, size_t num_buckets, ng5_allocator_t* alloc)
{
    unused(alloc);
    check_non_null(buckets);

    while (num_buckets--) {
        struct simple_bucket *bucket = buckets++;
        check_success(ng5_vector_drop(&bucket->entries));
        check_success(ng5_vector_drop(&bucket->freelist));
    }

    return STATUS_OK;
}

static void smart_bucket_lock(struct simple_bucket* bucket)
{
    spinlock_lock(&bucket->spinlock);
}

unused_fn
static void smart_bucket_unlock(struct simple_bucket* bucket)
{
    spinlock_unlock(&bucket->spinlock);
}

unused_fn
static void smart_bucket_freelist_push(struct simple_bucket* bucket, uint32_t idx)
{
    ng5_vector_t of_type(uint32_t) *freelist = &bucket->freelist;

    assert (freelist->num_elems + 1 < freelist->cap_elems);

    ng5_vector_push(freelist, &idx, 1);
    bucket->num_entries--;
}

unused_fn
static uint32_t smart_bucket_freelist_pop(struct simple_bucket* bucket, ng5_allocator_t* alloc)
{
    unused(alloc);

    ng5_vector_t of_type(uint32_t)            *freelist = &bucket->freelist;
    ng5_vector_t of_type(simple_bucket_entry) *entries  = &bucket->entries;

    struct simple_bucket_entry empty = {
            .str        = NULL,
            .value      = 0,
            .key_hash_2 = 0
    };

    if (unlikely(ng5_vector_is_empty(freelist))) {
        size_t new_slots;
        uint32_t last_slot = entries->cap_elems;
        check_success(ng5_vector_grow(&new_slots, freelist));
        check_success(ng5_vector_grow(NULL, entries));
        for (register uint32_t slot = 0; slot < new_slots; slot++) {
            uint32_t new_slot_id = last_slot + slot;
            check_success(ng5_vector_push(entries, &empty, 1));
            check_success(ng5_vector_push(freelist, &new_slot_id, 1));
        }
    }
    assert (!ng5_vector_is_empty(freelist));
    assert (ng5_vector_cap(freelist) ==ng5_vector_cap(entries));

    uint32_t result = *(uint32_t *) ng5_vector_pop(freelist);
    bucket->num_entries++;
    return result;
}

static int smart_bucket_insert(struct simple_bucket* bucket, const char* restrict key, string_id_t value,
        ng5_allocator_t* alloc, struct string_map_counters* counter)
{
    check_non_null(bucket);
    check_non_null(key);

    smart_bucket_lock(bucket);

    uint32_t                    needle_pos     = bucket->entries.cap_elems;
    struct simple_bucket_entry *data           = NULL;
    hash_t                      bloom_hash_key = hash_fnv(strlen(key), key);

    if (ng5_bloomfilter_test_and_set(&bucket->bloomfilter, &bloom_hash_key, sizeof(hash_t))) { /* OPTIMIZATION: insert with test whether this key was already inserted into the bucket */
        /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        needle_pos = bucket->num_entries > 0 ? simple_bucket_find_entry_by_key(counter, bucket, key) : bucket->entries.cap_elems;
        data       = (struct simple_bucket_entry *) ng5_vector_data(&bucket->entries);
    }

    if (needle_pos < bucket->entries.cap_elems) {
        /* entry found by key */
        assert (data != NULL);
        data->value = value;
    } else {
        /* no entry found */
        uint32_t free_slot = smart_bucket_freelist_pop(bucket, alloc);
        data = (struct simple_bucket_entry *) ng5_vector_data(&bucket->entries);
        struct simple_bucket_entry *entry = data + free_slot;
        assert(entry->key_hash_2 == 0);
        entry->str     = key;
        entry->key_hash_2 = get_hashcode_2(key);
        entry->value   = value;
    }

    smart_bucket_lock(bucket);
    return STATUS_OK;
}

static int smart_map_insert_bulk(ng5_vector_t of_type(simple_bucket)* buckets, char* const* restrict keys,
        const string_id_t* restrict values, size_t* restrict bucket_idxs, size_t num_pairs, ng5_allocator_t* alloc,
        struct string_map_counters* counter)
{
    check_non_null(buckets)
    check_non_null(keys)
    check_non_null(values)
    check_non_null(bucket_idxs)

    struct simple_bucket *buckets_data = (struct simple_bucket *) ng5_vector_data(buckets);
    int status = STATUS_OK;
    for (register size_t i = 0; status == STATUS_OK && i < num_pairs; i++) {
        size_t       bucket_idx      = bucket_idxs[i];
        const char  *key             = keys[i];
        string_id_t     value           = values[i];

        struct simple_bucket *bucket = buckets_data + bucket_idx;
        status = smart_bucket_insert(bucket, key, value, alloc, counter);
    }

    return status;
}

static int smart_map_insert_exact(ng5_vector_t of_type(simple_bucket)* buckets, const char * restrict key,
        string_id_t value, size_t bucket_idx, ng5_allocator_t* alloc, struct string_map_counters* counter)
{
    check_non_null(buckets)
    check_non_null(key)

    struct simple_bucket *buckets_data = (struct simple_bucket *) ng5_vector_data(buckets);
    struct simple_bucket *bucket = buckets_data + bucket_idx;
    return smart_bucket_insert(bucket, key, value, alloc, counter);
}
