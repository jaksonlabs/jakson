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

#ifndef JAK_VECTOR_H
#define JAK_VECTOR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <sys/mman.h>

#include <jakson/stdinc.h>
#include <jakson/stdx/jak_alloc.h>
#include <jakson/memfile/jak_memfile.h>

JAK_BEGIN_DECL

#define DECLARE_PRINTER_FUNC(type)                                                                                     \
    void jak_vector_##type##_printer_func(jak_memfile *dst, void ofType(T) *values, size_t num_elems);

DECLARE_PRINTER_FUNC(u_char)

DECLARE_PRINTER_FUNC(jak_i8)

DECLARE_PRINTER_FUNC(jak_i16)

DECLARE_PRINTER_FUNC(jak_i32)

DECLARE_PRINTER_FUNC(jak_i64)

DECLARE_PRINTER_FUNC(jak_u8)

DECLARE_PRINTER_FUNC(jak_u16)

DECLARE_PRINTER_FUNC(jak_u32)

DECLARE_PRINTER_FUNC(jak_u64)

DECLARE_PRINTER_FUNC(size_t)

#define VECTOR_PRINT_UCHAR  jak_vector_u_char_printer_func
#define VECTOR_PRINT_UINT8  jak_vector_u8_printer_func
#define VECTOR_PRINT_UINT16 jak_vector_u16_printer_func
#define VECTOR_PRINT_UINT32 jak_vector_u32_printer_func
#define VECTOR_PRINT_UINT64 jak_vector_u64_printer_func
#define VECTOR_PRINT_INT8   jak_vector_i8_printer_func
#define VECTOR_PRINT_INT16  jak_vector_i16_printer_func
#define VECTOR_PRINT_INT32  jak_vector_i32_printer_func
#define VECTOR_PRINT_INT64  jak_vector_i64_printer_func
#define VECTOR_PRINT_SIZE_T jak_vector_size_t_printer_func

/**
 * An implementation of the concrete data type Vector, a resizeable dynamic array.
 */
typedef struct jak_vector {
        /**
        *  Memory allocator that is used to get memory for user data
        */
        jak_allocator *allocator;

        /**
         *  Fixed number of bytes for a single element that should be stored in the vector
         */
        size_t elem_size;

        /**
         *  The number of elements currently stored in the vector
         */
        jak_u32 num_elems;

        /**
         *  The number of elements for which currently memory is reserved
         */
        jak_u32 cap_elems;

        /**
        * The grow factor considered for resize operations
        */
        float grow_factor;

        /**
         * A pointer to a memory address managed by 'allocator' that contains the user data
         */
        void *base;

        /**
         *  Error information
         */
        jak_error err;
} jak_vector;

/**
 * Utility implementation of generic vector to specialize for type of 'char *'
 */
typedef jak_vector ofType(const char *) jak_string_jak_vector_t;

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
bool jak_vector_create(jak_vector *out, const jak_allocator *alloc, size_t elem_size, size_t cap_elems);

bool jak_vector_serialize(FILE *file, jak_vector *vec);

bool jak_vector_deserialize(jak_vector *vec, jak_error *err, FILE *file);

/**
 * Provides hints on the OS kernel how to deal with memory inside this vector.
 *
 * @param vec non-null vector
 * @param madviseAdvice value to give underlying <code>madvise</code> syscall and advice, see man page
 * of <code>madvise</code>
 * @return STATUS_OK if success, otherwise a value indicating the JAK_ERROR
 */
bool jak_vector_memadvice(jak_vector *vec, int madviseAdvice);

/**
 * Sets the factor for determining the reallocation size in case of a resizing operation.
 *
 * Note that <code>factor</code> must be larger than one.
 *
 * @param vec non-null vector for which the grow factor should be changed
 * @param factor a positive real number larger than 1
 * @return STATUS_OK if success, otherwise a value indicating the JAK_ERROR
 */
bool jak_vector_set_grow_factor(jak_vector *vec, float factor);

/**
 * Frees up memory requested via the allocator.
 *
 * Depending on the allocator implementation, dropping the reserved memory might not take immediately effect.
 * The pointer 'vec' itself gets not freed.
 *
 * @param vec vector to be freed
 * @return STATUS_OK if success, and STATUS_NULL_PTR in case of NULL pointer to 'vec'
 */
