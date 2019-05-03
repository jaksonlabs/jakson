// file: slicelist.c

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

#include "slicelist.h"
// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N S T A N T S
//
// ---------------------------------------------------------------------------------------------------------------------

#define NG5_SLICE_LIST_TAG "slice-list"

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#define get_hashcode(key)    HashAdditive(strlen(key), key)



#define SLICE_SCAN_SIMD_INLINE_MACRO(slice, needleHash, needleStr) ({ \
    \
    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN_SIMD for '%s' started", needleStr);\
    assert(slice);\
    assert(needleStr);\
\
    register bool continueScan = false, keysMatch = false, keyHashsNoMatch, endReached = false;\
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);\
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);\
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);\
    register int matchIndex = -1;\
    register uint32_t index = 0;\
    __m256i simdSearchValue = _mm256_set1_epi64x(needleHash);\
    do {\
        if (!cacheHit) {\
\
            do {\
                matchIndex = -1;\
\
\
 \
                int restElements = NG5_SIMD_COMPARE_ELEMENT_COUNT - (slice->numElems- index);\
                endReached = (restElements > 0);\
\
                __m256i simdSearchData = _mm256_loadu_si256((__m256i *)(slice->keyHashColumn + index));\
\
                __m256i compareResult = _mm256_cmpeq_epi64(simdSearchData, simdSearchValue);\
                \
                if (!_mm256_testz_si256(compareResult, compareResult)) {\
                    unsigned bitmask = _mm256_movemask_epi8(compareResult);\
                    matchIndex = index + _bit_scan_forward(bitmask) / 8; \
                }\
                index += NG5_SIMD_COMPARE_ELEMENT_COUNT;\
\
            } while(!(endReached || matchIndex > -1));\
\
            keyHashsNoMatch = matchIndex == -1;\
            \
            keysMatch      = endReached || (!keyHashsNoMatch && (strcmp(slice->keyColumn[matchIndex], needleStr)==0));\
            \
            continueScan  = !endReached && !keysMatch;\
\
\
            slice->cacheIdx = !endReached && keysMatch ? matchIndex : slice->cacheIdx;\
        }\
\
    }\
    while (continueScan);\
\
    cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? matchIndex : slice->numElems);\
})

/* OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/* OPTIMIZATION: find function as macro */
#define SLICE_SCAN(slice, needleHash, needleStr)                                                                       \
({                                                                                                                     \
    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needleStr);                                               \
    assert(slice);                                                                                                     \
    assert(needleStr);                                                                                                 \
    \
    if (slice->numElems > 4) {\
        TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN for '%s' started", needleStr);\
\
    }\
\
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

uint32_t SLICE_SCAN_SIMD_INLINE(Slice *slice, Hash needleHash, const char *needleStr) {
    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN_SIMD for '%s' started", needleStr);
    assert(slice);
    assert(needleStr);

    register bool continueScan = false, keysMatch = false, keyHashsNoMatch, endReached = false;
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);
    register int matchIndex = -1;
    register uint32_t index = 0;
    __m256i simdSearchValue = _mm256_set1_epi64x(needleHash);

    do {
        if (!cacheHit) {

            do {
                matchIndex = -1;


                int restElements = NG5_SIMD_COMPARE_ELEMENT_COUNT - (slice->numElems - index);
                endReached = (restElements > 0 && index > NG5_SIMD_COMPARE_ELEMENT_COUNT);

                __m256i simdSearchData = _mm256_loadu_si256((__m256i *) (slice->keyHashColumn + index));

                __m256i compareResult = _mm256_cmpeq_epi64(simdSearchData, simdSearchValue);

                if (!_mm256_testz_si256(compareResult, compareResult)) {
                    unsigned bitmask = _mm256_movemask_epi8(compareResult);
                    matchIndex = index + _bit_scan_forward(bitmask) / 8;
                }
                index += NG5_SIMD_COMPARE_ELEMENT_COUNT;

            } while (!(endReached || matchIndex > -1));

            keyHashsNoMatch = matchIndex == -1;

            keysMatch = endReached || (!keyHashsNoMatch && (strcmp(slice->keyColumn[matchIndex], needleStr) == 0));

            continueScan = !endReached && !keysMatch;


            slice->cacheIdx = !endReached && keysMatch ? matchIndex : slice->cacheIdx;
        }

    } while (continueScan);


    return cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? matchIndex : slice->numElems);

}

/* OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/* OPTIMIZATION: find function as macro */
uint32_t SLICE_SCAN_SIMD(Slice *slice, Hash needleHash, const char *needleStr) {

    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN_SIMD for '%s' started", needleStr);
    assert(slice);
    assert(needleStr);

    register bool continueScan, keysMatch, keyHashsNoMatch, endReached;
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);

    // Create SIMD Scan
    SIMDScanOperation simdScanOperation;
    simdScanOperation.data = slice->keyHashColumn;
    simdScanOperation.searchValue = needleHash;
    simdScanOperation.elementCount = slice->numElems;
    SIMDScanPrepare(&simdScanOperation);

    do {
        if (!cacheHit) {
            \
            while (!SIMDScanExecuteSingleOperation(&simdScanOperation)) { ; }

            endReached = simdScanOperation.endReached;
            keyHashsNoMatch = simdScanOperation.matchIndex == -1;

            // 2 Branches (|| and &&)
            keysMatch = endReached ||
                        (!keyHashsNoMatch && (strcmp(slice->keyColumn[simdScanOperation.matchIndex], needleStr) == 0));
            // 1 Branch
            continueScan = !endReached && !keysMatch;
            // 1 Branch
            slice->cacheIdx = !endReached && keysMatch ? simdScanOperation.matchIndex : slice->cacheIdx;
        }
    } while (continueScan);
    SIMDScanFree(&simdScanOperation);

    return cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? simdScanOperation.matchIndex : slice->numElems);
}


uint32_t SLICE_RESEARCH_INLINE(Slice *slice, Hash needleHash, const char *needleStr) {
    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN_SIMD for '%s' started", needleStr);
    assert(slice);
    assert(needleStr);

    register bool keysMatch = false, endReached = true;
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);
    register int matchIndex = -1;
    int low = 0;

    if (!cacheHit) {

        int mid;
        int high = SLICE_KEY_COLUMN_MAX_ELEMS;
        while (low < high) {
            mid = low + (high - low) / 2;
            if (needleHash <= slice->keyHashColumn[mid])
                high = mid;
            else
                low = mid + 1;
        }

        // No match found
        if (low != high) {
            endReached = true;
        }
        else {
            while(slice->keyHashColumn[low] == needleHash) {
                keysMatch = slice->keyColumn[low] && (strcmp(slice->keyColumn[slice->keyHashMapping[low]], needleStr) == 0);
                if (keysMatch) {
                    matchIndex = low;
                    keysMatch = true;
                    endReached = false;
                    break;
                }
                low++;
            }
        }
    }

    slice->cacheIdx = !endReached && keysMatch ? matchIndex : slice->cacheIdx;

    return cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? matchIndex : slice->numElems);
}



