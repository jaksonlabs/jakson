/*
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

#include "carbon/carbon-error.h"

CARBON_EXPORT(bool)
carbon_error_init(carbon_err_t *err)
{
    if (err) {
        err->code = CARBON_ERR_NOERR;
        err->details = NULL;
        err->file = NULL;
        err->line = 0;
    }
    return (err != NULL);
}

CARBON_EXPORT(bool)
carbon_error_cpy(carbon_err_t *dst, const carbon_err_t *src)
{
    CARBON_NON_NULL_OR_ERROR(dst);
    CARBON_NON_NULL_OR_ERROR(src);
    *dst = *src;
    return true;
}

CARBON_EXPORT(bool)
carbon_error_drop(carbon_err_t *err)
{
    CARBON_NON_NULL_OR_ERROR(err);
    if (err->details) {
        free(err->details);
        err->details = NULL;
    }
    return true;
}

CARBON_EXPORT(bool)
carbon_error_set(carbon_err_t *err, int code, const char *file, uint32_t line)
{
    return carbon_error_set_wdetails(err, code, file, line, NULL);
}

CARBON_EXPORT(bool)
carbon_error_set_wdetails(carbon_err_t *err, int code, const char *file, uint32_t line, const char *details)
{
    if (err) {
        err->code = code;
        err->file = file;
        err->line = line;
        err->details = details ? strdup(details) : NULL;
#ifndef NDEBUG
        carbon_error_print_and_abort(err);
#endif
    }
    return (err != NULL);
}

CARBON_EXPORT(bool)
carbon_error_str(const char **errstr, const char **file, uint32_t *line, bool *details, const char **detailsstr,
                 const carbon_err_t *err)
{
    if (err) {
        if (err->code >= _carbon_nerr_str) {
            CARBON_OPTIONAL_SET(errstr, CARBON_ERRSTR_ILLEGAL_CODE)
        } else {
            CARBON_OPTIONAL_SET(errstr, _carbon_err_str[err->code])
        }
        CARBON_OPTIONAL_SET(file, err->file)
        CARBON_OPTIONAL_SET(line, err->line)
        CARBON_OPTIONAL_SET(details, err->details != NULL);
        CARBON_OPTIONAL_SET(detailsstr, err->details)
        return true;
    }
    return false;
}

CARBON_EXPORT(bool)
carbon_error_print(const carbon_err_t *err)
{
    if (err) {
        const char *errstr;
        const char *file;
        uint32_t line;
        bool has_details;
        const char *details;
        if (carbon_error_str(&errstr, &file, &line, &has_details, &details, err)) {
            fprintf(stderr, "*** ERROR ***   %s\n", errstr);
            fprintf(stderr, "                details: %s\n", has_details ? details : "no details");
            fprintf(stderr, "                source.: %s(%d)\n", file, line);
        } else {
            fprintf(stderr, "*** ERROR ***   internal error during error information fetch");
        }
    }
    return (err != NULL);
}

CARBON_EXPORT(bool)
carbon_error_print_and_abort(const carbon_err_t *err)
{
    carbon_error_print(err);
    abort();
}