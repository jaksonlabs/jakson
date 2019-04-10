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

#include <inttypes.h>
#include <ctype.h>
#include <locale.h>
#include "json/json.h"
#include "json/doc.h"
#include "utils/convert.h"

static struct {
    carbon_json_token_type_e token;
    const char *string;
} TOKEN_STRING[] = {
    {.token = NG5_JSON_TOKEN_SCOPE_OPEN, .string = "NG5_JSON_TOKEN_SCOPE_OPEN"},
    {.token = NG5_JSON_TOKEN_SCOPE_CLOSE, .string = "NG5_JSON_TOKEN_SCOPE_CLOSE"},
    {.token = NG5_JSON_TOKEN_STRING_LITERAL, .string = "JSON_TOKEN_STRING"},
    {.token = NG5_JSON_TOKEN_INT_NUMBER, .string = "NG5_JSON_TOKEN_INT_NUMBER"},
    {.token = NG5_JSON_TOKEN_REAL_NUMBER, .string = "NG5_JSON_TOKEN_REAL_NUMBER"},
    {.token = NG5_JSON_TOKEN_LITERAL_TRUE, .string = "NG5_JSON_TOKEN_LITERAL_TRUE"},
    {.token = NG5_JSON_TOKEN_LITERAL_FALSE, .string = "NG5_JSON_TOKEN_LITERAL_FALSE"},
    {.token = NG5_JSON_TOKEN_LITERAL_NULL, .string = "NG5_JSON_TOKEN_LITERAL_NULL"},
    {.token = NG5_JSON_TOKEN_COMMA, .string = "NG5_JSON_TOKEN_COMMA"},
    {.token = NG5_JSON_TOKEN_ASSIGNMENT, .string = "JSON_TOKEN_ASSIGMENT"},
    {.token = NG5_JSON_TOKEN_ARRAY_BEGIN, .string = "NG5_JSON_TOKEN_ARRAY_BEGIN"},
    {.token = NG5_JSON_TOKEN_ARRAY_END, .string = "NG5_JSON_TOKEN_ARRAY_END"},
    {.token = NG5_JSON_TOKEN_UNKNOWN, .string = "NG5_JSON_TOKEN_UNKNOWN"}
};

typedef struct
{
    carbon_json_token_type_e type;
    bool init;
} token_memory_t;

static int process_token(struct err *err, carbon_json_parse_err *error_desc, const carbon_json_token_t *token, struct vector ofType(carbon_json_token_type_e) *brackets,
                        token_memory_t *token_mem);

static int set_error(carbon_json_parse_err *error_desc, const carbon_json_token_t *token, const char *msg);


NG5_EXPORT(bool)
carbon_json_tokenizer_init(carbon_json_tokenizer_t *tokenizer, const char *input)
{
    NG5_NON_NULL_OR_ERROR(tokenizer)
    NG5_NON_NULL_OR_ERROR(input)
    tokenizer->cursor = input;
    tokenizer->token = (carbon_json_token_t) {
        .type = NG5_JSON_TOKEN_UNKNOWN,
        .length = 0,
        .column = 0,
        .line = 1,
        .string = NULL
    };
    carbon_error_init(&tokenizer->err);
    return true;
}

