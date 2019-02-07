/*
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
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "carbon/carbon-vector.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-strhash.h"
#include "carbon/strdic/carbon-strdic-sync.h"
#include "carbon/strhash/carbon-strhash-mem.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-bloom.h"

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char *str;
    bool inUse;
};

typedef struct SyncExtra {
    carbon_vec_t ofType(entry) contents;
    carbon_vec_t ofType(carbon_string_id_t_t) freelist;
    carbon_strhash_t index;
    carbon_spinlock_t lock;
} SyncExtra;

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static bool thisDrop(carbon_strdic_t *self);
static bool thisInsert(carbon_strdic_t *self, carbon_string_id_t **out, char *const *strings, size_t numStrings,
                      size_t numThreads);
static bool thisRemove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t numStrings);
static bool thisLocateSafe(carbon_strdic_t *self, carbon_string_id_t **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys);
static bool thisLocateFast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t numKeys);
static char **thisExtract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t numIds);
static bool thisFree(carbon_strdic_t *self, void *ptr);

static bool thisResetCounters(carbon_strdic_t *self);
static bool thisCounters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters);

static bool thisNumDistinct(carbon_strdic_t *self, size_t *num);

static bool thisGetContents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                           carbon_vec_t ofType(carbon_string_id_t) * carbon_string_id_ts);

static void lock(carbon_strdic_t *self);
static void unlock(carbon_strdic_t *self);

static int createExtra(carbon_strdic_t *self, size_t capacity, size_t numIndexBuckets,
                       size_t numIndexBucketCap, size_t numThreads);
static SyncExtra *thisExtra(carbon_strdic_t *self);

static int freelistPop(carbon_string_id_t *out, carbon_strdic_t *self);
static int freelistPush(carbon_strdic_t *self, carbon_string_id_t idx);

int carbon_strdic_create_sync(carbon_strdic_t *dic, size_t capacity, size_t num_indx_buckets,
                              size_t num_index_bucket_cap, size_t num_threads, const carbon_alloc_t *alloc)
{
    CARBON_NON_NULL_OR_ERROR(dic);

    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&dic->alloc, alloc));

    dic->tag = CARBON_STRDIC_TYPE_SYNC;
    dic->drop = thisDrop;
    dic->insert = thisInsert;
    dic->remove = thisRemove;
    dic->locate_safe = thisLocateSafe;
    dic->locate_fast = thisLocateFast;
    dic->extract = thisExtract;
    dic->free = thisFree;
    dic->resetCounters = thisResetCounters;
    dic->counters = thisCounters;
    dic->num_distinct = thisNumDistinct;
    dic->get_contents = thisGetContents;

    CARBON_CHECK_SUCCESS(createExtra(dic, capacity, num_indx_buckets, num_index_bucket_cap, num_threads));
    return true;
}

static void lock(carbon_strdic_t *self)
{
    assert(self->tag == CARBON_STRDIC_TYPE_SYNC);
    SyncExtra *extra = thisExtra(self);
    carbon_spinlock_acquire(&extra->lock);
}

static void unlock(carbon_strdic_t *self)
{
    assert(self->tag == CARBON_STRDIC_TYPE_SYNC);
    SyncExtra *extra = thisExtra(self);
    carbon_spinlock_release(&extra->lock);
}

static int createExtra(carbon_strdic_t *self, size_t capacity, size_t numIndexBuckets,
                       size_t numIndexBucketCap, size_t numThreads)
{
    self->extra = carbon_malloc(&self->alloc, sizeof(SyncExtra));
    SyncExtra *extra = thisExtra(self);
    carbon_spinlock_init(&extra->lock);
    CARBON_CHECK_SUCCESS(VectorCreate(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    CARBON_CHECK_SUCCESS(VectorCreate(&extra->freelist, &self->alloc, sizeof(carbon_string_id_t), capacity));
    struct entry empty = {
        .str    = NULL,
        .inUse = false
    };
    for (size_t i = 0; i < capacity; i++) {
        CARBON_CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        freelistPush(self, i);
    }
    CARBON_UNUSED(numThreads);

    carbon_alloc_t hashtableAlloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtableAlloc, &self->alloc));
#endif

    CARBON_CHECK_SUCCESS(carbon_strhash_create_inmemory(&extra->index, &hashtableAlloc, numIndexBuckets,
                                                        numIndexBucketCap));
    return true;
}

static SyncExtra *thisExtra(carbon_strdic_t *self)
{
    assert (self->tag == CARBON_STRDIC_TYPE_SYNC);
    return (SyncExtra *) self->extra;
}

static int freelistPop(carbon_string_id_t *out, carbon_strdic_t *self)
{
    assert (self->tag == CARBON_STRDIC_TYPE_SYNC);
    SyncExtra *extra = thisExtra(self);
    if (CARBON_BRANCH_UNLIKELY(VectorIsEmpty(&extra->freelist))) {
        size_t numNewPos;
        CARBON_CHECK_SUCCESS(VectorGrow(&numNewPos, &extra->freelist));
        CARBON_CHECK_SUCCESS(VectorGrow(NULL, &extra->contents));
        assert (extra->freelist.capElems == extra->contents.capElems);
        struct entry empty = {
            .inUse = false,
            .str    = NULL
        };
        while (numNewPos--) {
            size_t new_pos = VectorLength(&extra->contents);
            CARBON_CHECK_SUCCESS(VectorPush(&extra->freelist, &new_pos, 1));
            CARBON_CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        }
    }
    *out = *(carbon_string_id_t *) VectorPop(&extra->freelist);
    return true;
}

static int freelistPush(carbon_strdic_t *self, carbon_string_id_t idx)
{
    assert (self->tag == CARBON_STRDIC_TYPE_SYNC);
    SyncExtra *extra = thisExtra(self);
    CARBON_CHECK_SUCCESS(VectorPush(&extra->freelist, &idx, 1));
    assert (extra->freelist.capElems == extra->contents.capElems);
    return true;
}

static bool thisDrop(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)

    SyncExtra *extra = thisExtra(self);

    struct entry *entries = (struct entry *) extra->contents.base;
    for (size_t i = 0; i < extra->contents.numElems; i++) {
        struct entry *entry = entries + i;
        if (entry->inUse) {
            assert (entry->str);
            carbon_free(&self->alloc, entry->str);
            entry->str = NULL;
        }
    }

    VectorDrop(&extra->freelist);
    VectorDrop(&extra->contents);
    carbon_strhash_drop(&extra->index);
    carbon_free(&self->alloc, self->extra);

    return true;
}

static bool thisInsert(carbon_strdic_t *self, carbon_string_id_t **out, char *const *strings, size_t numStrings,
                      size_t numThreads)
{
    CARBON_TRACE(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", numStrings);
    carbon_timestamp_t begin = carbon_time_now_wallclock();

    CARBON_UNUSED(numThreads);

    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)
    lock(self);

    SyncExtra *extra          = thisExtra(self);

    carbon_alloc_t hashtableAlloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtableAlloc, &self->alloc));
#endif


    carbon_string_id_t  *idsOut = carbon_malloc(&hashtableAlloc, numStrings * sizeof(carbon_string_id_t));
    bool *foundMask;
    carbon_string_id_t *values;
    size_t numNotFound;

    /* query index for strings to get a boolean mask which strings are new and which must be added */
    /* This is for the case that the string dictionary is not empty to skip processing of those new elements
     * which are already contained */
    CARBON_TRACE(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

    /* NOTE: palatalization of the call to this function decreases performance */
    carbon_strhash_get_bulk_safe(&values, &foundMask, &numNotFound, &extra->index, strings, numStrings);

    /* OPTIMIZATION: use a carbon_bloom_t to check whether a string (which has not appeared in the
     * dictionary before this batch but might occur multiple times in the current batch) was seen
     * before (with a slight prob. of doing too much work) */
    carbon_bloom_t carbon_bloom_t;
    carbon_bloom_create(&carbon_bloom_t, 22 * numNotFound);

    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < numStrings; i++) {

        if (foundMask[i]) {
            idsOut[i] = values[i];
        } else {
            /* This path is taken only for strings that are not already contained in the dictionary. However,
             * since this insertion batch may contain duplicate string, querying for already inserted strings
             * must be done anyway for each string in the insertion batch that is inserted. */

            carbon_string_id_t string_id = 0;
            const char *key = (const char *)(strings[i]);

            bool found = false;
            carbon_string_id_t value;

            /* Query the carbon_bloom_t if the keys was already seend. If the filter returns "yes", a lookup
             * is requried since the filter maybe made a mistake. Of the filter returns "no", the
             * keys is new for sure. In this case, one can skip the lookup into the buckets. */
            size_t keyLength = strlen(key);
            carbon_hash_t bloomKey = keyLength > 0 ? CARBON_HASH_FNV(strlen(key), key) : 0; /* using a hash of a keys instead of the string keys itself avoids reading the entire string for computing k hashes inside the carbon_bloom_t */
            if (CARBON_BLOOM_TEST_AND_SET(&carbon_bloom_t, &bloomKey, sizeof(carbon_hash_t))) {
                /* ensure that the string really was seen (due to collisions in the bloom filter the keys might not
                 * been actually seen) */

                /* query index for strings to get a boolean mask which strings are new and which must be added */
                /* This is for the case that the string was not already contained in the string dictionary but may have
                 * duplicates in this insertion batch that are already inserted */
                carbon_strhash_get_bulk_safe_exact(&value, &found, &extra->index, key);  /* OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
            }

            if (found) {
                idsOut[i] = value;
            } else {

                /* register in contents list */
                bool pop_result = freelistPop(&string_id, self);
                CARBON_PRINT_ERROR_AND_DIE_IF(!pop_result, CARBON_ERR_SLOTBROKEN)
                struct entry *entries = (struct entry *) VectorData(&extra->contents);
                struct entry *entry   = entries + string_id;
                assert (!entry->inUse);
                entry->inUse         = true;
                entry->str            = strdup(strings[i]);
                idsOut[i]            = string_id;

                /* add for not yet registered pairs to buffer for fast import */
                carbon_strhash_put_exact_fast(&extra->index, entry->str, string_id);
            }
        }
    }

    /* set potential non-null out parameters */
    CARBON_OPTIONAL_SET_OR_ELSE(out, idsOut, carbon_free(&self->alloc, idsOut));

    /* cleanup */
    carbon_free(&hashtableAlloc, foundMask);
    carbon_free(&hashtableAlloc, values);
    carbon_bloom_drop(&carbon_bloom_t);

    unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_INFO(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

    return true;

}

