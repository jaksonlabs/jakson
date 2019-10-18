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

#ifndef HAD_CARBON_PATCH_H
#define HAD_CARBON_PATCH_H

#include <jakson/stdinc.h>

BEGIN_DECL

typedef struct carbon carbon;
typedef struct carbon_array_it carbon_array_it;
typedef struct carbon_find carbon_find;

/* Opens a read-write enabled iterator for patching a record revision without creating a new one. */
fn_result carbon_patch_begin(carbon_array_it *it, carbon *doc);

/* Closes a read-write ennabled iterator , which was previously opened via 'carbon_patch_end' */
fn_result carbon_patch_end(carbon_array_it *it);

bool carbon_patch_find_begin(carbon_find *out, const char *dot_path, carbon *doc);

fn_result carbon_patch_find_end(carbon_find *find);

END_DECL

#endif
