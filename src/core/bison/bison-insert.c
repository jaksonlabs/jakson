/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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
#include "core/bison/bison-media.h"

static bool push_media_type(struct bison_insert *inserter, enum bison_field_type type)
{
        error_if_null(inserter)
        char c = *memfile_peek(&inserter->memfile, 1);
        if (unlikely(c != 0)) {
                /* not enough space; enlarge container */
                memfile_move(&inserter->memfile, sizeof(media_type_t));
        }
        bison_media_write(&inserter->memfile, type);
        return true;
}

NG5_EXPORT(bool) bison_insert_create(struct bison_insert *inserter, struct bison_array_it *context)
{
        error_if_null(inserter)
        error_if_null(context)
        bison_array_it_lock(context);
        inserter->context = context;
        memfile_dup(&inserter->memfile, &context->memfile);
        error_init(&inserter->err);
        inserter->position = memfile_tell(&context->memfile);
        return true;
}

NG5_EXPORT(bool) bison_insert_null(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_NULL);
}

NG5_EXPORT(bool) bison_insert_true(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_TRUE);
}

NG5_EXPORT(bool) bison_insert_false(struct bison_insert *inserter)
{
        return push_media_type(inserter, BISON_FIELD_TYPE_FALSE);
}

NG5_EXPORT(bool) bison_insert_drop(struct bison_insert *inserter)
{
        error_if_null(inserter)
        bison_array_it_unlock(inserter->context);
        return true;
}