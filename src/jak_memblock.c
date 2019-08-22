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

#include <jak_memblock.h>

struct jak_memblock {
        jak_offset_t blockLength;
        jak_offset_t last_byte;
        void *base;
        jak_error err;
};

bool memblock_create(struct jak_memblock **block, size_t size)
{
        JAK_ERROR_IF_NULL(block)
        JAK_ERROR_PRINT_IF(size == 0, JAK_ERR_ILLEGALARG)
        struct jak_memblock *result = JAK_MALLOC(sizeof(struct jak_memblock));
        JAK_zero_memory(result, sizeof(struct jak_memblock));
        JAK_ERROR_IF_NULL(result)
        result->blockLength = size;
        result->last_byte = 0;
        result->base = JAK_MALLOC(size);
        jak_error_init(&result->err);
        *block = result;
        return true;
}

bool memblock_zero_out(struct jak_memblock *block)
{
        JAK_ERROR_IF_NULL(block);
        JAK_zero_memory(block->base, block->blockLength);
        return true;
}

bool memblock_from_file(struct jak_memblock **block, FILE *file, size_t nbytes)
{
        memblock_create(block, nbytes);
        size_t numRead = fread((*block)->base, 1, nbytes, file);
        return numRead == nbytes ? true : false;
}

bool memblock_drop(struct jak_memblock *block)
{
        JAK_ERROR_IF_NULL(block)
        free(block->base);
        free(block);
        return true;
}

bool memblock_get_error(jak_error *out, struct jak_memblock *block)
{
        JAK_ERROR_IF_NULL(block);
        JAK_ERROR_IF_NULL(out);
        return jak_error_cpy(out, &block->err);
}

bool memblock_size(jak_offset_t *size, const struct jak_memblock *block)
{
        JAK_ERROR_IF_NULL(block)
        *size = block->blockLength;
        return true;
}

jak_offset_t memblock_last_used_byte(const struct jak_memblock *block)
{
        return block ? block->last_byte : 0;
}

bool memblock_write_to_file(FILE *file, const struct jak_memblock *block)
{
        size_t nwritten = fwrite(block->base, block->blockLength, 1, file);
        return nwritten == 1 ? true : false;
}

const char *memblock_raw_data(const struct jak_memblock *block)
{
        return (block && block->base ? block->base : NULL);
}

bool memblock_resize(struct jak_memblock *block, size_t size)
{
        JAK_ERROR_IF_NULL(block)
        JAK_ERROR_PRINT_IF(size == 0, JAK_ERR_ILLEGALARG)
        block->base = realloc(block->base, size);
        if (size > block->blockLength) {
                JAK_zero_memory(block->base + block->blockLength, (size - block->blockLength));
        }
        block->blockLength = size;
        return true;
}

bool memblock_write(struct jak_memblock *block, jak_offset_t position, const char *data, jak_offset_t nbytes)
{
        JAK_ERROR_IF_NULL(block)
        JAK_ERROR_IF_NULL(data)
        if (JAK_LIKELY(position + nbytes < block->blockLength)) {
                memcpy(block->base + position, data, nbytes);
                block->last_byte = JAK_max(block->last_byte, position + nbytes);
                return true;
        } else {
                return false;
        }
}

bool memblock_cpy(struct jak_memblock **dst, struct jak_memblock *src)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        JAK_check_success(memblock_create(dst, src->blockLength));
        memcpy((*dst)->base, src->base, src->blockLength);
        JAK_ASSERT((*dst)->base);
        JAK_ASSERT((*dst)->blockLength == src->blockLength);
        JAK_ASSERT(memcmp((*dst)->base, src->base, src->blockLength) == 0);
        (*dst)->last_byte = src->last_byte;
        return true;
}

bool memblock_shrink(struct jak_memblock *block)
{
        JAK_ERROR_IF_NULL(block)
        block->blockLength = block->last_byte;
        block->base = realloc(block->base, block->blockLength);
        return true;
}

bool memblock_move_right(struct jak_memblock *block, jak_offset_t where, size_t nbytes)
{
        return memblock_move_ex(block, where, nbytes, true);
}

bool memblock_move_left(struct jak_memblock *block, jak_offset_t where, size_t nbytes)
{
        JAK_ERROR_IF_NULL(block)
        JAK_ERROR_IF(where + nbytes >= block->blockLength, &block->err, JAK_ERR_OUTOFBOUNDS)
        size_t remainder = block->blockLength - where - nbytes;
        if (remainder > 0) {
                memmove(block->base + where, block->base + where + nbytes, remainder);
                JAK_ASSERT(block->last_byte >= nbytes);
                block->last_byte -= nbytes;
                JAK_zero_memory(block->base + block->blockLength - nbytes, nbytes)
                return true;
        } else {
                return false;
        }
}

bool memblock_move_ex(struct jak_memblock *block, jak_offset_t where, size_t nbytes, bool zero_out)
{
        JAK_ERROR_IF_NULL(block)
        JAK_ERROR_IF(where >= block->blockLength, &block->err, JAK_ERR_OUTOFBOUNDS);
        JAK_ERROR_IF(nbytes == 0, &block->err, JAK_ERR_ILLEGALARG);

        /* resize (if needed) */
        if (block->last_byte + nbytes > block->blockLength) {
                size_t new_length = (block->last_byte + nbytes);
                block->base = realloc(block->base, new_length);
                JAK_ERROR_IF(!block->base, &block->err, JAK_ERR_REALLOCERR);
                if (zero_out) {
                        JAK_zero_memory(block->base + block->blockLength, (new_length - block->blockLength));
                }
                block->blockLength = new_length;
        }

        memmove(block->base + where + nbytes, block->base + where, block->last_byte - where);
        if (zero_out) {
                JAK_zero_memory(block->base + where, nbytes);
        }
        block->last_byte += nbytes;
        return true;
}

void *memblock_move_contents_and_drop(struct jak_memblock *block)
{
        void *result = block->base;
        block->base = NULL;
        free(block);
        return result;
}

bool memfile_update_last_byte(struct jak_memblock *block, size_t where)
{
        JAK_ERROR_IF_NULL(block);
        JAK_ERROR_IF(where >= block->blockLength, &block->err, JAK_ERR_ILLEGALSTATE);
        block->last_byte = JAK_max(block->last_byte, where);
        return true;
}