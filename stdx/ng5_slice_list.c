#include <stdx/ng5_slice_list.h>

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R
//
// ---------------------------------------------------------------------------------------------------------------------

static void slice_create(ng5_slice_t * restrict slice, size_t elem_size, ng5_allocator_t *alloc);
static void slice_drop(ng5_slice_t * restrict slice, ng5_allocator_t *alloc);

static void append(ng5_slice_list_t *list, ng5_slice_t *slice);

uint32_t slice_find_scan_default(ng5_slice_t *slice, const void *needle);

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

int ng5_slice_list_create(ng5_slice_list_t *list, const ng5_allocator_t *alloc, size_t elem_size)
{
    check_non_null(list)
    check_non_null(alloc)
    check_non_null(elem_size)
    check_non_null(slice_cap)

    allocator_this_or_default(&list->alloc, alloc);
    NG5_HEAP_ALLOC(ng5_slice_t, slice, 1, &list->alloc);
    slice_create(slice, elem_size, &list->alloc);
    ng5_spinlock_create(&list->spinlock);
    ng5_bitset_create(&list->slice_freelist, 1);

    list->slice_elem_size   = elem_size;
    list->slice_elem_size   = elem_size;
    list->head              =
      list->tail            =
      list->appender        = slice;

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

int ng5_slice_list_insert(ng5_slice_list_t *list, const void *elems, size_t neleme)
{
    unused(list);
    unused(elems);
    unused(neleme);
    NOT_YET_IMPLEMENTED
}

// ---------------------------------------------------------------------------------------------------------------------
//
//  H E L P E R   I M P L E M E N T A T I O N
//
// ---------------------------------------------------------------------------------------------------------------------

static void slice_create(ng5_slice_t * restrict slice, size_t elem_size, ng5_allocator_t *alloc)
{
    assert(slice);

//    size_t num_elems    = SLICE_DATA_SIZE / elem_size;
//    slice->next         = NULL;
//    slice->find         = slice_find_scan_default;
//
//    panic_if(num_elems == 0, "single element will not fit into CPU cache: abort");
//
//    ng5_bitset_create(&slice->in_use_flags, num_elems);
//    slice->freelist     = allocator_malloc(alloc, num_elems * sizeof(uint32_t));
//    slice->num_freelist = num_elems;
//    while (num_elems--) {
//        slice->freelist[num_elems] = num_elems;
//    }
//
//    slice->num_reads_all = slice->num_reads_hit = 0;

    unused(slice);
    unused(elem_size);
    unused(alloc);

}

unused_fn static void slice_drop(ng5_slice_t * restrict slice, ng5_allocator_t *alloc)
{
    unused(slice);
    unused(alloc);
}

unused_fn static void append(ng5_slice_list_t *list, ng5_slice_t *slice)
{
    unused(slice);
    unused(list);
}

uint32_t slice_find_scan_default(ng5_slice_t *slice, const void *needle)
{
    unused(slice);
    unused(needle);
    return 0;
}