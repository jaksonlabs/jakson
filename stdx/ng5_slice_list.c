#include <stdx/ng5_slice_list.h>

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

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

static void appender_new(ng5_slice_list_t* list);

uint32_t slice_find_scan_default(ng5_slice_t *slice, hash_t needle_hash, const char *needle_str);

static void lock(ng5_slice_list_t* list);
static void unlock(ng5_slice_list_t* list);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int ng5_slice_list_create(ng5_slice_list_t *list, const ng5_allocator_t *alloc, size_t slice_cap)
{
    check_non_null(list)
    check_non_null(slice_cap)

    allocator_this_or_default(&list->alloc, alloc);
    ng5_spinlock_create(&list->spinlock);

    list->appender  = NULL;

    ng5_vector_create(&list->slices, &list->alloc, sizeof(ng5_slice_t), slice_cap);
    ng5_vector_create(&list->descriptors, &list->alloc, sizeof(ng5_slice_desc_t), slice_cap);
    ng5_vector_create(&list->filters, &list->alloc, sizeof(ng5_bloomfilter_t), slice_cap);
    ng5_vector_create(&list->bounds, &list->alloc, sizeof(ng5_hash_bounds_t), slice_cap);

    appender_new(list);

    return STATUS_OK;
}

int ng5_slice_list_drop(ng5_slice_list_t *list)
{
    unused(list);
    NOT_YET_IMPLEMENTED
}

int ng5_slice_list_lookup(ng5_slice_handle_t *handle, ng5_slice_list_t *list, const void *needle)
{
    unused(handle);
    unused(list);
    unused(needle);
    NOT_YET_IMPLEMENTED
}

