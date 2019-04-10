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

#include "core/mem/memfile.h"

bool carbon_memfile_open(memfile_t *file, carbon_memblock_t *block, carbon_memfile_mode_e mode)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(block)
    file->memblock = block;
    file->pos = 0;
    file->bit_mode = false;
    file->mode = mode;
    carbon_error_init(&file->err);
    return true;
}

bool carbon_memfile_seek(memfile_t *file, offset_t pos)
{
    CARBON_NON_NULL_OR_ERROR(file)
    offset_t file_size;
    carbon_memblock_size(&file_size, file->memblock);
    if (CARBON_UNLIKELY(pos >= file_size)) {
        if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
            offset_t new_size = pos + 1;
            carbon_memblock_resize(file->memblock, new_size);
        } else {
            error(&file->err, CARBON_ERR_MEMSTATE)
            return false;
        }
    }
    file->pos = pos;
    return true;
}

bool carbon_memfile_rewind(memfile_t *file)
{
    CARBON_NON_NULL_OR_ERROR(file)
    file->pos = 0;
    return true;
}

bool carbon_memfile_tell(offset_t *pos, const memfile_t *file)
{
    CARBON_NON_NULL_OR_ERROR(pos)
    CARBON_NON_NULL_OR_ERROR(file)
    *pos = file->pos;
    return true;
}

size_t carbon_memfile_size(memfile_t *file)
{
    if (!file || !file->memblock) {
        return 0;
    } else {
        size_t size;
        carbon_memblock_size(&size, file->memblock);
        return size;
    }
}

size_t carbon_memfile_remain_size(memfile_t *file)
{
    assert(file->pos <= carbon_memfile_size(file));
    return carbon_memfile_size(file) - file->pos;
}

bool carbon_memfile_shrink(memfile_t *file)
{
    CARBON_NON_NULL_OR_ERROR(file);
    if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
        int status = carbon_memblock_shrink(file->memblock);
        size_t size;
        carbon_memblock_size(&size, file->memblock);
        assert(size == file->pos);
        return status;
    } else {
        error(&file->err, CARBON_ERR_WRITEPROT)
        return false;
    }
}

const carbon_byte_t *carbon_memfile_read(memfile_t *file, offset_t nbytes)
{
    const carbon_byte_t *result = carbon_memfile_peek(file, nbytes);
    file->pos += nbytes;
    return result;
}

bool carbon_memfile_skip(memfile_t *file, offset_t nbytes)
{
    offset_t required_size = file->pos + nbytes;
    file->pos += nbytes;
    offset_t file_size;
    carbon_memblock_size(&file_size, file->memblock);

    if (CARBON_UNLIKELY(required_size >= file_size)) {
        if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
            carbon_memblock_resize(file->memblock, required_size * 1.7f);
        } else {
            error(&file->err, CARBON_ERR_WRITEPROT);
            return false;
        }
    }
    assert(file->pos < carbon_memfile_size(file));
    return true;
}

const carbon_byte_t *carbon_memfile_peek(memfile_t *file, offset_t nbytes)
{
    offset_t file_size;
    carbon_memblock_size(&file_size, file->memblock);
    if (CARBON_UNLIKELY(file->pos + nbytes > file_size)) {
        error(&file->err, CARBON_ERR_READOUTOFBOUNDS);
        return NULL;
    } else {
        const carbon_byte_t *result = carbon_memblock_raw_data(file->memblock) + file->pos;
        return result;
    }
}

bool memfile_write(memfile_t *file, const void *data, offset_t nbytes)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(data)
    if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
        if (CARBON_LIKELY(nbytes != 0)) {
            offset_t file_size;
            carbon_memblock_size(&file_size, file->memblock);
            offset_t required_size = file->pos + nbytes;
            if (CARBON_UNLIKELY(required_size >= file_size)) {
                carbon_memblock_resize(file->memblock, required_size * 1.7f);
            }

            if (CARBON_UNLIKELY(!carbon_memblock_write(file->memblock, file->pos, data, nbytes))) {
                return false;
            }
            file->pos += nbytes;
        }
        return true;
    } else {
        error(&file->err, CARBON_ERR_WRITEPROT);
        return false;
    }
}

