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

#include "carbon/carbon-slicelist.h"

#define CARBON_SLICE_LIST_TAG "slice-list"

#define get_hashcode(key)    CARBON_HASH_ADDITIVE(strlen(key), key)

/** OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/** OPTIMIZATION: find function as macro */
#define SLICE_SCAN(slice, needleHash, needleStr)                                                                       \
({                                                                                                                     \
    CARBON_TRACE(CARBON_SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needleStr);                                     \
    assert(slice);                                                                                                     \
    assert(needleStr);                                                                                                 \
                                                                                                                       \
    register bool continueScan, keysMatch, keyHashsNoMatch, endReached;                                                \
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);                                                 \
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);                   \
    register bool cacheHit = hashsEq && (strcmp(slice->key_column[slice->cacheIdx], needleStr) == 0);                   \
    register uint_fast32_t i = 0;                                                                                      \
    if (!cacheHit) {                                                                                                   \
        do {                                                                                                           \
            while ((keyHashsNoMatch = (slice->keyHashColumn[i]!=needleHash)) && i++<slice->num_elems) { ; }             \
            endReached    = ((i+1)>slice->num_elems);                                                                   \
            keysMatch      = endReached || (!keyHashsNoMatch && (strcmp(slice->key_column[i], needleStr)==0));          \
            continueScan  = !endReached && !keysMatch;                                                                 \
            i             += continueScan;                                                                             \
        }                                                                                                              \
        while (continueScan);                                                                                          \
        slice->cacheIdx = !endReached && keysMatch ? i : slice->cacheIdx;                                              \
    }                                                                                                                  \
    cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? i : slice->num_elems);                                     \
})

#define SLICE_BESEARCH(slice, needleHash, needleStr)                                                                   \
({                                                                                                                     \
    0; \
})

static void appenderNew(carbon_slice_list_t *list);
static void appenderSeal(Slice *slice);

static void lock(carbon_slice_list_t *list);
static void unlock(carbon_slice_list_t *list);

CARBON_EXPORT(bool)
carbon_slice_list_create(carbon_slice_list_t *list, const carbon_alloc_t *alloc, size_t sliceCapacity)
{
    CARBON_NON_NULL_OR_ERROR(list)
    CARBON_NON_NULL_OR_ERROR(sliceCapacity)

    carbon_alloc_this_or_std(&list->alloc, alloc);
    carbon_spinlock_init(&list->lock);
    carbon_error_init(&list->err);

    carbon_vec_create(&list->slices, &list->alloc, sizeof(Slice), sliceCapacity);
    carbon_vec_create(&list->descriptors, &list->alloc, sizeof(SliceDescriptor), sliceCapacity);
    carbon_vec_create(&list->filters, &list->alloc, sizeof(carbon_bloom_t), sliceCapacity);
    carbon_vec_create(&list->bounds, &list->alloc, sizeof(HashBounds), sliceCapacity);

    CARBON_ZERO_MEMORY(carbon_vec_data(&list->slices), sliceCapacity * sizeof(Slice));
    CARBON_ZERO_MEMORY(carbon_vec_data(&list->descriptors), sliceCapacity * sizeof(SliceDescriptor));
    CARBON_ZERO_MEMORY(carbon_vec_data(&list->filters), sliceCapacity * sizeof(carbon_bloom_t));
    CARBON_ZERO_MEMORY(carbon_vec_data(&list->bounds), sliceCapacity * sizeof(HashBounds));

    appenderNew(list);

    return true;
}

CARBON_EXPORT(bool)
SliceListDrop(carbon_slice_list_t *list)
{
    CARBON_UNUSED(list);
//    NOT_YET_IMPLEMENTED
    // TODO: implement
    carbon_vec_drop(&list->slices);
    carbon_vec_drop(&list->descriptors);
    carbon_vec_drop(&list->bounds);
    for (size_t i = 0; i < list->filters.num_elems; i++) {
        carbon_bloom_t *filter = CARBON_VECTOR_GET(&list->filters, i, carbon_bloom_t);
        carbon_bloom_drop(filter);
    }
    carbon_vec_drop(&list->filters);
    return true;
}

CARBON_EXPORT(bool)
SliceListIsEmpty(const carbon_slice_list_t *list)
{
    return (carbon_vec_is_empty(&list->slices));
}

