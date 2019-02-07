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

#ifndef CARBON_SLICELIST_H
#define CARBON_SLICELIST_H

#include "carbon-common.h"
#include "carbon-vector.h"
#include "carbon-bitmap.h"
#include "carbon-spinlock.h"
#include "carbon-bloom.h"
#include "carbon-hash.h"
#include "carbon-types.h"

CARBON_BEGIN_DECL

CARBON_FORWARD_STRUCT_DECL(Slice)

#ifndef CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME
#define CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME "1 of 100 in CPU L1"
#endif
#ifndef CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE
#define CARBON_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE (32768/100)
#endif

#ifndef CARBON_SLICE_LIST_TARGET_MEMORY_NAME
#define CARBON_SLICE_LIST_TARGET_MEMORY_NAME "10 of 100 in CPU L1"
#endif
#ifndef CARBON_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE
#define CARBON_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE (32768/10)
#endif

#define SLICE_DATA_SIZE (CARBON_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(slice_lookup_strat_e) - sizeof(uint32_t))

#define SLICE_KEY_COLUMN_MAX_ELEMS (SLICE_DATA_SIZE / 8 / 3) /** one array with elements of 64 bits each, 3 of them */

typedef enum slice_lookup_strat_e
{
    SLICE_LOOKUP_SCAN,
    SLICE_LOOKUP_BESEARCH,
} slice_lookup_strat_e;

typedef struct SliceDescriptor SliceDescriptor;

typedef struct Slice
{
    /** Enumeration to determine which strategy for 'find' is currently applied */
    slice_lookup_strat_e strat;

    /** Data stored inside this slice. By setting 'CARBON_SLICE_LIST_CPU_L3_SIZE_IN_BYTE' statically to the target
     * CPU L3 size, it is intended that one entire 'CARBON_slice_t' structure fits into the L3 cache of the CPU.
     * It is assumed that at least one element can be inserted into a 'CARBON_slice_t' object (which means that
     * the type of elements to be inserted must be less or equal to SLICE_DATA_SIZE. In case an element is
     * removed from this list, data is physically moved to avoid a "sparse" list, i.e., it is alwalys
     * guaranteeed that 'data' contains continously elements without any gabs until 'num_elems' limit. This
     * avoids to lookup in a carbon_bitmap_t or other structure whether a particular element is removed or not; also
     * this does not steal an element from the domain of the used data type to encode 'not present' with a
     * particular values. However, a remove operation is expensive. */
    const char *key_column[SLICE_KEY_COLUMN_MAX_ELEMS];
    carbon_hash_t keyHashColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
    carbon_string_id_t carbon_string_id_tColumn[SLICE_KEY_COLUMN_MAX_ELEMS];

    /** The number of elements stored in 'key_colum', 'key_hash_column', and 'string_id_column' */
    uint32_t num_elems;

    uint32_t cacheIdx;
} Slice;

typedef struct CARBON_hash_bounds_t
{
    /** Min and max values inside this slice. Used to skip the lookup in the per-slice carbon_bloom_t during search */
    carbon_hash_t minHash,
         maxHash;
} HashBounds;

typedef struct SliceDescriptor
{
    /** The number of reads to this slice including misses and hits. Along with 'num_reads_hit' used to determine
     * the order of this element w.r.t. to other elements in the list */
    size_t numReadsAll;

    /** The number of reads to this slice that lead to a search hit. See 'num_reads_all' for the purpose. */
    size_t numReadsHit;

} SliceDescriptor;

typedef struct CARBON_slice_list_t
{
    carbon_alloc_t alloc;
    carbon_spinlock_t lock;

    carbon_vec_t ofType(CARBON_slice_t) slices;
    carbon_vec_t ofType(CARBON_slice_desc_t) descriptors;
    carbon_vec_t ofType(CARBON_bloomfilter_t) filters;
    carbon_vec_t ofType(CARBON_hash_bounds_t) bounds;

    uint32_t appenderIdx;

    carbon_err_t err;
} carbon_slice_list_t;

typedef struct CARBON_slice_handle_t
{
    Slice *container;
    const char *key;
    carbon_string_id_t value;
    bool is_contained;
} slice_handle_t;

CARBON_EXPORT(bool)
carbon_slice_list_create(carbon_slice_list_t *list, const carbon_alloc_t *alloc, size_t sliceCapacity);

CARBON_EXPORT(bool)
SliceListDrop(carbon_slice_list_t *list);

CARBON_EXPORT(bool)
carbon_slice_list_lookup(slice_handle_t *handle, carbon_slice_list_t *list, const char *needle);

CARBON_EXPORT(bool)
SliceListIsEmpty(const carbon_slice_list_t *list);

CARBON_EXPORT(bool)
carbon_slice_list_insert(carbon_slice_list_t *list, char **strings, carbon_string_id_t *ids, size_t npairs);

CARBON_EXPORT(bool)
SliceListRemove(carbon_slice_list_t *list, slice_handle_t *handle);

CARBON_END_DECL

#endif