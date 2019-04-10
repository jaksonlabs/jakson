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

#include <assert.h>
#include <inttypes.h>
#include <sys/mman.h>

#include "core/mem/memfile.h"
#include "stdx/vec.h"

#define DEFINE_PRINTER_FUNCTION_WCAST(type, castType, format_string)                                                   \
void vector_##type##_PrinterFunc(memfile_t *dst, void ofType(T) *values, size_t num_elems)                      \
{                                                                                                                      \
    char *data;                                                                                                        \
    type *typedValues = (type *) values;                                                                               \
                                                                                                                       \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                              \
    int nchars = sprintf(data, "[");                                                                                   \
    carbon_memfile_skip(dst, nchars);                                                                                  \
    for (size_t i = 0; i < num_elems; i++) {                                                                           \
        data = carbon_memfile_current_pos(dst, sizeof(type));                                                          \
        nchars = sprintf(data, format_string"%s", (castType) typedValues[i], i + 1 < num_elems ? ", " : "");           \
        carbon_memfile_skip(dst, nchars);                                                                              \
    }                                                                                                                  \
    data = carbon_memfile_current_pos(dst, sizeof(char));                                                              \
    nchars = sprintf(data, "]");                                                                                       \
    carbon_memfile_skip(dst, nchars);                                                                                  \
}

#define DEFINE_PRINTER_FUNCTION(type, format_string)                                                                   \
    DEFINE_PRINTER_FUNCTION_WCAST(type, type, format_string)

DEFINE_PRINTER_FUNCTION_WCAST(u_char, i8, "%d")
DEFINE_PRINTER_FUNCTION(i8, "%d")
DEFINE_PRINTER_FUNCTION(i16, "%d")
DEFINE_PRINTER_FUNCTION(i32, "%d")
DEFINE_PRINTER_FUNCTION(i64, "%"PRIi64)
DEFINE_PRINTER_FUNCTION(u8, "%d")
DEFINE_PRINTER_FUNCTION(u16, "%d")
DEFINE_PRINTER_FUNCTION(u32, "%d")
DEFINE_PRINTER_FUNCTION(u64, "%"PRIu64)
DEFINE_PRINTER_FUNCTION(size_t, "%zu")

bool carbon_vec_create(vec_t *out, const struct allocator *alloc, size_t elem_size, size_t cap_elems)
{
    CARBON_NON_NULL_OR_ERROR(out)
    out->allocator = malloc(sizeof(struct allocator));
    carbon_alloc_this_or_std(out->allocator, alloc);
    out->base = carbon_malloc(out->allocator, cap_elems * elem_size);
    out->num_elems = 0;
    out->cap_elems = cap_elems;
    out->elem_size = elem_size;
    out->grow_factor = 1.7f;
    carbon_error_init(&out->err);
    return true;
}

typedef struct
{
    char marker;
    u32 elem_size;
    u32 num_elems;
    u32 cap_elems;
    float grow_factor;

} vec_serialize_header_t;

CARBON_EXPORT(bool)
carbon_vec_serialize(FILE *file, vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(vec)

    vec_serialize_header_t header = {
        .marker = MARKER_SYMBOL_VECTOR_HEADER,
        .elem_size = vec->elem_size,
        .num_elems = vec->num_elems,
        .cap_elems = vec->cap_elems,
        .grow_factor = vec->grow_factor
    };
    int nwrite = fwrite(&header, sizeof(vec_serialize_header_t), 1, file);
    error_IF(nwrite != 1, &vec->err, CARBON_ERR_FWRITE_FAILED);
    nwrite = fwrite(vec->base, vec->elem_size, vec->num_elems, file);
    error_IF(nwrite != (int) vec->num_elems, &vec->err, CARBON_ERR_FWRITE_FAILED);

    return true;
}

CARBON_EXPORT(bool)
carbon_vec_deserialize(vec_t *vec, struct err *err, FILE *file)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(err)
    CARBON_NON_NULL_OR_ERROR(vec)

    offset_t start = ftell(file);
    int err_code = CARBON_ERR_NOERR;

    vec_serialize_header_t header;
    if (fread(&header, sizeof(vec_serialize_header_t), 1, file) != 1) {
        err_code = CARBON_ERR_FREAD_FAILED;
        goto error_handling;
    }

    if (header.marker != MARKER_SYMBOL_VECTOR_HEADER) {
        err_code = CARBON_ERR_CORRUPTED;
        goto error_handling;
    }

    vec->allocator = malloc(sizeof(struct allocator));
    carbon_alloc_this_or_std(vec->allocator, NULL);
    vec->base = carbon_malloc(vec->allocator, header.cap_elems * header.elem_size);
    vec->num_elems = header.num_elems;
    vec->cap_elems = header.cap_elems;
    vec->elem_size = header.elem_size;
    vec->grow_factor = header.grow_factor;
    carbon_error_init(&vec->err);

    if (fread(vec->base, header.elem_size, vec->num_elems, file) != vec->num_elems) {
        err_code = CARBON_ERR_FREAD_FAILED;
        goto error_handling;
    }

    return true;

error_handling:
    fseek(file, start, SEEK_SET);
    error(err, err_code);
    return false;
}

bool carbon_vec_memadvice(vec_t *vec, int madviseAdvice)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_UNUSED(vec);
    CARBON_UNUSED(madviseAdvice);
    madvise(vec->base, vec->cap_elems * vec->elem_size, madviseAdvice);
    return true;
}

bool carbon_vec_set_grow_factor(vec_t *vec, float factor)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_PRINT_ERROR_IF(factor <= 1.01f, CARBON_ERR_ILLEGALARG)
    vec->grow_factor = factor;
    return true;
}

