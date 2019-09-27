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

#ifndef JAK_SLICELIST_H
#define JAK_SLICELIST_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/jak_vector.h>
#include <jakson/std/jak_bitmap.h>
#include <jakson/std/jak_spinlock.h>
#include <jakson/std/jak_bloom.h>
#include <jakson/std/jak_hash.h>
#include <jakson/types.h>

JAK_BEGIN_DECL

JAK_FORWARD_STRUCT_DECL(jak_slice)

#ifndef JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME
#define JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_NAME "1 of 100 in CPU L1"
#endif
#ifndef JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE
#define JAK_SLICE_LIST_BLOOMFILTER_TARGET_MEMORY_SIZE_IN_BYTE (32768/100)
#endif

#ifndef JAK_SLICE_LIST_TARGET_MEMORY_NAME
#define JAK_SLICE_LIST_TARGET_MEMORY_NAME "10 of 100 in CPU L1"
#endif
#ifndef JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE
#define JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE (32768/10)
#endif

#define SLICE_DATA_SIZE (JAK_SLICE_LIST_TARGET_MEMORY_SIZE_IN_BYTE - sizeof(jak_slice_lookup_strat_e) - sizeof(jak_u32))

#define SLICE_KEY_COLUMN_MAX_ELEMS (SLICE_DATA_SIZE / 8 / 3) /** one array with elements of 64 bits each, 3 of them */

typedef enum jak_slice_lookup_strat_e {
        JAK_SLICE_LOOKUP_SCAN, JAK_SLICE_LOOKUP_BESEARCH,
} jak_slice_lookup_strat_e;

typedef struct jak_slice {
        /** Enumeration to determine which strategy for 'find' is currently applied */
        jak_slice_lookup_strat_e strat;

        /** Data stored inside this slice. By setting 'JAK_SLICE_LIST_CPU_L3_SIZE_IN_BYTE' statically to the target
         * CPU L3 size, it is intended that one entire 'JAK_slice_t' structure fits into the L3 cache of the CPU.
         * It is assumed that at least one element can be inserted into a 'JAK_slice_t' object (which means that
         * the type of elements to be inserted must be less or equal to SLICE_DATA_SIZE. In case an element is
         * removed from this list, data is physically moved to avoid a "sparse" list, i.e., it is alwalys
         * guaranteeed that 'data' contains continously elements without any gabs until 'num_elems' limit. This
         * avoids to lookup in a struct bitmap or other structure whether a particular element is removed or not; also
         * this does not steal an element from the domain of the used data type to encode 'not present' with a
         * particular values. However, a remove operation is expensive. */
        const char *key_column[SLICE_KEY_COLUMN_MAX_ELEMS];
        hash32_t key_hash_column[SLICE_KEY_COLUMN_MAX_ELEMS];
        jak_archive_field_sid_t jak_string_id_column[SLICE_KEY_COLUMN_MAX_ELEMS];

        /** The number of elements stored in 'key_colum', 'key_hash_column', and 'jak_string_id_column' */
        jak_u32 num_elems;

        jak_u32 cache_idx;
} jak_slice;

typedef struct jak_hash_bounds {
        /** Min and max values inside this slice. Used to skip the lookup in the per-slice jak_bitmap during search */
        hash32_t min_hash, max_hash;
} jak_hash_bounds;

typedef struct jak_slice_descriptor {
        /** The number of reads to this slice including misses and hits. Along with 'num_reads_hit' used to determine
         * the order of this element w.r.t. to other elements in the list */
        size_t num_reads_all;

        /** The number of reads to this slice that lead to a search hit. See 'num_reads_all' for the purpose. */
        size_t num_reads_hit;

} jak_slice_descriptor;

typedef struct jak_slice_list {
        jak_allocator alloc;
        jak_spinlock lock;

        jak_vector ofType(jak_slice) slices;
        jak_vector ofType(jak_slice_descriptor) descriptors;
        jak_vector ofType(jak_bloomfilter) filters;
        jak_vector ofType(jak_hash_bounds) bounds;

        jak_u32 appender_idx;
        jak_error err;
} jak_slice_list_t;

typedef struct jak_slice_handle {
        jak_slice *container;
        const char *key;
        jak_archive_field_sid_t value;
        bool is_contained;
} jak_slice_handle;

bool jak_slice_list_create(jak_slice_list_t *list, const jak_allocator *alloc, size_t slice_capacity);
bool jak_slice_list_drop(jak_slice_list_t *list);
bool jak_slice_list_lookup(jak_slice_handle *handle, jak_slice_list_t *list, const char *needle);
bool jak_slice_list_is_empty(const jak_slice_list_t *list);
bool jak_slice_list_insert(jak_slice_list_t *list, char **strings, jak_archive_field_sid_t *ids, size_t npairs);
bool jak_slice_list_remove(jak_slice_list_t *list, jak_slice_handle *handle);

JAK_END_DECL

#endif