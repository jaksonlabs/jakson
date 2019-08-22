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

#include <math.h>

#include <jak_slicelist.h>
#include <jak_hash.h>

#define JAK_SLICE_LIST_TAG "slice-list"

#define get_hashcode(key)    JAK_HASH_ADDITIVE(strlen(key), key)

/** OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/** OPTIMIZATION: find function as macro */
#define SLICE_SCAN(slice, needle_hash, needle_str)                                                                     \
({                                                                                                                     \
    JAK_trace(JAK_SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needle_str);                                    \
    JAK_ASSERT(slice);                                                                                                     \
    JAK_ASSERT(needle_str);                                                                                                \
                                                                                                                       \
    register bool continueScan, keysMatch, keyHashsNoMatch, endReached;                                                \
    register bool cacheAvailable = (slice->cacheIdx != (jak_u32) -1);                                                 \
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needle_hash);                  \
    register bool cacheHit = hashsEq && (strcmp(slice->key_column[slice->cacheIdx], needle_str) == 0);                 \
    register uint_fast32_t i = 0;                                                                                      \
    if (!cacheHit) {                                                                                                   \
        do {                                                                                                           \
            while ((keyHashsNoMatch = (slice->keyHashColumn[i]!=needle_hash)) && i++<slice->num_elems) { ; }           \
            endReached    = ((i+1)>slice->num_elems);                                                                  \
            keysMatch      = endReached || (!keyHashsNoMatch && (strcmp(slice->key_column[i], needle_str)==0));        \
            continueScan  = !endReached && !keysMatch;                                                                 \
            i             += continueScan;                                                                             \
        }                                                                                                              \
        while (continueScan);                                                                                          \
        slice->cacheIdx = !endReached && keysMatch ? i : slice->cacheIdx;                                              \
    }                                                                                                                  \
    cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? i : slice->num_elems);                                    \
})

#define SLICE_BESEARCH(slice, needle_hash, needle_str)                                                                 \
({                                                                                                                     \
    0; \
})

static void appenderNew(slice_list_t *list);

static void appenderSeal(Slice *slice);

static void lock(slice_list_t *list);

static void unlock(slice_list_t *list);

bool slice_list_create(slice_list_t *list, const jak_allocator *alloc, size_t sliceCapacity)
{
        JAK_ERROR_IF_NULL(list)
        JAK_ERROR_IF_NULL(sliceCapacity)

        jak_alloc_this_or_std(&list->alloc, alloc);
        spin_init(&list->lock);
        jak_error_init(&list->err);

        vec_create(&list->slices, &list->alloc, sizeof(Slice), sliceCapacity);
        vec_create(&list->descriptors, &list->alloc, sizeof(SliceDescriptor), sliceCapacity);
        vec_create(&list->filters, &list->alloc, sizeof(jak_bitmap), sliceCapacity);
        vec_create(&list->bounds, &list->alloc, sizeof(HashBounds), sliceCapacity);

        JAK_zero_memory(vec_data(&list->slices), sliceCapacity * sizeof(Slice));
        JAK_zero_memory(vec_data(&list->descriptors), sliceCapacity * sizeof(SliceDescriptor));
        JAK_zero_memory(vec_data(&list->filters), sliceCapacity * sizeof(jak_bitmap));
        JAK_zero_memory(vec_data(&list->bounds), sliceCapacity * sizeof(HashBounds));

        appenderNew(list);

        return true;
}

bool SliceListDrop(slice_list_t *list)
{
        JAK_UNUSED(list);

        vec_drop(&list->slices);
        vec_drop(&list->descriptors);
        vec_drop(&list->bounds);
        for (size_t i = 0; i < list->filters.num_elems; i++) {
                jak_bitmap *filter = vec_get(&list->filters, i, jak_bitmap);
                jak_bloom_drop(filter);
        }
        vec_drop(&list->filters);
        return true;
}

bool SliceListIsEmpty(const slice_list_t *list)
{
        return (vec_is_empty(&list->slices));
}

