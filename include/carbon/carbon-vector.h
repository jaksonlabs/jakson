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

#ifndef CARBON_VECTOR_H
#define CARBON_VECTOR_H

#include <sys/mman.h>

#include "carbon-common.h"
#include "carbon-alloc.h"

CARBON_BEGIN_DECL

typedef struct carbon_memfile carbon_memfile_t;

#define DECLARE_PRINTER_FUNC(type)                                                                                     \
    void vector_##type##_PrinterFunc(carbon_memfile_t *dst, void ofType(T) *values, size_t num_elems);

DECLARE_PRINTER_FUNC(u_char)
DECLARE_PRINTER_FUNC(int8_t)
DECLARE_PRINTER_FUNC(int16_t)
DECLARE_PRINTER_FUNC(int32_t)
DECLARE_PRINTER_FUNC(int64_t)
DECLARE_PRINTER_FUNC(uint8_t)
DECLARE_PRINTER_FUNC(uint16_t)
DECLARE_PRINTER_FUNC(uint32_t)
DECLARE_PRINTER_FUNC(uint64_t)
DECLARE_PRINTER_FUNC(size_t)

#define VECTOR_PRINT_UCHAR  vector_u_char_PrinterFunc
#define VECTOR_PRINT_UINT8  vector_uint8_t_PrinterFunc
#define VECTOR_PRINT_UINT16 vector_uint16_t_PrinterFunc
#define VECTOR_PRINT_UINT32 vector_uint32_t_PrinterFunc
#define VECTOR_PRINT_UINT64 vector_uint64_t_PrinterFunc
#define VECTOR_PRINT_INT8   vector_int8_t_PrinterFunc
#define VECTOR_PRINT_INT16  vector_int16_t_PrinterFunc
#define VECTOR_PRINT_INT32  vector_int32_t_PrinterFunc
#define VECTOR_PRINT_INT64  vector_int64_t_PrinterFunc
#define VECTOR_PRINT_SIZE_T vector_size_t_PrinterFunc

/**
 * An implementation of the concrete data type Vector, a resizeable dynamic array.
 */
typedef struct carbon_vec
{
    /**
    *  Memory allocator that is used to get memory for user data
    */
    carbon_alloc_t *allocator;

    /**
     *  Fixed number of bytes for a single element that should be stored in the vector
     */
    size_t elemSize;

    /**
     *  The number of elements currently stored in the vector
     */
    uint32_t num_elems;

    /**
     *  The number of elements for which currently memory is reserved
     */
    uint32_t cap_elems;

    /**
    * The grow factor considered for resize operations
    */
    float growFactor;

    /**
     * A pointer to a memory address managed by 'allocator' that contains the user data
     */
    void *base;

    /**
     *  Error information
     */
    carbon_err_t err;
} carbon_vec_t;

/**
 * Utility implementation of generic vector to specialize for type of 'char *'
 */
typedef carbon_vec_t ofType(char *) StringVector;
typedef carbon_vec_t ofType(const char *) carbon_string_ref_vec;

#define STRING_VECTOR_CREATE(vec, alloc, cap_elems)                                                                     \
    carbon_vec_create(vec, alloc, sizeof(char *), cap_elems);

#define STRING_REF_VECTOR_CREATE(vec, alloc, cap_elems)                                                                 \
    STRING_VECTOR_CREATE(vec, alloc, cap_elems)                                                                         \

#define STRING_VECTOR_DROP(vec)                                                                                        \
({                                                                                                                     \
    for (size_t i = 0; i < vec->num_elems; i++) {                                                                       \
        char *s = *CARBON_VECTOR_GET(vec, i, char *);                                                                         \
        free (s);                                                                                                      \
    }                                                                                                                  \
    carbon_vec_drop(vec);                                                                                                   \
})

#define STRING_REF_VECTOR_DROP(vec)                                                                                    \
    carbon_vec_drop(vec);

#define STRING_VECTOR_PUSH(vec, string)                                                                                \
({                                                                                                                     \
    char *cpy = strdup(string);                                                                                        \
    carbon_vec_push(vec, &cpy, 1);                                                                                          \
})

#define STRING_REF_VECTOR_PUSH(vec, string)                                                                            \
    carbon_vec_push(vec, &string, 1)

/**
 * Constructs a new vector for elements of size 'elem_size', reserving memory for 'cap_elems' elements using
 * the allocator 'alloc'.
 *
 * @param out non-null vector that should be constructed
 * @param alloc an allocator
 * @param elemSize fixed-length element size
 * @param cap_elems number of elements for which memory should be reserved
 * @return STATUS_OK if success, and STATUS_NULLPTR in case of NULL pointer parameters
 */
CARBON_EXPORT(bool)
carbon_vec_create(carbon_vec_t *out, const carbon_alloc_t *alloc, size_t elemSize, size_t cap_elems);


