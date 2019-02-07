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

#include <ctype.h>
#include <locale.h>
#include "carbon/carbon-json.h"
#include "carbon/carbon-doc.h"
#include "carbon/carbon-convert.h"

static struct {
    carbon_json_token_type_e token;
    const char *string;
} TOKEN_STRING[] = {
    {.token = CARBON_JSON_TOKEN_SCOPE_OPEN, .string = "CARBON_JSON_TOKEN_SCOPE_OPEN"},
    {.token = CARBON_JSON_TOKEN_SCOPE_CLOSE, .string = "CARBON_JSON_TOKEN_SCOPE_CLOSE"},
    {.token = CARBON_JSON_TOKEN_STRING_LITERAL, .string = "JSON_TOKEN_STRING"},
    {.token = CARBON_JSON_TOKEN_INT_NUMBER, .string = "CARBON_JSON_TOKEN_INT_NUMBER"},
    {.token = CARBON_JSON_TOKEN_REAL_NUMBER, .string = "CARBON_JSON_TOKEN_REAL_NUMBER"},
    {.token = CARBON_JSON_TOKEN_LITERAL_TRUE, .string = "CARBON_JSON_TOKEN_LITERAL_TRUE"},
    {.token = CARBON_JSON_TOKEN_LITERAL_FALSE, .string = "CARBON_JSON_TOKEN_LITERAL_FALSE"},
    {.token = CARBON_JSON_TOKEN_LITERAL_NULL, .string = "CARBON_JSON_TOKEN_LITERAL_NULL"},
    {.token = CARBON_JSON_TOKEN_COMMA, .string = "CARBON_JSON_TOKEN_COMMA"},
    {.token = CARBON_JSON_TOKEN_ASSIGNMENT, .string = "JSON_TOKEN_ASSIGMENT"},
    {.token = CARBON_JSON_TOKEN_ARRAY_BEGIN, .string = "CARBON_JSON_TOKEN_ARRAY_BEGIN"},
    {.token = CARBON_JSON_TOKEN_ARRAY_END, .string = "CARBON_JSON_TOKEN_ARRAY_END"},
    {.token = CARBON_JSON_TOKEN_UNKNOWN, .string = "CARBON_JSON_TOKEN_UNKNOWN"}
};

typedef struct
{
    carbon_json_token_type_e type;
    bool init;
} TokenMemory;

static int processToken(carbon_err_t *err, carbon_json_parse_err_t *errorDesc, const carbon_json_token_t *token, carbon_vec_t ofType(JsonTokenType) *brackets,
                        TokenMemory *tokenMemory);

static int setError(carbon_json_parse_err_t *errorDesc, const carbon_json_token_t *token, const char *msg);


