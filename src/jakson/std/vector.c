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

#include <inttypes.h>
#include <sys/mman.h>

#include <jakson/mem/file.h>
#include <jakson/std/vector.h>

#define DEFINE_PRINTER_FUNCTION_WCAST(type, castType, format_string)                                                   \
void vector_##type##_printer_func(memfile *dst, void ofType(T) *values, size_t num_elems)                      \
{                                                                                                                      \
    char *data;                                                                                                        \
    type *typedValues = (type *) values;                                                                               \
                                                                                                                       \
    data = memfile_current_pos(dst, sizeof(char));                                                              \
    int nchars = sprintf(data, "[");                                                                                   \
    memfile_skip(dst, nchars);                                                                                  \
    for (size_t i = 0; i < num_elems; i++) {                                                                           \
        data = memfile_current_pos(dst, sizeof(type));                                                          \
        nchars = sprintf(data, format_string"%s", (castType) typedValues[i], i + 1 < num_elems ? ", " : "");           \
        memfile_skip(dst, nchars);                                                                              \
    }                                                                                                                  \
    data = memfile_current_pos(dst, sizeof(char));                                                              \
    nchars = sprintf(data, "]");                                                                                       \
    memfile_skip(dst, nchars);                                                                                  \
}

#define DEFINE_PRINTER_FUNCTION(type, format_string)                                                                   \
    DEFINE_PRINTER_FUNCTION_WCAST(type, type, format_string)

DEFINE_PRINTER_FUNCTION_WCAST(u_char, i8, "%d")

DEFINE_PRINTER_FUNCTION(i8, "%d")

DEFINE_PRINTER_FUNCTION(i16, "%d")

DEFINE_PRINTER_FUNCTION(i32, "%d")

DEFINE_PRINTER_FUNCTION(i64, "%"
        PRIi64)

DEFINE_PRINTER_FUNCTION(u8, "%d")

DEFINE_PRINTER_FUNCTION(u16, "%d")

DEFINE_PRINTER_FUNCTION(u32, "%d")

DEFINE_PRINTER_FUNCTION(u64, "%"
        PRIu64)

DEFINE_PRINTER_FUNCTION(size_t, "%zu")

bool vector_create(vector *out, const allocator *alloc, size_t elem_size, size_t cap_elems)
{
        ERROR_IF_NULL(out)
        out->allocator = MALLOC(sizeof(allocator));
        alloc_this_or_std(out->allocator, alloc);
        out->base = alloc_malloc(out->allocator, cap_elems * elem_size);
        out->num_elems = 0;
        out->cap_elems = cap_elems;
        out->elem_size = elem_size;
        out->grow_factor = 1.7f;
        error_init(&out->err);
        return true;
}

typedef struct vector_serialize_header {
        char marker;
        u32 elem_size;
        u32 num_elems;
        u32 cap_elems;
        float grow_factor;
} vector_serialize_header;

bool vector_serialize(FILE *file, vector *vec)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(vec)

        vector_serialize_header header =
                {.marker = MARKER_SYMBOL_VECTOR_HEADER, .elem_size = vec->elem_size, .num_elems = vec
                        ->num_elems, .cap_elems = vec->cap_elems, .grow_factor = vec->grow_factor};
        int nwrite = fwrite(&header, sizeof(vector_serialize_header), 1, file);
        ERROR_IF(nwrite != 1, &vec->err, ERR_FWRITE_FAILED);
        nwrite = fwrite(vec->base, vec->elem_size, vec->num_elems, file);
        ERROR_IF(nwrite != (int) vec->num_elems, &vec->err, ERR_FWRITE_FAILED);

        return true;
}

bool vector_deserialize(vector *vec, err *err, FILE *file)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(err)
        ERROR_IF_NULL(vec)

        offset_t start = ftell(file);
        int err_code = ERR_NOERR;

        vector_serialize_header header;
        if (fread(&header, sizeof(vector_serialize_header), 1, file) != 1) {
                err_code = ERR_FREAD_FAILED;
                goto error_handling;
        }

        if (header.marker != MARKER_SYMBOL_VECTOR_HEADER) {
                err_code = ERR_CORRUPTED;
                goto error_handling;
        }

        vec->allocator = MALLOC(sizeof(allocator));
        alloc_this_or_std(vec->allocator, NULL);
        vec->base = alloc_malloc(vec->allocator, header.cap_elems * header.elem_size);
        vec->num_elems = header.num_elems;
        vec->cap_elems = header.cap_elems;
        vec->elem_size = header.elem_size;
        vec->grow_factor = header.grow_factor;
        error_init(&vec->err);

        if (fread(vec->base, header.elem_size, vec->num_elems, file) != vec->num_elems) {
                err_code = ERR_FREAD_FAILED;
                goto error_handling;
        }

        return true;

        error_handling:
        fseek(file, start, SEEK_SET);
        ERROR(err, err_code);
        return false;
}

bool vector_memadvice(vector *vec, int madviseAdvice)
{
        ERROR_IF_NULL(vec);
        UNUSED(vec);
        UNUSED(madviseAdvice);
        madvise(vec->base, vec->cap_elems * vec->elem_size, madviseAdvice);
        return true;
}

bool vector_set_grow_factor(vector *vec, float factor)
{
        ERROR_IF_NULL(vec);
        ERROR_PRINT_IF(factor <= 1.01f, ERR_ILLEGALARG)
        vec->grow_factor = factor;
        return true;
}

bool vector_drop(vector *vec)
{
        ERROR_IF_NULL(vec)
        alloc_free(vec->allocator, vec->base);
        free(vec->allocator);
        vec->base = NULL;
        return true;
}

