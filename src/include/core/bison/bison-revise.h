/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef BISON_REVISE_H
#define BISON_REVISE_H

#include "shared/common.h"
#include "shared/error.h"
#include "core/oid/oid.h"
#include "core/bison/bison.h"

NG5_BEGIN_DECL

NG5_DEFINE_ERROR_GETTER(bison_revise)

/**
 * Acquires a new revision context for the bison document.
 *
 * In case of an already running revision, the function returns <code>false</code> without blocking.
 * Otherwise, <code>bison_revise_begin</code> is called internally.
 *
 * @param context non-null pointer to revision context
 * @param doc document that should be revised
 * @return <code>false</code> in case of an already running revision. Otherwise returns value of
 *                            <code>bison_revise_begin</code>
 */
NG5_EXPORT(bool) bison_revise_try_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *doc);

NG5_EXPORT(bool) bison_revise_begin(struct bison_revise *context, struct bison *revised_doc, struct bison *original);

NG5_EXPORT(bool) bison_revise_gen_object_id(object_id_t *out, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_iterator_open(struct bison_array_it *it, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_iterator_close(struct bison_array_it *it);

NG5_EXPORT(bool) bison_revise_find_open(struct bison_find *out, const char *dot_path, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_find_close(struct bison_find *find);

NG5_EXPORT(bool) bison_revise_remove(const char *dot_path, struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_remove_one(const char *dot_path, struct bison *rev_doc, struct bison *doc);

NG5_EXPORT(bool) bison_revise_pack(struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_shrink(struct bison_revise *context);

NG5_EXPORT(const struct bison *) bison_revise_end(struct bison_revise *context);

NG5_EXPORT(bool) bison_revise_abort(struct bison_revise *context);

NG5_END_DECL

#endif
