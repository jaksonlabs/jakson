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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/archive.h>
#include <jakson/std/jak_spinlock.h>
#include <jakson/archive/jak_archive_io.h>

typedef struct jak_archive_io_context {
        jak_error err;
        FILE *file;
        jak_spinlock lock;
        jak_offset_t last_pos;
} jak_archive_io_context;

bool jak_io_context_create(jak_archive_io_context **context, jak_error *err, const char *file_path)
{
        JAK_ERROR_IF_NULL(context);
        JAK_ERROR_IF_NULL(err);
        JAK_ERROR_IF_NULL(file_path);

        jak_archive_io_context *result = JAK_MALLOC(sizeof(jak_archive_io_context));

        if (!result) {
                JAK_ERROR(err, JAK_ERR_MALLOCERR);
                return false;
        }

        jak_spinlock_init(&result->lock);
        jak_error_init(&result->err);

        result->file = fopen(file_path, "r");

        if (!result->file) {
                JAK_ERROR(err, JAK_ERR_FOPEN_FAILED);
                result->file = NULL;
                return false;
        } else {
                *context = result;
                return true;
        }
}

jak_error *jak_io_context_get_error(jak_archive_io_context *context)
{
        return context ? &context->err : NULL;
}

FILE *jak_io_context_lock_and_access(jak_archive_io_context *context)
{
        if (context) {
                jak_spinlock_acquire(&context->lock);
                context->last_pos = ftell(context->file);
                return context->file;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_NULLPTR);
                return NULL;
        }
}

bool jak_io_context_unlock(jak_archive_io_context *context)
{
        if (context) {
                fseek(context->file, context->last_pos, SEEK_SET);
                jak_spinlock_release(&context->lock);
                return true;
        } else {
                JAK_ERROR(&context->err, JAK_ERR_NULLPTR);
                return false;
        }
}

bool jak_io_context_drop(jak_archive_io_context *context)
{
        JAK_ERROR_IF_NULL(context);
        JAK_OPTIONAL(context->file != NULL, fclose(context->file);
                context->file = NULL)
        free(context);
        return true;
}