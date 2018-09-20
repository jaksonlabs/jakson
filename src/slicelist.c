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

#define get_hashcode(key)    hash_additive(strlen(key), key)

/* OPTIMIZATION: we have only one item to find. Use branch-less scan instead of branching scan */
/* OPTIMIZATION: find function as macro */
#define slice_find_scan_default(slice, needle_hash, needle_str)                                                        \
({                                                                                                                     \
    TRACE(NG5_SLICE_LIST_TAG, "slice_find_scan_default for '%s' started", needle_str);                                 \
    assert(slice);                                                                                                     \
    assert(needle_str);                                                                                                \
                                                                                                                       \
    register bool          continue_scan, key_match, key_hash_no_match, end_reached;                                   \
    register bool          cache_on   = (slice->cache_idx != (uint32_t) -1);                                           \
    register bool          hash_eq    = cache_on && (slice->key_hash_column[slice->cache_idx] == needle_hash);         \
    register bool          cache_hit  = hash_eq && (strcmp(slice->key_column[slice->cache_idx], needle_str) == 0);     \
    register uint_fast32_t i          = 0;                                                                             \
    if (!cache_hit) {                                                                                                  \
        do {                                                                                                           \
            while ((key_hash_no_match = (slice->key_hash_column[i]!=needle_hash)) && i++<slice->num_elems) { ; }       \
            end_reached    = ((i+1)>slice->num_elems);                                                                 \
            key_match      = end_reached || (!key_hash_no_match && (strcmp(slice->key_column[i], needle_str)==0));     \
            continue_scan  = !end_reached && !key_match;                                                               \
            i             += continue_scan;                                                                            \
        }                                                                                                              \
        while (continue_scan);                                                                                         \
        slice->cache_idx = !end_reached && key_match ? i : slice->cache_idx;                                           \
    }                                                                                                                  \
    cache_hit ? slice->cache_idx : (!end_reached && key_match ? i : slice->num_elems);                                 \
})

#define slice_find_besearch(slice, needle_hash, needle_str)                                                            \
({                                                                                                                     \
    0; \
})
// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

static void appender_new(ng5_slice_list_t *list);
static void appender_seal(ng5_slice_t *slice);

uint32_t slice_find_scan_2(ng5_slice_t *slice, hash_t needle_hash, const char *needle_str);

static void lock(ng5_slice_list_t *list);
static void unlock(ng5_slice_list_t *list);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int ng5_slice_list_create(ng5_slice_list_t *list, const Allocator *alloc, size_t slice_cap)
{
    CHECK_NON_NULL(list)
    CHECK_NON_NULL(slice_cap)

    AllocatorThisOrDefault(&list->alloc, alloc);
    ng5_spinlock_create(&list->spinlock);

    VectorCreate(&list->slices, &list->alloc, sizeof(ng5_slice_t), slice_cap);
    VectorCreate(&list->descriptors, &list->alloc, sizeof(ng5_slice_desc_t), slice_cap);
    VectorCreate(&list->filters, &list->alloc, sizeof(Bloomfilter), slice_cap);
    VectorCreate(&list->bounds, &list->alloc, sizeof(ng5_hash_bounds_t), slice_cap);

    ZERO_MEMORY(ng5_vector_data(&list->slices), slice_cap * sizeof(ng5_slice_t));
    ZERO_MEMORY(ng5_vector_data(&list->descriptors), slice_cap * sizeof(ng5_slice_desc_t));
    ZERO_MEMORY(ng5_vector_data(&list->filters), slice_cap * sizeof(Bloomfilter));
    ZERO_MEMORY(ng5_vector_data(&list->bounds), slice_cap * sizeof(ng5_hash_bounds_t));

    appender_new(list);

    return STATUS_OK;
}

int ng5_slice_list_drop(ng5_slice_list_t *list)
{
    UNUSED(list);
//    NOT_YET_IMPLEMENTED
    // TODO: implement
    VectorDrop(&list->slices);
    VectorDrop(&list->descriptors);
    VectorDrop(&list->bounds);
    for (size_t i = 0; i < list->filters.num_elems; i++) {
        Bloomfilter *filter = ng5_vector_get(&list->filters, i, Bloomfilter);
        BloomfilterDrop(filter);
    }
    VectorDrop(&list->filters);
    return STATUS_OK;
}

int ng5_slice_list_is_empty(const ng5_slice_list_t *list)
{
    return (ng5_vector_is_empty(&list->slices));
}