CARBON_EXPORT(bool)
carbon_json_tokenizer_init(carbon_json_tokenizer_t *tokenizer, const char *input)
{
    CARBON_NON_NULL_OR_ERROR(tokenizer)
    CARBON_NON_NULL_OR_ERROR(input)
    tokenizer->cursor = input;
    tokenizer->token = (carbon_json_token_t) {
        .type = CARBON_JSON_TOKEN_UNKNOWN,
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
    if (CARBON_BRANCH_LIKELY(*tokenizer->cursor != '\0')) {
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
            } while (isspace(c = *tokenizer->cursor));
            return carbon_json_tokenizer_next(tokenizer);
        } else if (c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
            tokenizer->token.type = c == '{' ? CARBON_JSON_TOKEN_SCOPE_OPEN : c == '}' ? CARBON_JSON_TOKEN_SCOPE_CLOSE :
                                    c == '[' ? CARBON_JSON_TOKEN_ARRAY_BEGIN : c == ']' ? CARBON_JSON_TOKEN_ARRAY_END :
                                    c == ':' ? CARBON_JSON_TOKEN_ASSIGNMENT : CARBON_JSON_TOKEN_COMMA;
            tokenizer->token.column++;
            tokenizer->token.length = 1;
            tokenizer->cursor++;
        } else if(c == '"') {
            bool escapeQuote = false;
            tokenizer->token.type = CARBON_JSON_TOKEN_STRING_LITERAL;
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
                if (CARBON_BRANCH_UNLIKELY(c == '\\' && last_1_c == '\\')) {
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
                tokenizer->token.type = CARBON_JSON_TOKEN_LITERAL_TRUE;
                tokenizer->token.length = lenTrueNull;
            } else if (cursorLen > lenFalse && strncmp(tokenizer->cursor, "false", lenFalse) == 0) {
                tokenizer->token.type = CARBON_JSON_TOKEN_LITERAL_FALSE;
                tokenizer->token.length = lenFalse;
            } else if (cursorLen > lenTrueNull && strncmp(tokenizer->cursor, "null", lenTrueNull) == 0) {
                tokenizer->token.type = CARBON_JSON_TOKEN_LITERAL_NULL;
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
            tokenizer->token.type = fracFound ? CARBON_JSON_TOKEN_REAL_NUMBER : CARBON_JSON_TOKEN_INT_NUMBER;
        } else {
caseTokenUnknown:
            tokenizer->token.type = CARBON_JSON_TOKEN_UNKNOWN;
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

static bool parseObject(carbon_json_ast_node_object_t *object, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static bool parseArray(carbon_json_ast_node_array_t *array, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static void parseString(carbon_json_ast_node_string_t *string, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static void parseNumber(carbon_json_ast_node_number_t *number, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static bool parseElement(carbon_json_ast_node_element_t *element, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static bool parseElements(carbon_json_ast_node_elements_t *elements, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx);
static bool parseTokenStream(carbon_json_t *json, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream);
static carbon_json_token_t getToken(carbon_vec_t ofType(JsonToken) *tokenStream, size_t tokenIdx);
static void connectChildAndParentsMember(carbon_json_ast_node_member_t *member);
static void connectChildAndParentsObject(carbon_json_ast_node_object_t *object);
static void connectChildAndParentsArray(carbon_json_ast_node_array_t *array);
static void connectChildAndParentsValue(carbon_json_ast_node_value_t *value);
static void connectChildAndParentsElement(carbon_json_ast_node_element_t *element);
static void connectChildAndParents(carbon_json_t *json);
static bool jsonAstNodeMemberPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_member_t *member);
static bool jsonAstNodeObjectPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_object_t *object);
static bool jsonAstNodeArrayPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_array_t *array);
static void jsonAstNodeStringPrint(FILE *file, carbon_json_ast_node_string_t *string);
static bool jsonAstNodeNumberPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_number_t *number);
static bool jsonAstNodeValuePrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_value_t *value);
static bool jsonAstNodeElementPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_element_t *element);

#define NEXT_TOKEN(x) { *x = *x + 1; }
#define PREV_TOKEN(x) { *x = *x - 1; }

CARBON_EXPORT(bool)
carbon_json_parser_create(JsonParser *parser, carbon_doc_bulk_t *partition)
{
    CARBON_NON_NULL_OR_ERROR(parser)
    CARBON_NON_NULL_OR_ERROR(partition)

    parser->partition = partition;
    carbon_error_init(&parser->err);

    return true;
}

bool carbon_json_parse(CARBON_NULLABLE carbon_json_t *json, CARBON_NULLABLE carbon_json_parse_err_t *error_desc,
                       JsonParser *parser, const char *input)
{
    CARBON_NON_NULL_OR_ERROR(parser)
    CARBON_NON_NULL_OR_ERROR(input)

    carbon_vec_t ofType(JsonTokenType) brackets;
    carbon_vec_t ofType(JsonToken) tokenStream;

    carbon_json_t retval = {
        .element = malloc(sizeof(carbon_json_ast_node_element_t))
    };
    carbon_error_init(&retval.err);
    const carbon_json_token_t *token;
    int status;

    carbon_json_tokenizer_init(&parser->tokenizer, input);
    carbon_vec_create(&brackets, NULL, sizeof(carbon_json_token_type_e), 15);
    carbon_vec_create(&tokenStream, NULL, sizeof(carbon_json_token_t), 200);

    TokenMemory tokenMemory = {
        .init = true,
        .type = CARBON_JSON_TOKEN_UNKNOWN
    };

    while ((token = carbon_json_tokenizer_next(&parser->tokenizer))) {
        if (CARBON_BRANCH_LIKELY((status = processToken(&parser->err, error_desc, token, &brackets, &tokenMemory)) == true)) {
            carbon_json_token_t *newToken = VECTOR_NEW_AND_GET(&tokenStream, carbon_json_token_t);
            carbon_json_token_dup(newToken, token);
        } else {
            goto cleanup;
        }
    }
    if (!carbon_vec_is_empty(&brackets)) {
        carbon_json_token_type_e type = *VECTOR_PEEK(&brackets, carbon_json_token_type_e);
        char buffer[1024];
        sprintf(&buffer[0], "Unexpected end of file: missing '%s' to match unclosed '%s' (if any)",
                type == CARBON_JSON_TOKEN_SCOPE_OPEN ? "}" : "]", type == CARBON_JSON_TOKEN_SCOPE_OPEN ? "{" : "[");
        status = setError(error_desc, token, &buffer[0]);
        goto cleanup;
    }

    if (!parseTokenStream(&retval, &parser->err, &tokenStream)) {
        return false;
    }

    CARBON_OPTIONAL_SET_OR_ELSE(json, retval, carbon_json_drop(json));
    status = true;

    cleanup:
    VectorDrop(&brackets);
    VectorDrop(&tokenStream);
    return status;
}

bool testConditionValue(carbon_err_t *err, carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        for (size_t i = 0; i < value->value.object->value->members.numElems; i++) {
            carbon_json_ast_node_member_t *member = VECTOR_GET(&value->value.object->value->members, i, carbon_json_ast_node_member_t);
            if (!testConditionValue(err, &member->value.value)) {
                return false;
            }
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY: {
        carbon_json_ast_node_elements_t *elements = &value->value.array->elements;
        carbon_json_ast_node_value_type_e valueType = CARBON_JSON_AST_NODE_VALUE_TYPE_NULL;

        for (size_t i = 0; i < elements->elements.numElems; i++) {
            carbon_json_ast_node_element_t *element = VECTOR_GET(&elements->elements, i, carbon_json_ast_node_element_t);
            valueType = ((i == 0 || valueType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL) ? element->value.value_type : valueType);

            /** Test "All elements in array of same type" condition */
            if ((element->value.value_type != CARBON_JSON_AST_NODE_VALUE_TYPE_NULL) &&
                (valueType == CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE && (element->value.value_type != CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE || element->value.value_type != CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE)) &&
                (valueType == CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE && (element->value.value_type != CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE || element->value.value_type != CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE)) &&
                ((valueType != CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE && valueType != CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE) && valueType != element->value.value_type))
            {
                char message[] = "JSON file constraint broken: arrays of mixed types detected";
                char *result = malloc(strlen(message) + 1);
                strcpy(result, &message[0]);
                CARBON_ERROR_WDETAILS(err, CARBON_ERR_ARRAYOFMIXEDTYPES, result);
                free(result);
                return false;
            }

            switch (element->value.value_type) {
            case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT: {
                carbon_json_ast_node_object_t *object = element->value.value.object;
                for (size_t i = 0; i < object->value->members.numElems; i++) {
                    carbon_json_ast_node_member_t *member = VECTOR_GET(&object->value->members, i, carbon_json_ast_node_member_t);
                    if (!testConditionValue(err, &member->value.value)) {
                        return false;
                    }
                }
            } break;
            case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY: {/** Test "No Array of Arrays" condition */
                char message[] = "JSON file constraint broken: arrays of arrays detected";
                char *result = malloc(strlen(message) + 1);
                strcpy(result, &message[0]);
                CARBON_ERROR_WDETAILS(err, CARBON_ERR_ARRAYOFARRAYS, result);
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

CARBON_EXPORT(bool)
carbon_jest_test_doc(carbon_err_t *err, carbon_json_t *json)
{
    return (testConditionValue(err, &json->element->value));
}

static carbon_json_token_t getToken(carbon_vec_t ofType(JsonToken) *tokenStream, size_t tokenIdx)
{
    return *(carbon_json_token_t *) VectorAt(tokenStream, tokenIdx);
}

bool parseMembers(carbon_err_t *err, carbon_json_ast_node_members_t *members, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_vec_create(&members->members, NULL, sizeof(carbon_json_ast_node_member_t), 20);
    carbon_json_token_t delimiterToken;

    do {
        carbon_json_ast_node_member_t *member = VECTOR_NEW_AND_GET(&members->members, carbon_json_ast_node_member_t);
        carbon_json_token_t keyNameToken = getToken(tokenStream, *tokenIdx);

        member->key.value = malloc(keyNameToken.length + 1);
        strncpy(member->key.value, keyNameToken.string, keyNameToken.length);
        member->key.value[keyNameToken.length] = '\0';

        NEXT_TOKEN(tokenIdx); /** skip assignment token */
        NEXT_TOKEN(tokenIdx);
        carbon_json_token_t valueToken = getToken(tokenStream, *tokenIdx);

        switch (valueToken.type) {
        case CARBON_JSON_TOKEN_SCOPE_OPEN:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT;
            member->value.value.value.object = malloc(sizeof(carbon_json_ast_node_object_t));
            if(!parseObject(member->value.value.value.object, err, tokenStream, tokenIdx)) {
                return false;
            }
            break;
        case CARBON_JSON_TOKEN_ARRAY_BEGIN:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY;
            member->value.value.value.array = malloc(sizeof(carbon_json_ast_node_array_t));
            if(!parseArray(member->value.value.value.array, err, tokenStream, tokenIdx)) {
                return false;
            }
            break;
        case CARBON_JSON_TOKEN_STRING_LITERAL:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_STRING;
            member->value.value.value.string = malloc(sizeof(carbon_json_ast_node_string_t));
            parseString(member->value.value.value.string, tokenStream, tokenIdx);
            break;
        case CARBON_JSON_TOKEN_INT_NUMBER:
        case CARBON_JSON_TOKEN_REAL_NUMBER:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER;
            member->value.value.value.number = malloc(sizeof(carbon_json_ast_node_number_t));
            parseNumber(member->value.value.value.number, tokenStream, tokenIdx);
            break;
        case CARBON_JSON_TOKEN_LITERAL_TRUE:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE;
            NEXT_TOKEN(tokenIdx);
            break;
        case CARBON_JSON_TOKEN_LITERAL_FALSE:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE;
            NEXT_TOKEN(tokenIdx);
            break;
        case CARBON_JSON_TOKEN_LITERAL_NULL:
            member->value.value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_NULL;
            NEXT_TOKEN(tokenIdx);
            break;
        default:
            CARBON_ERROR(err, CARBON_ERR_PARSETYPE)
            return false;
        }

        delimiterToken = getToken(tokenStream, *tokenIdx);
        NEXT_TOKEN(tokenIdx);
    } while (delimiterToken.type == CARBON_JSON_TOKEN_COMMA);
    PREV_TOKEN(tokenIdx);
    return true;
}

static bool parseObject(carbon_json_ast_node_object_t *object, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    assert(getToken(tokenStream, *tokenIdx).type == CARBON_JSON_TOKEN_SCOPE_OPEN);
    NEXT_TOKEN(tokenIdx);  /** Skip '{' */
    object->value = malloc(sizeof(carbon_json_ast_node_members_t));

    /** test whether this is an empty object */
    carbon_json_token_t token = getToken(tokenStream, *tokenIdx);

    if (token.type != CARBON_JSON_TOKEN_SCOPE_CLOSE) {
        if(!parseMembers(err, object->value, tokenStream, tokenIdx)) {
            return false;
        }
    } else {
        carbon_vec_create(&object->value->members, NULL, sizeof(carbon_json_ast_node_member_t), 20);
    }

    NEXT_TOKEN(tokenIdx);  /** Skip '}' */
    return true;
}

static bool parseArray(carbon_json_ast_node_array_t *array, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_json_token_t token = getToken(tokenStream, *tokenIdx);
    CARBON_UNUSED(token);
    assert(token.type == CARBON_JSON_TOKEN_ARRAY_BEGIN);
    NEXT_TOKEN(tokenIdx); /** Skip '[' */

    carbon_vec_create(&array->elements.elements, NULL, sizeof(carbon_json_ast_node_element_t), 250);
    if (!parseElements(&array->elements, err, tokenStream, tokenIdx)) {
        return false;
    }

    NEXT_TOKEN(tokenIdx); /** Skip ']' */
    return true;
}

static void parseString(carbon_json_ast_node_string_t *string, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_json_token_t token = getToken(tokenStream, *tokenIdx);
    assert(token.type == CARBON_JSON_TOKEN_STRING_LITERAL);

    string->value = malloc(token.length + 1);
    if (CARBON_BRANCH_LIKELY(token.length > 0)) {
        strncpy(string->value, token.string, token.length);
    }
    string->value[token.length] = '\0';
    NEXT_TOKEN(tokenIdx);
}

static void parseNumber(carbon_json_ast_node_number_t *number, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_json_token_t token = getToken(tokenStream, *tokenIdx);
    assert(token.type == CARBON_JSON_TOKEN_REAL_NUMBER || token.type == CARBON_JSON_TOKEN_INT_NUMBER);

    char *value = malloc(token.length + 1);
    strncpy(value, token.string, token.length);
    value[token.length] = '\0';

    if (token.type == CARBON_JSON_TOKEN_INT_NUMBER) {
        int64_t assumeSigned = carbon_convert_atoi64(value);
        if (value[0] == '-') {
            number->value_type = CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER;
            number->value.signed_integer = assumeSigned;
        } else {
            uint64_t assumeUnsigned = carbon_convert_atoiu64(value);
            if (assumeUnsigned > (uint64_t) assumeSigned) {
                number->value_type = CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER;
                number->value.unsigned_integer = assumeUnsigned;
            } else {
                number->value_type = CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER;
                number->value.signed_integer = assumeSigned;
            }
        }
    } else {
        number->value_type = CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER;
        setlocale( LC_ALL | ~LC_NUMERIC, "");
        number->value.float_number = strtof(value, NULL);
    }

    free(value);
    NEXT_TOKEN(tokenIdx);
}

static bool parseElement(carbon_json_ast_node_element_t *element, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_json_token_t token = getToken(tokenStream, *tokenIdx);

    if (token.type == CARBON_JSON_TOKEN_SCOPE_OPEN) { /** Parse object */
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT;
        element->value.value.object = malloc(sizeof(carbon_json_ast_node_object_t));
        if (!parseObject(element->value.value.object, err, tokenStream, tokenIdx)) {
            return false;
        }
    } else if (token.type == CARBON_JSON_TOKEN_ARRAY_BEGIN) { /** Parse array */
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY;
        element->value.value.array = malloc(sizeof(carbon_json_ast_node_array_t));
        if (!parseArray(element->value.value.array, err, tokenStream, tokenIdx)) {
            return false;
        }
    } else if (token.type == CARBON_JSON_TOKEN_STRING_LITERAL) { /** Parse string */
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_STRING;
        element->value.value.string = malloc(sizeof(carbon_json_ast_node_string_t));
        parseString(element->value.value.string, tokenStream, tokenIdx);
    } else if (token.type == CARBON_JSON_TOKEN_REAL_NUMBER || token.type == CARBON_JSON_TOKEN_INT_NUMBER) { /** Parse number */
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER;
        element->value.value.number = malloc(sizeof(carbon_json_ast_node_number_t));
        parseNumber(element->value.value.number, tokenStream, tokenIdx);
    } else if (token.type == CARBON_JSON_TOKEN_LITERAL_TRUE) {
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE;
        NEXT_TOKEN(tokenIdx);
    } else if (token.type == CARBON_JSON_TOKEN_LITERAL_FALSE) {
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE;
        NEXT_TOKEN(tokenIdx);
    } else if (token.type == CARBON_JSON_TOKEN_LITERAL_NULL) {
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_NULL;
        NEXT_TOKEN(tokenIdx);
    } else {
        element->value.value_type = CARBON_JSON_AST_NODE_VALUE_TYPE_NULL;
    }
    return true;
}

static bool parseElements(carbon_json_ast_node_elements_t *elements, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream, size_t *tokenIdx)
{
    carbon_json_token_t delimiter;
    do {
        if (!parseElement(VECTOR_NEW_AND_GET(&elements->elements, carbon_json_ast_node_element_t), err,
                     tokenStream, tokenIdx)) {
            return false;
        }
        delimiter = getToken(tokenStream, *tokenIdx);
        NEXT_TOKEN(tokenIdx);
    } while (delimiter.type == CARBON_JSON_TOKEN_COMMA);
    PREV_TOKEN(tokenIdx);
    return true;
}

static bool parseTokenStream(carbon_json_t *json, carbon_err_t *err, carbon_vec_t ofType(JsonToken) *tokenStream)
{
    size_t tokenIdx = 0;
    if (!parseElement(json->element, err, tokenStream, &tokenIdx)) {
        return false;
    }
    connectChildAndParents(json);
    return true;
}


static void connectChildAndParentsMember(carbon_json_ast_node_member_t *member)
{
    connectChildAndParentsElement(&member->value);
}

static void connectChildAndParentsObject(carbon_json_ast_node_object_t *object)
{
    object->value->parent = object;
    for (size_t i = 0; i < object->value->members.numElems; i++) {
        carbon_json_ast_node_member_t *member = VECTOR_GET(&object->value->members, i, carbon_json_ast_node_member_t);
        member->parent = object->value;

        member->key.parent = member;

        member->value.parent_type = CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_MEMBER;
        member->value.parent.member = member;

        connectChildAndParentsMember(member);
    }
}

static void connectChildAndParentsArray(carbon_json_ast_node_array_t *array)
{
    array->elements.parent = array;
    for (size_t i = 0; i < array->elements.elements.numElems; i++) {
        carbon_json_ast_node_element_t *element = VECTOR_GET(&array->elements.elements, i, carbon_json_ast_node_element_t);
        element->parent_type = CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_ELEMENTS;
        element->parent.elements = &array->elements;
        connectChildAndParentsElement(element);
    }
}

static void connectChildAndParentsValue(carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        connectChildAndParentsObject(value->value.object);
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        connectChildAndParentsArray(value->value.array);
        break;
    default:
        break;
    }
}

static void connectChildAndParentsElement(carbon_json_ast_node_element_t *element)
{
    element->value.parent = element;
    connectChildAndParentsValue(&element->value);
}

static void connectChildAndParents(carbon_json_t *json)
{
    json->element->parent_type = CARBON_JSON_AST_NODE_ELEMENT_PARENT_TYPE_JSON;
    json->element->parent.json = json;
    connectChildAndParentsElement(json->element);
}

static bool isValue(carbon_json_token_type_e token) {
    return (token == CARBON_JSON_TOKEN_STRING_LITERAL || token == CARBON_JSON_TOKEN_REAL_NUMBER || token == CARBON_JSON_TOKEN_INT_NUMBER ||
        token == CARBON_JSON_TOKEN_LITERAL_TRUE || token == CARBON_JSON_TOKEN_LITERAL_FALSE || token == CARBON_JSON_TOKEN_LITERAL_NULL );
}

static int processToken(carbon_err_t *err, carbon_json_parse_err_t *errorDesc, const carbon_json_token_t *token, carbon_vec_t ofType(JsonTokenType) *brackets,
                        TokenMemory *tokenMemory)
{
    switch (token->type) {
    case CARBON_JSON_TOKEN_SCOPE_OPEN:
    case CARBON_JSON_TOKEN_ARRAY_BEGIN:
        carbon_vec_push(brackets, &token->type, 1);
        break;
    case CARBON_JSON_TOKEN_SCOPE_CLOSE:
    case CARBON_JSON_TOKEN_ARRAY_END: {
        if (!carbon_vec_is_empty(brackets)) {
            carbon_json_token_type_e bracket = *VECTOR_PEEK(brackets, carbon_json_token_type_e);
            if ((token->type == CARBON_JSON_TOKEN_ARRAY_END && bracket == CARBON_JSON_TOKEN_ARRAY_BEGIN) ||
                (token->type == CARBON_JSON_TOKEN_SCOPE_CLOSE && bracket == CARBON_JSON_TOKEN_SCOPE_OPEN)) {
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

    switch (tokenMemory->type) {
    case CARBON_JSON_TOKEN_SCOPE_OPEN:

        switch (token->type) {
        case CARBON_JSON_TOKEN_STRING_LITERAL:
        case CARBON_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return setError(errorDesc, token, "Expected key name or '}'");
        }
        break;
    case CARBON_JSON_TOKEN_STRING_LITERAL:
        switch (token->type) {
        case CARBON_JSON_TOKEN_ASSIGNMENT:
        case CARBON_JSON_TOKEN_COMMA:
        case CARBON_JSON_TOKEN_ARRAY_END:
        case CARBON_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return setError(errorDesc, token, "Expected key name (missing ':'), enumeration (','), "
                "end of enumeration (']'), or end of object ('}')");
        }
        break;
    case CARBON_JSON_TOKEN_SCOPE_CLOSE:
    case CARBON_JSON_TOKEN_INT_NUMBER:
    case CARBON_JSON_TOKEN_REAL_NUMBER:
    case CARBON_JSON_TOKEN_LITERAL_TRUE:
    case CARBON_JSON_TOKEN_LITERAL_FALSE:
    case CARBON_JSON_TOKEN_LITERAL_NULL:
        switch (token->type) {
        case CARBON_JSON_TOKEN_COMMA:
        case CARBON_JSON_TOKEN_ARRAY_END:
        case CARBON_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return setError(errorDesc, token, "Expected enumeration (','), end of enumeration (']'), "
                "or end of object ('})");
        }
        break;
    case CARBON_JSON_TOKEN_ASSIGNMENT:
    case CARBON_JSON_TOKEN_COMMA:
        switch (token->type) {
        case CARBON_JSON_TOKEN_STRING_LITERAL:
        case CARBON_JSON_TOKEN_REAL_NUMBER:
        case CARBON_JSON_TOKEN_INT_NUMBER:
        case CARBON_JSON_TOKEN_SCOPE_OPEN:
        case CARBON_JSON_TOKEN_ARRAY_BEGIN:
        case CARBON_JSON_TOKEN_LITERAL_TRUE:
        case CARBON_JSON_TOKEN_LITERAL_FALSE:
        case CARBON_JSON_TOKEN_LITERAL_NULL:
            break;
        default:
            return setError(errorDesc, token, "Expected key name, or value (string, number, object, enumeration, true, "
                "false, or null).");
        }
        break;
    case CARBON_JSON_TOKEN_ARRAY_BEGIN:
        switch (token->type) {
        case CARBON_JSON_TOKEN_ARRAY_END:
        case CARBON_JSON_TOKEN_STRING_LITERAL:
        case CARBON_JSON_TOKEN_REAL_NUMBER:
        case CARBON_JSON_TOKEN_INT_NUMBER:
        case CARBON_JSON_TOKEN_SCOPE_OPEN:
        case CARBON_JSON_TOKEN_ARRAY_BEGIN:
        case CARBON_JSON_TOKEN_LITERAL_TRUE:
        case CARBON_JSON_TOKEN_LITERAL_FALSE:
        case CARBON_JSON_TOKEN_LITERAL_NULL:
            break;
        default:
            return setError(errorDesc, token, "End of enumeration (']'), enumeration (','), or "
                "end of enumeration (']')");
        }
        break;
    case CARBON_JSON_TOKEN_ARRAY_END:
        switch (token->type) {
        case CARBON_JSON_TOKEN_COMMA:
        case CARBON_JSON_TOKEN_ARRAY_END:
        case CARBON_JSON_TOKEN_SCOPE_CLOSE:
            break;
        default:
            return setError(errorDesc, token, "End of enumeration (']'), enumeration (','), or "
                "end of object ('}')");
        }
        break;
    case CARBON_JSON_TOKEN_UNKNOWN:
        if (tokenMemory->init) {
            if (token->type != CARBON_JSON_TOKEN_SCOPE_OPEN && token->type != CARBON_JSON_TOKEN_ARRAY_BEGIN && !isValue(token->type)) {
                return setError(errorDesc, token, "Expected JSON document: missing '{' or '['");
            }
            tokenMemory->init = false;
        } else {
            return setError(errorDesc, token, "Unexpected token");
        }
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOJSONTOKEN)
        return false;
    }

    tokenMemory->type = token->type;
    return true;
}

static int setError(carbon_json_parse_err_t *errorDesc, const carbon_json_token_t *token, const char *msg)
{
    if (errorDesc) {
        errorDesc->token = token;
        errorDesc->token_type_str = token ? TOKEN_STRING[token->type].string : "(no token)";
        errorDesc->msg = msg;
    }
    return false;
}

static bool jsonAstNodeMemberPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_member_t *member)
{
    fprintf(file, "\"%s\": ", member->key.value);
    return jsonAstNodeValuePrint(file, err, &member->value.value);
}

static bool jsonAstNodeObjectPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_object_t *object)
{
    fprintf(file, "{");
    for (size_t i = 0; i < object->value->members.numElems; i++) {
        carbon_json_ast_node_member_t *member = VECTOR_GET(&object->value->members, i, carbon_json_ast_node_member_t);
        if (!jsonAstNodeMemberPrint(file, err, member)) {
            return false;
        }
        fprintf(file, "%s", i + 1 < object->value->members.numElems ? ", " : "");
    }
    fprintf(file, "}");
    return true;
}

static bool jsonAstNodeArrayPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_array_t *array)
{
    fprintf(file, "[");
    for (size_t i = 0; i < array->elements.elements.numElems; i++) {
        carbon_json_ast_node_element_t *element = VECTOR_GET(&array->elements.elements, i, carbon_json_ast_node_element_t);
        if (!jsonAstNodeElementPrint(file, err, element)) {
            return false;
        }
        fprintf(file, "%s", i + 1 < array->elements.elements.numElems ? ", " : "");
    }
    fprintf(file, "]");
    return true;
}

static void jsonAstNodeStringPrint(FILE *file, carbon_json_ast_node_string_t *string)
{
    fprintf(file, "\"%s\"", string->value);
}

static bool jsonAstNodeNumberPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_number_t *number)
{
    switch (number->value_type) {
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER:
        fprintf(file, "%f", number->value.float_number);
        break;
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER:
        fprintf(file, "%" PRIu64, number->value.unsigned_integer);
        break;
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER:
        fprintf(file, "%" PRIi64, number->value.signed_integer);
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOJSONNUMBERT);
        return false;
    }
    return true;
}

static bool jsonAstNodeValuePrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_value_t *value)
{
    switch (value->value_type) {
    case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        if (!jsonAstNodeObjectPrint(file, err, value->value.object)) {
            return false;
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        if (!jsonAstNodeArrayPrint(file, err, value->value.array)) {
            return false;
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
        jsonAstNodeStringPrint(file, value->value.string);
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER:
        if (!jsonAstNodeNumberPrint(file, err, value->value.number)) {
            return false;
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
        fprintf(file, "true");
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE:
        fprintf(file, "false");
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
        fprintf(file, "null");
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE);
        return false;
    }
    return true;
}

static bool jsonAstNodeElementPrint(FILE *file, carbon_err_t *err, carbon_json_ast_node_element_t *element)
{
    return jsonAstNodeValuePrint(file, err, &element->value);
}

static bool jsonAstNodeValueDrop(carbon_json_ast_node_value_t *value, carbon_err_t *err);

static bool jsonAstNodeElementDrop(carbon_json_ast_node_element_t *element, carbon_err_t *err)
{
    return jsonAstNodeValueDrop(&element->value, err);
}

static bool jsonAstNodeMemberDrop(carbon_json_ast_node_member_t *member, carbon_err_t *err)
{
    free(member->key.value);
    return jsonAstNodeElementDrop(&member->value, err);
}

static bool jsonAstNodeMembersDrop(carbon_json_ast_node_members_t *members, carbon_err_t *err)
{
    for (size_t i = 0; i < members->members.numElems; i++) {
        carbon_json_ast_node_member_t *member = VECTOR_GET(&members->members, i, carbon_json_ast_node_member_t);
        if (!jsonAstNodeMemberDrop(member, err)) {
            return false;
        }
    }
    VectorDrop(&members->members);
    return true;
}

static bool jsonAstNodeElementsDrop(carbon_json_ast_node_elements_t *elements, carbon_err_t *err)
{
    for (size_t i = 0; i < elements->elements.numElems; i++) {
        carbon_json_ast_node_element_t *element = VECTOR_GET(&elements->elements, i, carbon_json_ast_node_element_t);
        if (!jsonAstNodeElementDrop(element, err)) {
            return false;
        }
    }
    VectorDrop(&elements->elements);
    return true;
}

static bool jsonAstNodeObjectDrop(carbon_json_ast_node_object_t *object, carbon_err_t *err)
{
    if (!jsonAstNodeMembersDrop(object->value, err)) {
        return false;
    } else {
        free(object->value);
        return true;
    }
}

static bool jsonAstNodeArrayDrop(carbon_json_ast_node_array_t *array, carbon_err_t *err)
{
    return jsonAstNodeElementsDrop(&array->elements, err);
}

static void jsonAstNodeStringDrop(carbon_json_ast_node_string_t *string)
{
    free(string->value);
}

static void jsonAstNodeNumberDrop(carbon_json_ast_node_number_t *number)
{
    CARBON_UNUSED(number);
}

static bool jsonAstNodeValueDrop(carbon_json_ast_node_value_t *value, carbon_err_t *err)
{
    switch (value->value_type) {
    case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        if (!jsonAstNodeObjectDrop(value->value.object, err)) {
            return false;
        } else {
            free(value->value.object);
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
        if (!jsonAstNodeArrayDrop(value->value.array, err)) {
            return false;
        } else {
            free(value->value.array);
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
        jsonAstNodeStringDrop(value->value.string);
        free(value->value.string);
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER:
        jsonAstNodeNumberDrop(value->value.number);
        free(value->value.number);
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
        break;
    default:
        CARBON_ERROR(err, CARBON_ERR_NOTYPE)
        return false;

    }
    return true;
}

bool carbon_json_drop(carbon_json_t *json)
{
    carbon_json_ast_node_element_t *element = json->element;
    if (!jsonAstNodeValueDrop(&element->value, &json->err)) {
        return false;
    } else {
        free(json->element);
        return true;
    }
}

bool carbon_json_print(FILE *file, carbon_json_t *json)
{
    return jsonAstNodeElementPrint(file, &json->err, json->element);
}