bool jak_vector_drop(jak_vector *vec);

/**
 * Returns information on whether elements are stored in this vector or not.
 * @param vec non-null pointer to the vector
 * @return Returns <code>STATUS_TRUE</code> if <code>vec</code> is empty. Otherwise <code>STATUS_FALSE</code> unless
 *         an JAK_ERROR occurs. In case an JAK_ERROR is occured, the return value is neither <code>STATUS_TRUE</code> nor
 *         <code>STATUS_FALSE</code> but an value indicating that JAK_ERROR.
 */
bool jak_vector_is_empty(const jak_vector *vec);

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
bool jak_vector_push(jak_vector *vec, const void *data, size_t num_elems);

const void *jak_vector_peek(jak_vector *vec);

#define JAK_VECTOR_PEEK(vec, type) (type *)(jak_vector_peek(vec))

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
bool jak_vector_repeated_push(jak_vector *vec, const void *data, size_t how_often);

/**
 * Returns a pointer to the last element in this vector, or <code>NULL</code> is the vector is already empty.
 * The number of elements contained in that vector is decreased, too.
 *
 * @param vec non-null pointer to the vector
 * @return Pointer to last element, or <code>NULL</code> if vector is empty
 */
const void *jak_vector_pop(jak_vector *vec);

bool jak_vector_clear(jak_vector *vec);

/**
 * Shinks the vector's internal data block to fits its real size, i.e., remove reserved memory
 *
 * @param vec
 * @return
 */
bool jak_vector_shrink(jak_vector *vec);

/**
 * Increases the capacity of that vector according the internal grow factor
 * @param numNewSlots a pointer to a value that will store the number of newly created slots in that vector if
 *                      <code>num_new_slots</code> is non-null. If this parameter is <code>NULL</code>, it is ignored.
 * @param vec non-null pointer to the vector that should be grown
 * @return STATUS_OK in case of success, and another value indicating an JAK_ERROR otherwise.
 */
bool jak_vector_grow(size_t *numNewSlots, jak_vector *vec);

bool jak_vector_grow_to(jak_vector *vec, size_t capacity);

/**
 * Returns the number of elements currently stored in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t jak_vector_length(const jak_vector *vec);

#define JAK_VECTOR_GET(vec, pos, type) (type *) jak_vector_at(vec, pos)

#define JAK_VECTOR_NEW_AND_GET(vec, type)                                                                              \
({                                                                                                                     \
    type obj;                                                                                                          \
    size_t vectorLength = jak_vector_length(vec);                                                                      \
    jak_vector_push(vec, &obj, 1);                                                                                     \
    JAK_VECTOR_GET(vec, vectorLength, type);                                                                           \
})

const void *jak_vector_at(const jak_vector *vec, size_t pos);

/**
 * Returns the number of elements for which memory is currently reserved in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
size_t jak_vector_capacity(const jak_vector *vec);

/**
 * Set the internal size of <code>vec</code> to its capacity.
 */
bool jak_vector_enlarge_size_to_capacity(jak_vector *vec);

bool jak_vector_zero_memory(jak_vector *vec);

bool jak_vector_zero_memory_in_range(jak_vector *vec, size_t from, size_t to);

bool jak_vector_set(jak_vector *vec, size_t pos, const void *data);

bool jak_vector_cpy(jak_vector *dst, const jak_vector *src);

bool jak_vector_cpy_to(jak_vector *dst, jak_vector *src);

/**
 * Gives raw data access to data stored in the vector; do not manipulate this data since otherwise the vector
 * might get corrupted.
 *
 * @param vec the vector for which the operation is started
 * @return pointer to user-data managed by this vector
 */
const void *jak_vector_data(const jak_vector *vec);

char *jak_vector_string(const jak_vector ofType(T) *vec,
                    void (*printerFunc)(jak_memfile *dst, void ofType(T) *values, size_t num_elems));

#define JAK_VECTOR_ALL(vec, type) (type *) jak_vector_data(vec)

JAK_END_DECL

#endif
