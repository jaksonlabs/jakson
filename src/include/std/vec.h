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

#ifndef NG5_VECTOR_H
#define NG5_VECTOR_H

#include <sys/mman.h>

#include "shared/common.h"
#include "core/alloc/alloc.h"

NG5_BEGIN_DECL

typedef struct carbon_memfile memfile_t;

#define DECLARE_PRINTER_FUNC(type)                                                                                     \
    void vector_##type##_PrinterFunc(memfile_t *dst, void ofType(T) *values, size_t num_elems);

DECLARE_PRINTER_FUNC(u_char)
DECLARE_PRINTER_FUNC(i8)
DECLARE_PRINTER_FUNC(i16)
DECLARE_PRINTER_FUNC(i32)
DECLARE_PRINTER_FUNC(i64)
DECLARE_PRINTER_FUNC(u8)
DECLARE_PRINTER_FUNC(u16)
DECLARE_PRINTER_FUNC(u32)
DECLARE_PRINTER_FUNC(u64)
DECLARE_PRINTER_FUNC(size_t)

#define VECTOR_PRINT_UCHAR  vector_u_char_PrinterFunc
#define VECTOR_PRINT_UINT8  vector_u8_PrinterFunc
#define VECTOR_PRINT_UINT16 vector_u16_PrinterFunc
#define VECTOR_PRINT_UINT32 vector_u32_PrinterFunc
#define VECTOR_PRINT_UINT64 vector_u64_PrinterFunc
#define VECTOR_PRINT_INT8   vector_i8_PrinterFunc
#define VECTOR_PRINT_INT16  vector_i16_PrinterFunc
#define VECTOR_PRINT_INT32  vector_i32_PrinterFunc
#define VECTOR_PRINT_INT64  vector_i64_PrinterFunc
#define VECTOR_PRINT_SIZE_T vector_size_t_PrinterFunc

/**
 * An implementation of the concrete data type Vector, a resizeable dynamic array.
 */
typedef struct carbon_vec
{
    /**
    *  Memory allocator that is used to get memory for user data
    */
    struct allocator *allocator;

    /**
     *  Fixed number of bytes for a single element that should be stored in the vector
     */
    size_t elem_size;

    /**
     *  The number of elements currently stored in the vector
     */
    u32 num_elems;

    /**
     *  The number of elements for which currently memory is reserved
     */
    u32 cap_elems;

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
    struct err err;
} vec_t;

/**
 * Utility implementation of generic vector to specialize for type of 'char *'
 */
typedef vec_t ofType(char *) StringVector;
typedef vec_t ofType(const char *) carbon_string_ref_vec;

#define STRING_VECTOR_CREATE(vec, alloc, cap_elems)                                                                     \
    carbon_vec_create(vec, alloc, sizeof(char *), cap_elems);

#define STRING_REF_VECTOR_CREATE(vec, alloc, cap_elems)                                                                 \
    STRING_VECTOR_CREATE(vec, alloc, cap_elems)                                                                         \

#define STRING_VECTOR_DROP(vec)                                                                                        \
({                                                                                                                     \
    for (size_t i = 0; i < vec->num_elems; i++) {                                                                      \
        char *s = *NG5_VECTOR_GET(vec, i, char *);                                                                  \
        free (s);                                                                                                      \
    }                                                                                                                  \
    carbon_vec_drop(vec);                                                                                              \
})

#define STRING_REF_VECTOR_DROP(vec)                                                                                    \
    carbon_vec_drop(vec);

#define STRING_VECTOR_PUSH(vec, string)                                                                                \
({                                                                                                                     \
    char *cpy = strdup(string);                                                                                        \
    carbon_vec_push(vec, &cpy, 1);                                                                                     \
})

#define STRING_REF_VECTOR_PUSH(vec, string)                                                                            \
    carbon_vec_push(vec, &string, 1)

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
NG5_EXPORT(bool)
carbon_vec_create(vec_t *out, const struct allocator *alloc, size_t elem_size, size_t cap_elems);

NG5_EXPORT(bool)
carbon_vec_serialize(FILE *file, vec_t *vec);

NG5_EXPORT(bool)
carbon_vec_deserialize(vec_t *vec, struct err *err, FILE *file);

/**
 * Provides hints on the OS kernel how to deal with memory inside this vector.
 *
 * @param vec non-null vector
 * @param madviseAdvice value to give underlying <code>madvise</code> syscall and advice, see man page
 * of <code>madvise</code>
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
NG5_EXPORT(bool)
carbon_vec_memadvice(vec_t *vec, int madviseAdvice);

/**
 * Sets the factor for determining the reallocation size in case of a resizing operation.
 *
 * Note that <code>factor</code> must be larger than one.
 *
 * @param vec non-null vector for which the grow factor should be changed
 * @param factor a positive real number larger than 1
 * @return STATUS_OK if success, otherwise a value indicating the error
 */
NG5_EXPORT(bool)
carbon_vec_set_grow_factor(vec_t *vec, float factor);

