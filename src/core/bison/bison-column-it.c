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

NG5_EXPORT(bool) bison_column_it_create(struct bison_column_it *it, struct memfile *memfile, struct err *err,
        offset_t payload_start)
{
        error_if_null(it);
        error_if_null(memfile);
        error_if_null(err);

        it->payload_start = payload_start;
        error_init(&it->err);
        spin_init(&it->lock);
        memfile_open(&it->memfile, memfile->memblock, memfile->mode);
        memfile_seek(&it->memfile, payload_start);

        error_if(memfile_remain_size(&it->memfile) < sizeof(u8) + sizeof(media_type_t), err, NG5_ERR_CORRUPTED);

        u8 marker = *memfile_read(&it->memfile, sizeof(u8));
        error_if_with_details(marker != BISON_MARKER_COLUMN_BEGIN, err, NG5_ERR_ILLEGALOP,
                "column begin marker ('(') not found");

        enum bison_field_type type = (enum bison_field_type) *memfile_read(&it->memfile, sizeof(media_type_t));
        it->type = type;

        /* header consists of column begin marker and contained element type */
        it->payload_start += sizeof(u8) + sizeof(media_type_t);
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

NG5_EXPORT(const void *) bison_column_it_values(enum bison_field_type *type, u64 *nvalues, struct bison_column_it *it)
{
        error_if_null(it);
        memfile_seek(&it->memfile, it->payload_start);
        const void *result = memfile_peek(&it->memfile, sizeof(void));
        ng5_optional_set(type, it->type);
        char c = *(const char *) result;
        u64 contained_values = 0;
        while (c != BISON_MARKER_COLUMN_END) {

                if (c != 0) {
                        contained_values++;

                        switch (it->type) {
                        case BISON_FIELD_TYPE_NULL:
                        case BISON_FIELD_TYPE_TRUE:
                        case BISON_FIELD_TYPE_FALSE:
                                /* nothing to do; no payload */
                                memfile_skip(&it->memfile, sizeof(media_type_t));
                                break;
                        case BISON_FIELD_TYPE_NUMBER_U8:
                        case BISON_FIELD_TYPE_NUMBER_I8:
                                /* skip value */
                                memfile_skip(&it->memfile, sizeof(u8));
                                break;
                        case BISON_FIELD_TYPE_NUMBER_U16:
                        case BISON_FIELD_TYPE_NUMBER_I16:
                                /* skip value */
                                memfile_skip(&it->memfile, sizeof(u16));
                                break;
                        case BISON_FIELD_TYPE_NUMBER_U32:
                        case BISON_FIELD_TYPE_NUMBER_I32:
                                /* skip value */
                                memfile_skip(&it->memfile, sizeof(u32));
                                break;
                        case BISON_FIELD_TYPE_NUMBER_U64:
                        case BISON_FIELD_TYPE_NUMBER_I64:
                                /* skip value */
                                memfile_skip(&it->memfile, sizeof(u64));
                                break;
                        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                                /* skip value */
                                memfile_skip(&it->memfile, sizeof(float));
                                break;
                        default:
                        error(&it->err, NG5_ERR_INTERNALERR);
                        }
                        c = *memfile_peek(&it->memfile, sizeof(char));

                } else {
                        while ((c = *memfile_peek(&it->memfile, sizeof(char))) == 0)
                        {
                                memfile_skip(&it->memfile, sizeof(char));
                        }
                }
        }
        assert(c == BISON_MARKER_COLUMN_END);
        ng5_optional_set(nvalues, contained_values);
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