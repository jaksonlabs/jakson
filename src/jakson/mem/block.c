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

#include <jakson/mem/block.h>

typedef struct memblock {
        offset_t blockLength;
        offset_t last_byte;
        void *base;
        err err;
} memblock;

bool memblock_create(memblock **block, size_t size)
{
        ERROR_IF_NULL(block)
        ERROR_PRINT_IF(size == 0, ERR_ILLEGALARG)
        memblock *result = MALLOC(sizeof(memblock));
        ZERO_MEMORY(result, sizeof(memblock));
        ERROR_IF_NULL(result)
        result->blockLength = size;
        result->last_byte = 0;
        result->base = MALLOC(size);
        error_init(&result->err);
        *block = result;
        return true;
}

bool memblock_zero_out(memblock *block)
{
        ERROR_IF_NULL(block);
        ZERO_MEMORY(block->base, block->blockLength);
        return true;
}

bool memblock_from_file(memblock **block, FILE *file, size_t nbytes)
{
        memblock_create(block, nbytes);
        size_t numRead = fread((*block)->base, 1, nbytes, file);
        return numRead == nbytes ? true : false;
}

bool memblock_from_raw_data(memblock **block, const void *data, size_t nbytes)
{
        ERROR_IF_NULL(block)
        ERROR_IF_NULL(data)
        ERROR_IF_NULL(nbytes)

        memblock *result = MALLOC(sizeof(memblock));
        ERROR_IF_NULL(result)
        result->blockLength = nbytes;
        result->last_byte = nbytes;
        result->base = MALLOC(nbytes);
        ERROR_IF_NULL(result->base)
        memcpy(result->base, data, nbytes);
        error_init(&result->err);
        *block = result;
        return true;
}

bool memblock_drop(memblock *block)
{
        ERROR_IF_NULL(block)
        free(block->base);
        free(block);
        return true;
}

bool memblock_get_error(err *out, memblock *block)
{
        ERROR_IF_NULL(block);
        ERROR_IF_NULL(out);
        return error_cpy(out, &block->err);
}

bool memblock_size(offset_t *size, const memblock *block)
{
        ERROR_IF_NULL(block)
        *size = block->blockLength;
        return true;
}

offset_t memblock_last_used_byte(const memblock *block)
{
        return block ? block->last_byte : 0;
}

bool memblock_write_to_file(FILE *file, const memblock *block)
{
        size_t nwritten = fwrite(block->base, block->blockLength, 1, file);
        return nwritten == 1 ? true : false;
}

const char *memblock_raw_data(const memblock *block)
{
        return (block && block->base ? block->base : NULL);
}

bool memblock_resize(memblock *block, size_t size)
{
        ERROR_IF_NULL(block)
        ERROR_PRINT_IF(size == 0, ERR_ILLEGALARG)
        block->base = realloc(block->base, size);
        if (size > block->blockLength) {
                ZERO_MEMORY(block->base + block->blockLength, (size - block->blockLength));
        }
        block->blockLength = size;
        return true;
}

bool memblock_write(memblock *block, offset_t position, const char *data, offset_t nbytes)
{
        ERROR_IF_NULL(block)
        ERROR_IF_NULL(data)
        if (LIKELY(position + nbytes < block->blockLength)) {
                memcpy(block->base + position, data, nbytes);
                block->last_byte = JAK_MAX(block->last_byte, position + nbytes);
                return true;
        } else {
                return false;
        }
}

bool memblock_cpy(memblock **dst, memblock *src)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        CHECK_SUCCESS(memblock_create(dst, src->blockLength));
        memcpy((*dst)->base, src->base, src->blockLength);
        JAK_ASSERT((*dst)->base);
        JAK_ASSERT((*dst)->blockLength == src->blockLength);
        JAK_ASSERT(memcmp((*dst)->base, src->base, src->blockLength) == 0);
        (*dst)->last_byte = src->last_byte;
        return true;
}

bool memblock_shrink(memblock *block)
{
        ERROR_IF_NULL(block)
        block->blockLength = block->last_byte;
        block->base = realloc(block->base, block->blockLength);
        return true;
}

bool memblock_move_right(memblock *block, offset_t where, size_t nbytes)
{
        return memblock_move_ex(block, where, nbytes, true);
}

bool memblock_move_left(memblock *block, offset_t where, size_t nbytes)
{
        ERROR_IF_NULL(block)
        ERROR_IF(where + nbytes >= block->blockLength, &block->err, ERR_OUTOFBOUNDS)
        size_t remainder = block->blockLength - where - nbytes;
        if (remainder > 0) {
                memmove(block->base + where, block->base + where + nbytes, remainder);
                JAK_ASSERT(block->last_byte >= nbytes);
                block->last_byte -= nbytes;
                ZERO_MEMORY(block->base + block->blockLength - nbytes, nbytes)
                return true;
        } else {
                return false;
        }
}

bool memblock_move_ex(memblock *block, offset_t where, size_t nbytes, bool zero_out)
{
        ERROR_IF_NULL(block)
        ERROR_IF(where >= block->blockLength, &block->err, ERR_OUTOFBOUNDS);
        ERROR_IF(nbytes == 0, &block->err, ERR_ILLEGALARG);

        /** resize (if needed) */
        if (block->last_byte + nbytes > block->blockLength) {
                size_t new_length = (block->last_byte + nbytes);
                block->base = realloc(block->base, new_length);
                ERROR_IF(!block->base, &block->err, ERR_REALLOCERR);
                if (zero_out) {
                        ZERO_MEMORY(block->base + block->blockLength, (new_length - block->blockLength));
                }
                block->blockLength = new_length;
        }

        memmove(block->base + where + nbytes, block->base + where, block->last_byte - where);
        if (zero_out) {
                ZERO_MEMORY(block->base + where, nbytes);
        }
        block->last_byte += nbytes;
        return true;
}

void *memblock_move_contents_and_drop(memblock *block)
{
        void *result = block->base;
        block->base = NULL;
        free(block);
        return result;
}

bool memfile_update_last_byte(memblock *block, size_t where)
{
        ERROR_IF_NULL(block);
        ERROR_IF(where >= block->blockLength, &block->err, ERR_ILLEGALSTATE);
        block->last_byte = JAK_MAX(block->last_byte, where);
        return true;
}