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
#include <jak_uintvar_stream.h>

bool memfile_open(struct jak_memfile *file, struct jak_memblock *block, enum access_mode mode)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(block)
        JAK_zero_memory(file, sizeof(struct jak_memfile))
        file->memblock = block;
        file->pos = 0;
        file->bit_mode = false;
        file->mode = mode;
        file->saved_pos_ptr = 0;
        error_init(&file->err);
        return true;
}

bool memfile_clone(struct jak_memfile *dst, struct jak_memfile *src)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(src)
        memfile_open(dst, src->memblock, src->mode);
        memfile_seek(dst, memfile_tell(src));
        dst->bit_mode = src->bit_mode;
        dst->saved_pos_ptr = src->saved_pos_ptr;
        error_cpy(&dst->err, &src->err);
        memcpy(&dst->saved_pos, &src->saved_pos, JAK_ARRAY_LENGTH(src->saved_pos));
        return true;
}

bool memfile_seek(struct jak_memfile *file, jak_offset_t pos)
{
        JAK_ERROR_IF_NULL(file)
        jak_offset_t file_size = 0;
        memblock_size(&file_size, file->memblock);
        if (JAK_UNLIKELY(pos >= file_size)) {
                if (file->mode == READ_WRITE) {
                        jak_offset_t new_size = pos + 1;
                        memblock_resize(file->memblock, new_size);
                } else {
                        error(&file->err, JAK_ERR_MEMSTATE)
                        return false;
                }
        }
        file->pos = pos;
        return true;
}

bool memfile_seek_from_here(struct jak_memfile *file, signed_offset_t where)
{
        jak_offset_t now = memfile_tell(file);
        jak_offset_t then = now + where;
        return memfile_seek(file, then);
}

bool memfile_rewind(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        file->pos = 0;
        return true;
}

bool memfile_grow(struct jak_memfile *file_in, size_t grow_by_bytes)
{
        JAK_ERROR_IF_NULL(file_in)
        if (JAK_LIKELY(grow_by_bytes > 0)) {
                jak_offset_t block_size;
                memblock_size(&block_size, file_in->memblock);
                memblock_resize(file_in->memblock, (block_size + grow_by_bytes));
        }
        return true;
}

bool memfile_get_offset(jak_offset_t *pos, const struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(pos)
        JAK_ERROR_IF_NULL(file)
        *pos = file->pos;
        return true;
}

size_t memfile_size(struct jak_memfile *file)
{
        if (!file || !file->memblock) {
                return 0;
        } else {
                jak_u64 size;
                memblock_size(&size, file->memblock);
                return size;
        }
}

bool memfile_cut(struct jak_memfile *file, size_t how_many_bytes)
{
        JAK_ERROR_IF_NULL(file);
        jak_offset_t block_size;
        memblock_size(&block_size, file->memblock);

        if (how_many_bytes > 0 && block_size > how_many_bytes) {
                size_t new_block_size = block_size - how_many_bytes;
                memblock_resize(file->memblock, new_block_size);
                file->pos = JAK_min(file->pos, new_block_size);
                return true;
        } else {
                error(&file->err, JAK_ERR_ILLEGALARG);
                return false;
        }
}

size_t memfile_remain_size(struct jak_memfile *file)
{
        JAK_ASSERT(file->pos <= memfile_size(file));
        return memfile_size(file) - file->pos;
}

bool memfile_shrink(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (file->mode == READ_WRITE) {
                int status = memblock_shrink(file->memblock);
                jak_u64 size;
                memblock_size(&size, file->memblock);
                JAK_ASSERT(size == file->pos);
                return status;
        } else {
                error(&file->err, JAK_ERR_WRITEPROT)
                return false;
        }
}

const char *memfile_read(struct jak_memfile *file, jak_offset_t nbytes)
{
        const char *result = memfile_peek(file, nbytes);
        file->pos += nbytes;
        return result;
}

jak_u8 memfile_read_byte(struct jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_u8);
}