bool carbon_vec_drop(vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    carbon_free(vec->allocator, vec->base);
    free(vec->allocator);
    vec->base = NULL;
    return true;
}

bool carbon_vec_is_empty(const vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->num_elems == 0 ? true : false;
}

bool carbon_vec_push(vec_t *vec, const void *data, size_t num_elems)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t next_num = vec->num_elems + num_elems;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
    vec->num_elems += num_elems;
    return true;
}

const void *carbon_vec_peek(vec_t *vec)
{
    if (!vec) {
        return NULL;
    } else {
        return (vec->num_elems > 0) ? carbon_vec_at(vec, vec->num_elems - 1) : NULL;
    }
}

bool carbon_vec_repeated_push(vec_t *vec, const void *data, size_t how_often)
{
    CARBON_NON_NULL_OR_ERROR(vec && data)
    size_t next_num = vec->num_elems + how_often;
    while (next_num > vec->cap_elems) {
        size_t more = next_num - vec->cap_elems;
        vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    for (size_t i = 0; i < how_often; i++) {
        memcpy(vec->base + (vec->num_elems + i) * vec->elem_size, data, vec->elem_size);
    }

    vec->num_elems += how_often;
    return true;
}

const void *carbon_vec_pop(vec_t *vec)
{
    void *result;
    if (CARBON_LIKELY((result = (vec ? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elem_size : NULL) : NULL))
                   != NULL)) {
        vec->num_elems--;
    }
    return result;
}

bool carbon_vec_clear(vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    vec->num_elems = 0;
    return true;
}

bool VectorShrink(vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    if (vec->num_elems < vec->cap_elems) {
        vec->cap_elems = CARBON_MAX(1, vec->num_elems);
        vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    }
    return true;
}

bool carbon_vec_grow(size_t *numNewSlots, vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    size_t freeSlotsBefore = vec->cap_elems - vec->num_elems;

    vec->cap_elems = (vec->cap_elems * vec->grow_factor) + 1;
    vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    size_t freeSlotsAfter = vec->cap_elems - vec->num_elems;
    if (CARBON_LIKELY(numNewSlots != NULL)) {
        *numNewSlots = freeSlotsAfter - freeSlotsBefore;
    }
    return true;
}

CARBON_EXPORT(bool)
carbon_vec_grow_to(vec_t *vec, size_t capacity)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    vec->cap_elems = CARBON_MAX(vec->cap_elems, capacity);
    vec->base = carbon_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
    return true;
}

size_t carbon_vec_length(const vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->num_elems;
}

const void *carbon_vec_at(const vec_t *vec, size_t pos)
{
    return (vec && pos < vec->num_elems) ? vec->base + pos * vec->elem_size : NULL;
}

size_t carbon_vec_capacity(const vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    return vec->cap_elems;
}

bool carbon_vec_enlarge_size_to_capacity(vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    vec->num_elems = vec->cap_elems;
    return true;
}

CARBON_EXPORT(bool)
carbon_vec_zero_memory(vec_t *vec)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    CARBON_ZERO_MEMORY(vec->base, vec->elem_size * vec->num_elems);
    return true;
}

CARBON_EXPORT(bool)
carbon_vec_zero_memory_in_range(vec_t *vec, size_t from, size_t to)
{
    CARBON_NON_NULL_OR_ERROR(vec);
    assert(from < to);
    assert(to <= vec->cap_elems);
    CARBON_ZERO_MEMORY(vec->base + from * vec->elem_size, vec->elem_size * (to -from));
    return true;
}

bool carbon_vec_set(vec_t *vec, size_t pos, const void *data)
{
    CARBON_NON_NULL_OR_ERROR(vec)
    assert(pos < vec->num_elems);
    memcpy(vec->base + pos * vec->elem_size, data, vec->elem_size);
    return true;
}

bool carbon_vec_cpy(vec_t *dst, const vec_t *src)
{
    CARBON_CHECK_SUCCESS(carbon_vec_create(dst, NULL, src->elem_size, src->num_elems));
    dst->num_elems = src->num_elems;
    if (dst->num_elems > 0) {
        memcpy(dst->base, src->base, src->elem_size * src->num_elems);
    }
    return true;
}

CARBON_EXPORT(bool)
carbon_vec_cpy_to(vec_t *dst, vec_t *src)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    void *handle = realloc(dst->base, src->cap_elems * src->elem_size);
    if (handle) {
        dst->elem_size = src->elem_size;
        dst->num_elems = src->num_elems;
        dst->cap_elems = src->cap_elems;
        dst->grow_factor = src->grow_factor;
        dst->base = handle;
        memcpy(dst->base, src->base, src->cap_elems * src->elem_size);
        carbon_error_cpy(&dst->err, &src->err);
        return true;
    } else {
        error(&src->err, CARBON_ERR_HARDCOPYFAILED)
        return false;
    }
}

const void *carbon_vec_data(const vec_t *vec)
{
    return vec ? vec->base : NULL;
}

char *vec_to_string(const vec_t ofType(T) *vec,
                     void (*printerFunc)(memfile_t *dst, void ofType(T) *values, size_t num_elems))
{
    carbon_memblock_t *block;
    memfile_t file;
    carbon_memblock_create(&block, vec->num_elems * vec->elem_size);
    carbon_memfile_open(&file, block, CARBON_MEMFILE_MODE_READWRITE);
    printerFunc(&file, vec->base, vec->num_elems);
    return carbon_memblock_move_contents_and_drop(block);
}
