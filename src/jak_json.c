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
#include <jak_json.h>
#include <jak_doc.h>
#include <jak_utils_convert.h>
#include <jak_utils_numbers.h>
#include "jak_json.h"

static struct {
        enum json_token_type token;
        const char *string;
} TOKEN_STRING[] = {{.token = OBJECT_OPEN, .string = "OBJECT_OPEN"},
                    {.token = OBJECT_CLOSE, .string = "OBJECT_CLOSE"},
                    {.token = LITERAL_STRING, .string = "JSON_TOKEN_STRING"},
                    {.token = LITERAL_INT, .string = "LITERAL_INT"},
                    {.token = LITERAL_FLOAT, .string = "LITERAL_FLOAT"},
                    {.token = LITERAL_TRUE, .string = "LITERAL_TRUE"},
                    {.token = LITERAL_FALSE, .string = "LITERAL_FALSE"},
                    {.token = LITERAL_NULL, .string = "LITERAL_NULL"},
                    {.token = COMMA, .string = "COMMA"},
                    {.token = ASSIGN, .string = "JSON_TOKEN_ASSIGMENT"},
                    {.token = ARRAY_OPEN, .string = "ARRAY_OPEN"},
                    {.token = ARRAY_CLOSE, .string = "ARRAY_CLOSE"},
                    {.token = JSON_UNKNOWN, .string = "JSON_UNKNOWN"}};

struct token_memory {
        enum json_token_type type;
        bool init;
};

static int process_token(struct jak_error *err, struct jak_json_err *error_desc, const struct jak_json_token *token,
                         struct jak_vector ofType(enum json_token_type) *brackets, struct token_memory *token_mem);

static int set_error(struct jak_json_err *error_desc, const struct jak_json_token *token, const char *msg);

bool json_tokenizer_init(struct jak_json_tokenizer *tokenizer, const char *input)
{
        JAK_ERROR_IF_NULL(tokenizer)
        JAK_ERROR_IF_NULL(input)
        tokenizer->cursor = input;
        tokenizer->token =
                (struct jak_json_token) {.type = JSON_UNKNOWN, .length = 0, .column = 0, .line = 1, .string = NULL};
        error_init(&tokenizer->err);
        return true;
}

static void
parse_string_token(struct jak_json_tokenizer *tokenizer, char c, char delimiter, char delimiter2, char delimiter3,
                   bool include_start, bool include_end)
{
        bool escapeQuote = false;
        tokenizer->token.type = LITERAL_STRING;
        if (!include_start) {
                tokenizer->token.string++;
        }
        tokenizer->token.column++;
        char last_1_c = '\0', last_2_c = '\0', last_3_c = '\0', last_4_c = '\0';
        c = *(++tokenizer->cursor);
        while ((escapeQuote || (c != delimiter && c != delimiter2 && c != delimiter3)) && c != '\r' && c != '\n') {
                next_char:
                tokenizer->token.length++;
                last_4_c = last_3_c;
                last_3_c = last_2_c;
                last_2_c = last_1_c;
                last_1_c = c;
                c = *(++tokenizer->cursor);
                if (JAK_UNLIKELY(c == '\\' && last_1_c == '\\')) {
                        goto next_char;
                }
                escapeQuote = c == '"' && last_1_c == '\\'
                              && ((last_2_c == '\\' && last_3_c == '\\' && last_4_c != '\\')
                                  || (last_2_c != '\\' && last_3_c == '\\')
                                  || (last_2_c != '\\' && last_3_c != '\\'));
        }

        if (include_end) {
                tokenizer->token.length++;
        } else {
                tokenizer->cursor++;
        }

        tokenizer->cursor += (c == '\r' || c == '\n') ? 1 : 0;
}

const struct jak_json_token *json_tokenizer_next(struct jak_json_tokenizer *tokenizer)
{
        if (JAK_LIKELY(*tokenizer->cursor != '\0')) {
                char c = *tokenizer->cursor;
                tokenizer->token.string = tokenizer->cursor;
                tokenizer->token.column += tokenizer->token.length;
                tokenizer->token.length = 0;
                if (c == '\n' || c == '\r') {
                        tokenizer->token.line += c == '\n' ? 1 : 0;
                        tokenizer->token.column = c == '\n' ? 0 : tokenizer->token.column;
                        tokenizer->cursor++;
                        return json_tokenizer_next(tokenizer);
                } else if (isspace(c)) {
                        do {
                                tokenizer->cursor++;
                                tokenizer->token.column++;
                        } while (isspace(c = *tokenizer->cursor) && c != '\n');
                        return json_tokenizer_next(tokenizer);
                } else if (c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
                        tokenizer->token.type =
                                c == '{' ? OBJECT_OPEN : c == '}' ? OBJECT_CLOSE : c == '[' ? ARRAY_OPEN : c == ']'
                                                                                                           ? ARRAY_CLOSE
                                                                                                           : c == ':'
                                                                                                             ? ASSIGN
                                                                                                             : COMMA;
                        tokenizer->token.column++;
                        tokenizer->token.length = 1;
                        tokenizer->cursor++;
                } else if (c != '"' && (isalpha(c) || c == '_') &&
                           (strlen(tokenizer->cursor) >= 4 && (strncmp(tokenizer->cursor, "null", 4) != 0 &&
                                                               strncmp(tokenizer->cursor, "true", 4) != 0)) &&
                           (strlen(tokenizer->cursor) >= 5 && strncmp(tokenizer->cursor, "false", 5) != 0)) {
                        parse_string_token(tokenizer, c, ' ', ':', ',', true, true);
                } else if (c == '"') {
                        parse_string_token(tokenizer, c, '"', '"', '"', false, false);
                } else if (c == 't' || c == 'f' || c == 'n') {
                        const unsigned lenTrueNull = 4;
                        const unsigned lenFalse = 5;
                        const unsigned cursorLen = strlen(tokenizer->cursor);
                        if (cursorLen >= lenTrueNull && strncmp(tokenizer->cursor, "true", lenTrueNull) == 0) {
                                tokenizer->token.type = LITERAL_TRUE;
                                tokenizer->token.length = lenTrueNull;
                        } else if (cursorLen >= lenFalse && strncmp(tokenizer->cursor, "false", lenFalse) == 0) {
                                tokenizer->token.type = LITERAL_FALSE;
                                tokenizer->token.length = lenFalse;
                        } else if (cursorLen >= lenTrueNull && strncmp(tokenizer->cursor, "null", lenTrueNull) == 0) {
                                tokenizer->token.type = LITERAL_NULL;
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
                        } while ((((isdigit(c)) || (c == '.' && fracFound <= 1)
                                   || (plusMinusAllowed && (plusMinusFound <= 1) && ((c == '+') || (c == '-')))
                                   || ((c == 'e' || c == 'E') && expFound <= 1))) && c != '\n' && c != '\r');

                        if (!isdigit(*(tokenizer->cursor - 1))) {
                                tokenizer->token.column -= tokenizer->token.length;
                                goto caseTokenUnknown;
                        }
                        tokenizer->cursor += (c == '\r' || c == '\n') ? 1 : 0;
                        tokenizer->token.type = fracFound ? LITERAL_FLOAT : LITERAL_INT;
                } else {
                        caseTokenUnknown:
                        tokenizer->token.type = JSON_UNKNOWN;
                        tokenizer->token.column++;
                        tokenizer->token.length = strlen(tokenizer->cursor);
                        tokenizer->cursor += tokenizer->token.length;
                }
                return &tokenizer->token;
        } else {
                return NULL;
        }
}