/**
 * Frees up memory requested via the allocator.
 *
 * Depending on the allocator implementation, dropping the reserved memory might not take immediately effect.
 * The pointer 'vec' itself gets not freed.
 *
 * @param vec vector to be freed
 * @return STATUS_OK if success, and STATUS_NULL_PTR in case of NULL pointer to 'vec'
 */
NG5_EXPORT(bool)
carbon_vec_drop(vec_t *vec);

/**
 * Returns information on whether elements are stored in this vector or not.
 * @param vec non-null pointer to the vector
 * @return Returns <code>STATUS_TRUE</code> if <code>vec</code> is empty. Otherwise <code>STATUS_FALSE</code> unless
 *         an error occurs. In case an error is occured, the return value is neither <code>STATUS_TRUE</code> nor
 *         <code>STATUS_FALSE</code> but an value indicating that error.
 */
NG5_EXPORT(bool)
carbon_vec_is_empty(const vec_t *vec);

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
NG5_EXPORT(bool)
carbon_vec_push(vec_t *vec,
               const void *data,
               size_t num_elems);

NG5_EXPORT(const void *)
carbon_vec_peek(vec_t *vec);

#define VECTOR_PEEK(vec, type) (type *)(carbon_vec_peek(vec))

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
NG5_EXPORT(bool)
carbon_vec_repeated_push(vec_t *vec, const void *data, size_t how_often);

/**
 * Returns a pointer to the last element in this vector, or <code>NULL</code> is the vector is already empty.
 * The number of elements contained in that vector is decreased, too.
 *
 * @param vec non-null pointer to the vector
 * @return Pointer to last element, or <code>NULL</code> if vector is empty
 */
NG5_EXPORT(const void *)
carbon_vec_pop(vec_t *vec);

NG5_EXPORT(bool)
carbon_vec_clear(vec_t *vec);

/**
 * Shinks the vector's internal data block to fits its real size, i.e., remove reserved memory
 *
 * @param vec
 * @return
 */
NG5_EXPORT(bool)
VectorShrink(vec_t *vec);

/**
 * Increases the capacity of that vector according the internal grow factor
 * @param numNewSlots a pointer to a value that will store the number of newly created slots in that vector if
 *                      <code>num_new_slots</code> is non-null. If this parameter is <code>NULL</code>, it is ignored.
 * @param vec non-null pointer to the vector that should be grown
 * @return STATUS_OK in case of success, and another value indicating an error otherwise.
 */
NG5_EXPORT(bool)
carbon_vec_grow(size_t *numNewSlots, vec_t *vec);

NG5_EXPORT(bool)
carbon_vec_grow_to(vec_t *vec, size_t capacity);

/**
 * Returns the number of elements currently stored in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
NG5_EXPORT(size_t)
carbon_vec_length(const vec_t *vec);

#define vec_get(vec, pos, type) (type *) carbon_vec_at(vec, pos)

#define VECTOR_NEW_AND_GET(vec, type)                                                                                  \
({                                                                                                                     \
    type template;                                                                                                     \
    size_t vectorLength = carbon_vec_length(vec);                                                                      \
    carbon_vec_push(vec, &template, 1);                                                                                \
    vec_get(vec, vectorLength, type);                                                                        \
})

NG5_EXPORT(const void *)
carbon_vec_at(const vec_t *vec, size_t pos);

/**
 * Returns the number of elements for which memory is currently reserved in the vector
 *
 * @param vec the vector for which the operation is started
 * @return 0 in case of NULL pointer to 'vec', or the number of elements otherwise.
 */
NG5_EXPORT(size_t)
carbon_vec_capacity(const vec_t *vec);

/**
 * Set the internal size of <code>vec</code> to its capacity.
 */
NG5_EXPORT(bool)
carbon_vec_enlarge_size_to_capacity(vec_t *vec);

NG5_EXPORT(bool)
carbon_vec_zero_memory(vec_t *vec);

NG5_EXPORT(bool)
carbon_vec_zero_memory_in_range(vec_t *vec, size_t from, size_t to);

NG5_EXPORT(bool)
carbon_vec_set(vec_t *vec, size_t pos, const void *data);

NG5_EXPORT(bool)
carbon_vec_cpy(vec_t *dst, const vec_t *src);

NG5_EXPORT(bool)
carbon_vec_cpy_to(vec_t *dst, vec_t *src);

/**
 * Gives raw data access to data stored in the vector; do not manipulate this data since otherwise the vector
 * might get corrupted.
 *
 * @param vec the vector for which the operation is started
 * @return pointer to user-data managed by this vector
 */
NG5_EXPORT(const void *)
carbon_vec_data(const vec_t *vec);

NG5_EXPORT(char *)
vec_to_string(const vec_t ofType(T) *vec,
        void (*printerFunc)(memfile_t *dst, void ofType(T) *values, size_t num_elems));

#define vec_all(vec, type) (type *) carbon_vec_data(vec)

NG5_END_DECL

#endif
