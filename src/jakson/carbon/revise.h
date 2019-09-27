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

#ifndef CARBON_REVISE_H
#define CARBON_REVISE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/stdx/unique_id.h>
#include <jakson/carbon.h>

BEGIN_DECL

/**
 * Acquires a new revision context for the carbon document.
 *
 * In case of an already running revision, the function returns <code>false</code> without blocking.
 * Otherwise, <code>carbon_revise_begin</code> is called internally.
 *
 * @param context non-null pointer to revision context
 * @param doc document that should be revised
 * @return <code>false</code> in case of an already running revision. Otherwise returns value of
 *                            <code>carbon_revise_begin</code>
 */
bool carbon_revise_try_begin(carbon_revise *context, carbon *revised_doc, carbon *doc);
bool carbon_revise_begin(carbon_revise *context, carbon *revised_doc, carbon *original);
const carbon *carbon_revise_end(carbon_revise *context);

bool carbon_revise_key_generate(unique_id_t *out, carbon_revise *context);

bool carbon_revise_key_set_unsigned(carbon_revise *context, u64 key_value);
bool carbon_revise_key_set_signed(carbon_revise *context, i64 key_value);
bool carbon_revise_key_set_string(carbon_revise *context, const char *key_value);

bool carbon_revise_iterator_open(carbon_array_it *it, carbon_revise *context);
bool carbon_revise_iterator_close(carbon_array_it *it);

bool carbon_revise_find_open(carbon_find *out, const char *dot_path, carbon_revise *context);
bool carbon_revise_find_close(carbon_find *find);

bool carbon_revise_remove(const char *dot_path, carbon_revise *context);
bool carbon_revise_remove_one(const char *dot_path, carbon *rev_doc, carbon *doc);

bool carbon_revise_pack(carbon_revise *context);
bool carbon_revise_shrink(carbon_revise *context);

bool carbon_revise_abort(carbon_revise *context);

END_DECL

#endif