void json_token_dup(struct jak_json_token *dst, const struct jak_json_token *src)
{
        JAK_ASSERT(dst);
        JAK_ASSERT(src);
        memcpy(dst, src, sizeof(struct jak_json_token));
}

void json_token_print(FILE *file, const struct jak_json_token *token)
{
        char *string = JAK_MALLOC(token->length + 1);
        strncpy(string, token->string, token->length);
        string[token->length] = '\0';
        fprintf(file,
                "{\"type\": \"%s\", \"line\": %d, \"column\": %d, \"length\": %d, \"text\": \"%s\"}",
                TOKEN_STRING[token->type].string,
                token->line,
                token->column,
                token->length,
                string);
        free(string);
}

static bool parse_object(struct jak_json_object_t *object, struct jak_error *err,
                         struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx);

static bool parse_array(struct jak_json_array *array, struct jak_error *err,
                        struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx);

static void parse_string(struct jak_json_string *string, struct jak_vector ofType(struct jak_json_token) *token_stream,
                         size_t *token_idx);

static void parse_number(struct jak_json_number *number, struct jak_vector ofType(struct jak_json_token) *token_stream,
                         size_t *token_idx);

static bool parse_element(struct jak_json_element *element, struct jak_error *err,
                          struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx);

static bool parse_elements(struct jak_json_elements *elements, struct jak_error *err,
                           struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx);

static bool parse_token_stream(struct jak_json *json, struct jak_error *err,
                               struct jak_vector ofType(struct jak_json_token) *token_stream);

static struct jak_json_token get_token(struct jak_vector ofType(struct jak_json_token) *token_stream, size_t token_idx);

static void connect_child_and_parents_member(struct jak_json_prop *member);

static void connect_child_and_parents_object(struct jak_json_object_t *object);

static void connect_child_and_parents_array(struct jak_json_array *array);

static void connect_child_and_parents_value(struct jak_json_node_value *value);

static void connect_child_and_parents_element(struct jak_json_element *element);

static void connect_child_and_parents(struct jak_json *json);

static bool json_ast_node_member_print(FILE *file, struct jak_error *err, struct jak_json_prop *member);

static bool json_ast_node_object_print(FILE *file, struct jak_error *err, struct jak_json_object_t *object);

static bool json_ast_node_array_print(FILE *file, struct jak_error *err, struct jak_json_array *array);

static void json_ast_node_string_print(FILE *file, struct jak_json_string *string);

static bool json_ast_node_number_print(FILE *file, struct jak_error *err, struct jak_json_number *number);

static bool json_ast_node_value_print(FILE *file, struct jak_error *err, struct jak_json_node_value *value);

static bool json_ast_node_element_print(FILE *file, struct jak_error *err, struct jak_json_element *element);

#define NEXT_TOKEN(x) { *x = *x + 1; }
#define PREV_TOKEN(x) { *x = *x - 1; }

bool json_parser_create(struct jak_json_parser *parser)
{
        JAK_ERROR_IF_NULL(parser)

        error_init(&parser->err);

        return true;
}

