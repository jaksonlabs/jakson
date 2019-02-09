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

#include "carbon/carbon-memblock.h"
#include "carbon/carbon-error.h"

typedef struct carbon_memblock
{
    carbon_off_t  blockLength;
    carbon_off_t  lastByte;
    void   *base;
    carbon_err_t  err;
} carbon_memblock_t;

bool carbon_memblock_create(carbon_memblock_t **block, size_t size)
{
    CARBON_NON_NULL_OR_ERROR(block)
    CARBON_PRINT_ERROR_IF(size == 0, CARBON_ERR_ILLEGALARG)
    carbon_memblock_t *result = malloc(sizeof(carbon_memblock_t));
    CARBON_NON_NULL_OR_ERROR(result)
    result->blockLength = size;
    result->lastByte = 0;
    result->base = malloc(size);
    carbon_error_init(&result->err);
    *block = result;
    return true;
}

bool carbon_memblock_from_file(carbon_memblock_t **block, FILE *file, size_t nbytes)
{
    carbon_memblock_create(block, nbytes);
    size_t numRead = fread((*block)->base, 1, nbytes, file);
    return numRead == nbytes ? true : false;
}

bool carbon_memblock_drop(carbon_memblock_t *block)
{
    CARBON_NON_NULL_OR_ERROR(block)
    free(block->base);
    free(block);
    return true;
}

CARBON_EXPORT(bool)
carbon_memblock_get_error(carbon_err_t *out, carbon_memblock_t *block)
{
    CARBON_NON_NULL_OR_ERROR(block);
    CARBON_NON_NULL_OR_ERROR(out);
    return carbon_error_cpy(out, &block->err);
}

bool carbon_memblock_size(carbon_off_t *size, const carbon_memblock_t *block)
{
    CARBON_NON_NULL_OR_ERROR(block)
    *size = block->blockLength;
    return true;
}

bool carbon_memblock_write_to_file(FILE *file, const carbon_memblock_t *block)
{
    size_t nwritten = fwrite(block->base, block->blockLength, 1, file);
    return nwritten == 1 ? true : false;
}

const carbon_byte_t *carbon_memblock_raw_data(const carbon_memblock_t *block)
{
    return (block && block->base ? block->base : NULL);
}

bool carbon_memblock_resize(carbon_memblock_t *block, size_t size)
{
    CARBON_NON_NULL_OR_ERROR(block)
    CARBON_PRINT_ERROR_IF(size == 0, CARBON_ERR_ILLEGALARG)
    block->base = realloc(block->base, size);
    block->blockLength = size;
    return true;
}

bool carbon_memblock_write(carbon_memblock_t *block,
                           carbon_off_t position,
                           const carbon_byte_t *data,
                           carbon_off_t nbytes)
{
    CARBON_NON_NULL_OR_ERROR(block)
    CARBON_NON_NULL_OR_ERROR(data)
    assert(position + nbytes < block->blockLength);
    memcpy(block->base + position, data, nbytes);
    block->lastByte = CARBON_MAX(block->lastByte, position + nbytes);
    return true;
}

bool carbon_memblock_cpy(carbon_memblock_t **dst, carbon_memblock_t *src)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_CHECK_SUCCESS(carbon_memblock_create(dst, src->blockLength));
    memcpy((*dst)->base, src->base, src->blockLength);
    assert((*dst)->base);
    assert((*dst)->blockLength == src->blockLength);
    assert(memcmp((*dst)->base, src->base, src->blockLength) == 0);
    return true;
}

bool carbon_memblock_shrink(carbon_memblock_t *block)
{
    CARBON_NON_NULL_OR_ERROR(block)
    block->blockLength = block->lastByte;
    block->base = realloc(block->base, block->blockLength);
    return true;
}

void *carbon_memblock_move_contents_and_drop(carbon_memblock_t *block)
{
    void *result = block->base;
    block->base = NULL;
    free(block);
    return result;
}