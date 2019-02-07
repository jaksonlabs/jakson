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

#include "carbon/carbon-slicelist.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

#define CARBON_SLICE_LIST_TAG "slice-list"

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define get_hashcode(key)    CARBON_HASH_ADDITIVE(strlen(key), key)

/* OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/* OPTIMIZATION: find function as macro */
#define SLICE_SCAN(slice, needleHash, needleStr)                                                                       \
({                                                                                                                     \
    CARBON_TRACE(CARBON_SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needleStr);                                     \
    assert(slice);                                                                                                     \
    assert(needleStr);                                                                                                 \
                                                                                                                       \
    register bool continueScan, keysMatch, keyHashsNoMatch, endReached;                                                \
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);                                                 \
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);                   \
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);                   \
    register uint_fast32_t i = 0;                                                                                      \
    if (!cacheHit) {                                                                                                   \
        do {                                                                                                           \
            while ((keyHashsNoMatch = (slice->keyHashColumn[i]!=needleHash)) && i++<slice->numElems) { ; }             \
            endReached    = ((i+1)>slice->numElems);                                                                   \
            keysMatch      = endReached || (!keyHashsNoMatch && (strcmp(slice->keyColumn[i], needleStr)==0));          \
            continueScan  = !endReached && !keysMatch;                                                                 \
            i             += continueScan;                                                                             \
        }                                                                                                              \
        while (continueScan);                                                                                          \
        slice->cacheIdx = !endReached && keysMatch ? i : slice->cacheIdx;                                              \
    }                                                                                                                  \
    cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? i : slice->numElems);                                     \
})

#define SLICE_BESEARCH(slice, needleHash, needleStr)                                                                   \
({                                                                                                                     \
    0; \
})
// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

static void appenderNew(SliceList *list);
static void appenderSeal(Slice *slice);

static void lock(SliceList *list);
static void unlock(SliceList *list);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

CARBON_EXPORT(bool)
SliceListCreate(SliceList *list, const carbon_alloc_t *alloc, size_t sliceCapacity)
{
    CARBON_NON_NULL_OR_ERROR(list)
    CARBON_NON_NULL_OR_ERROR(sliceCapacity)

    carbon_alloc_this_or_std(&list->alloc, alloc);
    carbon_spinlock_init(&list->lock);
    carbon_error_init(&list->err);

    VectorCreate(&list->slices, &list->alloc, sizeof(Slice), sliceCapacity);
    VectorCreate(&list->descriptors, &list->alloc, sizeof(SliceDescriptor), sliceCapacity);
    VectorCreate(&list->filters, &list->alloc, sizeof(carbon_bloom_t), sliceCapacity);
    VectorCreate(&list->bounds, &list->alloc, sizeof(HashBounds), sliceCapacity);

    CARBON_ZERO_MEMORY(VectorData(&list->slices), sliceCapacity * sizeof(Slice));
    CARBON_ZERO_MEMORY(VectorData(&list->descriptors), sliceCapacity * sizeof(SliceDescriptor));
    CARBON_ZERO_MEMORY(VectorData(&list->filters), sliceCapacity * sizeof(carbon_bloom_t));
    CARBON_ZERO_MEMORY(VectorData(&list->bounds), sliceCapacity * sizeof(HashBounds));

    appenderNew(list);

    return true;
}

CARBON_EXPORT(bool)
SliceListDrop(SliceList *list)
{
    CARBON_UNUSED(list);
//    NOT_YET_IMPLEMENTED
    // TODO: implement
    VectorDrop(&list->slices);
    VectorDrop(&list->descriptors);
    VectorDrop(&list->bounds);
    for (size_t i = 0; i < list->filters.numElems; i++) {
        carbon_bloom_t *filter = VECTOR_GET(&list->filters, i, carbon_bloom_t);
        carbon_bloom_drop(filter);
    }
    VectorDrop(&list->filters);
    return true;
}

CARBON_EXPORT(bool)
SliceListIsEmpty(const SliceList *list)
{
    return (VectorIsEmpty(&list->slices));
}

