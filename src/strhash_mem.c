// file: strhash_mem.c

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

#include "strhash_mem.h"
#include "spinlock.h"
#include "stdlib.h"
#include "misc.h"
#include "alloc_tracer.h"
#include "time.h"
#include "bloomfilter.h"
#include "slicelist.h"

#define HASHCODE_OF(key)      HashBernstein(strlen(key), key)

#define SMART_MAP_TAG "strhash-mem"

typedef struct Bucket
{
    SliceList sliceList;
} Bucket;

typedef struct MemExtra
{
    Vector ofType(Bucket) buckets;
} MemExtra;

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  P R O T O T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

static int thisDrop(struct StringHashTable *self);
static int thisPutSafeBulk(struct StringHashTable *self,
                           char *const *keys,
                           const StringId *values,
                           size_t numPairs);
static int thisPutFastBulk(struct StringHashTable *self,
                           char *const *keys,
                           const StringId *values,
                           size_t numPairs);
static int thisPutSafeExact(struct StringHashTable *self, const char *key, StringId value);
static int thisPutFastExact(struct StringHashTable *self, const char *key, StringId value);
static int thisGetSafe(struct StringHashTable *self, StringId **out, bool **foundMask, size_t *numNotFound,
                       char *const *keys, size_t numKeys);
static int thisGetSafeExact(struct StringHashTable *self, StringId *out, bool *foundMask, const char *key);
static int thisGetFast(struct StringHashTable *self, StringId **out, char *const *keys, size_t numKeys);
static int thisUpdateKeyFast(struct StringHashTable *self, const StringId *values, char *const *keys,
                             size_t numKeys);
static int thisRemove(struct StringHashTable *self, char *const *keys, size_t numKeys);
static int thisFree(struct StringHashTable *self, void *ptr);

static int thisInsertBulk(Vector ofType(Bucket) *buckets,
                          char *const *restrict keys,
                          const StringId *restrict values,
                          size_t *restrict bucketIdxs,
                          size_t numPairs,
                          Allocator *alloc,
                          StringHashCounters *counter);

static int thisInsertExact(Vector ofType(Bucket) *buckets, const char *restrict key,
                           StringId value, size_t bucketIdx, Allocator *alloc, StringHashCounters *counter);
static int thisFetchBulk(Vector ofType(Bucket) *buckets, StringId *valuesOut,
                         bool *keyFoundMask,
                         size_t *numKeysNotFound, size_t *bucketIdxs, char *const *keys, size_t numKeys,
                         Allocator *alloc, StringHashCounters *counter);
static int thisFetchSingle(Vector ofType(Bucket) *buckets,
                           StringId *valueOut,
                           bool *keyFound,
                           const size_t bucketIdx,
                           const char *key,
                           StringHashCounters *counter);

static int thisCreateExtra(struct StringHashTable *self, size_t numBuckets, size_t capBuckets);
static MemExtra *thisGetExta(struct StringHashTable *self);
static int BucketCreate(Bucket *buckets, size_t numBuckets, size_t bucketCap,
                        Allocator *alloc);
static int BucketDrop(Bucket *buckets, size_t numBuckets, Allocator *alloc);
static int BucketInsert(Bucket *bucket, const char *restrict key, StringId value,
                        Allocator *alloc, StringHashCounters *counter) FORCE_INLINE;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int StringHashtableCreateMem(struct StringHashTable *map, const Allocator *alloc, size_t numBuckets,
                             size_t capBuckets)
{
    CHECK_SUCCESS(AllocatorThisOrDefault(&map->allocator, alloc));

    numBuckets = numBuckets < 1 ? 1 : numBuckets;
    capBuckets = capBuckets < 1 ? 1 : capBuckets;

    map->tag = STRINGHASHTABLE_MEM;
    map->drop = thisDrop;
    map->putSafeBulk = thisPutSafeBulk;
    map->putFastBulk = thisPutFastBulk;
    map->putSafeExact = thisPutSafeExact;
    map->putFastExact = thisPutFastExact;
    map->getSafeBulk = thisGetSafe;
    map->getFast = thisGetFast;
    map->updateKeyFast = thisUpdateKeyFast;
    map->remove = thisRemove;
    map->free = thisFree;
    map->getSafeExact = thisGetSafeExact;

    StringHashTableResetCounters(map);
    CHECK_SUCCESS(thisCreateExtra(map, numBuckets, capBuckets));
    return STATUS_OK;
}

