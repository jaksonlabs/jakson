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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <ctype.h>
#include <jak_utils_convert.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_dot.h>
#include <jak_string.h>
#include <jak_string_utils.h>

enum dot_token_type {
        TOKEN_DOT,
        TOKEN_STRING,
        TOKEN_NUMBER,
        TOKEN_UNKNOWN,
        TOKEN_EOF
};

struct dot_token {
        enum dot_token_type type;
        const char *str;
        jak_u32 len;
};

static const char *next_token(struct dot_token *token, const char *str)
{
        JAK_ASSERT(token);
        JAK_ASSERT(str);

        str = jak_strings_skip_blanks(str);
        char c = *str;
        if (c) {
                if (isalpha(c)) {
                        token->type = TOKEN_STRING;
                        token->str = str;
                        bool skip_esc = false;
                        jak_u32 strlen = 0;
                        while (c && (isalpha(c) && (c != '\n') && (c != '\t') && (c != '\r') && (c != ' '))) {
                                if (!skip_esc) {
                                        if (c == '\\') {
                                                skip_esc = true;
                                        }
                                } else {
                                        skip_esc = false;
                                }
                                strlen++;
                                c = *(++str);
                        }
                        token->len = strlen;
                } else if (c == '\"') {
                        token->type = TOKEN_STRING;
                        token->str = str;
                        c = *(++str);
                        bool skip_esc = false;
                        bool end_found = false;
                        jak_u32 strlen = 1;
                        while (c && !end_found) {
                                if (!skip_esc) {
                                        if (c == '\\') {
                                                skip_esc = true;
                                        } else if (c == '\"') {
                                                end_found = true;
                                        }
                                } else {
                                        skip_esc = false;
                                }

                                strlen++;
                                c = *(++str);
                        }
                        token->len = strlen;
                } else if (c == '.') {
                        token->type = TOKEN_DOT;
                        token->str = str;
                        token->len = 1;
                        str++;
                } else if (isdigit(c)) {
                        token->type = TOKEN_NUMBER;
                        token->str = str;
                        jak_u32 strlen = 0;
                        while (c && isdigit(c)) {
                                c = *(++str);
                                strlen++;
                        }
                        token->len = strlen;
                } else {
                        token->type = TOKEN_UNKNOWN;
                        token->str = str;
                        token->len = strlen(str);
                }
        } else {
                token->type = TOKEN_EOF;
        }
        return str;
}

bool jak_carbon_dot_path_create(jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(path)
        jak_error_init(&path->err);
        path->path_len = 0;
        JAK_ZERO_MEMORY(&path->nodes, JAK_ARRAY_LENGTH(path->nodes) * sizeof(jak_carbon_dot_node));
        return true;
}

bool jak_carbon_dot_path_from_string(jak_carbon_dot_path *path, const char *path_string)
{
        JAK_ERROR_IF_NULL(path)
        JAK_UNUSED(path_string);

        struct dot_token token;
        int status = JAK_ERR_NOERR;
        jak_carbon_dot_path_create(path);

        enum path_entry {
                DOT, ENTRY
        } expected_entry = ENTRY;
        path_string = next_token(&token, path_string);
        while (token.type != TOKEN_EOF) {
                expected_entry = token.type == TOKEN_DOT ? DOT : ENTRY;
                switch (token.type) {
                        case TOKEN_DOT:
                                if (expected_entry != DOT) {
                                        status = JAK_ERR_PARSE_DOT_EXPECTED;
                                        goto cleanup_and_error;
                                }
                                break;
                        case TOKEN_STRING:
                                if (expected_entry != ENTRY) {
                                        status = JAK_ERR_PARSE_ENTRY_EXPECTED;
                                        goto cleanup_and_error;
                                } else {
                                        jak_carbon_dot_path_add_nkey(path, token.str, token.len);
                                }
                                break;
                        case TOKEN_NUMBER:
                                if (expected_entry != ENTRY) {
                                        status = JAK_ERR_PARSE_ENTRY_EXPECTED;
                                        goto cleanup_and_error;
                                } else {
                                        jak_u64 num = jak_convert_atoiu64(token.str);
                                        jak_carbon_dot_path_add_idx(path, num);
                                }
                                break;
                        case TOKEN_UNKNOWN:
                                status = JAK_ERR_PARSE_UNKNOWN_TOKEN;
                                goto cleanup_and_error;
                        default: JAK_ERROR(&path->err, JAK_ERR_INTERNALERR);
                                break;
                }
                path_string = next_token(&token, path_string);
        }

        return true;

        cleanup_and_error:
        jak_carbon_dot_path_drop(path);
        JAK_ERROR_NO_ABORT(&path->err, status);
        return false;
}

bool jak_carbon_dot_path_add_key(jak_carbon_dot_path *dst, const char *key)
{
        return jak_carbon_dot_path_add_nkey(dst, key, strlen(key));
}

