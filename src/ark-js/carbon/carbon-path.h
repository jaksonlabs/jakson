/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef carbon_PATH_H
#define carbon_PATH_H

#include <ark-js/shared/common.h>
#include <ark-js/carbon/carbon-dot.h>

NG5_BEGIN_DECL

struct carbon_path_evaluator
{
        struct carbon *doc;
        struct carbon_array_it root_it;
        enum carbon_path_status status;
        struct err err;

        struct {
                enum carbon_container_type container_type;
                union {
                        struct {
                                struct carbon_array_it *it;
                        } array;


                        struct {
                                struct carbon_column_it *it;
                                u32 elem_pos;
                        } column;

                } containers;
        } result;
};

NG5_EXPORT(bool) carbon_path_evaluator_begin(struct carbon_path_evaluator *eval, struct carbon_dot_path *path,
        struct carbon *doc);

NG5_EXPORT(bool) carbon_path_evaluator_begin_mutable(struct carbon_path_evaluator *eval, const struct carbon_dot_path *path,
        struct carbon_revise *context);

NG5_EXPORT(bool) carbon_path_evaluator_status(enum carbon_path_status *status, struct carbon_path_evaluator *state);

NG5_EXPORT(bool) carbon_path_evaluator_has_result(struct carbon_path_evaluator *state);

NG5_EXPORT(bool) carbon_path_evaluator_end(struct carbon_path_evaluator *state);

NG5_EXPORT(bool) carbon_path_exists(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_array(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_column(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_object(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_container(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_null(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_number(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_boolean(struct carbon *doc, const char *path);

NG5_EXPORT(bool) carbon_path_is_string(struct carbon *doc, const char *path);

NG5_END_DECL

#endif
