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

#include <jak_memfile.h>

bool jak_memfile_open(jak_memfile *file, jak_memblock *block, jak_access_mode_e mode)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(block)
        JAK_ZERO_MEMORY(file, sizeof(jak_memfile))
        file->memblock = block;
        file->pos = 0;
        file->bit_mode = false;
        file->mode = mode;
        file->saved_pos_ptr = 0;
        jak_error_init(&file->err);
        return true;
}

bool jak_memfile_clone(jak_memfile *dst, jak_memfile *src)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        jak_memfile_open(dst, src->memblock, src->mode);
        jak_memfile_seek(dst, jak_memfile_tell(src));
        dst->bit_mode = src->bit_mode;
        dst->saved_pos_ptr = src->saved_pos_ptr;
        jak_error_cpy(&dst->err, &src->err);
        memcpy(&dst->saved_pos, &src->saved_pos, JAK_ARRAY_LENGTH(src->saved_pos));
        return true;
}

bool jak_memfile_seek(jak_memfile *file, jak_offset_t pos)
{
        JAK_ERROR_IF_NULL(file)
        jak_offset_t file_size = 0;
        jak_memblock_size(&file_size, file->memblock);
        if (JAK_UNLIKELY(pos >= file_size)) {
                if (file->mode == JAK_READ_WRITE) {
                        jak_offset_t new_size = pos + 1;
                        jak_memblock_resize(file->memblock, new_size);
                } else {
                        JAK_ERROR(&file->err, JAK_ERR_MEMSTATE)
                        return false;
                }
        }
        file->pos = pos;
        return true;
}

bool jak_memfile_seek_relative(jak_memfile *file, jak_signed_offset_t where)
{
        jak_offset_t now = jak_memfile_tell(file);
        jak_offset_t then = now + where;
        return jak_memfile_seek(file, then);
}

bool jak_memfile_rewind(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        file->pos = 0;
        return true;
}

bool jak_memfile_grow(jak_memfile *file_in, size_t grow_by_bytes)
{
        JAK_ERROR_IF_NULL(file_in)
        if (JAK_LIKELY(grow_by_bytes > 0)) {
                jak_offset_t block_size = 0;
                jak_memblock_size(&block_size, file_in->memblock);
                jak_memblock_resize(file_in->memblock, (block_size + grow_by_bytes));
        }
        return true;
}

bool jak_memfile_get_offset(jak_offset_t *pos, const jak_memfile *file)
{
        JAK_ERROR_IF_NULL(pos)
        JAK_ERROR_IF_NULL(file)
        *pos = file->pos;
        return true;
}

size_t jak_memfile_size(jak_memfile *file)
{
        if (!file || !file->memblock) {
                return 0;
        } else {
                jak_u64 size;
                jak_memblock_size(&size, file->memblock);
                return size;
        }
}

bool jak_memfile_cut(jak_memfile *file, size_t how_many_bytes)
{
        JAK_ERROR_IF_NULL(file);
        jak_offset_t block_size = 0;
        jak_memblock_size(&block_size, file->memblock);

        if (how_many_bytes > 0 && block_size > how_many_bytes) {
                size_t new_block_size = block_size - how_many_bytes;
                jak_memblock_resize(file->memblock, new_block_size);
                file->pos = JAK_MIN(file->pos, new_block_size);
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_ILLEGALARG);
                return false;
        }
}

size_t jak_memfile_remaining_size(jak_memfile *file)
{
        JAK_ASSERT(file->pos <= jak_memfile_size(file));
        return jak_memfile_size(file) - file->pos;
}

bool jak_memfile_shrink(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (file->mode == JAK_READ_WRITE) {
                int status = jak_memblock_shrink(file->memblock);
                jak_u64 size;
                jak_memblock_size(&size, file->memblock);
                JAK_ASSERT(size == file->pos);
                return status;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_WRITEPROT)
                return false;
        }
}

const char *jak_memfile_read(jak_memfile *file, jak_offset_t nbytes)
{
        const char *result = jak_memfile_peek(file, nbytes);
        file->pos += nbytes;
        return result;
}

jak_u8 jak_memfile_read_byte(jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_u8);
}

jak_u8 jak_memfile_peek_byte(jak_memfile *file)
{
        return *JAK_MEMFILE_PEEK(file, jak_u8);
}

jak_u64 jak_memfile_read_u64(jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_u64);
}

jak_i64 jak_memfile_read_i64(jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_i64);
}

void *jak_memfile_raw_data_at(jak_memfile *file, jak_u64 offset)
{
        if (JAK_LIKELY(offset <= jak_memblock_last_used_byte(file->memblock))) {
                return jak_memfile_raw_data(file) + offset;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_OUTOFBOUNDS)
                return NULL;
        }
}

void *jak_memfile_raw_data_from_here(jak_memfile *file)
{
        return jak_memfile_raw_data_at(file, jak_memfile_tell(file));
}

bool jak_memfile_write_zero_at(jak_memfile *file, jak_u64 offset, size_t how_many)
{
        if (JAK_LIKELY(offset <= jak_memblock_last_used_byte(file->memblock))) {
                jak_memfile_save_position(file);
                jak_memfile_seek(file, offset);
                bool ret = jak_memfile_write_zero(file, how_many);
                jak_memfile_restore_position(file);
                return ret;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_OUTOFBOUNDS)
                return false;
        }
}

jak_uintvar_marker_t *jak_memfile_cast_as_uintvar_marker(jak_memfile *file)
{
        return (jak_uintvar_marker_t *) jak_memfile_peek(file, sizeof(jak_uintvar_marker_t));
}

