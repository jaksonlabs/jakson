// file: string_id_map.c

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

#include <stdx/string_id_map.h>
#include <stdx/asnyc.h>
#include <stdlib.h>
#include <stdx/algorithm.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

struct simple_bucket_entry {
  bool        in_use;
  const char *str;
  size_t      str_len;
  uint64_t    value;
};

struct simple_bucket {
  bool                                       is_sorted;
  struct spinlock                            spinlock;
  struct vector of_type(simple_bucket_entry) entries;
  struct vector of_type(size_t)              freelist;
  size_t                                    *indicies;
};

struct simple_extra {
  struct vector of_type(simple_bucket) buckets;
};

// ---------------------------------------------------------------------------------------------------------------------
//  PARALLEL
// ---------------------------------------------------------------------------------------------------------------------

struct parallel_extra {

};

/**
 * A single entry inside the list of entires inside a bucket. Each such entry represents a pair inside that
 * list where the key has a hash collision with the key of another entry.
 */
struct NG5_BUCKET_PACKING string_map_entry {
  /**
   * Flag to indiciate whether this entry is free, or in use.
   * This flag is required since the implementation uses an update-in-place strategy which requires lookup
   * operations to skip entries that are physically stored in the list but logically not in use. Whether a
   * particular free entry is taken to write a new entry to it, is however, controlled over the freelist of
   * the parent bucket list (see <code>string_map_bucket_data</code>)
   */
  bool             in_use;
  /**
   * A pointer to the key. This pointer is required to enable a string comparision between a requested key
   * and that particular key at hand.
   */
  const char      *str_key;
  /**
   * A value assigned to that key.
   */
  uint64_t         int_value;
};

/**
 * Data stored in Tier 3 of one single bucket.
 */
struct string_map_bucket_data {
  /**
   * A fixed-length array of entries each with colliding keys. Which entry is in use and which is free
   * is determined via a per-entry flag, and for a fast lookup to find a free place for new elements to add,
   * in a freelist (see below)
   */
  struct string_map_entry  non_cache_entries[NG5_CONFIG_BUCKET_CAPACITY];

  /**
   * A freelist that determines one or more free positions inside the list of entries. An entry is free
   * is it was not used so far, or it was removed. This, however, is additionally indicated by the corresponding
   * flag inside the entry.
   */
  size_t                   non_cache_entries_freelist[NG5_CONFIG_BUCKET_CAPACITY];

  /**
   * The number of free positions left in the freelist
   */
  size_t                   non_cache_entries_freelist_size;

  /**
   * A flag that determines whether the entry list is sorted or not. This flag serves as an effort buffer in
   * case binary search is used to find an requested entry. If binary search is used, the entry list must be sorted.
   * However, this is at most one pre-processing step that is executed each time before a binary search is started.
   * To avoid this effort, after successful sorting, the flag <code>non_cache_entries_sorted</code> is set to
   * <b>true</b>, and the sorting pre-processing is only executed if <code>non_cache_entries_sorted</code> is
   * <b>false</b>. Only in case an <i>insert</i> operation to the entries list that destroys the lists order, this
   * flag is set to <b>false</b> (or in case of fresh initialization, of course).
   */
  bool                     non_cache_entries_sorted;

  /**
   * Flag indicating the current search mode
   */
  enum search_strategy { SEARCH_SCAN_SINGLE, SEARCH_SCAN_MULTI, SEARCH_BSEARCH }
          search_mode;

  /**
   * A pointer to that entry that was last found by the search (scan or binary).
   */
  struct string_map_entry *search_cache;
};

/**
 * Tier 4 for bucket data
 */
struct string_map_swap_space {
  /**
   * Mapping of ids to offsets in the data file
   */
  FILE                    *index_file;

  /**
   * Actual data swapped out to disk
   */
  FILE                    *data_file;
};

struct thread_local_bucket {
  /**
  * Tier 1: A pointer to an element inside this bucket that was read the latest. In case a read operation
  * can be answered by the reference stored in Tier 1, the referred element is always additionally handled in its
  * origin place as it found have been found there in order to let the successful read-operation benefit that element.
  * In simpler words, if the latest object comes from Tier 2, it is additionally moved one position towards the
  * head of the Tier 2 cache. If on the other hand, the latest object comes from Tier 3, it is swapped into the
  * cache as described for Tier 3. An object referred by latest can never be in Tier 4 (see below).
  */
  struct string_map_entry       *latest;