bool
json_parse(struct jak_json *json, struct jak_json_err *error_desc, struct jak_json_parser *parser, const char *input)
{
        JAK_ERROR_IF_NULL(parser)
        JAK_ERROR_IF_NULL(input)

        struct jak_vector ofType(enum json_token_type) brackets;
        struct jak_vector ofType(struct jak_json_token) token_stream;

        struct jak_json retval;
        JAK_zero_memory(&retval, sizeof(struct jak_json))
        retval.element = JAK_MALLOC(sizeof(struct jak_json_element));
        error_init(&retval.err);
        const struct jak_json_token *token;
        int status;

        json_tokenizer_init(&parser->tokenizer, input);
        vec_create(&brackets, NULL, sizeof(enum json_token_type), 15);
        vec_create(&token_stream, NULL, sizeof(struct jak_json_token), 200);

        struct token_memory token_mem = {.init = true, .type = JSON_UNKNOWN};

        while ((token = json_tokenizer_next(&parser->tokenizer))) {
                if (JAK_LIKELY(
                        (status = process_token(&parser->err, error_desc, token, &brackets, &token_mem)) == true)) {
                        struct jak_json_token *newToken = vec_new_and_get(&token_stream, struct jak_json_token);
                        json_token_dup(newToken, token);
                } else {
                        goto cleanup;
                }
        }
        if (!vec_is_empty(&brackets)) {
                enum json_token_type type = *VECTOR_PEEK(&brackets, enum json_token_type);
                char buffer[1024];
                sprintf(&buffer[0],
                        "Unexpected end of file: missing '%s' to match unclosed '%s' (if any)",
                        type == OBJECT_OPEN ? "}" : "]",
                        type == OBJECT_OPEN ? "{" : "[");
                status = set_error(error_desc, token, &buffer[0]);
                goto cleanup;
        }

        if (!parse_token_stream(&retval, &parser->err, &token_stream)) {
                return false;
        }

        JAK_optional_set_or_else(json, retval, json_drop(json));
        status = true;

        cleanup:
        vec_drop(&brackets);
        vec_drop(&token_stream);
        return status;
}

bool test_condition_value(struct jak_error *err, struct jak_json_node_value *value)
{
        switch (value->value_type) {
                case JSON_VALUE_OBJECT:
                        for (size_t i = 0; i < value->value.object->value->members.num_elems; i++) {
                                struct jak_json_prop *member = vec_get(&value->value.object->value->members, i,
                                                                       struct jak_json_prop);
                                if (!test_condition_value(err, &member->value.value)) {
                                        return false;
                                }
                        }
                        break;
                case JSON_VALUE_ARRAY: {
                        struct jak_json_elements *elements = &value->value.array->elements;
                        enum json_value_type value_type = JSON_VALUE_NULL;

                        for (size_t i = 0; i < elements->elements.num_elems; i++) {
                                struct jak_json_element *element = vec_get(&elements->elements, i,
                                                                           struct jak_json_element);
                                value_type =
                                        ((i == 0 || value_type == JSON_VALUE_NULL) ? element->value.value_type
                                                                                   : value_type);

                                /** Test "All elements in array of same type" condition */
                                if ((element->value.value_type != JSON_VALUE_NULL) && (value_type == JSON_VALUE_TRUE
                                                                                       && (element->value.value_type !=
                                                                                           JSON_VALUE_TRUE
                                                                                           ||
                                                                                           element->value.value_type !=
                                                                                           JSON_VALUE_FALSE))
                                    && (value_type == JSON_VALUE_FALSE && (element->value.value_type != JSON_VALUE_TRUE
                                                                           || element->value.value_type !=
                                                                              JSON_VALUE_FALSE))
                                    && ((value_type != JSON_VALUE_TRUE && value_type != JSON_VALUE_FALSE)
                                        && value_type != element->value.value_type)) {
                                        char message[] = "JSON file constraint broken: arrays of mixed types detected";
                                        char *result = JAK_MALLOC(strlen(message) + 1);
                                        strcpy(result, &message[0]);
                                        error_with_details(err, JAK_ERR_ARRAYOFMIXEDTYPES, result);
                                        free(result);
                                        return false;
                                }

                                switch (element->value.value_type) {
                                        case JSON_VALUE_OBJECT: {
                                                struct jak_json_object_t *object = element->value.value.object;
                                                for (size_t i = 0; i < object->value->members.num_elems; i++) {
                                                        struct jak_json_prop
                                                                *member = vec_get(&object->value->members, i,
                                                                                  struct jak_json_prop);
                                                        if (!test_condition_value(err, &member->value.value)) {
                                                                return false;
                                                        }
                                                }
                                        }
                                                break;
                                        case JSON_VALUE_ARRAY: {/** Test "No Array of Arrays" condition */
                                                char message[] = "JSON file constraint broken: arrays of arrays detected";
                                                char *result = JAK_MALLOC(strlen(message) + 1);
                                                strcpy(result, &message[0]);
                                                error_with_details(err, JAK_ERR_ARRAYOFARRAYS, result);
                                                free(result);
                                                return false;
                                        }
                                        default:
                                                break;
                                }
                        }
                }
                        break;
                default:
                        break;
        }
        return true;
}

bool json_test(struct jak_error *err, struct jak_json *json)
{
        return (test_condition_value(err, &json->element->value));
}

static struct jak_json_token get_token(struct jak_vector ofType(struct jak_json_token) *token_stream, size_t token_idx)
{
        return *(struct jak_json_token *) vec_at(token_stream, token_idx);
}

