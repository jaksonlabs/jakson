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

#include <jak_stdinc.h>
#include <jak_string.h>
#include <jak_utils_hexdump.h>
#include <jak_memblock.h>

JAK_BEGIN_DECL

struct jak_memfile {
        struct jak_memblock *memblock;
        jak_offset_t pos;
        jak_offset_t saved_pos[10];
        jak_i8 saved_pos_ptr;
        bool bit_mode;
        size_t current_read_bit, current_write_bit, bytes_completed;
        enum access_mode mode;
        jak_error err;
};

#define JAK_MEMFILE_PEEK(file, type)                                                                                   \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) memfile_peek(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE(file, type)                                                                              \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= sizeof(type));                                                                \
    (type*) memfile_read(file, sizeof(type));                                                                          \
})

#define JAK_MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                               \
    (const type *) JAK_MEMFILE_READ(file, how_many * sizeof(type))

#define JAK_MEMFILE_READ(file, nbytes)                                                                                 \
({                                                                                                                     \
    JAK_ASSERT (memfile_remain_size(file) >= nbytes);                                                                      \
    memfile_read(file, nbytes);                                                                                        \
})

#define memfile_tell(file)                                                                                             \
({                                                                                                                     \
    jak_offset_t offset = 0;                                                                                               \
    memfile_get_offset(&offset, file);                                                                                 \
    offset;                                                                                                            \
})

bool memfile_open(struct jak_memfile *file, struct jak_memblock *block, enum access_mode mode);

bool memfile_clone(struct jak_memfile *dst, struct jak_memfile *src);

bool memfile_seek(struct jak_memfile *file, jak_offset_t pos);

bool memfile_seek_from_here(struct jak_memfile *file, signed_offset_t where);

bool memfile_rewind(struct jak_memfile *file);

bool memfile_grow(struct jak_memfile *file_in, size_t grow_by_bytes);

bool memfile_get_offset(jak_offset_t *pos, const struct jak_memfile *file);

size_t memfile_size(struct jak_memfile *file);

bool memfile_cut(struct jak_memfile *file, size_t how_many_bytes);

size_t memfile_remain_size(struct jak_memfile *file);

bool memfile_shrink(struct jak_memfile *file);

const char *memfile_read(struct jak_memfile *file, jak_offset_t nbytes);

jak_u8 memfile_read_byte(struct jak_memfile *file);

jak_u8 memfile_peek_byte(struct jak_memfile *file);

jak_u64 memfile_read_u64(struct jak_memfile *file);

jak_i64 memfile_read_i64(struct jak_memfile *file);

bool memfile_skip(struct jak_memfile *file, signed_offset_t nbytes);

#define memfile_skip_byte(file) memfile_skip(file, sizeof(jak_u8))

const char *memfile_peek(struct jak_memfile *file, jak_offset_t nbytes);

bool memfile_write_byte(struct jak_memfile *file, jak_u8 data);

bool memfile_write(struct jak_memfile *file, const void *data, jak_offset_t nbytes);

bool memfile_write_zero(struct jak_memfile *file, size_t how_many);

bool memfile_begin_bit_mode(struct jak_memfile *file);

bool memfile_write_bit(struct jak_memfile *file, bool flag);

bool memfile_read_bit(struct jak_memfile *file);

jak_offset_t memfile_save_position(struct jak_memfile *file);

bool memfile_restore_position(struct jak_memfile *file);

signed_offset_t memfile_ensure_space(struct jak_memfile *memfile, jak_u64 nbytes);

jak_u64 memfile_read_uintvar_stream(jak_u8 *nbytes, struct jak_memfile *memfile);

bool memfile_skip_uintvar_stream(struct jak_memfile *memfile);

jak_u64 memfile_peek_uintvar_stream(jak_u8 *nbytes, struct jak_memfile *memfile);

jak_u64 memfile_write_uintvar_stream(jak_u64 *nbytes_moved, struct jak_memfile *memfile, jak_u64 value);

signed_offset_t memfile_update_uintvar_stream(struct jak_memfile *memfile, jak_u64 value);

bool memfile_seek_to_start(struct jak_memfile *file);

bool memfile_seek_to_end(struct jak_memfile *file);

/**
 * Moves the contents of the underlying memory block <code>nbytes</code> towards the end of the file.
 * The offset in the memory block from where this move is done is the current position stored in this file.
 * In case of not enough space, the underlying memory block is resized.
 */
bool memfile_inplace_insert(struct jak_memfile *file, size_t nbytes);

bool memfile_inplace_remove(struct jak_memfile *file, size_t nbytes_from_here);

bool memfile_end_bit_mode(size_t *num_bytes_written, struct jak_memfile *file);

void *memfile_current_pos(struct jak_memfile *file, jak_offset_t nbytes);

bool memfile_hexdump(struct jak_string *sb, struct jak_memfile *file);

bool memfile_hexdump_printf(FILE *file, struct jak_memfile *memfile);

bool memfile_hexdump_print(struct jak_memfile *memfile);

JAK_END_DECL

#endif