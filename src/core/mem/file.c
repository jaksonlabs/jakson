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

#include "core/mem/file.h"
#include "stdx/varuint.h"

bool memfile_open(struct memfile *file, struct memblock *block, enum access_mode mode)
{
        error_if_null(file)
        error_if_null(block)
        file->memblock = block;
        file->pos = 0;
        file->bit_mode = false;
        file->mode = mode;
        file->saved_pos_ptr = 0;
        error_init(&file->err);
        return true;
}

NG5_EXPORT(bool) memfile_dup(struct memfile *dst, struct memfile *src)
{
        error_if_null(dst)
        error_if_null(src)
        memfile_open(dst, src->memblock, src->mode);
        memfile_seek(dst, memfile_tell(src));
        return true;
}

bool memfile_seek(struct memfile *file, offset_t pos)
{
        error_if_null(file)
        offset_t file_size;
        memblock_size(&file_size, file->memblock);
        if (unlikely(pos >= file_size)) {
                if (file->mode == READ_WRITE) {
                        offset_t new_size = pos + 1;
                        memblock_resize(file->memblock, new_size);
                } else {
                        error(&file->err, NG5_ERR_MEMSTATE)
                        return false;
                }
        }
        file->pos = pos;
        return true;
}

bool memfile_rewind(struct memfile *file)
{
        error_if_null(file)
        file->pos = 0;
        return true;
}

NG5_EXPORT(bool) memfile_grow(struct memfile *file_in, size_t grow_by_bytes, bool zero_out)
{
        error_if_null(file_in)
        if (likely(grow_by_bytes > 0)) {
                offset_t block_size;
                memblock_size(&block_size, file_in->memblock);
                memblock_resize_ex(file_in->memblock, (block_size + grow_by_bytes), zero_out);
        }
        return true;
}

bool memfile_get_offset(offset_t *pos, const struct memfile *file)
{
        error_if_null(pos)
        error_if_null(file)
        *pos = file->pos;
        return true;
}

size_t memfile_size(struct memfile *file)
{
        if (!file || !file->memblock) {
                return 0;
        } else {
                u64 size;
                memblock_size(&size, file->memblock);
                return size;
        }
}

NG5_EXPORT(bool) memfile_cut(struct memfile *file, size_t how_many_bytes)
{
        error_if_null(file);
        offset_t block_size;
        memblock_size(&block_size, file->memblock);

        if (how_many_bytes > 0 && block_size > how_many_bytes) {
                size_t new_block_size = block_size - how_many_bytes;
                memblock_resize(file->memblock, new_block_size);
                file->pos = ng5_min(file->pos, new_block_size);
                return true;
        } else {
                error(&file->err, NG5_ERR_ILLEGALARG);
                return false;
        }
}

size_t memfile_remain_size(struct memfile *file)
{
        assert(file->pos <= memfile_size(file));
        return memfile_size(file) - file->pos;
}

bool memfile_shrink(struct memfile *file)
{
        error_if_null(file);
        if (file->mode == READ_WRITE) {
                int status = memblock_shrink(file->memblock);
                u64 size;
                memblock_size(&size, file->memblock);
                assert(size == file->pos);
                return status;
        } else {
                error(&file->err, NG5_ERR_WRITEPROT)
                return false;
        }
}

const char *memfile_read(struct memfile *file, offset_t nbytes)
{
        const char *result = memfile_peek(file, nbytes);
        file->pos += nbytes;
        return result;
}

bool memfile_skip(struct memfile *file, offset_t nbytes)
{
        offset_t required_size = file->pos + nbytes;
        file->pos += nbytes;
        offset_t file_size;
        memblock_size(&file_size, file->memblock);

        if (unlikely(required_size >= file_size)) {
                if (file->mode == READ_WRITE) {
                        memblock_resize(file->memblock, required_size * 1.7f);
                } else {
                        error(&file->err, NG5_ERR_WRITEPROT);
                        return false;
                }
        }
        memfile_update_last_byte(file->memblock, file->pos);
        assert(file->pos < memfile_size(file));
        return true;
}

