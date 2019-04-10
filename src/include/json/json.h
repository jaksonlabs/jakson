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

#ifndef NG5_JSON_H
#define NG5_JSON_H

#include "shared/common.h"
#include "std/vec.h"

NG5_BEGIN_DECL

/** forwarded */
typedef struct carbon_json carbon_json_t;

typedef struct carbon_json_ast_node_element carbon_json_ast_node_element_t;

typedef struct carbon_json_ast_node_object carbon_json_ast_node_object_t;

typedef struct carbon_json_ast_node_array carbon_json_ast_node_array_t;

typedef struct carbon_json_ast_node_string carbon_json_ast_node_string_t;

typedef struct carbon_json_ast_node_number carbon_json_ast_node_number_t;

typedef struct carbon_json_ast_node_members carbon_json_ast_node_members_t;

typedef struct carbon_json_ast_node_member carbon_json_ast_node_member_t;

typedef struct carbon_json_ast_node_elements carbon_json_ast_node_elements_t;

enum json_token_type {
        OBJECT_OPEN,
        OBJECT_CLOSE,
        LITERAL_STRING,
        LITERAL_INT,
        LITERAL_FLOAT,
        LITERAL_TRUE,
        LITERAL_FALSE,
        LITERAL_NULL,
        COMMA,
        ASSIGN,
        ARRAY_OPEN,
        ARRAY_CLOSE,
        JSON_UNKNOWN
};

struct json_token {
        enum json_token_type type;
        const char *string;
        unsigned line;
        unsigned column;
        unsigned length;
};

struct json_err {
        const struct json_token *token;
        const char *token_type_str;
        const char *msg;
};

struct json_tokenizer {
        const char *cursor;
        struct json_token token;
        struct err err;
};

struct json_parser {
        struct json_tokenizer tokenizer;
        struct doc_bulk *partition;
        struct err err;
};

NG5_EXPORT(bool) carbon_json_tokenizer_init(struct json_tokenizer *tokenizer, const char *input);

NG5_EXPORT(const struct json_token *)carbon_json_tokenizer_next(struct json_tokenizer *tokenizer);

NG5_EXPORT(void) carbon_json_token_dup(struct json_token *dst, const struct json_token *src);

NG5_EXPORT(void) carbon_json_token_print(FILE *file, const struct json_token *token);

NG5_EXPORT(bool) carbon_json_parser_create(struct json_parser *parser, struct doc_bulk *partition);

NG5_EXPORT(bool) carbon_json_parse(NG5_NULLABLE carbon_json_t *json, NG5_NULLABLE struct json_err *error_desc,
        struct json_parser *parser, const char *input);

NG5_EXPORT(bool) carbon_json_test_doc(struct err *err, carbon_json_t *json);

enum json_parent {
        JSON_PARENT_OBJECT,
        JSON_PARENT_MEMBER,
        JSON_PARENT_ELEMENTS
};

enum json_value_type {
        JSON_VALUE_OBJECT,
        JSON_VALUE_ARRAY,
        JSON_VALUE_STRING,
        JSON_VALUE_NUMBER,
        JSON_VALUE_TRUE,
        JSON_VALUE_FALSE,
        JSON_VALUE_NULL
};

typedef struct carbon_json {
        carbon_json_ast_node_element_t *element;
        struct err err;
} carbon_json_t;

typedef struct carbon_json_ast_node_value {
        carbon_json_ast_node_element_t *parent;

        enum json_value_type value_type;

        union {
                carbon_json_ast_node_object_t *object;
                carbon_json_ast_node_array_t *array;
                carbon_json_ast_node_string_t *string;
                carbon_json_ast_node_number_t *number;
                void *ptr;
        } value;
} carbon_json_ast_node_value_t;

typedef struct carbon_json_ast_node_object {
        carbon_json_ast_node_value_t *parent;
        carbon_json_ast_node_members_t *value;
} carbon_json_ast_node_object_t;

typedef struct carbon_json_ast_node_element {
        enum json_parent parent_type;

        union {
                carbon_json_t *json;
                carbon_json_ast_node_member_t *member;
                carbon_json_ast_node_elements_t *elements;
                void *ptr;
        } parent;

        carbon_json_ast_node_value_t value;

} carbon_json_ast_node_element_t;

typedef struct carbon_json_ast_node_string {
        carbon_json_ast_node_member_t *parent;
        char *value;
} carbon_json_ast_node_string_t;

typedef struct carbon_json_ast_node_member {
        carbon_json_ast_node_members_t *parent;
        carbon_json_ast_node_string_t key;
        carbon_json_ast_node_element_t value;
} carbon_json_ast_node_member_t;

typedef struct carbon_json_ast_node_members {
        carbon_json_ast_node_object_t *parent;
        struct vector ofType(carbon_json_ast_node_member_t) members;
} carbon_json_ast_node_members_t;

typedef struct carbon_json_ast_node_elements {
        carbon_json_ast_node_array_t *parent;
        struct vector ofType(carbon_json_ast_node_element_t) elements;
} carbon_json_ast_node_elements_t;

typedef struct carbon_json_ast_node_array {
        carbon_json_ast_node_value_t *parent;
        carbon_json_ast_node_elements_t elements;
} carbon_json_ast_node_array_t;

typedef enum {
        NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER,
        NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER,
        NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER
} carbon_json_ast_node_number_value_type_e;

typedef struct carbon_json_ast_node_number {
        carbon_json_ast_node_value_t *parent;
        carbon_json_ast_node_number_value_type_e value_type;

        union {
                float float_number;
                i64 signed_integer;
                u64 unsigned_integer;
        } value;
} carbon_json_ast_node_number_t;

NG5_EXPORT(bool) carbon_json_drop(carbon_json_t *json);

NG5_EXPORT(bool) carbon_json_print(FILE *file, carbon_json_t *json);

NG5_DEFINE_GET_ERROR_FUNCTION(json, carbon_json_t, json);

NG5_END_DECL

#endif