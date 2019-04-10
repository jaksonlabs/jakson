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

#include "std/vec.h"
#include "core/async/spin.h"
#include "stdx/strhash.h"
#include "core/encode/encode_sync.h"
#include "core/strhash/strhash_mem.h"
#include "utils/time.h"
#include "std/bloom.h"
#include "hash/fnv.h"
#include "hash/add.h"
#include "hash/xor.h"
#include "hash/rot.h"
#include "hash/sax.h"

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char *str;
    bool in_use;
};

struct sync_extra {
    struct vector ofType(entry) contents;
    struct vector ofType(carbon_string_id_t_t) freelist;
    struct strhash index;
    struct spinlock lock;
};

static bool this_drop(struct strdic *self);
static bool this_insert(struct strdic *self, field_sid_t **out, char *const *strings, size_t num_strings,
                        size_t num_threads);
static bool this_remove(struct strdic *self, field_sid_t *strings, size_t num_strings);
static bool this_locate_safe(struct strdic *self, field_sid_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys);
static bool this_locate_fast(struct strdic *self, field_sid_t **out, char *const *keys,
                          size_t num_keys);
static char **this_extract(struct strdic *self, const field_sid_t *ids, size_t num_ids);
static bool this_free(struct strdic *self, void *ptr);

static bool this_reset_counters(struct strdic *self);
static bool this_counters(struct strdic *self, struct strhash_counters *counters);

static bool this_num_distinct(struct strdic *self, size_t *num);

static bool this_get_contents(struct strdic *self, struct vector ofType (char *) * strings,
                           struct vector ofType(field_sid_t) * string_ids);

static void lock(struct strdic *self);
static void unlock(struct strdic *self);

static int create_extra(struct strdic *self, size_t capacity, size_t num_index_buckets,
                       size_t num_index_bucket_cap, size_t num_threads);
static struct sync_extra *this_extra(struct strdic *self);

static int freelist_pop(field_sid_t *out, struct strdic *self);
static int freelist_push(struct strdic *self, field_sid_t idx);

int carbon_strdic_create_sync(struct strdic *dic, size_t capacity, size_t num_indx_buckets,
                              size_t num_index_bucket_cap, size_t num_threads, const struct allocator *alloc)
{
    NG5_NON_NULL_OR_ERROR(dic);

    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&dic->alloc, alloc));

    dic->tag = SYNC;
    dic->drop = this_drop;
    dic->insert = this_insert;
    dic->remove = this_remove;
    dic->locate_safe = this_locate_safe;
    dic->locate_fast = this_locate_fast;
    dic->extract = this_extract;
    dic->free = this_free;
    dic->resetCounters = this_reset_counters;
    dic->counters = this_counters;
    dic->num_distinct = this_num_distinct;
    dic->get_contents = this_get_contents;

    NG5_CHECK_SUCCESS(create_extra(dic, capacity, num_indx_buckets, num_index_bucket_cap, num_threads));
    return true;
}

static void lock(struct strdic *self)
{
    assert(self->tag == SYNC);
    struct sync_extra *extra = this_extra(self);
    carbon_spinlock_acquire(&extra->lock);
}

static void unlock(struct strdic *self)
{
    assert(self->tag == SYNC);
    struct sync_extra *extra = this_extra(self);
    carbon_spinlock_release(&extra->lock);
}

