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

#include <jakson/stdx/slicelist.h>
#include <jakson/std/hash.h>

#define SLICE_LIST_TAG "slice-list"

#define get_hashcode(key)    HASH_ADDITIVE(strlen(key), key)

/** OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/** OPTIMIZATION: find function as macro */
#define SLICE_SCAN(slice, needle_hash, needle_str)                                                                     \
({                                                                                                                     \
    TRACE(SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needle_str);                                    \
    JAK_ASSERT(slice);                                                                                                     \
    JAK_ASSERT(needle_str);                                                                                                \
                                                                                                                       \
    register bool continueScan, keysMatch, keyHashsNoMatch, endReached;                                                \
    register bool cacheAvailable = (slice->cache_idx != (u32) -1);                                                 \
    register bool hashsEq = cacheAvailable && (slice->key_hash_column[slice->cache_idx] == needle_hash);                  \
    register bool cacheHit = hashsEq && (strcmp(slice->key_column[slice->cache_idx], needle_str) == 0);                 \
    register uint_fast32_t i = 0;                                                                                      \
    if (!cacheHit) {                                                                                                   \
        do {                                                                                                           \
            while ((keyHashsNoMatch = (slice->key_hash_column[i]!=needle_hash)) && i++<slice->num_elems) { ; }           \
            endReached    = ((i+1)>slice->num_elems);                                                                  \
            keysMatch      = endReached || (!keyHashsNoMatch && (strcmp(slice->key_column[i], needle_str)==0));        \
            continueScan  = !endReached && !keysMatch;                                                                 \
            i             += continueScan;                                                                             \
        }                                                                                                              \
        while (continueScan);                                                                                          \
        slice->cache_idx = !endReached && keysMatch ? i : slice->cache_idx;                                              \
    }                                                                                                                  \
    cacheHit ? slice->cache_idx : (!endReached && keysMatch ? i : slice->num_elems);                                    \
})

#define SLICE_BESEARCH(slice, needle_hash, needle_str)                                                                 \
({                                                                                                                     \
    0; \
})

static void appenderNew(slice_list_t *list);

static void appenderSeal(slice *slice);

static void _slicelist_lock(slice_list_t *list);

static void _slicelist_unlock(slice_list_t *list);

bool slice_list_create(slice_list_t *list, const allocator *alloc, size_t slice_capacity)
{
        ERROR_IF_NULL(list)
        ERROR_IF_NULL(slice_capacity)

        alloc_this_or_std(&list->alloc, alloc);
        spinlock_init(&list->lock);
        error_init(&list->err);

        vector_create(&list->slices, &list->alloc, sizeof(slice), slice_capacity);
        vector_create(&list->descriptors, &list->alloc, sizeof(slice_descriptor), slice_capacity);
        vector_create(&list->filters, &list->alloc, sizeof(bitmap), slice_capacity);
        vector_create(&list->bounds, &list->alloc, sizeof(hash_bounds), slice_capacity);

        ZERO_MEMORY(vector_data(&list->slices), slice_capacity * sizeof(slice));
        ZERO_MEMORY(vector_data(&list->descriptors), slice_capacity * sizeof(slice_descriptor));
        ZERO_MEMORY(vector_data(&list->filters), slice_capacity * sizeof(bitmap));
        ZERO_MEMORY(vector_data(&list->bounds), slice_capacity * sizeof(hash_bounds));

        appenderNew(list);

        return true;
}

bool slice_list_drop(slice_list_t *list)
{
        UNUSED(list);

        vector_drop(&list->slices);
        vector_drop(&list->descriptors);
        vector_drop(&list->bounds);
        for (size_t i = 0; i < list->filters.num_elems; i++) {
                bitmap *filter = VECTOR_GET(&list->filters, i, bitmap);
                bloom_drop(filter);
        }
        vector_drop(&list->filters);
        return true;
}

bool slice_list_is_empty(const slice_list_t *list)
{
        return (vector_is_empty(&list->slices));
}

