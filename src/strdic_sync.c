// file: strdic_async.c

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "vector.h"
#include "spinlock.h"
#include "strhash.h"
#include "strdic_sync.h"
#include "stdlib.h"
#include "strhash_mem.h"
#include "alloc_tracer.h"
#include "parallel.h"
#include "time.h"
#include "bloomfilter.h"

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char                               *str;
    bool                                in_use;
};

struct naive_extra {
    Vector ofType(entry)        contents;
    Vector ofType(string_id_t)  freelist;
    struct string_map                index;
    struct ng5_spinlock                     lock;
};

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int this_drop(struct Dictionary *self);
static int this_insert(struct Dictionary *self, StringId **out, char * const*strings, size_t num_strings,
        size_t nthreads);
static int this_remove(struct Dictionary *self, StringId *strings, size_t num_strings);
static int this_locate_safe(struct Dictionary* self, StringId** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys);
static int this_locate_fast(struct Dictionary* self, StringId** out, char* const* keys,
        size_t num_keys);
static char **this_extract(struct Dictionary *self, const StringId *ids, size_t num_ids);
static int this_free(struct Dictionary *self, void *ptr);

static int this_reset_counters(struct Dictionary *self);
static int this_counters(struct Dictionary *self, struct string_map_counters *counters);

static int this_num_distinct(struct Dictionary *self, size_t *num);

static void lock(struct Dictionary *self);
static void unlock(struct Dictionary *self);

static int extra_create(struct Dictionary *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads);
static struct naive_extra *this_extra(struct Dictionary *self);

static int freelist_pop(StringId *out, struct Dictionary *self);
static int freelist_push(struct Dictionary *self, StringId idx);

int string_dic_create_sync(struct Dictionary* dic, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads, const Allocator* alloc)
{
    CHECK_NON_NULL(dic);

    CHECK_SUCCESS(AllocatorThisOrDefault(&dic->alloc, alloc));

    dic->tag            = STRING_DIC_NAIVE;
    dic->drop           = this_drop;
    dic->insert         = this_insert;
    dic->remove         = this_remove;
    dic->locate_safe    = this_locate_safe;
    dic->locate_fast    = this_locate_fast;
    dic->extract        = this_extract;
    dic->free           = this_free;
    dic->reset_counters = this_reset_counters;
    dic->counters       = this_counters;
    dic->num_distinct   = this_num_distinct;

    CHECK_SUCCESS(extra_create(dic, capacity, num_index_buckets, num_index_bucket_cap, nthreads));
    return STATUS_OK;
}

