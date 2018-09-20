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

NG5_BEGIN_DECL

typedef struct ng5_slice_t ng5_slice_t;


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


#define SLICE_DATA_SIZE (NG5_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(slice_lookup_strat_e) - sizeof(ng5_slice_find_func_t) - sizeof(uint32_t))


/* A function that implements the particular search strategy applied to a slice */
typedef uint32_t (*ng5_slice_find_func_t)(ng5_slice_t *slice, hash_t needle_hash, const char *needle_str);

#define SLICE_KEY_COLUMN_MAX_ELEMS (SLICE_DATA_SIZE / 8 / 3) /* one array with elements of 64 bits each, 3 of them */

typedef enum slice_lookup_strat_e
{
    SLICE_LOOKUP_SCAN,
    SLICE_LOOKUP_BESEARCH,
} slice_lookup_strat_e;

typedef struct ng5_slice_desc_t ng5_slice_desc_t;

/* A slice is a fixed-size partition inside an array. It is optimized for a mixed' A slice size is chosen such that a
 * single slice will fit into the L3 CPU cache. A slice as it own is a small self-optimizing and self-containing data
 * container that performs a local optimization w.r.t. the data sorting order and the strategy used to find a particular
 * element. A series of slices makes an array but only in a logically sense. However, besides a slice-local
 * optimization, the slice list itself optimizes the the order of elements (i.e., slices) depending on the data access
 * that was observed. Since the slice list is scanned slice-by-slice, the list tries to reduce the expected number
 * of unnecessary scanned slices by re-ordering the list to increase the probability for a hit as soon as possible.
 * For this, slices that had a relative high search hit rate are moved to the front of the list. */
typedef struct ng5_slice_t
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
    const char *key_column[SLICE_KEY_COLUMN_MAX_ELEMS];
    hash_t key_hash_column[SLICE_KEY_COLUMN_MAX_ELEMS];
    StringId string_id_column[SLICE_KEY_COLUMN_MAX_ELEMS];

    /* The number of elements stored in 'key_colum', 'key_hash_column', and 'string_id_column' */
    uint32_t num_elems;

    uint32_t cache_idx;
} ng5_slice_t;

typedef struct ng5_hash_bounds_t
{
    /* Min and max value inside this slice. Used to skip the lookup in the per-slice bloomfilter during search */
    hash_t min_hash,
        max_hash;
} ng5_hash_bounds_t;

typedef struct ng5_slice_desc_t
{
    /* The number of reads to this slice including misses and hits. Along with 'num_reads_hit' used to determine
     * the order of this element w.r.t. to other elements in the list */
    size_t num_reads_all;

    /* The number of reads to this slice that lead to a search hit. See 'num_reads_all' for the purpose. */
    size_t num_reads_hit;

} ng5_slice_desc_t;

typedef struct ng5_slice_list_t
{
    Allocator alloc;
    ng5_spinlock_t spinlock;

    Vector ofType(ng5_slice_t) slices;
    Vector ofType(ng5_slice_desc_t) descriptors;
    Vector ofType(ng5_bloomfilter_t) filters;
    Vector ofType(ng5_hash_bounds_t) bounds;

    uint32_t appender_idx;
} ng5_slice_list_t;

typedef struct ng5_slice_handle_t
{
    ng5_slice_t *container;
    const char *key;
    StringId value;
    bool is_contained;
} ng5_slice_handle_t;

int ng5_slice_list_create(ng5_slice_list_t *list, const Allocator *alloc, size_t slice_cap);

int ng5_slice_list_drop(ng5_slice_list_t *list);

int ng5_slice_list_lookup_by_key(ng5_slice_handle_t *handle, ng5_slice_list_t *list, const char *needle);

int ng5_slice_list_is_empty(const ng5_slice_list_t *list);

int ng5_slice_list_insert(ng5_slice_list_t *list, char **strings, StringId *ids, size_t npairs);

int ng5_slice_list_remove(ng5_slice_list_t *list, ng5_slice_handle_t *handle);

int ng5_slice_list_lock(ng5_slice_list_t *list);

int ng5_slice_list_unlock(ng5_slice_list_t *list);

NG5_END_DECL

#endif