static int thisDrop(struct StringHashTable *self)
{
    assert(self->tag == STRINGHASHTABLE_MEM);
    MemExtra *extra = thisGetExta(self);
    Bucket *data = (Bucket *) VectorData(&extra->buckets);
    CHECK_SUCCESS(BucketDrop(data, extra->buckets.capElems, &self->allocator));
    VectorDrop(&extra->buckets);
    AllocatorFree(&self->allocator, self->extra);
    return STATUS_OK;
}

static int thisPutSafeBulk(struct StringHashTable *self,
                           char *const *keys,
                           const StringId *values,
                           size_t numPairs)
{
    assert(self->tag == STRINGHASHTABLE_MEM);
    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = AllocatorMalloc(&self->allocator, numPairs * sizeof(size_t));

    PREFETCH_WRITE(bucketIdxs);

    for (size_t i = 0; i < numPairs; i++) {
        const char *key = keys[i];
        Hash hash = HASHCODE_OF(key);
        bucketIdxs[i] = hash % extra->buckets.capElems;
    }

    PREFETCH_READ(bucketIdxs);
    PREFETCH_READ(keys);
    PREFETCH_READ(values);

    CHECK_SUCCESS(thisInsertBulk(&extra->buckets, keys, values, bucketIdxs, numPairs, &self->allocator,
                                 &self->counters));
    CHECK_SUCCESS(AllocatorFree(&self->allocator, bucketIdxs));
    return STATUS_OK;
}

static int thisPutSafeExact(struct StringHashTable *self, const char *key, StringId value)
{
    assert(self->tag == STRINGHASHTABLE_MEM);
    MemExtra *extra = thisGetExta(self);

    Hash hash = HASHCODE_OF(key);
    size_t bucketIdx = hash % extra->buckets.capElems;

    PREFETCH_READ(key);

    CHECK_SUCCESS(thisInsertExact(&extra->buckets, key, value, bucketIdx, &self->allocator,
                                  &self->counters));

    return STATUS_OK;
}

static int thisPutFastExact(struct StringHashTable *self, const char *key, StringId value)
{
    return thisPutSafeExact(self, key, value);
}

static int thisPutFastBulk(struct StringHashTable *self,
                           char *const *keys,
                           const StringId *values,
                           size_t numPairs)
{
    return thisPutSafeBulk(self, keys, values, numPairs);
}

static int thisFetchBulk(Vector ofType(Bucket) *buckets, StringId *valuesOut,
                         bool *keyFoundMask,
                         size_t *numKeysNotFound, size_t *bucketIdxs, char *const *keys, size_t numKeys,
                         Allocator *alloc, StringHashCounters *counter)
{
    UNUSED(counter);
    UNUSED(alloc);

    SliceHandle result_handle;
    size_t numNotFound = 0;
    Bucket *data = (Bucket *) VectorData(buckets);

    PREFETCH_WRITE(valuesOut);

    for (size_t i = 0; i < numKeys; i++) {
        Bucket *bucket = data + bucketIdxs[i];
        const char *key = keys[i];

        SliceListLookupByKey(&result_handle, &bucket->sliceList, key);

        numNotFound += result_handle.isContained ? 0 : 1;
        keyFoundMask[i] = result_handle.isContained;
        valuesOut[i] = result_handle.isContained ? result_handle.value : -1;
    }

    *numKeysNotFound = numNotFound;
    return STATUS_OK;
}

static int thisFetchSingle(Vector ofType(Bucket) *buckets,
                           StringId *valueOut,
                           bool *keyFound,
                           const size_t bucketIdx,
                           const char *key,
                           StringHashCounters *counter)
{
    UNUSED(counter);

    SliceHandle handle;
    Bucket *data = (Bucket *) VectorData(buckets);

    PREFETCH_WRITE(valueOut);
    PREFETCH_WRITE(keyFound);

    Bucket *bucket = data + bucketIdx;

    /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    SliceListLookupByKey(&handle, &bucket->sliceList, key);
    *keyFound = !SliceListIsEmpty(&bucket->sliceList) && handle.isContained;
    *valueOut = (*keyFound) ? handle.value : -1;

    return STATUS_OK;
}