bool parse_members(struct jak_error *err, struct jak_json_members *members,
                   struct jak_vector ofType(struct jak_json_token) *token_stream,
                   size_t *token_idx)
{
        vec_create(&members->members, NULL, sizeof(struct jak_json_prop), 20);
        struct jak_json_token delimiter_token;

        do {
                struct jak_json_prop *member = vec_new_and_get(&members->members, struct jak_json_prop);
                struct jak_json_token keyNameToken = get_token(token_stream, *token_idx);

                member->key.value = JAK_MALLOC(keyNameToken.length + 1);
                strncpy(member->key.value, keyNameToken.string, keyNameToken.length);
                member->key.value[keyNameToken.length] = '\0';

                NEXT_TOKEN(token_idx); /** skip assignment token */
                NEXT_TOKEN(token_idx);
                struct jak_json_token valueToken = get_token(token_stream, *token_idx);

                switch (valueToken.type) {
                        case OBJECT_OPEN:
                                member->value.value.value_type = JSON_VALUE_OBJECT;
                                member->value.value.value.object = JAK_MALLOC(sizeof(struct jak_json_object_t));
                                if (!parse_object(member->value.value.value.object, err, token_stream, token_idx)) {
                                        return false;
                                }
                                break;
                        case ARRAY_OPEN:
                                member->value.value.value_type = JSON_VALUE_ARRAY;
                                member->value.value.value.array = JAK_MALLOC(sizeof(struct jak_json_array));
                                if (!parse_array(member->value.value.value.array, err, token_stream, token_idx)) {
                                        return false;
                                }
                                break;
                        case LITERAL_STRING:
                                member->value.value.value_type = JSON_VALUE_STRING;
                                member->value.value.value.string = JAK_MALLOC(sizeof(struct jak_json_string));
                                parse_string(member->value.value.value.string, token_stream, token_idx);
                                break;
                        case LITERAL_INT:
                        case LITERAL_FLOAT:
                                member->value.value.value_type = JSON_VALUE_NUMBER;
                                member->value.value.value.number = JAK_MALLOC(sizeof(struct jak_json_number));
                                parse_number(member->value.value.value.number, token_stream, token_idx);
                                break;
                        case LITERAL_TRUE:
                                member->value.value.value_type = JSON_VALUE_TRUE;
                                NEXT_TOKEN(token_idx);
                                break;
                        case LITERAL_FALSE:
                                member->value.value.value_type = JSON_VALUE_FALSE;
                                NEXT_TOKEN(token_idx);
                                break;
                        case LITERAL_NULL:
                                member->value.value.value_type = JSON_VALUE_NULL;
                                NEXT_TOKEN(token_idx);
                                break;
                        default: error(err, JAK_ERR_PARSETYPE)
                                return false;
                }

                delimiter_token = get_token(token_stream, *token_idx);
                NEXT_TOKEN(token_idx);
        } while (delimiter_token.type == COMMA);
        PREV_TOKEN(token_idx);
        return true;
}

static bool parse_object(struct jak_json_object_t *object, struct jak_error *err,
                         struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx)
{
        JAK_ASSERT(get_token(token_stream, *token_idx).type == OBJECT_OPEN);
        NEXT_TOKEN(token_idx);  /** Skip '{' */
        object->value = JAK_MALLOC(sizeof(struct jak_json_members));

        /** test whether this is an empty object */
        struct jak_json_token token = get_token(token_stream, *token_idx);

        if (token.type != OBJECT_CLOSE) {
                if (!parse_members(err, object->value, token_stream, token_idx)) {
                        return false;
                }
        } else {
                vec_create(&object->value->members, NULL, sizeof(struct jak_json_prop), 20);
        }

        NEXT_TOKEN(token_idx);  /** Skip '}' */
        return true;
}

static bool parse_array(struct jak_json_array *array, struct jak_error *err,
                        struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx)
{
        struct jak_json_token token = get_token(token_stream, *token_idx);
        JAK_UNUSED(token);
        JAK_ASSERT(token.type == ARRAY_OPEN);
        NEXT_TOKEN(token_idx); /** Skip '[' */

        vec_create(&array->elements.elements, NULL, sizeof(struct jak_json_element), 250);
        if (!parse_elements(&array->elements, err, token_stream, token_idx)) {
                return false;
        }

        NEXT_TOKEN(token_idx); /** Skip ']' */
        return true;
}

static void parse_string(struct jak_json_string *string, struct jak_vector ofType(struct jak_json_token) *token_stream,
                         size_t *token_idx)
{
        struct jak_json_token token = get_token(token_stream, *token_idx);
        JAK_ASSERT(token.type == LITERAL_STRING);

        string->value = JAK_MALLOC(token.length + 1);
        if (JAK_LIKELY(token.length > 0)) {
                strncpy(string->value, token.string, token.length);
        }
        string->value[token.length] = '\0';
        NEXT_TOKEN(token_idx);
}

static void parse_number(struct jak_json_number *number, struct jak_vector ofType(struct jak_json_token) *token_stream,
                         size_t *token_idx)
{
        struct jak_json_token token = get_token(token_stream, *token_idx);
        JAK_ASSERT(token.type == LITERAL_FLOAT || token.type == LITERAL_INT);

        char *value = JAK_MALLOC(token.length + 1);
        strncpy(value, token.string, token.length);
        value[token.length] = '\0';

        if (token.type == LITERAL_INT) {
                jak_i64 assumeSigned = convert_atoi64(value);
                if (value[0] == '-') {
                        number->value_type = JSON_NUMBER_SIGNED;
                        number->value.signed_integer = assumeSigned;
                } else {
                        jak_u64 assumeUnsigned = convert_atoiu64(value);
                        if (assumeUnsigned >= (jak_u64) assumeSigned) {
                                number->value_type = JSON_NUMBER_UNSIGNED;
                                number->value.unsigned_integer = assumeUnsigned;
                        } else {
                                number->value_type = JSON_NUMBER_SIGNED;
                                number->value.signed_integer = assumeSigned;
                        }
                }
        } else {
                number->value_type = JSON_NUMBER_FLOAT;
                setlocale(LC_ALL | ~LC_NUMERIC, "");
                number->value.float_number = strtof(value, NULL);
        }

        free(value);
        NEXT_TOKEN(token_idx);
}