bool jak_memfile_skip(jak_memfile *file, jak_signed_offset_t nbytes)
{
        jak_offset_t required_size = file->pos + nbytes;
        file->pos += nbytes;
        jak_offset_t file_size = 0;
        jak_memblock_size(&file_size, file->memblock);

        if (JAK_UNLIKELY(required_size >= file_size)) {
                if (file->mode == JAK_READ_WRITE) {
                        jak_memblock_resize(file->memblock, required_size * 1.7f);
                } else {
                        JAK_ERROR(&file->err, JAK_ERR_WRITEPROT);
                        return false;
                }
        }
        jak_memfile_update_last_byte(file->memblock, file->pos);
        JAK_ASSERT(file->pos < jak_memfile_size(file));
        return true;
}

const char *jak_memfile_peek(jak_memfile *file, jak_offset_t nbytes)
{
        jak_offset_t file_size = 0;
        jak_memblock_size(&file_size, file->memblock);
        if (JAK_UNLIKELY(file->pos + nbytes > file_size)) {
                JAK_ERROR(&file->err, JAK_ERR_READOUTOFBOUNDS);
                return NULL;
        } else {
                const char *result = jak_memblock_raw_data(file->memblock) + file->pos;
                return result;
        }
}

void *jak_memfile_raw_data(jak_memfile *file)
{
        if (JAK_LIKELY(file != NULL)) {
                return (void *) jak_memblock_raw_data(file->memblock);
        } else {
                JAK_ERROR(&file->err, JAK_ERR_NULLPTR)
                return NULL;
        }
}

bool jak_memfile_write_byte(jak_memfile *file, jak_u8 data)
{
        return jak_memfile_write(file, &data, sizeof(jak_u8));
}

bool jak_memfile_write(jak_memfile *file, const void *data, jak_offset_t nbytes)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(data)
        if (file->mode == JAK_READ_WRITE) {
                if (JAK_LIKELY(nbytes != 0)) {
                        jak_offset_t file_size = 0;
                        jak_memblock_size(&file_size, file->memblock);
                        jak_offset_t required_size = file->pos + nbytes;
                        if (JAK_UNLIKELY(required_size >= file_size)) {
                                jak_memblock_resize(file->memblock, required_size * 1.7f);
                        }

                        if (JAK_UNLIKELY(!jak_memblock_write(file->memblock, file->pos, data, nbytes))) {
                                return false;
                        }
                        file->pos += nbytes;
                }
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_WRITEPROT);
                return false;
        }
}

bool jak_memfile_write_ntimes(jak_memfile *file, const void *data, jak_offset_t nbytes, jak_u64 n)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(data)
        bool status = true;
        for (jak_u64 i = 0; status && i < n; i++) {
                status &= jak_memfile_write(file, data, nbytes);
        }
        return true;
}

bool jak_memfile_write_zero(jak_memfile *file, size_t how_many)
{
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(how_many);
        char empty = 0;
        while (how_many--) {
                jak_memfile_write(file, &empty, sizeof(char));
        }
        return true;
}

bool jak_memfile_memcpy(jak_memfile *dst_file, jak_offset_t dst_off, jak_memfile *src_file, jak_offset_t src_off,
        jak_u64 nbytes)
{
        JAK_ERROR_IF_NULL(dst_file);
        JAK_ERROR_IF_NULL(src_file);
        JAK_ERROR_IF(dst_off >= jak_memblock_last_used_byte(dst_file->memblock) ||
                             src_off >= jak_memblock_last_used_byte(src_file->memblock) ||
                             dst_off + nbytes >= jak_memblock_last_used_byte(dst_file->memblock) ||
                             src_off + nbytes >= jak_memblock_last_used_byte(src_file->memblock),
                             &src_file->err, JAK_ERR_OUTOFBOUNDS)
        void *src_ptr = jak_memfile_raw_data_at(src_file, src_off);
        void *dst_ptr = jak_memfile_raw_data_at(dst_file, dst_off);
        if (JAK_UNLIKELY((src_file->memblock == dst_file->memblock) &&
                                 (dst_off == src_off || (dst_off < src_off && dst_off + nbytes >= src_off) ||
                                 (src_off < dst_off && src_off + nbytes >= dst_off)))) {
                memmove(dst_ptr, src_ptr, nbytes);
        } else {
                memcpy(dst_ptr, src_ptr, nbytes);
        }
        return true;
}

bool jak_memfile_bit_mode_begin(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (file->mode == JAK_READ_WRITE) {
                file->bit_mode = true;
                file->current_read_bit = file->current_write_bit = file->bytes_completed = 0;
                file->bytes_completed = 0;
                jak_offset_t offset;
                char empty = '\0';
                jak_memfile_get_offset(&offset, file);
                jak_memfile_write(file, &empty, sizeof(char));
                jak_memfile_seek(file, offset);
        } else {
                JAK_ERROR(&file->err, JAK_ERR_WRITEPROT);
                return false;
        }

        return true;
}

bool jak_memfile_write_bit(jak_memfile *file, bool flag)
{
        JAK_ERROR_IF_NULL(file);
        file->current_read_bit = 0;

        if (file->bit_mode) {
                if (file->current_write_bit < 8) {
                        jak_offset_t offset;
                        jak_memfile_get_offset(&offset, file);
                        char byte = *jak_memfile_read(file, sizeof(char));
                        char mask = 1 << file->current_write_bit;
                        if (flag) {
                                JAK_SET_BITS(byte, mask);
                        } else {
                                JAK_UNSET_BITS(byte, mask);
                        }
                        jak_memfile_seek(file, offset);
                        jak_memfile_write(file, &byte, sizeof(char));
                        jak_memfile_seek(file, offset);
                        file->current_write_bit++;
                } else {
                        file->current_write_bit = 0;
                        file->bytes_completed++;
                        char empty = '\0';
                        jak_offset_t off;
                        jak_memfile_skip(file, 1);
                        jak_memfile_get_offset(&off, file);
                        jak_memfile_write(file, &empty, sizeof(char));
                        jak_memfile_seek(file, off);

                        return jak_memfile_write_bit(file, flag);
                }
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_NOBITMODE);
                return false;
        }
}

