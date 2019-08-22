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

#ifndef JAK_JSON_H
#define JAK_JSON_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

JAK_BEGIN_DECL

/** forwarded */

struct jak_json;

struct jak_json_element;

struct jak_json_object_t;

struct jak_json_array;

struct jak_json_string;

struct jak_json_number;

struct jak_json_members;

struct jak_json_prop;

struct jak_json_elements;

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

struct jak_json_token {
        enum json_token_type type;
        const char *string;
        unsigned line;
        unsigned column;
        unsigned length;
};

struct jak_json_err {
        const struct jak_json_token *token;
        const char *token_type_str;
        const char *msg;
};

struct jak_json_tokenizer {
        const char *cursor;
        struct jak_json_token token;
        jak_error err;
};

struct jak_json_parser {
        struct jak_json_tokenizer tokenizer;
        jak_error err;
};

enum json_parent {
        JSON_PARENT_OBJECT, JSON_PARENT_MEMBER, JSON_PARENT_ELEMENTS
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

enum json_list_type {
        JSON_LIST_TYPE_EMPTY,
        JSON_LIST_TYPE_VARIABLE_OR_NESTED,
        JSON_LIST_TYPE_FIXED_U8,
        JSON_LIST_TYPE_FIXED_U16,
        JSON_LIST_TYPE_FIXED_U32,
        JSON_LIST_TYPE_FIXED_U64,
        JSON_LIST_TYPE_FIXED_I8,
        JSON_LIST_TYPE_FIXED_I16,
        JSON_LIST_TYPE_FIXED_I32,
        JSON_LIST_TYPE_FIXED_I64,
        JSON_LIST_TYPE_FIXED_FLOAT,
        JSON_LIST_TYPE_FIXED_NULL,
        JSON_LIST_TYPE_FIXED_BOOLEAN
};

struct jak_json {
        struct jak_json_element *element;
        jak_error err;
};

struct jak_json_node_value {
        struct jak_json_element *parent;

        enum json_value_type value_type;

        union {
                struct jak_json_object_t *object;
                struct jak_json_array *array;
                struct jak_json_string *string;
                struct jak_json_number *number;
                void *ptr;
        } value;
};

struct jak_json_object_t {
        struct jak_json_node_value *parent;
        struct jak_json_members *value;
};

struct jak_json_element {
        enum json_parent parent_type;

        union {
                struct jak_json *json;
                struct jak_json_prop *member;
                struct jak_json_elements *elements;
                void *ptr;
        } parent;

        struct jak_json_node_value value;

};

struct jak_json_string {
        struct jak_json_prop *parent;
        char *value;
};

struct jak_json_prop {
        struct jak_json_members *parent;
        struct jak_json_string key;
        struct jak_json_element value;
};

struct jak_json_members {
        struct jak_json_object_t *parent;
        struct jak_vector ofType(struct jak_json_prop) members;
};

struct jak_json_elements {
        struct jak_json_array *parent;
        struct jak_vector ofType(struct jak_json_element) elements;
};

struct jak_json_array {
        struct jak_json_node_value *parent;
        struct jak_json_elements elements;
};

enum json_number_type {
        JSON_NUMBER_FLOAT, JSON_NUMBER_UNSIGNED, JSON_NUMBER_SIGNED
};

struct jak_json_number {
        struct jak_json_node_value *parent;
        enum json_number_type value_type;

        union {
                float float_number;
                jak_i64 signed_integer;
                jak_u64 unsigned_integer;
        } value;
};

bool json_tokenizer_init(struct jak_json_tokenizer *tokenizer, const char *input);

const struct jak_json_token *json_tokenizer_next(struct jak_json_tokenizer *tokenizer);

void json_token_dup(struct jak_json_token *dst, const struct jak_json_token *src);

void json_token_print(FILE *file, const struct jak_json_token *token);

bool json_parser_create(struct jak_json_parser *parser);

bool json_parse(struct jak_json *json, struct jak_json_err *error_desc, struct jak_json_parser *parser,
                const char *input);

bool json_test(jak_error *err, struct jak_json *json);

bool json_drop(struct jak_json *json);

bool json_print(FILE *file, struct jak_json *json);

bool json_list_is_empty(const struct jak_json_elements *elements);

bool json_list_length(jak_u32 *len, const struct jak_json_elements *elements);

enum json_list_type json_fitting_type(enum json_list_type current, enum json_list_type to_add);

bool json_array_get_type(enum json_list_type *type, const struct jak_json_array *array);

JAK_DEFINE_GET_ERROR_FUNCTION(json, struct jak_json, json);

JAK_END_DECL

#endif