  /**
  * Enumeration to remember where the latest object reference comes from
  */
  enum lates_origin {
    TIER_2, TIER_3
  }                              latest_origin;

  /**
  * Position of the latest element in the Tier 2 cache resp Tier 3 list in order to perform the tier-specific
  * locality optimization (i.e., the move operations inside Tier 2 resp. from Tier 3 to Tier 2 in order to
  * increase spatial locality)
  */
  union {
    size_t                      cache_position;
    size_t                      non_cache_position;
  }                              latest_origin_ref;

  /**
  * Tier 2: The least recently read entries in this bucket. This cache is always read with a forward sequential
  * single threaded scan (i.e., a minimum on synchronization effort, and no pre-processing requirements);
  * for a tiny number of elements, a good strategy. Therefore, the cache size must be set to a reasonable
  * small number. However, to increase the lookup performance even further, a requested and found element in
  * the cache is moved one position towards the first element. If an element was found for a <i>read</i> operation,
  * <code>latest</code> is also updated to this element.
  */
  struct string_map_entry         cache[NG5_CONFIG_BUCKET_CACHE_SIZE];

  /**
  * The actual cache size, in case the number of elements in this bucket is less than the maximum cache size
  */
  size_t                          cache_size;

  /**
  * Tier 3: All entries that are not stored in the cache. The search for an element is done via either a
  * multi-threaded scan or a binary search depending on what is more suitable according to the underlying cost model
  * (which considers the pre-processing effort for binary search and others). In case of a <code>read</code>
  * operation to on of these values, this read value is removed from this entry list and stored in the cache. In case
  * the cache is not yet full (i.e., <code>cache_size<code> is less than the maximum cache size<code>, the element is
  * just stored at thetail of the cache. In cache the cache is full, the element at hand is stored by swapping the
  * last element in the cache into this list, and the read element at hand into the cache. In any case, if the element
  * was found for a <i>read</i> operation, <code>latest</code> is also updated to this element. However, elements
  * newly stored in this map are added to the Tier 3. In case there is not enough space, an element from Tier 3
  * is randomly picked and swapped out into the swap space (see below) and its position is occupied by the new
  * element.
  */
  struct string_map_bucket_data   non_cached_entries;

  /**
  * Tier 4: Entries stored on the disk in a partition-exclusive file (swap space). Entries are swapped out in order
  * to match the maximum memory requirements for this map (in case of adding new elements, or in case swapped-out
  * elements are stored in Tier 3 again (see next). In case Tier 4 is considered for a lookup operation and
  * the lookup finds an entry, this entry in swapped into Tier 3. For this, an element from Tier 3 is randomly
  * picked and swapped out to the position of the element from the swap sapce at hand.
  */
  struct string_map_swap_space    swap_space;
};

/**
 * A 4-tier bucket that realizes a list of key-value pairs where the keys conflict with each other
 */
struct string_map_bucket {
  struct vector of_type(thread_local_bucket) thread_local_buckets;
  struct vector of_type(pthread_t)           threads;
};

/**
 * The StringIdMap data structure
 */
struct parallel_string_id_map
{

