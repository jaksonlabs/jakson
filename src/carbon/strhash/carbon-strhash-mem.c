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

#include "carbon/strhash/carbon-strhash-mem.h"
#include "carbon/carbon-spinlock.h"
#include "carbon/carbon-sort.h"
#include "carbon/alloc/carbon-alloc_tracer.h"
#include "carbon/carbon-time.h"
#include "carbon/carbon-bloom.h"
#include "carbon/carbon-slicelist.h"

#define HASHCODE_OF(key)      CARBON_HASH_BERNSTEIN(strlen(key), key)

#define SMART_MAP_TAG "strhash-mem"

typedef struct Bucket
{
    SliceList sliceList;
} Bucket;

typedef struct MemExtra
{
    carbon_vec_t ofType(Bucket) buckets;
} MemExtra;

static int this_drop(carbon_strhash_t *self);
static int thisPutSafeBulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t numPairs);
static int thisPutFastBulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t numPairs);
static int thisPutSafeExact(carbon_strhash_t *self, const char *key, carbon_string_id_t value);
static int thisPutFastExact(carbon_strhash_t *self, const char *key, carbon_string_id_t value);
static int thisGetSafe(carbon_strhash_t *self, carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys);
static int thisGetSafeExact(carbon_strhash_t *self, carbon_string_id_t *out, bool *found_mask, const char *key);
static int thisGetFast(carbon_strhash_t *self, carbon_string_id_t **out, char *const *keys, size_t num_keys);
static int thisUpdateKeyFast(carbon_strhash_t *self, const carbon_string_id_t *values, char *const *keys,
                             size_t num_keys);
static int this_remove(carbon_strhash_t *self, char *const *keys, size_t num_keys);
static int this_free(carbon_strhash_t *self, void *ptr);

static int thisInsertBulk(carbon_vec_t ofType(Bucket) *buckets,
                          char *const *restrict keys,
                          const carbon_string_id_t *restrict values,
                          size_t *restrict bucketIdxs,
                          size_t numPairs,
                          carbon_alloc_t *alloc,
                          carbon_string_hash_counters_t *counter);

static int thisInsertExact(carbon_vec_t ofType(Bucket) *buckets, const char *restrict key,
                           carbon_string_id_t value, size_t bucketIdx, carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);
static int thisFetchBulk(carbon_vec_t ofType(Bucket) *buckets, carbon_string_id_t *valuesOut,
                         bool *keyFoundMask,
                         size_t *numKeysNotFound, size_t *bucketIdxs, char *const *keys, size_t num_keys,
                         carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);
static int thisFetchSingle(carbon_vec_t ofType(Bucket) *buckets,
                           carbon_string_id_t *valueOut,
                           bool *keyFound,
                           const size_t bucketIdx,
                           const char *key,
                           carbon_string_hash_counters_t *counter);

static int this_create_extra(carbon_strhash_t *self, size_t numBuckets, size_t capBuckets);
static MemExtra *thisGetExta(carbon_strhash_t *self);
static int BucketCreate(Bucket *buckets, size_t numBuckets, size_t bucketCap,
                        carbon_alloc_t *alloc);
static int BucketDrop(Bucket *buckets, size_t numBuckets, carbon_alloc_t *alloc);
static int BucketInsert(Bucket *bucket, const char *restrict key, carbon_string_id_t value,
                        carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter);

bool carbon_strhash_create_inmemory(carbon_strhash_t *map, const carbon_alloc_t *alloc, size_t num_buckets,
                                    size_t capBuckets)
{
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&map->allocator, alloc));

    num_buckets = num_buckets < 1 ? 1 : num_buckets;
    capBuckets = capBuckets < 1 ? 1 : capBuckets;

    map->tag = CARBON_STRHASH_INMEMORY;
    map->drop = this_drop;
    map->put_bulk_safe = thisPutSafeBulk;
    map->put_bulk_fast = thisPutFastBulk;
    map->put_exact_safe = thisPutSafeExact;
    map->put_exact_fast = thisPutFastExact;
    map->get_bulk_safe = thisGetSafe;
    map->get_fast = thisGetFast;
    map->update_key_fast = thisUpdateKeyFast;
    map->remove = this_remove;
    map->free = this_free;
    map->get_exact_safe = thisGetSafeExact;
    carbon_error_init(&map->err);

    carbon_strhash_reset_counters(map);
    CARBON_CHECK_SUCCESS(this_create_extra(map, num_buckets, capBuckets));
    return true;
}