uint32_t
SLICE_RESEARCH_SIMD_KARY(Slice* slice, Hash needleHash, const char *needleStr) {
    __m256i simdSearchValue = _mm256_set1_epi64x(needleHash);
    register bool continueScan = false, keysMatch = false, keyHashsNoMatch, endReached;
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);

    register int matchIndex = -1;
    register size_t index = 0;
    __m256i simdSearchData;
    __m256i compareResult;
    register size_t i = 0;
    register size_t resultIndex;
    size_t startLevel = 0;

    size_t levels[] = {4, 24, 124, 624 };
    size_t levelMult[] =  {4,20, 100, 500};

    do {
        while (index < slice->numElemsAfterCompressing) {
            simdSearchData = _mm256_loadu_si256((__m256i *) (slice->keyHashColumn + index));

            compareResult = _mm256_cmpeq_epi64(simdSearchData, simdSearchValue);

            if (!_mm256_testz_si256(compareResult, compareResult)) {
                unsigned bitmask = _mm256_movemask_epi8(compareResult);
                matchIndex = index + _bit_scan_forward(bitmask) / 8;
                index = levels[startLevel];
                ++startLevel;
                break;
            }

            compareResult = _mm256_cmpgt_epi64(simdSearchValue, simdSearchData);
            if (!_mm256_testz_si256(compareResult, compareResult)) {
                unsigned bitmask = _mm256_movemask_epi8(compareResult);
                int bitPos = (_bit_scan_reverse(bitmask) / 8) + 1;
                index = levels[startLevel] + ((bitPos) * levelMult[startLevel]);
                ++startLevel;
            } else {
                index = levels[startLevel];
                ++startLevel;
            }
        }

        if (matchIndex == -1 || index >= slice->numElemsAfterCompressing) {
            endReached = true;
            break;
        }

        for (i = 0; i < slice->duplicates[matchIndex]; ++i) {
            resultIndex = slice->keyHashMapping2[slice->keyHashMapping[matchIndex + i]];
            if (strcmp(slice->keyColumn[resultIndex], needleStr) == 0) {
                slice->cacheIdx =  matchIndex;
                matchIndex = resultIndex;
                keysMatch = true;
                break;
            }
        }
    } while(true);

    if (!keysMatch && index < slice->numElems) {

        index = slice->numElemsAfterCompressing;
        // Take care of the rest with normal scan
 // slice->numElemsAfterCompressing;
        do {
            if (!cacheHit) {

                do {
                    matchIndex = -1;


                    int restElements = NG5_SIMD_COMPARE_ELEMENT_COUNT - (slice->numElems - index);
                    endReached = (restElements > 0 && index > NG5_SIMD_COMPARE_ELEMENT_COUNT);

                    simdSearchData = _mm256_loadu_si256((__m256i *) (slice->keyHashColumn + index));

                    compareResult = _mm256_cmpeq_epi64(simdSearchData, simdSearchValue);

                    if (!_mm256_testz_si256(compareResult, compareResult)) {
                        unsigned bitmask = _mm256_movemask_epi8(compareResult);
                        matchIndex = index + _bit_scan_forward(bitmask) / 8;
                    }
                    index += NG5_SIMD_COMPARE_ELEMENT_COUNT;

                } while (!(endReached || matchIndex > -1));

                keyHashsNoMatch = matchIndex == -1;

                keysMatch = endReached || (!keyHashsNoMatch && (strcmp(slice->keyColumn[matchIndex], needleStr) == 0));

                continueScan = !endReached && !keysMatch;
                slice->cacheIdx = !endReached && keysMatch ? matchIndex : slice->cacheIdx;
            }

        } while (continueScan);
    }

    return !endReached && keysMatch ? matchIndex : slice->numElems;

}