bool jak_memfile_read_bit(jak_memfile *file)
{
        if (!file) {
                return false;
        }

        file->current_write_bit = 0;

        if (file->bit_mode) {
                if (file->current_read_bit < 8) {
                        jak_offset_t offset;
                        jak_memfile_get_offset(&offset, file);

                        char mask = 1 << file->current_read_bit;
                        char byte = *jak_memfile_read(file, sizeof(char));
                        jak_memfile_seek(file, offset);
                        bool result = ((byte & mask) >> file->current_read_bit) == true;
                        file->current_read_bit++;
                        return result;
                } else {
                        file->current_read_bit = 0;
                        jak_memfile_skip(file, sizeof(char));
                        return jak_memfile_read_bit(file);
                }
        } else {
                JAK_ERROR(&file->err, JAK_ERR_NOBITMODE);
                return false;
        }
}

jak_offset_t jak_memfile_save_position(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        jak_offset_t pos = jak_memfile_tell(file);
        if (JAK_LIKELY(file->saved_pos_ptr < (jak_i8) (JAK_ARRAY_LENGTH(file->saved_pos)))) {
                file->saved_pos[file->saved_pos_ptr++] = pos;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_STACK_OVERFLOW)
        }
        return pos;
}

bool jak_memfile_restore_position(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (JAK_LIKELY(file->saved_pos_ptr >= 0)) {
                jak_offset_t pos = file->saved_pos[--file->saved_pos_ptr];
                jak_memfile_seek(file, pos);
                return true;
        } else {
                JAK_ERROR(&file->err, JAK_ERR_STACK_UNDERFLOW)
                return false;
        }
}

jak_signed_offset_t jak_memfile_ensure_space(jak_memfile *memfile, jak_u64 nbytes)
{
        JAK_ERROR_IF_NULL(memfile)

        JAK_DECLARE_AND_INIT(jak_offset_t, block_size);

        jak_memblock_size(&block_size, memfile->memblock);
        JAK_ASSERT(memfile->pos < block_size);
        size_t diff = block_size - memfile->pos;
        if (diff < nbytes) {
                jak_memfile_grow(memfile, nbytes - diff);
        }

        jak_memfile_save_position(memfile);
        jak_offset_t current_off = jak_memfile_tell(memfile);
        jak_signed_offset_t shift = 0;
        for (jak_u64 i = 0; i < nbytes; i++) {
                char c = *jak_memfile_read(memfile, 1);
                if (JAK_UNLIKELY(c != 0)) {
                        /* not enough space; enlarge container */
                        jak_memfile_seek(memfile, current_off);
                        jak_memfile_inplace_insert(memfile, nbytes - i);
                        shift += nbytes - i;
                        break;
                }
        }
        jak_memfile_restore_position(memfile);

        return shift;
}

static void __jak_uintvar_move_memory(jak_u8 bytes_used_now, jak_u8 bytes_used_then, jak_memfile *memfile)
{
        if (bytes_used_now < bytes_used_then) {
                jak_u8 inc = bytes_used_then - bytes_used_now;
                jak_memfile_inplace_insert(memfile, inc);
        } else if (bytes_used_now > bytes_used_then) {
                jak_u8 dec = bytes_used_now - bytes_used_then;
                jak_memfile_inplace_remove(memfile, dec);
        }
}

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
bool jak_memfile_get_tail_buffer(jak_offset_t *buffer_start, jak_u64 *size, jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        jak_offset_t last_byte = jak_memblock_last_used_byte(file->memblock);
        jak_offset_t block_size;
        jak_memblock_size(&block_size, file->memblock);
        assert(last_byte < block_size);
        JAK_OPTIONAL_SET(buffer_start, last_byte);
        JAK_OPTIONAL_SET(size, (block_size - last_byte));
        return true;
}

bool jak_memfile_force_tail_buffer_start_at(jak_memfile *file, jak_offset_t buffer_start)
{
        JAK_ERROR_IF_NULL(file)
        jak_u64 block_size;
        jak_memblock_size(&block_size, file->memblock);
        JAK_ERROR_IF(buffer_start >= block_size, &file->err, JAK_ERR_OUTOFBOUNDS);
        return jak_memblock_force_set_last_used_byte(file->memblock, buffer_start);
}

jak_u64 jak_memfile_read_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile)
{
        jak_u8 nbytes_read;
        jak_u64 result = jak_uintvar_stream_read(&nbytes_read, (jak_uintvar_stream_t) jak_memfile_peek(memfile, sizeof(char)));
        jak_memfile_skip(memfile, nbytes_read);
        JAK_OPTIONAL_SET(nbytes, nbytes_read);
        return result;
}

bool jak_memfile_skip_uintvar_stream(jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile)
        jak_memfile_read_uintvar_stream(NULL, memfile);
        return true;
}

jak_u64 jak_memfile_peek_uintvar_stream(jak_u8 *nbytes, jak_memfile *memfile)
{
        jak_memfile_save_position(memfile);
        jak_u64 result = jak_memfile_read_uintvar_stream(nbytes, memfile);
        jak_memfile_restore_position(memfile);
        return result;
}

jak_u64 jak_memfile_write_uintvar_stream(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value)
{
        jak_u8 required_blocks = JAK_UINTVAR_STREAM_REQUIRED_BLOCKS(value);
        jak_signed_offset_t shift = jak_memfile_ensure_space(memfile, required_blocks);
        jak_uintvar_stream_t dst = (jak_uintvar_stream_t) jak_memfile_peek(memfile, sizeof(char));
        jak_uintvar_stream_write(dst, value);
        jak_memfile_skip(memfile, required_blocks);
        JAK_OPTIONAL_SET(nbytes_moved, shift);
        return required_blocks;
}