static int create_extra(struct strdic *self, size_t capacity, size_t num_index_buckets,
                       size_t num_index_bucket_cap, size_t num_threads)
{
    self->extra = carbon_malloc(&self->alloc, sizeof(struct sync_extra));
    struct sync_extra *extra = this_extra(self);
    carbon_spinlock_init(&extra->lock);
    NG5_CHECK_SUCCESS(carbon_vec_create(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    NG5_CHECK_SUCCESS(carbon_vec_create(&extra->freelist, &self->alloc, sizeof(field_sid_t), capacity));
    struct entry empty = {
        .str    = NULL,
        .in_use = false
    };
    for (size_t i = 0; i < capacity; i++) {
        NG5_CHECK_SUCCESS(carbon_vec_push(&extra->contents, &empty, 1));
        freelist_push(self, i);
    }
    NG5_UNUSED(num_threads);

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif

    NG5_CHECK_SUCCESS(carbon_strhash_create_inmemory(&extra->index, &hashtable_alloc, num_index_buckets,
                                                        num_index_bucket_cap));
    return true;
}

static struct sync_extra *this_extra(struct strdic *self)
{
    assert (self->tag == SYNC);
    return (struct sync_extra *) self->extra;
}

static int freelist_pop(field_sid_t *out, struct strdic *self)
{
    assert (self->tag == SYNC);
    struct sync_extra *extra = this_extra(self);
    if (NG5_UNLIKELY(carbon_vec_is_empty(&extra->freelist))) {
        size_t num_new_pos;
        NG5_CHECK_SUCCESS(carbon_vec_grow(&num_new_pos, &extra->freelist));
        NG5_CHECK_SUCCESS(carbon_vec_grow(NULL, &extra->contents));
        assert (extra->freelist.cap_elems == extra->contents.cap_elems);
        struct entry empty = {
            .in_use = false,
            .str    = NULL
        };
        while (num_new_pos--) {
            size_t new_pos = carbon_vec_length(&extra->contents);
            NG5_CHECK_SUCCESS(carbon_vec_push(&extra->freelist, &new_pos, 1));
            NG5_CHECK_SUCCESS(carbon_vec_push(&extra->contents, &empty, 1));
        }
    }
    *out = *(field_sid_t *) carbon_vec_pop(&extra->freelist);
    return true;
}

static int freelist_push(struct strdic *self, field_sid_t idx)
{
    assert (self->tag == SYNC);
    struct sync_extra *extra = this_extra(self);
    NG5_CHECK_SUCCESS(carbon_vec_push(&extra->freelist, &idx, 1));
    assert (extra->freelist.cap_elems == extra->contents.cap_elems);
    return true;
}

static bool this_drop(struct strdic *self)
{
    NG5_CHECK_TAG(self->tag, SYNC)

    struct sync_extra *extra = this_extra(self);

    struct entry *entries = (struct entry *) extra->contents.base;
    for (size_t i = 0; i < extra->contents.num_elems; i++) {
        struct entry *entry = entries + i;
        if (entry->in_use) {
            assert (entry->str);
            carbon_free(&self->alloc, entry->str);
            entry->str = NULL;
        }
    }

    carbon_vec_drop(&extra->freelist);
    carbon_vec_drop(&extra->contents);
    carbon_strhash_drop(&extra->index);
    carbon_free(&self->alloc, self->extra);

    return true;
}

static bool this_insert(struct strdic *self, field_sid_t **out, char *const *strings, size_t num_strings,
                        size_t num_threads)
{
    NG5_TRACE(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", num_strings);
    timestamp_t begin = carbon_time_now_wallclock();

    NG5_UNUSED(num_threads);

    NG5_CHECK_TAG(self->tag, SYNC)
    lock(self);

    struct sync_extra *extra          = this_extra(self);

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif


    field_sid_t  *ids_out = carbon_malloc(&hashtable_alloc, num_strings * sizeof(field_sid_t));
    bool *found_mask;
    field_sid_t *values;
    size_t num_not_found;

    /** query index for strings to get a boolean mask which strings are new and which must be added */
    /** This is for the case that the string dictionary is not empty to skip processing of those new elements
     * which are already contained */
    NG5_TRACE(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

    /** NOTE: palatalization of the call to this function decreases performance */
    carbon_strhash_get_bulk_safe(&values, &found_mask, &num_not_found, &extra->index, strings, num_strings);

    /** OPTIMIZATION: use a bloom_t to check whether a string (which has not appeared in the
     * dictionary before this batch but might occur multiple times in the current batch) was seen
     * before (with a slight prob. of doing too much work) */
    bloom_t bloom_t;
    carbon_bloom_create(&bloom_t, 22 * num_not_found);

    /** copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < num_strings; i++) {

        if (found_mask[i]) {
            ids_out[i] = values[i];
        } else {
            /** This path is taken only for strings that are not already contained in the dictionary. However,
             * since this insertion batch may contain duplicate string, querying for already inserted strings
             * must be done anyway for each string in the insertion batch that is inserted. */

            field_sid_t string_id = 0;
            const char *key = (const char *)(strings[i]);

            bool found = false;
            field_sid_t value;

            /** Query the bloom_t if the keys was already seend. If the filter returns "yes", a lookup
             * is requried since the filter maybe made a mistake. Of the filter returns "no", the
             * keys is new for sure. In this case, one can skip the lookup into the buckets. */
            size_t key_length = strlen(key);
            hash32_t bloom_key = key_length > 0 ? NG5_HASH_FNV(strlen(key), key) : 0; /** using a hash of a keys instead of the string keys itself avoids reading the entire string for computing k hashes inside the bloom_t */
            if (NG5_BLOOM_TEST_AND_SET(&bloom_t, &bloom_key, sizeof(hash32_t))) {
                /** ensure that the string really was seen (due to collisions in the bloom filter the keys might not
                 * been actually seen) */

                /** query index for strings to get a boolean mask which strings are new and which must be added */
                /** This is for the case that the string was not already contained in the string dictionary but may have
                 * duplicates in this insertion batch that are already inserted */
                carbon_strhash_get_bulk_safe_exact(&value, &found, &extra->index, key);  /** OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
            }

            if (found) {
                ids_out[i] = value;
            } else {

                /** register in contents list */
                bool pop_result = freelist_pop(&string_id, self);
                NG5_PRINT_ERROR_AND_DIE_IF(!pop_result, NG5_ERR_SLOTBROKEN)
                struct entry *entries = (struct entry *) carbon_vec_data(&extra->contents);
                struct entry *entry   = entries + string_id;
                assert (!entry->in_use);
                entry->in_use         = true;
                entry->str            = strdup(strings[i]);
                ids_out[i]            = string_id;

                /** add for not yet registered pairs to buffer for fast import */
                carbon_strhash_put_exact_fast(&extra->index, entry->str, string_id);
            }
        }
    }

    /** set potential non-null out parameters */
    NG5_OPTIONAL_SET_OR_ELSE(out, ids_out, carbon_free(&self->alloc, ids_out));

    /** cleanup */
    carbon_free(&hashtable_alloc, found_mask);
    carbon_free(&hashtable_alloc, values);
    carbon_bloom_drop(&bloom_t);

    unlock(self);

    timestamp_t end = carbon_time_now_wallclock();
    NG5_UNUSED(begin);
    NG5_UNUSED(end);
    NG5_INFO(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

    return true;

}

static bool this_remove(struct strdic *self, field_sid_t *strings, size_t num_strings)
{
    NG5_NON_NULL_OR_ERROR(self);
    NG5_NON_NULL_OR_ERROR(strings);
    NG5_NON_NULL_OR_ERROR(num_strings);
    NG5_CHECK_TAG(self->tag, SYNC)
    lock(self);

    struct sync_extra *extra = this_extra(self);

    size_t num_strings_to_delete = 0;
    char **string_to_delete = carbon_malloc(&self->alloc, num_strings * sizeof(char *));
    field_sid_t *string_ids_to_delete =
        carbon_malloc(&self->alloc, num_strings * sizeof(field_sid_t));

    /** remove strings from contents NG5_vector, and skip duplicates */
    for (size_t i = 0; i < num_strings; i++) {
        field_sid_t field_sid_t = strings[i];
        struct entry *entry   = (struct entry *) carbon_vec_data(&extra->contents) + field_sid_t;
        if (NG5_LIKELY(entry->in_use)) {
            string_to_delete[num_strings_to_delete]    = entry->str;
            string_ids_to_delete[num_strings_to_delete] = strings[i];
            entry->str    = NULL;
            entry->in_use = false;
            num_strings_to_delete++;
            NG5_CHECK_SUCCESS(freelist_push(self, field_sid_t));
        }
    }

    /** remove from index */
    NG5_CHECK_SUCCESS(carbon_strhash_remove(&extra->index, string_to_delete, num_strings_to_delete));

    /** free up resources for strings that should be removed */
    for (size_t i = 0; i < num_strings_to_delete; i++) {
        free (string_to_delete[i]);
    }

    /** cleanup */
    carbon_free(&self->alloc, string_to_delete);
    carbon_free(&self->alloc, string_ids_to_delete);

    unlock(self);
    return true;
}

static bool this_locate_safe(struct strdic *self, field_sid_t **out, bool **found_mask,
                          size_t *num_not_found, char *const *keys, size_t num_keys)
{
    timestamp_t begin = carbon_time_now_wallclock();
    NG5_TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function invoked for %zu strings", num_keys)

    NG5_NON_NULL_OR_ERROR(self);
    NG5_NON_NULL_OR_ERROR(out);
    NG5_NON_NULL_OR_ERROR(found_mask);
    NG5_NON_NULL_OR_ERROR(num_not_found);
    NG5_NON_NULL_OR_ERROR(keys);
    NG5_NON_NULL_OR_ERROR(num_keys);
    NG5_CHECK_TAG(self->tag, SYNC)

    lock(self);
    struct sync_extra *extra = this_extra(self);
    int status = carbon_strhash_get_bulk_safe(out, found_mask, num_not_found, &extra->index, keys, num_keys);
    unlock(self);

    timestamp_t end = carbon_time_now_wallclock();
    NG5_UNUSED(begin);
    NG5_UNUSED(end);
    NG5_TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function done: %f seconds spent here", (end-begin)/1000.0f)

    return status;
}

static bool this_locate_fast(struct strdic *self, field_sid_t **out, char *const *keys,
                          size_t num_keys)
{
    NG5_CHECK_TAG(self->tag, SYNC)

    bool   *found_mask;
    size_t  num_not_found;

    /** use safer but in principle more slower implementation */
    int     result = this_locate_safe(self, out, &found_mask, &num_not_found, keys, num_keys);

    /** cleanup */
    this_free(self, found_mask);

    return result;
}

static char **this_extract(struct strdic *self, const field_sid_t *ids, size_t num_ids)
{
    if (NG5_UNLIKELY(!self || !ids || num_ids == 0 || self->tag != SYNC)) {
        return NULL;
    }

    lock(self);

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    allocatorTrace(&hashtable_alloc);
#else
    carbon_alloc_this_or_std(&hashtable_alloc, &self->alloc);
#endif

    struct sync_extra *extra = this_extra(self);
    char **result = carbon_malloc(&hashtable_alloc, num_ids * sizeof(char *));
    struct entry *entries = (struct entry *) carbon_vec_data(&extra->contents);

    /** Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    carbon_vec_memadvice(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < num_ids; i++) {
        field_sid_t field_sid_t = ids[i];
        assert(field_sid_t < carbon_vec_length(&extra->contents));
        assert(field_sid_t == NG5_NULL_ENCODED_STRING || entries[field_sid_t].in_use);
        result[i] = field_sid_t != NG5_NULL_ENCODED_STRING ? entries[field_sid_t].str : NG5_NULL_TEXT;
    }

    unlock(self);
    return result;
}

static bool this_free(struct strdic *self, void *ptr)
{
    NG5_UNUSED(self);

    struct allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtable_alloc));
#else
    NG5_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->alloc));
#endif

    return carbon_free(&hashtable_alloc, ptr);
}

static bool this_reset_counters(struct strdic *self)
{
    NG5_CHECK_TAG(self->tag, SYNC)
    struct sync_extra *extra = this_extra(self);
    NG5_CHECK_SUCCESS(carbon_strhash_reset_counters(&extra->index));
    return true;
}

static bool this_counters(struct strdic *self, struct strhash_counters *counters)
{
    NG5_CHECK_TAG(self->tag, SYNC)
    struct sync_extra *extra = this_extra(self);
    NG5_CHECK_SUCCESS(carbon_strhash_get_counters(counters, &extra->index));
    return true;
}

static bool this_num_distinct(struct strdic *self, size_t *num)
{
    NG5_CHECK_TAG(self->tag, SYNC)
    struct sync_extra *extra = this_extra(self);
    *num = carbon_vec_length(&extra->contents);
    return true;
}

static bool this_get_contents(struct strdic *self, struct vector ofType (char *) * strings,
                           struct vector ofType(field_sid_t) * string_ids)
{
    NG5_CHECK_TAG(self->tag, SYNC);
    struct sync_extra *extra = this_extra(self);

    for (field_sid_t i = 0; i < extra->contents.num_elems; i++) {
        const struct entry *e = vec_get(&extra->contents, i, struct entry);
        if (e->in_use) {
            carbon_vec_push(strings, &e->str, 1);
            carbon_vec_push(string_ids, &i, 1);
        }
    }
    return true;
}