static bool parse_element(struct jak_json_element *element, struct jak_error *err,
                          struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx)
{
        struct jak_json_token token = get_token(token_stream, *token_idx);

        if (token.type == OBJECT_OPEN) { /** Parse object */
                element->value.value_type = JSON_VALUE_OBJECT;
                element->value.value.object = JAK_MALLOC(sizeof(struct jak_json_object_t));
                if (!parse_object(element->value.value.object, err, token_stream, token_idx)) {
                        return false;
                }
        } else if (token.type == ARRAY_OPEN) { /** Parse array */
                element->value.value_type = JSON_VALUE_ARRAY;
                element->value.value.array = JAK_MALLOC(sizeof(struct jak_json_array));
                if (!parse_array(element->value.value.array, err, token_stream, token_idx)) {
                        return false;
                }
        } else if (token.type == LITERAL_STRING) { /** Parse string */
                element->value.value_type = JSON_VALUE_STRING;
                element->value.value.string = JAK_MALLOC(sizeof(struct jak_json_string));
                parse_string(element->value.value.string, token_stream, token_idx);
        } else if (token.type == LITERAL_FLOAT || token.type == LITERAL_INT) { /** Parse number */
                element->value.value_type = JSON_VALUE_NUMBER;
                element->value.value.number = JAK_MALLOC(sizeof(struct jak_json_number));
                parse_number(element->value.value.number, token_stream, token_idx);
        } else if (token.type == LITERAL_TRUE) {
                element->value.value_type = JSON_VALUE_TRUE;
                NEXT_TOKEN(token_idx);
        } else if (token.type == LITERAL_FALSE) {
                element->value.value_type = JSON_VALUE_FALSE;
                NEXT_TOKEN(token_idx);
        } else if (token.type == LITERAL_NULL) {
                element->value.value_type = JSON_VALUE_NULL;
                NEXT_TOKEN(token_idx);
        } else {
                element->value.value_type = JSON_VALUE_NULL;
        }
        return true;
}

static bool parse_elements(struct jak_json_elements *elements, struct jak_error *err,
                           struct jak_vector ofType(struct jak_json_token) *token_stream, size_t *token_idx)
{
        struct jak_json_token delimiter;
        do {
                struct jak_json_token current = get_token(token_stream, *token_idx);
                if (current.type != ARRAY_CLOSE && current.type != OBJECT_CLOSE) {
                        if (!parse_element(vec_new_and_get(&elements->elements, struct jak_json_element),
                                           err,
                                           token_stream,
                                           token_idx)) {
                                return false;
                        }
                }
                delimiter = get_token(token_stream, *token_idx);
                NEXT_TOKEN(token_idx);
        } while (delimiter.type == COMMA);
        PREV_TOKEN(token_idx);
        return true;
}

static bool parse_token_stream(struct jak_json *json, struct jak_error *err,
                               struct jak_vector ofType(struct jak_json_token) *token_stream)
{
        size_t token_idx = 0;
        if (!parse_element(json->element, err, token_stream, &token_idx)) {
                return false;
        }
        connect_child_and_parents(json);
        return true;
}

static void connect_child_and_parents_member(struct jak_json_prop *member)
{
        connect_child_and_parents_element(&member->value);
}

static void connect_child_and_parents_object(struct jak_json_object_t *object)
{
        object->value->parent = object;
        for (size_t i = 0; i < object->value->members.num_elems; i++) {
                struct jak_json_prop *member = vec_get(&object->value->members, i, struct jak_json_prop);
                member->parent = object->value;

                member->key.parent = member;

                member->value.parent_type = JSON_PARENT_MEMBER;
                member->value.parent.member = member;

                connect_child_and_parents_member(member);
        }
}

static void connect_child_and_parents_array(struct jak_json_array *array)
{
        array->elements.parent = array;
        for (size_t i = 0; i < array->elements.elements.num_elems; i++) {
                struct jak_json_element *element = vec_get(&array->elements.elements, i, struct jak_json_element);
                element->parent_type = JSON_PARENT_ELEMENTS;
                element->parent.elements = &array->elements;
                connect_child_and_parents_element(element);
        }
}

static void connect_child_and_parents_value(struct jak_json_node_value *value)
{
        switch (value->value_type) {
                case JSON_VALUE_OBJECT:
                        connect_child_and_parents_object(value->value.object);
                        break;
                case JSON_VALUE_ARRAY:
                        connect_child_and_parents_array(value->value.array);
                        break;
                default:
                        break;
        }
}

static void connect_child_and_parents_element(struct jak_json_element *element)
{
        element->value.parent = element;
        connect_child_and_parents_value(&element->value);
}

static void connect_child_and_parents(struct jak_json *json)
{
        json->element->parent_type = JSON_PARENT_OBJECT;
        json->element->parent.json = json;
        connect_child_and_parents_element(json->element);
}

static bool isValue(enum json_token_type token)
{
        return (token == LITERAL_STRING || token == LITERAL_FLOAT || token == LITERAL_INT || token == LITERAL_TRUE
                || token == LITERAL_FALSE || token == LITERAL_NULL);
}

static int process_token(struct jak_error *err, struct jak_json_err *error_desc, const struct jak_json_token *token,
                         struct jak_vector ofType(enum json_token_type) *brackets, struct token_memory *token_mem)
{
        switch (token->type) {
                case OBJECT_OPEN:
                case ARRAY_OPEN:
                        vec_push(brackets, &token->type, 1);
                        break;
                case OBJECT_CLOSE:
                case ARRAY_CLOSE: {
                        if (!vec_is_empty(brackets)) {
                                enum json_token_type bracket = *VECTOR_PEEK(brackets, enum json_token_type);
                                if ((token->type == ARRAY_CLOSE && bracket == ARRAY_OPEN)
                                    || (token->type == OBJECT_CLOSE && bracket == OBJECT_OPEN)) {
                                        vec_pop(brackets);
                                } else {
                                        goto pushEntry;
                                }
                        } else {
                                pushEntry:
                                vec_push(brackets, &token->type, 1);
                        }
                }
                        break;
                default:
                        break;
        }

