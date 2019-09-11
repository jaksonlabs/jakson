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
#include <jak_uintvar_stream.h>
#include <jak_uintvar_marker.h>

JAK_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//  types
// ---------------------------------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------------------------------
//  macros
// ---------------------------------------------------------------------------------------------------------------------

#define JAK_MEMFILE_PEEK(file, type)                                                                                   \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remaining_size(file) >= sizeof(type));                                                        \
    (type*) jak_memfile_peek(file, sizeof(type));                                                                      \
})

#define JAK_MEMFILE_READ_TYPE(file, type)                                                                              \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remaining_size(file) >= sizeof(type));                                                        \
    (type*) jak_memfile_read(file, sizeof(type));                                                                      \
})

#define JAK_MEMFILE_READ_TYPE_LIST(file, type, how_many)                                                               \
    (const type *) JAK_MEMFILE_READ(file, how_many * sizeof(type))

#define JAK_MEMFILE_READ(file, nbytes)                                                                                 \
({                                                                                                                     \
    JAK_ASSERT (jak_memfile_remaining_size(file) >= nbytes);                                                              \
    jak_memfile_read(file, nbytes);                                                                                    \
})

#define jak_memfile_tell(file)                                                                                         \
({                                                                                                                     \
    jak_offset_t offset = 0;                                                                                           \
    jak_memfile_get_offset(&offset, file);                                                                             \
    offset;                                                                                                            \
})

// ---------------------------------------------------------------------------------------------------------------------
//  construction / deconstruction
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_open(jak_memfile *file, jak_memblock *block, jak_access_mode_e mode);
bool jak_memfile_clone(jak_memfile *dst, jak_memfile *src);

// ---------------------------------------------------------------------------------------------------------------------
//  low-level operations
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_bit_mode_begin(jak_memfile *file);
bool jak_memfile_write_bit(jak_memfile *file, bool flag);
bool jak_memfile_read_bit(jak_memfile *file);
bool jak_memfile_bit_mode_end(size_t *num_bytes_written, jak_memfile *file);

bool jak_memfile_write(jak_memfile *file, const void *data, jak_offset_t nbytes);
bool jak_memfile_write_ntimes(jak_memfile *file, const void *data, jak_offset_t nbytes, jak_u64 n);
const char *jak_memfile_read(jak_memfile *file, jak_offset_t nbytes);
const char *jak_memfile_peek(jak_memfile *file, jak_offset_t nbytes);
void *jak_memfile_raw_data(jak_memfile *file);

bool jak_memfile_skip(jak_memfile *file, jak_signed_offset_t nbytes);
#define JAK_MEMFILE_SKIP_BYTE(file) jak_memfile_skip(file, sizeof(jak_u8))

bool jak_memfile_write_zero(jak_memfile *file, size_t how_many);

bool jak_memfile_memcpy(jak_memfile *dst_file, jak_offset_t dst_off, jak_memfile *src_file, jak_offset_t src_off,
        jak_u64 nbytes);

// ---------------------------------------------------------------------------------------------------------------------
//  convenient operations
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_write_byte(jak_memfile *file, jak_u8 data);
jak_u8 jak_memfile_read_byte(jak_memfile *file);
jak_u8 jak_memfile_peek_byte(jak_memfile *file);
jak_u64 jak_memfile_read_u64(jak_memfile *file);
jak_i64 jak_memfile_read_i64(jak_memfile *file);

void *jak_memfile_raw_data_at(jak_memfile *file, jak_u64 offset);
void *jak_memfile_raw_data_from_here(jak_memfile *file);

bool jak_memfile_write_zero_at(jak_memfile *file, jak_u64 offset, size_t how_many);

jak_uintvar_marker_t *jak_memfile_cast_as_uintvar_marker(jak_memfile *file);

