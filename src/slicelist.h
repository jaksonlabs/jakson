// file: slicelist.h

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

#ifndef NG5_SLICELIST
#define NG5_SLICELIST

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------

#include "common.h"
#include "vector.h"
#include "bitmap.h"
#include "spinlock.h"
#include "bloomfilter.h"
#include "hash.h"
#include "simd.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E   F O R W A R D I N G
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct Slice Slice;

// ---------------------------------------------------------------------------------------------------------------------
//
//  C O N F I G
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME
#define NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME "1 of 100 in CPU L1"
#endif
#ifndef NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE
#define NG5_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE (32768/100)
#endif

#ifndef NG5_SLICE_LIST_TARGET_MEMORY_NAME
#define NG5_SLICE_LIST_TARGET_MEMORY_NAME "10 of 100 in CPU L1"
#endif
#ifndef NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE
#define NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE (32768/10)
#endif

#define SLICE_DATA_SIZE (NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(slice_lookup_strat_e) - sizeof(uint32_t))

#define SLICE_KEY_COLUMN_MAX_ELEMS 300 // (SLICE_DATA_SIZE / 8 / 3) /* one array with elements of 64 bits each, 3 of them */

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef enum slice_lookup_strat_e
{
    SLICE_LOOKUP_SCAN,
    SLICE_LOOKUP_BESEARCH,
} slice_lookup_strat_e;

typedef struct SliceDescriptor SliceDescriptor;

/* A slice is a fixed-size partition inside an array. It is optimized for a mixed' A slice size is chosen such that a
 * single slice will fit into the L3 CPU cache. A slice as it own is a small self-optimizing and self-containing data
 * container that performs a local optimization w.r.t. the data sorting order and the strategy used to find a particular
 * element. A series of slices makes an array but only in a logically sense. However, besides a slice-local
 * optimization, the slice list itself optimizes the the order of elements (i.e., slices) depending on the data access
 * that was observed. Since the slice list is scanned slice-by-slice, the list tries to reduce the expected number
 * of unnecessary scanned slices by re-ordering the list to increase the probability for a hit as soon as possible.
 * For this, slices that had a relative high search hit rate are moved to the front of the list. */
typedef struct Slice
{
    /* Enumeration to determine which strategy for 'find' is currently applied */
    slice_lookup_strat_e strat;

    /* Data stored inside this slice. By setting 'NG5_SLICE_LIST_CPU_L3_SIZE_IN_BYTE' statically to the target
     * CPU L3 size, it is intended that one entire 'ng5_slice_t' structure fits into the L3 cache of the CPU.
     * It is assumed that at least one element can be inserted into a 'ng5_slice_t' object (which means that
     * the type of elements to be inserted must be less or equal to SLICE_DATA_SIZE. In case an element is
     * removed from this list, data is physically moved to avoid a "sparse" list, i.e., it is alwalys
     * guaranteeed that 'data' contains continously elements without any gabs until 'num_elems' limit. This
     * avoids to lookup in a bitmap or other structure whether a particular element is removed or not; also
     * this does not steal an element from the domain of the used data type to encode 'not present' with a
     * particular value. However, a remove operation is expensive. */
    const char *keyColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
    Hash keyHashColumn[SLICE_KEY_COLUMN_MAX_ELEMS];
    StringId stringIdColumn[SLICE_KEY_COLUMN_MAX_ELEMS];

    /* The number of elements stored in 'key_colum', 'key_hash_column', and 'string_id_column' */
    uint32_t numElems;

    uint32_t cacheIdx;
} Slice;

typedef struct ng5_hash_bounds_t
{
    /* Min and max value inside this slice. Used to skip the lookup in the per-slice bloomfilter during search */
    Hash minHash,
         maxHash;
} HashBounds;

typedef struct SliceDescriptor
{
    /* The number of reads to this slice including misses and hits. Along with 'num_reads_hit' used to determine
     * the order of this element w.r.t. to other elements in the list */
    size_t numReadsAll;

    /* The number of reads to this slice that lead to a search hit. See 'num_reads_all' for the purpose. */
    size_t numReadsHit;

} SliceDescriptor;

typedef struct ng5_slice_list_t
{
    Allocator alloc;
    Spinlock lock;

    Vector ofType(ng5_slice_t) slices;
    Vector ofType(ng5_slice_desc_t) descriptors;
    Vector ofType(ng5_bloomfilter_t) filters;
    Vector ofType(ng5_hash_bounds_t) bounds;

    uint32_t appenderIdx;
} SliceList;

typedef struct ng5_slice_handle_t
{
    Slice *container;
    const char *key;
    StringId value;
    bool isContained;
} SliceHandle;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

int SliceListCreate(SliceList *list, const Allocator *alloc, size_t sliceCapacity);

int SliceListDrop(SliceList *list);

int SliceListLookupByKey(SliceHandle *handle, SliceList *list, const char *needle);

int SliceListIsEmpty(const SliceList *list);

int SliceListInsert(SliceList *list, char **strings, StringId *ids, size_t npairs);

int SliceListRemove(SliceList *list, SliceHandle *handle);

NG5_END_DECL

#endif