        switch (token_mem->type) {
                case OBJECT_OPEN:
                        switch (token->type) {
                                case LITERAL_STRING:
                                case OBJECT_CLOSE:
                                        break;
                                default:
                                        return set_error(error_desc, token, "Expected key name or '}'");
                        }
                        break;
                case LITERAL_STRING:
                        switch (token->type) {
                                case ASSIGN:
                                case COMMA:
                                case ARRAY_CLOSE:
                                case OBJECT_CLOSE:
                                        break;
                                default:
                                        return set_error(error_desc, token,
                                                         "Expected key name (missing ':'), enumeration (','), "
                                                         "end of enumeration (']'), or end of object ('}')");
                        }
                        break;
                case OBJECT_CLOSE:
                case LITERAL_INT:
                case LITERAL_FLOAT:
                case LITERAL_TRUE:
                case LITERAL_FALSE:
                case LITERAL_NULL:
                        switch (token->type) {
                                case COMMA:
                                case ARRAY_CLOSE:
                                case OBJECT_CLOSE:
                                        break;
                                default:
                                        return set_error(error_desc, token,
                                                         "Expected enumeration (','), end of enumeration (']'), "
                                                         "or end of object ('})");
                        }
                        break;
                case ASSIGN:
                case COMMA:
                        switch (token->type) {
                                case LITERAL_STRING:
                                case LITERAL_FLOAT:
                                case LITERAL_INT:
                                case OBJECT_OPEN:
                                case ARRAY_OPEN:
                                case LITERAL_TRUE:
                                case LITERAL_FALSE:
                                case LITERAL_NULL:
                                        break;
                                default:
                                        return set_error(error_desc,
                                                         token,
                                                         "Expected key name, or value (string, number, object, enumeration, true, "
                                                         "false, or null).");
                        }
                        break;
                case ARRAY_OPEN:
                        switch (token->type) {
                                case ARRAY_CLOSE:
                                case LITERAL_STRING:
                                case LITERAL_FLOAT:
                                case LITERAL_INT:
                                case OBJECT_OPEN:
                                case ARRAY_OPEN:
                                case LITERAL_TRUE:
                                case LITERAL_FALSE:
                                case LITERAL_NULL:
                                        break;
                                default:
                                        return set_error(error_desc, token,
                                                         "End of enumeration (']'), enumeration (','), or "
                                                         "end of enumeration (']')");
                        }
                        break;
                case ARRAY_CLOSE:
                        switch (token->type) {
                                case COMMA:
                                case ARRAY_CLOSE:
                                case OBJECT_CLOSE:
                                        break;
                                default:
                                        return set_error(error_desc, token,
                                                         "End of enumeration (']'), enumeration (','), or "
                                                         "end of object ('}')");
                        }
                        break;
                case JSON_UNKNOWN:
                        if (token_mem->init) {
                                if (token->type != OBJECT_OPEN && token->type != ARRAY_OPEN && !isValue(token->type)) {
                                        return set_error(error_desc, token,
                                                         "Expected JSON document: missing '{' or '['");
                                }
                                token_mem->init = false;
                        } else {
                                return set_error(error_desc, token, "Unexpected token");
                        }
                        break;
                default: error(err, JAK_ERR_NOJSONTOKEN)
                        return false;
        }

        token_mem->type = token->type;
        return true;
}

static int set_error(struct jak_json_err *error_desc, const struct jak_json_token *token, const char *msg)
{
        if (error_desc) {
                error_desc->token = token;
                error_desc->token_type_str = token ? TOKEN_STRING[token->type].string : "(no token)";
                error_desc->msg = msg;
        }
        return false;
}

static bool json_ast_node_member_print(FILE *file, struct jak_error *err, struct jak_json_prop *member)
{
        fprintf(file, "\"%s\": ", member->key.value);
        return json_ast_node_value_print(file, err, &member->value.value);
}

static bool json_ast_node_object_print(FILE *file, struct jak_error *err, struct jak_json_object_t *object)
{
        fprintf(file, "{");
        for (size_t i = 0; i < object->value->members.num_elems; i++) {
                struct jak_json_prop *member = vec_get(&object->value->members, i, struct jak_json_prop);
                if (!json_ast_node_member_print(file, err, member)) {
                        return false;
                }
                fprintf(file, "%s", i + 1 < object->value->members.num_elems ? ", " : "");
        }
        fprintf(file, "}");
        return true;
}

static bool json_ast_node_array_print(FILE *file, struct jak_error *err, struct jak_json_array *array)
{
        fprintf(file, "[");
        for (size_t i = 0; i < array->elements.elements.num_elems; i++) {
                struct jak_json_element *element = vec_get(&array->elements.elements, i, struct jak_json_element);
                if (!json_ast_node_element_print(file, err, element)) {
                        return false;
                }
                fprintf(file, "%s", i + 1 < array->elements.elements.num_elems ? ", " : "");
        }
        fprintf(file, "]");
        return true;
}

static void json_ast_node_string_print(FILE *file, struct jak_json_string *string)
{
        fprintf(file, "\"%s\"", string->value);
}

static bool json_ast_node_number_print(FILE *file, struct jak_error *err, struct jak_json_number *number)
{
        switch (number->value_type) {
                case JSON_NUMBER_FLOAT:
                        fprintf(file, "%f", number->value.float_number);
                        break;
                case JSON_NUMBER_UNSIGNED:
                        fprintf(file, "%" PRIu64, number->value.unsigned_integer);
                        break;
                case JSON_NUMBER_SIGNED:
                        fprintf(file, "%" PRIi64, number->value.signed_integer);
                        break;
                default: error(err, JAK_ERR_NOJSONNUMBERT);
                        return false;
        }
        return true;
}

