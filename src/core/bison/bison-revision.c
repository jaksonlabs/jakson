/**
 * Copyright 2019 Marcus Pinnecke
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

#include "core/bison/bison-revision.h"
#include "stdx/varuint.h"

NG5_EXPORT(bool) bison_revision_create(struct memfile *file)
{
        error_if_null(file)

        varuint_t revision = NG5_MEMFILE_PEEK(file, varuint_t);

        /* in case not enough space is available for writing revision (variable-length) number, enlarge */
        size_t remain = memfile_remain_size(file);
        offset_t rev_off = memfile_tell(file);
        if (unlikely(remain < varuint_max_blocks())) {
                memfile_write_zero(file, varuint_max_blocks());
                memfile_seek(file, rev_off);
        }

        u8 bytes_written = varuint_write(revision, 0);
        memfile_skip(file, bytes_written);

        return true;
}

NG5_EXPORT(bool) bison_revision_skip(struct memfile *file)
{
        error_if_null(file)
        memfile_read_varuint(NULL, file);
        return true;
}

NG5_EXPORT(bool) bison_revision_read(u64 *revision, struct memfile *file)
{
        error_if_null(file)
        error_if_null(revision)
        *revision = memfile_read_varuint(NULL, file);
        return true;
}

NG5_EXPORT(bool) bison_revision_peek(u64 *revision, struct memfile *file)
{
        error_if_null(file)
        error_if_null(revision)
        *revision = memfile_peek_varuint(NULL, file);
        return true;
}

NG5_EXPORT(bool) bison_revision_inc(struct memfile *file)
{
        u64 rev;
        bison_revision_peek(&rev, file);
        rev++;
        memfile_update_varuint(file, rev);
        return true;
}