static void lock(struct Dictionary *self)
{
    assert(self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    ng5_spinlock_lock(&extra->lock);
}

static void unlock(struct Dictionary *self)
{
    assert(self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    ng5_spinlock_unlock(&extra->lock);
}

static int extra_create(struct Dictionary *self, size_t capacity, size_t num_index_buckets,
        size_t num_index_bucket_cap, size_t nthreads)
{
    self->extra = AllocatorMalloc(&self->alloc, sizeof(struct naive_extra));
    struct naive_extra *extra = this_extra(self);
    ng5_spinlock_create(&extra->lock);
    CHECK_SUCCESS(VectorCreate(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    CHECK_SUCCESS(VectorCreate(&extra->freelist, &self->alloc, sizeof(StringId), capacity));
    struct entry empty = {
        .str    = NULL,
        .in_use = false
    };
    for (size_t i = 0; i < capacity; i++) {
        CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        freelist_push(self, i);
    }
    UNUSED(nthreads);

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtable_alloc, &self->alloc));
#endif

    CHECK_SUCCESS(string_hashtable_create_scan1_cache(&extra->index, &hashtable_alloc, num_index_buckets,
              num_index_bucket_cap));
    return STATUS_OK;
}

static struct naive_extra *this_extra(struct Dictionary *self)
{
    assert (self->tag == STRING_DIC_NAIVE);
    return (struct naive_extra *) self->extra;
}

static int freelist_pop(StringId *out, struct Dictionary *self)
{
    assert (self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    if (BRANCH_UNLIKELY(ng5_vector_is_empty(&extra->freelist))) {
        size_t num_new_pos;
        CHECK_SUCCESS(ng5_vector_grow(&num_new_pos, &extra->freelist));
        CHECK_SUCCESS(ng5_vector_grow(NULL, &extra->contents));
        assert (extra->freelist.cap_elems == extra->contents.cap_elems);
        struct entry empty = {
            .in_use = false,
            .str    = NULL
        };
        while (num_new_pos--) {
            size_t new_pos = ng5_vector_len(&extra->contents);
            CHECK_SUCCESS(VectorPush(&extra->freelist, &new_pos, 1));
            CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        }
    }
    *out = *(StringId *) ng5_vector_pop(&extra->freelist);
    return STATUS_OK;
}

static int freelist_push(struct Dictionary *self, StringId idx)
{
    assert (self->tag == STRING_DIC_NAIVE);
    struct naive_extra *extra = this_extra(self);
    CHECK_SUCCESS(VectorPush(&extra->freelist, &idx, 1));
    assert (extra->freelist.cap_elems == extra->contents.cap_elems);
    return STATUS_OK;
}

static int this_drop(struct Dictionary *self)
{
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)

    struct naive_extra *extra = this_extra(self);

    struct entry *entries = (struct entry *) extra->contents.base;
    for (size_t i = 0; i < extra->contents.num_elems; i++) {
        struct entry *entry = entries + i;
        if (entry->in_use) {
            assert (entry->str);
            AllocatorFree(&self->alloc, entry->str);
            entry->str = NULL;
        }
    }

    VectorDrop(&extra->freelist);
    VectorDrop(&extra->contents);
    string_lookup_drop(&extra->index);
    AllocatorFree(&self->alloc, self->extra);

    return STATUS_OK;
}

static int this_insert(struct Dictionary *self, StringId **out, char * const*strings, size_t num_strings,
        size_t nthreads)
{
    TRACE(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", num_strings);
    timestamp_t begin = time_current_time_ms();

    UNUSED(nthreads);

    CHECK_TAG(self->tag, STRING_DIC_NAIVE)
    lock(self);

    struct naive_extra *extra          = this_extra(self);

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtable_alloc, &self->alloc));
#endif


    StringId  *ids_out                  = AllocatorMalloc(&hashtable_alloc, num_strings * sizeof(StringId));
    bool         *found_mask;
    StringId  *values;
    size_t        num_not_found;

    /* query index for strings to get a boolean mask which strings are new and which must be added */
    /* This is for the case that the string dictionary is not empty to skip processing of those new elements
     * which are already contained */
    TRACE(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

    /* NOTE: palatalization of the call to this function decreases performance */
    string_lookup_get_safe_bulk(&values, &found_mask, &num_not_found, &extra->index, strings, num_strings);

    /* OPTIMIZATION: use a bloomfilter to check whether a string (which has not appeared in the
     * dictionary before this batch but might occur multiple times in the current batch) was seen
     * before (with a slight prob. of doing too much work) */
    Bloomfilter bloomfilter;
    BloomfilterCreate(&bloomfilter, 22 * num_not_found);

    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < num_strings; i++) {

        if (found_mask[i]) {
            ids_out[i] = values[i];
        } else {
            /* This path is taken only for strings that are not already contained in the dictionary. However,
             * since this insertion batch may contain duplicate string, querying for already inserted strings
             * must be done anyway for each string in the insertion batch that is inserted. */

            StringId        string_id;
            const char        *key = (const char *)(strings[i]);

            bool               found = false;
            StringId        value;

            /* Query the bloomfilter if the key was already seend. If the filter returns "yes", a lookup
             * is requried since the filter maybe made a mistake. Of the filter returns "no", the
             * key is new for sure. In this case, one can skip the lookup into the buckets. */
            hash_t bloom_key = hash_fnv(strlen(key), key); /* using a hash of a key instead of the string key itself avoids reading the entire string for computing k hashes inside the bloomfilter */
            if (BLOOMFILTER_TEST_AND_SET(&bloomfilter, &bloom_key, sizeof(hash_t))) {
                /* ensure that the string really was seen (due to collisions in the bloom filter the key might not
                 * been actually seen) */

                /* query index for strings to get a boolean mask which strings are new and which must be added */
                /* This is for the case that the string was not already contained in the string dictionary but may have
                 * duplicates in this insertion batch that are already inserted */
                string_lookup_get_safe_exact(&value, &found, &extra->index, key);  /* OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
            }

            if (found) {
                ids_out[i] = value;
            } else {

                /* register in contents list */
                PANIC_IF(freelist_pop(&string_id, self) != STATUS_OK, "slot management broken");
                struct entry *entries = (struct entry *) ng5_vector_data(&extra->contents);
                struct entry *entry   = entries + string_id;
                assert (!entry->in_use);
                entry->in_use         = true;
                entry->str            = strdup(strings[i]);
                ids_out[i]            = string_id;

                /* add for not yet registered pairs to buffer for fast import */
                string_lookup_put_fast_exact(&extra->index, entry->str, string_id);
            }
        }
    }

    /* set potential non-null out parameters */
    OPTIONAL_SET_OR_ELSE(out, ids_out, AllocatorFree(&self->alloc, ids_out));

    /* cleanup */
    AllocatorFree(&hashtable_alloc, found_mask);
    AllocatorFree(&hashtable_alloc, values);
    BloomfilterDrop(&bloomfilter);

    unlock(self);

    timestamp_t end = time_current_time_ms();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

    return STATUS_OK;

}

static int this_remove(struct Dictionary *self, StringId *strings, size_t num_strings)
{
    CHECK_NON_NULL(self);
    CHECK_NON_NULL(strings);
    CHECK_NON_NULL(num_strings);
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)
    lock(self);

    struct naive_extra *extra = this_extra(self);

    size_t num_strings_to_delete = 0;
    char **strings_to_delete = AllocatorMalloc(&self->alloc, num_strings * sizeof(char *));
    StringId *string_ids_to_delete = AllocatorMalloc(&self->alloc, num_strings * sizeof(StringId));

    /* remove strings from contents ng5_vector, and skip duplicates */
    for (size_t i = 0; i < num_strings; i++) {
        StringId string_id = strings[i];
        struct entry *entry   = (struct entry *) ng5_vector_data(&extra->contents) + string_id;
        if (BRANCH_LIKELY(entry->in_use)) {
            strings_to_delete[num_strings_to_delete]    = entry->str;
            string_ids_to_delete[num_strings_to_delete] = strings[i];
            entry->str    = NULL;
            entry->in_use = false;
            num_strings_to_delete++;
            CHECK_SUCCESS(freelist_push(self, string_id));
        }
    }

    /* remove from index */
    CHECK_SUCCESS(string_lookup_remove(&extra->index, strings_to_delete, num_strings_to_delete));

    /* free up resources for strings that should be removed */
    for (size_t i = 0; i < num_strings_to_delete; i++) {
        free (strings_to_delete[i]);
    }

    /* cleanup */
    AllocatorFree(&self->alloc, strings_to_delete);
    AllocatorFree(&self->alloc, string_ids_to_delete);

    unlock(self);
    return STATUS_OK;
}