static bool json_ast_node_value_print(FILE *file, struct jak_error *err, struct jak_json_node_value *value)
{
        switch (value->value_type) {
                case JSON_VALUE_OBJECT:
                        if (!json_ast_node_object_print(file, err, value->value.object)) {
                                return false;
                        }
                        break;
                case JSON_VALUE_ARRAY:
                        if (!json_ast_node_array_print(file, err, value->value.array)) {
                                return false;
                        }
                        break;
                case JSON_VALUE_STRING:
                        json_ast_node_string_print(file, value->value.string);
                        break;
                case JSON_VALUE_NUMBER:
                        if (!json_ast_node_number_print(file, err, value->value.number)) {
                                return false;
                        }
                        break;
                case JSON_VALUE_TRUE:
                        fprintf(file, "true");
                        break;
                case JSON_VALUE_FALSE:
                        fprintf(file, "false");
                        break;
                case JSON_VALUE_NULL:
                        fprintf(file, "null");
                        break;
                default: error(err, JAK_ERR_NOTYPE);
                        return false;
        }
        return true;
}

static bool json_ast_node_element_print(FILE *file, struct jak_error *err, struct jak_json_element *element)
{
        return json_ast_node_value_print(file, err, &element->value);
}

static bool json_ast_node_value_drop(struct jak_json_node_value *value, struct jak_error *err);

static bool json_ast_node_element_drop(struct jak_json_element *element, struct jak_error *err)
{
        return json_ast_node_value_drop(&element->value, err);
}

static bool json_ast_node_member_drop(struct jak_json_prop *member, struct jak_error *err)
{
        free(member->key.value);
        return json_ast_node_element_drop(&member->value, err);
}

static bool json_ast_node_members_drop(struct jak_json_members *members, struct jak_error *err)
{
        for (size_t i = 0; i < members->members.num_elems; i++) {
                struct jak_json_prop *member = vec_get(&members->members, i, struct jak_json_prop);
                if (!json_ast_node_member_drop(member, err)) {
                        return false;
                }
        }
        vec_drop(&members->members);
        return true;
}

static bool json_ast_node_elements_drop(struct jak_json_elements *elements, struct jak_error *err)
{
        for (size_t i = 0; i < elements->elements.num_elems; i++) {
                struct jak_json_element *element = vec_get(&elements->elements, i, struct jak_json_element);
                if (!json_ast_node_element_drop(element, err)) {
                        return false;
                }
        }
        vec_drop(&elements->elements);
        return true;
}

static bool json_ast_node_object_drop(struct jak_json_object_t *object, struct jak_error *err)
{
        if (!json_ast_node_members_drop(object->value, err)) {
                return false;
        } else {
                free(object->value);
                return true;
        }
}

static bool json_ast_node_array_drop(struct jak_json_array *array, struct jak_error *err)
{
        return json_ast_node_elements_drop(&array->elements, err);
}

static void json_ast_node_string_drop(struct jak_json_string *string)
{
        free(string->value);
}

static void json_ast_node_number_drop(struct jak_json_number *number)
{
        JAK_UNUSED(number);
}

static bool json_ast_node_value_drop(struct jak_json_node_value *value, struct jak_error *err)
{
        switch (value->value_type) {
                case JSON_VALUE_OBJECT:
                        if (!json_ast_node_object_drop(value->value.object, err)) {
                                return false;
                        } else {
                                free(value->value.object);
                        }
                        break;
                case JSON_VALUE_ARRAY:
                        if (!json_ast_node_array_drop(value->value.array, err)) {
                                return false;
                        } else {
                                free(value->value.array);
                        }
                        break;
                case JSON_VALUE_STRING:
                        json_ast_node_string_drop(value->value.string);
                        free(value->value.string);
                        break;
                case JSON_VALUE_NUMBER:
                        json_ast_node_number_drop(value->value.number);
                        free(value->value.number);
                        break;
                case JSON_VALUE_TRUE:
                case JSON_VALUE_FALSE:
                case JSON_VALUE_NULL:
                        break;
                default: error(err, JAK_ERR_NOTYPE)
                        return false;

        }
        return true;
}

bool json_drop(struct jak_json *json)
{
        struct jak_json_element *element = json->element;
        if (!json_ast_node_value_drop(&element->value, &json->err)) {
                return false;
        } else {
                free(json->element);
                return true;
        }
}

bool json_print(FILE *file, struct jak_json *json)
{
        return json_ast_node_element_print(file, &json->err, json->element);
}

bool json_list_is_empty(const struct jak_json_elements *elements)
{
        return elements->elements.num_elems == 0;
}

bool json_list_length(jak_u32 *len, const struct jak_json_elements *elements)
{
        JAK_ERROR_IF_NULL(len)
        JAK_ERROR_IF_NULL(elements)
        *len = elements->elements.num_elems;
        return true;
}

