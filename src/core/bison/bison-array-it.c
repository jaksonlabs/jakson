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

#include "core/bison/bison.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-insert.h"

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct bison *doc, offset_t payload_start,
        enum access_mode mode)
{
        error_if_null(it);
        error_if_null(doc);

        it->payload_start = payload_start;
        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, doc->memblock, mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8), &doc->err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_ARRAY_BEGIN, &doc->err, NG5_ERR_ILLEGALOP,
                "array begin marker ('[') not found");

        it->payload_start += sizeof(u8);
        bison_array_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it, struct bison *doc)
{
        ng5_unused(it);
        ng5_unused(doc);
        return false;
}

/**
 * Locks the iterator with a spinlock. A call to <code>bison_array_it_unlock</code> is required for unlocking.
 */
NG5_EXPORT(bool) bison_array_it_lock(struct bison_array_it *it)
{
        error_if_null(it);
        spin_acquire(&it->lock);
        return true;
}

/**
 * Unlocks the iterator
 */
NG5_EXPORT(bool) bison_array_it_unlock(struct bison_array_it *it)
{
        error_if_null(it);
        spin_release(&it->lock);
        return true;
}

NG5_EXPORT(bool) bison_array_it_rewind(struct bison_array_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it)
{
        error_if_null(it);
        char c = *memfile_peek(&it->memfile, 1);
        bool is_empty_slot = c == 0;
        bool is_array_end = c == BISON_MARKER_ARRAY_END;
        if (!is_empty_slot && !is_array_end) {
                return true;
        } else {
                if (!is_array_end) {
                        bool null_found;
                        u8 byte;
                        do {
                                byte = *memfile_read(&it->memfile, sizeof(u8));
                                null_found = byte == 0;
                        } while (null_found);
                        assert(byte == BISON_MARKER_ARRAY_END);
                }

                return false;
        }
}

NG5_EXPORT(bool) bison_array_it_field_type(enum bison_field_type *type, struct bison_array_it *it)
{
        error_if_null(type)
        error_if_null(it)
        error_if(memfile_remain_size(&it->memfile) < 1, &it->err, NG5_ERR_ILLEGALOP);
        u8 media_type = *memfile_read(&it->memfile, 1);
        error_if(media_type == 0, &it->err, NG5_ERR_NOTFOUND)
        error_if(media_type == BISON_MARKER_ARRAY_END, &it->err, NG5_ERR_OUTOFBOUNDS)
        *type = media_type;
        return true;
}

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_insert(struct bison_insert *inserter, struct bison_array_it *it)
{
        error_if_null(inserter)
        error_if_null(it)
        return bison_insert_create(inserter, it);
}

NG5_EXPORT(bool) bison_array_it_remove(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_update(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}