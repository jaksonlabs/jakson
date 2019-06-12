/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file is for internal usage only; do not call these functions from outside
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

#include <utils/hexdump.h>
#include "core/bison/bison.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-int.h"

static void marker_insert(struct memfile *memfile, u8 marker);

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);

        u8 array_begin_marker = BISON_MARKER_ARRAY_BEGIN;
        u8 array_end_marker = BISON_MARKER_ARRAY_END;

        bison_int_ensure_space(memfile, sizeof(u8));

        // --- DEBUG
        offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        printf("step1:\n");
        hexdump_print(stdout, memblock_raw_data(memfile->memblock), block_size);
        // --- DEBUG

        marker_insert(memfile, array_begin_marker);

        // --- DEBUG
        memblock_size(&block_size, memfile->memblock);
        printf("step2:\n");
        hexdump_print(stdout, memblock_raw_data(memfile->memblock), block_size);
        // --- DEBUG

        bison_int_ensure_space(memfile, nbytes + sizeof(u8));

        // --- DEBUG
        memblock_size(&block_size, memfile->memblock);
        printf("step3:\n");
        hexdump_print(stdout, memblock_raw_data(memfile->memblock), block_size);
        // --- DEBUG

        offset_t payload_begin = memfile_tell(memfile);
        size_t remain = memfile_remain_size(memfile);
        size_t span = ng5_max(nbytes, remain) - ng5_min(nbytes, remain);

        /* array may fit into memory if memory does not contain any (non-null) data */
        size_t upper = ng5_min(span, nbytes);
        size_t non_null_pos = 0;
        const char *data = memfile_read(memfile, upper);
        for (; non_null_pos < upper; non_null_pos++) {
                if (data[non_null_pos] != 0) {
                        break;
                }
        }
        if (non_null_pos != nbytes) {
                /* array does fit only partially into memory since some non-null data was read */
                size_t required = nbytes - non_null_pos;
                if (non_null_pos > 0) {
                        /* re-use some null data */
                        memfile_seek(memfile, payload_begin + non_null_pos - 1);
                        memfile_move(memfile, required);
                } else {
                        /* no null data was available at all */
                        memfile_move(memfile, nbytes);
                }
        } else {
                /* nothing to do: n bytes of null data was read */
        }

        marker_insert(memfile, array_end_marker);

        /* seek to first entry in array */
        memfile_seek(memfile, payload_begin);

        return true;
}

NG5_EXPORT(bool) bison_int_ensure_space(struct memfile *memfile, u64 nbytes)
{
        error_if_null(memfile)

        offset_t block_size;
        memblock_size(&block_size, memfile->memblock);
        assert(memfile->pos < block_size);
        size_t diff = block_size - memfile->pos;
        if (diff < nbytes) {
                memfile_grow(memfile, nbytes - diff, true);
        }

        char c = *memfile_peek(memfile, 1);
        if (unlikely(c != 0)) {
                /* not enough space; enlarge container */
                memfile_move(memfile, nbytes);
        }
        return true;
}


static void marker_insert(struct memfile *memfile, u8 marker)
{
        /* check whether marker can be written, otherwise make space for it */
        char c = *memfile_peek(memfile, sizeof(u8));
        if (c != 0) {
                memfile_move(memfile, sizeof(u8));
        }
        memfile_write(memfile, &marker, sizeof(u8));
}