static int this_drop(carbon_strhash_t *self)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    MemExtra *extra = thisGetExta(self);
    Bucket *data = (Bucket *) carbon_vec_data(&extra->buckets);
    CARBON_CHECK_SUCCESS(BucketDrop(data, extra->buckets.capElems, &self->allocator));
    carbon_vec_drop(&extra->buckets);
    carbon_free(&self->allocator, self->extra);
    return true;
}

static int thisPutSafeBulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t numPairs)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = carbon_malloc(&self->allocator, numPairs * sizeof(size_t));

    CARBON_PREFETCH_WRITE(bucketIdxs);

    for (size_t i = 0; i < numPairs; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = HASHCODE_OF(key);
        bucketIdxs[i] = hash % extra->buckets.capElems;
    }

    CARBON_PREFETCH_READ(bucketIdxs);
    CARBON_PREFETCH_READ(keys);
    CARBON_PREFETCH_READ(values);

    CARBON_CHECK_SUCCESS(thisInsertBulk(&extra->buckets, keys, values, bucketIdxs, numPairs, &self->allocator,
                                 &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucketIdxs));
    return true;
}

static int thisPutSafeExact(carbon_strhash_t *self, const char *key, carbon_string_id_t value)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    MemExtra *extra = thisGetExta(self);

    carbon_hash_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucketIdx = hash % extra->buckets.capElems;

    CARBON_PREFETCH_READ(key);

    CARBON_CHECK_SUCCESS(thisInsertExact(&extra->buckets, key, value, bucketIdx, &self->allocator,
                                  &self->counters));

    return true;
}

static int thisPutFastExact(carbon_strhash_t *self, const char *key, carbon_string_id_t value)
{
    return thisPutSafeExact(self, key, value);
}

static int thisPutFastBulk(carbon_strhash_t *self,
                           char *const *keys,
                           const carbon_string_id_t *values,
                           size_t numPairs)
{
    return thisPutSafeBulk(self, keys, values, numPairs);
}

static int thisFetchBulk(carbon_vec_t ofType(Bucket) *buckets, carbon_string_id_t *valuesOut,
                         bool *keyFoundMask,
                         size_t *numKeysNotFound, size_t *bucketIdxs, char *const *keys, size_t num_keys,
                         carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    SliceHandle result_handle;
    size_t num_not_found = 0;
    Bucket *data = (Bucket *) carbon_vec_data(buckets);

    CARBON_PREFETCH_WRITE(valuesOut);

    for (size_t i = 0; i < num_keys; i++) {
        Bucket *bucket = data + bucketIdxs[i];
        const char *key = keys[i];
        if (CARBON_BRANCH_LIKELY(key != NULL)) {
            SliceListLookupByKey(&result_handle, &bucket->sliceList, key);
        } else {
            result_handle.isContained = true;
            result_handle.value = CARBON_NULL_ENCODED_STRING;
        }



        num_not_found += result_handle.isContained ? 0 : 1;
        keyFoundMask[i] = result_handle.isContained;
        valuesOut[i] = result_handle.isContained ? result_handle.value : ((carbon_string_id_t) -1);
    }

    *numKeysNotFound = num_not_found;
    return true;
}

static int thisFetchSingle(carbon_vec_t ofType(Bucket) *buckets,
                           carbon_string_id_t *valueOut,
                           bool *keyFound,
                           const size_t bucketIdx,
                           const char *key,
                           carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);

    SliceHandle handle;
    Bucket *data = (Bucket *) carbon_vec_data(buckets);

    CARBON_PREFETCH_WRITE(valueOut);
    CARBON_PREFETCH_WRITE(keyFound);

    Bucket *bucket = data + bucketIdx;

    /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    SliceListLookupByKey(&handle, &bucket->sliceList, key);
    *keyFound = !SliceListIsEmpty(&bucket->sliceList) && handle.isContained;
    *valueOut = (*keyFound) ? handle.value : ((carbon_string_id_t) -1);

    return true;
}