int ng5_slice_list_insert(ng5_slice_list_t *list, char **strings, StringId *ids, size_t npairs)
{
    lock(list);

    while (npairs--) {

        const char *key = *strings++;
        StringId value = *ids++;
        hash_t key_hash = get_hashcode(key);
        ng5_slice_handle_t handle;
        int status;

        assert (key);

        /* check whether the key-value pair is already contained in one slice */
        status = ng5_slice_list_lookup_by_key(&handle, list, key);

        if (status == STATUS_OK) {
            /* pair was found, do not insert it twice */
            assert (value == handle.value);
            continue;
        }
        else {
            /* pair is not found; append it */
            ng5_hash_bounds_t *restrict bounds = ng5_vector_all(&list->bounds, ng5_hash_bounds_t);
            Bloomfilter *restrict filters = ng5_vector_all(&list->filters, Bloomfilter);
            ng5_slice_t *restrict slices = ng5_vector_all(&list->slices, ng5_slice_t);

            if (list->appender_idx != 0) { ; // TODO: remove
            }

            ng5_slice_t *restrict appender = slices + list->appender_idx;
            Bloomfilter *restrict appender_filter = filters + list->appender_idx;
            ng5_hash_bounds_t *restrict appender_bounds = bounds + list->appender_idx;

            DEBUG(NG5_SLICE_LIST_TAG,
                  "appender # of elems: %zu, limit: %zu",
                  appender->num_elems,
                  SLICE_KEY_COLUMN_MAX_ELEMS);
            assert(appender->num_elems < SLICE_KEY_COLUMN_MAX_ELEMS);
            appender->key_column[appender->num_elems] = key;
            appender->key_hash_column[appender->num_elems] = key_hash;
            appender->string_id_column[appender->num_elems] = value;
            appender_bounds->min_hash = appender_bounds->min_hash < key_hash ?
                                        appender_bounds->min_hash : key_hash;
            appender_bounds->max_hash = appender_bounds->max_hash > key_hash ?
                                        appender_bounds->max_hash : key_hash;
            BLOOMFILTER_SET(appender_filter, &key_hash, sizeof(hash_t));
            appender->num_elems++;
            if (BRANCH_UNLIKELY(appender->num_elems == SLICE_KEY_COLUMN_MAX_ELEMS)) {
                appender_seal(appender);
                appender_new(list);
            }
        }
    }

    unlock(list);
    return STATUS_OK;
}

int ng5_slice_list_lookup_by_key(ng5_slice_handle_t *handle, ng5_slice_list_t *list, const char *needle)
{
    UNUSED(list);
    UNUSED(handle);
    UNUSED(needle);

    hash_t key_hash = get_hashcode(needle);
    uint32_t num_slices = ng5_vector_len(&list->slices);

    /* check whether the key-value pair is already contained in one slice */
    ng5_hash_bounds_t *restrict bounds = ng5_vector_all(&list->bounds, ng5_hash_bounds_t);
    Bloomfilter *restrict filters = ng5_vector_all(&list->filters, Bloomfilter);
    ng5_slice_t *restrict slices = ng5_vector_all(&list->slices, ng5_slice_t);
    ng5_slice_desc_t *restrict descs = ng5_vector_all(&list->descriptors, ng5_slice_desc_t);

    for (register uint32_t i = 0; i < num_slices; i++) {
        ng5_slice_desc_t *restrict desc = descs + i;
        ng5_hash_bounds_t *restrict bound = bounds + i;
        ng5_slice_t *restrict slice = slices + i;

        desc->num_reads_all++;

        if (slice->num_elems > 0) {
            bool key_hash_in = key_hash >= bound->min_hash && key_hash <= bound->max_hash;
            if (key_hash_in) {
                Bloomfilter *restrict filter = filters + i;
                bool maybe_in = BLOOMFILTER_TEST(filter, &key_hash, sizeof(hash_t));
                if (maybe_in) {
                    DEBUG(NG5_SLICE_LIST_TAG, "ng5_slice_list_lookup_by_key key(%s) -> ?", needle);
                    uint32_t pair_pos;

                    switch (slice->strat) {
                    case SLICE_LOOKUP_SCAN:pair_pos = slice_find_scan_default(slice, key_hash, needle);
                        break;
                    case SLICE_LOOKUP_BESEARCH:pair_pos = slice_find_besearch(slice, key_hash, needle);
                        break;
                    default: PANIC("unknown slice find strategy");
                    }

                    DEBUG(NG5_SLICE_LIST_TAG,
                          "ng5_slice_list_lookup_by_key key(%s) -> pos(%zu in slice #%zu)",
                          needle,
                          pair_pos,
                          i);
                    if (pair_pos < slice->num_elems) {
                        /* pair is contained */
                        desc->num_reads_hit++;
                        handle->is_contained = true;
                        handle->value = slice->string_id_column[pair_pos];
                        handle->key = needle;
                        handle->container = slice;

                        desc->num_reads_hit++;
                        return STATUS_OK;
                    }
                }
                else {
                    /* bloomfilter is sure that pair is not contained */
                    continue;
                }
            }
            else {
                /* key hash is not inside bounds of hashes in slice */
                continue;
            }
        }
    }

    handle->is_contained = false;

    return STATUS_NOTFOUND;
}