jak_u8 memfile_peek_byte(struct jak_memfile *file)
{
        return *JAK_MEMFILE_PEEK(file, jak_u8);
}

jak_u64 memfile_read_u64(struct jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_u64);
}

jak_i64 memfile_read_i64(struct jak_memfile *file)
{
        return *JAK_MEMFILE_READ_TYPE(file, jak_i64);
}

bool memfile_skip(struct jak_memfile *file, signed_offset_t nbytes)
{
        jak_offset_t required_size = file->pos + nbytes;
        file->pos += nbytes;
        jak_offset_t file_size;
        memblock_size(&file_size, file->memblock);

        if (JAK_UNLIKELY(required_size >= file_size)) {
                if (file->mode == READ_WRITE) {
                        memblock_resize(file->memblock, required_size * 1.7f);
                } else {
                        error(&file->err, JAK_ERR_WRITEPROT);
                        return false;
                }
        }
        memfile_update_last_byte(file->memblock, file->pos);
        JAK_ASSERT(file->pos < memfile_size(file));
        return true;
}

const char *memfile_peek(struct jak_memfile *file, jak_offset_t nbytes)
{
        jak_offset_t file_size;
        memblock_size(&file_size, file->memblock);
        if (JAK_UNLIKELY(file->pos + nbytes > file_size)) {
                error(&file->err, JAK_ERR_READOUTOFBOUNDS);
                return NULL;
        } else {
                const char *result = memblock_raw_data(file->memblock) + file->pos;
                return result;
        }
}

bool memfile_write_byte(struct jak_memfile *file, jak_u8 data)
{
        return memfile_write(file, &data, sizeof(jak_u8));
}

bool memfile_write(struct jak_memfile *file, const void *data, jak_offset_t nbytes)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(data)
        if (file->mode == READ_WRITE) {
                if (JAK_LIKELY(nbytes != 0)) {
                        jak_offset_t file_size;
                        memblock_size(&file_size, file->memblock);
                        jak_offset_t required_size = file->pos + nbytes;
                        if (JAK_UNLIKELY(required_size >= file_size)) {
                                memblock_resize(file->memblock, required_size * 1.7f);
                        }

                        if (JAK_UNLIKELY(!memblock_write(file->memblock, file->pos, data, nbytes))) {
                                return false;
                        }
                        file->pos += nbytes;
                }
                return true;
        } else {
                error(&file->err, JAK_ERR_WRITEPROT);
                return false;
        }
}

bool memfile_write_zero(struct jak_memfile *file, size_t how_many)
{
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(how_many);
        char empty = 0;
        while (how_many--) {
                memfile_write(file, &empty, sizeof(char));
        }
        return true;
}

bool memfile_begin_bit_mode(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (file->mode == READ_WRITE) {
                file->bit_mode = true;
                file->current_read_bit = file->current_write_bit = file->bytes_completed = 0;
                file->bytes_completed = 0;
                jak_offset_t offset;
                char empty = '\0';
                memfile_get_offset(&offset, file);
                memfile_write(file, &empty, sizeof(char));
                memfile_seek(file, offset);
        } else {
                error(&file->err, JAK_ERR_WRITEPROT);
                return false;
        }

        return true;
}

bool memfile_write_bit(struct jak_memfile *file, bool flag)
{
        JAK_ERROR_IF_NULL(file);
        file->current_read_bit = 0;

        if (file->bit_mode) {
                if (file->current_write_bit < 8) {
                        jak_offset_t offset;
                        memfile_get_offset(&offset, file);
                        char byte = *memfile_read(file, sizeof(char));
                        char mask = 1 << file->current_write_bit;
                        if (flag) {
                                JAK_set_bits(byte, mask);
                        } else {
                                JAK_unset_bits(byte, mask);
                        }
                        memfile_seek(file, offset);
                        memfile_write(file, &byte, sizeof(char));
                        memfile_seek(file, offset);
                        file->current_write_bit++;
                } else {
                        file->current_write_bit = 0;
                        file->bytes_completed++;
                        char empty = '\0';
                        jak_offset_t off;
                        memfile_skip(file, 1);
                        memfile_get_offset(&off, file);
                        memfile_write(file, &empty, sizeof(char));
                        memfile_seek(file, off);

                        return memfile_write_bit(file, flag);
                }
                return true;
        } else {
                error(&file->err, JAK_ERR_NOBITMODE);
                return false;
        }
}