static bool thisRemove(carbon_strdic_t *self, carbon_string_id_t *strings, size_t numStrings)
{
    CARBON_NON_NULL_OR_ERROR(self);
    CARBON_NON_NULL_OR_ERROR(strings);
    CARBON_NON_NULL_OR_ERROR(numStrings);
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)
    lock(self);

    SyncExtra *extra = thisExtra(self);

    size_t numStringsToDelete = 0;
    char **stringsToDelete = carbon_malloc(&self->alloc, numStrings * sizeof(char *));
    carbon_string_id_t *carbon_string_id_tsToDelete =
        carbon_malloc(&self->alloc, numStrings * sizeof(carbon_string_id_t));

    /* remove strings from contents CARBON_vector, and skip duplicates */
    for (size_t i = 0; i < numStrings; i++) {
        carbon_string_id_t carbon_string_id_t = strings[i];
        struct entry *entry   = (struct entry *) VectorData(&extra->contents) + carbon_string_id_t;
        if (CARBON_BRANCH_LIKELY(entry->inUse)) {
            stringsToDelete[numStringsToDelete]    = entry->str;
            carbon_string_id_tsToDelete[numStringsToDelete] = strings[i];
            entry->str    = NULL;
            entry->inUse = false;
            numStringsToDelete++;
            CARBON_CHECK_SUCCESS(freelistPush(self, carbon_string_id_t));
        }
    }

    /* remove from index */
    CARBON_CHECK_SUCCESS(carbon_strhash_remove(&extra->index, stringsToDelete, numStringsToDelete));

    /* free up resources for strings that should be removed */
    for (size_t i = 0; i < numStringsToDelete; i++) {
        free (stringsToDelete[i]);
    }

    /* cleanup */
    carbon_free(&self->alloc, stringsToDelete);
    carbon_free(&self->alloc, carbon_string_id_tsToDelete);

    unlock(self);
    return true;
}