jak_signed_offset_t jak_memfile_update_uintvar_stream(jak_memfile *memfile, jak_u64 value)
{
        JAK_ERROR_IF_NULL(memfile);

        jak_u8 bytes_used_now, bytes_used_then;

        jak_memfile_peek_uintvar_stream(&bytes_used_now, memfile);
        bytes_used_then = JAK_UINTVAR_STREAM_REQUIRED_BLOCKS(value);

        __jak_uintvar_move_memory(bytes_used_now, bytes_used_then, memfile);

        jak_uintvar_stream_t dst = (jak_uintvar_stream_t) jak_memfile_peek(memfile, sizeof(char));
        jak_u8 required_blocks = jak_uintvar_stream_write(dst, value);
        jak_memfile_skip(memfile, required_blocks);

        return bytes_used_then - bytes_used_now;
}

jak_u64 jak_memfile_read_uintvar_marker(jak_u8 *nbytes, jak_memfile *memfile)
{
        jak_u8 nbytes_read;
        jak_u64 result = jak_uintvar_marker_read(&nbytes_read, (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char)));
        jak_memfile_skip(memfile, nbytes_read);
        JAK_OPTIONAL_SET(nbytes, nbytes_read);
        return result;
}

bool jak_memfile_skip_uintvar_marker(jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile);
        jak_memfile_read_uintvar_marker(NULL, memfile);
        return true;
}

jak_u64 jak_memfile_peek_uintvar_marker(jak_u8 *nbytes, jak_memfile *memfile)
{
        jak_memfile_save_position(memfile);
        jak_u64 result = jak_memfile_read_uintvar_marker(nbytes, memfile);
        jak_memfile_restore_position(memfile);
        return result;
}

jak_uintvar_marker_marker_type_e jak_memfile_peek_uintvar_marker_type(jak_memfile *memfile)
{
        jak_uintvar_marker_t src = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
        return jak_uintvar_marker_peek_marker(src);
}

jak_uintvar_marker_marker_type_e jak_memfile_read_uintvar_marker_type(jak_memfile *memfile)
{
        jak_uintvar_marker_marker_type_e ret = jak_memfile_peek_uintvar_marker_type(memfile);
        jak_memfile_skip(memfile, JAK_UINTVAR_MARKER_BYTES_NEEDED_FOR_MARKER());
        return ret;
}

jak_u64 jak_memfile_write_uintvar_marker(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value)
{
        jak_u8 num_bytes_needed = jak_uintvar_marker_bytes_needed_complete(value);
        jak_signed_offset_t shift = jak_memfile_ensure_space(memfile, num_bytes_needed);
        jak_uintvar_marker_t dst = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
        jak_uintvar_marker_write(dst, value);
        jak_memfile_skip(memfile, num_bytes_needed);
        JAK_OPTIONAL_SET(nbytes_moved, shift);
        return num_bytes_needed;
}

jak_signed_offset_t jak_memfile_update_uintvar_marker(jak_memfile *memfile, jak_u64 value)
{
        JAK_ERROR_IF_NULL(memfile);

        jak_u8 bytes_used_now, bytes_used_then;

        jak_memfile_peek_uintvar_marker(&bytes_used_now, memfile);
        bytes_used_then = jak_uintvar_marker_bytes_needed_complete(value);

        __jak_uintvar_move_memory(bytes_used_now, bytes_used_then, memfile);

        jak_uintvar_marker_t dst = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
        jak_u8 required_bytes = jak_uintvar_marker_write(dst, value);
        jak_memfile_skip(memfile, required_bytes + sizeof(jak_marker_t));

        return bytes_used_then - bytes_used_now;
}

jak_u8 jak_memfile_sizeof_uintvar_marker(jak_memfile *memfile)
{
        if (JAK_LIKELY(memfile != NULL)) {
                jak_uintvar_marker_t uintvar = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
                return jak_uintvar_marker_sizeof(uintvar);
        } else {
                JAK_ERROR(&memfile->err, JAK_ERR_NULLPTR);
                return false;
        }
}

bool jak_memfile_uintvar_column_create(jak_memfile *memfile, jak_u64 num_elems)
{
        return jak_memfile_uintvar_column_create_wtype(memfile, JAK_UINTVAR_8, num_elems);
}

bool jak_memfile_uintvar_column_create_wtype(jak_memfile *memfile, jak_uintvar_marker_marker_type_e type, jak_u64 num_elems)
{
        JAK_ERROR_IF_NULL(memfile)

        jak_memfile_save_position(memfile);

        /* element counter */
        jak_memfile_write_uintvar_marker(NULL, memfile, num_elems);

        /* element list */
        jak_u8 elem_type_marker_size = sizeof(jak_marker_t);
        jak_u64 elem_list_size = num_elems * jak_uintvar_marker_bytes_needed_for_value_by_type(type);
        jak_memfile_ensure_space(memfile, elem_type_marker_size + elem_list_size);

        jak_u8 zero = 0;
        void *raw_data = (void *) jak_memfile_raw_data_from_here(memfile);
        jak_uintvar_marker_write_marker(raw_data, type);
        jak_memfile_skip(memfile, sizeof(jak_marker_t));
        jak_memfile_write_ntimes(memfile, &zero, sizeof(jak_u8), elem_list_size);

        jak_memfile_restore_position(memfile);

        return true;
}

