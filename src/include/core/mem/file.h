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

#ifndef NG5_MEMFILE_H
#define NG5_MEMFILE_H

#include "block.h"

NG5_BEGIN_DECL

typedef enum
{
    NG5_MEMFILE_MODE_READWRITE,
    NG5_MEMFILE_MODE_READONLY
} carbon_memfile_mode_e;

typedef struct carbon_memfile
{
    carbon_memblock_t *memblock;
    offset_t pos;
    bool bit_mode;
    size_t current_read_bit, current_write_bit, bytes_completed;
    carbon_memfile_mode_e mode;
    struct err err;
} memfile_t;

#define NG5_MEMFILE_PEEK(file, type)                                                                                \
({                                                                                                                     \
    assert (carbon_memfile_remain_size(file) >= sizeof(type));                                                         \
    (type*) carbon_memfile_peek(file, sizeof(type));                                                                   \
})

#define NG5_MEMFILE_READ_TYPE(file, type)                                                                           \
({                                                                                                                     \
    assert (carbon_memfile_remain_size(file) >= sizeof(type));                                                         \
    (type*) carbon_memfile_read(file, sizeof(type));                                                                   \
})

#define NG5_MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                            \
    (const type *) NG5_MEMFILE_READ(file, how_many * sizeof(type))

#define NG5_MEMFILE_READ(file, nbytes)                                                                              \
({                                                                                                                     \
    assert (carbon_memfile_remain_size(file) >= nbytes);                                                               \
    carbon_memfile_read(file, nbytes);                                                                                 \
})

#define memfile_tell(file)                                                                                      \
({                                                                                                                     \
    offset_t offset;                                                                                               \
    carbon_memfile_tell(&offset, file);                                                                                \
    offset;                                                                                                            \
})

NG5_EXPORT(bool)
carbon_memfile_open(memfile_t *file, carbon_memblock_t *block, carbon_memfile_mode_e mode);

NG5_EXPORT(bool)
carbon_memfile_seek(memfile_t *file, offset_t pos);

NG5_EXPORT(bool)
carbon_memfile_rewind(memfile_t *file);

NG5_EXPORT(bool)
carbon_memfile_tell(offset_t *pos, const memfile_t *file);

NG5_EXPORT(size_t)
carbon_memfile_size(memfile_t *file);

NG5_EXPORT(size_t)
carbon_memfile_remain_size(memfile_t *file);

NG5_EXPORT(bool)
carbon_memfile_shrink(memfile_t *file);

NG5_EXPORT(const carbon_byte_t *)
carbon_memfile_read(memfile_t *file, offset_t nbytes);

NG5_EXPORT(bool)
carbon_memfile_skip(memfile_t *file, offset_t nbytes);

NG5_EXPORT(const carbon_byte_t *)
carbon_memfile_peek(memfile_t *file, offset_t nbytes);

NG5_EXPORT(bool)
memfile_write(memfile_t *file, const void *data, offset_t nbytes);

NG5_EXPORT(bool)
carbon_memfile_begin_bit_mode(memfile_t *file);

NG5_EXPORT(bool)
memfile_write_bit(memfile_t *file, bool flag);

NG5_EXPORT(bool)
carbon_memfile_read_bit(memfile_t *file);

NG5_EXPORT(bool)
carbon_memfile_end_bit_mode(NG5_NULLABLE size_t *num_bytes_written, memfile_t *file);

NG5_EXPORT(void *)
carbon_memfile_current_pos(memfile_t *file, offset_t nbytes);

NG5_END_DECL

#endif