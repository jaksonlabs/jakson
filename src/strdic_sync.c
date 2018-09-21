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
#include "strhash_mem.h"
#include "time.h"
#include "bloomfilter.h"

#define STRING_DIC_SYNC_TAG "string-dic-sync"

struct entry {
    char *str;
    bool inUse;
};

typedef struct SyncExtra {
    Vector ofType(entry) contents;
    Vector ofType(stringId_t) freelist;
    struct StringHashTable index;
    Spinlock lock;
} SyncExtra;

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int thisDrop(StringDictionary *self);
static int thisInsert(StringDictionary *self, StringId **out, char *const *strings, size_t numStrings,
                      size_t numThreads);
static int thisRemove(StringDictionary *self, StringId *strings, size_t numStrings);
static int thisLocateSafe(StringDictionary *self, StringId **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys);
static int thisLocateFast(StringDictionary *self, StringId **out, char *const *keys,
                          size_t numKeys);
static char **thisExtract(StringDictionary *self, const StringId *ids, size_t numIds);
static int thisFree(StringDictionary *self, void *ptr);

static int thisResetCounters(StringDictionary *self);
static int thisCounters(StringDictionary *self, StringHashCounters *counters);

static int thisNumDistinct(StringDictionary *self, size_t *num);

static void lock(StringDictionary *self);
static void unlock(StringDictionary *self);

static int createExtra(StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                       size_t numIndexBucketCap, size_t numThreads);
static SyncExtra *thisExtra(StringDictionary *self);

static int freelistPop(StringId *out, StringDictionary *self);
static int freelistPush(StringDictionary *self, StringId idx);

int StringDicationaryCreateSync(StringDictionary *dic, size_t capacity, size_t numIndexBuckets,
                                size_t numIndexBucketCap, size_t numThreads, const Allocator *alloc)
{
    CHECK_NON_NULL(dic);

    CHECK_SUCCESS(AllocatorThisOrDefault(&dic->alloc, alloc));

    dic->tag = STRDIC_SYNC;
    dic->drop = thisDrop;
    dic->insert = thisInsert;
    dic->remove = thisRemove;
    dic->locateSafe = thisLocateSafe;
    dic->locateFast = thisLocateFast;
    dic->extract = thisExtract;
    dic->free = thisFree;
    dic->resetCounters = thisResetCounters;
    dic->counters = thisCounters;
    dic->numDistinct = thisNumDistinct;

    CHECK_SUCCESS(createExtra(dic, capacity, numIndexBuckets, numIndexBucketCap, numThreads));
    return STATUS_OK;
}

static void lock(StringDictionary *self)
{
    assert(self->tag == STRDIC_SYNC);
    SyncExtra *extra = thisExtra(self);
    SpinlockAcquire(&extra->lock);
}

static void unlock(StringDictionary *self)
{
    assert(self->tag == STRDIC_SYNC);
    SyncExtra *extra = thisExtra(self);
    SpinlockRelease(&extra->lock);
}

static int createExtra(StringDictionary *self, size_t capacity, size_t numIndexBuckets,
                       size_t numIndexBucketCap, size_t numThreads)
{
    self->extra = AllocatorMalloc(&self->alloc, sizeof(SyncExtra));
    SyncExtra *extra = thisExtra(self);
    SpinlockCreate(&extra->lock);
    CHECK_SUCCESS(VectorCreate(&extra->contents, &self->alloc, sizeof(struct entry), capacity));
    CHECK_SUCCESS(VectorCreate(&extra->freelist, &self->alloc, sizeof(StringId), capacity));
    struct entry empty = {
        .str    = NULL,
        .inUse = false
    };
    for (size_t i = 0; i < capacity; i++) {
        CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        freelistPush(self, i);
    }
    UNUSED(numThreads);

    Allocator hashtableAlloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtableAlloc, &self->alloc));
#endif

    CHECK_SUCCESS(StringHashtableCreateMem(&extra->index, &hashtableAlloc, numIndexBuckets,
                                           numIndexBucketCap));
    return STATUS_OK;
}