const char *memfile_peek(struct memfile *file, offset_t nbytes)
{
        offset_t file_size;
        memblock_size(&file_size, file->memblock);
        if (unlikely(file->pos + nbytes > file_size)) {
                error(&file->err, NG5_ERR_READOUTOFBOUNDS);
                return NULL;
        } else {
                const char *result = memblock_raw_data(file->memblock) + file->pos;
                return result;
        }
}

bool memfile_write(struct memfile *file, const void *data, offset_t nbytes)
{
        error_if_null(file)
        error_if_null(data)
        if (file->mode == READ_WRITE) {
                if (likely(nbytes != 0)) {
                        offset_t file_size;
                        memblock_size(&file_size, file->memblock);
                        offset_t required_size = file->pos + nbytes;
                        if (unlikely(required_size >= file_size)) {
                                memblock_resize(file->memblock, required_size * 1.7f);
                        }

                        if (unlikely(!memblock_write(file->memblock, file->pos, data, nbytes))) {
                                return false;
                        }
                        file->pos += nbytes;
                }
                return true;
        } else {
                error(&file->err, NG5_ERR_WRITEPROT);
                return false;
        }
}

NG5_EXPORT(bool) memfile_write_zero(struct memfile *file, size_t how_many)
{
        error_if_null(file);
        error_if_null(how_many);
        char empty = 0;
        while (how_many--) {
                memfile_write(file, &empty, sizeof(char));
        }
        return true;
}

bool memfile_begin_bit_mode(struct memfile *file)
{
        error_if_null(file);
        if (file->mode == READ_WRITE) {
                file->bit_mode = true;
                file->current_read_bit = file->current_write_bit = file->bytes_completed = 0;
                file->bytes_completed = 0;
                offset_t offset;
                char empty = '\0';
                memfile_get_offset(&offset, file);
                memfile_write(file, &empty, sizeof(char));
                memfile_seek(file, offset);
        } else {
                error(&file->err, NG5_ERR_WRITEPROT);
                return false;
        }

        return true;
}

bool memfile_write_bit(struct memfile *file, bool flag)
{
        error_if_null(file);
        file->current_read_bit = 0;

        if (file->bit_mode) {
                if (file->current_write_bit < 8) {
                        offset_t offset;
                        memfile_get_offset(&offset, file);
                        char byte = *memfile_read(file, sizeof(char));
                        char mask = 1 << file->current_write_bit;
                        if (flag) {
                                ng5_set_bits(byte, mask);
                        } else {
                                ng5_unset_bits(byte, mask);
                        }
                        memfile_seek(file, offset);
                        memfile_write(file, &byte, sizeof(char));
                        memfile_seek(file, offset);
                        file->current_write_bit++;
                } else {
                        file->current_write_bit = 0;
                        file->bytes_completed++;
                        char empty = '\0';
                        offset_t off;
                        memfile_skip(file, 1);
                        memfile_get_offset(&off, file);
                        memfile_write(file, &empty, sizeof(char));
                        memfile_seek(file, off);

                        return memfile_write_bit(file, flag);
                }
                return true;
        } else {
                error(&file->err, NG5_ERR_NOBITMODE);
                return false;
        }
}

