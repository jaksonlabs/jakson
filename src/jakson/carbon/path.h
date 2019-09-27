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

#ifndef CARBON_PATH_H
#define CARBON_PATH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/carbon/dot.h>
#include <jakson/carbon/containers.h>

BEGIN_DECL

typedef struct carbon_path_evaluator {
        carbon *doc;
        carbon_array_it root_it;
        carbon_path_status_e status;
        err err;
        struct {
                carbon_container_e container_type;
                union {
                        struct {
                                carbon_array_it it;
                        } array;

                        struct {
                                carbon_object_it it;
                        } object;

                        struct {
                                carbon_column_it it;
                                u32 elem_pos;
                        } column;

                } containers;
        } result;
} carbon_path_evaluator;

bool carbon_path_evaluator_begin(carbon_path_evaluator *eval, carbon_dot_path *path, carbon *doc);
bool carbon_path_evaluator_begin_mutable(carbon_path_evaluator *eval, const carbon_dot_path *path, carbon_revise *context);
bool carbon_path_evaluator_end(carbon_path_evaluator *state);

bool carbon_path_evaluator_status(carbon_path_status_e *status, carbon_path_evaluator *state);
bool carbon_path_evaluator_has_result(carbon_path_evaluator *state);
bool carbon_path_exists(carbon *doc, const char *path);

bool carbon_path_is_array(carbon *doc, const char *path);
bool carbon_path_is_column(carbon *doc, const char *path);
bool carbon_path_is_object(carbon *doc, const char *path);
bool carbon_path_is_container(carbon *doc, const char *path);
bool carbon_path_is_null(carbon *doc, const char *path);
bool carbon_path_is_number(carbon *doc, const char *path);
bool carbon_path_is_boolean(carbon *doc, const char *path);
bool carbon_path_is_string(carbon *doc, const char *path);

END_DECL

#endif
