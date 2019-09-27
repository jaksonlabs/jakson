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

#ifndef MEMFILE_H
#define MEMFILE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/string.h>
#include <jakson/hexdump.h>
#include <jakson/mem/block.h>

BEGIN_DECL

typedef struct memfile {
        memblock *memblock;
        offset_t pos;
        offset_t saved_pos[10];
        i8 saved_pos_ptr;
        bool bit_mode;
        size_t current_read_bit, current_write_bit, bytes_completed;
        access_mode_e mode;
        err err;
} memfile;

#define MEMFILE_PEEK(file, type)                                                                                   \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) memfile_peek(file, sizeof(type));                                                                          \
})

#define MEMFILE_READ_TYPE(file, type)                                                                              \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= sizeof(type));                                                                \
    (type *) memfile_read(file, sizeof(type));                                                                          \
})

#define MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                               \
    (const type *) MEMFILE_READ(file, how_many * sizeof(type))

#define MEMFILE_READ(file, nbytes)                                                                                 \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= nbytes);                                                                      \
    memfile_read(file, nbytes);                                                                                        \
})

#define memfile_tell(file)                                                                                             \
({                                                                                                                     \
    offset_t offset = 0;                                                                                               \
    memfile_get_offset(&offset, file);                                                                                 \
    offset;                                                                                                            \
})

bool memfile_open(memfile *file, memblock *block, access_mode_e mode);
bool memfile_clone(memfile *dst, memfile *src);

bool memfile_seek(memfile *file, offset_t pos);
bool memfile_seek_from_here(memfile *file, signed_offset_t where);
bool memfile_rewind(memfile *file);
bool memfile_grow(memfile *file_in, size_t grow_by_bytes);
bool memfile_get_offset(offset_t *pos, const memfile *file);
size_t memfile_size(memfile *file);
bool memfile_cut(memfile *file, size_t how_many_bytes);
size_t memfile_remain_size(memfile *file);
bool memfile_shrink(memfile *file);
const char *memfile_read(memfile *file, offset_t nbytes);
u8 memfile_read_byte(memfile *file);
u8 memfile_peek_byte(memfile *file);
u64 memfile_read_u64(memfile *file);
i64 memfile_read_i64(memfile *file);
bool memfile_skip(memfile *file, signed_offset_t nbytes);
#define MEMFILE_SKIP_BYTE(file) memfile_skip(file, sizeof(u8))
const char *memfile_peek(memfile *file, offset_t nbytes);
bool memfile_write_byte(memfile *file, u8 data);
bool memfile_write(memfile *file, const void *data, offset_t nbytes);
bool memfile_write_zero(memfile *file, size_t how_many);
bool memfile_begin_bit_mode(memfile *file);
bool memfile_write_bit(memfile *file, bool flag);
bool memfile_read_bit(memfile *file);
offset_t memfile_save_position(memfile *file);
bool memfile_restore_position(memfile *file);
signed_offset_t memfile_ensure_space(memfile *memfile, u64 nbytes);
u64 memfile_read_uintvar_stream(u8 *nbytes, memfile *memfile);
bool memfile_skip_uintvar_stream(memfile *memfile);
u64 memfile_peek_uintvar_stream(u8 *nbytes, memfile *memfile);
u64 memfile_write_uintvar_stream(u64 *nbytes_moved, memfile *memfile, u64 value);
signed_offset_t memfile_update_uintvar_stream(memfile *memfile, u64 value);
bool memfile_seek_to_start(memfile *file);
bool memfile_seek_to_end(memfile *file);

/**
 * Moves the contents of the underlying memory block <code>nbytes</code> towards the end of the file.
 * The offset in the memory block from where this move is done is the current position stored in this file.
 * In case of not enough space, the underlying memory block is resized.
 */
bool memfile_inplace_insert(memfile *file, size_t nbytes);
bool memfile_inplace_remove(memfile *file, size_t nbytes_from_here);
bool memfile_end_bit_mode(size_t *num_bytes_written, memfile *file);
void *memfile_current_pos(memfile *file, offset_t nbytes);
bool memfile_hexdump(string_buffer *sb, memfile *file);
bool memfile_hexdump_printf(FILE *file, memfile *memfile);
bool memfile_hexdump_print(memfile *memfile);

END_DECL

#endif