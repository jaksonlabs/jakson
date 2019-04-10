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
        NG5_NON_NULL_OR_ERROR(block)
        NG5_PRINT_ERROR_IF(size == 0, NG5_ERR_ILLEGALARG)
        struct memblock *result = malloc(sizeof(struct memblock));
        NG5_NON_NULL_OR_ERROR(result)
        result->blockLength = size;
        result->lastByte = 0;
        result->base = malloc(size);
        error_init(&result->err);
        *block = result;
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
        NG5_NON_NULL_OR_ERROR(block)
        free(block->base);
        free(block);
        return true;
}

NG5_EXPORT(bool) memblock_get_error(struct err *out, struct memblock *block)
{
        NG5_NON_NULL_OR_ERROR(block);
        NG5_NON_NULL_OR_ERROR(out);
        return error_cpy(out, &block->err);
}

bool memblock_size(offset_t *size, const struct memblock *block)
{
        NG5_NON_NULL_OR_ERROR(block)
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

bool memblock_resize(struct memblock *block, size_t size)
{
        NG5_NON_NULL_OR_ERROR(block)
        NG5_PRINT_ERROR_IF(size == 0, NG5_ERR_ILLEGALARG)
        block->base = realloc(block->base, size);
        block->blockLength = size;
        return true;
}

bool memblock_write(struct memblock *block, offset_t position, const char *data, offset_t nbytes)
{
        NG5_NON_NULL_OR_ERROR(block)
        NG5_NON_NULL_OR_ERROR(data)
        if (NG5_LIKELY(position + nbytes < block->blockLength)) {
                memcpy(block->base + position, data, nbytes);
                block->lastByte = NG5_MAX(block->lastByte, position + nbytes);
                return true;
        } else {
                return false;
        }
}

bool memblock_cpy(struct memblock **dst, struct memblock *src)
{
        NG5_NON_NULL_OR_ERROR(dst)
        NG5_NON_NULL_OR_ERROR(src)
        NG5_CHECK_SUCCESS(memblock_create(dst, src->blockLength));
        memcpy((*dst)->base, src->base, src->blockLength);
        assert((*dst)->base);
        assert((*dst)->blockLength == src->blockLength);
        assert(memcmp((*dst)->base, src->base, src->blockLength) == 0);
        return true;
}

bool memblock_shrink(struct memblock *block)
{
        NG5_NON_NULL_OR_ERROR(block)
        block->blockLength = block->lastByte;
        block->base = realloc(block->base, block->blockLength);
        return true;
}

void *memblock_move_contents_and_drop(struct memblock *block)
{
        void *result = block->base;
        block->base = NULL;
        free(block);
        return result;
}