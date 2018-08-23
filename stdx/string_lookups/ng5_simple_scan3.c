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

#include <stdx/string_lookups/ng5_simple_scan3.h>
#include <stdx/ng5_spinlock.h>
#include <stdlib.h>
#include <stdx/ng5_algorithm.h>

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

struct simple_bucket_entry {
  bool        in_use;
  const char *str;
  size_t      str_len;
  string_id_t    value;
};

struct simple_bucket {
  struct spinlock                            spinlock;
  struct ng5_vector of_type(simple_bucket_entry) entries;
  struct ng5_vector of_type(size_t)              freelist;
};

struct simple_extra {
  struct ng5_vector of_type(simple_bucket) buckets;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

static int simple_drop(struct string_map *self);
static int simple_put_test(struct string_map *self, char *const *keys, const string_id_t *values, size_t num_pairs);
static int simple_put_blind(struct string_map *self, char *const *keys, const string_id_t *values, size_t num_pairs);
static int simple_get_test(struct string_map *self, string_id_t **out, bool **found_mask, size_t *num_not_found,
        char *const *keys, size_t num_keys);
static int simple_get_blind(struct string_map *self, string_id_t **out, char *const *keys, size_t num_keys);
static int simple_update_key_blind(struct string_map *self, const string_id_t *values, char *const *keys, size_t num_keys);
static int simple_remove(struct string_map *self, char *const *keys, size_t num_keys);
static int simple_free(struct string_map *self, void *ptr);

static int simple_create_extra(struct string_map *self, float grow_factor, size_t num_buckets, size_t cap_buckets);
static struct simple_extra *simple_extra(struct string_map *self);
static int simple_bucket_create(struct simple_bucket *buckets, size_t num_buckets, size_t bucket_cap,
        float grow_factor, ng5_allocator_t *alloc);
static int simple_bucket_drop(struct simple_bucket *buckets, size_t num_buckets, ng5_allocator_t *alloc);
static int simple_map_insert(struct ng5_vector of_type(simple_bucket)* buckets, char* const* keys,
        const string_id_t* values, size_t* bucket_idxs, size_t num_pairs, ng5_allocator_t *alloc);
static void simple_bucket_lock(struct simple_bucket *bucket) force_inline;
static void simple_bucket_unlock(struct simple_bucket *bucket) force_inline;
static int simple_bucket_insert(struct simple_bucket *bucket, const char *key, string_id_t value,
        ng5_allocator_t *alloc) force_inline;
static void simple_bucket_freelist_push(struct simple_bucket *bucket, size_t idx);
static size_t simple_bucket_freelist_pop(struct simple_bucket *bucket, ng5_allocator_t *alloc) force_inline;
static size_t simple_bucket_find_entry_by_key(struct simple_bucket *bucket, const char *key, ng5_allocator_t *alloc);


// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  SIMPLE
// ---------------------------------------------------------------------------------------------------------------------

int string_hashtable_create_scan3(struct string_map* map, const ng5_allocator_t* alloc, size_t num_buckets,
        size_t cap_buckets, float bucket_grow_factor)
{
    check_success(allocator_this_or_default(&map->allocator, alloc));
    map->tag              = STRING_ID_MAP_SIMPLE;
    map->drop             = simple_drop;
    map->put_safe         = simple_put_test;
    map->put_fast        = simple_put_blind;
    map->get_safe         = simple_get_test;
    map->get_fast        = simple_get_blind;
    map->update_key_fast = simple_update_key_blind;
    map->remove           = simple_remove;
    map->free             = simple_free;

    check_success(simple_create_extra(map, bucket_grow_factor, num_buckets, cap_buckets));
    return STATUS_OK;
}

static int simple_drop(struct string_map *self)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    struct simple_extra *extra = simple_extra(self);
    struct simple_bucket *data = (struct simple_bucket *) vector_data(&extra->buckets);
    check_success(simple_bucket_drop(data, extra->buckets.cap_elems, &self->allocator));
    vector_drop(&extra->buckets);
    allocator_free(&self->allocator, self->extra);
    return STATUS_OK;
}