static int thisGetSafe(struct StringHashTable *self, StringId **out, bool **foundMask, size_t *numNotFound,
                       char *const *keys, size_t numKeys)
{
    assert(self->tag == STRINGHASHTABLE_MEM);

    Timestamp begin = TimeCurrentSystemTime();
    TRACE(SMART_MAP_TAG, "'get_safe' function invoked for %zu strings", numKeys)

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtable_alloc, &self->allocator));
#endif

    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = AllocatorMalloc(&self->allocator, numKeys * sizeof(size_t));
    StringId *valuesOut = AllocatorMalloc(&self->allocator, numKeys * sizeof(StringId));
    bool *foundMask_out = AllocatorMalloc(&self->allocator, numKeys * sizeof(bool));

    assert(bucketIdxs != NULL);
    assert(valuesOut != NULL);
    assert(foundMask_out != NULL);

    for (register size_t i = 0; i < numKeys; i++) {
        const char *key = keys[i];
        Hash hash = HASHCODE_OF(key);
        bucketIdxs[i] = hash % extra->buckets.capElems;
        PREFETCH_READ((Bucket *) VectorData(&extra->buckets) + bucketIdxs[i]);
    }

    TRACE(SMART_MAP_TAG, "'get_safe' function invoke fetch...for %zu strings", numKeys)
    CHECK_SUCCESS(thisFetchBulk(&extra->buckets, valuesOut, foundMask_out, numNotFound, bucketIdxs,
                                keys, numKeys, &self->allocator, &self->counters));
    CHECK_SUCCESS(AllocatorFree(&self->allocator, bucketIdxs));
    TRACE(SMART_MAP_TAG, "'get_safe' function invok fetch: done for %zu strings", numKeys)

    assert(valuesOut != NULL);
    assert(foundMask_out != NULL);

    *out = valuesOut;
    *foundMask = foundMask_out;

    Timestamp end = TimeCurrentSystemTime();
    UNUSED(begin);
    UNUSED(end);
    TRACE(SMART_MAP_TAG, "'get_safe' function done: %f seconds spent here", (end - begin) / 1000.0f)

    return STATUS_OK;
}

static int thisGetSafeExact(struct StringHashTable *self, StringId *out, bool *foundMask, const char *key)
{
    assert(self->tag == STRINGHASHTABLE_MEM);

    Allocator hashtable_alloc;
#if defined(NG5_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CHECK_SUCCESS(AllocatorThisOrDefault(&hashtable_alloc, &self->allocator));
#endif

    MemExtra *extra = thisGetExta(self);

    Hash hash = HASHCODE_OF(key);
    size_t bucketIdx = hash % extra->buckets.capElems;
    PREFETCH_READ((Bucket *) VectorData(&extra->buckets) + bucketIdx);

    CHECK_SUCCESS(thisFetchSingle(&extra->buckets, out, foundMask, bucketIdx, key, &self->counters));

    return STATUS_OK;
}

static int thisGetFast(struct StringHashTable *self, StringId **out, char *const *keys, size_t numKeys)
{
    bool *foundMask;
    size_t numNotFound;
    int status = thisGetSafe(self, out, &foundMask, &numNotFound, keys, numKeys);
    thisFree(self, foundMask);
    return status;
}

static int thisUpdateKeyFast(struct StringHashTable *self, const StringId *values, char *const *keys,
                             size_t numKeys)
{
    UNUSED(self);
    UNUSED(values);
    UNUSED(keys);
    UNUSED(numKeys);
    return STATUS_NOTIMPL;
}

static int simple_map_remove(MemExtra *extra, size_t *bucketIdxs, char *const *keys, size_t numKeys,
                             Allocator *alloc, StringHashCounters *counter)
{
    UNUSED(counter);
    UNUSED(alloc);

    SliceHandle handle;
    Bucket *data = (Bucket *) VectorData(&extra->buckets);

    for (register size_t i = 0; i < numKeys; i++) {
        Bucket *bucket = data + bucketIdxs[i];
        const char *key = keys[i];

        /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        SliceListLookupByKey(&handle, &bucket->sliceList, key);
        if (BRANCH_LIKELY(handle.isContained)) {
            SliceListRemove(&bucket->sliceList, &handle);
        }
    }
    return STATUS_OK;
}

static int thisRemove(struct StringHashTable *self, char *const *keys, size_t numKeys)
{
    assert(self->tag == STRINGHASHTABLE_MEM);

    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = AllocatorMalloc(&self->allocator, numKeys * sizeof(size_t));
    for (register size_t i = 0; i < numKeys; i++) {
        const char *key = keys[i];
        Hash hash = HASHCODE_OF(key);
        bucketIdxs[i] = hash % extra->buckets.capElems;
    }

    CHECK_SUCCESS(simple_map_remove(extra, bucketIdxs, keys, numKeys, &self->allocator, &self->counters));
    CHECK_SUCCESS(AllocatorFree(&self->allocator, bucketIdxs));
    return STATUS_OK;
}