static int this_locate_safe(struct Dictionary* self, StringId** out, bool** found_mask,
        size_t* num_not_found, char* const* keys, size_t num_keys)
{
    timestamp_t begin = time_current_time_ms();
    TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function invoked for %zu strings", num_keys)

    CHECK_NON_NULL(self);
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(found_mask);
    CHECK_NON_NULL(num_not_found);
    CHECK_NON_NULL(keys);
    CHECK_NON_NULL(num_keys);
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)

    lock(self);
    struct naive_extra *extra = this_extra(self);
    int status = string_lookup_get_safe_bulk(out, found_mask, num_not_found, &extra->index, keys, num_keys);
    unlock(self);

    timestamp_t end = time_current_time_ms();
    UNUSED(begin);
    UNUSED(end);
    TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function done: %f seconds spent here", (end-begin)/1000.0f)

    return status;
}

static int this_locate_fast(struct Dictionary* self, StringId** out, char* const* keys,
        size_t num_keys)
{
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)

    bool   *found_mask;
    size_t  num_not_found;

    /* use safer but in principle more slower implementation */
    int     result         = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /* cleanup */
    this_free(self, found_mask);

    return  result;
}

static char **this_extract(struct Dictionary *self, const StringId *ids, size_t num_ids)
{
    if (BRANCH_UNLIKELY(!self || !ids || num_ids == 0 || self->tag != STRING_DIC_NAIVE)) {
        return NULL;
    }

    lock(self);

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    allocator_TRACE(&hashtable_alloc);
#else
    AllocatorThisOrDefault(&hashtable_alloc, &self->alloc);
#endif

    struct naive_extra *extra = this_extra(self);
    char **result = AllocatorMalloc(&hashtable_alloc, num_ids * sizeof(char *));
    struct entry *entries = (struct entry *) ng5_vector_data(&extra->contents);

    /* Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    ng5_vector_advise(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < num_ids; i++) {
        StringId string_id = ids[i];
        assert(string_id < ng5_vector_len(&extra->contents));
        assert(entries[string_id].in_use);
        result[i] = entries[string_id].str;
    }

    unlock(self);
    return result;
}

static int this_free(struct Dictionary *self, void *ptr)
{
    UNUSED(self);

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtable_alloc, &self->alloc));
#endif

    return AllocatorFree(&hashtable_alloc, ptr);
}

static int this_reset_counters(struct Dictionary *self)
{
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    CHECK_SUCCESS(string_lookup_reset_counters(&extra->index));
    return STATUS_OK;
}

static int this_counters(struct Dictionary *self, struct string_map_counters *counters)
{
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    CHECK_SUCCESS(string_lookup_counters(counters, &extra->index));
    return STATUS_OK;
}

static int this_num_distinct(struct Dictionary *self, size_t *num)
{
    CHECK_TAG(self->tag, STRING_DIC_NAIVE)
    struct naive_extra *extra = this_extra(self);
    *num = ng5_vector_len(&extra->contents);
    return STATUS_OK;
}