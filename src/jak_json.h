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

typedef enum jak_json_token_type {
        JAK_OBJECT_OPEN,
        JAK_OBJECT_CLOSE,
        JAK_LITERAL_STRING,
        JAK_LITERAL_INT,
        JAK_LITERAL_FLOAT,
        JAK_LITERAL_TRUE,
        JAK_LITERAL_FALSE,
        JAK_LITERAL_NULL,
        JAK_COMMA,
        JAK_ASSIGN,
        JAK_ARRAY_OPEN,
        JAK_ARRAY_CLOSE,
        JAK_JSON_UNKNOWN
} jak_json_token_e;

typedef struct jak_json_token {
        jak_json_token_e type;
        const char *string;
        unsigned line;
        unsigned column;
        unsigned length;
} jak_json_token;

typedef struct jak_json_err {
        const jak_json_token *token;
        const char *token_type_str;
        const char *msg;
} jak_json_err;

typedef struct jak_json_tokenizer {
        const char *cursor;
        jak_json_token token;
        jak_error err;
} jak_json_tokenizer;

typedef struct jak_json_parser {
        jak_json_tokenizer tokenizer;
        jak_error err;
} jak_json_parser;

typedef enum json_parent {
        JAK_JSON_PARENT_OBJECT, JAK_JSON_PARENT_MEMBER, JAK_JSON_PARENT_ELEMENTS
} json_parent_e;

typedef enum jak_json_value_type_e {
        JAK_JSON_VALUE_OBJECT,
        JAK_JSON_VALUE_ARRAY,
        JAK_JSON_VALUE_STRING,
        JAK_JSON_VALUE_NUMBER,
        JAK_JSON_VALUE_TRUE,
        JAK_JSON_VALUE_FALSE,
        JAK_JSON_VALUE_NULL
} jak_json_value_type_e;

typedef enum jak_json_list_type_e {
        JAK_JSON_LIST_EMPTY,
        JAK_JSON_LIST_VARIABLE_OR_NESTED,
        JAK_JSON_LIST_FIXED_U8,
        JAK_JSON_LIST_FIXED_U16,
        JAK_JSON_LIST_FIXED_U32,
        JAK_JSON_LIST_FIXED_U64,
        JAK_JSON_LIST_FIXED_I8,
        JAK_JSON_LIST_FIXED_I16,
        JAK_JSON_LIST_FIXED_I32,
        JAK_JSON_LIST_FIXED_I64,
        JAK_JSON_LIST_FIXED_FLOAT,
        JAK_JSON_LIST_FIXED_NULL,
        JAK_JSON_LIST_FIXED_BOOLEAN
} jak_json_list_type_e;

typedef struct jak_json {
        jak_json_element *element;
        jak_error err;
} jak_json;

typedef struct jak_json_node_value {
        jak_json_element *parent;
        jak_json_value_type_e value_type;
        union {
                jak_json_object *object;
                jak_json_array *array;
                jak_json_string *string;
                jak_json_number *number;
                void *ptr;
        } value;
} jak_json_node_value;

typedef struct jak_json_object {
        jak_json_node_value *parent;
        jak_json_members *value;
} jak_json_object;

typedef struct jak_json_element {
        json_parent_e parent_type;
        union {
                jak_json *jak_json;
                jak_json_prop *member;
                jak_json_elements *elements;
                void *ptr;
        } parent;
        jak_json_node_value value;
} jak_json_element;

typedef struct jak_json_string {
        jak_json_prop *parent;
        char *value;
} jak_json_string;

typedef struct jak_json_prop {
        jak_json_members *parent;
        jak_json_string key;
        jak_json_element value;
} jak_json_prop;

typedef struct jak_json_members {
        jak_json_object *parent;
        jak_vector ofType(jak_json_prop) members;
} jak_json_members;

typedef struct jak_json_elements {
        jak_json_array *parent;
        jak_vector ofType(jak_json_element) elements;
} jak_json_elements;

typedef struct jak_json_array {
        jak_json_node_value *parent;
        jak_json_elements elements;
} jak_json_array;

typedef enum json_number_type {
        JAK_JSON_NUMBER_FLOAT, JAK_JSON_NUMBER_UNSIGNED, JAK_JSON_NUMBER_SIGNED
} json_number_type_e;

typedef struct jak_json_number {
        jak_json_node_value *parent;
        json_number_type_e value_type;
        union {
                float float_number;
                jak_i64 signed_integer;
                jak_u64 unsigned_integer;
        } value;
} jak_json_number;

bool jak_json_tokenizer_init(jak_json_tokenizer *tokenizer, const char *input);
const jak_json_token *json_tokenizer_next(jak_json_tokenizer *tokenizer);
void jak_json_token_dup(jak_json_token *dst, const jak_json_token *src);
void jak_json_token_print(FILE *file, const jak_json_token *token);
bool jak_json_parser_create(jak_json_parser *parser);
bool jak_json_parse(jak_json *json, jak_json_err *error_desc, jak_json_parser *parser, const char *input);
bool jak_json_test(jak_error *err, jak_json *jak_json);
bool jak_json_drop(jak_json *jak_json);
bool jak_json_print(FILE *file, jak_json *jak_json);
bool jak_json_list_is_empty(const jak_json_elements *elements);
bool jak_json_list_length(jak_u32 *len, const jak_json_elements *elements);
jak_json_list_type_e jak_json_fitting_type(jak_json_list_type_e current, jak_json_list_type_e to_add);
bool jak_json_array_get_type(jak_json_list_type_e *type, const jak_json_array *array);

JAK_DEFINE_GET_ERROR_FUNCTION(jak_json, jak_json, jak_json);

JAK_END_DECL

#endif