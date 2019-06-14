/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements an (read-/write) iterator for (JSON) arrays in BISON
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

#include "core/bison/bison-column-it.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-int.h"

NG5_EXPORT(bool) bison_column_it_create(struct bison_column_it *it, struct memfile *memfile, struct err *err,
        offset_t column_start_offset)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->column_start_offset = column_start_offset;
        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, column_start_offset);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8) + sizeof(media_type_t), err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_COLUMN_BEGIN, err, NG5_ERR_ILLEGALOP,
                "column begin marker ('(') not found");

        enum bison_field_type type = (enum bison_field_type) *memfile_read(&it->memfile, sizeof(media_type_t));
        it->type = type;

        it->column_num_elements_offset = memfile_tell(&it->memfile);
        it->column_num_elements = *memfile_read(&it->memfile, sizeof(u32));
        it->column_capacity_offset = memfile_tell(&it->memfile);
        it->column_capacity = *memfile_read(&it->memfile, sizeof(u32));

        /* header consists of column begin marker and contained element type */
        it->payload_start = memfile_tell(&it->memfile);
        bison_column_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_column_it_insert(struct bison_insert *inserter, struct bison_column_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_insert_create_for_column(inserter, it);
}

NG5_EXPORT(bool) bison_column_it_fast_forward(struct bison_column_it *it)
{
        error_if_null(it);
        bison_column_it_values(NULL, NULL, it);
        return true;
}

NG5_EXPORT(const void *) bison_column_it_values(enum bison_field_type *type, u32 *nvalues, struct bison_column_it *it)
{
        error_if_null(it);
        memfile_seek(&it->memfile, it->column_num_elements_offset);
        u32 num_elements = *NG5_MEMFILE_READ_TYPE(&it->memfile, u32);

        memfile_seek(&it->memfile, it->column_capacity_offset);
        u32 cap_elements = *NG5_MEMFILE_READ_TYPE(&it->memfile, u32);

        memfile_seek(&it->memfile, it->payload_start);
        const void *result = memfile_peek(&it->memfile, sizeof(void));

        ng5_optional_set(type, it->type);
        ng5_optional_set(nvalues, num_elements);

        u32 skip = cap_elements * bison_int_get_type_value_size(it->type);
        memfile_seek(&it->memfile, it->payload_start + skip);

        char end = *memfile_read(&it->memfile, sizeof(u8));

        error_if_and_return(end != BISON_MARKER_COLUMN_END, &it->err, NG5_ERR_CORRUPTED, NULL);

        return result;
}

/**
 * Locks the iterator with a spinlock. A call to <code>bison_column_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_column_it_lock(struct bison_column_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_column_it_unlock(struct bison_column_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_column_it_rewind(struct bison_column_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->memfile, it->payload_start);
}