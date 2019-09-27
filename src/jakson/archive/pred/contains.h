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

#ifndef STRING_PRED_CONTAINS_H
#define STRING_PRED_CONTAINS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/archive/pred.h>

BEGIN_DECL

BUILT_IN(static bool) __string_pred_contains_func(size_t *idxs_matching, size_t *num_matching, char **strings, size_t num_strings, void *capture)
{
        size_t result_size = 0;
        const char *needle = (const char *) capture;

        for (size_t i = 0; i < num_strings; i++) {
                if (strstr(strings[i], needle) != NULL) {
                        idxs_matching[result_size++] = i;
                }
        }

        *num_matching = result_size;

        return true;
}

BUILT_IN(static bool)

string_pred_contains_init(string_pred *pred)
{
        ERROR_IF_NULL(pred);
        pred->limit = QUERY_LIMIT_NONE;
        pred->func = __string_pred_contains_func;
        return true;
}

END_DECL

#endif