static SyncExtra *thisExtra(StringDictionary *self)
{
    assert (self->tag == STRDIC_SYNC);
    return (SyncExtra *) self->extra;
}

static int freelistPop(StringId *out, StringDictionary *self)
{
    assert (self->tag == STRDIC_SYNC);
    SyncExtra *extra = thisExtra(self);
    if (BRANCH_UNLIKELY(VectorIsEmpty(&extra->freelist))) {
        size_t numNewPos;
        CHECK_SUCCESS(VectorGrow(&numNewPos, &extra->freelist));
        CHECK_SUCCESS(VectorGrow(NULL, &extra->contents));
        assert (extra->freelist.capElems == extra->contents.capElems);
        struct entry empty = {
            .inUse = false,
            .str    = NULL
        };
        while (numNewPos--) {
            size_t new_pos = VectorLength(&extra->contents);
            CHECK_SUCCESS(VectorPush(&extra->freelist, &new_pos, 1));
            CHECK_SUCCESS(VectorPush(&extra->contents, &empty, 1));
        }
    }
    *out = *(StringId *) VectorPop(&extra->freelist);
    return STATUS_OK;
}

static int freelistPush(StringDictionary *self, StringId idx)
{
    assert (self->tag == STRDIC_SYNC);
    SyncExtra *extra = thisExtra(self);
    CHECK_SUCCESS(VectorPush(&extra->freelist, &idx, 1));
    assert (extra->freelist.capElems == extra->contents.capElems);
    return STATUS_OK;
}

static int thisDrop(StringDictionary *self)
{
    CHECK_TAG(self->tag, STRDIC_SYNC)

    SyncExtra *extra = thisExtra(self);

    struct entry *entries = (struct entry *) extra->contents.base;
    for (size_t i = 0; i < extra->contents.numElems; i++) {
        struct entry *entry = entries + i;
        if (entry->inUse) {
            assert (entry->str);
            AllocatorFree(&self->alloc, entry->str);
            entry->str = NULL;
        }
    }

    VectorDrop(&extra->freelist);
    VectorDrop(&extra->contents);
    StringHashTableDrop(&extra->index);
    AllocatorFree(&self->alloc, self->extra);

    return STATUS_OK;
}

static int thisInsert(StringDictionary *self, StringId **out, char *const *strings, size_t numStrings,
                      size_t numThreads)
{
    TRACE(STRING_DIC_SYNC_TAG, "local string dictionary insertion invoked for %zu strings", numStrings);
    Timestamp begin = TimeCurrentSystemTime();

    UNUSED(numThreads);

    CHECK_TAG(self->tag, STRDIC_SYNC)
    lock(self);

    SyncExtra *extra          = thisExtra(self);

    Allocator hashtableAlloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtableAlloc, &self->alloc));