static bool thisLocateSafe(carbon_strdic_t *self, carbon_string_id_t **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys)
{
    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function invoked for %zu strings", numKeys)

    CARBON_NON_NULL_OR_ERROR(self);
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(foundMask);
    CARBON_NON_NULL_OR_ERROR(numNotFound);
    CARBON_NON_NULL_OR_ERROR(keys);
    CARBON_NON_NULL_OR_ERROR(numKeys);
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)

    lock(self);
    SyncExtra *extra = thisExtra(self);
    int status = carbon_strhash_get_bulk_safe(out, foundMask, numNotFound, &extra->index, keys, numKeys);
    unlock(self);

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_TRACE(STRING_DIC_SYNC_TAG, "'locate_safe' function done: %f seconds spent here", (end-begin)/1000.0f)

    return status;
}

static bool thisLocateFast(carbon_strdic_t *self, carbon_string_id_t **out, char *const *keys,
                          size_t numKeys)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)

    bool   *foundMask;
    size_t  numNotFound;

    /* use safer but in principle more slower implementation */
    int     result = thisLocateSafe(self, out, &foundMask, &numNotFound, keys, numKeys);

    /* cleanup */
    thisFree(self, foundMask);

    return result;
}

static char **thisExtract(carbon_strdic_t *self, const carbon_string_id_t *ids, size_t numIds)
{
    if (CARBON_BRANCH_UNLIKELY(!self || !ids || numIds == 0 || self->tag != CARBON_STRDIC_TYPE_SYNC)) {
        return NULL;
    }

    lock(self);

    carbon_alloc_t hashtableAlloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    allocatorTrace(&hashtableAlloc);
#else
    carbon_alloc_this_or_std(&hashtableAlloc, &self->alloc);
#endif

    SyncExtra *extra = thisExtra(self);
    char **result = carbon_malloc(&hashtableAlloc, numIds * sizeof(char *));
    struct entry *entries = (struct entry *) VectorData(&extra->contents);

    /* Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    VectorMemoryAdvice(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < numIds; i++) {
        carbon_string_id_t carbon_string_id_t = ids[i];
        assert(carbon_string_id_t < VectorLength(&extra->contents));
        assert(carbon_string_id_t == CARBON_NULL_ENCODED_STRING || entries[carbon_string_id_t].inUse);
        result[i] = carbon_string_id_t != CARBON_NULL_ENCODED_STRING ? entries[carbon_string_id_t].str : CARBON_NULL_TEXT;
    }

    unlock(self);
    return result;
}

static bool thisFree(carbon_strdic_t *self, void *ptr)
{
    CARBON_UNUSED(self);

    carbon_alloc_t hashtableAlloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtableAlloc, &self->alloc));
#endif

    return carbon_free(&hashtableAlloc, ptr);
}

static bool thisResetCounters(carbon_strdic_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)
    SyncExtra *extra = thisExtra(self);
    CARBON_CHECK_SUCCESS(carbon_strhash_reset_counters(&extra->index));
    return true;
}

static bool thisCounters(carbon_strdic_t *self, carbon_string_hash_counters_t *counters)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)
    SyncExtra *extra = thisExtra(self);
    CARBON_CHECK_SUCCESS(carbon_strhash_get_counters(counters, &extra->index));
    return true;
}

static bool thisNumDistinct(carbon_strdic_t *self, size_t *num)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC)
    SyncExtra *extra = thisExtra(self);
    *num = VectorLength(&extra->contents);
    return true;
}

static bool thisGetContents(carbon_strdic_t *self, carbon_vec_t ofType (char *) * strings,
                           carbon_vec_t ofType(carbon_string_id_t) * carbon_string_id_ts)
{
    CARBON_CHECK_TAG(self->tag, CARBON_STRDIC_TYPE_SYNC);
    SyncExtra *extra = thisExtra(self);

    for (carbon_string_id_t i = 0; i < extra->contents.numElems; i++) {
        const struct entry *e = VECTOR_GET(&extra->contents, i, struct entry);
        if (e->inUse) {
            VectorPush(strings, &e->str, 1);
            VectorPush(carbon_string_id_ts, &i, 1);
        }
    }
    return true;
}