/**
 * Returns the offset and size (in byte) of the tail buffer in this memfile.
 * The tail buffer is the remaining reserved space beyond the last written byte in this memfile and the
 * last available byte in the underyling memblock. In simpler words, the tail buffer is the capacity in the
 * memblock that is not yet used by the memfile.
 *
 * @param buffer_start returns the offset in this memfile where the buffer starts
 * @param size returns the current buffer size
 * @param file the input memfile
 * @return <b>true</b> in case of success, otherwise <b>false</b>. In case of no success, the error member in
 *                     <code>file</code> contains the reason of failure.
 */
bool jak_memfile_get_tail_buffer(jak_offset_t *buffer_start, jak_u64 *size, jak_memfile *file);

bool jak_memfile_force_tail_buffer_start_at(jak_memfile *file, jak_offset_t buffer_start);

// ---------------------------------------------------------------------------------------------------------------------
//  variable-length unsigned integers (stream based)
// ---------------------------------------------------------------------------------------------------------------------

jak_u64 jak_memfile_read_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
bool jak_memfile_skip_uintvar_stream(jak_memfile *memfile);
jak_u64 jak_memfile_peek_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile);
jak_u64 jak_memfile_write_uintvar_stream(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value);
jak_signed_offset_t jak_memfile_update_uintvar_stream(jak_memfile *memfile, jak_u64 value);

// ---------------------------------------------------------------------------------------------------------------------
//  variable-length unsigned integers (marker based)
// ---------------------------------------------------------------------------------------------------------------------

jak_u64 jak_memfile_read_uintvar_marker(jak_u8 *nbytes, jak_memfile *memfile);
bool jak_memfile_skip_uintvar_marker(jak_memfile *memfile);
jak_u64 jak_memfile_peek_uintvar_marker(jak_u8 *nbytes, jak_memfile *memfile);
jak_u64 jak_memfile_write_uintvar_marker(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value);
jak_signed_offset_t jak_memfile_update_uintvar_marker(jak_memfile *memfile, jak_u64 value);

jak_u8 jak_memfile_sizeof_uintvar_marker(jak_memfile *memfile);

jak_uintvar_marker_marker_type_e jak_memfile_peek_uintvar_marker_type(jak_memfile *memfile);
jak_uintvar_marker_marker_type_e jak_memfile_read_uintvar_marker_type(jak_memfile *memfile);

// ---------------------------------------------------------------------------------------------------------------------
//  variable-length unsigned integer columns (marker based)
// ---------------------------------------------------------------------------------------------------------------------

/*
 * A variable-length integer column (uintvar_column) is an array of a fix number of elements where each element
 * has the same variable-length integer type encoded using marker-based encoding. The encoding is as follows
 *      [N-type][N][elem-type][e1][e2]...[eN]
 * First a (marker-based) variable-length element number N is written by writing first the int-type (here N-type),
 * and then the value using that int-type. Then, the contents are written. For this, first the int-type (here elem-type)
 * for all elements e1, e2,..., eN is written. Finally, all elements N of type int-type are written.
 *
 * For creation, the list e1,e2,...,eN is initialized with N zeros, leading to
 *      [N-type][N][c][0][0]...[0]
 * When updating (setting) a particular element, the column may be rewritten in-place, and elem-type may change.
 */