void *jak_memfile_uintvar_column_raw_payload(jak_uintvar_marker_marker_type_e *elem_type, jak_u64 *num_elems, jak_memfile *memfile)
{
        if (JAK_LIKELY(memfile != NULL)) {
                jak_memfile_save_position(memfile);
                jak_u64 num = jak_memfile_read_uintvar_marker(NULL, memfile);
                jak_uintvar_marker_marker_type_e type = jak_memfile_read_uintvar_marker_type(memfile);
                JAK_OPTIONAL_SET(elem_type, type)
                JAK_OPTIONAL_SET(num_elems, num)
                void *ret = jak_memfile_raw_data_from_here(memfile);
                jak_memfile_restore_position(memfile);
                return ret;
        } else {
                JAK_ERROR(&memfile->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

static bool __jak_uintvar_column_copy_contents(jak_memfile *dst_memfile, jak_offset_t column_dst, jak_memfile *src_memfile, jak_offset_t column_src)
{
        assert(dst_memfile);
        assert(src_memfile);
        assert(dst_memfile->memblock != src_memfile->memblock || (column_dst < jak_memblock_last_used_byte(src_memfile->memblock)));
        assert(dst_memfile->memblock != src_memfile->memblock || (column_src < jak_memblock_last_used_byte(src_memfile->memblock)));

        jak_uintvar_marker_marker_type_e src_elem_type, dst_elem_type;
        jak_u64 src_num_elems, dst_num_elems;
        jak_u8 src_elem_size, dst_elem_size;
        const void *src_data;
        void *dst_data;

        jak_memfile_save_position(dst_memfile);
        jak_memfile_save_position(src_memfile);

        jak_memfile_seek(src_memfile, column_src);
        src_data = jak_memfile_uintvar_column_raw_payload(&src_elem_type, &src_num_elems, src_memfile);

        jak_memfile_seek(dst_memfile, column_dst);
        dst_data = jak_memfile_uintvar_column_raw_payload(&dst_elem_type, &dst_num_elems, dst_memfile);

        src_elem_size = jak_uintvar_marker_bytes_needed_for_value_by_type(src_elem_type);
        dst_elem_size = jak_uintvar_marker_bytes_needed_for_value_by_type(dst_elem_type);

        assert(dst_num_elems >= src_num_elems);

        JAK_DEBUG_ONLY_BLOCK(
                if (dst_memfile->memblock == src_memfile->memblock) {
                        /* column memory areas must not overlap */
                        jak_memfile_save_position(src_memfile);
                        jak_memfile_save_position(dst_memfile);
                        jak_memfile_seek(src_memfile, column_src);
                        jak_u64 src_column_size = jak_memfile_uintvar_column_size_total(src_memfile);
                        jak_memfile_seek(dst_memfile, column_dst);
                        jak_u64 dst_column_size = jak_memfile_uintvar_column_size_total(dst_memfile);
                        bool err_condition = column_src == column_dst ||
                                             (column_src < column_dst && column_src + src_column_size >= column_dst) ||
                                             (column_dst < column_src && column_dst + dst_column_size >= column_src);
                        jak_memfile_restore_position(src_memfile);
                        jak_memfile_restore_position(dst_memfile);
                        if(err_condition) {
                                jak_memfile_restore_position(src_memfile);
                                jak_memfile_restore_position(dst_memfile);
                                JAK_ERROR(&src_memfile->err, JAK_ERR_OVERLAP)
                                return false;
                        }
                }
        )

        if (src_elem_type == dst_elem_type) {
                assert(src_elem_size == dst_elem_size);
                memcpy(dst_data, src_data, src_num_elems * src_elem_size);
        } else {
                JAK_ZERO_MEMORY(dst_data, src_num_elems * dst_elem_size)
                for (jak_u64 i = 0; i < src_num_elems; i++) {
                        const void *src_elem = src_data + (i * src_elem_size);
                        void *dst_elem = dst_data + (i * dst_elem_size);
                        memcpy(dst_elem, src_elem, src_elem_size);
                }
        }

        jak_memfile_restore_position(src_memfile);
        jak_memfile_restore_position(dst_memfile);
        return true;
}

static jak_u64 __jak_uintvar_column_create_and_insert(jak_memfile *dst_memfile, jak_offset_t dst_offset,
                                        jak_uintvar_marker_marker_type_e type, jak_u64 num_elems,
                                                      jak_memfile *src_memfile, jak_offset_t src_offset)
{
        jak_memfile_seek(dst_memfile, dst_offset);
        jak_memfile_uintvar_column_create_wtype(dst_memfile, type, num_elems);
        jak_u64 new_column_size = jak_memfile_uintvar_column_size_total(dst_memfile);
        __jak_uintvar_column_copy_contents(dst_memfile, dst_offset, src_memfile, src_offset);
        return new_column_size;
}

static void __jak_uintvar_column_overwrite(jak_memfile *dst_memfile, jak_offset_t dst_offset,
                                           jak_memfile *src_memfile, jak_offset_t src_offset,
                                           jak_u64 required_diff, jak_u64 new_column_size)
{
        jak_memfile_save_position(dst_memfile);
        /* remove the old column by overwriting with the new column*/
        jak_memfile_seek(dst_memfile, dst_offset);
        jak_u64 old_column_size = jak_memfile_uintvar_column_size_total(dst_memfile);
        jak_memfile_seek(dst_memfile, dst_offset + old_column_size);
        jak_memfile_ensure_space(dst_memfile, required_diff);
        jak_memfile_memcpy(dst_memfile, dst_offset, src_memfile, src_offset, new_column_size);
        jak_memfile_restore_position(dst_memfile);
}

bool jak_memfile_uintvar_column_set(jak_memfile *memfile, jak_u64 idx, jak_u64 value)
{
        JAK_ERROR_IF_NULL(memfile)

        jak_memfile_save_position(memfile);

        /* element counter */
        jak_offset_t  column_start = jak_memfile_tell(memfile);
        jak_u64 num_elems = jak_memfile_read_uintvar_marker(NULL, memfile);
        if (JAK_LIKELY(idx < num_elems)) {
                jak_uintvar_marker_marker_type_e type_is = jak_memfile_peek_uintvar_marker_type(memfile);
                jak_uintvar_marker_marker_type_e type_required = jak_uintvar_marker_type_for(value);
                if (jak_uintvar_marker_compare(type_is, type_required) < 0) {
                        /* The new value does not fit into the int-type of the list; the column must be rewritten.
                         *
                         * For the required inplace update, instead of allocating a temporary buffer directly, the
                         * unused tail of the underlying memfiles memblock is used (if such a tail exists). In
                         * those cases, where enough memory is tailing in the memblock, no reallocation is needed
                         * for the process. In case there is not enough memory for temporarily storing the modified
                         * column, a temporary buffer is allocated to avoid reallocation of the entire memblock.
                         * Finally, in the case the inplace update will require a reallocation of the memblock
                         * because there is not enough capacity to store the longer column, the reallocation is
                         * forced before the update with enough capacities to re-use the memblock as temporary
                         * buffer again. With this process, a reallocation is executed only if there is not other
                         * chance because the current memblock cannot hold the updated column, and an allocation
                         * of a temporary buffer is only executed if otherwise a reallocation of the memblock
                         * would have been done.
                         *
                         * Yeah, memory management.
                         *      -- Marcus, Aug 28 2019
                         */
                        jak_u64 buffer_size;

                        jak_memfile_get_tail_buffer(NULL, &buffer_size, memfile);

                        jak_memfile_seek(memfile, column_start);
                        jak_u8 col_num_elem = jak_memfile_sizeof_uintvar_marker(memfile);
                        jak_u8 col_elem_type = sizeof(jak_marker_t);
                        jak_u64 col_old_elem_size = jak_uintvar_marker_bytes_needed_for_value_by_type(type_is);
                        jak_u64 col_new_elem_size = jak_uintvar_marker_bytes_needed_for_value_by_type(type_required);
                        assert(col_new_elem_size > col_old_elem_size);

                        jak_u64 col_new_size_total = col_num_elem + col_elem_type + (num_elems * col_new_elem_size);
                        jak_u64 memfile_remaining = jak_memfile_remaining_size(memfile);
                        jak_u64 required_diff = col_new_elem_size - col_old_elem_size;

                        if (required_diff > memfile_remaining) {
                                /* The rewrite must lead to a reallocation because the memfile has not enough
                                 * space to hold the rewritten column. Because of that, force a reallocation with
                                 * a large-enough tail buffer and use this tail buffer as temporary buffer */
                                jak_u64 bytes_to_grow = required_diff + col_new_size_total;
                                jak_memfile_grow(memfile, bytes_to_grow);

                                JAK_DEBUG_ONLY_BLOCK(
                                        /* just test that everything was executed correctly */
                                        jak_u64 test_buffer_size;
                                        jak_u64 test_remaining = jak_memfile_remaining_size(memfile);
                                        jak_memfile_get_tail_buffer(NULL, &test_buffer_size, memfile);
                                        assert(test_remaining >= bytes_to_grow);
                                        assert(test_buffer_size >= col_new_size_total);
                                )

                                goto store_in_tail_buffer;
                        } else {
                                /* The rewritten column will fit into this memfile. Check now what space to use for
                                 * the temporary buffer. In case the memfile tail buffer is large enough, use this
                                 * one. Otherwise, temporarily reallocate a new piece of dynamic memory. */
                                jak_offset_t buffer_start = 0;
                                jak_u64 new_column_size = 0;

                                if (col_new_size_total <= buffer_size)
store_in_tail_buffer:           {
                                        /* the rewritten column fits into the buffer */
                                        jak_memfile_get_tail_buffer(&buffer_start, &buffer_size, memfile);
                                        JAK_DEBUG_ONLY_BLOCK(assert(buffer_size >= col_new_size_total); )

                                        /* build the new column */
                                        new_column_size = __jak_uintvar_column_create_and_insert(memfile, buffer_start,
                                                type_required, num_elems, memfile, column_start);

                                        /* remove the old column by overwriting with the new column*/
                                        __jak_uintvar_column_overwrite(memfile, column_start, memfile, buffer_start,
                                                                       required_diff, new_column_size);

                                        /* clear up the buffer */
                                        jak_memfile_write_zero_at(memfile, buffer_start, new_column_size);
                                        jak_memfile_force_tail_buffer_start_at(memfile, buffer_start);

                                } else {
                                        /* the rewritten column is larger than the buffer */

                                        /* create temporary allocated buffer */
                                        jak_memblock *tmp_block;
                                        jak_memfile tmp_file;
                                        jak_memblock_create(&tmp_block, col_new_size_total);
                                        jak_memfile_open(&tmp_file, tmp_block, JAK_READ_WRITE);

                                        /* build the new column */
                                        new_column_size = __jak_uintvar_column_create_and_insert(&tmp_file,
                                                0, type_required, num_elems, memfile, column_start);

                                        /* remove the old column by overwriting with the new column*/
                                        __jak_uintvar_column_overwrite(memfile, column_start, &tmp_file, 0,
                                                required_diff, new_column_size);

                                        /* cleanup temporary allocated buffer */
                                        jak_memblock_drop(tmp_block);
                                }
                        }
                } else {
                        /* the new value fits into the int-type of the list; just update the value in the list */
                        jak_memfile_read_uintvar_marker_type(memfile);
                        jak_u8 elem_size = jak_uintvar_marker_bytes_needed_for_value_by_type(type_is);
                        jak_memfile_seek_relative(memfile, idx * elem_size);
                        jak_u8 skip = jak_uintvar_marker_write_value_only(jak_memfile_raw_data_from_here(memfile), type_is, value);
                        jak_memfile_skip(memfile, skip);
                }
        } else {
                jak_memfile_restore_position(memfile);
                JAK_ERROR(&memfile->err, JAK_ERR_OUTOFBOUNDS)
                return false;
        }

        jak_memfile_restore_position(memfile);

        return true;
}

bool jak_memfile_uintvar_column_skip(jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile)
        jak_u64 total_size = jak_memfile_uintvar_column_size_total(memfile);
        jak_memfile_skip(memfile, total_size);
        return true;
}

jak_u64 jak_memfile_uintvar_column_size_header(jak_uintvar_marker_marker_type_e *value_type, jak_u64 *num_elems, jak_offset_t *payload_start_off, jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile)
        jak_memfile_save_position(memfile);

        /* read over header that is a uintvar (marker-based), which encodes the number of elements in the column,
         * also read contained element type, which is just specified by a marker */
        jak_offset_t start = jak_memfile_tell(memfile);
        jak_u64 elem_num = jak_memfile_read_uintvar_marker(NULL, memfile);
        jak_uintvar_marker_marker_type_e val_type = jak_uintvar_marker_peek_marker(jak_memfile_raw_data_from_here(memfile));
        jak_memfile_skip(memfile, JAK_UINTVAR_MARKER_BYTES_NEEDED_FOR_MARKER());
        jak_offset_t end = jak_memfile_tell(memfile);

        JAK_OPTIONAL_SET(num_elems, elem_num);
        JAK_OPTIONAL_SET(value_type, val_type);
        JAK_OPTIONAL_SET(payload_start_off, end);

        jak_memfile_restore_position(memfile);

        return end - start;
}

jak_u64 jak_memfile_uintvar_column_size_payload(jak_offset_t *column_end_off, jak_memfile *memfile)
{
        jak_u64 payload_start;
        jak_u64 num_elems;
        jak_uintvar_marker_marker_type_e value_type;

        jak_memfile_save_position(memfile);

        jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, memfile);
        jak_offset_t start = payload_start;
        jak_memfile_seek(memfile, payload_start + (num_elems * jak_uintvar_marker_bytes_needed_for_value_by_type(value_type)));
        jak_offset_t end = jak_memfile_tell(memfile);

        JAK_OPTIONAL_SET(column_end_off, end)

        jak_memfile_restore_position(memfile);

        return end - start;
}