/**
 * Provides hints on the OS kernel how to deal with memory inside this vector.
 *
 * @param vec non-null vector
 * @param madviseAdvice value to give underlying <code>madvise</code> syscall and advice, see man page
 * of <code>madvise</code>
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
CARBON_EXPORT(bool)
carbon_vec_memadvice(carbon_vec_t *vec, int madviseAdvice);

/**
 * Sets the factor for determining the reallocation size in case of a resizing operation.
 *
 * Note that <code>factor</code> must be larger than one.
 *
 * @param vec non-null vector for which the grow factor should be changed
 * @param factor a positive real number larger than 1
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
CARBON_EXPORT(bool)
VectorSetGrowFactor(carbon_vec_t *vec, float factor);

/**
 * Frees up memory requested via the allocator.
 *
 * Depending on the allocator implementation, dropping the reserved memory might not take immediately effect.
 * The pointer 'vec' itself gets not freed.
 *
 * @param vec vector to be freed
 * @return STATUS_OK if success, and STATUS_NULL_PTR in case of NULL pointer to 'vec'
 */
CARBON_EXPORT(bool)
carbon_vec_drop(carbon_vec_t *vec);

/**
 * Returns information on whether elements are stored in this vector or not.
 * @param vec non-null pointer to the vector
 * @return Returns <code>STATUS_TRUE</code> if <code>vec</code> is empty. Otherwise <code>STATUS_FALSE</code> unless
 *         an error occurs. In case an error is occured, the return value is neither <code>STATUS_TRUE</code> nor
 *         <code>STATUS_FALSE</code> but an value indicating that error.
 */
CARBON_EXPORT(bool)
carbon_vec_is_empty(const carbon_vec_t *vec);

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
CARBON_EXPORT(bool)
carbon_vec_push(carbon_vec_t *vec,
               const void *data,
               size_t num_elems);

CARBON_EXPORT(const void *)
VectorPeek(carbon_vec_t *vec);

#define VECTOR_PEEK(vec, type) (type *)(VectorPeek(vec))

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
CARBON_EXPORT(bool)
carbon_vec_repeated_push(carbon_vec_t *vec, const void *data, size_t howOften);

/**
 * Returns a pointer to the last element in this vector, or <code>NULL</code> is the vector is already empty.
 * The number of elements contained in that vector is decreased, too.
 *
 * @param vec non-null pointer to the vector
 * @return Pointer to last element, or <code>NULL</code> if vector is empty
 */
CARBON_EXPORT(const void *)
carbon_vec_pop(carbon_vec_t *vec);

CARBON_EXPORT(bool)
carbon_vec_clear(carbon_vec_t *vec);

/**
 * Shinks the vector's internal data block to fits its real size, i.e., remove reserved memory
 *
 * @param vec
 * @return
 */
CARBON_EXPORT(bool)
VectorShrink(carbon_vec_t *vec);

/**
 * Increases the capacity of that vector according the internal grow factor
 * @param numNewSlots a pointer to a value that will store the number of newly created slots in that vector if
 *                      <code>num_new_slots</code> is non-null. If this parameter is <code>NULL</code>, it is ignored.
 * @param vec non-null pointer to the vector that should be grown
 * @return STATUS_OK in case of success, and another value indicating an error otherwise.
 */
CARBON_EXPORT(bool)
carbon_vec_grow(size_t *numNewSlots, carbon_vec_t *vec);

/**
 * Returns the number of elements currently stored in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
CARBON_EXPORT(size_t)
carbon_vec_length(const carbon_vec_t *vec);

#define CARBON_VECTOR_GET(vec, pos, type) (type *) VectorAt(vec, pos)

#define VECTOR_NEW_AND_GET(vec, type)                                                                                  \
({                                                                                                                     \
    type template;                                                                                                     \
    size_t vectorLength = carbon_vec_length(vec);                                                                           \
    carbon_vec_push(vec, &template, 1);                                                                                     \
    CARBON_VECTOR_GET(vec, vectorLength, type);                                                                               \
})

CARBON_EXPORT(const void *)
VectorAt(const carbon_vec_t *vec, size_t pos);

/**
 * Returns the number of elements for which memory is currently reserved in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
CARBON_EXPORT(size_t)
VectorCapacity(const carbon_vec_t *vec);

/**
 * Set the internal size of <code>vec</code> to its capacity.
 */
CARBON_EXPORT(bool)
carbon_vec_enlarge_size_to_capacity(carbon_vec_t *vec);

CARBON_EXPORT(bool)
carbon_vec_set(carbon_vec_t *vec, size_t pos, const void *data);

CARBON_EXPORT(bool)
carbon_vec_cpy(carbon_vec_t *dst, const carbon_vec_t *src);

/**
 * Gives raw data access to data stored in the vector; do not manipulate this data since otherwise the vector
 * might get corrupted.
 *
 * @param vec the vector for which the operation is started
 * @return pointer to user-data managed by this vector
 */
CARBON_EXPORT(const void *)
carbon_vec_data(const carbon_vec_t *vec);

CARBON_EXPORT(char *)
VectorToString(const carbon_vec_t ofType(T) *vec,
        void (*printerFunc)(carbon_memfile_t *dst, void ofType(T) *values, size_t num_elems));

#define CARBON_VECTOR_ALL(vec, type) (type *) carbon_vec_data(vec)

CARBON_END_DECL

#endif