  /**
   * Vector of list of 'string_hash_bucket' objects realizing the partition of the map for this particular thread
   */
  struct vector    buckets;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

static int simple_drop(struct string_id_map *self);
static int simple_put(struct string_id_map *self, char *const *keys, const uint64_t *values, size_t num_pairs);
static int simple_get(struct string_id_map *self, uint64_t **out, bool **found_mask, size_t *num_not_found,
                      char *const *keys, size_t num_keys);
static int simple_remove(struct string_id_map *self, char *const *keys, size_t num_keys);
static int simple_free(struct string_id_map *self, void *ptr);

static int simple_create_extra(struct string_id_map *self, float grow_factor, size_t num_buckets, size_t cap_buckets);
static struct simple_extra *simple_extra(struct string_id_map *self);
static int simple_bucket_create(struct simple_bucket *buckets, size_t num_buckets, size_t bucket_cap,
                                float grow_factor, struct allocator *alloc);
static int simple_bucket_drop(struct simple_bucket *buckets, size_t num_buckets, struct allocator *alloc);
static int simple_map_insert(struct vector of_type(simple_bucket)* buckets, char* const* keys,
        const uint64_t* values, size_t* bucket_idxs, size_t num_pairs, struct allocator *alloc);
static void simple_bucket_lock(struct simple_bucket *bucket) force_inline;
static void simple_bucket_unlock(struct simple_bucket *bucket) force_inline;
static int simple_bucket_insert(struct simple_bucket *bucket, const char *key, uint64_t value,
                                struct allocator *alloc) force_inline;
static void simple_bucket_freelist_push(struct simple_bucket *bucket, size_t idx);
static size_t simple_bucket_freelist_pop(struct simple_bucket *bucket, struct allocator *alloc) force_inline;
static size_t simple_bucket_find_entry_by_key(struct simple_bucket *bucket, const char *key, struct allocator *alloc);

inline static bool simple_bucket_cmp_less_entry(const void *lhs, const void *rhs);
inline static bool simple_bucket_cmp_eq_entry(const void *lhs, const void *rhs);
inline static bool simple_bucket_cmp_less_eq_entry(const void *lhs, const void *rhs);


// ---------------------------------------------------------------------------------------------------------------------
//  PARALLEL
// ---------------------------------------------------------------------------------------------------------------------

static int parallel_drop(struct string_id_map *self);
static int parallel_put(struct string_id_map *self, char *const *keys, const uint64_t *values, size_t num_pairs);
static int parallel_get(struct string_id_map *self, uint64_t **out, bool **found_mask, size_t *num_not_found,
        char *const *keys, size_t num_keys);
static int parallel_remove(struct string_id_map *self, char *const *keys, size_t num_keys);
static int parallel_free(struct string_id_map *self, void *ptr);

static int parallel_create_extra(struct string_id_map *self);
static void parallel_free_extra(struct string_id_map *self);
static struct parallel_extra *parallel_extra(struct string_id_map *self);


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

int string_id_map_create_simple(struct string_id_map *map, const struct allocator *alloc, size_t num_buckets,
        size_t cap_buckets, float bucket_grow_factor)
{
    check_success(allocator_this_or_default(&map->allocator, alloc));
    map->tag    = STRING_ID_MAP_SIMPLE;
    map->drop   = simple_drop;
    map->put    = simple_put;
    map->get    = simple_get;
    map->remove = simple_remove;
    map->free   = simple_free;

    check_success(simple_create_extra(map, bucket_grow_factor, num_buckets, cap_buckets));
    return STATUS_OK;
}

static int simple_drop(struct string_id_map *self)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    struct simple_extra *extra = simple_extra(self);
    struct simple_bucket *data = (struct simple_bucket *) vector_data(&extra->buckets);
    check_success(simple_bucket_drop(data, extra->buckets.cap_elems, &self->allocator));
    vector_drop(&extra->buckets);
    allocator_free(&self->allocator, self->extra);
    return STATUS_OK;
}