bool memfile_read_bit(struct jak_memfile *file)
{
        if (!file) {
                return false;
        }

        file->current_write_bit = 0;

        if (file->bit_mode) {
                if (file->current_read_bit < 8) {
                        jak_offset_t offset;
                        memfile_get_offset(&offset, file);

                        char mask = 1 << file->current_read_bit;
                        char byte = *memfile_read(file, sizeof(char));
                        memfile_seek(file, offset);
                        bool result = ((byte & mask) >> file->current_read_bit) == true;
                        file->current_read_bit++;
                        return result;
                } else {
                        file->current_read_bit = 0;
                        memfile_skip(file, sizeof(char));
                        return memfile_read_bit(file);
                }
        } else {
                error(&file->err, JAK_ERR_NOBITMODE);
                return false;
        }
}

jak_offset_t memfile_save_position(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        jak_offset_t pos = memfile_tell(file);
        if (JAK_LIKELY(file->saved_pos_ptr < (jak_i8) (JAK_ARRAY_LENGTH(file->saved_pos)))) {
                file->saved_pos[file->saved_pos_ptr++] = pos;
        } else {
                error(&file->err, JAK_ERR_STACK_OVERFLOW)
        }
        return pos;
}

bool memfile_restore_position(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        if (JAK_LIKELY(file->saved_pos_ptr >= 0)) {
                jak_offset_t pos = file->saved_pos[--file->saved_pos_ptr];
                memfile_seek(file, pos);
                return true;
        } else {
                error(&file->err, JAK_ERR_STACK_UNDERFLOW)
                return false;
        }
}

signed_offset_t memfile_ensure_space(struct jak_memfile *memfile, jak_u64 nbytes)
{
        JAK_ERROR_IF_NULL(memfile)

        jak_offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        JAK_ASSERT(memfile->pos < block_size);
        size_t diff = block_size - memfile->pos;
        if (diff < nbytes) {
                memfile_grow(memfile, nbytes - diff);
        }

        memfile_save_position(memfile);
        jak_offset_t current_off = memfile_tell(memfile);
        signed_offset_t shift = 0;
        for (jak_u32 i = 0; i < nbytes; i++) {
                char c = *memfile_read(memfile, 1);
                if (JAK_UNLIKELY(c != 0)) {
                        /* not enough space; enlarge container */
                        memfile_seek(memfile, current_off);
                        memfile_inplace_insert(memfile, nbytes - i);
                        shift += nbytes - i;
                        break;
                }
        }
        memfile_restore_position(memfile);

        return shift;
}

jak_u64 memfile_read_uintvar_stream(jak_u8 *nbytes, struct jak_memfile *memfile)
{
        jak_u8 nbytes_read;
        jak_u64 result = uintvar_stream_read(&nbytes_read, (uintvar_stream_t) memfile_peek(memfile, sizeof(char)));
        memfile_skip(memfile, nbytes_read);
        JAK_optional_set(nbytes, nbytes_read);
        return result;
}

bool memfile_skip_uintvar_stream(struct jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(memfile)
        memfile_read_uintvar_stream(NULL, memfile);
        return true;
}

jak_u64 memfile_peek_uintvar_stream(jak_u8 *nbytes, struct jak_memfile *memfile)
{
        memfile_save_position(memfile);
        jak_u64 result = memfile_read_uintvar_stream(nbytes, memfile);
        memfile_restore_position(memfile);
        return result;
}