jak_u64 jak_memfile_uintvar_column_size_total(jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile)

        jak_u64 num_elems, header_size, payload_size;
        jak_uintvar_marker_marker_type_e value_type;
        jak_offset_t payload_start_off;

        header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start_off, memfile);
        payload_size = jak_memfile_uintvar_column_size_payload(NULL, memfile);

        return header_size + payload_size;
}

jak_uintvar_marker_marker_type_e jak_memfile_uintvar_column_get_counter_type(jak_memfile *memfile)
{
        jak_memfile_save_position(memfile);
        jak_uintvar_marker_marker_type_e ret = jak_memfile_peek_uintvar_marker_type(memfile);;
        jak_memfile_restore_position(memfile);
        return ret;
}

jak_uintvar_marker_marker_type_e jak_memfile_uintvar_column_get_value_type(jak_memfile *memfile)
{
        jak_memfile_save_position(memfile);
        jak_memfile_read_uintvar_marker(NULL, memfile);
        jak_uintvar_marker_marker_type_e ret = jak_memfile_peek_uintvar_marker_type(memfile);
        jak_memfile_restore_position(memfile);
        return ret;
}

bool jak_memfile_uintvar_column_is_u8(jak_memfile *memfile)
{
        return jak_memfile_uintvar_column_get_value_type(memfile) == JAK_UINTVAR_8;
}

