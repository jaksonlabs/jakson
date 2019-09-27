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

#include <jakson/jak_stdinc.h>
#include <jakson/jak_error.h>
#include <jakson/jak_types.h>
#include <jakson/std/jak_string.h>
#include <jakson/carbon/jak_carbon_array_it.h>
#include <jakson/carbon/jak_carbon_column_it.h>
#include <jakson/carbon/jak_carbon_object_it.h>

JAK_BEGIN_DECL

typedef enum carbon_dot_node {
        JAK_DOT_NODE_ARRAY_IDX,
        JAK_DOT_NODE_KEY_NAME
} carbon_dot_node_e;

typedef struct jak_carbon_dot_node {
        carbon_dot_node_e type;
        union {
                char *string;
                jak_u32 idx;
        } identifier;
} jak_carbon_dot_node;

typedef struct jak_carbon_dot_path {
        jak_carbon_dot_node nodes[256];
        jak_u32 path_len;
        jak_error err;
} jak_carbon_dot_path;

typedef enum jak_carbon_path_status {
        JAK_CARBON_PATH_RESOLVED,
        JAK_CARBON_PATH_EMPTY_DOC,
        JAK_CARBON_PATH_NOSUCHINDEX,
        JAK_CARBON_PATH_NOSUCHKEY,
        JAK_CARBON_PATH_NOTTRAVERSABLE,
        JAK_CARBON_PATH_NOCONTAINER,
        JAK_CARBON_PATH_NOTANOBJECT,
        JAK_CARBON_PATH_NONESTING,
        JAK_CARBON_PATH_INTERNAL
} jak_carbon_path_status_e;

JAK_DEFINE_ERROR_GETTER(jak_carbon_dot_path)

bool jak_carbon_dot_path_create(jak_carbon_dot_path *path);
bool jak_carbon_dot_path_from_string(jak_carbon_dot_path *path, const char *path_string);
bool jak_carbon_dot_path_drop(jak_carbon_dot_path *path);

bool jak_carbon_dot_path_add_key(jak_carbon_dot_path *dst, const char *key);
bool jak_carbon_dot_path_add_nkey(jak_carbon_dot_path *dst, const char *key, size_t len);
bool jak_carbon_dot_path_add_idx(jak_carbon_dot_path *dst, jak_u32 idx);
bool jak_carbon_dot_path_len(jak_u32 *len, const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_is_empty(const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_type_at(carbon_dot_node_e *type_out, jak_u32 pos, const jak_carbon_dot_path *path);
bool jak_carbon_dot_path_idx_at(jak_u32 *idx, jak_u32 pos, const jak_carbon_dot_path *path);
const char *jak_carbon_dot_path_key_at(jak_u32 pos, const jak_carbon_dot_path *path);

bool jak_carbon_dot_path_to_str(jak_string *sb, jak_carbon_dot_path *path);
bool jak_carbon_dot_path_fprint(FILE *file, jak_carbon_dot_path *path);
bool jak_carbon_dot_path_print(jak_carbon_dot_path *path);

JAK_END_DECL

#endif