CARBON_EXPORT(bool)
carbon_slice_list_insert(carbon_slice_list_t *list, char **strings, carbon_string_id_t *ids, size_t num_pairs)
{
    lock(list);

    while (num_pairs--) {
        const char *key = *strings++;
        carbon_string_id_t value = *ids++;
        carbon_hash_t keyHash = get_hashcode(key);
        slice_handle_t handle;
        int status;

        assert (key);

        /** check whether the keys-values pair is already contained in one slice */
        status = carbon_slice_list_lookup(&handle, list, key);

        if (status == true) {
            /** pair was found, do not insert it twice */
            assert (value == handle.value);
            continue;
        }
        else {
            /** pair is not found; append it */
            HashBounds *restrict bounds = CARBON_VECTOR_ALL(&list->bounds, HashBounds);
            carbon_bloom_t *restrict filters = CARBON_VECTOR_ALL(&list->filters, carbon_bloom_t);
            Slice *restrict slices = CARBON_VECTOR_ALL(&list->slices, Slice);

            if (list->appender_idx != 0) { ; // TODO: remove
            }

            Slice *restrict appender = slices + list->appender_idx;
            carbon_bloom_t *restrict appenderFilter = filters + list->appender_idx;
            HashBounds *restrict appenderBounds = bounds + list->appender_idx;

            CARBON_DEBUG(CARBON_SLICE_LIST_TAG,
                  "appender # of elems: %zu, limit: %zu",
                  appender->num_elems,
                  SLICE_KEY_COLUMN_MAX_ELEMS);
            assert(appender->num_elems < SLICE_KEY_COLUMN_MAX_ELEMS);
            appender->key_column[appender->num_elems] = key;
            appender->keyHashColumn[appender->num_elems] = keyHash;
            appender->carbon_string_id_tColumn[appender->num_elems] = value;
            appenderBounds->minHash = appenderBounds->minHash < keyHash ?
                                       appenderBounds->minHash : keyHash;
            appenderBounds->maxHash = appenderBounds->maxHash > keyHash ?
                                       appenderBounds->maxHash : keyHash;
            CARBON_BLOOM_SET(appenderFilter, &keyHash, sizeof(carbon_hash_t));
            appender->num_elems++;
            if (CARBON_BRANCH_UNLIKELY(appender->num_elems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                appenderSeal(appender);
                appenderNew(list);
            }
        }
    }

    unlock(list);
    return true;
}

CARBON_EXPORT(bool)
carbon_slice_list_lookup(slice_handle_t *handle, carbon_slice_list_t *list, const char *needle)
{
    CARBON_UNUSED(list);
    CARBON_UNUSED(handle);
    CARBON_UNUSED(needle);

    carbon_hash_t keyHash = get_hashcode(needle);
    uint32_t numSlices = carbon_vec_length(&list->slices);

    /** check whether the keys-values pair is already contained in one slice */
    HashBounds *restrict bounds = CARBON_VECTOR_ALL(&list->bounds, HashBounds);
    carbon_bloom_t *restrict filters = CARBON_VECTOR_ALL(&list->filters, carbon_bloom_t);
    Slice *restrict slices = CARBON_VECTOR_ALL(&list->slices, Slice);
    SliceDescriptor *restrict descs = CARBON_VECTOR_ALL(&list->descriptors, SliceDescriptor);

    for (register uint32_t i = 0; i < numSlices; i++) {
        SliceDescriptor *restrict desc = descs + i;
        HashBounds *restrict bound = bounds + i;
        Slice *restrict slice = slices + i;

        desc->numReadsAll++;

        if (slice->num_elems > 0) {
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
                    if (pairPosition < slice->num_elems) {
                        /** pair is contained */
                        desc->numReadsHit++;
                        handle->is_contained = true;
                        handle->value = slice->carbon_string_id_tColumn[pairPosition];
                        handle->key = needle;
                        handle->container = slice;

                        desc->numReadsHit++;
                        return true;
                    }
                }
                else {
                    /** carbon_bloom_t is sure that pair is not contained */
                    continue;
                }
            }
            else {
                /** keys hash is not inside bounds of hashes in slice */
                continue;
            }
        }
    }

    handle->is_contained = false;

    return false;
}

CARBON_EXPORT(bool)
SliceListRemove(carbon_slice_list_t *list, slice_handle_t *handle)
{
    CARBON_UNUSED(list);
    CARBON_UNUSED(handle);
    CARBON_NOT_IMPLEMENTED
}

static void appenderNew(carbon_slice_list_t *list)
{
    /** ANTI-OPTIMIZATION: madvising sequential access to columns in slice decrease performance */

    /** the slice itself */
    Slice slice = {
        .strat     = SLICE_LOOKUP_SCAN,
        .num_elems = 0,
        .cacheIdx = (uint32_t) -1
    };

    uint32_t numSlices = carbon_vec_length(&list->slices);
    carbon_vec_push(&list->slices, &slice, 1);

    assert(SLICE_KEY_COLUMN_MAX_ELEMS > 0);

    /** the descriptor */
    SliceDescriptor desc = {
        .numReadsHit  = 0,
        .numReadsAll  = 0,
    };

    carbon_vec_push(&list->descriptors, &desc, 1);

    /** the lookup guards */
    assert(sizeof(carbon_bloom_t) <= CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
    carbon_bloom_t filter;

    /** NOTE: the size of each carbon_bloom_t lead to a false positive probability of 100%, i.e., number of items in the
     * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
     * in bits minus the header for the carbon_bloom_t) along with the number of used hash functions (4), lead to that
     * probability. However, the reason a carbon_bloom_t is used is to skip slices whch definitively do NOT contain the
     * keys-values pair - and that still works ;) */
    carbon_bloom_create(&filter,
                        (CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(carbon_bloom_t)) * 8);
    carbon_vec_push(&list->filters, &filter, 1);
    HashBounds bounds = {
        .minHash        = (carbon_hash_t) -1,
        .maxHash        = (carbon_hash_t) 0
    };
    carbon_vec_push(&list->bounds, &bounds, 1);

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
         list->slices.num_elems,
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
         (sizeof(carbon_slice_list_t) + list->slices.num_elems
             * (sizeof(Slice) + sizeof(SliceDescriptor) + (sizeof(uint32_t) * list->descriptors.num_elems)
                 + sizeof(carbon_bloom_t) + carbon_bitmap_nbits(&filter) / 8 + sizeof(HashBounds))) / 1024.0
             / 1024.0
    );

    /** register new slice as the current appender */
    list->appender_idx = numSlices;
}

static void appenderSeal(Slice *slice)
{
    CARBON_UNUSED(slice);
    //  slice->cacheIdx = 0;
    //  slice_sort(slice);
    //  slice->strat = SLICE_LOOKUP_BESEARCH;

    // TODO: sealing means sort and then replace 'find' with bsearch or something. Not yet implemented: sealed slices are also search in a linear fashion
}

static void lock(carbon_slice_list_t *list)
{
    carbon_spinlock_acquire(&list->lock);
}

static void unlock(carbon_slice_list_t *list)
{
    carbon_spinlock_release(&list->lock);
}