static int thisFree(struct StringHashTable *self, void *ptr)
{
    assert(self->tag == STRINGHASHTABLE_MEM);
    CHECK_SUCCESS(AllocatorFree(&self->allocator, ptr));
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R  I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

UNUSED_FUNCTION
static int thisCreateExtra(struct StringHashTable *self, size_t numBuckets, size_t capBuckets)
{
    if ((self->extra = AllocatorMalloc(&self->allocator, sizeof(MemExtra))) != NULL) {
        MemExtra *extra = thisGetExta(self);
        VectorCreate(&extra->buckets, &self->allocator, sizeof(Bucket), numBuckets);

        /* Optimization: notify the kernel that the list of buckets are accessed randomly (since hash based access)*/
        VectorMemoryAdvice(&extra->buckets, MADV_RANDOM | MADV_WILLNEED);


        Bucket *data = (Bucket *) VectorData(&extra->buckets);
        CHECK_SUCCESS(BucketCreate(data, numBuckets, capBuckets, &self->allocator));
        return STATUS_OK;
    }
    else {
        return STATUS_MALLOCERR;
    }
}

UNUSED_FUNCTION
static MemExtra *thisGetExta(struct StringHashTable *self)
{
    assert (self->tag == STRINGHASHTABLE_MEM);
    return (MemExtra *) (self->extra);
}

UNUSED_FUNCTION
static int BucketCreate(Bucket *buckets, size_t numBuckets, size_t bucketCap,
                        Allocator *alloc)
{
    CHECK_NON_NULL(buckets);

    // TODO: parallize this!
    while (numBuckets--) {
        Bucket *bucket = buckets++;
        SliceListCreate(&bucket->sliceList, alloc, bucketCap);
    }

    return STATUS_OK;
}

static int BucketDrop(Bucket *buckets, size_t numBuckets, Allocator *alloc)
{
    UNUSED(alloc);
    CHECK_NON_NULL(buckets);

    while (numBuckets--) {
        Bucket *bucket = buckets++;
        SliceListDrop(&bucket->sliceList);
    }

    return STATUS_OK;
}

static int BucketInsert(Bucket *bucket, const char *restrict key, StringId value,
                        Allocator *alloc, StringHashCounters *counter)
{
    UNUSED(counter);
    UNUSED(alloc);

    CHECK_NON_NULL(bucket);
    CHECK_NON_NULL(key);

    SliceHandle handle;

    /* Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    SliceListLookupByKey(&handle, &bucket->sliceList, key);

    if (handle.isContained) {
        /* entry found by key */
        assert(value == handle.value);
        //debug(SMART_MAP_TAG, "debug(SMART_MAP_TAG, \"*** put *** '%s' into bucket [new]\", key);*** put *** '%s' into bucket [already contained]", key);
    }
    else {
        /* no entry found */
        //debug(SMART_MAP_TAG, "*** put *** '%s' into bucket [new]", key);
        SliceListInsert(&bucket->sliceList, (char **) &key, &value, 1);
    }

    return STATUS_OK;
}

static int thisInsertBulk(Vector ofType(Bucket) *buckets,
                          char *const *restrict keys,
                          const StringId *restrict values,
                          size_t *restrict bucketIdxs,
                          size_t numPairs,
                          Allocator *alloc,
                          StringHashCounters *counter)
{
    CHECK_NON_NULL(buckets)
    CHECK_NON_NULL(keys)
    CHECK_NON_NULL(values)
    CHECK_NON_NULL(bucketIdxs)

    Bucket *buckets_data = (Bucket *) VectorData(buckets);
    int status = STATUS_OK;
    for (register size_t i = 0; status == STATUS_OK && i < numPairs; i++) {
        size_t bucketIdx = bucketIdxs[i];
        const char *key = keys[i];
        StringId value = values[i];

        Bucket *bucket = buckets_data + bucketIdx;
        status = BucketInsert(bucket, key, value, alloc, counter);
    }

    return status;
}

static int thisInsertExact(Vector ofType(Bucket) *buckets, const char *restrict key,
                           StringId value, size_t bucketIdx, Allocator *alloc, StringHashCounters *counter)
{
    CHECK_NON_NULL(buckets)
    CHECK_NON_NULL(key)

    Bucket *buckets_data = (Bucket *) VectorData(buckets);
    Bucket *bucket = buckets_data + bucketIdx;
    return BucketInsert(bucket, key, value, alloc, counter);
}
