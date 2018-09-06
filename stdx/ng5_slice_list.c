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
static void appender_seal(ng5_slice_t* slice);

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

    ng5_vector_create(&list->slices, &list->alloc, sizeof(ng5_slice_t), slice_cap);
    ng5_vector_create(&list->descriptors, &list->alloc, sizeof(ng5_slice_desc_t), slice_cap);
    ng5_vector_create(&list->filters, &list->alloc, sizeof(ng5_bloomfilter_t), slice_cap);
    ng5_vector_create(&list->bounds, &list->alloc, sizeof(ng5_hash_bounds_t), slice_cap);

    appender_new(list);

    return STATUS_OK;
}

int ng5_slice_list_insert(ng5_slice_list_t *list, char ** strings, string_id_t *ids, size_t npairs)
{
    unused(list);
    unused(strings);
    unused(ids);
    unused(npairs);

    lock(list);

    while (npairs--) {

        const char                        * key               = *strings++;
        string_id_t                         value             = *ids++;
        hash_t                              key_hash          = get_hashcode(key);
        uint32_t                            num_slices        = ng5_vector_len(&list->slices);

        /* check whether the key-value pair is already contained in one slice */
        ng5_hash_bounds_t        * restrict bounds      = ng5_vector_all(&list->bounds, ng5_hash_bounds_t);
        ng5_bloomfilter_t        * restrict filters     = ng5_vector_all(&list->filters, ng5_bloomfilter_t);
        ng5_slice_t              * restrict slices      = ng5_vector_all(&list->slices, ng5_slice_t);
        ng5_slice_desc_t         * restrict descs       = ng5_vector_all(&list->descriptors, ng5_slice_desc_t);

        ng5_slice_t              * restrict appender        = slices + list->appender_idx;
        ng5_bloomfilter_t        * restrict appender_filter = filters + list->appender_idx;
        ng5_hash_bounds_t        * restrict appender_bounds = bounds + list->appender_idx;

        for (register uint32_t i = 0; i < num_slices; i++) {
            ng5_slice_desc_t *restrict      desc        = descs + i;
            ng5_hash_bounds_t *restrict     bound       = bounds + i;
            bool                            key_hash_in = key_hash >= bound->min_hash && key_hash <= bound->max_hash;

            desc->num_reads_all++;

            if (key_hash_in) {
                ng5_bloomfilter_t *restrict filter       = filters + i;
                bool                        maybe_in     = ng5_bloomfilter_test(filter, &key_hash, sizeof(hash_t));
                if (maybe_in) {
                    ng5_slice_t  *restrict  slice        = slices + i;
                    uint32_t                pair_pos     = slice->find(slice, key_hash, key);
                    if (pair_pos < SLICE_KEY_COLUMN_MAX_ELEMS) {
                        /* pair is contained, do not insert it twice */
                        assert(slice->string_id_column[pair_pos] == value);
                        desc->num_reads_hit++;
                        goto next_pair;
                    }
                } else {
                    /* bloomfilter is sure that pair is not contained */
                    goto insert_pair;
                }
            } else {
                /* key hash is not inside bounds of hashes in slice */
                goto insert_pair;
            }
        }

insert_pair:
        assert(appender->num_elems < SLICE_KEY_COLUMN_MAX_ELEMS);
        appender->key_column[appender->num_elems]       = key;
        appender->key_hash_column[appender->num_elems]  = key_hash;
        appender->string_id_column[appender->num_elems] = value;
        appender_bounds->min_hash                       = appender_bounds->min_hash < key_hash ?
                                                          appender_bounds->min_hash : key_hash;
        appender_bounds->max_hash                       = appender_bounds->max_hash > key_hash ?
                                                          appender_bounds->max_hash : key_hash;
        ng5_bloomfilter_set(appender_filter, &key_hash, sizeof(hash_t));
        if (unlikely(appender->num_elems++ == SLICE_KEY_COLUMN_MAX_ELEMS)) {
            appender_seal(appender);
            appender_new(list);
        }

next_pair:
        continue;
    }

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
    assert(slice);
    assert(needle_str);

    bool     end_not_reached;
    bool     hash_not_matched;
    bool     key_matched;

    uint32_t i                = 0;

    do {
        for (hash_not_matched = true; hash_not_matched && i < slice->num_elems; i++) {
            hash_not_matched  = (slice->key_hash_column[i] != needle_hash);
        }
        end_not_reached       = (i < slice->num_elems);
        key_matched           = !end_not_reached && strcmp(slice->key_column[i], needle_str) == 0;
        end_not_reached      &= !key_matched;
    } while (end_not_reached);

    return key_matched ? i : slice->num_elems;
}

static void appender_new(ng5_slice_list_t* list)
{
    /* the slice itself */
    ng5_slice_t  slice       = {
        .find      = slice_find_scan_default,
        .strat     = SLICE_LOOKUP_SCAN,
        .num_elems = 0
    };

    uint32_t     num_slices         =     ng5_vector_len(&list->slices);
    ng5_vector_push(&list->slices, &slice, 1);
    size_t       max_elems_in_slice =     SLICE_KEY_COLUMN_MAX_ELEMS;

    assert(max_elems_in_slice > 0);

    /* the descriptor */
    ng5_slice_desc_t desc = {
        .num_reads_hit  = 0,
        .num_reads_all  = 0,
    };

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
         .min_hash        = (hash_t) -1,
         .max_hash        = (hash_t)  0
    };
    ng5_vector_push(&list->bounds, &bounds, 1);

    info(NG5_SLICE_LIST_TAG, "created new appender in slice list %p\n\t"
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
            ng5_bloomfilter_nbits(&filter),
            (pow(1 - exp(-(double)ng5_bloomfilter_nhashes() / ((double)ng5_bloomfilter_nbits(&filter) / (double)SLICE_KEY_COLUMN_MAX_ELEMS)), ng5_bloomfilter_nhashes(&filter))),
            sizeof(ng5_slice_t),
            (sizeof(ng5_slice_list_t) + list->slices.num_elems* (sizeof(ng5_slice_t) +sizeof(ng5_slice_desc_t) + (sizeof(uint32_t) * list->descriptors.num_elems) +sizeof(ng5_bloomfilter_t) + ng5_bloomfilter_nbits(&filter)/8 +sizeof(ng5_hash_bounds_t)))/1024.0/1024.0
            );

    /* register new slice as the current appender */
    list->appender_idx = num_slices;
}

static void appender_seal(ng5_slice_t* slice)
{
    unused(slice);
    // TODO: sealing means (radix) sort and then replace 'find' with bsearch or something. Not yet implemented: sealed slices are also search in a linear fashion
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