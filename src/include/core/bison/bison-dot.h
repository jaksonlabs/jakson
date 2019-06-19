/**
 * BISON Adaptive Binary JSON -- Copyright 2019 Marcus Pinnecke
 * This file implements the document format itself
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

#ifndef BISON_DOT_H
#define BISON_DOT_H

#include "shared/common.h"
#include "shared/error.h"
#include "shared/types.h"
#include "std/string_builder.h"

NG5_BEGIN_DECL

enum bison_dot_node_type
{
        DOT_NODE_ARRAY_IDX,
        DOT_NODE_KEY_NAME
};

struct bison_dot_node
{
        enum bison_dot_node_type type;
        union {
                char *string;
                u32 idx;
        } identifier;
};

struct bison_dot_path
{
        struct bison_dot_node nodes[256];
        u32 path_len;
        struct err err;
};

NG5_DEFINE_ERROR_GETTER(bison_dot_path)

NG5_EXPORT(bool) bison_dot_path_create(struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_from_string(struct bison_dot_path *path, const char *path_string);

NG5_EXPORT(bool) bison_dot_path_add_key(struct bison_dot_path *dst, const char *key);

NG5_EXPORT(bool) bison_dot_path_add_nkey(struct bison_dot_path *dst, const char *key, size_t len);

NG5_EXPORT(bool) bison_dot_path_add_idx(struct bison_dot_path *dst, u32 idx);

NG5_EXPORT(bool) bison_dot_path_len(u32 *len, const struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_is_empty(const struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_type_at(enum bison_dot_node_type *type_out, u32 pos, struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_idx_at(u32 *idx, u32 pos, struct bison_dot_path *path);

NG5_EXPORT(const char *) bison_dot_path_key_at(u32 pos, struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_drop(struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_to_str(struct string_builder *sb, struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_fprint(FILE *file, struct bison_dot_path *path);

NG5_EXPORT(bool) bison_dot_path_print(struct bison_dot_path *path);

NG5_END_DECL

#endif