bool jak_memfile_uintvar_column_is_u16(jak_memfile *memfile)
{
        return jak_memfile_uintvar_column_get_value_type(memfile) == JAK_UINTVAR_16;
}

bool jak_memfile_uintvar_column_is_u32(jak_memfile *memfile)
{
        return jak_memfile_uintvar_column_get_value_type(memfile) == JAK_UINTVAR_32;
}

bool jak_memfile_uintvar_column_is_u64(jak_memfile *memfile)
{
        return jak_memfile_uintvar_column_get_value_type(memfile) == JAK_UINTVAR_64;
}

const void *__jak_uintvar_column_read_payload(jak_u64 *num_elems, jak_memfile *memfile,
        jak_uintvar_marker_marker_type_e expected)
{
        const void *ret = NULL;
        jak_memfile_save_position(memfile);
        jak_u64 nelems = jak_memfile_read_uintvar_marker(NULL, memfile);
        JAK_OPTIONAL_SET(num_elems, nelems)
        jak_uintvar_marker_marker_type_e type = jak_memfile_read_uintvar_marker_type(memfile);
        if (JAK_LIKELY(type == expected)) {
                ret = jak_memfile_raw_data_from_here(memfile);
        } else {
                JAK_ERROR(&memfile->err, JAK_ERR_TYPEMISMATCH);
        }
        jak_memfile_restore_position(memfile);
        return ret;
}

const jak_u8 *jak_memfile_uintvar_column_read_u8(jak_u64 *num_elems, jak_memfile *memfile)
{
        return __jak_uintvar_column_read_payload(num_elems, memfile, JAK_UINTVAR_8);
}

const jak_u16 *jak_memfile_uintvar_column_read_u16(jak_u64 *num_elems, jak_memfile *memfile)
{
        return __jak_uintvar_column_read_payload(num_elems, memfile, JAK_UINTVAR_16);
}

const jak_u32 *jak_memfile_uintvar_column_read_u32(jak_u64 *num_elems, jak_memfile *memfile)
{
        return __jak_uintvar_column_read_payload(num_elems, memfile, JAK_UINTVAR_32);
}