const carbon_json_token_t *carbon_json_tokenizer_next(carbon_json_tokenizer_t *tokenizer)
{
    if (NG5_LIKELY(*tokenizer->cursor != '\0')) {
        char c = *tokenizer->cursor;
        tokenizer->token.string = tokenizer->cursor;
        tokenizer->token.column += tokenizer->token.length;
        tokenizer->token.length = 0;
        if (c == '\n' || c == '\r') {
            tokenizer->token.line += c == '\n' ? 1 : 0;
            tokenizer->token.column = c == '\n' ? 0 : tokenizer->token.column;
            tokenizer->cursor++;
            return carbon_json_tokenizer_next(tokenizer);
        } else if (isspace(c)) {
            do {
                tokenizer->cursor++;
                tokenizer->token.column++;
            } while (isspace(c = *tokenizer->cursor) && c != '\n');
            return carbon_json_tokenizer_next(tokenizer);
        } else if (c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
            tokenizer->token.type = c == '{' ? NG5_JSON_TOKEN_SCOPE_OPEN : c == '}' ? NG5_JSON_TOKEN_SCOPE_CLOSE :
                                    c == '[' ? NG5_JSON_TOKEN_ARRAY_BEGIN : c == ']' ? NG5_JSON_TOKEN_ARRAY_END :
                                    c == ':' ? NG5_JSON_TOKEN_ASSIGNMENT : NG5_JSON_TOKEN_COMMA;
            tokenizer->token.column++;
            tokenizer->token.length = 1;
            tokenizer->cursor++;
        } else if(c == '"') {
            bool escapeQuote = false;
            tokenizer->token.type = NG5_JSON_TOKEN_STRING_LITERAL;
            tokenizer->token.string++;
            tokenizer->token.column++;
            char last_1_c = '\0', last_2_c = '\0', last_3_c = '\0', last_4_c = '\0';
            c = *(++tokenizer->cursor);
            while ((escapeQuote || c != '"') && c != '\r' && c != '\n')
            {
next_char:
                tokenizer->token.length++;
                last_4_c = last_3_c;
                last_3_c = last_2_c;
                last_2_c = last_1_c;
                last_1_c = c;
                c = *(++tokenizer->cursor);
                if (NG5_UNLIKELY(c == '\\' && last_1_c == '\\')) {
                    goto next_char;
                }
                escapeQuote =  c == '"' && last_1_c == '\\' && ((last_2_c == '\\' && last_3_c == '\\' && last_4_c != '\\') ||
                                                             (last_2_c != '\\' && last_3_c == '\\') ||
                                                             (last_2_c != '\\' && last_3_c != '\\'));
            }
            tokenizer->cursor++;
            tokenizer->cursor += (c == '\r' || c == '\n') ? 1 : 0;
        } else if(c == 't' || c == 'f' || c == 'n') {
            const unsigned lenTrueNull = 4;
            const unsigned lenFalse = 5;
            const unsigned cursorLen = strlen(tokenizer->cursor);
            if (cursorLen > lenTrueNull && strncmp(tokenizer->cursor, "true", lenTrueNull) == 0) {
                tokenizer->token.type = NG5_JSON_TOKEN_LITERAL_TRUE;
                tokenizer->token.length = lenTrueNull;
            } else if (cursorLen > lenFalse && strncmp(tokenizer->cursor, "false", lenFalse) == 0) {
                tokenizer->token.type = NG5_JSON_TOKEN_LITERAL_FALSE;
                tokenizer->token.length = lenFalse;
            } else if (cursorLen > lenTrueNull && strncmp(tokenizer->cursor, "null", lenTrueNull) == 0) {
                tokenizer->token.type = NG5_JSON_TOKEN_LITERAL_NULL;
                tokenizer->token.length = lenTrueNull;
            } else {
                goto caseTokenUnknown;
            }
            tokenizer->token.column++;
            tokenizer->cursor += tokenizer->token.length;
        } else if (c == '-' || isdigit(c)) {
            unsigned fracFound = 0, expFound = 0, plusMinusFound = 0;
            bool plusMinusAllowed = false;
            bool onlyDigitsAllowed = false;
            do {
                onlyDigitsAllowed |= plusMinusAllowed;
                plusMinusAllowed = (expFound == 1);
                c = *(++tokenizer->cursor);
                fracFound += c == '.';
                expFound += (c == 'e') || (c == 'E');
                plusMinusFound += plusMinusAllowed && ((c == '+') || (c == '-')) ? 1 : 0;
                tokenizer->token.length++;
            } while ((((isdigit(c)) || (c == '.' && fracFound <= 1) ||
                (plusMinusAllowed && (plusMinusFound <= 1) && ((c == '+') || (c == '-'))) ||
                ((c == 'e' || c == 'E') && expFound <= 1))) && c != '\n' && c != '\r');

            if (!isdigit(*(tokenizer->cursor - 1))) {
                tokenizer->token.column -= tokenizer->token.length;
                goto caseTokenUnknown;
            }
            tokenizer->cursor += (c == '\r' || c == '\n') ? 1 : 0;
            tokenizer->token.type = fracFound ? NG5_JSON_TOKEN_REAL_NUMBER : NG5_JSON_TOKEN_INT_NUMBER;
        } else {
caseTokenUnknown:
            tokenizer->token.type = NG5_JSON_TOKEN_UNKNOWN;
            tokenizer->token.column++;
            tokenizer->token.length = strlen(tokenizer->cursor);
            tokenizer->cursor += tokenizer->token.length;
        }
        return &tokenizer->token;
    } else {
        return NULL;
    }
}

void carbon_json_token_dup(carbon_json_token_t *dst, const carbon_json_token_t *src)
{
    assert(dst);
    assert(src);
    memcpy(dst, src, sizeof(carbon_json_token_t));
}

void carbon_json_token_print(FILE *file, const carbon_json_token_t *token)
{
    char *string = malloc(token->length + 1);
    strncpy(string, token->string, token->length);
    string[token->length] = '\0';
    fprintf(file, "{\"type\": \"%s\", \"line\": %d, \"column\": %d, \"length\": %d, \"text\": \"%s\"}",
            TOKEN_STRING[token->type].string, token->line, token->column, token->length, string);
    free(string);
}