bool jak_memfile_uintvar_column_create(jak_memfile *memfile, jak_u64 num_elems);
bool jak_memfile_uintvar_column_create_wtype(jak_memfile *memfile, jak_uintvar_marker_marker_type_e type, jak_u64 num_elems);
void *jak_memfile_uintvar_column_raw_payload(jak_uintvar_marker_marker_type_e *elem_type, jak_u64 *num_elems, jak_memfile *memfile);
bool jak_memfile_uintvar_column_set(jak_memfile *memfile, jak_u64 idx, jak_u64 value);
bool jak_memfile_uintvar_column_skip(jak_memfile *memfile);
jak_u64 jak_memfile_uintvar_column_size_header(jak_uintvar_marker_marker_type_e *value_type, jak_u64 *num_elems, jak_offset_t *payload_start_off, jak_memfile *memfile);
jak_u64 jak_memfile_uintvar_column_size_payload(jak_offset_t *column_end_off, jak_memfile *memfile);
jak_u64 jak_memfile_uintvar_column_size_total(jak_memfile *memfile);
jak_uintvar_marker_marker_type_e jak_memfile_uintvar_column_get_counter_type(jak_memfile *memfile);
jak_uintvar_marker_marker_type_e jak_memfile_uintvar_column_get_value_type(jak_memfile *memfile);
bool jak_memfile_uintvar_column_is_u8(jak_memfile *memfile);
bool jak_memfile_uintvar_column_is_u16(jak_memfile *memfile);
bool jak_memfile_uintvar_column_is_u32(jak_memfile *memfile);
bool jak_memfile_uintvar_column_is_u64(jak_memfile *memfile);
const jak_u8 *jak_memfile_uintvar_column_read_u8(jak_u64 *num_elems, jak_memfile *memfile);
const jak_u16 *jak_memfile_uintvar_column_read_u16(jak_u64 *num_elems, jak_memfile *memfile);
const jak_u32 *jak_memfile_uintvar_column_read_u32(jak_u64 *num_elems, jak_memfile *memfile);
const jak_u64 *jak_memfile_uintvar_column_read_u64(jak_u64 *num_elems, jak_memfile *memfile);
jak_u8 jak_memfile_uintvar_column_u8_at(jak_memfile *memfile, jak_u64 pos);
jak_u16 jak_memfile_uintvar_column_u16_at(jak_memfile *memfile, jak_u64 pos);
jak_u32 jak_memfile_uintvar_column_u32_at(jak_memfile *memfile, jak_u64 pos);
jak_u64 jak_memfile_uintvar_column_u64_at(jak_memfile *memfile, jak_u64 pos);
bool jak_memfile_uintvar_column_to_str(jak_string *str, jak_memfile *memfile);

// ---------------------------------------------------------------------------------------------------------------------
//  positioning
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_seek(jak_memfile *file, jak_offset_t pos);
bool jak_memfile_seek_to_start(jak_memfile *file);
bool jak_memfile_seek_to_end(jak_memfile *file);
bool jak_memfile_seek_relative(jak_memfile *file, jak_signed_offset_t where);
bool jak_memfile_rewind(jak_memfile *file);
jak_offset_t jak_memfile_save_position(jak_memfile *file);
bool jak_memfile_restore_position(jak_memfile *file);

// ---------------------------------------------------------------------------------------------------------------------
//  size manipulation
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_grow(jak_memfile *file_in, size_t grow_by_bytes);
bool jak_memfile_cut(jak_memfile *file, size_t how_many_bytes);
bool jak_memfile_shrink(jak_memfile *file);
jak_signed_offset_t jak_memfile_ensure_space(jak_memfile *memfile, jak_u64 nbytes);

/**
 * Moves the contents of the underlying memory block <code>nbytes</code> towards the end of the file.
 * The offset in the memory block from where this move is done is the current position stored in this file.
 * In case of not enough space, the underlying memory block is resized.
 */
bool jak_memfile_inplace_insert(jak_memfile *file, size_t nbytes);
bool jak_memfile_inplace_remove(jak_memfile *file, size_t nbytes_from_here);

// ---------------------------------------------------------------------------------------------------------------------
//  information
// ---------------------------------------------------------------------------------------------------------------------

size_t jak_memfile_remaining_size(jak_memfile *file);
bool jak_memfile_get_offset(jak_offset_t *pos, const jak_memfile *file);
size_t jak_memfile_size(jak_memfile *file);
void *jak_memfile_current_pos(jak_memfile *file, jak_offset_t nbytes);

// ---------------------------------------------------------------------------------------------------------------------
//  diagnostics
// ---------------------------------------------------------------------------------------------------------------------

bool jak_memfile_hexdump(jak_string *sb, jak_memfile *file);
bool jak_memfile_hexdump_printf(FILE *file, jak_memfile *memfile);
bool jak_memfile_hexdump_print(jak_memfile *memfile);
JAK_DEFINE_GET_ERROR_FUNCTION(memfile, jak_memfile, file)

JAK_END_DECL

#endif