static int thisGetSafe(carbon_strhash_t *self, carbon_string_id_t **out, bool **found_mask, size_t *num_not_found,
                       char *const *keys, size_t num_keys)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invoked for %zu strings", num_keys)

    carbon_alloc_t hashtable_alloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    carbon_string_id_t *valuesOut = carbon_malloc(&self->allocator, num_keys * sizeof(carbon_string_id_t));
    bool *foundMask_out = carbon_malloc(&self->allocator, num_keys * sizeof(bool));

    assert(bucketIdxs != NULL);
    assert(valuesOut != NULL);
    assert(foundMask_out != NULL);

    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = key && strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
        bucketIdxs[i] = hash % extra->buckets.capElems;
        CARBON_PREFETCH_READ((Bucket *) carbon_vec_data(&extra->buckets) + bucketIdxs[i]);
    }

    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invoke fetch...for %zu strings", num_keys)
    CARBON_CHECK_SUCCESS(thisFetchBulk(&extra->buckets, valuesOut, foundMask_out, num_not_found, bucketIdxs,
                                keys, num_keys, &self->allocator, &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucketIdxs));
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function invok fetch: done for %zu strings", num_keys)

    assert(valuesOut != NULL);
    assert(foundMask_out != NULL);

    *out = valuesOut;
    *found_mask = foundMask_out;

    carbon_timestamp_t end = carbon_time_now_wallclock();
    CARBON_UNUSED(begin);
    CARBON_UNUSED(end);
    CARBON_TRACE(SMART_MAP_TAG, "'get_safe' function done: %f seconds spent here", (end - begin) / 1000.0f)

    return true;
}

static int thisGetSafeExact(carbon_strhash_t *self, carbon_string_id_t *out, bool *found_mask, const char *key)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    carbon_alloc_t hashtable_alloc;
#if defined(CARBON_CONFIG_TRACE_STRING_DIC_ALLOC) && !defined(NDEBUG)
    CHECK_SUCCESS(allocator_TRACE(&hashtable_alloc));
#else
    CARBON_CHECK_SUCCESS(carbon_alloc_this_or_std(&hashtable_alloc, &self->allocator));
#endif

    MemExtra *extra = thisGetExta(self);

    carbon_hash_t hash = strcmp("", key) != 0 ? HASHCODE_OF(key) : 0;
    size_t bucketIdx = hash % extra->buckets.capElems;
    CARBON_PREFETCH_READ((Bucket *) carbon_vec_data(&extra->buckets) + bucketIdx);

    CARBON_CHECK_SUCCESS(thisFetchSingle(&extra->buckets, out, found_mask, bucketIdx, key, &self->counters));

    return true;
}

static int thisGetFast(carbon_strhash_t *self, carbon_string_id_t **out, char *const *keys, size_t num_keys)
{
    bool *found_mask;
    size_t num_not_found;
    int status = thisGetSafe(self, out, &found_mask, &num_not_found, keys, num_keys);
    this_free(self, found_mask);
    return status;
}

static int thisUpdateKeyFast(carbon_strhash_t *self, const carbon_string_id_t *values, char *const *keys,
                             size_t num_keys)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(values);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_keys);
    CARBON_ERROR(&self->err, CARBON_ERR_NOTIMPL);
    carbon_error_print(&self->err);
    return false;
}

static int simple_map_remove(MemExtra *extra, size_t *bucketIdxs, char *const *keys, size_t num_keys,
                             carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    SliceHandle handle;
    Bucket *data = (Bucket *) carbon_vec_data(&extra->buckets);

    for (register size_t i = 0; i < num_keys; i++) {
        Bucket *bucket = data + bucketIdxs[i];
        const char *key = keys[i];

        /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
        SliceListLookupByKey(&handle, &bucket->sliceList, key);
        if (CARBON_BRANCH_LIKELY(handle.isContained)) {
            SliceListRemove(&bucket->sliceList, &handle);
        }
    }
    return true;
}

static int this_remove(carbon_strhash_t *self, char *const *keys, size_t num_keys)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);

    MemExtra *extra = thisGetExta(self);
    size_t *bucketIdxs = carbon_malloc(&self->allocator, num_keys * sizeof(size_t));
    for (register size_t i = 0; i < num_keys; i++) {
        const char *key = keys[i];
        carbon_hash_t hash = HASHCODE_OF(key);
        bucketIdxs[i] = hash % extra->buckets.capElems;
    }

    CARBON_CHECK_SUCCESS(simple_map_remove(extra, bucketIdxs, keys, num_keys, &self->allocator, &self->counters));
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, bucketIdxs));
    return true;
}

static int this_free(carbon_strhash_t *self, void *ptr)
{
    assert(self->tag == CARBON_STRHASH_INMEMORY);
    CARBON_CHECK_SUCCESS(carbon_free(&self->allocator, ptr));
    return true;
}

