/*
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

#ifndef CARBON_MEMFILE_H
#define CARBON_MEMFILE_H

#include "carbon-memblock.h"

CARBON_BEGIN_DECL

typedef enum
{
    CARBON_MEMFILE_MODE_READWRITE,
    CARBON_MEMFILE_MODE_READONLY
} carbon_memfile_mode_e;

typedef struct carbon_memfile
{
    carbon_memblock_t *memblock;
    carbon_off_t pos;
    bool bit_mode;
    size_t current_read_bit, current_write_bit, bytes_completed;
    carbon_memfile_mode_e mode;
    carbon_err_t err;
} carbon_memfile_t;

#define CARBON_MEMFILE_PEEK(file, type)                            \
({                                                          \
    assert (carbon_memfile_remain_size(file) >= sizeof(type));       \
    (type*) carbon_memfile_peek(file, sizeof(type));                \
})

#define CARBON_MEMFILE_READ_TYPE(file, type)                       \
({                                                          \
    assert (carbon_memfile_remain_size(file) >= sizeof(type));       \
    (type*) carbon_memfile_read(file, sizeof(type));                \
})

#define CARBON_MEMFILE_READ(file, nbytes)                          \
({                                                          \
    assert (carbon_memfile_remain_size(file) >= nbytes);             \
    carbon_memfile_read(file, nbytes);                              \
})

#define CARBON_MEMFILE_TELL(file)                                  \
({                                                          \
    carbon_off_t offset;                                          \
    carbon_memfile_tell(&offset, file);                             \
    offset;                                                 \
})

CARBON_EXPORT(bool)
carbon_memfile_open(carbon_memfile_t *file, carbon_memblock_t *block, carbon_memfile_mode_e mode);

CARBON_EXPORT(bool)
carbon_memfile_seek(carbon_memfile_t *file, carbon_off_t pos);

CARBON_EXPORT(bool)
carbon_memfile_rewind(carbon_memfile_t *file);

CARBON_EXPORT(bool)
carbon_memfile_tell(carbon_off_t *pos, const carbon_memfile_t *file);

CARBON_EXPORT(size_t)
carbon_memfile_size(carbon_memfile_t *file);

CARBON_EXPORT(size_t)
carbon_memfile_remain_size(carbon_memfile_t *file);

CARBON_EXPORT(bool)
carbon_memfile_shrink(carbon_memfile_t *file);

CARBON_EXPORT(const carbon_byte_t *)
carbon_memfile_read(carbon_memfile_t *file, carbon_off_t nbytes);

CARBON_EXPORT(bool)
carbon_memfile_skip(carbon_memfile_t *file, carbon_off_t nbytes);

CARBON_EXPORT(const carbon_byte_t *)
carbon_memfile_peek(carbon_memfile_t *file, carbon_off_t nbytes);

CARBON_EXPORT(bool)
carbon_memfile_write(carbon_memfile_t *file, const void *data, carbon_off_t nbytes);

CARBON_EXPORT(bool)
carbon_memfile_begin_bit_mode(carbon_memfile_t *file);

CARBON_EXPORT(bool)
carbon_memfile_write_bit(carbon_memfile_t *file, bool flag);

CARBON_EXPORT(bool)
carbon_memfile_read_bit(carbon_memfile_t *file);

CARBON_EXPORT(bool)
carbon_memfile_end_bit_mode(CARBON_NULLABLE size_t *num_bytes_written, carbon_memfile_t *file);

CARBON_EXPORT(void *)
carbon_memfile_current_pos(carbon_memfile_t *file, carbon_off_t nbytes);

CARBON_END_DECL

#endif