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
#include <jakson/utils/convert.h>
#include <jakson/carbon/dot.h>
#include <jakson/std/string.h>
#include <jakson/utils/strings.h>

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
        u32 len;
};

static const char *next_token(struct dot_token *token, const char *str)
{
        JAK_ASSERT(token);
        JAK_ASSERT(str);

        str = strings_skip_blanks(str);
        char c = *str;
        if (c) {
                if (isalpha(c)) {
                        token->type = TOKEN_STRING;
                        token->str = str;
                        bool skip_esc = false;
                        u32 strlen = 0;
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
                        u32 strlen = 1;
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
                        u32 strlen = 0;
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

bool carbon_dot_path_create(carbon_dot_path *path)
{
        ERROR_IF_NULL(path)
        error_init(&path->err);
        path->path_len = 0;
        ZERO_MEMORY(&path->nodes, ARRAY_LENGTH(path->nodes) * sizeof(carbon_dot_node));
        return true;
}

bool carbon_dot_path_from_string(carbon_dot_path *path, const char *path_string)
{
        ERROR_IF_NULL(path)
        UNUSED(path_string);

        struct dot_token token;
        int status = ERR_NOERR;
        carbon_dot_path_create(path);

        enum path_entry {
                DOT, ENTRY
        } expected_entry = ENTRY;
        path_string = next_token(&token, path_string);
        while (token.type != TOKEN_EOF) {
                expected_entry = token.type == TOKEN_DOT ? DOT : ENTRY;
                switch (token.type) {
                        case TOKEN_DOT:
                                if (expected_entry != DOT) {
                                        status = ERR_PARSE_DOT_EXPECTED;
                                        goto cleanup_and_error;
                                }
                                break;
                        case TOKEN_STRING:
                                if (expected_entry != ENTRY) {
                                        status = ERR_PARSE_ENTRY_EXPECTED;
                                        goto cleanup_and_error;
                                } else {
                                        carbon_dot_path_add_nkey(path, token.str, token.len);
                                }
                                break;
                        case TOKEN_NUMBER:
                                if (expected_entry != ENTRY) {
                                        status = ERR_PARSE_ENTRY_EXPECTED;
                                        goto cleanup_and_error;
                                } else {
                                        u64 num = convert_atoiu64(token.str);
                                        carbon_dot_path_add_idx(path, num);
                                }
                                break;
                        case TOKEN_UNKNOWN:
                                status = ERR_PARSE_UNKNOWN_TOKEN;
                                goto cleanup_and_error;
                        default: ERROR(&path->err, ERR_INTERNALERR);
                                break;
                }
                path_string = next_token(&token, path_string);
        }

        return true;

        cleanup_and_error:
        carbon_dot_path_drop(path);
        ERROR_NO_ABORT(&path->err, status);
        return false;
}

bool carbon_dot_path_add_key(carbon_dot_path *dst, const char *key)
{
        return carbon_dot_path_add_nkey(dst, key, strlen(key));
}

bool carbon_dot_path_add_nkey(carbon_dot_path *dst, const char *key, size_t len)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(key)
        if (LIKELY(dst->path_len < ARRAY_LENGTH(dst->nodes))) {
                carbon_dot_node *node = dst->nodes + dst->path_len++;
                bool enquoted = strings_is_enquoted_wlen(key, len);
                node->type = DOT_NODE_KEY_NAME;
                node->identifier.string = strndup(enquoted ? key + 1 : key, len);
                if (enquoted) {
                        char *str_wo_rightspaces = strings_remove_tailing_blanks(node->identifier.string);
                        size_t l = strlen(str_wo_rightspaces);
                        node->identifier.string[l - 1] = '\0';
                }
                JAK_ASSERT(!strings_is_enquoted(node->identifier.string));
                return true;
        } else {
                ERROR(&dst->err, ERR_OUTOFBOUNDS)
                return false;
        }
}

bool carbon_dot_path_add_idx(carbon_dot_path *dst, u32 idx)
{
        ERROR_IF_NULL(dst)
        if (LIKELY(dst->path_len < ARRAY_LENGTH(dst->nodes))) {
                carbon_dot_node *node = dst->nodes + dst->path_len++;
                node->type = DOT_NODE_ARRAY_IDX;
                node->identifier.idx = idx;
                return true;
        } else {
                ERROR(&dst->err, ERR_OUTOFBOUNDS)
                return false;
        }
}

bool carbon_dot_path_len(u32 *len, const carbon_dot_path *path)
{
        ERROR_IF_NULL(len)
        ERROR_IF_NULL(path)
        *len = path->path_len;
        return true;
}

bool carbon_dot_path_is_empty(const carbon_dot_path *path)
{
        ERROR_IF_NULL(path)
        return (path->path_len == 0);
}

bool carbon_dot_path_type_at(carbon_dot_node_type_e *type_out, u32 pos, const carbon_dot_path *path)
{
        ERROR_IF_NULL(type_out)
        ERROR_IF_NULL(path)
        if (LIKELY(pos < ARRAY_LENGTH(path->nodes))) {
                *type_out = path->nodes[pos].type;
        } else {
                ERROR(&((carbon_dot_path *) path)->err, ERR_OUTOFBOUNDS)
                return false;
        }
        return true;
}

bool carbon_dot_path_idx_at(u32 *idx, u32 pos, const carbon_dot_path *path)
{
        ERROR_IF_NULL(idx)
        ERROR_IF_NULL(path)
        ERROR_IF_AND_RETURN(pos >= ARRAY_LENGTH(path->nodes), &((carbon_dot_path *) path)->err,
                            ERR_OUTOFBOUNDS, NULL);
        ERROR_IF_AND_RETURN(path->nodes[pos].type != DOT_NODE_ARRAY_IDX, &((carbon_dot_path *) path)->err,
                            ERR_TYPEMISMATCH, NULL);

        *idx = path->nodes[pos].identifier.idx;
        return true;
}

const char *carbon_dot_path_key_at(u32 pos, const carbon_dot_path *path)
{
        ERROR_IF_NULL(path)
        ERROR_IF_AND_RETURN(pos >= ARRAY_LENGTH(path->nodes), &((carbon_dot_path *) path)->err,
                            ERR_OUTOFBOUNDS, NULL);
        ERROR_IF_AND_RETURN(path->nodes[pos].type != DOT_NODE_KEY_NAME, &((carbon_dot_path *) path)->err,
                            ERR_TYPEMISMATCH, NULL);

        return path->nodes[pos].identifier.string;
}

bool carbon_dot_path_drop(carbon_dot_path *path)
{
        ERROR_IF_NULL(path)
        for (u32 i = 0; i < path->path_len; i++) {
                carbon_dot_node *node = path->nodes + i;
                if (node->type == DOT_NODE_KEY_NAME) {
                        free(node->identifier.string);
                }
        }
        path->path_len = 0;
        return true;
}

bool carbon_dot_path_to_str(string_buffer *sb, carbon_dot_path *path)
{
        ERROR_IF_NULL(path)
        for (u32 i = 0; i < path->path_len; i++) {
                carbon_dot_node *node = path->nodes + i;
                switch (node->type) {
                        case DOT_NODE_KEY_NAME: {
                                bool empty_str = strlen(node->identifier.string) == 0;
                                bool quotes_required =
                                        empty_str || strings_contains_blank_char(node->identifier.string);
                                if (quotes_required) {
                                        string_buffer_add_char(sb, '"');
                                }
                                if (!empty_str) {
                                        string_buffer_add(sb, node->identifier.string);
                                }
                                if (quotes_required) {
                                        string_buffer_add_char(sb, '"');
                                }
                        }
                                break;
                        case DOT_NODE_ARRAY_IDX:
                                string_buffer_add_u32(sb, node->identifier.idx);
                                break;
                }
                if (i + 1 < path->path_len) {
                        string_buffer_add_char(sb, '.');
                }
        }
        return true;
}

bool carbon_dot_path_fprint(FILE *file, carbon_dot_path *path)
{
        ERROR_IF_NULL(file);
        ERROR_IF_NULL(path);
        string_buffer sb;
        string_buffer_create(&sb);
        carbon_dot_path_to_str(&sb, path);
        fprintf(file, "%s", string_cstr(&sb));
        string_buffer_drop(&sb);
        return true;
}

bool carbon_dot_path_print(carbon_dot_path *path)
{
        return carbon_dot_path_fprint(stdout, path);
}