int ng5_slice_list_insert(ng5_slice_list_t *list, char ** strings, string_id_t *ids, size_t npairs)
{
    unused(list);
    unused(strings);
    unused(ids);
    unused(npairs);

    lock(list);
//
//    while (npairs--) {
//
//        entry_pair_t insert_pair = {
//            .key              = *strings++,
//            .value            = *ids++,
//            .key_hash         = get_hashcode(insert_pair.key)
//        };
//
//        /* Search for pair via its key-hash (to avoid strcmp-calls) */
//        size_t             max_slices  = ng5_vector_len(&list->descriptors);
//        ng5_slice_desc_t  *descriptors = ng5_vector_all(&list->descriptors, ng5_slice_desc_t);
//        ng5_bloomfilter_t *guards      = ng5_vector_all(&list->filters, ng5_bloomfilter_t);
//
//        // TODO: rank and reorder descs is not implemented... do it ;)
//        for (size_t desc_idx = 0; desc_idx < max_slices; desc_idx++) {
//            ng5_slice_desc_t  *desc    = descriptors + desc_idx;
//            ng5_slice_t       *slice   = desc->slice;
//            ng5_bloomfilter_t *guard   = guards + desc_idx;
//
//            desc->num_reads_all++;
//
//            if (slice->data[desc->min_idx].key_hash <= insert_pair.key_hash &&
//                    slice->data[desc->max_idx].key_hash <= insert_pair.key_hash &&
//                    ng5_bloomfilter_test(guard, insert_pair.key_hash, sizeof(hash_t))) {
//                /* There is a probability that the (key, value)-pair is stored in that particular slice,
//                 * since the key hash is in the bounds of the hashes stored inside the slice and the
//                 * bloomfilter is not sure whether the pair is contained or not */
//                uint32_t pos = slice->find(slice, &insert_pair);
//                if (pos < slice->num_elems) {
//                    /* pair is already inserted */
//                    desc->num_reads_hit++;
//                    goto continue_with_next_pair;
//                }
//            }
//        }
//
//        /* If this point is reached, the pair is not contained in any slice (including the appender) so far */
//        /* Insert the entry into the appender, and register the key hash for the minimum and maximum bound and in the
//         * bloomfilter */
//        ng5_slice_t      *appender        = list->appender;
//        uint32_t          insert_pair_pos = appender->num_elems++;
//        uint32_t          old_max_idx     = appender->desc->max_idx;
//        uint32_t          old_min_idx     = appender->desc->min_idx;
//        ng5_slice_desc_t *app_desc        = appender->desc;
//        entry_pair_t     *app_data        = appender->data;
//
//        app_data[insert_pair_pos]         = insert_pair;
//
//        if (old_max_idx != SLICE_KEY_COLUMN_MAX_ELEMS) {
//            assert(old_min_idx != SLICE_KEY_COLUMN_MAX_ELEMS);
//
//            app_desc->max_idx = app_data[old_max_idx].key_hash < app_data[insert_pair_pos].key_hash ?
//                                insert_pair_pos : old_max_idx;
//            app_desc->min_idx = app_data[old_min_idx].key_hash > app_data[insert_pair_pos].key_hash ?
//                                insert_pair_pos : old_min_idx;
//
//            ng5_bloomfilter_set(appender->)
//        }
//
//        /* pair was inserted, or already found */
//continue_with_next_pair:
//        continue;
//    }

    unlock(list);
    return STATUS_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

uint32_t slice_find_scan_default(ng5_slice_t *slice, hash_t needle_hash, const char *needle_str)
{
    unused(slice);
    unused(needle_hash);
    unused(needle_str);
    return 0;
}

static void appender_new(ng5_slice_list_t* list)
{
    /* the slice itself */
    ng5_slice_t  slice       = {
        .find      = slice_find_scan_default,
        .strat     = SLICE_LOOKUP_SCAN,
        .num_elems = 0
    };

    ng5_slice_t *slice_ptr    =     ng5_vector_push_and_get(&list->slices, &slice, ng5_slice_t);
    size_t max_elems_in_slice =     SLICE_KEY_COLUMN_MAX_ELEMS;

    assert(max_elems_in_slice > 0);

    /* the descriptor */
    ng5_slice_desc_t desc = {
        .num_reads_hit  = 0,
        .num_reads_all  = 0,
        .num_freelist   = SLICE_KEY_COLUMN_MAX_ELEMS,
        .freelist       = allocator_malloc(&list->alloc, max_elems_in_slice * sizeof(uint32_t))
    };

    while (max_elems_in_slice--) {
        desc.freelist[max_elems_in_slice] = max_elems_in_slice;
    }

    ng5_vector_push(&list->descriptors, &desc, 1);

    /* the lookup guards */
    assert(sizeof(ng5_bloomfilter_t) <= NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE);
    ng5_bloomfilter_t filter;

    /* NOTE: the size of each bloomfilter lead to a false positive probability of 100%, i.e., number of items in the
     * slice is around 32644 depending on the CPU cache size, the number of actual bits in the filter (Cache line size
     * in bits minus the header for the bloomfilter) along with the number of used hash functions (4), lead to that
     * probability. However, the reason a bloomfilter is used is to skip slices whch definitively do NOT contain the
     * key-value pair - and that still works ;) */
    ng5_bloomfilter_create(&filter, (NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(ng5_bloomfilter_t)) * 8);
    ng5_vector_push(&list->filters, &filter, 1);
    ng5_hash_bounds_t bounds = {
         .min_idx        = SLICE_KEY_COLUMN_MAX_ELEMS,
         .max_idx        = SLICE_KEY_COLUMN_MAX_ELEMS,
    };
    ng5_vector_push(&list->bounds, &bounds, 1);

    info(NG5_SLICE_LIST_TAG, "created new appender in slice list %p\n\t"
            "# of slices (incl. appender) in total...............: %zu\n\t"
            "Slice target memory size............................: %zuB (%s)\n\t"
            "Bloomfilter target memory size......................: %zuB (%s)\n\t"
            "Max # of (key, hash, string) in appender/slice......: %zu\n\t"
            "Bits used in per-slice bloomfilter..................: %zu\n\t"
            "Prob. of bloomfilter to produce false-positives.....: %f%%\n\t"
            "Single slice type size..............................: %zuB\n\t"
            "Total slice-list size...............................: %f MiB",
            list,
            list->slices.num_elems,
            (size_t) NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE,
            NG5_SLICE_LIST_TARGET_MEMORY_NAME,
            (size_t) NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE,
            NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME,
            (size_t) SLICE_KEY_COLUMN_MAX_ELEMS,
            ng5_bloomfilter_nbits(&filter),
            100 * (pow(1 - exp(-(double)ng5_bloomfilter_nhashes() / ((double)ng5_bloomfilter_nbits(&filter) / (double)SLICE_KEY_COLUMN_MAX_ELEMS)), ng5_bloomfilter_nhashes(&filter))),
            sizeof(ng5_slice_t),
            (sizeof(ng5_slice_list_t) + list->slices.num_elems* (sizeof(ng5_slice_t) +sizeof(ng5_slice_desc_t) + (sizeof(uint32_t) * list->descriptors.num_elems) +sizeof(ng5_bloomfilter_t) + ng5_bloomfilter_nbits(&filter)/8 +sizeof(ng5_hash_bounds_t)))/1024.0/1024.0
            );

    /* register new slice as the current appender */
    list->appender = slice_ptr;
}

unused_fn static void appender_insert(ng5_slice_list_t* list)
{
    unused(list);
}

static void lock(ng5_slice_list_t* list)
{
    ng5_spinlock_lock(&list->spinlock);
}

static void unlock(ng5_slice_list_t* list)
{
    ng5_spinlock_unlock(&list->spinlock);
}