#endif


    StringId  *idsOut = AllocatorMalloc(&hashtableAlloc, numStrings * sizeof(StringId));
    bool *foundMask;
    StringId *values;
    size_t numNotFound;

    /* query index for strings to get a boolean mask which strings are new and which must be added */
    /* This is for the case that the string dictionary is not empty to skip processing of those new elements
     * which are already contained */
    TRACE(STRING_DIC_SYNC_TAG, "local string dictionary check for new strings in insertion bulk%s", "...");

    /* NOTE: palatalization of the call to this function decreases performance */
    StringHashTableGetSafeBulk(&values, &foundMask, &numNotFound, &extra->index, strings, numStrings);

    /* OPTIMIZATION: use a bloomfilter to check whether a string (which has not appeared in the
     * dictionary before this batch but might occur multiple times in the current batch) was seen
     * before (with a slight prob. of doing too much work) */
    Bloomfilter bloomfilter;
    BloomfilterCreate(&bloomfilter, 22 * numNotFound);

    /* copy string ids for already known strings to their result position resp. add those which are new */
    for (size_t i = 0; i < numStrings; i++) {

        if (foundMask[i]) {
            idsOut[i] = values[i];
        } else {
            /* This path is taken only for strings that are not already contained in the dictionary. However,
             * since this insertion batch may contain duplicate string, querying for already inserted strings
             * must be done anyway for each string in the insertion batch that is inserted. */

            StringId stringId;
            const char *key = (const char *)(strings[i]);

            bool found = false;
            StringId value;

            /* Query the bloomfilter if the key was already seend. If the filter returns "yes", a lookup
             * is requried since the filter maybe made a mistake. Of the filter returns "no", the
             * key is new for sure. In this case, one can skip the lookup into the buckets. */
            Hash bloomKey = HashFnv(strlen(key), key); /* using a hash of a key instead of the string key itself avoids reading the entire string for computing k hashes inside the bloomfilter */
            if (BLOOMFILTER_TEST_AND_SET(&bloomfilter, &bloomKey, sizeof(Hash))) {
                /* ensure that the string really was seen (due to collisions in the bloom filter the key might not
                 * been actually seen) */

                /* query index for strings to get a boolean mask which strings are new and which must be added */
                /* This is for the case that the string was not already contained in the string dictionary but may have
                 * duplicates in this insertion batch that are already inserted */
                StringHashTableGetSafeExact(&value, &found, &extra->index, key);  /* OPTIMIZATION: use specialized function for "exact" query to avoid unnessecary malloc calls to manage set of results if only a single result is needed */
            }

            if (found) {
                idsOut[i] = value;
            } else {

                /* register in contents list */
                PANIC_IF(freelistPop(&stringId, self) != STATUS_OK, "slot management broken");
                struct entry *entries = (struct entry *) VectorData(&extra->contents);
                struct entry *entry   = entries + stringId;
                assert (!entry->inUse);
                entry->inUse         = true;
                entry->str            = strdup(strings[i]);
                idsOut[i]            = stringId;

                /* add for not yet registered pairs to buffer for fast import */
                StringHashTablePutFastExact(&extra->index, entry->str, stringId);
            }
        }
    }

    /* set potential non-null out parameters */
    OPTIONAL_SET_OR_ELSE(out, idsOut, AllocatorFree(&self->alloc, idsOut));

    /* cleanup */
    AllocatorFree(&hashtableAlloc, foundMask);
    AllocatorFree(&hashtableAlloc, values);
    BloomfilterDrop(&bloomfilter);

    unlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    INFO(STRING_DIC_SYNC_TAG, "insertion operation done: %f seconds spent here", (end - begin)/1000.0f)

    return STATUS_OK;

}

static int thisRemove(StringDictionary *self, StringId *strings, size_t numStrings)
{
    CHECK_NON_NULL(self);
    CHECK_NON_NULL(strings);
    CHECK_NON_NULL(numStrings);
    CHECK_TAG(self->tag, STRDIC_SYNC)
    lock(self);

    SyncExtra *extra = thisExtra(self);

    size_t numStringsToDelete = 0;
    char **stringsToDelete = AllocatorMalloc(&self->alloc, numStrings * sizeof(char *));
    StringId *stringIdsToDelete = AllocatorMalloc(&self->alloc, numStrings * sizeof(StringId));

    /* remove strings from contents ng5_vector, and skip duplicates */
    for (size_t i = 0; i < numStrings; i++) {
        StringId stringId = strings[i];
        struct entry *entry   = (struct entry *) VectorData(&extra->contents) + stringId;
        if (BRANCH_LIKELY(entry->inUse)) {
            stringsToDelete[numStringsToDelete]    = entry->str;
            stringIdsToDelete[numStringsToDelete] = strings[i];
            entry->str    = NULL;
            entry->inUse = false;
            numStringsToDelete++;
            CHECK_SUCCESS(freelistPush(self, stringId));
        }
    }

    /* remove from index */
    CHECK_SUCCESS(StringHashTableRemove(&extra->index, stringsToDelete, numStringsToDelete));

    /* free up resources for strings that should be removed */
    for (size_t i = 0; i < numStringsToDelete; i++) {
        free (stringsToDelete[i]);
    }

    /* cleanup */
    AllocatorFree(&self->alloc, stringsToDelete);
    AllocatorFree(&self->alloc, stringIdsToDelete);

    unlock(self);
    return STATUS_OK;
}

