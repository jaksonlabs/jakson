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

#define DEFINE_PRINTER_FUNCTION_WCAST(type, castType, format_string)                                                    \
void vector_##type##_PrinterFunc(carbon_memfile_t *dst, void ofType(T) *values, size_t num_elems)                         \
{                                                                                                                      \
    char *data;                                                                                                        \
    type *typedValues = (type *) values;                                                                               \
                                                                                                                       \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                                  \
    int nchars = sprintf(data, "[");                                                                                   \
    carbon_memfile_skip(dst, nchars);                                                                                          \
    for (size_t i = 0; i < num_elems; i++) {                                                                            \
        data = carbon_memfile_current_pos(dst, sizeof(type));                                                              \
        nchars = sprintf(data, format_string"%s", (castType) typedValues[i], i + 1 < num_elems ? ", " : "");             \
        carbon_memfile_skip(dst, nchars);                                                                                      \
    }                                                                                                                  \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                                  \
    nchars = sprintf(data, "]");                                                                                       \
    carbon_memfile_skip(dst, nchars);                                                                                          \
}

#define DEFINE_PRINTER_FUNCTION(type, format_string)                                                                    \
    DEFINE_PRINTER_FUNCTION_WCAST(type, type, format_string)

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

bool carbon_vec_create(carbon_vec_t *out, const carbon_alloc_t *alloc, size_t elemSize, size_t cap_elems)
{
    CARBON_NON_NULL_OR_ERROR(out)
    out->allocator = malloc(sizeof(carbon_alloc_t));
    carbon_alloc_this_or_std(out->allocator, alloc);
    out->base = carbon_malloc(out->allocator, cap_elems * elemSize);
    out->num_elems = 0;
    out->cap_elems = cap_elems;
    out->elemSize = elemSize;
    out->growFactor = 1.7f;
    carbon_error_init(&out->err);
    return true;
}

bool carbon_vec_memadvice(carbon_vec_t *vec, int madviseAdvice)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_UNUSED(vec);
    CARBON_UNUSED(madviseAdvice);
    madvise(vec->base, vec->cap_elems * vec->elemSize, madviseAdvice);
    return true;
}

bool VectorSetGrowFactor(carbon_vec_t *vec, float factor)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_PRINT_ERROR_IF(factor <= 1.01f, CARBON_ERR_ILLEGALARG)
    vec->growFactor = factor;
    return true;
}

bool carbon_vec_drop(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    carbon_free(vec->allocator, vec->base);
    free(vec->allocator);
    vec->base = NULL;
    return true;
}

bool carbon_vec_is_empty(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->num_elems == 0 ? true : false;
}

bool carbon_vec_push(carbon_vec_t *vec, const void *data, size_t num_elems)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t nextNum = vec->num_elems + num_elems;
    while (nextNum > vec->cap_elems) {
        size_t more = nextNum - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->growFactor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elemSize);
    }
    memcpy(vec->base + vec->num_elems * vec->elemSize, data, num_elems * vec->elemSize);
    vec->num_elems += num_elems;
    return true;
}

const void *VectorPeek(carbon_vec_t *vec)
{
    if (!vec) {
        return NULL;
    } else {
        return (vec->num_elems > 0) ? VectorAt(vec, vec->num_elems - 1) : NULL;
    }
}

bool carbon_vec_repeated_push(carbon_vec_t *vec, const void *data, size_t howOften)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t nextNum = vec->num_elems + howOften;
    while (nextNum > vec->cap_elems) {
        size_t more = nextNum - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->growFactor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elemSize);
    }
    for (size_t i = 0; i < howOften; i++) {
        memcpy(vec->base + (vec->num_elems + i) * vec->elemSize, data, vec->elemSize);
    }

    vec->num_elems += howOften;
    return true;
}

const void *carbon_vec_pop(carbon_vec_t *vec)
{
    void *result;
    if (CARBON_BRANCH_LIKELY((result = (vec ? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elemSize : NULL) : NULL))
                   != NULL)) {
        vec->num_elems--;
    }
    return result;
}

bool carbon_vec_clear(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    vec->num_elems = 0;
    return true;
}

bool VectorShrink(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    if (vec->num_elems < vec->cap_elems) {
        vec->cap_elems = vec->num_elems;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elemSize);
    }
    return true;
}

bool carbon_vec_grow(size_t *numNewSlots, carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    size_t freeSlotsBefore = vec->cap_elems - vec->num_elems;

    vec->cap_elems = (vec->cap_elems * vec->growFactor) + 1;
    vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elemSize);
    size_t freeSlotsAfter = vec->cap_elems - vec->num_elems;
    if (CARBON_BRANCH_LIKELY(numNewSlots != NULL)) {
        *numNewSlots = freeSlotsAfter - freeSlotsBefore;
    }
    return true;
}

size_t carbon_vec_length(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->num_elems;
}

const void *VectorAt(const carbon_vec_t *vec, size_t pos)
{
    return (vec && pos < vec->num_elems) ? vec->base + pos * vec->elemSize : NULL;
}

size_t VectorCapacity(const carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->cap_elems;
}

bool VectorEnlargeSizeToCapacity(carbon_vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    vec->num_elems = vec->cap_elems;
    return true;
}

bool VectorSet(carbon_vec_t *vec, size_t pos, const void *data)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    assert(pos < vec->num_elems);
    memcpy(vec->base + pos * vec->elemSize, data, vec->elemSize);
    return true;
}

bool VectorCpy(carbon_vec_t *dst, const carbon_vec_t *src)
{
    CARBON_CHECK_SUCCESS(carbon_vec_create(dst, NULL, src->elemSize, src->num_elems));
    dst->num_elems = src->num_elems;
    if (dst->num_elems > 0) {
        memcpy(dst->base, src->base, src->elemSize * src->num_elems);
    }
    return true;
}

const void *carbon_vec_data(const carbon_vec_t *vec)
{
    return vec ? vec->base : NULL;
}

char *VectorToString(const carbon_vec_t ofType(T) *vec,
                     void (*printerFunc)(carbon_memfile_t *dst, void ofType(T) *values, size_t num_elems))
{
    carbon_memblock_t *block;
    carbon_memfile_t file;
    carbon_memblock_create(&block, vec->num_elems * vec->elemSize);
    carbon_memfile_open(&file, block, CARBON_MEMFILE_MODE_READWRITE);
    printerFunc(&file, vec->base, vec->num_elems);
    return carbon_memblock_move_contents_and_drop(block);
}