bool slice_list_insert(slice_list_t *list, char **strings, archive_field_sid_t *ids, size_t num_pairs)
{
        _slicelist_lock(list);

        while (num_pairs--) {
                const char *key = *strings++;
                archive_field_sid_t value = *ids++;
                hash32_t keyHash = get_hashcode(key);
                slice_handle handle;
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
                        hash_bounds *restrict bounds = VECTOR_ALL(&list->bounds, hash_bounds);
                        bitmap *restrict filters = VECTOR_ALL(&list->filters, bitmap);
                        slice *restrict slices = VECTOR_ALL(&list->slices, slice);

                        slice *restrict appender = slices + list->appender_idx;
                        bitmap *restrict appenderFilter = filters + list->appender_idx;
                        hash_bounds *restrict appenderBounds = bounds + list->appender_idx;

                        DEBUG(SLICE_LIST_TAG,
                                  "appender # of elems: %zu, limit: %zu",
                                  appender->num_elems,
                                  SLICE_KEY_COLUMN_MAX_ELEMS);
                        JAK_ASSERT(appender->num_elems < SLICE_KEY_COLUMN_MAX_ELEMS);
                        appender->key_column[appender->num_elems] = key;
                        appender->key_hash_column[appender->num_elems] = keyHash;
                        appender->string_id_column[appender->num_elems] = value;
                        appenderBounds->min_hash = appenderBounds->min_hash < keyHash ? appenderBounds->min_hash : keyHash;
                        appenderBounds->max_hash = appenderBounds->max_hash > keyHash ? appenderBounds->max_hash : keyHash;
                        BLOOM_SET(appenderFilter, &keyHash, sizeof(hash32_t));
                        appender->num_elems++;
                        if (UNLIKELY(appender->num_elems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                                appenderSeal(appender);
                                appenderNew(list);
                        }
                }
        }

        _slicelist_unlock(list);
        return true;
}

