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

#include <jak_memfile.h>
#include <jak_vector.h>

#define DEFINE_PRINTER_FUNCTION_WCAST(type, castType, format_string)                                                   \
void vector_##type##_PrinterFunc(struct jak_memfile *dst, void ofType(T) *values, size_t num_elems)                      \
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

DEFINE_PRINTER_FUNCTION_WCAST(u_char, jak_i8, "%d")

DEFINE_PRINTER_FUNCTION(jak_i8, "%d")

DEFINE_PRINTER_FUNCTION(jak_i16, "%d")

DEFINE_PRINTER_FUNCTION(jak_i32, "%d")

DEFINE_PRINTER_FUNCTION(jak_i64, "%"
        PRIi64)

DEFINE_PRINTER_FUNCTION(jak_u8, "%d")

DEFINE_PRINTER_FUNCTION(jak_u16, "%d")

DEFINE_PRINTER_FUNCTION(jak_u32, "%d")

DEFINE_PRINTER_FUNCTION(jak_u64, "%"
        PRIu64)

DEFINE_PRINTER_FUNCTION(size_t, "%zu")

bool vec_create(struct jak_vector *out, const jak_allocator *alloc, size_t elem_size, size_t cap_elems)
{
        JAK_ERROR_IF_NULL(out)
        out->allocator = JAK_MALLOC(sizeof(jak_allocator));
        jak_alloc_this_or_std(out->allocator, alloc);
        out->base = jak_alloc_malloc(out->allocator, cap_elems * elem_size);
        out->num_elems = 0;
        out->cap_elems = cap_elems;
        out->elem_size = elem_size;
        out->grow_factor = 1.7f;
        jak_error_init(&out->err);
        return true;
}

struct vector_serialize_header {
        char marker;
        jak_u32 elem_size;
        jak_u32 num_elems;
        jak_u32 cap_elems;
        float grow_factor;
};

bool vec_serialize(FILE *file, struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(vec)

        struct vector_serialize_header header =
                {.marker = JAK_MARKER_SYMBOL_VECTOR_HEADER, .elem_size = vec->elem_size, .num_elems = vec
                        ->num_elems, .cap_elems = vec->cap_elems, .grow_factor = vec->grow_factor};
        int nwrite = fwrite(&header, sizeof(struct vector_serialize_header), 1, file);
        JAK_ERROR_IF(nwrite != 1, &vec->err, JAK_ERR_FWRITE_FAILED);
        nwrite = fwrite(vec->base, vec->elem_size, vec->num_elems, file);
        JAK_ERROR_IF(nwrite != (int) vec->num_elems, &vec->err, JAK_ERR_FWRITE_FAILED);

        return true;
}

bool vec_deserialize(struct jak_vector *vec, jak_error *err, FILE *file)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(err)
        JAK_ERROR_IF_NULL(vec)

        jak_offset_t start = ftell(file);
        int err_code = JAK_ERR_NOERR;

        struct vector_serialize_header header;
        if (fread(&header, sizeof(struct vector_serialize_header), 1, file) != 1) {
                err_code = JAK_ERR_FREAD_FAILED;
                goto error_handling;
        }

        if (header.marker != JAK_MARKER_SYMBOL_VECTOR_HEADER) {
                err_code = JAK_ERR_CORRUPTED;
                goto error_handling;
        }

        vec->allocator = JAK_MALLOC(sizeof(jak_allocator));
        jak_alloc_this_or_std(vec->allocator, NULL);
        vec->base = jak_alloc_malloc(vec->allocator, header.cap_elems * header.elem_size);
        vec->num_elems = header.num_elems;
        vec->cap_elems = header.cap_elems;
        vec->elem_size = header.elem_size;
        vec->grow_factor = header.grow_factor;
        jak_error_init(&vec->err);

        if (fread(vec->base, header.elem_size, vec->num_elems, file) != vec->num_elems) {
                err_code = JAK_ERR_FREAD_FAILED;
                goto error_handling;
        }

        return true;

        error_handling:
        fseek(file, start, SEEK_SET);
        JAK_ERROR(err, err_code);
        return false;
}

bool vec_memadvice(struct jak_vector *vec, int madviseAdvice)
{
        JAK_ERROR_IF_NULL(vec);
        JAK_UNUSED(vec);
        JAK_UNUSED(madviseAdvice);
        madvise(vec->base, vec->cap_elems * vec->elem_size, madviseAdvice);
        return true;
}

bool vec_set_grow_factor(struct jak_vector *vec, float factor)
{
        JAK_ERROR_IF_NULL(vec);
        JAK_ERROR_PRINT_IF(factor <= 1.01f, JAK_ERR_ILLEGALARG)
        vec->grow_factor = factor;
        return true;
}

bool vec_drop(struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        jak_alloc_free(vec->allocator, vec->base);
        free(vec->allocator);
        vec->base = NULL;
        return true;
}

bool vec_is_empty(const struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        return vec->num_elems == 0 ? true : false;
}

