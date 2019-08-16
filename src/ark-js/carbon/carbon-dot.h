/**
 * Columnar Binary JSON -- Copyright 2019 Marcus Pinnecke
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

#ifndef CARBON_DOT_H
#define CARBON_DOT_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/error.h>
#include <ark-js/shared/types.h>
#include <ark-js/shared/stdx/string.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-column-it.h>
#include <ark-js/carbon/carbon-object-it.h>

ARK_BEGIN_DECL

struct carbon; /* forwarded from carbon.h */

enum carbon_dot_node_type {
    DOT_NODE_ARRAY_IDX,
    DOT_NODE_KEY_NAME
};

struct carbon_dot_node {
    enum carbon_dot_node_type type;
    union {
        char *string;
        u32 idx;
    } identifier;
};

struct carbon_dot_path {
    struct carbon_dot_node nodes[256];
    u32 path_len;
    struct err err;
};

enum carbon_path_status {
    CARBON_PATH_RESOLVED,
    CARBON_PATH_EMPTY_DOC,
    CARBON_PATH_NOSUCHINDEX,
    CARBON_PATH_NOSUCHKEY,
    CARBON_PATH_NOTTRAVERSABLE,
    CARBON_PATH_NOCONTAINER,
    CARBON_PATH_NOTANOBJECT,
    CARBON_PATH_NONESTING,
    CARBON_PATH_INTERNAL
};

ARK_DEFINE_ERROR_GETTER(carbon_dot_path)

bool carbon_dot_path_create(struct carbon_dot_path *path);

bool carbon_dot_path_from_string(struct carbon_dot_path *path, const char *path_string);

bool carbon_dot_path_add_key(struct carbon_dot_path *dst, const char *key);

bool carbon_dot_path_add_nkey(struct carbon_dot_path *dst, const char *key, size_t len);

bool carbon_dot_path_add_idx(struct carbon_dot_path *dst, u32 idx);

bool carbon_dot_path_len(u32 *len, const struct carbon_dot_path *path);

bool carbon_dot_path_is_empty(const struct carbon_dot_path *path);

bool carbon_dot_path_type_at(enum carbon_dot_node_type *type_out, u32 pos, const struct carbon_dot_path *path);

bool carbon_dot_path_idx_at(u32 *idx, u32 pos, const struct carbon_dot_path *path);

const char *carbon_dot_path_key_at(u32 pos, const struct carbon_dot_path *path);

bool carbon_dot_path_drop(struct carbon_dot_path *path);

bool carbon_dot_path_to_str(struct string *sb, struct carbon_dot_path *path);

bool carbon_dot_path_fprint(FILE *file, struct carbon_dot_path *path);

bool carbon_dot_path_print(struct carbon_dot_path *path);

ARK_END_DECL

#endif