int ng5_slice_list_remove(ng5_slice_list_t *list, ng5_slice_handle_t *handle)
{
    UNUSED(list);
    UNUSED(handle);
    NOT_YET_IMPLEMENTED
}

int ng5_slice_list_lock(ng5_slice_list_t *list)
{
    UNUSED(list);
    //  ng5_spinlock_lock(&list->spinlock);
    return STATUS_OK;
}

int ng5_slice_list_unlock(ng5_slice_list_t *list)
{
    UNUSED(list);
    //   ng5_spinlock_unlock(&list->spinlock);
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------



UNUSED_FUNCTION uint32_t slice_find_scan_2(ng5_slice_t *slice, hash_t needle_hash, const char *needle_str)
{
    TRACE(NG5_SLICE_LIST_TAG, "slice_find_scan_default for '%s' started", needle_str);
    assert(slice);
    assert(needle_str);


    if (BRANCH_LIKELY(slice->cache_idx != (uint32_t) -1)) {
        bool hash_eq = (slice->key_hash_column[slice->cache_idx] != needle_hash);
        if (hash_eq && strcmp(slice->key_column[slice->cache_idx], needle_str) == 0) {
            TRACE(NG5_SLICE_LIST_TAG, "slice_find_scan_default for '%s': DONE (in cache)", needle_str);
            return slice->cache_idx;
        }
    }

    for (size_t i = 0; i < slice->num_elems; i++) {
        assert (slice->key_column[i]);
        if (slice->key_hash_column[i] == needle_hash && strcmp(slice->key_column[i], needle_str) == 0) {
            slice->cache_idx = i;
            return i;
        }
    }

    return slice->num_elems;
}

static void appender_new(ng5_slice_list_t *list)
{
    /* ANTI-OPTIMIZATION: madvising sequential access to columns in slice decrease performance */

    /* the slice itself */
    ng5_slice_t slice = {
        .strat     = SLICE_LOOKUP_SCAN,
        .num_elems = 0,
        .cache_idx = (uint32_t) -1
    };

    uint32_t num_slices = ng5_vector_len(&list->slices);
    VectorPush(&list->slices, &slice, 1);

    assert(SLICE_KEY_COLUMN_MAX_ELEMS > 0);

    /* the descriptor */
    ng5_slice_desc_t desc = {
        .num_reads_hit  = 0,
        .num_reads_all  = 0,
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
    ng5_hash_bounds_t bounds = {
        .min_hash        = (hash_t) -1,
        .max_hash        = (hash_t) 0
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
         list->slices.num_elems,
         (size_t) NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
         NG5_SLICE_LIST_TARGET_MEMORY_NAME,
         (size_t) NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE,
         NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
         (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
         BitmapNumBits(&filter),
         (pow(1 - exp(-(double) BitmapNumHashs()
                          / ((double) BitmapNumBits(&filter) / (double) SLICE_KEY_COLUMN_MAX_ELEMS)),
              BitmapNumHashs(&filter))),
         sizeof(ng5_slice_t),
         (sizeof(ng5_slice_list_t) + list->slices.num_elems
             * (sizeof(ng5_slice_t) + sizeof(ng5_slice_desc_t) + (sizeof(uint32_t) * list->descriptors.num_elems)
                 + sizeof(Bloomfilter) + BitmapNumBits(&filter) / 8 + sizeof(ng5_hash_bounds_t))) / 1024.0
             / 1024.0
    );

    /* register new slice as the current appender */
    list->appender_idx = num_slices;
}

static void appender_seal(ng5_slice_t *slice)
{
    UNUSED(slice);
    //  slice->cache_idx = 0;
    //  slice_sort(slice);
    //  slice->strat = SLICE_LOOKUP_BESEARCH;

    // TODO: sealing means (radix) sort and then replace 'find' with bsearch or something. Not yet implemented: sealed slices are also search in a linear fashion
}

static void lock(ng5_slice_list_t *list)
{
    ng5_spinlock_lock(&list->spinlock);
}

static void unlock(ng5_slice_list_t *list)
{
    ng5_spinlock_unlock(&list->spinlock);
}