bool vector_is_empty(const vector *vec)
{
        ERROR_IF_NULL(vec)
        return vec->num_elems == 0 ? true : false;
}

bool vector_push(vector *vec, const void *data, size_t num_elems)
{
        ERROR_IF_NULL(vec && data)
        size_t next_num = vec->num_elems + num_elems;
        while (next_num > vec->cap_elems) {
                size_t more = next_num - vec->cap_elems;
                vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
                vec->base = alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
        vec->num_elems += num_elems;
        return true;
}

const void *vector_peek(vector *vec)
{
        if (!vec) {
                return NULL;
        } else {
                return (vec->num_elems > 0) ? vector_at(vec, vec->num_elems - 1) : NULL;
        }
}

bool vector_repeated_push(vector *vec, const void *data, size_t how_often)
{
        ERROR_IF_NULL(vec && data)
        size_t next_num = vec->num_elems + how_often;
        while (next_num > vec->cap_elems) {
                size_t more = next_num - vec->cap_elems;
                vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
                vec->base = alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        for (size_t i = 0; i < how_often; i++) {
                memcpy(vec->base + (vec->num_elems + i) * vec->elem_size, data, vec->elem_size);
        }

        vec->num_elems += how_often;
        return true;
}

const void *vector_pop(vector *vec)
{
        void *result;
        if (LIKELY((result = (vec ? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elem_size : NULL)
                                      : NULL)) != NULL)) {
                vec->num_elems--;
        }
        return result;
}

bool vector_clear(vector *vec)
{
        ERROR_IF_NULL(vec)
        vec->num_elems = 0;
        return true;
}

bool vector_shrink(vector *vec)
{
        ERROR_IF_NULL(vec);
        if (vec->num_elems < vec->cap_elems) {
                vec->cap_elems = JAK_MAX(1, vec->num_elems);
                vec->base = alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        return true;
}

bool vector_grow(size_t *numNewSlots, vector *vec)
{
        ERROR_IF_NULL(vec)
        size_t freeSlotsBefore = vec->cap_elems - vec->num_elems;

        vec->cap_elems = (vec->cap_elems * vec->grow_factor) + 1;
        vec->base = alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        size_t freeSlotsAfter = vec->cap_elems - vec->num_elems;
        if (LIKELY(numNewSlots != NULL)) {
                *numNewSlots = freeSlotsAfter - freeSlotsBefore;
        }
        return true;
}

bool vector_grow_to(vector *vec, size_t capacity)
{
        ERROR_IF_NULL(vec);
        vec->cap_elems = JAK_MAX(vec->cap_elems, capacity);
        vec->base = alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        return true;
}

size_t vector_length(const vector *vec)
{
        ERROR_IF_NULL(vec)
        return vec->num_elems;
}

const void *vector_at(const vector *vec, size_t pos)
{
        return (vec && pos < vec->num_elems) ? vec->base + pos * vec->elem_size : NULL;
}

size_t vector_capacity(const vector *vec)
{
        ERROR_IF_NULL(vec)
        return vec->cap_elems;
}

bool vector_enlarge_size_to_capacity(vector *vec)
{
        ERROR_IF_NULL(vec);
        vec->num_elems = vec->cap_elems;
        return true;
}

bool vector_zero_memory(vector *vec)
{
        ERROR_IF_NULL(vec);
        ZERO_MEMORY(vec->base, vec->elem_size * vec->num_elems);
        return true;
}

bool vector_zero_memory_in_range(vector *vec, size_t from, size_t to)
{
        ERROR_IF_NULL(vec);
        JAK_ASSERT(from < to);
        JAK_ASSERT(to <= vec->cap_elems);
        ZERO_MEMORY(vec->base + from * vec->elem_size, vec->elem_size * (to - from));
        return true;
}

bool vector_set(vector *vec, size_t pos, const void *data)
{
        ERROR_IF_NULL(vec)
        JAK_ASSERT(pos < vec->num_elems);
        memcpy(vec->base + pos * vec->elem_size, data, vec->elem_size);
        return true;
}

bool vector_cpy(vector *dst, const vector *src)
{
        CHECK_SUCCESS(vector_create(dst, NULL, src->elem_size, src->num_elems));
        dst->num_elems = src->num_elems;
        if (dst->num_elems > 0) {
                memcpy(dst->base, src->base, src->elem_size * src->num_elems);
        }
        return true;
}

bool vector_cpy_to(vector *dst, vector *src)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        void *handle = realloc(dst->base, src->cap_elems * src->elem_size);
        if (handle) {
                dst->elem_size = src->elem_size;
                dst->num_elems = src->num_elems;
                dst->cap_elems = src->cap_elems;
                dst->grow_factor = src->grow_factor;
                dst->base = handle;
                memcpy(dst->base, src->base, src->cap_elems * src->elem_size);
                error_cpy(&dst->err, &src->err);
                return true;
        } else {
                ERROR(&src->err, ERR_HARDCOPYFAILED)
                return false;
        }
}

const void *vector_data(const vector *vec)
{
        return vec ? vec->base : NULL;
}

char *vector_string(const vector ofType(T) *vec,
                    void (*printerFunc)(memfile *dst, void ofType(T) *values, size_t num_elems))
{
        memblock *block;
        memfile file;
        memblock_create(&block, vec->num_elems * vec->elem_size);
        memfile_open(&file, block, READ_WRITE);
        printerFunc(&file, vec->base, vec->num_elems);
        return memblock_move_contents_and_drop(block);
}
