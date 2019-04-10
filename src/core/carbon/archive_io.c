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

#include "core/carbon/archive.h"
#include "core/async/spin.h"
#include "core/carbon/archive_io.h"

typedef struct carbon_io_context
{
    struct err err;
    FILE *file;
    carbon_spinlock_t lock;
    offset_t last_pos;
} carbon_io_context_t;


NG5_EXPORT(bool)
carbon_io_context_create(carbon_io_context_t **context, struct err *err, const char *file_path)
{
    NG5_NON_NULL_OR_ERROR(context);
    NG5_NON_NULL_OR_ERROR(err);
    NG5_NON_NULL_OR_ERROR(file_path);

    carbon_io_context_t *result = malloc(sizeof(carbon_io_context_t));

    if (!result) {
        error(err, NG5_ERR_MALLOCERR);
        return false;
    }

    carbon_spinlock_init(&result->lock);
    carbon_error_init(&result->err);

    result->file = fopen(file_path, "r");

    if (!result->file) {
        error(err, NG5_ERR_FOPEN_FAILED);
        result->file = NULL;
        return false;
    } else {
        *context = result;
        return true;
    }
}

NG5_EXPORT(struct err *)
carbon_io_context_get_error(carbon_io_context_t *context)
{
    return context ? &context->err : NULL;
}

NG5_EXPORT(FILE *)
carbon_io_context_lock_and_access(carbon_io_context_t *context)
{
    if (context) {
        carbon_spinlock_acquire(&context->lock);
        context->last_pos = ftell(context->file);
        return context->file;
    } else {
        error(&context->err, NG5_ERR_NULLPTR);
        return NULL;
    }
}

NG5_EXPORT(bool)
carbon_io_context_unlock(carbon_io_context_t *context)
{
    if (context) {
        fseek(context->file, context->last_pos, SEEK_SET);
        carbon_spinlock_release(&context->lock);
        return true;
    } else {
        error(&context->err, NG5_ERR_NULLPTR);
        return false;
    }
}

NG5_EXPORT(bool)
carbon_io_context_drop(carbon_io_context_t *context)
{
    NG5_NON_NULL_OR_ERROR(context);
    NG5_OPTIONAL(context->file != NULL, fclose(context->file); context->file = NULL)
    free(context);
    return true;
}