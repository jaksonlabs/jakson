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

#include <core/bison/bison.h>
#include "core/bison/bison_array_it.h"

NG5_EXPORT(bool) bison_array_it_create(struct bison_array_it *it, struct bison *doc, offset_t payload_start)
{
        error_if_null(it);
        error_if_null(doc);

        it->doc = doc;
        it->payload_start = payload_start;
        error_init(&it->err);

        bison_array_it_rewind(it);

        return true;
}

NG5_EXPORT(bool) bison_array_it_drop(struct bison_array_it *it, struct bison *doc)
{
        ng5_unused(it);
        ng5_unused(doc);
        return false;
}

NG5_EXPORT(bool) bison_array_it_rewind(struct bison_array_it *it)
{
        error_if_null(it);
        error_if(it->payload_start >= memfile_size(&it->doc->memfile), &it->err, NG5_ERR_OUTOFBOUNDS);
        return memfile_seek(&it->doc->memfile, it->payload_start);
}

NG5_EXPORT(bool) bison_array_it_has_next(struct bison_array_it *it)
{
        error_if_null(it);
        return memfile_peek(&it->doc->memfile, 1) != 0;
}

NG5_EXPORT(bool) bison_array_it_next(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_prev(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
}

NG5_EXPORT(bool) bison_array_it_swap(struct bison_array_it *lhs, struct bison_array_it *rhs)
{
        ng5_unused(lhs);
        ng5_unused(rhs);
        return false;
}

NG5_EXPORT(bool) bison_array_it_push_back(struct bison_array_it *it)
{
        ng5_unused(it);
        return false;
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