static int simple_put_test(struct string_map* self, char* const* keys, const string_id_t* values, size_t num_pairs)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_pairs * sizeof(size_t));

    for (size_t i = 0; i < num_pairs; i++) {
        const char *key        = keys[i];
        hash_t      hash       = hash_jenkins(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_insert(&extra->buckets, keys, values, bucket_idxs, num_pairs, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static int simple_put_blind(struct string_map *self, char *const *keys, const string_id_t *values, size_t num_pairs)
{
    return simple_put_test(self, keys, values, num_pairs);
}

static size_t simple_bucket_find_entry_by_key(struct simple_bucket *bucket, const char *key, ng5_allocator_t *alloc)
{
    unused(alloc);

    struct simple_bucket_entry *data = (struct simple_bucket_entry *) vector_data(&bucket->entries);
    size_t key_str_len = strlen(key);

    bool found = false;
    size_t i;
    for (i = 0; i < bucket->entries.cap_elems && !found; i++) {
        found = (data[i].in_use && data[i].str_len == key_str_len && strcmp(data[i].str, key) == 0);
    }

    return found ? (i - 1) : bucket->entries.cap_elems;
}

static int simple_map_fetch(struct ng5_vector of_type(simple_bucket) *buckets, string_id_t *values_out, bool *key_found_mask,
        size_t *num_keys_not_found, size_t *bucket_idxs, char *const *keys, size_t num_keys,
        ng5_allocator_t *alloc)
{
    size_t num_not_found = 0;
    struct simple_bucket *data = (struct simple_bucket *) vector_data(buckets);

    for (size_t i = 0; i < num_keys; i++) {
        struct simple_bucket       *bucket     = data + bucket_idxs[i];
        simple_bucket_lock(bucket);

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

static int simple_get_test(struct string_map* self, string_id_t** out, bool** found_mask, size_t* num_not_found,
        char* const* keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);

    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    string_id_t *values_out = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    bool *found_mask_out = allocator_malloc(&self->allocator, num_keys * sizeof(bool));

    for (size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = hash_jenkins(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_fetch(&extra->buckets, values_out, found_mask_out, num_not_found, bucket_idxs,
            keys, num_keys, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    *out = values_out;
    *found_mask = found_mask_out;
    return STATUS_OK;
}

static int simple_get_blind(struct string_map *self, string_id_t **out, char *const *keys, size_t num_keys)
{
    bool* found_mask;
    size_t num_not_found;
    int status = simple_get_test(self, out, &found_mask, &num_not_found, keys, num_keys);
    simple_free(self, &found_mask);
    return status;
}

static int simple_update_key_blind(struct string_map *self, const string_id_t *values, char *const *keys, size_t num_keys)
{
    unused(self);
    unused(values);
    unused(keys);
    unused(num_keys);
    return STATUS_NOTIMPL;
}

static int simple_map_remove(struct simple_extra *extra, size_t *bucket_idxs, char *const *keys, size_t num_keys, ng5_allocator_t *alloc)
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
        }
        simple_bucket_unlock(bucket);
    }
    return STATUS_OK;
}

static int simple_remove(struct string_map *self, char *const *keys, size_t num_keys)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);

    struct simple_extra *extra = simple_extra(self);
    size_t *bucket_idxs = allocator_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (size_t i = 0; i < num_keys; i++) {
        const char *key        = keys[i];
        hash_t      hash       = hash_jenkins(strlen(key), key);
        bucket_idxs[i]         = hash % extra->buckets.cap_elems;
    }

    check_success(simple_map_remove(extra, bucket_idxs, keys, num_keys, &self->allocator));
    check_success(allocator_free(&self->allocator, bucket_idxs));
    return STATUS_OK;
}

static int simple_free(struct string_map *self, void *ptr)
{
    assert(self->tag == STRING_ID_MAP_SIMPLE);
    check_success(allocator_free(&self->allocator, ptr));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

unused_fn
static int simple_create_extra(struct string_map *self, float grow_factor, size_t num_buckets, size_t cap_buckets)
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
static struct simple_extra *simple_extra(struct string_map *self)
{
    assert (self->tag == STRING_ID_MAP_SIMPLE);
    return (struct simple_extra *)(self->extra);
}

unused_fn
static int simple_bucket_create(struct simple_bucket *buckets, size_t num_buckets, size_t bucket_cap,
        float grow_factor, ng5_allocator_t *alloc)
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
        spinlock_create(&bucket->spinlock);
        vector_create(&bucket->entries, alloc, sizeof(struct simple_bucket_entry), bucket_cap);
        vector_create(&bucket->freelist, alloc, sizeof(size_t), bucket_cap);
        vector_set_growfactor(&bucket->entries, grow_factor);
        vector_set_growfactor(&bucket->freelist, grow_factor);
        vector_repreat_push(&bucket->entries, &entry, bucket_cap);
    }

    return STATUS_OK;
}

static int simple_bucket_drop(struct simple_bucket *buckets, size_t num_buckets, ng5_allocator_t *alloc)
{
    unused(alloc);
    check_non_null(buckets);

    while (num_buckets--) {
        struct simple_bucket *bucket = buckets++;
        check_success(vector_drop(&bucket->entries));
        check_success(vector_drop(&bucket->freelist));
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
    struct ng5_vector of_type(size_t)              *freelist = &bucket->freelist;
    assert (freelist->num_elems + 1 < freelist->cap_elems);
    vector_push(freelist, &idx, 1);
}

unused_fn
static size_t simple_bucket_freelist_pop(struct simple_bucket *bucket, ng5_allocator_t *alloc)
{
    struct ng5_vector of_type(size_t)              *freelist = &bucket->freelist;
    struct ng5_vector of_type(simple_bucket_entry) *entries  = &bucket->entries;

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
        size_t *new_slot_ids = allocator_malloc(alloc, new_slots * sizeof(size_t));
        for (size_t slot = 0; slot < new_slots; slot++) {
            size_t new_slot_id = last_slot + slot;
            new_slot_ids[slot] = new_slot_id;
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

static int simple_bucket_insert(struct simple_bucket *bucket, const char *key, string_id_t value, ng5_allocator_t *alloc)
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
    }


    simple_bucket_lock(bucket);
    return STATUS_OK;
}

static int simple_map_insert(struct ng5_vector of_type(simple_bucket)* buckets, char* const* keys,
        const string_id_t* values, size_t* bucket_idxs, size_t num_pairs, ng5_allocator_t *alloc)
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
        string_id_t     value           = values[i];

        struct simple_bucket *bucket = buckets_data + bucket_idx;
        status = simple_bucket_insert(bucket, key, value, alloc);
    }

    return status;
}

