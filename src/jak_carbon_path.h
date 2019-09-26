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

#ifndef JAK_CARBON_PATH_H
#define JAK_CARBON_PATH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_containers.h>

JAK_BEGIN_DECL

typedef struct jak_carbon_path_evaluator {
        jak_carbon *doc;
        jak_carbon_array_it root_it;
        jak_carbon_path_status_e status;
        jak_error err;
        struct {
                jak_carbon_container_e container_type;
                union {
                        struct {
                                jak_carbon_array_it it;
                        } array;

                        struct {
                                jak_carbon_object_it it;
                        } object;

                        struct {
                                jak_carbon_column_it it;
                                jak_u32 elem_pos;
                        } column;

                } containers;
        } result;
} jak_carbon_path_evaluator;

bool jak_carbon_path_evaluator_begin(jak_carbon_path_evaluator *eval, jak_carbon_dot_path *path, jak_carbon *doc);
bool jak_carbon_path_evaluator_begin_mutable(jak_carbon_path_evaluator *eval, const jak_carbon_dot_path *path, jak_carbon_revise *context);
bool jak_carbon_path_evaluator_end(jak_carbon_path_evaluator *state);

bool jak_carbon_path_evaluator_status(jak_carbon_path_status_e *status, jak_carbon_path_evaluator *state);
bool jak_carbon_path_evaluator_has_result(jak_carbon_path_evaluator *state);
bool jak_carbon_path_exists(jak_carbon *doc, const char *path);

bool jak_carbon_path_is_array(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_column(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_object(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_container(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_null(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_number(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_boolean(jak_carbon *doc, const char *path);
bool jak_carbon_path_is_string(jak_carbon *doc, const char *path);

JAK_END_DECL

#endif