static int thisLocateSafe(StringDictionary *self, StringId **out, bool **foundMask,
                          size_t *numNotFound, char *const *keys, size_t numKeys)
{
    Timestamp begin = TimeCurrentSystemTime();
    TRACE(STRING_DIC_SYNC_TAG, "'locateSafe' function invoked for %zu strings", numKeys)

    CHECK_NON_NULL(self);
    CHECK_NON_NULL(out);
    CHECK_NON_NULL(foundMask);
    CHECK_NON_NULL(numNotFound);
    CHECK_NON_NULL(keys);
    CHECK_NON_NULL(numKeys);
    CHECK_TAG(self->tag, STRDIC_SYNC)

    lock(self);
    SyncExtra *extra = thisExtra(self);
    int status = StringHashTableGetSafeBulk(out, foundMask, numNotFound, &extra->index, keys, numKeys);
    unlock(self);

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    TRACE(STRING_DIC_SYNC_TAG, "'locateSafe' function done: %f seconds spent here", (end-begin)/1000.0f)

    return status;
}

static int thisLocateFast(StringDictionary *self, StringId **out, char *const *keys,
                          size_t numKeys)
{
    CHECK_TAG(self->tag, STRDIC_SYNC)

    bool   *foundMask;
    size_t  numNotFound;

    /* use safer but in principle more slower implementation */
    int     result = thisLocateSafe(self, out, &foundMask, &numNotFound, keys, numKeys);

    /* cleanup */
    thisFree(self, foundMask);

    return result;
}

static char **thisExtract(StringDictionary *self, const StringId *ids, size_t numIds)
{
    if (BRANCH_UNLIKELY(!self || !ids || numIds == 0 || self->tag != STRDIC_SYNC)) {
        return NULL;
    }

    lock(self);

    Allocator hashtableAlloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    allocatorTrace(&hashtableAlloc);
#else
    AllocatorThisOrDefault(&hashtableAlloc, &self->alloc);
#endif

    SyncExtra *extra = thisExtra(self);
    char **result = AllocatorMalloc(&hashtableAlloc, numIds * sizeof(char *));
    struct entry *entries = (struct entry *) VectorData(&extra->contents);

    /* Optimization: notify the kernel that the content list is accessed randomly (since hash based access)*/
    VectorMemoryAdvice(&extra->contents, MADV_RANDOM | MADV_WILLNEED);

    for (size_t i = 0; i < numIds; i++) {
        StringId stringId = ids[i];
        assert(stringId < VectorLength(&extra->contents));
        assert(entries[stringId].inUse);
        result[i] = entries[stringId].str;
    }

    unlock(self);
    return result;
}

static int thisFree(StringDictionary *self, void *ptr)
{
    UNUSED(self);

    Allocator hashtableAlloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocatorTrace(&hashtableAlloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtableAlloc, &self->alloc));
#endif

    return AllocatorFree(&hashtableAlloc, ptr);
}

static int thisResetCounters(StringDictionary *self)
{
    CHECK_TAG(self->tag, STRDIC_SYNC)
    SyncExtra *extra = thisExtra(self);
    CHECK_SUCCESS(StringHashTableResetCounters(&extra->index));
    return STATUS_OK;
}

static int thisCounters(StringDictionary *self, StringHashCounters *counters)
{
    CHECK_TAG(self->tag, STRDIC_SYNC)
    SyncExtra *extra = thisExtra(self);
    CHECK_SUCCESS(StringHashTableCounters(counters, &extra->index));
    return STATUS_OK;
}

static int thisNumDistinct(StringDictionary *self, size_t *num)
{
    CHECK_TAG(self->tag, STRDIC_SYNC)
    SyncExtra *extra = thisExtra(self);
    *num = VectorLength(&extra->contents);
    return STATUS_OK;
}