enum json_list_type json_fitting_type(enum json_list_type current, enum json_list_type to_add)
{
        if (current == JSON_LIST_TYPE_VARIABLE_OR_NESTED || to_add == JSON_LIST_TYPE_VARIABLE_OR_NESTED) {
                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
        }
        if (current == JSON_LIST_TYPE_EMPTY || current == JSON_LIST_TYPE_FIXED_BOOLEAN ||
            to_add == JSON_LIST_TYPE_EMPTY || to_add == JSON_LIST_TYPE_FIXED_BOOLEAN) {
                if (current == to_add) {
                        return current;
                } else {
                        if ((to_add == JSON_LIST_TYPE_FIXED_BOOLEAN && current == JSON_LIST_TYPE_FIXED_NULL) ||
                            (to_add == JSON_LIST_TYPE_FIXED_NULL && current == JSON_LIST_TYPE_FIXED_BOOLEAN)) {
                                return JSON_LIST_TYPE_FIXED_BOOLEAN;
                        } else {
                                if (to_add == JSON_LIST_TYPE_EMPTY) {
                                        return current;
                                } else if (current == JSON_LIST_TYPE_EMPTY) {
                                        return to_add;
                                } else {

                                        return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        }
                }
        } else {
                switch (current) {
                        case JSON_LIST_TYPE_FIXED_NULL:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                        case JSON_LIST_TYPE_FIXED_FLOAT:
                                        case JSON_LIST_TYPE_FIXED_BOOLEAN:
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                        case JSON_LIST_TYPE_FIXED_U64:
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return to_add;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_FLOAT:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                        case JSON_LIST_TYPE_FIXED_FLOAT:
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                        case JSON_LIST_TYPE_FIXED_U64:
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_FIXED_FLOAT;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_U8:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_U8;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                                return JSON_LIST_TYPE_FIXED_U8;
                                        case JSON_LIST_TYPE_FIXED_U16:
                                                return JSON_LIST_TYPE_FIXED_U16;
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_U32;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_FIXED_U64;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                                return JSON_LIST_TYPE_FIXED_I16;
                                        case JSON_LIST_TYPE_FIXED_I16:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_U16:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_U16;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                                return JSON_LIST_TYPE_FIXED_U16;
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_U32;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_FIXED_U64;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I16:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_U32:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_U32;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_U32;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_FIXED_U64;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_U64:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_U64;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_FIXED_U64;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_I8:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_I8;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                                return JSON_LIST_TYPE_FIXED_I16;
                                        case JSON_LIST_TYPE_FIXED_U16:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                                return JSON_LIST_TYPE_FIXED_I8;
                                        case JSON_LIST_TYPE_FIXED_I16:
                                                return JSON_LIST_TYPE_FIXED_I16;
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_I16:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_I16;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                                return JSON_LIST_TYPE_FIXED_I16;
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_I32:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                                return JSON_LIST_TYPE_FIXED_I32;
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        case JSON_LIST_TYPE_FIXED_I64:
                                switch (to_add) {
                                        case JSON_LIST_TYPE_FIXED_NULL:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        case JSON_LIST_TYPE_FIXED_U8:
                                        case JSON_LIST_TYPE_FIXED_U16:
                                        case JSON_LIST_TYPE_FIXED_U32:
                                        case JSON_LIST_TYPE_FIXED_U64:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                        case JSON_LIST_TYPE_FIXED_I8:
                                        case JSON_LIST_TYPE_FIXED_I16:
                                        case JSON_LIST_TYPE_FIXED_I32:
                                        case JSON_LIST_TYPE_FIXED_I64:
                                                return JSON_LIST_TYPE_FIXED_I64;
                                        default:
                                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                }
                        default:
                                return JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                }
        }
}

static enum json_list_type number_type_to_list_type(enum number_min_type type)
{
        switch (type) {
                case NUMBER_U8:
                        return JSON_LIST_TYPE_FIXED_U8;
                case NUMBER_U16:
                        return JSON_LIST_TYPE_FIXED_U16;
                case NUMBER_U32:
                        return JSON_LIST_TYPE_FIXED_U32;
                case NUMBER_U64:
                        return JSON_LIST_TYPE_FIXED_U64;
                case NUMBER_I8:
                        return JSON_LIST_TYPE_FIXED_I8;
                case NUMBER_I16:
                        return JSON_LIST_TYPE_FIXED_I16;
                case NUMBER_I32:
                        return JSON_LIST_TYPE_FIXED_I32;
                case NUMBER_I64:
                        return JSON_LIST_TYPE_FIXED_I64;
                default: error_print(JAK_ERR_UNSUPPORTEDTYPE)
                        return JSON_LIST_TYPE_EMPTY;

        }
}

bool json_array_get_type(enum json_list_type *type, const struct jak_json_array *array)
{
        JAK_ERROR_IF_NULL(type)
        JAK_ERROR_IF_NULL(array)
        enum json_list_type list_type = JSON_LIST_TYPE_EMPTY;
        for (jak_u32 i = 0; i < array->elements.elements.num_elems; i++) {
                const struct jak_json_element *elem = vec_get(&array->elements.elements, i, struct jak_json_element);
                switch (elem->value.value_type) {
                        case JSON_VALUE_OBJECT:
                        case JSON_VALUE_ARRAY:
                        case JSON_VALUE_STRING:
                                list_type = JSON_LIST_TYPE_VARIABLE_OR_NESTED;
                                goto return_result;
                        case JSON_VALUE_NUMBER: {
                                enum json_list_type elem_type;
                                switch (elem->value.value.number->value_type) {
                                        case JSON_NUMBER_FLOAT:
                                                elem_type = JSON_LIST_TYPE_FIXED_FLOAT;
                                                break;
                                        case JSON_NUMBER_UNSIGNED:
                                                elem_type = number_type_to_list_type(number_min_type_unsigned(
                                                        elem->value.value.number->value.unsigned_integer));
                                                break;
                                        case JSON_NUMBER_SIGNED:
                                                elem_type = number_type_to_list_type(number_min_type_signed(
                                                        elem->value.value.number->value.signed_integer));
                                                break;
                                        default: error_print(JAK_ERR_UNSUPPORTEDTYPE);
                                                continue;
                                }

                                list_type = json_fitting_type(list_type, elem_type);
                                if (list_type == JSON_LIST_TYPE_VARIABLE_OR_NESTED) {
                                        goto return_result;
                                }
                                break;
                        }
                        case JSON_VALUE_TRUE:
                        case JSON_VALUE_FALSE:
                                list_type = json_fitting_type(list_type, JSON_LIST_TYPE_FIXED_BOOLEAN);
                                if (list_type == JSON_LIST_TYPE_VARIABLE_OR_NESTED) {
                                        goto return_result;
                                }
                                break;
                        case JSON_VALUE_NULL:
                                list_type = json_fitting_type(list_type, JSON_LIST_TYPE_FIXED_NULL);
                                if (list_type == JSON_LIST_TYPE_VARIABLE_OR_NESTED) {
                                        goto return_result;
                                }
                                break;
                        default: error_print(JAK_ERR_UNSUPPORTEDTYPE);
                                break;
                }
        }
        return_result:
        *type = list_type;
        return true;

}