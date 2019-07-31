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

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/carbon/oid/oid.h>
#include <ark-js/carbon/carbon.h>

ARK_BEGIN_DECL

ARK_DEFINE_ERROR_GETTER(carbon_revise)

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
ARK_EXPORT(bool) carbon_revise_try_begin(struct carbon_revise *context, struct carbon *revised_doc, struct carbon *doc);

ARK_EXPORT(bool) carbon_revise_begin(struct carbon_revise *context, struct carbon *revised_doc, struct carbon *original);

ARK_EXPORT(bool) carbon_revise_key_generate(object_id_t *out, struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_key_set_unsigned(struct carbon_revise *context, u64 key_value);

ARK_EXPORT(bool) carbon_revise_key_set_signed(struct carbon_revise *context, i64 key_value);

ARK_EXPORT(bool) carbon_revise_key_set_string(struct carbon_revise *context, const char *key_value);

ARK_EXPORT(bool) carbon_revise_iterator_open(struct carbon_array_it *it, struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_iterator_close(struct carbon_array_it *it);

ARK_EXPORT(bool) carbon_revise_find_open(struct carbon_find *out, const char *dot_path, struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_find_close(struct carbon_find *find);

ARK_EXPORT(bool) carbon_revise_remove(const char *dot_path, struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_remove_one(const char *dot_path, struct carbon *rev_doc, struct carbon *doc);

ARK_EXPORT(bool) carbon_revise_pack(struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_shrink(struct carbon_revise *context);

ARK_EXPORT(const struct carbon *) carbon_revise_end(struct carbon_revise *context);

ARK_EXPORT(bool) carbon_revise_abort(struct carbon_revise *context);

ARK_END_DECL

#endif
