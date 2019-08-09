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

#include <ark-js/carbon/archive/archive.h>
#include <ark-js/shared/async/spin.h>
#include <ark-js/carbon/archive/archive-io.h>

struct io_context {
        struct err err;
        FILE *file;
        struct spinlock lock;
        offset_t last_pos;
};

ARK_EXPORT(bool) io_context_create(struct io_context **context, struct err *err, const char *file_path)
{
        error_if_null(context);
        error_if_null(err);
        error_if_null(file_path);

        struct io_context *result = ark_malloc(sizeof(struct io_context));

        if (!result) {
                error(err, ARK_ERR_MALLOCERR);
                return false;
        }

        spin_init(&result->lock);
        error_init(&result->err);

        result->file = fopen(file_path, "r");

        if (!result->file) {
                error(err, ARK_ERR_FOPEN_FAILED);
                result->file = NULL;
                return false;
        } else {
                *context = result;
                return true;
        }
}

ARK_EXPORT(struct err *)io_context_get_error(struct io_context *context)
{
        return context ? &context->err : NULL;
}

ARK_EXPORT(FILE *)io_context_lock_and_access(struct io_context *context)
{
        if (context) {
                spin_acquire(&context->lock);
                context->last_pos = ftell(context->file);
                return context->file;
        } else {
                error(&context->err, ARK_ERR_NULLPTR);
                return NULL;
        }
}

ARK_EXPORT(bool) io_context_unlock(struct io_context *context)
{
        if (context) {
                fseek(context->file, context->last_pos, SEEK_SET);
                spin_release(&context->lock);
                return true;
        } else {
                error(&context->err, ARK_ERR_NULLPTR);
                return false;
        }
}

ARK_EXPORT(bool) io_context_drop(struct io_context *context)
{
        error_if_null(context);
        ark_optional(context->file != NULL, fclose(context->file);
                context->file = NULL)
        free(context);
        return true;
}