jak_u64 memfile_write_uintvar_stream(jak_u64 *nbytes_moved, struct jak_memfile *memfile, jak_u64 value)
{
        jak_u8 required_blocks = uintvar_stream_required_blocks(value);
        signed_offset_t shift = memfile_ensure_space(memfile, required_blocks);
        uintvar_stream_t dst = (uintvar_stream_t) memfile_peek(memfile, sizeof(char));
        uintvar_stream_write(dst, value);
        memfile_skip(memfile, required_blocks);
        JAK_optional_set(nbytes_moved, shift);
        return required_blocks;
}

signed_offset_t memfile_update_uintvar_stream(struct jak_memfile *memfile, jak_u64 value)
{
        JAK_ERROR_IF_NULL(memfile);

        jak_u8 bytes_used_now, bytes_used_then;

        memfile_peek_uintvar_stream(&bytes_used_now, memfile);
        bytes_used_then = uintvar_stream_required_blocks(value);

        if (bytes_used_now < bytes_used_then) {
                jak_u8 inc = bytes_used_then - bytes_used_now;
                memfile_inplace_insert(memfile, inc);
        } else if (bytes_used_now > bytes_used_then) {
                jak_u8 dec = bytes_used_now - bytes_used_then;
                memfile_inplace_remove(memfile, dec);
        }

        uintvar_stream_t dst = (uintvar_stream_t) memfile_peek(memfile, sizeof(char));
        jak_u8 required_blocks = uintvar_stream_write(dst, value);
        memfile_skip(memfile, required_blocks);

        return bytes_used_then - bytes_used_now;
}

bool memfile_seek_to_start(struct jak_memfile *file)
{
        return memfile_seek(file, 0);
}

bool memfile_seek_to_end(struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file)
        size_t size = memblock_last_used_byte(file->memblock);
        return memfile_seek(file, size);
}

bool memfile_inplace_insert(struct jak_memfile *file, size_t nbytes)
{
        JAK_ERROR_IF_NULL(file);
        return memblock_move_right(file->memblock, file->pos, nbytes);
}

bool memfile_inplace_remove(struct jak_memfile *file, size_t nbytes_from_here)
{
        JAK_ERROR_IF_NULL(file);
        return memblock_move_left(file->memblock, file->pos, nbytes_from_here);
}

bool memfile_end_bit_mode(size_t *num_bytes_written, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(file);
        file->bit_mode = false;
        if (file->current_write_bit <= 8) {
                memfile_skip(file, 1);
                file->bytes_completed++;
        }
        JAK_optional_set(num_bytes_written, file->bytes_completed);
        file->current_write_bit = file->bytes_completed = 0;
        return true;
}

void *memfile_current_pos(struct jak_memfile *file, jak_offset_t nbytes)
{
        if (file && nbytes > 0) {
                jak_offset_t file_size;
                memblock_size(&file_size, file->memblock);
                jak_offset_t required_size = file->pos + nbytes;
                if (JAK_UNLIKELY(file->pos + nbytes >= file_size)) {
                        if (file->mode == READ_WRITE) {
                                memblock_resize(file->memblock, required_size * 1.7f);
                        } else {
                                error(&file->err, JAK_ERR_WRITEPROT);
                                return NULL;
                        }
                }
                void *data = (void *) memfile_peek(file, nbytes);
                return data;
        } else {
                return NULL;
        }
}

bool memfile_hexdump(struct jak_string *sb, struct jak_memfile *file)
{
        JAK_ERROR_IF_NULL(sb);
        JAK_ERROR_IF_NULL(file);
        jak_offset_t block_size;
        memblock_size(&block_size, file->memblock);
        hexdump(sb, memblock_raw_data(file->memblock), block_size);
        return true;
}

bool memfile_hexdump_printf(FILE *file, struct jak_memfile *memfile)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(memfile)
        jak_offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        hexdump_print(file, memblock_raw_data(memfile->memblock), block_size);
        return true;
}

bool memfile_hexdump_print(struct jak_memfile *memfile)
{
        return memfile_hexdump_printf(stdout, memfile);
}