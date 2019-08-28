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

bool jak_memfile_seek_from_here(jak_memfile *file, jak_signed_offset_t where)
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

size_t jak_memfile_remain_size(jak_memfile *file)
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
        for (jak_u32 i = 0; i < nbytes; i++) {
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

jak_uintvar_marker_e jak_memfile_peek_uintvar_marker_type(jak_memfile *memfile)
{
        jak_uintvar_marker_t src = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
        return jak_uintvar_marker_type(src);
}

jak_u64 jak_memfile_write_uintvar_marker(jak_u64 *nbytes_moved, jak_memfile *memfile, jak_u64 value)
{
        jak_u8 num_bytes_needed = jak_uintvar_marker_required_size(value);
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
        bytes_used_then = jak_uintvar_marker_required_size(value);

        __jak_uintvar_move_memory(bytes_used_now, bytes_used_then, memfile);

        jak_uintvar_marker_t dst = (jak_uintvar_marker_t) jak_memfile_peek(memfile, sizeof(char));
        jak_u8 required_bytes = jak_uintvar_marker_write(dst, value);
        jak_memfile_skip(memfile, required_bytes + sizeof(jak_marker_t));

        return bytes_used_then - bytes_used_now;
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