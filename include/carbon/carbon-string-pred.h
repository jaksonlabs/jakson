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

#ifndef CARBON_STRING_PRED_H
#define CARBON_STRING_PRED_H

#include "carbon-common.h"

CARBON_BEGIN_DECL

typedef bool (*carbon_string_pred_func_t)(size_t *idxs_matching, size_t *num_matching,
                                     char **strings, size_t num_strings, void *capture);

typedef struct
{
    carbon_string_pred_func_t func;
    int64_t limit;
} carbon_string_pred_t;

CARBON_BUILT_IN(static bool)
carbon_string_pred_validate(carbon_err_t *err, const carbon_string_pred_t *pred)
{
    CARBON_NON_NULL_OR_ERROR(pred);
    CARBON_IMPLEMENTS_OR_ERROR(err, pred, func)
    return true;
}

CARBON_BUILT_IN(static bool)
carbon_string_pred_eval(const carbon_string_pred_t *pred, size_t *idxs_matching, size_t *num_matching,
                        char **strings, size_t num_strings, void *capture)
{
    assert(pred);
    assert(idxs_matching);
    assert(num_matching);
    assert(strings);
    assert(pred->func);
    return pred->func(idxs_matching, num_matching, strings, num_strings, capture);
}

CARBON_BUILT_IN(static bool)
carbon_string_pred_get_limit(int64_t *limit, const carbon_string_pred_t *pred)
{
    CARBON_NON_NULL_OR_ERROR(limit);
    CARBON_NON_NULL_OR_ERROR(pred);
    *limit = pred->limit;
    return true;
}

CARBON_END_DECL

#endif
