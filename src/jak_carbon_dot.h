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

#ifndef JAK_CARBON_DOT_H
#define JAK_CARBON_DOT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_error.h>
#include <jak_types.h>
#include <jak_string.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_column_it.h>
#include <jak_carbon_object_it.h>

JAK_BEGIN_DECL

struct jak_carbon; /* forwarded from carbon.h */

enum carbon_dot_node_type {
    DOT_NODE_ARRAY_IDX,
    DOT_NODE_KEY_NAME
};

struct jak_carbon_dot_node {
    enum carbon_dot_node_type type;
    union {
        char *string;
        jak_u32 idx;
    } identifier;
};

struct jak_carbon_dot_path {
    struct jak_carbon_dot_node nodes[256];
    jak_u32 path_len;
    struct jak_error err;
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

JAK_DEFINE_ERROR_GETTER(jak_carbon_dot_path)

bool carbon_dot_path_create(struct jak_carbon_dot_path *path);

bool carbon_dot_path_from_string(struct jak_carbon_dot_path *path, const char *path_string);

bool carbon_dot_path_add_key(struct jak_carbon_dot_path *dst, const char *key);

bool carbon_dot_path_add_nkey(struct jak_carbon_dot_path *dst, const char *key, size_t len);

bool carbon_dot_path_add_idx(struct jak_carbon_dot_path *dst, jak_u32 idx);

bool carbon_dot_path_len(jak_u32 *len, const struct jak_carbon_dot_path *path);

bool carbon_dot_path_is_empty(const struct jak_carbon_dot_path *path);

bool carbon_dot_path_type_at(enum carbon_dot_node_type *type_out, jak_u32 pos, const struct jak_carbon_dot_path *path);

bool carbon_dot_path_idx_at(jak_u32 *idx, jak_u32 pos, const struct jak_carbon_dot_path *path);

const char *carbon_dot_path_key_at(jak_u32 pos, const struct jak_carbon_dot_path *path);

bool carbon_dot_path_drop(struct jak_carbon_dot_path *path);

bool carbon_dot_path_to_str(struct jak_string *sb, struct jak_carbon_dot_path *path);

bool carbon_dot_path_fprint(FILE *file, struct jak_carbon_dot_path *path);

bool carbon_dot_path_print(struct jak_carbon_dot_path *path);

JAK_END_DECL

#endif