bool memfile_read_bit(struct memfile *file)
{
        if (!file) {
                return false;
        }

        file->current_write_bit = 0;

        if (file->bit_mode) {
                if (file->current_read_bit < 8) {
                        offset_t offset;
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
                error(&file->err, NG5_ERR_NOBITMODE);
                return false;
        }
}

NG5_EXPORT(bool) memfile_save_position(struct memfile *file)
{
        error_if_null(file);
        offset_t pos = memfile_tell(file);
        if (likely(file->saved_pos_ptr < (i8) (NG5_ARRAY_LENGTH(file->saved_pos)))) {
                file->saved_pos[file->saved_pos_ptr++] = pos;
                return true;
        } else {
                error(&file->err, NG5_ERR_STACK_OVERFLOW)
                return false;
        }

}

NG5_EXPORT(bool) memfile_restore_position(struct memfile *file)
{
        error_if_null(file);
        if (likely(file->saved_pos_ptr >= 0)) {
                offset_t pos = file->saved_pos[--file->saved_pos_ptr];
                memfile_seek(file, pos);
                return true;
        } else {
                error(&file->err, NG5_ERR_STACK_UNDERFLOW)
                return false;
        }
}

NG5_EXPORT(bool) memfile_ensure_space(struct memfile *memfile, u64 nbytes)
{
        error_if_null(memfile)

        offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        assert(memfile->pos < block_size);
        size_t diff = block_size - memfile->pos;
        if (diff < nbytes) {
                memfile_grow(memfile, nbytes - diff, true);
        }

        memfile_save_position(memfile);
        offset_t current_off = memfile_tell(memfile);
        for (u32 i = 0; i < nbytes; i++) {
                char c = *memfile_read(memfile, 1);
                if (unlikely(c != 0)) {
                        /* not enough space; enlarge container */
                        memfile_seek(memfile, current_off);
                        memfile_move_right(memfile, nbytes - i);
                        break;
                }
        }
        memfile_restore_position(memfile);

        return true;
}

NG5_EXPORT(u64) memfile_read_varuint(u8 *nbytes, struct memfile *memfile)
{
        u8 nbytes_read;
        u64 result = varuint_read(&nbytes_read, (varuint_t) memfile_peek(memfile, sizeof(char)));
        memfile_skip(memfile, nbytes_read);
        ng5_optional_set(nbytes, nbytes_read);
        return result;
}

NG5_EXPORT(u64) memfile_write_varuint(struct memfile *memfile, u64 value)
{
        u8 required_blocks = varuint_required_blocks(value);
        memfile_ensure_space(memfile, required_blocks);
        varuint_t dst = (varuint_t) memfile_peek(memfile, sizeof(char));
        varuint_write(dst, value);
        memfile_skip(memfile, required_blocks);
        return required_blocks;
}

NG5_EXPORT(bool) memfile_seek_to_end(struct memfile *file)
{
        error_if_null(file)
        size_t size = memblock_last_used_byte(file->memblock);
        return memfile_seek(file, size);
}

NG5_EXPORT(bool) memfile_move_right(struct memfile *file, size_t nbytes)
{
        error_if_null(file);
        return memblock_move_right(file->memblock, file->pos, nbytes);
}

NG5_EXPORT(bool) memfile_move_left(struct memfile *file, size_t nbytes_from_here)
{
        error_if_null(file);
        return memblock_move_left(file->memblock, file->pos, nbytes_from_here);
}

bool memfile_end_bit_mode(size_t *num_bytes_written, struct memfile *file)
{
        error_if_null(file);
        file->bit_mode = false;
        if (file->current_write_bit <= 8) {
                memfile_skip(file, 1);
                file->bytes_completed++;
        }
        ng5_optional_set(num_bytes_written, file->bytes_completed);
        file->current_write_bit = file->bytes_completed = 0;
        return true;
}

void *memfile_current_pos(struct memfile *file, offset_t nbytes)
{
        if (file && nbytes > 0) {
                offset_t file_size;
                memblock_size(&file_size, file->memblock);
                offset_t required_size = file->pos + nbytes;
                if (unlikely(file->pos + nbytes >= file_size)) {
                        if (file->mode == READ_WRITE) {
                                memblock_resize(file->memblock, required_size * 1.7f);
                        } else {
                                error(&file->err, NG5_ERR_WRITEPROT);
                                return NULL;
                        }
                }
                void *data = (void *) memfile_peek(file, nbytes);
                return data;
        } else {
                return NULL;
        }
}

NG5_EXPORT(bool) memfile_hexdump(struct string_builder *sb, struct memfile *file)
{
        error_if_null(sb);
        error_if_null(file);
        offset_t block_size;
        memblock_size(&block_size, file->memblock);
        hexdump(sb, memblock_raw_data(file->memblock), block_size);
        return true;
}

NG5_EXPORT(bool) memfile_hexdump_printf(FILE *file, struct memfile *memfile)
{
        error_if_null(file)
        error_if_null(memfile)
        offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        hexdump_print(file, memblock_raw_data(memfile->memblock), block_size);
        return true;
}

NG5_EXPORT(bool) memfile_hexdump_print(struct memfile *memfile)
{
        return memfile_hexdump_printf(stdout, memfile);
}