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

#include "core/mem/block.h"
#include "shared/error.h"

struct memblock {
        offset_t blockLength;
        offset_t lastByte;
        void *base;
        struct err err;
};

bool memblock_create(struct memblock **block, size_t size)
{
        error_if_null(block)
        error_print_if(size == 0, NG5_ERR_ILLEGALARG)
        struct memblock *result = malloc(sizeof(struct memblock));
        error_if_null(result)
        result->blockLength = size;
        result->lastByte = 0;
        result->base = malloc(size);
        error_init(&result->err);
        *block = result;
        return true;
}

NG5_EXPORT(bool) memblock_zero_out(struct memblock *block)
{
        error_if_null(block);
        ng5_zero_memory(block->base, block->blockLength);
        return true;
}

bool memblock_from_file(struct memblock **block, FILE *file, size_t nbytes)
{
        memblock_create(block, nbytes);
        size_t numRead = fread((*block)->base, 1, nbytes, file);
        return numRead == nbytes ? true : false;
}

bool memblock_drop(struct memblock *block)
{
        error_if_null(block)
        free(block->base);
        free(block);
        return true;
}

NG5_EXPORT(bool) memblock_get_error(struct err *out, struct memblock *block)
{
        error_if_null(block);
        error_if_null(out);
        return error_cpy(out, &block->err);
}

bool memblock_size(offset_t *size, const struct memblock *block)
{
        error_if_null(block)
        *size = block->blockLength;
        return true;
}

bool memblock_write_to_file(FILE *file, const struct memblock *block)
{
        size_t nwritten = fwrite(block->base, block->blockLength, 1, file);
        return nwritten == 1 ? true : false;
}

const char *memblock_raw_data(const struct memblock *block)
{
        return (block && block->base ? block->base : NULL);
}

NG5_EXPORT(bool) memblock_resize(struct memblock *block, size_t size)
{
        return memblock_resize_ex(block, size, false);
}

bool memblock_resize_ex(struct memblock *block, size_t size, bool zero_out)
{
        error_if_null(block)
        error_print_if(size == 0, NG5_ERR_ILLEGALARG)
        block->base = realloc(block->base, size);
        if (zero_out) {
                ng5_zero_memory(block->base + block->blockLength, (size - block->blockLength));
        }
        block->blockLength = size;
        return true;
}

bool memblock_write(struct memblock *block, offset_t position, const char *data, offset_t nbytes)
{
        error_if_null(block)
        error_if_null(data)
        if (likely(position + nbytes < block->blockLength)) {
                memcpy(block->base + position, data, nbytes);
                block->lastByte = ng5_max(block->lastByte, position + nbytes);
                return true;
        } else {
                return false;
        }
}

bool memblock_cpy(struct memblock **dst, struct memblock *src)
{
        error_if_null(dst)
        error_if_null(src)
        ng5_check_success(memblock_create(dst, src->blockLength));
        memcpy((*dst)->base, src->base, src->blockLength);
        assert((*dst)->base);
        assert((*dst)->blockLength == src->blockLength);
        assert(memcmp((*dst)->base, src->base, src->blockLength) == 0);
        (*dst)->lastByte = src->lastByte;
        return true;
}

bool memblock_shrink(struct memblock *block)
{
        error_if_null(block)
        block->blockLength = block->lastByte;
        block->base = realloc(block->base, block->blockLength);
        return true;
}

NG5_EXPORT(bool) memblock_move(struct memblock *block, offset_t where, size_t nbytes)
{
        return memblock_move_ex(block, where, nbytes, true);
}

NG5_EXPORT(bool) memblock_move_ex(struct memblock *block, offset_t where, size_t nbytes, bool zero_out)
{
        error_if_null(block)
        error_if(where >= block->blockLength, &block->err, NG5_ERR_OUTOFBOUNDS);
        error_if(nbytes == 0, &block->err, NG5_ERR_ILLEGALARG);

        /* resize (if needed) */
        if (block->lastByte + nbytes > block->blockLength) {
                size_t new_length = (block->lastByte + nbytes);
                block->base = realloc(block->base, new_length);
                error_if(!block->base, &block->err, NG5_ERR_REALLOCERR);
                if (zero_out) {
                        ng5_zero_memory(block->base + block->blockLength, (new_length - block->blockLength));
                }
                block->blockLength = new_length;
        }

        memmove(block->base + where + nbytes, block->base + where, block->lastByte - where);
        if (zero_out) {
                ng5_zero_memory(block->base + where, nbytes);
        }
        block->lastByte += nbytes;
        return true;
}

void *memblock_move_contents_and_drop(struct memblock *block)
{
        void *result = block->base;
        block->base = NULL;
        free(block);
        return result;
}