bool vec_push(struct jak_vector *vec, const void *data, size_t num_elems)
{
        JAK_ERROR_IF_NULL(vec && data)
        size_t next_num = vec->num_elems + num_elems;
        while (next_num > vec->cap_elems) {
                size_t more = next_num - vec->cap_elems;
                vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
                vec->base = jak_alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        memcpy(vec->base + vec->num_elems * vec->elem_size, data, num_elems * vec->elem_size);
        vec->num_elems += num_elems;
        return true;
}

const void *vec_peek(struct jak_vector *vec)
{
        if (!vec) {
                return NULL;
        } else {
                return (vec->num_elems > 0) ? vec_at(vec, vec->num_elems - 1) : NULL;
        }
}

bool vec_repeated_push(struct jak_vector *vec, const void *data, size_t how_often)
{
        JAK_ERROR_IF_NULL(vec && data)
        size_t next_num = vec->num_elems + how_often;
        while (next_num > vec->cap_elems) {
                size_t more = next_num - vec->cap_elems;
                vec->cap_elems = (vec->cap_elems + more) * vec->grow_factor;
                vec->base = jak_alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        for (size_t i = 0; i < how_often; i++) {
                memcpy(vec->base + (vec->num_elems + i) * vec->elem_size, data, vec->elem_size);
        }

        vec->num_elems += how_often;
        return true;
}

const void *vec_pop(struct jak_vector *vec)
{
        void *result;
        if (JAK_LIKELY((result = (vec ? (vec->num_elems > 0 ? vec->base + (vec->num_elems - 1) * vec->elem_size : NULL)
                                      : NULL)) != NULL)) {
                vec->num_elems--;
        }
        return result;
}

bool vec_clear(struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        vec->num_elems = 0;
        return true;
}

bool vec_shrink(struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec);
        if (vec->num_elems < vec->cap_elems) {
                vec->cap_elems = JAK_MAX(1, vec->num_elems);
                vec->base = jak_alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        }
        return true;
}

bool vec_grow(size_t *numNewSlots, struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        size_t freeSlotsBefore = vec->cap_elems - vec->num_elems;

        vec->cap_elems = (vec->cap_elems * vec->grow_factor) + 1;
        vec->base = jak_alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        size_t freeSlotsAfter = vec->cap_elems - vec->num_elems;
        if (JAK_LIKELY(numNewSlots != NULL)) {
                *numNewSlots = freeSlotsAfter - freeSlotsBefore;
        }
        return true;
}

bool vec_grow_to(struct jak_vector *vec, size_t capacity)
{
        JAK_ERROR_IF_NULL(vec);
        vec->cap_elems = JAK_MAX(vec->cap_elems, capacity);
        vec->base = jak_alloc_realloc(vec->allocator, vec->base, vec->cap_elems * vec->elem_size);
        return true;
}

size_t vec_length(const struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        return vec->num_elems;
}

const void *vec_at(const struct jak_vector *vec, size_t pos)
{
        return (vec && pos < vec->num_elems) ? vec->base + pos * vec->elem_size : NULL;
}

size_t vec_capacity(const struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec)
        return vec->cap_elems;
}

bool vec_enlarge_size_to_capacity(struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec);
        vec->num_elems = vec->cap_elems;
        return true;
}

bool vec_zero_memory(struct jak_vector *vec)
{
        JAK_ERROR_IF_NULL(vec);
        JAK_ZERO_MEMORY(vec->base, vec->elem_size * vec->num_elems);
        return true;
}

bool vec_zero_memory_in_range(struct jak_vector *vec, size_t from, size_t to)
{
        JAK_ERROR_IF_NULL(vec);
        JAK_ASSERT(from < to);
        JAK_ASSERT(to <= vec->cap_elems);
        JAK_ZERO_MEMORY(vec->base + from * vec->elem_size, vec->elem_size * (to - from));
        return true;
}

bool vec_set(struct jak_vector *vec, size_t pos, const void *data)
{
        JAK_ERROR_IF_NULL(vec)
        JAK_ASSERT(pos < vec->num_elems);
        memcpy(vec->base + pos * vec->elem_size, data, vec->elem_size);
        return true;
}

bool vec_cpy(struct jak_vector *dst, const struct jak_vector *src)
{
        JAK_CHECK_SUCCESS(vec_create(dst, NULL, src->elem_size, src->num_elems));
        dst->num_elems = src->num_elems;
        if (dst->num_elems > 0) {
                memcpy(dst->base, src->base, src->elem_size * src->num_elems);
        }
        return true;
}

bool vec_cpy_to(struct jak_vector *dst, struct jak_vector *src)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        void *handle = realloc(dst->base, src->cap_elems * src->elem_size);
        if (handle) {
                dst->elem_size = src->elem_size;
                dst->num_elems = src->num_elems;
                dst->cap_elems = src->cap_elems;
                dst->grow_factor = src->grow_factor;
                dst->base = handle;
                memcpy(dst->base, src->base, src->cap_elems * src->elem_size);
                jak_error_cpy(&dst->err, &src->err);
                return true;
        } else {
                JAK_ERROR(&src->err, JAK_ERR_HARDCOPYFAILED)
                return false;
        }
}

const void *vec_data(const struct jak_vector *vec)
{
        return vec ? vec->base : NULL;
}

char *vector_string(const struct jak_vector ofType(T) *vec,
                    void (*printerFunc)(struct jak_memfile *dst, void ofType(T) *values, size_t num_elems))
{
        jak_memblock *block;
        struct jak_memfile file;
        jak_memblock_create(&block, vec->num_elems * vec->elem_size);
        memfile_open(&file, block, JAK_READ_WRITE);
        printerFunc(&file, vec->base, vec->num_elems);
        return jak_memblock_move_contents_and_drop(block);
}