bool slice_list_lookup(slice_handle *handle, slice_list_t *list, const char *needle)
{
        UNUSED(list);
        UNUSED(handle);
        UNUSED(needle);

        hash32_t keyHash = get_hashcode(needle);
        u32 numSlices = vector_length(&list->slices);

        /** check whether the keys-values pair is already contained in one slice */
        hash_bounds *restrict bounds = VECTOR_ALL(&list->bounds, hash_bounds);
        bitmap *restrict filters = VECTOR_ALL(&list->filters, bitmap);
        slice *restrict slices = VECTOR_ALL(&list->slices, slice);
        slice_descriptor *restrict descs = VECTOR_ALL(&list->descriptors, slice_descriptor);

        for (register u32 i = 0; i < numSlices; i++) {
                slice_descriptor *restrict desc = descs + i;
                hash_bounds *restrict bound = bounds + i;
                slice *restrict slice = slices + i;

                desc->num_reads_all++;

                if (slice->num_elems > 0) {
                        bool keyHashIn = keyHash >= bound->min_hash && keyHash <= bound->max_hash;
                        if (keyHashIn) {
                                bitmap *restrict filter = filters + i;
                                bool maybeContained = BLOOM_TEST(filter, &keyHash, sizeof(hash32_t));
                                if (maybeContained) {
                                        DEBUG(SLICE_LIST_TAG,
                                                  "slice_list_lookup_by_key keys(%s) -> ?",
                                                  needle);
                                        u32 pairPosition;

                                        switch (slice->strat) {
                                                case SLICE_LOOKUP_SCAN:
                                                        pairPosition = SLICE_SCAN(slice, keyHash, needle);
                                                        break;
                                                case SLICE_LOOKUP_BESEARCH:
                                                        pairPosition = SLICE_BESEARCH(slice, keyHash, needle);
                                                        break;
                                                default: ERROR(&list->err, ERR_UNSUPFINDSTRAT)
                                                        return false;
                                        }

                                        DEBUG(SLICE_LIST_TAG,
                                                  "slice_list_lookup_by_key keys(%s) -> pos(%zu in slice #%zu)",
                                                  needle,
                                                  pairPosition,
                                                  i);
                                        if (pairPosition < slice->num_elems) {
                                                /** pair is contained */
                                                desc->num_reads_hit++;
                                                handle->is_contained = true;
                                                handle->value = slice->string_id_column[pairPosition];
                                                handle->key = needle;
                                                handle->container = slice;

                                                desc->num_reads_hit++;
                                                return true;
                                        }
                                } else {
                                        /** bitmap is sure that pair is not contained */
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

bool slice_list_remove(slice_list_t *list, slice_handle *handle)
{
        UNUSED(list);
        UNUSED(handle);
        NOT_IMPLEMENTED
}

static void appenderNew(slice_list_t *list)
{
        /** ANTI-OPTIMIZATION: madvising sequential access to columns in slice decrease performance */

        /** the slice itself */
        slice slice = {.strat     = SLICE_LOOKUP_SCAN, .num_elems = 0, .cache_idx = (u32) -1};

        u32 numSlices = vector_length(&list->slices);
        vector_push(&list->slices, &slice, 1);

        JAK_ASSERT(SLICE_KEY_COLUMN_MAX_ELEMS > 0);

        /** the descriptor */
        slice_descriptor desc = {.num_reads_hit  = 0, .num_reads_all  = 0,};

        vector_push(&list->descriptors, &desc, 1);

        /** the lookup guards */
        JAK_ASSERT(sizeof(bitmap) <= SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
        bitmap filter;

        /** NOTE: the size of each bitmap lead to a false positive probability of 100%, i.e., number of items in the
         * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
         * in bits minus the header for the bitmap) along with the number of used hash functions (4), lead to that
         * probability. However, the reason a bitmap is used is to skip slices whch definitively do NOT contain the
         * keys-values pair - and that still works ;) */
        bloom_create(&filter,
                         (SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(bitmap)) * 8);
        vector_push(&list->filters, &filter, 1);
        hash_bounds bounds = {.min_hash        = (hash32_t) -1, .max_hash        = (hash32_t) 0};
        vector_push(&list->bounds, &bounds, 1);

        INFO(SLICE_LIST_TAG,
                 "created new appender in slice list %p\n\t"
                 "# of slices (incl. appender) in total...............: %zu\n\t"
                 "slice target memory size............................: %zuB (%s)\n\t"
                 "bitmap target memory size......................: %zuB (%s)\n\t"
                 "Max # of (keys, hash, string_buffer) in appender/slice......: %zu\n\t"
                 "Bits used in per-slice bitmap..................: %zu\n\t"
                 "Prob. of bitmap to produce false-positives.....: %f\n\t"
                 "Single slice type size..............................: %zuB\n\t"
                 "Total slice-list size...............................: %f MiB",
                 list,
                 list->slices.num_elems,
                 (size_t) SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
                 SLICE_LIST_TARGET_MEMORY_NAME,
                 (size_t) SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE,
                 SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
                 (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
                 bitmap_nbits(&filter),
                 (pow(1 - exp(-(double) bloom_nhashs()
                              / ((double) bitmap_nbits(&filter) / (double) SLICE_KEY_COLUMN_MAX_ELEMS)),
                      bitmap_nbits(&filter))),
                 sizeof(slice),
                 (sizeof(slice_list_t) + list->slices.num_elems
                                         * (sizeof(slice) + sizeof(slice_descriptor) +
                                            (sizeof(u32) * list->descriptors.num_elems)
                                            + sizeof(bitmap) + bitmap_nbits(&filter) / 8 +
                                            sizeof(hash_bounds))) /
                 1024.0 / 1024.0);

        /** register new slice as the current appender */
        list->appender_idx = numSlices;
}

static void appenderSeal(slice *slice)
{
        UNUSED(slice);
        // TODO: sealing means sort and then replace 'find' with bsearch or something.
        // Not yet implemented: sealed slices are also search in a linear fashion
}

static void _slicelist_lock(slice_list_t *list)
{
        spinlock_acquire(&list->lock);
}

static void _slicelist_unlock(slice_list_t *list)
{
        spinlock_release(&list->lock);
}