bool slice_list_insert(slice_list_t *list, char **strings, jak_archive_field_sid_t *ids, size_t num_pairs)
{
        lock(list);

        while (num_pairs--) {
                const char *key = *strings++;
                jak_archive_field_sid_t value = *ids++;
                hash32_t keyHash = get_hashcode(key);
                slice_handle_t handle;
                int status;

                JAK_ASSERT (key);

                /** check whether the keys-values pair is already contained in one slice */
                status = slice_list_lookup(&handle, list, key);

                if (status == true) {
                        /** pair was found, do not insert it twice */
                        JAK_ASSERT (value == handle.value);
                        continue;
                } else {
                        /** pair is not found; append it */
                        HashBounds *restrict bounds = vec_all(&list->bounds, HashBounds);
                        jak_bitmap *restrict filters = vec_all(&list->filters, jak_bitmap);
                        Slice *restrict slices = vec_all(&list->slices, Slice);

                        Slice *restrict appender = slices + list->appender_idx;
                        jak_bitmap *restrict appenderFilter = filters + list->appender_idx;
                        HashBounds *restrict appenderBounds = bounds + list->appender_idx;

                        JAK_debug(JAK_SLICE_LIST_TAG,
                                  "appender # of elems: %zu, limit: %zu",
                                  appender->num_elems,
                                  SLICE_KEY_COLUMN_MAX_ELEMS);
                        JAK_ASSERT(appender->num_elems < SLICE_KEY_COLUMN_MAX_ELEMS);
                        appender->key_column[appender->num_elems] = key;
                        appender->keyHashColumn[appender->num_elems] = keyHash;
                        appender->string_id_tColumn[appender->num_elems] = value;
                        appenderBounds->minHash = appenderBounds->minHash < keyHash ? appenderBounds->minHash : keyHash;
                        appenderBounds->maxHash = appenderBounds->maxHash > keyHash ? appenderBounds->maxHash : keyHash;
                        JAK_BLOOM_SET(appenderFilter, &keyHash, sizeof(hash32_t));
                        appender->num_elems++;
                        if (JAK_UNLIKELY(appender->num_elems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                                appenderSeal(appender);
                                appenderNew(list);
                        }
                }
        }

        unlock(list);
        return true;
}

bool slice_list_lookup(slice_handle_t *handle, slice_list_t *list, const char *needle)
{
        JAK_UNUSED(list);
        JAK_UNUSED(handle);
        JAK_UNUSED(needle);

        hash32_t keyHash = get_hashcode(needle);
        jak_u32 numSlices = vec_length(&list->slices);

        /** check whether the keys-values pair is already contained in one slice */
        HashBounds *restrict bounds = vec_all(&list->bounds, HashBounds);
        jak_bitmap *restrict filters = vec_all(&list->filters, jak_bitmap);
        Slice *restrict slices = vec_all(&list->slices, Slice);
        SliceDescriptor *restrict descs = vec_all(&list->descriptors, SliceDescriptor);

        for (register jak_u32 i = 0; i < numSlices; i++) {
                SliceDescriptor *restrict desc = descs + i;
                HashBounds *restrict bound = bounds + i;
                Slice *restrict slice = slices + i;

                desc->numReadsAll++;

                if (slice->num_elems > 0) {
                        bool keyHashIn = keyHash >= bound->minHash && keyHash <= bound->maxHash;
                        if (keyHashIn) {
                                jak_bitmap *restrict filter = filters + i;
                                bool maybeContained = JAK_BLOOM_TEST(filter, &keyHash, sizeof(hash32_t));
                                if (maybeContained) {
                                        JAK_debug(JAK_SLICE_LIST_TAG,
                                                  "JAK_slice_list_lookup_by_key keys(%s) -> ?",
                                                  needle);
                                        jak_u32 pairPosition;

                                        switch (slice->strat) {
                                                case SLICE_LOOKUP_SCAN:
                                                        pairPosition = SLICE_SCAN(slice, keyHash, needle);
                                                        break;
                                                case SLICE_LOOKUP_BESEARCH:
                                                        pairPosition = SLICE_BESEARCH(slice, keyHash, needle);
                                                        break;
                                                default: JAK_ERROR(&list->err, JAK_ERR_UNSUPFINDSTRAT)
                                                        return false;
                                        }

                                        JAK_debug(JAK_SLICE_LIST_TAG,
                                                  "JAK_slice_list_lookup_by_key keys(%s) -> pos(%zu in slice #%zu)",
                                                  needle,
                                                  pairPosition,
                                                  i);
                                        if (pairPosition < slice->num_elems) {
                                                /** pair is contained */
                                                desc->numReadsHit++;
                                                handle->is_contained = true;
                                                handle->value = slice->string_id_tColumn[pairPosition];
                                                handle->key = needle;
                                                handle->container = slice;

                                                desc->numReadsHit++;
                                                return true;
                                        }
                                } else {
                                        /** jak_bitmap is sure that pair is not contained */
                                        continue;
                                }
                        } else {
                                /** keys hash is not inside bounds of hashes in slice */
                                continue;
                        }
                }
        }

        handle->is_contained = false;

        return false;
}

bool SliceListRemove(slice_list_t *list, slice_handle_t *handle)
{
        JAK_UNUSED(list);
        JAK_UNUSED(handle);
        JAK_NOT_IMPLEMENTED
}

static void appenderNew(slice_list_t *list)
{
        /** ANTI-OPTIMIZATION: madvising sequential access to columns in slice decrease performance */

        /** the slice itself */
        Slice slice = {.strat     = SLICE_LOOKUP_SCAN, .num_elems = 0, .cacheIdx = (jak_u32) -1};

        jak_u32 numSlices = vec_length(&list->slices);
        vec_push(&list->slices, &slice, 1);

        JAK_ASSERT(SLICE_KEY_COLUMN_MAX_ELEMS > 0);

        /** the descriptor */
        SliceDescriptor desc = {.numReadsHit  = 0, .numReadsAll  = 0,};

        vec_push(&list->descriptors, &desc, 1);

        /** the lookup guards */
        JAK_ASSERT(sizeof(jak_bitmap) <= JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
        jak_bitmap filter;

        /** NOTE: the size of each jak_bitmap lead to a false positive probability of 100%, i.e., number of items in the
         * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
         * in bits minus the header for the jak_bitmap) along with the number of used hash functions (4), lead to that
         * probability. However, the reason a jak_bitmap is used is to skip slices whch definitively do NOT contain the
         * keys-values pair - and that still works ;) */
        jak_bloom_create(&filter,
                         (JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(jak_bitmap)) * 8);
        vec_push(&list->filters, &filter, 1);
        HashBounds bounds = {.minHash        = (hash32_t) -1, .maxHash        = (hash32_t) 0};
        vec_push(&list->bounds, &bounds, 1);

        JAK_info(JAK_SLICE_LIST_TAG,
                 "created new appender in slice list %p\n\t"
                 "# of slices (incl. appender) in total...............: %zu\n\t"
                 "Slice target memory size............................: %zuB (%s)\n\t"
                 "jak_bitmap target memory size......................: %zuB (%s)\n\t"
                 "Max # of (keys, hash, string) in appender/slice......: %zu\n\t"
                 "Bits used in per-slice jak_bitmap..................: %zu\n\t"
                 "Prob. of jak_bitmap to produce false-positives.....: %f\n\t"
                 "Single slice type size..............................: %zuB\n\t"
                 "Total slice-list size...............................: %f MiB",
                 list,
                 list->slices.num_elems,
                 (size_t) JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
                 JAK_SLICE_LIST_TARGET_MEMORY_NAME,
                 (size_t) JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE,
                 JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
                 (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
                 jak_bitmap_nbits(&filter),
                 (pow(1 - exp(-(double) jak_bloom_nhashs()
                              / ((double) jak_bitmap_nbits(&filter) / (double) SLICE_KEY_COLUMN_MAX_ELEMS)),
                      jak_bitmap_nbits(&filter))),
                 sizeof(Slice),
                 (sizeof(slice_list_t) + list->slices.num_elems
                                         * (sizeof(Slice) + sizeof(SliceDescriptor) +
                                            (sizeof(jak_u32) * list->descriptors.num_elems)
                                            + sizeof(jak_bitmap) + jak_bitmap_nbits(&filter) / 8 +
                                            sizeof(HashBounds))) /
                 1024.0 / 1024.0);

        /** register new slice as the current appender */
        list->appender_idx = numSlices;
}

static void appenderSeal(Slice *slice)
{
        JAK_UNUSED(slice);
        // TODO: sealing means sort and then replace 'find' with bsearch or something.
        // Not yet implemented: sealed slices are also search in a linear fashion
}

static void lock(slice_list_t *list)
{
        spin_acquire(&list->lock);
}

static void unlock(slice_list_t *list)
{
        spin_release(&list->lock);
}