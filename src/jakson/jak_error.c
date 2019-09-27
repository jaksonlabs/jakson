/**
 * Copyright 2018 Marcus Pinnecke
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

#include <jakson/jak_error.h>

_Thread_local jak_error jak_global_error;

bool jak_error_init(jak_error *err)
{
        if (err) {
                err->code = JAK_ERR_NOERR;
                err->details = NULL;
                err->file = NULL;
                err->line = 0;
        }
        return (err != NULL);
}

bool jak_error_cpy(jak_error *dst, const jak_error *src)
{
        JAK_ERROR_IF_NULL(dst);
        JAK_ERROR_IF_NULL(src);
        *dst = *src;
        return true;
}

bool jak_error_drop(jak_error *err)
{
        JAK_ERROR_IF_NULL(err);
        if (err->details) {
                free(err->details);
                err->details = NULL;
        }
        return true;
}

bool jak_error_set(jak_error *err, int code, const char *file, jak_u32 line)
{
        return jak_error_set_wdetails(err, code, file, line, NULL);
}

bool jak_error_set_wdetails(jak_error *err, int code, const char *file, jak_u32 line, const char *details)
{
        if (err) {
                err->code = code;
                err->file = file;
                err->line = line;
                err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
                jak_error_print_to_stderr(err);
#endif
        }
        return (err != NULL);
}

bool jak_error_set_no_abort(jak_error *err, int code, const char *file, jak_u32 line)
{
        return jak_error_set_wdetails_no_abort(err, code, file, line, NULL);
}

bool jak_error_set_wdetails_no_abort(jak_error *err, int code, const char *file, jak_u32 line, const char *details)
{
        if (err) {
                err->code = code;
                err->file = file;
                err->line = line;
                err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
                jak_error_print_to_stderr(err);
#endif
        }
        return (err != NULL);
}

bool jak_error_str(const char **errstr, const char **file, jak_u32 *line, bool *details, const char **detailsstr,
               const jak_error *err)
{
        if (err) {
                if (err->code >= jak_global_nerr_str) {
                        JAK_OPTIONAL_SET(errstr, JAK_ERRSTR_ILLEGAL_CODE)
                } else {
                        JAK_OPTIONAL_SET(errstr, jak_global_err_str[err->code])
                }
                JAK_OPTIONAL_SET(file, err->file)
                JAK_OPTIONAL_SET(line, err->line)
                JAK_OPTIONAL_SET(details, err->details != NULL);
                JAK_OPTIONAL_SET(detailsstr, err->details)
                return true;
        }
        return false;
}

bool jak_error_print_to_stderr(const jak_error *err)
{
        if (err) {
                const char *errstr;
                const char *file;
                jak_u32 line;
                bool has_details;
                const char *details;
                if (jak_error_str(&errstr, &file, &line, &has_details, &details, err)) {
                        fprintf(stderr, "*** ERROR ***   %s\n", errstr);
                        fprintf(stderr, "                details: %s\n", has_details ? details : "no details");
                        fprintf(stderr, "                source.: %s(%d)\n", file, line);
                } else {
                        fprintf(stderr, "*** ERROR ***   internal JAK_ERROR during JAK_ERROR information fetch");
                }
                fflush(stderr);
        }
        return (err != NULL);
}

bool jak_error_print_and_abort(const jak_error *err)
{
        jak_error_print_to_stderr(err);
        abort();
}