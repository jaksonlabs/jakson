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

#include <jak_error.h>

bool error_init(struct jak_error *err)
{
        if (err) {
                err->code = JAK_ERR_NOERR;
                err->details = NULL;
                err->file = NULL;
                err->line = 0;
        }
        return (err != NULL);
}

bool error_cpy(struct jak_error *dst, const struct jak_error *src)
{
        error_if_null(dst);
        error_if_null(src);
        *dst = *src;
        return true;
}

bool error_drop(struct jak_error *err)
{
        error_if_null(err);
        if (err->details) {
                free(err->details);
                err->details = NULL;
        }
        return true;
}

bool error_set(struct jak_error *err, int code, const char *file, jak_u32 line)
{
        return error_set_wdetails(err, code, file, line, NULL);
}

bool error_set_wdetails(struct jak_error *err, int code, const char *file, jak_u32 line, const char *details)
{
        if (err) {
                err->code = code;
                err->file = file;
                err->line = line;
                err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
                error_print_and_abort(err);
#endif
        }
        return (err != NULL);
}

bool error_set_no_abort(struct jak_error *err, int code, const char *file, jak_u32 line)
{
        return error_set_wdetails_no_abort(err, code, file, line, NULL);
}

bool error_set_wdetails_no_abort(struct jak_error *err, int code, const char *file, jak_u32 line, const char *details)
{
        if (err) {
                err->code = code;
                err->file = file;
                err->line = line;
                err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
                error_print(code);
#endif
        }
        return (err != NULL);
}

bool error_str(const char **errstr, const char **file, jak_u32 *line, bool *details, const char **detailsstr,
               const struct jak_error *err)
{
        if (err) {
                if (err->code >= _nerr_str) {
                        JAK_optional_set(errstr, JAK_ERRSTR_ILLEGAL_CODE)
                } else {
                        JAK_optional_set(errstr, _err_str[err->code])
                }
                JAK_optional_set(file, err->file)
                JAK_optional_set(line, err->line)
                JAK_optional_set(details, err->details != NULL);
                JAK_optional_set(detailsstr, err->details)
                return true;
        }
        return false;
}

bool error_print_to_stderr(const struct jak_error *err)
{
        if (err) {
                const char *errstr;
                const char *file;
                jak_u32 line;
                bool has_details;
                const char *details;
                if (error_str(&errstr, &file, &line, &has_details, &details, err)) {
                        fprintf(stderr, "*** ERROR ***   %s\n", errstr);
                        fprintf(stderr, "                details: %s\n", has_details ? details : "no details");
                        fprintf(stderr, "                source.: %s(%d)\n", file, line);
                } else {
                        fprintf(stderr, "*** ERROR ***   internal error during error information fetch");
                }
                fflush(stderr);
        }
        return (err != NULL);
}

bool error_print_and_abort(const struct jak_error *err)
{
        error_print_to_stderr(err);
        abort();
}