uint32_t SLICE_RESEARCH_BINARY(Slice *slice, Hash needleHash, const char *needleStr) {
    TRACE(NG5_SLICE_LIST_TAG, "SLICE_SCAN_SIMD for '%s' started", needleStr);
    assert(slice);
    assert(needleStr);

    register bool keysMatch = false, endReached = true;
    register bool cacheAvailable = (slice->cacheIdx != (uint32_t) -1);
    register bool hashsEq = cacheAvailable && (slice->keyHashColumn[slice->cacheIdx] == needleHash);
    register bool cacheHit = hashsEq && (strcmp(slice->keyColumn[slice->cacheIdx], needleStr) == 0);
    register int matchIndex = -1;
    int low = 0;

    if (!cacheHit) {

        int mid;
        int high = SLICE_KEY_COLUMN_MAX_ELEMS;
        while (low < high) {
            mid = low + (high - low) / 2;
            if (needleHash <= slice->keyHashColumn[mid])
                high = mid;
            else
                low = mid + 1;
        }

        // No match found
        if (low != high) {
            endReached = true;
        }
        else {
            while(slice->keyHashColumn[low] == needleHash) {
                keysMatch = slice->keyColumn[low] && (strcmp(slice->keyColumn[slice->keyHashMapping[low]], needleStr) == 0);
                if (keysMatch) {
                    matchIndex = low;
                    keysMatch = true;
                    endReached = false;
                    break;
                }
                low++;
            }
        }
    }

    slice->cacheIdx = !endReached && keysMatch ? matchIndex : slice->cacheIdx;

    return cacheHit ? slice->cacheIdx : (!endReached && keysMatch ? matchIndex : slice->numElems);
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

    static void appenderNew(SliceList *list);

    static void appenderSeal(Slice *slice, SliceList *list);

    static void lock(SliceList *list);

    static void unlock(SliceList *list);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

    int SliceListCreate(SliceList *list, const Allocator *alloc, size_t sliceCapacity) {
        CHECK_NON_NULL(list)
        CHECK_NON_NULL(sliceCapacity)

        AllocatorThisOrDefault(&list->alloc, alloc);
        SpinlockCreate(&list->lock);

        VectorCreate(&list->slices, &list->alloc, sizeof(Slice), sliceCapacity);
        VectorCreate(&list->descriptors, &list->alloc, sizeof(SliceDescriptor), sliceCapacity);
        VectorCreate(&list->filters, &list->alloc, sizeof(Bloomfilter), sliceCapacity);
        VectorCreate(&list->bounds, &list->alloc, sizeof(HashBounds), sliceCapacity);

        ZERO_MEMORY(VectorData(&list->slices), sliceCapacity * sizeof(Slice));
        ZERO_MEMORY(VectorData(&list->descriptors), sliceCapacity * sizeof(SliceDescriptor));
        ZERO_MEMORY(VectorData(&list->filters), sliceCapacity * sizeof(Bloomfilter));
        ZERO_MEMORY(VectorData(&list->bounds), sliceCapacity * sizeof(HashBounds));

        appenderNew(list);

        return STATUS_OK;
    }

    int SliceListDrop(SliceList *list) {
        UNUSED(list);
//    NOT_YET_IMPLEMENTED
        // TODO: implement
        VectorDrop(&list->slices);
        VectorDrop(&list->descriptors);
        VectorDrop(&list->bounds);
        for (size_t i = 0; i < list->filters.numElems; i++) {
            Bloomfilter *filter = VECTOR_GET(&list->filters, i, Bloomfilter);
            BloomfilterDrop(filter);
        }
        VectorDrop(&list->filters);
        return STATUS_OK;
    }

    int SliceListIsEmpty(const SliceList *list) {
        return (VectorIsEmpty(&list->slices));
    }

    int SliceListInsert(SliceList *list, char **strings, StringId *ids, size_t numPairs) {
        lock(list);

        while (numPairs--) {
            const char *key = *strings++;
            StringId value = *ids++;
            Hash keyHash = get_hashcode(key);
            SliceHandle handle;
            int status;

            assert (key);

            /* check whether the key-value pair is already contained in one slice */
            status = SliceListLookupByKey(&handle, list, key);

            if (status == STATUS_OK) {
                /* pair was found, do not insert it twice */
                assert (value == handle.value);
                continue;
            } else {
                /* pair is not found; append it */
                HashBounds *restrict bounds = VECTOR_ALL(&list->bounds, HashBounds);
                Bloomfilter *restrict filters = VECTOR_ALL(&list->filters, Bloomfilter);
                Slice *restrict slices = VECTOR_ALL(&list->slices, Slice);

                if (list->appenderIdx != 0) { ; // TODO: remove
                }

                Slice *restrict appender = slices + list->appenderIdx;
                Bloomfilter *restrict appenderFilter = filters + list->appenderIdx;
                HashBounds *restrict appenderBounds = bounds + list->appenderIdx;

                DEBUG(NG5_SLICE_LIST_TAG,
                      "appender # of elems: %zu, limit: %zu",
                      appender->numElems,
                      SLICE_KEY_COLUMN_MAX_ELEMS);
                assert(appender->numElems < SLICE_KEY_COLUMN_MAX_ELEMS);
                appender->keyColumn[appender->numElems] = key;
                appender->keyHashColumn[appender->numElems] = keyHash;
                appender->stringIdColumn[appender->numElems] = value;
                appenderBounds->minHash = appenderBounds->minHash < keyHash ?
                                          appenderBounds->minHash : keyHash;
                appenderBounds->maxHash = appenderBounds->maxHash > keyHash ?
                                          appenderBounds->maxHash : keyHash;
                BLOOMFILTER_SET(appenderFilter, &keyHash, sizeof(Hash));
                appender->numElems++;
                if (BRANCH_UNLIKELY(appender->numElems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                    appenderSeal(appender, list);
                    appenderNew(list);
                }
            }
        }

        unlock(list);
        return STATUS_OK;
    }

    int SliceListLookupByKey(SliceHandle *handle, SliceList *list, const char *needle) {
        UNUSED(list);
        UNUSED(handle);
        UNUSED(needle);

        Hash keyHash = get_hashcode(needle);
        uint32_t numSlices = VectorLength(&list->slices);

        /* check whether the key-value pair is already contained in one slice */
        HashBounds *restrict bounds = VECTOR_ALL(&list->bounds, HashBounds);
        Bloomfilter *restrict filters = VECTOR_ALL(&list->filters, Bloomfilter);
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
                    Bloomfilter *restrict filter = filters + i;
                    bool maybeContained = BLOOMFILTER_TEST(filter, &keyHash, sizeof(Hash));
                    if (maybeContained) {
                        DEBUG(NG5_SLICE_LIST_TAG, "ng5_slice_list_lookup_by_key key(%s) -> ?", needle);
                        uint32_t pairPosition;

#ifdef ENABLE_MEASURE_TIME_SCANNED
                        Timestamp scan_begin = TimeCurrentSystemTime();
#endif
                        switch (slice->strat) {
                            case SLICE_LOOKUP_SCAN:

#ifdef ENABLE_BASELINE
                                pairPosition = SLICE_SCAN(slice, keyHash, needle);
#endif
#ifdef ENABLE_SLICE_SCAN_SIMD
                                pairPosition = SLICE_SCAN_SIMD(slice, keyHash, needle);
#endif

#ifdef ENABLE_SLICE_SCAN_SIMD_INLINE
                                pairPosition = SLICE_SCAN_SIMD_INLINE(slice, keyHash, needle);
#endif

#ifdef ENABLE_SLICE_SCAN_SIMD_MACRO
                                pairPosition = SLICE_SCAN_SIMD_INLINE_MACRO(slice, keyHash, needle);
#endif
                                break;
                            case SLICE_LOOKUP_BESEARCH:
#ifdef ENABLE_BASELINE_RESEARCH
                                pairPosition = SLICE_SCAN(slice, keyHash, needle);
#endif
#ifdef ENABLE_SEALED_KARY_SIMD_SCAN
                                pairPosition = SLICE_RESEARCH_SIMD_KARY(slice, keyHash, needle);
#endif

#ifdef ENABLE_SEALED_BINARY_SCAN
                                pairPosition = SLICE_RESEARCH_BINARY(slice, keyHash, needle);
#endif
                                break;
                            default: PANIC("unknown slice find strategy");
                        }


#ifdef ENABLE_MEASURE_TIME_SCANNED
                        Timestamp scan_end = TimeCurrentSystemTime();
                        timeElapsedScanning += (scan_end - scan_begin);
#endif

                        DEBUG(NG5_SLICE_LIST_TAG,
                              "ng5_slice_list_lookup_by_key key(%s) -> pos(%zu in slice #%zu)",
                              needle,
                              pairPosition,
                              i);
                        if (pairPosition < slice->numElems) {
                            /* pair is contained */
                            desc->numReadsHit++;
                            handle->isContained = true;
                            handle->value = slice->stringIdColumn[pairPosition];
                            handle->key = needle;
                            handle->container = slice;

                            desc->numReadsHit++;
                            return STATUS_OK;
                        }
                    } else {
                        /* bloomfilter is sure that pair is not contained */
                        continue;
                    }
                } else {
                    /* key hash is not inside bounds of hashes in slice */
                    continue;
                }
            }
        }

        handle->isContained = false;

        return STATUS_NOTFOUND;
    }

    int SliceListRemove(SliceList *list, SliceHandle *handle) {
        UNUSED(list);
        UNUSED(handle);
        NOT_YET_IMPLEMENTED
    }

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

    static void appenderNew(SliceList *list) {
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
        assert(sizeof(Bloomfilter) <= NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
        Bloomfilter filter;

        /* NOTE: the size of each bloomfilter lead to a false positive probability of 100%, i.e., number of items in the
         * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
         * in bits minus the header for the bloomfilter) along with the number of used hash functions (4), lead to that
         * probability. However, the reason a bloomfilter is used is to skip slices whch definitively do NOT contain the
         * key-value pair - and that still works ;) */
        BloomfilterCreate(&filter,
                          (NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(Bloomfilter)) * 8);
        VectorPush(&list->filters, &filter, 1);
        HashBounds bounds = {
                .minHash        = (Hash) -1,
                .maxHash        = (Hash) 0
        };
        VectorPush(&list->bounds, &bounds, 1);

        INFO(NG5_SLICE_LIST_TAG, "created new appender in slice list %p\n\t"
                                 "# of slices (incl. appender) in total...............: %zu\n\t"
                                 "Slice target memory size............................: %zuB (%s)\n\t"
                                 "Bloomfilter target memory size......................: %zuB (%s)\n\t"
                                 "Max # of (key, hash, string) in appender/slice......: %zu\n\t"
                                 "Bits used in per-slice bloomfilter..................: %zu\n\t"
                                 "Prob. of bloomfilter to produce false-positives.....: %f\n\t"
                                 "Single slice type size..............................: %zuB\n\t"
                                 "Total slice-list size...............................: %f MiB",
             list,
             list->slices.numElems,
             (size_t) NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
             NG5_SLICE_LIST_TARGET_MEMORY_NAME,
             (size_t) NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_0;
                     0;
                     50000;
                     0.642000;
                     101.079002;
                     101.721001;
                     38025251;
                     38025251;
                     18276368
                     IN_BYTE,
             NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
             (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
             BitmapNumBits(&filter),
             (pow(1 - exp(-(double) BitmapNumHashs()
                          / ((double) BitmapNumBits(&filter) / (double) SLICE_KEY_COLUMN_MAX_ELEMS)),
                  BitmapNumHashs(&filter))),
             sizeof(Slice),
             (sizeof(SliceList) + list->slices.numElems
                                  *
                                  (sizeof(Slice) + sizeof(SliceDescriptor) +
                                   (sizeof(uint32_t) * list->descriptors.numElems)
                                   + sizeof(Bloomfilter) + BitmapNumBits(&filter) / 8 + sizeof(HashBounds))) / 1024.0
             / 1024.0
        );

        /* register new slice as the current appender */
        list->appenderIdx = numSlices;
    }


    static void appenderSeal(Slice *slice, SliceList *list) {
        UNUSED(slice);
        UNUSED(list);

#ifdef ENABLE_MEASURE_TIME_SEALED
        Timestamp scan_begin = TimeCurrentSystemTime();
#endif
#ifdef ENABLE_SEALED_BINARY_SCAN
        Hash keyHashColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
        memcpy(keyHashColumn, slice->keyHashColumn, sizeof(keyHashColumn));

        selectionSort(keyHashColumn, SLICE_KEY_COLUMN_MAX_ELEMS, slice->keyHashMapping);

        lock(list);

        memcpy(slice->keyHashColumn, keyHashColumn, sizeof(keyHashColumn));
        slice->strat = SLICE_LOOKUP_BESEARCH;

        unlock(list);

#endif

#ifdef ENABLE_SEALED_KARY_SIMD_SCAN

        /**
         * The construction of the duplicate free k-ary search tree comes with high effort
         * Here we can surely optimize parts, e.g. creating the mappings/tree in place instead
         * of using new arrays
         */
        Hash keyHashColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
        Hash compressedColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
        Hash newHashes[SLICE_KEY_COLUMN_MAX_ELEMS];
        Hash newDuplicates[SLICE_KEY_COLUMN_MAX_ELEMS];
        Hash newMapping[SLICE_KEY_COLUMN_MAX_ELEMS];
        memcpy(keyHashColumn, slice->keyHashColumn, sizeof(keyHashColumn));

        size_t i;
        for(i = 0; i < SLICE_KEY_COLUMN_MAX_ELEMS; ++i) {
            slice->keyHashMapping[i] = i;
        }

        // Sort and create first mapping
        selectionSort(keyHashColumn, SLICE_KEY_COLUMN_MAX_ELEMS, slice->keyHashMapping);

        size_t result = removeDuplicates2(keyHashColumn, SLICE_KEY_COLUMN_MAX_ELEMS, newDuplicates, newHashes, newMapping);


        // Fill up to next size for complete k-ary search tree
        memcpy(&slice->keyHashMapping2, &slice->keyHashMapping, sizeof(slice->keyHashMapping));
        memcpy(compressedColumn, newHashes, sizeof(keyHashColumn));

        if(result < 124 && SLICE_KEY_COLUMN_MAX_ELEMS >= 124) {
            size_t newResult = 124;
            fillUpCompressedArray(newHashes, result, 124);
            slice_linearize(newHashes, compressedColumn, 0, newResult, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);
            // memcpy(compressedColumn + newResult, newHashes + newResult, result - newResult);
        }
        else if(result < 124) {
            size_t newResult = 24;
            slice_linearize(newHashes, compressedColumn, 0, newResult, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);
            // memcpy(compressedColumn + newResult, newHashes + newResult, result - newResult);
        }
        else if(result < 624 && SLICE_KEY_COLUMN_MAX_ELEMS >= 624) {
            size_t newResult = 624;
            fillUpCompressedArray(newHashes, result, 624);
            slice_linearize(newHashes, compressedColumn, 0, newResult, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);
            // memcpy(compressedColumn + newResult, newHashes + newResult, result - newResult);
        }
        else if(result < 624) {
            size_t newResult = 124;
            slice_linearize(newHashes, compressedColumn, 0, newResult, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);
            // memcpy(compressedColumn + newResult, newHashes + newResult, result - newResult);
        }
        else {
            size_t newResult = 624;
            slice_linearize(newHashes, compressedColumn, 0, newResult, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);
            //memcpy(compressedColumn + newResult, newHashes + newResult, result - newResult);
        }

        slice->numElemsAfterCompressing = result;

        // This one does the magic, create a linearized version of the tree
        // slice_linearize(newHashes, compressedColumn, 0, result, newMapping, newDuplicates, 5, slice->keyHashMapping2, slice->duplicates);

        lock(list);

         memcpy(slice->keyHashColumn, compressedColumn, sizeof(keyHashColumn));
         slice->strat = SLICE_LOOKUP_BESEARCH;

        unlock(list);
#endif

#ifdef ENABLE_MEASURE_TIME_SEALED
        Timestamp scan_end = TimeCurrentSystemTime();
        timeElapsedSealing += (scan_end - scan_begin);
#endif
    }


    static void lock(SliceList *list) {
        SpinlockAcquire(&list->lock);
    }

    static void unlock(SliceList *list) {
        SpinlockRelease(&list->lock);
    }