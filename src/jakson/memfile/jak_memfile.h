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

#ifndef JAK_MEMFILE_H
#define JAK_MEMFILE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include <jakson/std/jak_string.h>
#include <jakson/jak_utils_hexdump.h>
#include <jakson/memfile/jak_memblock.h>

JAK_BEGIN_DECL

typedef struct jak_memfile {
        jak_memblock *memblock;
        jak_offset_t pos;
        jak_offset_t saved_pos[10];
        jak_i8 saved_pos_ptr;
        bool bit_mode;
        size_t current_read_bit, current_write_bit, bytes_completed;
        jak_access_mode_e mode;
        jak_error err;
} jak_memfile;

#define JAK_MEMFILE_PEEK(file, type)                                                                                   \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) jak_memfile_peek(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE(file, type)                                                                              \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) jak_memfile_read(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                               \
    (const type *) JAK_MEMFILE_READ(file, how_many * sizeof(type))

#define JAK_MEMFILE_READ(file, nbytes)                                                                                 \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remain_size(file) >= nbytes);                                                                      \
    jak_memfile_read(file, nbytes);                                                                                        \
})

#define jak_memfile_tell(file)                                                                                             \
({                                                                                                                     \
    jak_offset_t offset = 0;                                                                                               \
    jak_memfile_get_offset(&offset, file);                                                                                 \
    offset;                                                                                                            \
})

bool jak_memfile_open(jak_memfile *file, jak_memblock *block, jak_access_mode_e mode);
bool jak_memfile_clone(jak_memfile *dst, jak_memfile *src);

bool jak_memfile_seek(jak_memfile *file, jak_offset_t pos);
bool jak_memfile_seek_from_here(jak_memfile *file, signed_offset_t where);
bool jak_memfile_rewind(jak_memfile *file);
bool jak_memfile_grow(jak_memfile *file_in, size_t grow_by_bytes);
bool jak_memfile_get_offset(jak_offset_t *pos, const jak_memfile *file);
size_t jak_memfile_size(jak_memfile *file);
bool jak_memfile_cut(jak_memfile *file, size_t how_many_bytes);
size_t jak_memfile_remain_size(jak_memfile *file);
bool jak_memfile_shrink(jak_memfile *file);
const char *jak_memfile_read(jak_memfile *file, jak_offset_t nbytes);
jak_u8 jak_memfile_read_byte(jak_memfile *file);
jak_u8 jak_memfile_peek_byte(jak_memfile *file);
jak_u64 jak_memfile_read_u64(jak_memfile *file);
jak_i64 jak_memfile_read_i64(jak_memfile *file);
bool jak_memfile_skip(jak_memfile *file, signed_offset_t nbytes);
#define JAK_MEMFILE_SKIP_BYTE(file) jak_memfile_skip(file, sizeof(jak_u8))
const char *jak_memfile_peek(jak_memfile *file, jak_offset_t nbytes);
bool jak_memfile_write_byte(jak_memfile *file, jak_u8 data);
bool jak_memfile_write(jak_memfile *file, const void *data, jak_offset_t nbytes);
bool jak_memfile_write_zero(jak_memfile *file, size_t how_many);
bool jak_memfile_begin_bit_mode(jak_memfile *file);
bool jak_memfile_write_bit(jak_memfile *file, bool flag);
bool jak_memfile_read_bit(jak_memfile *file);
jak_offset_t jak_memfile_save_position(jak_memfile *file);
bool jak_memfile_restore_position(jak_memfile *file);
signed_offset_t jak_memfile_ensure_space(jak_memfile *memfile, jak_u64 nbytes);
jak_u64 jak_memfile_read_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
bool jak_memfile_skip_uintvar_stream(jak_memfile *memfile);
jak_u64 jak_memfile_peek_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
jak_u64 jak_memfile_write_uintvar_stream(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value);
signed_offset_t jak_memfile_update_uintvar_stream(jak_memfile *memfile, jak_u64 value);
bool jak_memfile_seek_to_start(jak_memfile *file);
bool jak_memfile_seek_to_end(jak_memfile *file);

/**
 * Moves the contents of the underlying memory block <code>nbytes</code> towards the end of the file.
 * The offset in the memory block from where this move is done is the current position stored in this file.
 * In case of not enough space, the underlying memory block is resized.
 */
bool jak_memfile_inplace_insert(jak_memfile *file, size_t nbytes);
bool jak_memfile_inplace_remove(jak_memfile *file, size_t nbytes_from_here);
bool jak_memfile_end_bit_mode(size_t *num_bytes_written, jak_memfile *file);
void *jak_memfile_current_pos(jak_memfile *file, jak_offset_t nbytes);
bool jak_memfile_hexdump(jak_string *sb, jak_memfile *file);
bool jak_memfile_hexdump_printf(FILE *file, jak_memfile *memfile);
bool jak_memfile_hexdump_print(jak_memfile *memfile);

JAK_END_DECL

#endif