static int simple_put(struct string_id_map *self, char *const *keys, const uint64_t *values, size_t num_pairs)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_pairs * sizeof(size_t));

    for (size_t i = 0; i < num_pairs; i++) {
        const char *key        = keys[i];
        hash_t      hash       = jenkins_hash(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_insert(&extra->buckets, keys, values, bucket_idxs, num_pairs, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static void sort_bucket_indicies_ifneeded(struct simple_bucket *bucket, struct allocator *alloc) {
    if (!bucket->is_sorted) {
        struct simple_bucket_entry *data = (struct simple_bucket_entry *) vector_data(&bucket->entries);
        qsort_indicies(bucket->indicies, data, sizeof(struct simple_bucket_entry), simple_bucket_cmp_less_eq_entry,
                bucket->entries.cap_elems, alloc);
        bucket->is_sorted = true;
    }
}

static size_t simple_bucket_find_entry_by_key(struct simple_bucket *bucket, const char *key, struct allocator *alloc)
{
    struct simple_bucket_entry *data = (struct simple_bucket_entry *) vector_data(&bucket->entries);
    /* search for update operation */
    sort_bucket_indicies_ifneeded(bucket, alloc);

    struct simple_bucket_entry needle = {
            .str    = key,
            .in_use = true
    };

    size_t needle_pos = binary_search_indicies(bucket->indicies, data, sizeof(struct simple_bucket_entry),
            bucket->entries.cap_elems, &needle, simple_bucket_cmp_eq_entry,
            simple_bucket_cmp_less_entry);

    return needle_pos < bucket->entries.cap_elems ? bucket->indicies[needle_pos] : needle_pos;
}

static int simple_map_fetch(struct vector of_type(simple_bucket) *buckets, uint64_t *values_out, bool *key_found_mask,
                            size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
                            struct allocator *alloc)
{
    size_t num_not_found = 0;
    struct simple_bucket *data = (struct simple_bucket *) vector_data(buckets);

    for (size_t i = 0; i < num_keys; i++) {
        struct simple_bucket       *bucket     = data + bucket_idxs[i];
        simple_bucket_lock(bucket);

        if (i == 333) {
            printf("XXX1");
        }

        const char                 *key        = keys[i];
        size_t                      needle_pos = simple_bucket_find_entry_by_key(bucket, key, alloc);

        bool found         = needle_pos < bucket->entries.cap_elems;
        num_not_found     += found ? 0 : 1;
        key_found_mask[i]  = found;
        values_out[i]      = found ? (((struct simple_bucket_entry *) bucket->entries.base) + needle_pos)->value : -1;

        simple_bucket_unlock(bucket);
    }

    *num_keys_not_found = num_not_found;
    return STATUS_OK;
}

static int simple_get(struct string_id_map *self, uint64_t **out, bool **found_mask, size_t *num_not_found,
                      char *const *keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);

    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    uint64_t *values_out = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    bool *found_mask_out = allocator_malloc(&self->allocator, num_keys * sizeof(bool));

    for (size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = jenkins_hash(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_fetch(&extra->buckets, values_out, found_mask_out, num_not_found, bucket_idxs,
                                   keys, num_keys, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    *out = values_out;
    *found_mask = found_mask_out;
    return STATUS_OK;
}

static int simple_map_remove(struct simple_extra *extra, size_t *bucket_idxs, char *const *keys, size_t num_keys, struct allocator *alloc)
{
    struct simple_bucket *data = (struct simple_bucket *) vector_data(&extra->buckets);

    for (size_t i = 0; i < num_keys; i++) {
        struct simple_bucket* bucket     = data + bucket_idxs[i];
        const char           *key        = keys[i];

        simple_bucket_lock(bucket);
        size_t                needle_pos = simple_bucket_find_entry_by_key(bucket, key, alloc);
        if (likely(needle_pos < bucket->entries.cap_elems)) {
            struct simple_bucket_entry *entry = (struct simple_bucket_entry *) vector_data(&bucket->entries) + needle_pos;
            assert (entry->in_use);
            entry->in_use                     = false;
            simple_bucket_freelist_push(bucket, needle_pos);
            bucket->is_sorted                 = false;
        }
        simple_bucket_unlock(bucket);
    }
    return STATUS_OK;
}

static int simple_remove(struct string_id_map *self, char *const *keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);

    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = jenkins_hash(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_remove(extra, bucket_idxs, keys, num_keys, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static int simple_free(struct string_id_map *self, void *ptr)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    check_success(allocator_free(&self->allocator, ptr));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//  PARALLEL
// ---------------------------------------------------------------------------------------------------------------------


int string_id_map_create_parallel(struct string_id_map *map, const struct allocator *alloc, size_t num_buckets,
        size_t cap_buckets, float map_grow_factor, float bucket_grow_factor,
        size_t max_memory_size, const char *swap_space_dir)
{

    /**
     * The maximum size in byte this map is allowed to occupy during the runtime. The minimum size is the
     * memory footprint of a single bucket map managing having one parition with one entry in each cache level of that
     * single bucket. In case <code>max_memory_size</code> is less than this size, <code>max_memory_size</code>
     * is ignored.
     */
   // size_t max_memory_size;

    check_success(allocator_this_or_default(&map->allocator, alloc));
    map->tag    = STRING_ID_MAP_PARALLEL;
    map->drop   = parallel_drop;
    map->put    = parallel_put;
    map->get    = parallel_get;
    map->remove = parallel_remove;
    map->free   = parallel_free;
    check_success(parallel_create_extra(map));

    unused(num_buckets);
    unused(cap_buckets);
    unused(map_grow_factor);
    unused(bucket_grow_factor);
    unused(max_memory_size);
    unused(swap_space_dir);
    NOT_YET_IMPLEMENTED /* TODO: Implement */

    return STATUS_OK;
}

static int parallel_drop(struct string_id_map *self)
{
    unused(self);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

static int parallel_put(struct string_id_map *self, char *const *keys, const uint64_t *values, size_t num_pairs)
{
    unused(self);
    unused(keys);
    unused(values);
    unused(num_pairs);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

static int parallel_get(struct string_id_map *self, uint64_t **out, bool **found_mask, size_t *num_not_found,
        char *const *keys, size_t num_keys)
{
    unused(self);
    unused(out);
    unused(found_mask);
    unused(num_not_found);
    unused(keys);
    unused(num_keys);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

static int parallel_remove(struct string_id_map *self, char *const *keys, size_t num_keys)
{
    unused(self);
    unused(keys);
    unused(num_keys);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

static int parallel_free(struct string_id_map *self, void *ptr)
{
    unused(self);
    unused(ptr);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

unused_fn
static int simple_create_extra(struct string_id_map *self, float grow_factor, size_t num_buckets, size_t cap_buckets)
{
    if ((self->extra = allocator_malloc(&self->allocator, sizeof(struct simple_extra))) != NULL) {
        struct simple_extra *extra = simple_extra(self);
        vector_create(&extra->buckets, &self->allocator, sizeof(struct simple_bucket), num_buckets);
        struct simple_bucket *data = (struct simple_bucket *) vector_data(&extra->buckets);
        check_success(simple_bucket_create(data, num_buckets, cap_buckets, grow_factor, &self->allocator));
        return STATUS_OK;
    } else {
        return STATUS_MALLOCERR;
    }
}

unused_fn
static struct simple_extra *simple_extra(struct string_id_map *self)
{
    assert (self->tag == STRING_ID_MAP_SIMPLE);
    return (struct simple_extra *)(self->extra);
}

unused_fn
static int simple_bucket_create(struct simple_bucket *buckets, size_t num_buckets, size_t bucket_cap,
                                float grow_factor, struct allocator *alloc)
{
    check_non_null(buckets);

    struct simple_bucket_entry entry = {
            .str     = NULL,
            .in_use  = false,
            .str_len = 0,
            .value   = 0
    };

    while (num_buckets--) {
        struct simple_bucket *bucket = buckets++;
        bucket->is_sorted = false;
        bucket->indicies  = allocator_malloc(alloc, bucket_cap * sizeof(size_t));
        spinlock_create(&bucket->spinlock);
        vector_create(&bucket->entries, alloc, sizeof(struct simple_bucket_entry), bucket_cap);
        vector_create(&bucket->freelist, alloc, sizeof(size_t), bucket_cap);
        vector_set_growfactor(&bucket->entries, grow_factor);
        vector_set_growfactor(&bucket->freelist, grow_factor);
        for (size_t i = 0; i < bucket_cap; i++) {
            bucket->indicies[i] = i;
        }
        vector_push(&bucket->freelist,  bucket->indicies, bucket_cap);
        vector_repreat_push(&bucket->entries, &entry, bucket_cap);
    }

    return STATUS_OK;
}

static int simple_bucket_drop(struct simple_bucket *buckets, size_t num_buckets, struct allocator *alloc)
{
    check_non_null(buckets);

    while (num_buckets--) {
        struct simple_bucket *bucket = buckets++;
        check_success(vector_drop(&bucket->entries));
        check_success(vector_drop(&bucket->freelist));
        check_success(allocator_free(alloc, bucket->indicies));
    }

    return STATUS_OK;
}

static void simple_bucket_lock(struct simple_bucket *bucket)
{
    spinlock_lock(&bucket->spinlock);
}

unused_fn
static void simple_bucket_unlock(struct simple_bucket *bucket)
{
    spinlock_unlock(&bucket->spinlock);
}

unused_fn
static void simple_bucket_freelist_push(struct simple_bucket *bucket, size_t idx)
{
    struct vector of_type(size_t)              *freelist = &bucket->freelist;
    assert (freelist->num_elems + 1 < freelist->cap_elems);
    vector_push(freelist, &idx, 1);
}

unused_fn
static size_t simple_bucket_freelist_pop(struct simple_bucket *bucket, struct allocator *alloc)
{
    struct vector of_type(size_t)              *freelist = &bucket->freelist;
    struct vector of_type(simple_bucket_entry) *entries  = &bucket->entries;

    struct simple_bucket_entry empty = {
        .in_use  = false,
        .str     = NULL,
        .str_len = 0,
        .value   = 0
    };

    if (unlikely(vector_is_empty(freelist))) {
        size_t new_slots;
        size_t last_slot = entries->cap_elems;
        check_success(vector_grow(&new_slots, freelist));
        check_success(vector_grow(NULL, entries));
        bucket->indicies = allocator_realloc(alloc, bucket->indicies, entries->cap_elems * sizeof(size_t));
        size_t *new_slot_ids = allocator_malloc(alloc, new_slots * sizeof(size_t));
        for (size_t slot = 0; slot < new_slots; slot++) {
            size_t new_slot_id = last_slot + slot;
            new_slot_ids[slot] = new_slot_id;
            bucket->indicies[last_slot + slot] = new_slot_id;
            check_success(vector_push(entries, &empty, 1));
        }
        check_success(vector_push(freelist, new_slot_ids, new_slots));
        check_success(allocator_free(alloc, new_slot_ids));
    }
    assert (!vector_is_empty(freelist));
    assert (vector_cap(freelist) == vector_cap(entries));

    size_t result = *(size_t *)vector_pop(freelist);
    return result;
}

inline static bool simple_bucket_cmp_less_entry(const void *lhs, const void *rhs)
{

    struct simple_bucket_entry *a = (struct simple_bucket_entry *) lhs;
    struct simple_bucket_entry *b = (struct simple_bucket_entry *) rhs;

    if (!a->in_use) {
        return false;
    } else if (!b->in_use) {
        return true;
    } else if (!a->in_use && !b->in_use) {
        return true;
    } else {
        return strcmp(a->str, b->str) < 0;
    }
}

inline static bool simple_bucket_cmp_eq_entry(const void *lhs, const void *rhs)
{
    struct simple_bucket_entry *a = (struct simple_bucket_entry *) lhs;
    struct simple_bucket_entry *b = (struct simple_bucket_entry *) rhs;
    if (!a->in_use) {
        return false;
    } else if (!b->in_use) {
        return true;
    } else if (!a->in_use && !b->in_use) {
        return true;
    } else {
        return strcmp(a->str, b->str) == 0;
    }
}

inline static bool simple_bucket_cmp_less_eq_entry(const void *lhs, const void *rhs)
{
    return simple_bucket_cmp_less_entry(lhs, rhs) || simple_bucket_cmp_eq_entry(lhs, rhs);
}

static int simple_bucket_insert(struct simple_bucket *bucket, const char *key, uint64_t value, struct allocator *alloc)
{
    check_non_null(bucket);
    check_non_null(key);

    simple_bucket_lock(bucket);

    size_t                      needle_pos = simple_bucket_find_entry_by_key(bucket, key, alloc);
    struct simple_bucket_entry *data       = (struct simple_bucket_entry *) vector_data(&bucket->entries);


    if (needle_pos < bucket->entries.cap_elems) {
        /* entry found by key */
        data->value = value;
    } else {
        /* no entry found */
        size_t free_slot = simple_bucket_freelist_pop(bucket, alloc);
        data = (struct simple_bucket_entry *) vector_data(&bucket->entries);
        struct simple_bucket_entry *entry = data + free_slot;
        assert(!entry->in_use);
        entry->in_use  = true;
        entry->str     = key;

        entry->str_len = strlen(key);
        entry->value   = value;

        bucket->is_sorted = false;
    }


    simple_bucket_lock(bucket);
    return STATUS_OK;
}

static int simple_map_insert(struct vector of_type(simple_bucket)* buckets, char* const* keys,
        const uint64_t* values, size_t* bucket_idxs, size_t num_pairs, struct allocator *alloc)
{
    check_non_null(buckets)
    check_non_null(keys)
    check_non_null(values)
    check_non_null(bucket_idxs)

    struct simple_bucket *buckets_data = (struct simple_bucket *) vector_data(buckets);
    int status = STATUS_OK;
    for (size_t i = 0; status == STATUS_OK && i < num_pairs; i++) {
        size_t       bucket_idx      = bucket_idxs[i];
        const char  *key             = keys[i];
        uint64_t     value           = values[i];
        if (value == 1333) {
            printf("XXX");
        }

        struct simple_bucket *bucket = buckets_data + bucket_idx;
        status = simple_bucket_insert(bucket, key, value, alloc);
    }

    return status;
}


// ---------------------------------------------------------------------------------------------------------------------
//  PARALLEL
// ---------------------------------------------------------------------------------------------------------------------

static int parallel_create_extra(struct string_id_map *self)
{
    self->extra = allocator_malloc(&self->allocator, sizeof(struct parallel_extra));
    return self->extra ? STATUS_OK : STATUS_MALLOCERR;
}

unused_fn
static void parallel_free_extra(struct string_id_map *self)
{
    unused(self);
    NOT_YET_IMPLEMENTED /* TODO: Implement */
}

unused_fn
static struct parallel_extra *parallel_extra(struct string_id_map *self)
{
    assert (self->tag == STRING_ID_MAP_PARALLEL);
    return (struct parallel_extra *)(self->extra);
}
