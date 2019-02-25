/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef CARBON_JSON_H
#define CARBON_JSON_H

#include "carbon-common.h"
#include "carbon-vector.h"

CARBON_BEGIN_DECL

/** forwarded */
typedef struct carbon_json carbon_json_t;
typedef struct carbon_doc_bulk carbon_doc_bulk_t;
typedef struct carbon_json_ast_node_element carbon_json_ast_node_element_t;
typedef struct carbon_json_ast_node_object carbon_json_ast_node_object_t;
typedef struct carbon_json_ast_node_array carbon_json_ast_node_array_t;
typedef struct carbon_json_ast_node_string carbon_json_ast_node_string_t;
typedef struct carbon_json_ast_node_number carbon_json_ast_node_number_t;
typedef struct carbon_json_ast_node_members carbon_json_ast_node_members_t;
typedef struct carbon_json_ast_node_member carbon_json_ast_node_member_t;
typedef struct carbon_json_ast_node_elements carbon_json_ast_node_elements_t;

typedef enum
{
    CARBON_JSON_TOKEN_SCOPE_OPEN,
    CARBON_JSON_TOKEN_SCOPE_CLOSE,
    CARBON_JSON_TOKEN_STRING_LITERAL,
    CARBON_JSON_TOKEN_INT_NUMBER,
    CARBON_JSON_TOKEN_REAL_NUMBER,
    CARBON_JSON_TOKEN_LITERAL_TRUE,
    CARBON_JSON_TOKEN_LITERAL_FALSE,
    CARBON_JSON_TOKEN_LITERAL_NULL,
    CARBON_JSON_TOKEN_COMMA,
    CARBON_JSON_TOKEN_ASSIGNMENT,
    CARBON_JSON_TOKEN_ARRAY_BEGIN,
    CARBON_JSON_TOKEN_ARRAY_END,
    CARBON_JSON_TOKEN_UNKNOWN
} carbon_json_token_type_e;

typedef struct {
    carbon_json_token_type_e type;
    const char *string;
    unsigned line;
    unsigned column;
    unsigned length;
} carbon_json_token_t;

typedef struct carbon_json_parse_err
{
    const carbon_json_token_t *token;
    const char *token_type_str;
    const char *msg;
} carbon_json_parse_err_t;

typedef struct carbon_json_tokenizer
{
    const char *cursor;
    carbon_json_token_t token;
    carbon_err_t err;
} carbon_json_tokenizer_t;

typedef struct
{
    carbon_json_tokenizer_t tokenizer;
    carbon_doc_bulk_t *partition;
    carbon_err_t err;
} carbon_json_parser_t;

CARBON_EXPORT(bool)
carbon_json_tokenizer_init(carbon_json_tokenizer_t *tokenizer, const char *input);

CARBON_EXPORT(const carbon_json_token_t *)
carbon_json_tokenizer_next(carbon_json_tokenizer_t *tokenizer);

CARBON_EXPORT(void)
carbon_json_token_dup(carbon_json_token_t *dst, const carbon_json_token_t *src);

CARBON_EXPORT(void)
carbon_json_token_print(FILE *file, const carbon_json_token_t *token);

CARBON_EXPORT(bool)
carbon_json_parser_create(carbon_json_parser_t *parser, carbon_doc_bulk_t *partition);

CARBON_EXPORT(bool)
carbon_json_parse(CARBON_NULLABLE carbon_json_t *json, CARBON_NULLABLE carbon_json_parse_err_t *error_desc,
                  carbon_json_parser_t *parser, const char *input);

CARBON_EXPORT(bool)
carbon_json_test_doc(carbon_err_t *err, carbon_json_t *json);

typedef enum {
    CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_JSON,
    CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_MEMBER,
    CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_ELEMENTS
} carbon_json_ast_node_elem_parent_type_e;

typedef enum {
    CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT,
    CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY,
    CARBON_JSON_AST_NODE_VALUE_TYPE_STRING,
    CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER,
    CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE,
    CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE,
    CARBON_JSON_AST_NODE_VALUE_TYPE_NULL
} carbon_json_ast_node_value_type_e;

typedef struct carbon_json
{
    carbon_json_ast_node_element_t *element;
    carbon_err_t err;
} carbon_json_t;

typedef struct carbon_json_ast_node_value
{
    carbon_json_ast_node_element_t *parent;

    carbon_json_ast_node_value_type_e value_type;

    union {
        carbon_json_ast_node_object_t *object;
        carbon_json_ast_node_array_t *array;
        carbon_json_ast_node_string_t *string;
        carbon_json_ast_node_number_t *number;
        void *ptr;
    } value;
} carbon_json_ast_node_value_t;

typedef struct carbon_json_ast_node_object
{
    carbon_json_ast_node_value_t *parent;
    carbon_json_ast_node_members_t *value;
} carbon_json_ast_node_object_t;

typedef struct carbon_json_ast_node_element
{
    carbon_json_ast_node_elem_parent_type_e parent_type;

    union {
        carbon_json_t *json;
        carbon_json_ast_node_member_t *member;
        carbon_json_ast_node_elements_t *elements;
        void *ptr;
    } parent;

    carbon_json_ast_node_value_t value;

} carbon_json_ast_node_element_t;

typedef struct carbon_json_ast_node_string
{
    carbon_json_ast_node_member_t *parent;
    char *value;
} carbon_json_ast_node_string_t;

typedef struct carbon_json_ast_node_member
{
    carbon_json_ast_node_members_t *parent;
    carbon_json_ast_node_string_t key;
    carbon_json_ast_node_element_t value;
} carbon_json_ast_node_member_t;

typedef struct carbon_json_ast_node_members
{
    carbon_json_ast_node_object_t *parent;
    carbon_vec_t ofType(carbon_json_ast_node_member_t) members;
} carbon_json_ast_node_members_t;

typedef struct carbon_json_ast_node_elements
{
    carbon_json_ast_node_array_t *parent;
    carbon_vec_t ofType(carbon_json_ast_node_element_t) elements;
} carbon_json_ast_node_elements_t;

typedef struct carbon_json_ast_node_array
{
    carbon_json_ast_node_value_t *parent;
    carbon_json_ast_node_elements_t elements;
} carbon_json_ast_node_array_t;

typedef enum
{
    CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER,
    CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER,
    CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER
} carbon_json_ast_node_number_value_type_e;

typedef struct carbon_json_ast_node_number
{
    carbon_json_ast_node_value_t *parent;
    carbon_json_ast_node_number_value_type_e value_type;

    union {
        float float_number;
        int64_t signed_integer;
        uint64_t unsigned_integer;
    } value;
} carbon_json_ast_node_number_t;

CARBON_EXPORT(bool)
carbon_json_drop(carbon_json_t *json);

CARBON_EXPORT(bool)
carbon_json_print(FILE *file, carbon_json_t *json);

CARBON_DEFINE_GET_ERROR_FUNCTION(json, carbon_json_t, json);

CARBON_END_DECL

#endif