static bool parse_object(carbon_json_ast_node_object_t *object, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static bool parse_array(carbon_json_ast_node_array_t *array, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static void parse_string(carbon_json_ast_node_string_t *string, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static void parse_number(carbon_json_ast_node_number_t *number, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static bool parse_element(carbon_json_ast_node_element_t *element, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static bool parse_elements(carbon_json_ast_node_elements_t *elements, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx);
static bool parse_token_stream(carbon_json_t *json, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream);
static carbon_json_token_t get_token(struct vector ofType(carbon_json_token_t) *token_stream, size_t token_idx);
static void connect_child_and_parents_member(carbon_json_ast_node_member_t *member);
static void connect_child_and_parents_object(carbon_json_ast_node_object_t *object);
static void connect_child_and_parents_array(carbon_json_ast_node_array_t *array);
static void connect_child_and_parents_value(carbon_json_ast_node_value_t *value);
static void connect_child_and_parents_element(carbon_json_ast_node_element_t *element);
static void connect_child_and_parents(carbon_json_t *json);
static bool json_ast_node_member_print(FILE *file, struct err *err, carbon_json_ast_node_member_t *member);
static bool json_ast_node_object_print(FILE *file, struct err *err, carbon_json_ast_node_object_t *object);
static bool json_ast_node_array_print(FILE *file, struct err *err, carbon_json_ast_node_array_t *array);
static void json_ast_node_string_print(FILE *file, carbon_json_ast_node_string_t *string);
static bool json_ast_node_number_print(FILE *file, struct err *err, carbon_json_ast_node_number_t *number);
static bool json_ast_node_value_print(FILE *file, struct err *err, carbon_json_ast_node_value_t *value);
static bool json_ast_node_element_print(FILE *file, struct err *err, carbon_json_ast_node_element_t *element);

#define NEXT_TOKEN(x) { *x = *x + 1; }
#define PREV_TOKEN(x) { *x = *x - 1; }

NG5_EXPORT(bool)
carbon_json_parser_create(carbon_json_parser_t *parser, carbon_doc_bulk_t *partition)
{
    NG5_NON_NULL_OR_ERROR(parser)
    NG5_NON_NULL_OR_ERROR(partition)

    parser->partition = partition;
    carbon_error_init(&parser->err);

    return true;
}

bool carbon_json_parse(NG5_NULLABLE carbon_json_t *json, NG5_NULLABLE carbon_json_parse_err *error_desc,
                       carbon_json_parser_t *parser, const char *input)
{
    NG5_NON_NULL_OR_ERROR(parser)
    NG5_NON_NULL_OR_ERROR(input)

    struct vector ofType(carbon_json_token_type_e) brackets;
    struct vector ofType(carbon_json_token_t) token_stream;

    carbon_json_t retval = {
        .element = malloc(sizeof(carbon_json_ast_node_element_t))
    };
    carbon_error_init(&retval.err);
    const carbon_json_token_t *token;
    int status;

    carbon_json_tokenizer_init(&parser->tokenizer, input);
    carbon_vec_create(&brackets, NULL, sizeof(carbon_json_token_type_e), 15);
    carbon_vec_create(&token_stream, NULL, sizeof(carbon_json_token_t), 200);

    token_memory_t token_mem = {
        .init = true,
        .type = NG5_JSON_TOKEN_UNKNOWN
    };

    while ((token = carbon_json_tokenizer_next(&parser->tokenizer))) {
        if (NG5_LIKELY((status = process_token(&parser->err, error_desc, token, &brackets, &token_mem)) == true)) {
            carbon_json_token_t *newToken = VECTOR_NEW_AND_GET(&token_stream, carbon_json_token_t);
            carbon_json_token_dup(newToken, token);
        } else {
            goto cleanup;
        }
    }
    if (!carbon_vec_is_empty(&brackets)) {
        carbon_json_token_type_e type = *VECTOR_PEEK(&brackets, carbon_json_token_type_e);
        char buffer[1024];
        sprintf(&buffer[0], "Unexpected end of file: missing '%s' to match unclosed '%s' (if any)",
                type == NG5_JSON_TOKEN_SCOPE_OPEN ? "}" : "]", type == NG5_JSON_TOKEN_SCOPE_OPEN ? "{" : "[");
        status = set_error(error_desc, token, &buffer[0]);
        goto cleanup;
    }

    if (!parse_token_stream(&retval, &parser->err, &token_stream)) {
        return false;
    }

    NG5_OPTIONAL_SET_OR_ELSE(json, retval, carbon_json_drop(json));
    status = true;

    cleanup:
    carbon_vec_drop(&brackets);
    carbon_vec_drop(&token_stream);
    return status;
}

bool test_condition_value(struct err *err, carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        for (size_t i = 0; i < value->value.object->value->members.num_elems; i++) {
            carbon_json_ast_node_member_t *member = vec_get(&value->value.object->value->members, i, carbon_json_ast_node_member_t);
            if (!test_condition_value(err, &member->value.value)) {
                return false;
            }
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY: {
        carbon_json_ast_node_elements_t *elements = &value->value.array->elements;
        carbon_json_ast_node_value_type_e value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NULL;

        for (size_t i = 0; i < elements->elements.num_elems; i++) {
            carbon_json_ast_node_element_t *element = vec_get(&elements->elements, i, carbon_json_ast_node_element_t);
            value_type = ((i == 0 || value_type == NG5_JSON_AST_NODE_VALUE_TYPE_NULL) ? element->value.value_type : value_type);

            /** Test "All elements in array of same type" condition */
            if ((element->value.value_type != NG5_JSON_AST_NODE_VALUE_TYPE_NULL) &&
                (value_type == NG5_JSON_AST_NODE_VALUE_TYPE_TRUE && (element->value.value_type != NG5_JSON_AST_NODE_VALUE_TYPE_TRUE || element->value.value_type != NG5_JSON_AST_NODE_VALUE_TYPE_FALSE)) &&
                (value_type == NG5_JSON_AST_NODE_VALUE_TYPE_FALSE && (element->value.value_type != NG5_JSON_AST_NODE_VALUE_TYPE_TRUE || element->value.value_type != NG5_JSON_AST_NODE_VALUE_TYPE_FALSE)) &&
                ((value_type != NG5_JSON_AST_NODE_VALUE_TYPE_TRUE && value_type != NG5_JSON_AST_NODE_VALUE_TYPE_FALSE) && value_type != element->value.value_type))
            {
                char message[] = "JSON file constraint broken: arrays of mixed types detected";
                char *result = malloc(strlen(message) + 1);
                strcpy(result, &message[0]);
                error_WDETAILS(err, NG5_ERR_ARRAYOFMIXEDTYPES, result);
                free(result);
                return false;
            }

            switch (element->value.value_type) {
            case NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT: {
                carbon_json_ast_node_object_t *object = element->value.value.object;
                for (size_t i = 0; i < object->value->members.num_elems; i++) {
                    carbon_json_ast_node_member_t *member = vec_get(&object->value->members, i, carbon_json_ast_node_member_t);
                    if (!test_condition_value(err, &member->value.value)) {
                        return false;
                    }
                }
            } break;
            case NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY: {/** Test "No Array of Arrays" condition */
                char message[] = "JSON file constraint broken: arrays of arrays detected";
                char *result = malloc(strlen(message) + 1);
                strcpy(result, &message[0]);
                error_WDETAILS(err, NG5_ERR_ARRAYOFARRAYS, result);
                free(result);
                return false;
            }
            default:
                break;
            }
        }
    } break;
    default:
        break;
    }
    return true;
}

NG5_EXPORT(bool)
carbon_json_test_doc(struct err *err, carbon_json_t *json)
{
    return (test_condition_value(err, &json->element->value));
}

static carbon_json_token_t get_token(struct vector ofType(carbon_json_token_t) *token_stream, size_t token_idx)
{
    return *(carbon_json_token_t *) carbon_vec_at(token_stream, token_idx);
}

bool parse_members(struct err *err, carbon_json_ast_node_members_t *members, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_vec_create(&members->members, NULL, sizeof(carbon_json_ast_node_member_t), 20);
    carbon_json_token_t delimiter_token;

    do {
        carbon_json_ast_node_member_t *member = VECTOR_NEW_AND_GET(&members->members, carbon_json_ast_node_member_t);
        carbon_json_token_t keyNameToken = get_token(token_stream, *token_idx);

        member->key.value = malloc(keyNameToken.length + 1);
        strncpy(member->key.value, keyNameToken.string, keyNameToken.length);
        member->key.value[keyNameToken.length] = '\0';

        NEXT_TOKEN(token_idx); /** skip assignment token */
        NEXT_TOKEN(token_idx);
        carbon_json_token_t valueToken = get_token(token_stream, *token_idx);

        switch (valueToken.type) {
        case NG5_JSON_TOKEN_SCOPE_OPEN:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT;
            member->value.value.value.object = malloc(sizeof(carbon_json_ast_node_object_t));
            if(!parse_object(member->value.value.value.object, err, token_stream, token_idx)) {
                return false;
            }
            break;
        case NG5_JSON_TOKEN_ARRAY_BEGIN:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY;
            member->value.value.value.array = malloc(sizeof(carbon_json_ast_node_array_t));
            if(!parse_array(member->value.value.value.array, err, token_stream, token_idx)) {
                return false;
            }
            break;
        case NG5_JSON_TOKEN_STRING_LITERAL:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_STRING;
            member->value.value.value.string = malloc(sizeof(carbon_json_ast_node_string_t));
            parse_string(member->value.value.value.string, token_stream, token_idx);
            break;
        case NG5_JSON_TOKEN_INT_NUMBER:
        case NG5_JSON_TOKEN_REAL_NUMBER:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NUMBER;
            member->value.value.value.number = malloc(sizeof(carbon_json_ast_node_number_t));
            parse_number(member->value.value.value.number, token_stream, token_idx);
            break;
        case NG5_JSON_TOKEN_LITERAL_TRUE:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_TRUE;
            NEXT_TOKEN(token_idx);
            break;
        case NG5_JSON_TOKEN_LITERAL_FALSE:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_FALSE;
            NEXT_TOKEN(token_idx);
            break;
        case NG5_JSON_TOKEN_LITERAL_NULL:
            member->value.value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NULL;
            NEXT_TOKEN(token_idx);
            break;
        default:
            error(err, NG5_ERR_PARSETYPE)
            return false;
        }

        delimiter_token = get_token(token_stream, *token_idx);
        NEXT_TOKEN(token_idx);
    } while (delimiter_token.type == NG5_JSON_TOKEN_COMMA);
    PREV_TOKEN(token_idx);
    return true;
}

static bool parse_object(carbon_json_ast_node_object_t *object, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    assert(get_token(token_stream, *token_idx).type == NG5_JSON_TOKEN_SCOPE_OPEN);
    NEXT_TOKEN(token_idx);  /** Skip '{' */
    object->value = malloc(sizeof(carbon_json_ast_node_members_t));

    /** test whether this is an empty object */
    carbon_json_token_t token = get_token(token_stream, *token_idx);

    if (token.type != NG5_JSON_TOKEN_SCOPE_CLOSE) {
        if(!parse_members(err, object->value, token_stream, token_idx)) {
            return false;
        }
    } else {
        carbon_vec_create(&object->value->members, NULL, sizeof(carbon_json_ast_node_member_t), 20);
    }

    NEXT_TOKEN(token_idx);  /** Skip '}' */
    return true;
}

static bool parse_array(carbon_json_ast_node_array_t *array, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_json_token_t token = get_token(token_stream, *token_idx);
    NG5_UNUSED(token);
    assert(token.type == NG5_JSON_TOKEN_ARRAY_BEGIN);
    NEXT_TOKEN(token_idx); /** Skip '[' */

    carbon_vec_create(&array->elements.elements, NULL, sizeof(carbon_json_ast_node_element_t), 250);
    if (!parse_elements(&array->elements, err, token_stream, token_idx)) {
        return false;
    }

    NEXT_TOKEN(token_idx); /** Skip ']' */
    return true;
}

static void parse_string(carbon_json_ast_node_string_t *string, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_json_token_t token = get_token(token_stream, *token_idx);
    assert(token.type == NG5_JSON_TOKEN_STRING_LITERAL);

    string->value = malloc(token.length + 1);
    if (NG5_LIKELY(token.length > 0)) {
        strncpy(string->value, token.string, token.length);
    }
    string->value[token.length] = '\0';
    NEXT_TOKEN(token_idx);
}

static void parse_number(carbon_json_ast_node_number_t *number, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_json_token_t token = get_token(token_stream, *token_idx);
    assert(token.type == NG5_JSON_TOKEN_REAL_NUMBER || token.type == NG5_JSON_TOKEN_INT_NUMBER);

    char *value = malloc(token.length + 1);
    strncpy(value, token.string, token.length);
    value[token.length] = '\0';

    if (token.type == NG5_JSON_TOKEN_INT_NUMBER) {
        i64 assumeSigned = carbon_convert_atoi64(value);
        if (value[0] == '-') {
            number->value_type = NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER;
            number->value.signed_integer = assumeSigned;
        } else {
            u64 assumeUnsigned = carbon_convert_atoiu64(value);
            if (assumeUnsigned > (u64) assumeSigned) {
                number->value_type = NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER;
                number->value.unsigned_integer = assumeUnsigned;
            } else {
                number->value_type = NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER;
                number->value.signed_integer = assumeSigned;
            }
        }
    } else {
        number->value_type = NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER;
        setlocale( LC_ALL | ~LC_NUMERIC, "");
        number->value.float_number = strtof(value, NULL);
    }

    free(value);
    NEXT_TOKEN(token_idx);
}

static bool parse_element(carbon_json_ast_node_element_t *element, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_json_token_t token = get_token(token_stream, *token_idx);

    if (token.type == NG5_JSON_TOKEN_SCOPE_OPEN) { /** Parse object */
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT;
        element->value.value.object = malloc(sizeof(carbon_json_ast_node_object_t));
        if (!parse_object(element->value.value.object, err, token_stream, token_idx)) {
            return false;
        }
    } else if (token.type == NG5_JSON_TOKEN_ARRAY_BEGIN) { /** Parse array */
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY;
        element->value.value.array = malloc(sizeof(carbon_json_ast_node_array_t));
        if (!parse_array(element->value.value.array, err, token_stream, token_idx)) {
            return false;
        }
    } else if (token.type == NG5_JSON_TOKEN_STRING_LITERAL) { /** Parse string */
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_STRING;
        element->value.value.string = malloc(sizeof(carbon_json_ast_node_string_t));
        parse_string(element->value.value.string, token_stream, token_idx);
    } else if (token.type == NG5_JSON_TOKEN_REAL_NUMBER || token.type == NG5_JSON_TOKEN_INT_NUMBER) { /** Parse number */
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NUMBER;
        element->value.value.number = malloc(sizeof(carbon_json_ast_node_number_t));
        parse_number(element->value.value.number, token_stream, token_idx);
    } else if (token.type == NG5_JSON_TOKEN_LITERAL_TRUE) {
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_TRUE;
        NEXT_TOKEN(token_idx);
    } else if (token.type == NG5_JSON_TOKEN_LITERAL_FALSE) {
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_FALSE;
        NEXT_TOKEN(token_idx);
    } else if (token.type == NG5_JSON_TOKEN_LITERAL_NULL) {
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NULL;
        NEXT_TOKEN(token_idx);
    } else {
        element->value.value_type = NG5_JSON_AST_NODE_VALUE_TYPE_NULL;
    }
    return true;
}

static bool parse_elements(carbon_json_ast_node_elements_t *elements, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream, size_t *token_idx)
{
    carbon_json_token_t delimiter;
    do {
        if (!parse_element(VECTOR_NEW_AND_GET(&elements->elements, carbon_json_ast_node_element_t), err,
                     token_stream, token_idx)) {
            return false;
        }
        delimiter = get_token(token_stream, *token_idx);
        NEXT_TOKEN(token_idx);
    } while (delimiter.type == NG5_JSON_TOKEN_COMMA);
    PREV_TOKEN(token_idx);
    return true;
}

static bool parse_token_stream(carbon_json_t *json, struct err *err, struct vector ofType(carbon_json_token_t) *token_stream)
{
    size_t token_idx = 0;
    if (!parse_element(json->element, err, token_stream, &token_idx)) {
        return false;
    }
    connect_child_and_parents(json);
    return true;
}


static void connect_child_and_parents_member(carbon_json_ast_node_member_t *member)
{
    connect_child_and_parents_element(&member->value);
}

static void connect_child_and_parents_object(carbon_json_ast_node_object_t *object)
{
    object->value->parent = object;
    for (size_t i = 0; i < object->value->members.num_elems; i++) {
        carbon_json_ast_node_member_t *member = vec_get(&object->value->members, i, carbon_json_ast_node_member_t);
        member->parent = object->value;

        member->key.parent = member;

        member->value.parent_type = NG5_JSON_AST_NODE_ELEMENT_PARENT_TYPE_MEMBER;
        member->value.parent.member = member;

        connect_child_and_parents_member(member);
    }
}

static void connect_child_and_parents_array(carbon_json_ast_node_array_t *array)
{
    array->elements.parent = array;
    for (size_t i = 0; i < array->elements.elements.num_elems; i++) {
        carbon_json_ast_node_element_t *element = vec_get(&array->elements.elements, i, carbon_json_ast_node_element_t);
        element->parent_type = NG5_JSON_AST_NODE_ELEMENT_PARENT_TYPE_ELEMENTS;
        element->parent.elements = &array->elements;
        connect_child_and_parents_element(element);
    }
}

static void connect_child_and_parents_value(carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        connect_child_and_parents_object(value->value.object);
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        connect_child_and_parents_array(value->value.array);
        break;
    default:
        break;
    }
}

static void connect_child_and_parents_element(carbon_json_ast_node_element_t *element)
{
    element->value.parent = element;
    connect_child_and_parents_value(&element->value);
}

static void connect_child_and_parents(carbon_json_t *json)
{
    json->element->parent_type = NG5_JSON_AST_NODE_ELEMENT_PARENT_TYPE_JSON;
    json->element->parent.json = json;
    connect_child_and_parents_element(json->element);
}

static bool isValue(carbon_json_token_type_e token) {
    return (token == NG5_JSON_TOKEN_STRING_LITERAL || token == NG5_JSON_TOKEN_REAL_NUMBER || token == NG5_JSON_TOKEN_INT_NUMBER ||
        token == NG5_JSON_TOKEN_LITERAL_TRUE || token == NG5_JSON_TOKEN_LITERAL_FALSE || token == NG5_JSON_TOKEN_LITERAL_NULL );
}

static int process_token(struct err *err, carbon_json_parse_err *error_desc, const carbon_json_token_t *token, struct vector ofType(carbon_json_token_type_e) *brackets,
                        token_memory_t *token_mem)
{
    switch (token->type) {
    case NG5_JSON_TOKEN_SCOPE_OPEN:
    case NG5_JSON_TOKEN_ARRAY_BEGIN:
        carbon_vec_push(brackets, &token->type, 1);
        break;
    case NG5_JSON_TOKEN_SCOPE_CLOSE:
    case NG5_JSON_TOKEN_ARRAY_END: {
        if (!carbon_vec_is_empty(brackets)) {
            carbon_json_token_type_e bracket = *VECTOR_PEEK(brackets, carbon_json_token_type_e);
            if ((token->type == NG5_JSON_TOKEN_ARRAY_END && bracket == NG5_JSON_TOKEN_ARRAY_BEGIN) ||
                (token->type == NG5_JSON_TOKEN_SCOPE_CLOSE && bracket == NG5_JSON_TOKEN_SCOPE_OPEN)) {
                carbon_vec_pop(brackets);
            } else {
                goto pushEntry;
            }
        } else {
            pushEntry:
            carbon_vec_push(brackets, &token->type, 1);
        }
    } break;
    default:
        break;
    }

    switch (token_mem->type) {
    case NG5_JSON_TOKEN_SCOPE_OPEN:
        switch (token->type) {
        case NG5_JSON_TOKEN_STRING_LITERAL:
        case NG5_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return set_error(error_desc, token, "Expected key name or '}'");
        }
        break;
    case NG5_JSON_TOKEN_STRING_LITERAL:
        switch (token->type) {
        case NG5_JSON_TOKEN_ASSIGNMENT:
        case NG5_JSON_TOKEN_COMMA:
        case NG5_JSON_TOKEN_ARRAY_END:
        case NG5_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return set_error(error_desc, token, "Expected key name (missing ':'), enumeration (','), "
                "end of enumeration (']'), or end of object ('}')");
        }
        break;
    case NG5_JSON_TOKEN_SCOPE_CLOSE:
    case NG5_JSON_TOKEN_INT_NUMBER:
    case NG5_JSON_TOKEN_REAL_NUMBER:
    case NG5_JSON_TOKEN_LITERAL_TRUE:
    case NG5_JSON_TOKEN_LITERAL_FALSE:
    case NG5_JSON_TOKEN_LITERAL_NULL:
        switch (token->type) {
        case NG5_JSON_TOKEN_COMMA:
        case NG5_JSON_TOKEN_ARRAY_END:
        case NG5_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return set_error(error_desc, token, "Expected enumeration (','), end of enumeration (']'), "
                "or end of object ('})");
        }
        break;
    case NG5_JSON_TOKEN_ASSIGNMENT:
    case NG5_JSON_TOKEN_COMMA:
        switch (token->type) {
        case NG5_JSON_TOKEN_STRING_LITERAL:
        case NG5_JSON_TOKEN_REAL_NUMBER:
        case NG5_JSON_TOKEN_INT_NUMBER:
        case NG5_JSON_TOKEN_SCOPE_OPEN:
        case NG5_JSON_TOKEN_ARRAY_BEGIN:
        case NG5_JSON_TOKEN_LITERAL_TRUE:
        case NG5_JSON_TOKEN_LITERAL_FALSE:
        case NG5_JSON_TOKEN_LITERAL_NULL:
            break;
        default:
            return set_error(error_desc, token, "Expected key name, or value (string, number, object, enumeration, true, "
                "false, or null).");
        }
        break;
    case NG5_JSON_TOKEN_ARRAY_BEGIN:
        switch (token->type) {
        case NG5_JSON_TOKEN_ARRAY_END:
        case NG5_JSON_TOKEN_STRING_LITERAL:
        case NG5_JSON_TOKEN_REAL_NUMBER:
        case NG5_JSON_TOKEN_INT_NUMBER:
        case NG5_JSON_TOKEN_SCOPE_OPEN:
        case NG5_JSON_TOKEN_ARRAY_BEGIN:
        case NG5_JSON_TOKEN_LITERAL_TRUE:
        case NG5_JSON_TOKEN_LITERAL_FALSE:
        case NG5_JSON_TOKEN_LITERAL_NULL:
            break;
        default:
            return set_error(error_desc, token, "End of enumeration (']'), enumeration (','), or "
                "end of enumeration (']')");
        }
        break;
    case NG5_JSON_TOKEN_ARRAY_END:
        switch (token->type) {
        case NG5_JSON_TOKEN_COMMA:
        case NG5_JSON_TOKEN_ARRAY_END:
        case NG5_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return set_error(error_desc, token, "End of enumeration (']'), enumeration (','), or "
                "end of object ('}')");
        }
        break;
    case NG5_JSON_TOKEN_UNKNOWN:
        if (token_mem->init) {
            if (token->type != NG5_JSON_TOKEN_SCOPE_OPEN && token->type != NG5_JSON_TOKEN_ARRAY_BEGIN && !isValue(token->type)) {
                return set_error(error_desc, token, "Expected JSON document: missing '{' or '['");
            }
            token_mem->init = false;
        } else {
            return set_error(error_desc, token, "Unexpected token");
        }
        break;
    default:
        error(err, NG5_ERR_NOJSONTOKEN)
        return false;
    }

    token_mem->type = token->type;
    return true;
}

static int set_error(carbon_json_parse_err *error_desc, const carbon_json_token_t *token, const char *msg)
{
    if (error_desc) {
        error_desc->token = token;
        error_desc->token_type_str = token ? TOKEN_STRING[token->type].string : "(no token)";
        error_desc->msg = msg;
    }
    return false;
}

static bool json_ast_node_member_print(FILE *file, struct err *err, carbon_json_ast_node_member_t *member)
{
    fprintf(file, "\"%s\": ", member->key.value);
    return json_ast_node_value_print(file, err, &member->value.value);
}

static bool json_ast_node_object_print(FILE *file, struct err *err, carbon_json_ast_node_object_t *object)
{
    fprintf(file, "{");
    for (size_t i = 0; i < object->value->members.num_elems; i++) {
        carbon_json_ast_node_member_t *member = vec_get(&object->value->members, i, carbon_json_ast_node_member_t);
        if (!json_ast_node_member_print(file, err, member)) {
            return false;
        }
        fprintf(file, "%s", i + 1 < object->value->members.num_elems ? ", " : "");
    }
    fprintf(file, "}");
    return true;
}

static bool json_ast_node_array_print(FILE *file, struct err *err, carbon_json_ast_node_array_t *array)
{
    fprintf(file, "[");
    for (size_t i = 0; i < array->elements.elements.num_elems; i++) {
        carbon_json_ast_node_element_t *element = vec_get(&array->elements.elements, i, carbon_json_ast_node_element_t);
        if (!json_ast_node_element_print(file, err, element)) {
            return false;
        }
        fprintf(file, "%s", i + 1 < array->elements.elements.num_elems ? ", " : "");
    }
    fprintf(file, "]");
    return true;
}

static void json_ast_node_string_print(FILE *file, carbon_json_ast_node_string_t *string)
{
    fprintf(file, "\"%s\"", string->value);
}

static bool json_ast_node_number_print(FILE *file, struct err *err, carbon_json_ast_node_number_t *number)
{
    switch (number->value_type) {
    case NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER:
        fprintf(file, "%f", number->value.float_number);
        break;
    case NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER:
        fprintf(file, "%" PRIu64, number->value.unsigned_integer);
        break;
    case NG5_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER:
        fprintf(file, "%" PRIi64, number->value.signed_integer);
        break;
    default:
        error(err, NG5_ERR_NOJSONNUMBERT);
        return false;
    }
    return true;
}

static bool json_ast_node_value_print(FILE *file, struct err *err, carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        if (!json_ast_node_object_print(file, err, value->value.object)) {
            return false;
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        if (!json_ast_node_array_print(file, err, value->value.array)) {
            return false;
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_STRING:
        json_ast_node_string_print(file, value->value.string);
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_NUMBER:
        if (!json_ast_node_number_print(file, err, value->value.number)) {
            return false;
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_TRUE:
        fprintf(file, "true");
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_FALSE:
        fprintf(file, "false");
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_NULL:
        fprintf(file, "null");
        break;
    default:
        error(err, NG5_ERR_NOTYPE);
        return false;
    }
    return true;
}

static bool json_ast_node_element_print(FILE *file, struct err *err, carbon_json_ast_node_element_t *element)
{
    return json_ast_node_value_print(file, err, &element->value);
}

static bool json_ast_node_value_drop(carbon_json_ast_node_value_t *value, struct err *err);

static bool json_ast_node_element_drop(carbon_json_ast_node_element_t *element, struct err *err)
{
    return json_ast_node_value_drop(&element->value, err);
}

static bool json_ast_node_member_drop(carbon_json_ast_node_member_t *member, struct err *err)
{
    free(member->key.value);
    return json_ast_node_element_drop(&member->value, err);
}

static bool json_ast_node_members_drop(carbon_json_ast_node_members_t *members, struct err *err)
{
    for (size_t i = 0; i < members->members.num_elems; i++) {
        carbon_json_ast_node_member_t *member = vec_get(&members->members, i, carbon_json_ast_node_member_t);
        if (!json_ast_node_member_drop(member, err)) {
            return false;
        }
    }
    carbon_vec_drop(&members->members);
    return true;
}

static bool json_ast_node_elements_drop(carbon_json_ast_node_elements_t *elements, struct err *err)
{
    for (size_t i = 0; i < elements->elements.num_elems; i++) {
        carbon_json_ast_node_element_t *element = vec_get(&elements->elements, i, carbon_json_ast_node_element_t);
        if (!json_ast_node_element_drop(element, err)) {
            return false;
        }
    }
    carbon_vec_drop(&elements->elements);
    return true;
}

static bool json_ast_node_object_drop(carbon_json_ast_node_object_t *object, struct err *err)
{
    if (!json_ast_node_members_drop(object->value, err)) {
        return false;
    } else {
        free(object->value);
        return true;
    }
}

static bool json_ast_node_array_drop(carbon_json_ast_node_array_t *array, struct err *err)
{
    return json_ast_node_elements_drop(&array->elements, err);
}

static void json_ast_node_string_drop(carbon_json_ast_node_string_t *string)
{
    free(string->value);
}

static void json_ast_node_number_drop(carbon_json_ast_node_number_t *number)
{
    NG5_UNUSED(number);
}

static bool json_ast_node_value_drop(carbon_json_ast_node_value_t *value, struct err *err)
{
    switch (value->value_type) {
    case NG5_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        if (!json_ast_node_object_drop(value->value.object, err)) {
            return false;
        } else {
            free(value->value.object);
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        if (!json_ast_node_array_drop(value->value.array, err)) {
            return false;
        } else {
            free(value->value.array);
        }
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_STRING:
        json_ast_node_string_drop(value->value.string);
        free(value->value.string);
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_NUMBER:
        json_ast_node_number_drop(value->value.number);
        free(value->value.number);
        break;
    case NG5_JSON_AST_NODE_VALUE_TYPE_TRUE:
    case NG5_JSON_AST_NODE_VALUE_TYPE_FALSE:
    case NG5_JSON_AST_NODE_VALUE_TYPE_NULL:
        break;
    default:
        error(err, NG5_ERR_NOTYPE)
        return false;

    }
    return true;
}

bool carbon_json_drop(carbon_json_t *json)
{
    carbon_json_ast_node_element_t *element = json->element;
    if (!json_ast_node_value_drop(&element->value, &json->err)) {
        return false;
    } else {
        free(json->element);
        return true;
    }
}

bool carbon_json_print(FILE *file, carbon_json_t *json)
{
    return json_ast_node_element_print(file, &json->err, json->element);
}