bool jak_carbon_dot_path_add_nkey(jak_carbon_dot_path *dst, const char *key, size_t len)
{
        JAK_ERROR_IF_NULL(dst)
        JAK_ERROR_IF_NULL(key)
        if (JAK_LIKELY(dst->path_len < JAK_ARRAY_LENGTH(dst->nodes))) {
                jak_carbon_dot_node *node = dst->nodes + dst->path_len++;
                bool enquoted = jak_strings_is_enquoted_wlen(key, len);
                node->type = JAK_DOT_NODE_KEY_NAME;
                node->identifier.string = strndup(enquoted ? key + 1 : key, len);
                if (enquoted) {
                        char *str_wo_rightspaces = jak_strings_remove_tailing_blanks(node->identifier.string);
                        size_t l = strlen(str_wo_rightspaces);
                        node->identifier.string[l - 1] = '\0';
                }
                JAK_ASSERT(!jak_strings_is_enquoted(node->identifier.string));
                return true;
        } else {
                JAK_ERROR(&dst->err, JAK_ERR_OUTOFBOUNDS)
                return false;
        }
}

bool jak_carbon_dot_path_add_idx(jak_carbon_dot_path *dst, jak_u32 idx)
{
        JAK_ERROR_IF_NULL(dst)
        if (JAK_LIKELY(dst->path_len < JAK_ARRAY_LENGTH(dst->nodes))) {
                jak_carbon_dot_node *node = dst->nodes + dst->path_len++;
                node->type = JAK_DOT_NODE_ARRAY_IDX;
                node->identifier.idx = idx;
                return true;
        } else {
                JAK_ERROR(&dst->err, JAK_ERR_OUTOFBOUNDS)
                return false;
        }
}

bool jak_carbon_dot_path_len(jak_u32 *len, const jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(len)
        JAK_ERROR_IF_NULL(path)
        *len = path->path_len;
        return true;
}

bool jak_carbon_dot_path_is_empty(const jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(path)
        return (path->path_len == 0);
}

bool jak_carbon_dot_path_type_at(carbon_dot_node_e *type_out, jak_u32 pos, const jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(type_out)
        JAK_ERROR_IF_NULL(path)
        if (JAK_LIKELY(pos < JAK_ARRAY_LENGTH(path->nodes))) {
                *type_out = path->nodes[pos].type;
        } else {
                JAK_ERROR(&((jak_carbon_dot_path *) path)->err, JAK_ERR_OUTOFBOUNDS)
                return false;
        }
        return true;
}

bool jak_carbon_dot_path_idx_at(jak_u32 *idx, jak_u32 pos, const jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(idx)
        JAK_ERROR_IF_NULL(path)
        JAK_ERROR_IF_AND_RETURN(pos >= JAK_ARRAY_LENGTH(path->nodes), &((jak_carbon_dot_path *) path)->err,
                            JAK_ERR_OUTOFBOUNDS, NULL);
        JAK_ERROR_IF_AND_RETURN(path->nodes[pos].type != JAK_DOT_NODE_ARRAY_IDX, &((jak_carbon_dot_path *) path)->err,
                            JAK_ERR_TYPEMISMATCH, NULL);

        *idx = path->nodes[pos].identifier.idx;
        return true;
}

const char *jak_carbon_dot_path_key_at(jak_u32 pos, const jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(path)
        JAK_ERROR_IF_AND_RETURN(pos >= JAK_ARRAY_LENGTH(path->nodes), &((jak_carbon_dot_path *) path)->err,
                            JAK_ERR_OUTOFBOUNDS, NULL);
        JAK_ERROR_IF_AND_RETURN(path->nodes[pos].type != JAK_DOT_NODE_KEY_NAME, &((jak_carbon_dot_path *) path)->err,
                            JAK_ERR_TYPEMISMATCH, NULL);

        return path->nodes[pos].identifier.string;
}

bool jak_carbon_dot_path_drop(jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(path)
        for (jak_u32 i = 0; i < path->path_len; i++) {
                jak_carbon_dot_node *node = path->nodes + i;
                if (node->type == JAK_DOT_NODE_KEY_NAME) {
                        free(node->identifier.string);
                }
        }
        path->path_len = 0;
        return true;
}

bool jak_carbon_dot_path_to_str(jak_string *sb, jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(path)
        for (jak_u32 i = 0; i < path->path_len; i++) {
                jak_carbon_dot_node *node = path->nodes + i;
                switch (node->type) {
                        case JAK_DOT_NODE_KEY_NAME: {
                                bool empty_str = strlen(node->identifier.string) == 0;
                                bool quotes_required =
                                        empty_str || jak_strings_contains_blank_char(node->identifier.string);
                                if (quotes_required) {
                                        jak_string_add_char(sb, '"');
                                }
                                if (!empty_str) {
                                        jak_string_add(sb, node->identifier.string);
                                }
                                if (quotes_required) {
                                        jak_string_add_char(sb, '"');
                                }
                        }
                                break;
                        case JAK_DOT_NODE_ARRAY_IDX:
                                jak_string_add_u32(sb, node->identifier.idx);
                                break;
                }
                if (i + 1 < path->path_len) {
                        jak_string_add_char(sb, '.');
                }
        }
        return true;
}

bool jak_carbon_dot_path_fprint(FILE *file, jak_carbon_dot_path *path)
{
        JAK_ERROR_IF_NULL(file);
        JAK_ERROR_IF_NULL(path);
        jak_string sb;
        jak_string_create(&sb);
        jak_carbon_dot_path_to_str(&sb, path);
        fprintf(file, "%s", jak_string_cstr(&sb));
        jak_string_drop(&sb);
        return true;
}

bool jak_carbon_dot_path_print(jak_carbon_dot_path *path)
{
        return jak_carbon_dot_path_fprint(stdout, path);
}