const jak_u64 *jak_memfile_uintvar_column_read_u64(jak_u64 *num_elems, jak_memfile *memfile)
{
        return __jak_uintvar_column_read_payload(num_elems, memfile, JAK_UINTVAR_64);
}

jak_u8 jak_memfile_uintvar_column_u8_at(jak_memfile *memfile, jak_u64 pos)
{
        return jak_memfile_uintvar_column_read_u8(NULL, memfile)[pos];
}

jak_u16 jak_memfile_uintvar_column_u16_at(jak_memfile *memfile, jak_u64 pos)
{
        return jak_memfile_uintvar_column_read_u16(NULL, memfile)[pos];
}

jak_u32 jak_memfile_uintvar_column_u32_at(jak_memfile *memfile, jak_u64 pos)
{
        return jak_memfile_uintvar_column_read_u32(NULL, memfile)[pos];
}

jak_u64 jak_memfile_uintvar_column_u64_at(jak_memfile *memfile, jak_u64 pos)
{
        return jak_memfile_uintvar_column_read_u64(NULL, memfile)[pos];
}

bool jak_memfile_uintvar_column_to_str(jak_string *str, jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(str)
        JAK_ERROR_IF_NULL(memfile)

        jak_memfile_save_position(memfile);
        jak_memfile_save_position(memfile);
        jak_uintvar_marker_marker_type_e num_elems_type = jak_memfile_peek_uintvar_marker_type(memfile);
        jak_u64 num_elems = jak_memfile_read_uintvar_marker(NULL, memfile);

        jak_uintvar_marker_marker_type_e elem_type = jak_memfile_read_uintvar_marker_type(memfile);

        jak_string_add_char(str, '[');
        jak_string_add_char(str, jak_uintvar_marker_strs[num_elems_type]);
        jak_string_add(str, "][");
        jak_string_add_u64(str, num_elems);
        jak_string_add(str, "] [");
        jak_string_add_char(str, jak_uintvar_marker_strs[elem_type]);
        jak_string_add(str, "]");

        jak_memfile_restore_position(memfile);

        for (jak_u64 i = 0; i < num_elems; i++) {
                jak_string_add_char(str, '[');
                switch (elem_type) {
                        case JAK_UINTVAR_8:
                                jak_string_add_u8(str, jak_memfile_uintvar_column_u8_at(memfile, i));
                                break;
                        case JAK_UINTVAR_16:
                                jak_string_add_u16(str, jak_memfile_uintvar_column_u16_at(memfile, i));
                                break;
                        case JAK_UINTVAR_32:
                                jak_string_add_u32(str, jak_memfile_uintvar_column_u32_at(memfile, i));
                                break;
                        case JAK_UINTVAR_64:
                                jak_string_add_u64(str, jak_memfile_uintvar_column_u64_at(memfile, i));
                                break;
                        default:
                                jak_memfile_restore_position(memfile);
                                JAK_ERROR(&memfile->err, JAK_ERR_INTERNALERR);
                                return false;
                }
                jak_string_add_char(str, ']');
        }

        jak_memfile_restore_position(memfile);
        return true;
}

bool jak_memfile_seek_to_start(jak_memfile *file)
{
        return jak_memfile_seek(file, 0);
}

bool jak_memfile_seek_to_end(jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        size_t size = jak_memblock_last_used_byte(file->memblock);
        return jak_memfile_seek(file, size);
}

bool jak_memfile_inplace_insert(jak_memfile *file, size_t nbytes)
{
        JAK_ERROR_IF_NULL(file);
        return jak_memblock_move_right(file->memblock, file->pos, nbytes);
}

bool jak_memfile_inplace_remove(jak_memfile *file, size_t nbytes_from_here)
{
        JAK_ERROR_IF_NULL(file);
        return jak_memblock_move_left(file->memblock, file->pos, nbytes_from_here);
}

bool jak_memfile_bit_mode_end(size_t *num_bytes_written, jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        file->bit_mode = false;
        if (file->current_write_bit <= 8) {
                jak_memfile_skip(file, 1);
                file->bytes_completed++;
        }
        JAK_OPTIONAL_SET(num_bytes_written, file->bytes_completed);
        file->current_write_bit = file->bytes_completed = 0;
        return true;
}

void *jak_memfile_current_pos(jak_memfile *file, jak_offset_t nbytes)
{
        if (file && nbytes > 0) {
                jak_offset_t file_size = 0;
                jak_memblock_size(&file_size, file->memblock);
                jak_offset_t required_size = file->pos + nbytes;
                if (JAK_UNLIKELY(file->pos + nbytes >= file_size)) {
                        if (file->mode == JAK_READ_WRITE) {
                                jak_memblock_resize(file->memblock, required_size * 1.7f);
                        } else {
                                JAK_ERROR(&file->err, JAK_ERR_WRITEPROT);
                                return NULL;
                        }
                }
                void *data = (void *) jak_memfile_peek(file, nbytes);
                return data;
        } else {
                return NULL;
        }
}

bool jak_memfile_hexdump(jak_string *sb, jak_memfile *file)
{
        JAK_ERROR_IF_NULL(sb);
        JAK_ERROR_IF_NULL(file);
        JAK_DECLARE_AND_INIT(jak_offset_t, block_size)
        jak_memblock_size(&block_size, file->memblock);
        jak_hexdump(sb, jak_memblock_raw_data(file->memblock), block_size);
        return true;
}

bool jak_memfile_hexdump_printf(FILE *file, jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(memfile)
        JAK_DECLARE_AND_INIT(jak_offset_t, block_size)
        jak_memblock_size(&block_size, memfile->memblock);
        jak_hexdump_print(file, jak_memblock_raw_data(memfile->memblock), block_size);
        return true;
}

bool jak_memfile_hexdump_print(jak_memfile *memfile)
{
        return jak_memfile_hexdump_printf(stdout, memfile);
}