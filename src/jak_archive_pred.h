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

#ifndef JAK_STRING_PRED_H
#define JAK_STRING_PRED_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>

JAK_BEGIN_DECL

typedef bool
(*jak_string_pred_func_t)(size_t *idxs_matching, size_t *num_matching, char **strings, size_t num_strings,
                          void *capture);

struct jak_string_pred_t {
        jak_string_pred_func_t func;
        jak_i64 limit;
};

JAK_BUILT_IN(static bool) jak_string_pred_validate(struct jak_error *err, const struct jak_string_pred_t *pred)
{
        JAK_ERROR_IF_NULL(pred);
        JAK_ERROR_IF_NOT_IMPLEMENTED(err, pred, func)
        return true;
}

JAK_BUILT_IN(static bool) jak_string_pred_eval(const struct jak_string_pred_t *pred, size_t *idxs_matching,
                                               size_t *num_matching, char **strings, size_t num_strings, void *capture)
{
        JAK_ASSERT(pred);
        JAK_ASSERT(idxs_matching);
        JAK_ASSERT(num_matching);
        JAK_ASSERT(strings);
        JAK_ASSERT(pred->func);
        return pred->func(idxs_matching, num_matching, strings, num_strings, capture);
}

JAK_BUILT_IN(static bool) jak_string_pred_get_limit(jak_i64 *limit, const struct jak_string_pred_t *pred)
{
        JAK_ERROR_IF_NULL(limit);
        JAK_ERROR_IF_NULL(pred);
        *limit = pred->limit;
        return true;
}

JAK_END_DECL

#endif
