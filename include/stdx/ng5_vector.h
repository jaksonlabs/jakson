// file: vector.h

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

#ifndef _NG5_VECTOR
#define _NG5_VECTOR

#include <sys/mman.h>

#include <ng5_common.h>
#include <stdx/ng5_allocator.h>

/**
 * An implementation of the concrete data type Vector, a resizeable dynamic array.
 */
typedef struct ng5_vector_t
{
    /**
    *  Memory allocator that is used to get memory for user data
    */
    ng5_allocator_t     allocator;

    /**
     *  Fixed number of bytes for a single element that should be stored in the vector
     */
    size_t              elem_size;

    /**
     *  The number of elements currently stored in the vector
     */
    size_t              num_elems;

    /**
     *  The number of elements for which currently memory is reserved
     */
    size_t              cap_elems;

    /**
    * The grow factor considered for resize operations
    */
    float               grow_factor;

    /**
     *  A pointer to a memory address managed by 'allocator' that contains the user data
     */
    void               *base;
} ng5_vector_t;

/**
 * Constructs a new vector for elements of size 'elem_size', reserving memory for 'cap_elems' elements using
 * the allocator 'alloc'.
 *
 * @param out non-null vector that should be constructed
 * @param alloc an allocator
 * @param elem_size fixed-length element size
 * @param cap_elems number of elements for which memory should be reserved
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
int ng5_vector_create(ng5_vector_t* out, const ng5_allocator_t* alloc, size_t elem_size, size_t cap_elems);

/**
 * Provides hints on the OS kernel how to deal with memory inside this vector.
 *
 * @param vec non-null vector
 * @param madvise_advice value to give underlying <code>madvise</code> syscall and advice, see man page
 * of <code>madvise</code>
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
int ng5_vector_advise(ng5_vector_t* vec, int madvise_advice);

/**
 * Sets the factor for determining the reallocation size in case of a resizing operation.
 *
 * Note that <code>factor</code> must be larger than one.
 *
 * @param vec non-null vector for which the grow factor should be changed
 * @param factor a positive real number larger than 1
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
int ng5_vector_set_growfactor(ng5_vector_t* vec, float factor);

/**
 * Frees up memory requested via the allocator.
 *
 * Depending on the allocator implementation, dropping the reserved memory might not take immediately effect.
 * The pointer 'vec' itself gets not freed.
 *
 * @param vec vector to be freed
 * @return STATUS_OK if success, and STATUS_NULL_PTR in case of NULL pointer to 'vec'
 */
int ng5_vector_drop(ng5_vector_t* vec);

/**
 * Returns information on whether elements are stored in this vector or not.
 * @param vec non-null pointer to the vector
 * @return Returns <code>STATUS_TRUE</code> if <code>vec</code> is empty. Otherwise <code>STATUS_FALSE</code> unless
 *         an error occurs. In case an error is occured, the return value is neither <code>STATUS_TRUE</code> nor
 *         <code>STATUS_FALSE</code> but an value indicating that error.
 */
int ng5_vector_is_empty(ng5_vector_t* vec);

/**
 * Appends 'num_elems' elements stored in 'data' into the vector by copying num_elems * vec->elem_size into the
 * vectors memory block.
 *
 * In case the capacity is not sufficient, the vector gets automatically resized.
 *
 * @param vec the vector in which the data should be pushed
 * @param data non-null pointer to data that should be appended. Must be at least size of 'num_elems' * vec->elem_size.
 * @param num_elems number of elements stored in data
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
int ng5_vector_push(ng5_vector_t* vec, const void* data, size_t num_elems) force_inline;    /* OPTIMIZATION: Force inline */

void *ng5_vector_push_and_get(ng5_vector_t* vec, const void* data, size_t num_elems) force_inline; /* OPTIMIZATION: Force inline */

/**
 * Appends 'how_many' elements of the same source stored in 'data' into the vector by copying how_many * vec->elem_size
 * into the vectors memory block.
 *
 * In case the capacity is not sufficient, the vector gets automatically resized.
 *
 * @param vec the vector in which the data should be pushed
 * @param data non-null pointer to data that should be appended. Must be at least size of one vec->elem_size.
 * @param num_elems number of elements stored in data
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
int ng5_vector_repreat_push(ng5_vector_t* vec, const void* data, size_t how_many);

/**
 * Returns a pointer to the last element in this vector, or <code>NULL</code> is the vector is already empty.
 * The number of elements contained in that vector is decreased, too.
 *
 * @param vec non-null pointer to the vector
 * @return Pointer to last element, or <code>NULL</code> if vector is empty
 */
const void *ng5_vector_pop(ng5_vector_t* vec) force_inline; /* OPTIMIZATION: Force inline */

/**
 * Increases the capacity of that vector according the internal grow factor
 * @param num_new_slots a pointer to a value that will store the number of newly created slots in that vector if
 *                      <code>num_new_slots</code> is non-null. If this parameter is <code>NULL</code>, it is ignored.
 * @param vec non-null pointer to the vector that should be grown
 * @return STATUS_OK in case of success, and another value indicating an error otherwise.
 */
int ng5_vector_grow(size_t* num_new_slots, ng5_vector_t* vec);

/**
 * Returns the number of elements currently stored in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t ng5_vector_len(const ng5_vector_t* vec);

#define ng5_vector_get(vec, pos, type) (type *) ng5_vector_at(vec, pos);

const void *ng5_vector_at(const ng5_vector_t* vec, size_t pos) force_inline; /* OPTIMIZATION: Force inline */

/**
 * Returns the number of elements for which memory is currently reserved in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t ng5_vector_cap(const ng5_vector_t* vec);

/**
 * Set the internal size of <code>vec</code> to its capacity.
 */
int ng5_vector_enlarge_size(ng5_vector_t* vec);

int ng5_vector_set(ng5_vector_t* vec, size_t pos, const void* data);

/**
 * Gives raw data access to data stored in the vector; do not manipulate this data since otherwise the vector
 * might get corrupted.
 *
 * @param vec the vector for which the operation is started
 * @return pointer to user-data managed by this vector
 */
const void *ng5_vector_data(const ng5_vector_t* vec);

#define ng5_vector_all(vec, type) (type *) ng5_vector_data(vec)

#endif