CARBON_EXPORT(bool)
SliceListInsert(SliceList *list, char **strings, carbon_string_id_t *ids, size_t numPairs)
{
    lock(list);

    while (numPairs--) {
        const char *key = *strings++;
        carbon_string_id_t value = *ids++;
        carbon_hash_t keyHash = get_hashcode(key);
        SliceHandle handle;
        int status;

        assert (key);

        /* check whether the keys-values pair is already contained in one slice */
        status = SliceListLookupByKey(&handle, list, key);

        if (status == true) {
            /* pair was found, do not insert it twice */
            assert (value == handle.value);
            continue;
        }
        else {
            /* pair is not found; append it */
            HashBounds *restrict bounds = VECTOR_ALL(&list->bounds, HashBounds);
            carbon_bloom_t *restrict filters = VECTOR_ALL(&list->filters, carbon_bloom_t);
            Slice *restrict slices = VECTOR_ALL(&list->slices, Slice);

            if (list->appenderIdx != 0) { ; // TODO: remove
            }

            Slice *restrict appender = slices + list->appenderIdx;
            carbon_bloom_t *restrict appenderFilter = filters + list->appenderIdx;
            HashBounds *restrict appenderBounds = bounds + list->appenderIdx;

            CARBON_DEBUG(CARBON_SLICE_LIST_TAG,
                  "appender # of elems: %zu, limit: %zu",
                  appender->numElems,
                  SLICE_KEY_COLUMN_MAX_ELEMS);
            assert(appender->numElems < SLICE_KEY_COLUMN_MAX_ELEMS);
            appender->keyColumn[appender->numElems] = key;
            appender->keyHashColumn[appender->numElems] = keyHash;
            appender->carbon_string_id_tColumn[appender->numElems] = value;
            appenderBounds->minHash = appenderBounds->minHash < keyHash ?
                                       appenderBounds->minHash : keyHash;
            appenderBounds->maxHash = appenderBounds->maxHash > keyHash ?
                                       appenderBounds->maxHash : keyHash;
            CARBON_BLOOM_SET(appenderFilter, &keyHash, sizeof(carbon_hash_t));
            appender->numElems++;
            if (CARBON_BRANCH_UNLIKELY(appender->numElems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                appenderSeal(appender);
                appenderNew(list);
            }
        }
    }

    unlock(list);
    return true;
}

CARBON_EXPORT(bool)
SliceListLookupByKey(SliceHandle *handle, SliceList *list, const char *needle)
{
    CARBON_UNUSED(list);
    CARBON_UNUSED(handle);
    CARBON_UNUSED(needle);

    carbon_hash_t keyHash = get_hashcode(needle);
    uint32_t numSlices = VectorLength(&list->slices);

    /* check whether the keys-values pair is already contained in one slice */
    HashBounds *restrict bounds = VECTOR_ALL(&list->bounds, HashBounds);
    carbon_bloom_t *restrict filters = VECTOR_ALL(&list->filters, carbon_bloom_t);
    Slice *restrict slices = VECTOR_ALL(&list->slices, Slice);
    SliceDescriptor *restrict descs = VECTOR_ALL(&list->descriptors, SliceDescriptor);

    for (register uint32_t i = 0; i < numSlices; i++) {
        SliceDescriptor *restrict desc = descs + i;
        HashBounds *restrict bound = bounds + i;
        Slice *restrict slice = slices + i;

        desc->numReadsAll++;

        if (slice->numElems > 0) {
            bool keyHashIn = keyHash >= bound->minHash && keyHash <= bound->maxHash;
            if (keyHashIn) {
                carbon_bloom_t *restrict filter = filters + i;
                bool maybeContained = CARBON_BLOOM_TEST(filter, &keyHash, sizeof(carbon_hash_t));
                if (maybeContained) {
                    CARBON_DEBUG(CARBON_SLICE_LIST_TAG, "CARBON_slice_list_lookup_by_key keys(%s) -> ?", needle);
                    uint32_t pairPosition;

                    switch (slice->strat) {
                    case SLICE_LOOKUP_SCAN:
                        pairPosition = SLICE_SCAN(slice, keyHash, needle);
                        break;
                    case SLICE_LOOKUP_BESEARCH:
                        pairPosition = SLICE_BESEARCH(slice, keyHash, needle);
                        break;
                    default:
                        CARBON_ERROR(&list->err, CARBON_ERR_UNSUPFINDSTRAT)
                        return false;
                    }

                    CARBON_DEBUG(CARBON_SLICE_LIST_TAG,
                          "CARBON_slice_list_lookup_by_key keys(%s) -> pos(%zu in slice #%zu)",
                          needle,
                          pairPosition,
                          i);
                    if (pairPosition < slice->numElems) {
                        /* pair is contained */
                        desc->numReadsHit++;
                        handle->isContained = true;
                        handle->value = slice->carbon_string_id_tColumn[pairPosition];
                        handle->key = needle;
                        handle->container = slice;

                        desc->numReadsHit++;
                        return true;
                    }
                }
                else {
                    /* carbon_bloom_t is sure that pair is not contained */
                    continue;
                }
            }
            else {
                /* keys hash is not inside bounds of hashes in slice */
                continue;
            }
        }
    }

    handle->isContained = false;

    return false;
}

CARBON_EXPORT(bool)
SliceListRemove(SliceList *list, SliceHandle *handle)
{
    CARBON_UNUSED(list);
    CARBON_UNUSED(handle);
    CARBON_NOT_IMPLEMENTED
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static void appenderNew(SliceList *list)
{
    /* ANTI-OPTIMIZATION: madvising sequential access to columns in slice decrease performance */

    /* the slice itself */
    Slice slice = {
        .strat     = SLICE_LOOKUP_SCAN,
        .numElems = 0,
        .cacheIdx = (uint32_t) -1
    };

    uint32_t numSlices = VectorLength(&list->slices);
    VectorPush(&list->slices, &slice, 1);

    assert(SLICE_KEY_COLUMN_MAX_ELEMS > 0);

    /* the descriptor */
    SliceDescriptor desc = {
        .numReadsHit  = 0,
        .numReadsAll  = 0,
    };

    VectorPush(&list->descriptors, &desc, 1);

    /* the lookup guards */
    assert(sizeof(carbon_bloom_t) <= CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
    carbon_bloom_t filter;

    /* NOTE: the size of each carbon_bloom_t lead to a false positive probability of 100%, i.e., number of items in the
     * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
     * in bits minus the header for the carbon_bloom_t) along with the number of used hash functions (4), lead to that
     * probability. However, the reason a carbon_bloom_t is used is to skip slices whch definitively do NOT contain the
     * keys-values pair - and that still works ;) */
    carbon_bloom_create(&filter,
                        (CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(carbon_bloom_t)) * 8);
    VectorPush(&list->filters, &filter, 1);
    HashBounds bounds = {
        .minHash        = (carbon_hash_t) -1,
        .maxHash        = (carbon_hash_t) 0
    };
    VectorPush(&list->bounds, &bounds, 1);

    CARBON_INFO(CARBON_SLICE_LIST_TAG, "created new appender in slice list %p\n\t"
        "# of slices (incl. appender) in total...............: %zu\n\t"
        "Slice target memory size............................: %zuB (%s)\n\t"
        "carbon_bloom_t target memory size......................: %zuB (%s)\n\t"
        "Max # of (keys, hash, string) in appender/slice......: %zu\n\t"
        "Bits used in per-slice carbon_bloom_t..................: %zu\n\t"
        "Prob. of carbon_bloom_t to produce false-positives.....: %f\n\t"
        "Single slice type size..............................: %zuB\n\t"
        "Total slice-list size...............................: %f MiB",
         list,
         list->slices.numElems,
         (size_t) CARBON_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
         CARBON_SLICE_LIST_TARGET_MEMORY_NAME,
         (size_t) CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE,
         CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
         (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
         carbon_bitmap_nbits(&filter),
         (pow(1 - exp(-(double) carbon_bloom_nhashs()
                          / ((double) carbon_bitmap_nbits(&filter) / (double) SLICE_KEY_COLUMN_MAX_ELEMS)),
              carbon_bitmap_nbits(&filter))),
         sizeof(Slice),
         (sizeof(SliceList) + list->slices.numElems
             * (sizeof(Slice) + sizeof(SliceDescriptor) + (sizeof(uint32_t) * list->descriptors.numElems)
                 + sizeof(carbon_bloom_t) + carbon_bitmap_nbits(&filter) / 8 + sizeof(HashBounds))) / 1024.0
             / 1024.0
    );

    /* register new slice as the current appender */
    list->appenderIdx = numSlices;
}

static void appenderSeal(Slice *slice)
{
    CARBON_UNUSED(slice);
    //  slice->cacheIdx = 0;
    //  slice_sort(slice);
    //  slice->strat = SLICE_LOOKUP_BESEARCH;

    // TODO: sealing means sort and then replace 'find' with bsearch or something. Not yet implemented: sealed slices are also search in a linear fashion
}

static void lock(SliceList *list)
{
    carbon_spinlock_acquire(&list->lock);
}

static void unlock(SliceList *list)
{
    carbon_spinlock_release(&list->lock);
}