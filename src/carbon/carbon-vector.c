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

#include <sys/mman.h>

#include "carbon/carbon-memfile.h"
#include "carbon/carbon-vector.h"

#define DEFINE_PRINTER_FUNCTION_WCAST(type, castType, formatString)                                                    \
void vector_##type##_PrinterFunc(carbon_memfile_t *dst, void ofType(T) *values, size_t numElems)                         \
{                                                                                                                      \
    char *data;                                                                                                        \
    type *typedValues = (type *) values;                                                                               \
                                                                                                                       \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                                  \
    int nchars = sprintf(data, "[");                                                                                   \
    carbon_memfile_skip(dst, nchars);                                                                                          \
    for (size_t i = 0; i < numElems; i++) {                                                                            \
        data = carbon_memfile_current_pos(dst, sizeof(type));                                                              \
        nchars = sprintf(data, formatString"%s", (castType) typedValues[i], i + 1 < numElems ? ", " : "");             \
        carbon_memfile_skip(dst, nchars);                                                                                      \
    }                                                                                                                  \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                                  \
    nchars = sprintf(data, "]");                                                                                       \
    carbon_memfile_skip(dst, nchars);                                                                                          \
}

#define DEFINE_PRINTER_FUNCTION(type, formatString)                                                                    \
    DEFINE_PRINTER_FUNCTION_WCAST(type, type, formatString)

DEFINE_PRINTER_FUNCTION_WCAST(u_char, int8_t, "%d")
DEFINE_PRINTER_FUNCTION(int8_t, "%d")
DEFINE_PRINTER_FUNCTION(int16_t, "%d")
DEFINE_PRINTER_FUNCTION(int32_t, "%d")
DEFINE_PRINTER_FUNCTION(int64_t, "%"PRIi64)
DEFINE_PRINTER_FUNCTION(uint8_t, "%d")
DEFINE_PRINTER_FUNCTION(uint16_t, "%d")
DEFINE_PRINTER_FUNCTION(uint32_t, "%d")
DEFINE_PRINTER_FUNCTION(uint64_t, "%"PRIu64)
DEFINE_PRINTER_FUNCTION(size_t, "%zu")

bool VectorCreate(carbon_vec_t *out, const carbon_alloc_t *alloc, size_t elemSize, size_t capElems)
{
    CARBON_NON_NULL_OR_ERROR(out)
    out->allocator = malloc(sizeof(carbon_alloc_t));
    carbon_alloc_this_or_std(out->allocator, alloc);
    out->base = carbon_malloc(out->allocator, capElems * elemSize);
    out->numElems = 0;
    out->capElems = capElems;
    out->elemSize = elemSize;
    out->growFactor = 1.7f;
    carbon_error_init(&out->err);
    return true;
}

bool VectorMemoryAdvice(carbon_vec_t *vec, int madviseAdvice)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_UNUSED(vec);
    CARBON_UNUSED(madviseAdvice);
    madvise(vec->base, vec->capElems * vec->elemSize, madviseAdvice);
    return true;
}

bool VectorSetGrowFactor(carbon_vec_t *vec, float factor)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_PRINT_ERROR_IF(factor <= 1.01f, CARBON_ERR_ILLEGALARG)
    vec->growFactor = factor;
    return true;
}

bool VectorDrop(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    carbon_free(vec->allocator, vec->base);
    free(vec->allocator);
    vec->base = NULL;
    return true;
}

bool VectorIsEmpty(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->numElems == 0 ? true : false;
}

bool VectorPush(carbon_vec_t *vec, const void *data, size_t numElems)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t nextNum = vec->numElems + numElems;
    while (nextNum > vec->capElems) {
        size_t more = nextNum - vec->capElems;
        vec->capElems = (vec->capElems + more) * vec->growFactor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    }
    memcpy(vec->base + vec->numElems * vec->elemSize, data, numElems * vec->elemSize);
    vec->numElems += numElems;
    return true;
}

const void *VectorPeek(carbon_vec_t *vec)
{
    if (!vec) {
        return NULL;
    } else {
        return (vec->numElems > 0) ? VectorAt(vec, vec->numElems - 1) : NULL;
    }
}

bool VectorRepreatedPush(carbon_vec_t *vec, const void *data, size_t howOften)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t nextNum = vec->numElems + howOften;
    while (nextNum > vec->capElems) {
        size_t more = nextNum - vec->capElems;
        vec->capElems = (vec->capElems + more) * vec->growFactor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    }
    for (size_t i = 0; i < howOften; i++) {
        memcpy(vec->base + (vec->numElems + i) * vec->elemSize, data, vec->elemSize);
    }

    vec->numElems += howOften;
    return true;
}

const void *VectorPop(carbon_vec_t *vec)
{
    void *result;
    if (CARBON_BRANCH_LIKELY((result = (vec ? (vec->numElems > 0 ? vec->base + (vec->numElems - 1) * vec->elemSize : NULL) : NULL))
                   != NULL)) {
        vec->numElems--;
    }
    return result;
}

bool VectorClear(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    vec->numElems = 0;
    return true;
}

bool VectorShrink(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    if (vec->numElems < vec->capElems) {
        vec->capElems = vec->numElems;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    }
    return true;
}

bool VectorGrow(size_t *numNewSlots, carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    size_t freeSlotsBefore = vec->capElems - vec->numElems;

    vec->capElems = (vec->capElems * vec->growFactor) + 1;
    vec->base = carbon_realloc(vec->allocator, vec->base, vec->capElems * vec->elemSize);
    size_t freeSlotsAfter = vec->capElems - vec->numElems;
    if (CARBON_BRANCH_LIKELY(numNewSlots != NULL)) {
        *numNewSlots = freeSlotsAfter - freeSlotsBefore;
    }
    return true;
}

size_t VectorLength(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->numElems;
}

const void *VectorAt(const carbon_vec_t *vec, size_t pos)
{
    return (vec && pos < vec->numElems) ? vec->base + pos * vec->elemSize : NULL;
}

size_t VectorCapacity(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->capElems;
}

bool VectorEnlargeSizeToCapacity(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    vec->numElems = vec->capElems;
    return true;
}

bool VectorSet(carbon_vec_t *vec, size_t pos, const void *data)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    assert(pos < vec->numElems);
    memcpy(vec->base + pos * vec->elemSize, data, vec->elemSize);
    return true;
}

bool VectorCpy(carbon_vec_t *dst, const carbon_vec_t *src)
{
    CARBON_CHECK_SUCCESS(VectorCreate(dst, NULL, src->elemSize, src->numElems));
    dst->numElems = src->numElems;
    if (dst->numElems > 0) {
        memcpy(dst->base, src->base, src->elemSize * src->numElems);
    }
    return true;
}

const void *VectorData(const carbon_vec_t *vec)
{
    return vec ? vec->base : NULL;
}

char *VectorToString(const carbon_vec_t ofType(T) *vec,
                     void (*printerFunc)(carbon_memfile_t *dst, void ofType(T) *values, size_t numElems))
{
    carbon_memblock_t *block;
    carbon_memfile_t file;
    carbon_memblock_create(&block, vec->numElems * vec->elemSize);
    carbon_memfile_open(&file, block, CARBON_MEMFILE_MODE_READWRITE);
    printerFunc(&file, vec->base, vec->numElems);
    return carbon_memblock_move_contents_and_drop(block);
}
