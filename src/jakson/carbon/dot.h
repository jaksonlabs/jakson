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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/error.h>
#include <jakson/types.h>
#include <jakson/std/string.h>
#include <jakson/carbon/array_it.h>
#include <jakson/carbon/column_it.h>
#include <jakson/carbon/object_it.h>

BEGIN_DECL

typedef enum carbon_dot_node_type {
        DOT_NODE_ARRAY_IDX,
        DOT_NODE_KEY_NAME
} carbon_dot_node_type_e;

typedef struct carbon_dot_node {
    carbon_dot_node_type_e type;
        union {
                char *string;
                u32 idx;
        } identifier;
} carbon_dot_node;

typedef struct carbon_dot_path {
        carbon_dot_node nodes[256];
        u32 path_len;
        err err;
} carbon_dot_path;

typedef enum carbon_path_status {
        CARBON_PATH_RESOLVED,
        CARBON_PATH_EMPTY_DOC,
        CARBON_PATH_NOSUCHINDEX,
        CARBON_PATH_NOSUCHKEY,
        CARBON_PATH_NOTTRAVERSABLE,
        CARBON_PATH_NOCONTAINER,
        CARBON_PATH_NOTANOBJECT,
        CARBON_PATH_NONESTING,
        CARBON_PATH_INTERNAL
} carbon_path_status_e;

bool carbon_dot_path_create(carbon_dot_path *path);
bool carbon_dot_path_from_string(carbon_dot_path *path, const char *path_string);
bool carbon_dot_path_drop(carbon_dot_path *path);

bool carbon_dot_path_add_key(carbon_dot_path *dst, const char *key);
bool carbon_dot_path_add_nkey(carbon_dot_path *dst, const char *key, size_t len);
bool carbon_dot_path_add_idx(carbon_dot_path *dst, u32 idx);
bool carbon_dot_path_len(u32 *len, const carbon_dot_path *path);
bool carbon_dot_path_is_empty(const carbon_dot_path *path);
bool carbon_dot_path_type_at(carbon_dot_node_type_e *type_out, u32 pos, const carbon_dot_path *path);
bool carbon_dot_path_idx_at(u32 *idx, u32 pos, const carbon_dot_path *path);
const char *carbon_dot_path_key_at(u32 pos, const carbon_dot_path *path);

bool carbon_dot_path_to_str(string_buffer *sb, carbon_dot_path *path);
bool carbon_dot_path_fprint(FILE *file, carbon_dot_path *path);
bool carbon_dot_path_print(carbon_dot_path *path);

END_DECL

#endif