bool carbon_memfile_begin_bit_mode(memfile_t *file)
{
    CARBON_NON_NULL_OR_ERROR(file);
    if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
        file->bit_mode = true;
        file->current_read_bit = file->current_write_bit = file->bytes_completed = 0;
        file->bytes_completed = 0;
        offset_t offset;
        char empty = '\0';
        carbon_memfile_tell(&offset, file);
            memfile_write(file, &empty, sizeof(char));
        carbon_memfile_seek(file, offset);
    } else {
        error(&file->err, CARBON_ERR_WRITEPROT);
        return false;
    }

    return true;
}

bool memfile_write_bit(memfile_t *file, bool flag)
{
    CARBON_NON_NULL_OR_ERROR(file);
    file->current_read_bit = 0;

    if (file->bit_mode) {
        if (file->current_write_bit < 8) {
            offset_t offset;
            carbon_memfile_tell(&offset, file);
            char byte = *carbon_memfile_read(file, sizeof(char));
            char mask = 1 << file->current_write_bit;
            if (flag) {
                CARBON_FIELD_SET(byte, mask);
            } else {
                CARBON_FIELD_CLEAR(byte, mask);
            }
            carbon_memfile_seek(file, offset);
                memfile_write(file, &byte, sizeof(char));
            carbon_memfile_seek(file, offset);
            file->current_write_bit++;
        } else {
            file->current_write_bit = 0;
            file->bytes_completed++;
            char empty = '\0';
            offset_t off;
            carbon_memfile_skip(file, 1);
            carbon_memfile_tell(&off, file);
                memfile_write(file, &empty, sizeof(char));
            carbon_memfile_seek(file, off);

            return memfile_write_bit(file, flag);
        }
        return true;
    } else {
        error(&file->err, CARBON_ERR_NOBITMODE);
        return false;
    }
}

bool carbon_memfile_read_bit(memfile_t *file)
{
    if (!file) {
        return false;
    }

    file->current_write_bit = 0;

    if (file->bit_mode) {
        if (file->current_read_bit < 8) {
            offset_t offset;
            carbon_memfile_tell(&offset, file);

            char mask = 1 << file->current_read_bit;
            char byte = *carbon_memfile_read(file, sizeof(char));
            carbon_memfile_seek(file, offset);
            bool result = ((byte & mask) >> file->current_read_bit) == true;
            file->current_read_bit++;
            return result;
        } else {
            file->current_read_bit = 0;
            carbon_memfile_skip(file, sizeof(char));
            return carbon_memfile_read_bit(file);
        }
    } else {
        error(&file->err, CARBON_ERR_NOBITMODE);
        return false;
    }
}

bool carbon_memfile_end_bit_mode(CARBON_NULLABLE size_t *num_bytes_written, memfile_t *file)
{
    CARBON_NON_NULL_OR_ERROR(file);
    file->bit_mode = false;
    if (file->current_write_bit <= 8) {
        carbon_memfile_skip(file, 1);
        file->bytes_completed++;
    }
    CARBON_OPTIONAL_SET(num_bytes_written, file->bytes_completed);
    file->current_write_bit = file->bytes_completed = 0;
    return true;
}

void *carbon_memfile_current_pos(memfile_t *file, offset_t nbytes)
{
    if (file && nbytes > 0) {
        offset_t file_size;
        carbon_memblock_size(&file_size, file->memblock);
        offset_t required_size = file->pos + nbytes;
        if (CARBON_UNLIKELY(file->pos + nbytes >= file_size)) {
            if (file->mode == CARBON_MEMFILE_MODE_READWRITE) {
                carbon_memblock_resize(file->memblock, required_size * 1.7f);
            } else {
                error(&file->err, CARBON_ERR_WRITEPROT);
                return NULL;
            }
        }
        void *data = (void *) carbon_memfile_peek(file, nbytes);
        return data;
    } else {
        return NULL;
    }
}