CARBON_FUNC_UNUSED
static int this_create_extra(carbon_strhash_t *self, size_t numBuckets, size_t capBuckets)
{
    if ((self->extra = carbon_malloc(&self->allocator, sizeof(MemExtra))) != NULL) {
        MemExtra *extra = thisGetExta(self);
        carbon_vec_create(&extra->buckets, &self->allocator, sizeof(Bucket), numBuckets);

        /** Optimization: notify the kernel that the list of buckets are accessed randomly (since hash based access)*/
        carbon_vec_memadvice(&extra->buckets, MADV_RANDOM | MADV_WILLNEED);


        Bucket *data = (Bucket *) carbon_vec_data(&extra->buckets);
        CARBON_CHECK_SUCCESS(BucketCreate(data, numBuckets, capBuckets, &self->allocator));
        return true;
    }
    else {
        CARBON_ERROR(&self->err, CARBON_ERR_MALLOCERR);
        return false;
    }
}

CARBON_FUNC_UNUSED
static MemExtra *thisGetExta(carbon_strhash_t *self)
{
    assert (self->tag == CARBON_STRHASH_INMEMORY);
    return (MemExtra *) (self->extra);
}

CARBON_FUNC_UNUSED
static int BucketCreate(Bucket *buckets, size_t numBuckets, size_t bucketCap,
                        carbon_alloc_t *alloc)
{
    CARBON_NON_NULL_OR_ERROR(buckets);

    // TODO: parallize this!
    while (numBuckets--) {
        Bucket *bucket = buckets++;
        SliceListCreate(&bucket->sliceList, alloc, bucketCap);
    }

    return true;
}

static int BucketDrop(Bucket *buckets, size_t numBuckets, carbon_alloc_t *alloc)
{
    CARBON_UNUSED(alloc);
    CARBON_NON_NULL_OR_ERROR(buckets);

    while (numBuckets--) {
        Bucket *bucket = buckets++;
        SliceListDrop(&bucket->sliceList);
    }

    return true;
}

static int BucketInsert(Bucket *bucket, const char *restrict key, carbon_string_id_t value,
                        carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_UNUSED(counter);
    CARBON_UNUSED(alloc);

    CARBON_NON_NULL_OR_ERROR(bucket);
    CARBON_NON_NULL_OR_ERROR(key);

    SliceHandle handle;

    /** Optimization 1/5: EMPTY GUARD (but before "find" call); if this bucket has no occupied slots, do not perform any lookup and comparison */
    SliceListLookupByKey(&handle, &bucket->sliceList, key);

    if (handle.isContained) {
        /** entry found by keys */
        assert(value == handle.value);
        //debug(SMART_MAP_TAG, "debug(SMART_MAP_TAG, \"*** put *** '%s' into bucket [new]\", keys);*** put *** '%s' into bucket [already contained]", keys);
    }
    else {
        /** no entry found */
        //debug(SMART_MAP_TAG, "*** put *** '%s' into bucket [new]", keys);
        SliceListInsert(&bucket->sliceList, (char **) &key, &value, 1);
    }

    return true;
}

static int thisInsertBulk(carbon_vec_t ofType(Bucket) *buckets,
                          char *const *restrict keys,
                          const carbon_string_id_t *restrict values,
                          size_t *restrict bucketIdxs,
                          size_t numPairs,
                          carbon_alloc_t *alloc,
                          carbon_string_hash_counters_t *counter)
{
    CARBON_NON_NULL_OR_ERROR(buckets)
    CARBON_NON_NULL_OR_ERROR(keys)
    CARBON_NON_NULL_OR_ERROR(values)
    CARBON_NON_NULL_OR_ERROR(bucketIdxs)

    Bucket *buckets_data = (Bucket *) carbon_vec_data(buckets);
    int status = true;
    for (register size_t i = 0; status == true && i < numPairs; i++) {
        size_t bucketIdx = bucketIdxs[i];
        const char *key = keys[i];
        carbon_string_id_t value = values[i];

        Bucket *bucket = buckets_data + bucketIdx;
        status = BucketInsert(bucket, key, value, alloc, counter);
    }

    return status;
}

static int thisInsertExact(carbon_vec_t ofType(Bucket) *buckets, const char *restrict key,
                           carbon_string_id_t value, size_t bucketIdx, carbon_alloc_t *alloc, carbon_string_hash_counters_t *counter)
{
    CARBON_NON_NULL_OR_ERROR(buckets)
    CARBON_NON_NULL_OR_ERROR(key)

    Bucket *buckets_data = (Bucket *) carbon_vec_data(buckets);
    Bucket *bucket = buckets_data + bucketIdx;
    return BucketInsert(bucket, key, value, alloc, counter);
}
