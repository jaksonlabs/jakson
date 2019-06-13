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

#include "core/bison/bison.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-media.h"
#include "core/bison/bison-int.h"
#include "utils/hexdump.h"

static void marker_insert(struct memfile *memfile, u8 marker);

NG5_EXPORT(bool) bison_int_insert_array(struct memfile *memfile, size_t nbytes)
{
        error_if_null(memfile);

        u8 array_begin_marker = BISON_MARKER_ARRAY_BEGIN;
        u8 array_end_marker = BISON_MARKER_ARRAY_END;

        bison_int_ensure_space(memfile, sizeof(u8));
        marker_insert(memfile, array_begin_marker);

        bison_int_ensure_space(memfile, nbytes + sizeof(u8));

        offset_t payload_begin = memfile_tell(memfile);
        memfile_seek(memfile, payload_begin + nbytes);
        marker_insert(memfile, array_end_marker);

        /* seek to first entry in array */
        memfile_seek(memfile, payload_begin);

        return true;
}

NG5_EXPORT(bool) bison_int_insert_column(struct memfile *memfile_in, struct err *err_in, enum bison_field_type type, size_t capactity)
{
        error_if_null(memfile_in);
        error_if_null(err_in);

        ng5_unused(capactity);

        switch (type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I8:
        case BISON_FIELD_TYPE_NUMBER_I16:
        case BISON_FIELD_TYPE_NUMBER_I32:
        case BISON_FIELD_TYPE_NUMBER_I64:
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                break; /* all types from above are fixed-length and therefore supported in a column */
        default:
                error_with_details(err_in, NG5_ERR_BADTYPE, "BISON column supports fixed-length types only")
        }

        u8 column_begin_marker = BISON_MARKER_COLUMN_BEGIN;
        u8 column_end_marker = BISON_MARKER_COLUMN_END;

        bison_int_ensure_space(memfile_in, sizeof(u8));
        marker_insert(memfile_in, column_begin_marker);
        bison_int_ensure_space(memfile_in, sizeof(media_type_t));
        bison_media_write(memfile_in, type);
        offset_t payload_begin = memfile_tell(memfile_in);

        size_t type_size = sizeof(media_type_t); /* at least the media type marker is required */
        switch (type) {
        case BISON_FIELD_TYPE_NULL:
        case BISON_FIELD_TYPE_TRUE:
        case BISON_FIELD_TYPE_FALSE:
                /* only media type marker is required */
                break;
        case BISON_FIELD_TYPE_NUMBER_U8:
        case BISON_FIELD_TYPE_NUMBER_I8:
                type_size += sizeof(u8);
                break;
        case BISON_FIELD_TYPE_NUMBER_U16:
        case BISON_FIELD_TYPE_NUMBER_I16:
                type_size += sizeof(u16);
                break;
        case BISON_FIELD_TYPE_NUMBER_U32:
        case BISON_FIELD_TYPE_NUMBER_I32:
                type_size += sizeof(u32);
                break;
        case BISON_FIELD_TYPE_NUMBER_U64:
        case BISON_FIELD_TYPE_NUMBER_I64:
                type_size += sizeof(u64);
                break;
        case BISON_FIELD_TYPE_NUMBER_FLOAT:
                type_size += sizeof(float);
                break;
        default:
                error(err_in, NG5_ERR_INTERNALERR);
        }

        size_t nbytes = capactity * type_size;
        bison_int_ensure_space(memfile_in, nbytes + sizeof(u8));


        memfile_seek(memfile_in, payload_begin + nbytes);
        marker_insert(memfile_in, column_end_marker);

        /* seek to first entry in column */
        memfile_seek(memfile_in, payload_begin);

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