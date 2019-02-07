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

#include "carbon/carbon-doc.h"
#include "carbon/carbon-columndoc.h"
#include "carbon/carbon-json.h"
#include "carbon/carbon-sort.h"

char VALUE_NULL = '\0';

static void createObjectModel(carbon_doc_obj_t *model, carbon_doc_t *doc);

static void createTypedVector(carbon_doc_entries_t *entry);

static void entryModelDrop(carbon_doc_entries_t *entry);

static bool printValue(FILE *file, carbon_field_type_e type, const carbon_vec_t ofType(<T>) *values);

static void printObject(FILE *file, const carbon_doc_obj_t *model);

static bool importJsonObject(carbon_doc_obj_t *target, carbon_err_t *err, const carbon_json_ast_node_object_t *jsonObject);

static void sortMetaModelEntries(carbon_columndoc_obj_t *metaModel);

CARBON_EXPORT(bool)
carbon_doc_bulk_create(carbon_doc_bulk_t *bulk, carbon_strdic_t *dic)
{
    CARBON_NON_NULL_OR_ERROR(bulk)
    CARBON_NON_NULL_OR_ERROR(dic)
    bulk->dic = dic;
    VectorCreate(&bulk->keys, NULL, sizeof(char *), 500);
    VectorCreate(&bulk->values, NULL, sizeof(char *), 1000);
    VectorCreate(&bulk->models, NULL, sizeof(carbon_doc_t), 50);
    return true;
}

carbon_doc_obj_t *carbon_doc_bulk_new_obj(carbon_doc_t *model)
{
    if (!model) {
        return NULL;
    } else {
        carbon_doc_obj_t *retval = VECTOR_NEW_AND_GET(&model->obj_model, carbon_doc_obj_t);
        createObjectModel(retval, model);
        return retval;
    }
}

CARBON_EXPORT(bool)
carbon_doc_bulk_get_dic_conetnts(carbon_vec_t ofType (const char *) **strings,
                                 carbon_vec_t ofType(carbon_string_id_t) **carbon_string_id_ts,
                                 const carbon_doc_bulk_t *context)
{
    CARBON_NON_NULL_OR_ERROR(context)

    size_t numDistinctValues;
    carbon_strdic_num_distinct(&numDistinctValues, context->dic);
    carbon_vec_t ofType (const char *) *resultStrings = malloc(sizeof(carbon_vec_t));
    carbon_vec_t ofType (carbon_string_id_t) *resultcarbon_string_id_ts = malloc(sizeof(carbon_vec_t));
    VectorCreate(resultStrings, NULL, sizeof(const char *), numDistinctValues);
    VectorCreate(resultcarbon_string_id_ts, NULL, sizeof(carbon_string_id_t), numDistinctValues);

    int status = carbon_strdic_get_contents(resultStrings, resultcarbon_string_id_ts, context->dic);
    CARBON_CHECK_SUCCESS(status);
    *strings = resultStrings;
    *carbon_string_id_ts = resultcarbon_string_id_ts;

    return status;
}

carbon_doc_t *carbon_doc_bulk_new_doc(carbon_doc_bulk_t *context, carbon_field_type_e type)
{
    if (!context) {
        return NULL;
    }

    carbon_doc_t template, *model;
    size_t idx = VectorLength(&context->models);
    VectorPush(&context->models, &template, 1);
    model = VECTOR_GET(&context->models, idx, carbon_doc_t);
    model->context = context;
    model->type = type;

    VectorCreate(&model->obj_model, NULL, sizeof(carbon_doc_obj_t), 10);

    return model;
}

CARBON_EXPORT(bool)
carbon_doc_bulk_Drop(carbon_doc_bulk_t *bulk)
{
    CARBON_NON_NULL_OR_ERROR(bulk)
    for (size_t i = 0; i < bulk->keys.numElems; i++) {
        char *string = *VECTOR_GET(&bulk->keys, i, char *);
        free (string);
    }
    for (size_t i = 0; i < bulk->values.numElems; i++) {
        char *string = *VECTOR_GET(&bulk->values, i, char *);
        free (string);
    }
    for (size_t i = 0; i < bulk->models.numElems; i++) {
        carbon_doc_t *model = VECTOR_GET(&bulk->models, i, carbon_doc_t);
        for (size_t j = 0; j < model->obj_model.numElems; j++) {
            carbon_doc_obj_t *objectModel = VECTOR_GET(&model->obj_model, j, carbon_doc_obj_t);
            carbon_doc_drop(objectModel);
        }
        VectorDrop(&model->obj_model);
    }

    VectorDrop(&bulk->keys);
    VectorDrop(&bulk->values);
    VectorDrop(&bulk->models);
    return true;
}

CARBON_EXPORT(bool)
carbon_doc_bulk_shrink(carbon_doc_bulk_t *bulk)
{
    CARBON_NON_NULL_OR_ERROR(bulk)
    VectorShrink(&bulk->keys);
    VectorShrink(&bulk->values);
    return true;
}

CARBON_EXPORT(bool)
carbon_doc_bulk_print(FILE *file, carbon_doc_bulk_t *bulk)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(bulk)

    fprintf(file, "{");
    char **keyStrings = VECTOR_ALL(&bulk->keys, char *);
    fprintf(file, "\"Key Strings\": [");
    for (size_t i = 0; i < bulk->keys.numElems; i++) {
        fprintf(file, "\"%s\"%s", keyStrings[i], i + 1 < bulk->keys.numElems ? ", " : "");
    }
    fprintf(file, "], ");

    char **valueStrings = VECTOR_ALL(&bulk->values, char *);
    fprintf(file, "\"Value Strings\": [");
    for (size_t i = 0; i < bulk->values.numElems; i++) {
        fprintf(file, "\"%s\"%s", valueStrings[i], i + 1 < bulk->values.numElems ? ", " : "");
    }
    fprintf(file, "]}");

    return true;
}

CARBON_EXPORT(bool)
carbon_doc_print(FILE *file, const carbon_doc_t *model)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(model)

    if (model->obj_model.numElems == 0) {
        fprintf(file, "{ }");
    }

    if (model->obj_model.numElems > 1) {
        fprintf(file, "[");
    }

    for (size_t numEntries = 0; numEntries < model->obj_model.numElems; numEntries++) {
        carbon_doc_obj_t *object = VECTOR_GET(&model->obj_model, numEntries, carbon_doc_obj_t);
        printObject(file, object);
        fprintf(file, "%s", numEntries + 1 < model->obj_model.numElems ? ", " : "");
    }


    if (model->obj_model.numElems > 1) {
        fprintf(file, "]");
    }

    return true;
}

const carbon_vec_t ofType(carbon_doc_entries_t) *carbon_doc_get_entries(const carbon_doc_obj_t *model)
{
    return &model->entries;
}

void carbon_doc_print_entries(FILE *file, const carbon_doc_entries_t *entries)
{
    fprintf(file, "{\"Key\": \"%s\"", entries->key);
}

void carbon_doc_drop(carbon_doc_obj_t *model)
{
    for (size_t i = 0; i < model->entries.numElems; i++) {
        carbon_doc_entries_t *entry = VECTOR_GET(&model->entries, i, carbon_doc_entries_t);
        entryModelDrop(entry);
    }
    VectorDrop(&model->entries);
}

bool carbon_doc_obj_add_key(carbon_doc_entries_t **out,
                            carbon_doc_obj_t *obj,
                            const char *key,
                            carbon_field_type_e type)
{
    CARBON_NON_NULL_OR_ERROR(out)
    CARBON_NON_NULL_OR_ERROR(obj)
    CARBON_NON_NULL_OR_ERROR(key)

    size_t entry_idx;
    char *key_dup = strdup(key);

    carbon_doc_entries_t entry_model = {
        .type = type,
        .key = key_dup,
        .context = obj
    };

    createTypedVector(&entry_model);
    VectorPush(&obj->doc->context->keys, &key_dup, 1);

    entry_idx = VectorLength(&obj->entries);
    VectorPush(&obj->entries, &entry_model, 1);

    *out = VECTOR_GET(&obj->entries, entry_idx, carbon_doc_entries_t);

    return true;
}

bool carbon_doc_obj_push_primtive(carbon_doc_entries_t *entry, const void *value)
{
    CARBON_NON_NULL_OR_ERROR(entry)
    CARBON_NON_NULL_OR_ERROR((entry->type == carbon_field_type_null) || (value != NULL))

    switch(entry->type) {
        case carbon_field_type_null:
            VectorPush(&entry->values, &VALUE_NULL, 1);
        break;
        case carbon_field_type_string: {
            char *string = value ? strdup((char *) value) : NULL;
            VectorPush(&entry->context->doc->context->values, &string, 1);
            VectorPush(&entry->values, &string, 1);
        }
        break;
    default:
            VectorPush(&entry->values, value, 1);
        break;
    }
    return true;
}

bool carbon_doc_obj_push_object(carbon_doc_obj_t **out, carbon_doc_entries_t *entry)
{
    CARBON_NON_NULL_OR_ERROR(out);
    CARBON_NON_NULL_OR_ERROR(entry);

    assert(entry->type == carbon_field_type_object);

    carbon_doc_obj_t objectModel;

    createObjectModel(&objectModel, entry->context->doc);
    size_t length = VectorLength(&entry->values);
    VectorPush(&entry->values, &objectModel, 1);

    *out = VECTOR_GET(&entry->values, length, carbon_doc_obj_t);

    return true;
}

static carbon_field_type_e valueTypeForJsonNumber(bool *success, carbon_err_t *err, const carbon_json_ast_node_number_t *number)
{
    *success = true;
    switch(number->value_type) {
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER:
        return carbon_field_type_float;
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER: {
        uint64_t test = number->value.unsigned_integer;
        if (test <= CARBON_LIMITS_UINT8_MAX) {
            return carbon_field_type_uint8;
        } else if (test <= CARBON_LIMITS_UINT16_MAX) {
            return carbon_field_type_uint16;
        } else if (test <= CARBON_LIMITS_UINT32_MAX) {
            return carbon_field_type_uint32;
        } else {
            return carbon_field_type_uint64;
        }
    }
    case CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER: {
        int64_t test = number->value.signed_integer;
        if (test >= CARBON_LIMITS_INT8_MIN && test <= CARBON_LIMITS_INT8_MAX) {
            return carbon_field_type_int8;
        } else if (test >= CARBON_LIMITS_INT16_MIN && test <= CARBON_LIMITS_INT16_MAX) {
            return carbon_field_type_int16;
        } else if (test >= CARBON_LIMITS_INT32_MIN && test <= CARBON_LIMITS_INT32_MAX) {
            return carbon_field_type_int32;
        } else {
            return carbon_field_type_int64;
        }
    }
    default:
        CARBON_ERROR(err, CARBON_ERR_NOJSONNUMBERT);
        *success = false;
	return carbon_field_type_int8;
    }
}

static void importJsonObjectStringProperty(carbon_doc_obj_t *target, const char *key, const carbon_json_ast_node_string_t *string)
{
    carbon_doc_entries_t *entry;
    carbon_doc_obj_add_key(&entry, target, key, carbon_field_type_string);
    carbon_doc_obj_push_primtive(entry, string->value);
}

static bool importJsonObjectNumberProperty(carbon_doc_obj_t *target, carbon_err_t *err, const char *key, const carbon_json_ast_node_number_t *number)
{
    carbon_doc_entries_t *entry;
    bool success;
    carbon_field_type_e numberPhyscialType = valueTypeForJsonNumber(&success, err, number);
    if (!success) {
        return false;
    }
    carbon_doc_obj_add_key(&entry, target, key, numberPhyscialType);
    carbon_doc_obj_push_primtive(entry, &number->value);
    return true;
}

static void importJsonObjectBooleanProperty(carbon_doc_obj_t *target, const char *key, carbon_bool_t value)
{
    carbon_doc_entries_t *entry;
    carbon_doc_obj_add_key(&entry, target, key, carbon_field_type_bool);
    carbon_doc_obj_push_primtive(entry, &value);
}

static void importJsonObjectNullProperty(carbon_doc_obj_t *target, const char *key)
{
    carbon_doc_entries_t *entry;
    carbon_doc_obj_add_key(&entry, target, key, carbon_field_type_null);
    carbon_doc_obj_push_primtive(entry, NULL);
}

static bool importJsonObjectObjectProperty(carbon_doc_obj_t *target, carbon_err_t *err, const char *key, const carbon_json_ast_node_object_t *object)
{
    carbon_doc_entries_t *entry;
    carbon_doc_obj_t *nestedObject;
    carbon_doc_obj_add_key(&entry, target, key, carbon_field_type_object);
    carbon_doc_obj_push_object(&nestedObject, entry);
    return importJsonObject(nestedObject, err, object);
}

static bool importJsonObjectArrayProperty(carbon_doc_obj_t *target, carbon_err_t *err, const char *key, const carbon_json_ast_node_array_t *array)
{
    carbon_doc_entries_t *entry;

    if (!VectorIsEmpty(&array->elements.elements))
    {
        size_t numElements = array->elements.elements.numElems;

        /** Find first type that is not null unless the entire array is of type null */
        carbon_json_ast_node_value_type_e arrayDataType = CARBON_JSON_AST_NODE_VALUE_TYPE_NULL;
        carbon_field_type_e documentModelArrayValueType;

        for (size_t i = 0; i < numElements && arrayDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL; i++) {
            const carbon_json_ast_node_element_t *element = VECTOR_GET(&array->elements.elements, i, carbon_json_ast_node_element_t);
            arrayDataType = element->value.value_type;
        }

        switch (arrayDataType) {
        case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
            documentModelArrayValueType = carbon_field_type_object;
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
            documentModelArrayValueType = carbon_field_type_string;
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER: {
            /** find smallest fitting physical number type */
            carbon_field_type_e arrayNumberType = carbon_field_type_null;
            for (size_t i = 0; i < numElements; i++) {
                const carbon_json_ast_node_element_t *element = VECTOR_GET(&array->elements.elements, i, carbon_json_ast_node_element_t);
                if (CARBON_BRANCH_UNLIKELY(element->value.value_type == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL)) {
                    continue;
                } else {
                    bool success;
                    carbon_field_type_e elementNumberType = valueTypeForJsonNumber(&success, err, element->value.value.number);
                    if (!success) {
                        return false;
                    }
                    assert(elementNumberType == carbon_field_type_int8 || elementNumberType == carbon_field_type_int16 ||
                           elementNumberType == carbon_field_type_int32 || elementNumberType == carbon_field_type_int64 ||
                           elementNumberType == carbon_field_type_uint8 || elementNumberType == carbon_field_type_uint16 ||
                           elementNumberType == carbon_field_type_uint32 || elementNumberType == carbon_field_type_uint64 ||
                           elementNumberType == carbon_field_type_float);
                    if (CARBON_BRANCH_UNLIKELY(arrayNumberType == carbon_field_type_null)) {
                        arrayNumberType = elementNumberType;
                    } else {
                        if (arrayNumberType == carbon_field_type_int8) {
                            arrayNumberType = elementNumberType;
                        } else if (arrayNumberType == carbon_field_type_int16) {
                            if (elementNumberType != carbon_field_type_int8) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_int32) {
                            if (elementNumberType != carbon_field_type_int8 && elementNumberType != carbon_field_type_int16) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_int64) {
                            if (elementNumberType != carbon_field_type_int8 && elementNumberType != carbon_field_type_int16 &&
                                elementNumberType != carbon_field_type_int32) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_uint8) {
                            arrayNumberType = elementNumberType;
                        } else if (arrayNumberType == carbon_field_type_uint16) {
                            if (elementNumberType != carbon_field_type_uint16) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_uint32) {
                            if (elementNumberType != carbon_field_type_uint8 && elementNumberType != carbon_field_type_uint16) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_uint64) {
                            if (elementNumberType != carbon_field_type_uint8 && elementNumberType != carbon_field_type_uint16 &&
                                elementNumberType != carbon_field_type_uint32) {
                                arrayNumberType = elementNumberType;
                            }
                        } else if (arrayNumberType == carbon_field_type_float) {
                            break;
                        }
                    }
                }
            }
            assert(arrayNumberType != carbon_field_type_null);
            documentModelArrayValueType = arrayNumberType;
        } break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE:
        case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
            documentModelArrayValueType = carbon_field_type_bool;
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
            documentModelArrayValueType = carbon_field_type_null;
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
            CARBON_ERROR(err, CARBON_ERR_ERRINTERNAL) /** array type is illegal here */
            return false;
        default:
            CARBON_ERROR(err, CARBON_ERR_NOTYPE)
            return false;
        }

        carbon_doc_obj_add_key(&entry, target, key, documentModelArrayValueType);

        for (size_t i = 0; i < numElements; i++)
        {
            const carbon_json_ast_node_element_t *element = VECTOR_GET(&array->elements.elements, i, carbon_json_ast_node_element_t);
            carbon_json_ast_node_value_type_e astNodeDataType = element->value.value_type;

            switch (documentModelArrayValueType) {
            case carbon_field_type_object: {
                carbon_doc_obj_t *nestedObject;
                carbon_doc_obj_push_object(&nestedObject, entry);
                if (astNodeDataType != CARBON_JSON_AST_NODE_VALUE_TYPE_NULL) {
                    /** the object is null by definition, if no entries are contained */
                    if (!importJsonObject(nestedObject, err, element->value.value.object)) {
                        return false;
                    }
                }
            } break;
            case carbon_field_type_string: {
                assert(astNodeDataType == arrayDataType || astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL);
                carbon_doc_obj_push_primtive(entry, astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ?
                                                    CARBON_NULL_ENCODED_STRING :
                                                    element->value.value.string->value);
            } break;
            case carbon_field_type_int8:
            case carbon_field_type_int16:
            case carbon_field_type_int32:
            case carbon_field_type_int64:
            case carbon_field_type_uint8:
            case carbon_field_type_uint16:
            case carbon_field_type_uint32:
            case carbon_field_type_uint64:
            case carbon_field_type_float: {
                assert(astNodeDataType == arrayDataType || astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL);
                switch(documentModelArrayValueType) {
                case carbon_field_type_int8: {
                    carbon_int8_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_INT8 :
                                      (carbon_int8_t) element->value.value.number->value.signed_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_int16: {
                    carbon_int16_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_INT16 :
                                       (carbon_int16_t) element->value.value.number->value.signed_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_int32: {
                    carbon_int32_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_INT32 :
                                       (carbon_int32_t) element->value.value.number->value.signed_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_int64: {
                    carbon_int64_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_INT64 :
                                       (carbon_int64_t) element->value.value.number->value.signed_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_uint8: {
                    carbon_uint8_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_UINT8 :
                                       (carbon_uint8_t) element->value.value.number->value.unsigned_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_uint16: {
                    carbon_uint16_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_UINT16 :
                                        (carbon_uint16_t) element->value.value.number->value.unsigned_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_uint32: {
                    carbon_uin32_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_UINT32 :
                                        (carbon_uin32_t) element->value.value.number->value.unsigned_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_uint64: {
                    carbon_uin64_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL ? CARBON_NULL_UINT64 :
                                        (carbon_uin64_t) element->value.value.number->value.unsigned_integer;
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                case carbon_field_type_float: {
                    carbon_float_t value = CARBON_NULL_FLOAT;
                    if (astNodeDataType != CARBON_JSON_AST_NODE_VALUE_TYPE_NULL) {
                        carbon_json_ast_node_number_value_type_e elementNumberType = element->value.value.number->value_type;
                        if (elementNumberType == CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_REAL_NUMBER) {
                            value = element->value.value.number->value.float_number;
                        } else if (elementNumberType == CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_UNSIGNED_INTEGER) {
                            value = element->value.value.number->value.unsigned_integer;
                        } else if (elementNumberType == CARBON_JSON_AST_NODE_NUMBER_VALUE_TYPE_SIGNED_INTEGER) {
                            value = element->value.value.number->value.signed_integer;
                        } else {
                            CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR) /** type mismatch */
                            return false;
                        }
                    }
                    carbon_doc_obj_push_primtive(entry, &value);
                } break;
                default:
                    CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR) /** not a number type  */
                    return false;
                }
            } break;
            case carbon_field_type_bool:
                if (CARBON_BRANCH_LIKELY(astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE ||
                                  astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE)) {
                    carbon_bool_t value = astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE ?
                                         CARBON_BOOLEAN_TRUE : CARBON_BOOLEAN_FALSE;
                    carbon_doc_obj_push_primtive(entry, &value);
                } else {
                    assert(astNodeDataType == CARBON_JSON_AST_NODE_VALUE_TYPE_NULL);
                    carbon_bool_t value = CARBON_NULL_BOOLEAN;
                    carbon_doc_obj_push_primtive(entry, &value);
                }
                break;
            case carbon_field_type_null:
                assert(astNodeDataType == arrayDataType);
                carbon_doc_obj_push_primtive(entry, NULL);
                break;
            default:
                CARBON_ERROR(err, CARBON_ERR_NOTYPE)
                return false;
            }
        }
    } else {
        importJsonObjectNullProperty(target, key);
    }
    return true;
}

static bool importJsonObject(carbon_doc_obj_t *target, carbon_err_t *err, const carbon_json_ast_node_object_t *jsonObject)
{
    for (size_t i = 0; i < jsonObject->value->members.numElems; i++) {
        carbon_json_ast_node_member_t *member = VECTOR_GET(&jsonObject->value->members, i, carbon_json_ast_node_member_t);
        carbon_json_ast_node_value_type_e valueType = member->value.value.value_type;
        switch (valueType) {
        case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
            importJsonObjectStringProperty(target, member->key.value, member->value.value.value.string);
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER:
            if (!importJsonObjectNumberProperty(target, err, member->key.value, member->value.value.value.number)) {
                return false;
            }
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
        case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE: {
            carbon_bool_t value = valueType == CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE ? CARBON_BOOLEAN_TRUE : CARBON_BOOLEAN_FALSE;
            importJsonObjectBooleanProperty(target, member->key.value, value);
        } break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
            importJsonObjectNullProperty(target, member->key.value);
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
            if (!importJsonObjectObjectProperty(target, err, member->key.value, member->value.value.value.object)) {
                return false;
            }
            break;
        case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
            if (!importJsonObjectArrayProperty(target, err, member->key.value, member->value.value.value.array)) {
                return false;
            }
            break;
        default:
            CARBON_ERROR(err, CARBON_ERR_NOTYPE);
            return false;
        }
    }
    return true;
}

static bool importJson(carbon_doc_obj_t *target, carbon_err_t *err, const carbon_json_t *json, carbon_doc_entries_t *partition)
{
    carbon_json_ast_node_value_type_e valueType = json->element->value.value_type;
    switch (valueType) {
    case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
        if (!importJsonObject(target, err, json->element->value.value.object)) {
            return false;
        }
        break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY: {
        const carbon_vec_t ofType(carbon_json_ast_node_element_t) *arrayContent = &json->element->value.value.array->elements.elements;
        if (!VectorIsEmpty(arrayContent)) {
            const carbon_json_ast_node_element_t *first = VECTOR_GET(arrayContent, 0, carbon_json_ast_node_element_t);
            switch (first->value.value_type) {
            case CARBON_JSON_AST_NODE_VALUE_TYPE_OBJECT:
                if (!importJsonObject(target, err, first->value.value.object)) {
                    return false;
                }
                for (size_t i = 1; i < arrayContent->numElems; i++) {
                    const carbon_json_ast_node_element_t *element = VECTOR_GET(arrayContent, i, carbon_json_ast_node_element_t);
                    carbon_doc_obj_t *nested;
                    carbon_doc_obj_push_object(&nested, partition);
                    if (!importJsonObject(nested, err, element->value.value.object)) {
                        return false;
                    }
                }
                break;
            case CARBON_JSON_AST_NODE_VALUE_TYPE_ARRAY:
            case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
            case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER:
            case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
            case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE:
            case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
            default:
                CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR) /** Unsupported operation in arrays */
                break;
            }
        }
    } break;
    case CARBON_JSON_AST_NODE_VALUE_TYPE_STRING:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NUMBER:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_TRUE:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_FALSE:
    case CARBON_JSON_AST_NODE_VALUE_TYPE_NULL:
    default:
        CARBON_ERROR(err, CARBON_ERR_JSONTYPE);
        return false;
    }
    return true;
}

carbon_doc_obj_t *carbon_doc_bulk_add_json(carbon_doc_entries_t *partition, carbon_json_t *json)
{
    if (!partition || !json) {
        return NULL;
    }

    carbon_doc_obj_t *convertedJson;
    carbon_doc_obj_push_object(&convertedJson, partition);
    if (!importJson(convertedJson, &json->err, json, partition)) {
        return NULL;
    }

    return convertedJson;
}

carbon_doc_obj_t *carbon_doc_entries_get_root(const carbon_doc_entries_t *partition)
{
    return partition ? partition->context : NULL;
}

carbon_doc_entries_t *carbon_doc_bulk_new_entries(carbon_doc_bulk_t *dst)
{
    carbon_doc_entries_t *partition;
    carbon_doc_t *model = carbon_doc_bulk_new_doc(dst, carbon_field_type_object);
    carbon_doc_obj_t *object = carbon_doc_bulk_new_obj(model);
    carbon_doc_obj_add_key(&partition, object, "/", carbon_field_type_object);
    return partition;
}

#define DEFINE_CARBON_TYPE_LQ_FUNC(type)                                         \
static bool compare##type##LessEqFunc(const void *lhs, const void *rhs)         \
{                                                                               \
    type a = *(type *) lhs;                                                     \
    type b = *(type *) rhs;                                                     \
    return (a <= b);                                                            \
}

DEFINE_CARBON_TYPE_LQ_FUNC(carbon_bool_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_float_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_int8_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_int16_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_int32_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_int64_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_uint8_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_uint16_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_uin32_t)
DEFINE_CARBON_TYPE_LQ_FUNC(carbon_uin64_t)

static bool compareEncodedStringLessEqFunc(const void *lhs, const void *rhs, void *args)
{
    carbon_strdic_t *dic = (carbon_strdic_t *) args;
    carbon_string_id_t *a = (carbon_string_id_t *) lhs;
    carbon_string_id_t *b = (carbon_string_id_t *) rhs;
    char **aString = carbon_strdic_extract(dic, a, 1);
    char **bString = carbon_strdic_extract(dic, b, 1);
    bool lq = strcmp(*aString, *bString) <= 0;
    carbon_strdic_free(dic, aString);
    carbon_strdic_free(dic, bString);
    return lq;
}

static void sortNestedPrimitiveObject(carbon_columndoc_obj_t *metaModel)
{
    if (metaModel->parent->read_optimized) {
        for (size_t i = 0; i < metaModel->obj_prop_vals.numElems; i++) {
            carbon_columndoc_obj_t *nestedModel = VECTOR_GET(&metaModel->obj_prop_vals, i, carbon_columndoc_obj_t);
            sortMetaModelEntries(nestedModel);
        }
    }
}

#define DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(type)                                       \
static bool compare##type##ArrayLessEqFunc(const void *lhs, const void *rhs)        \
{                                                                                   \
    carbon_vec_t ofType(type) *a = (carbon_vec_t *) lhs;                                        \
    carbon_vec_t ofType(type) *b = (carbon_vec_t *) rhs;                                        \
    const type *aValues = VECTOR_ALL(a, type);                                      \
    const type *bValues = VECTOR_ALL(b, type);                                      \
    size_t maxCompareIdx = a->numElems < b->numElems ? a->numElems : b->numElems;   \
    for (size_t i = 0; i < maxCompareIdx; i++) {                                    \
        if (aValues[i] > bValues[i]) {                                              \
            return false;                                                           \
        }                                                                           \
    }                                                                               \
    return true;                                                                    \
}

DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_bool_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_int8_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_int16_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_int32_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_int64_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_uint8_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_uint16_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_uin32_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_uin64_t)
DEFINE_CARBON_ARRAY_TYPE_LQ_FUNC(carbon_float_t)

static bool compareEncodedStringArrayLessEqFunc(const void *lhs, const void *rhs, void *args)
{
    carbon_strdic_t *dic = (carbon_strdic_t *) args;
    carbon_vec_t ofType(carbon_string_id_t) *a = (carbon_vec_t *) lhs;
    carbon_vec_t ofType(carbon_string_id_t) *b = (carbon_vec_t *) rhs;
    const carbon_string_id_t *aValues = VECTOR_ALL(a, carbon_string_id_t);
    const carbon_string_id_t *bValues = VECTOR_ALL(b, carbon_string_id_t);
    size_t maxCompareIdx = a->numElems < b->numElems ? a->numElems : b->numElems;
    for (size_t i = 0; i < maxCompareIdx; i++) {
        char **aString = carbon_strdic_extract(dic, aValues + i, 1);
        char **bString = carbon_strdic_extract(dic, bValues + i, 1);
        bool greater = strcmp(*aString, *bString) > 0;
        carbon_strdic_free(dic, aString);
        carbon_strdic_free(dic, bString);
        if (greater) {
            return false;
        }
    }
    return true;
}

static void sortedNestedArrayObjects(carbon_columndoc_obj_t *metaModel)
{
    if (metaModel->parent->read_optimized) {
        for (size_t i = 0; i < metaModel->obj_array_props.numElems; i++) {
            carbon_columndoc_columngroup_t *arrayColumns = VECTOR_GET(&metaModel->obj_array_props, i, carbon_columndoc_columngroup_t);
            for (size_t j = 0; j < arrayColumns->columns.numElems; j++) {
                carbon_columndoc_column_t *column = VECTOR_GET(&arrayColumns->columns, j, carbon_columndoc_column_t);
                carbon_vec_t ofType(uint32_t) *arrayIndices = &column->array_positions;
                carbon_vec_t ofType(carbon_vec_t ofType(<T>)) *valuesForIndicies = &column->values;
                assert (arrayIndices->numElems == valuesForIndicies->numElems);

                for (size_t k = 0; k < arrayIndices->numElems; k++) {
                    carbon_vec_t ofType(<T>) *valuesForIndex = VECTOR_GET(valuesForIndicies, k, carbon_vec_t);
                    if (column->type == carbon_field_type_object) {
                        for (size_t l = 0; l < valuesForIndex->numElems; l++) {
                            carbon_columndoc_obj_t *nestedObject = VECTOR_GET(valuesForIndex, l, carbon_columndoc_obj_t);
                            sortMetaModelEntries(nestedObject);
                        }
                    }
                }
            }
        }
    }
}

#define SORT_META_MODEL_VALUES(keyVector, valueVector, valueType, compareValueFunc)                                    \
{                                                                                                                      \
    size_t numElements = VectorLength(&keyVector);                                                                     \
                                                                                                                       \
    if (numElements > 0) {                                                                                             \
        size_t *valueIndicies = malloc(sizeof(size_t) * numElements);                                                  \
        for (size_t i = 0; i < numElements; i++) {                                                                     \
            valueIndicies[i] = i;                                                                                      \
        }                                                                                                              \
                                                                                                                       \
        carbon_vec_t ofType(carbon_string_id_t) keyCpy;                                                                                \
        carbon_vec_t ofType(valueType) valueCpy;                                                                             \
                                                                                                                       \
        VectorCpy(&keyCpy, &keyVector);                                                                                \
        VectorCpy(&valueCpy, &valueVector);                                                                            \
                                                                                                                       \
        valueType *values = VECTOR_ALL(&valueCpy, valueType);                                                          \
                                                                                                                       \
        carbon_sort_qsort_indicies(valueIndicies, values, sizeof(valueType), compareValueFunc, numElements,                         \
                      keyVector.allocator);                                                                            \
                                                                                                                       \
        for (size_t i = 0; i < numElements; i++) {                                                                     \
            VectorSet(&keyVector, i, VECTOR_GET(&keyCpy, valueIndicies[i], carbon_string_id_t));                                 \
            VectorSet(&valueVector, i, VECTOR_GET(&valueCpy, valueIndicies[i], valueType));                            \
        }                                                                                                              \
                                                                                                                       \
                                                                                                                       \
        free(valueIndicies);                                                                                           \
        VectorDrop(&keyCpy);                                                                                           \
        VectorDrop(&valueCpy);                                                                                         \
    }                                                                                                                  \
}

static void sortMetaModelStringValues(carbon_vec_t ofType(carbon_string_id_t) *keyVector, carbon_vec_t ofType(carbon_string_id_t) *valueVector,
                                      carbon_strdic_t *dic)
{
    size_t numElements = VectorLength(keyVector);

    if (numElements > 0) {
        size_t *valueIndicies = malloc(sizeof(size_t) * numElements);
        for (size_t i = 0; i < numElements; i++) {
            valueIndicies[i] = i;
        }

        carbon_vec_t ofType(carbon_string_id_t) keyCpy;
        carbon_vec_t ofType(carbon_string_id_t) valueCpy;

        VectorCpy(&keyCpy, keyVector);
        VectorCpy(&valueCpy, valueVector);

        carbon_string_id_t *values = VECTOR_ALL(&valueCpy, carbon_string_id_t);

        carbon_sort_qsort_indicies_wargs(valueIndicies,
                                         values,
                                         sizeof(carbon_string_id_t),
                                         compareEncodedStringLessEqFunc,
                                         numElements,
                                         keyVector->allocator,
                                         dic);

        for (size_t i = 0; i < numElements; i++) {
            VectorSet(keyVector, i, VECTOR_GET(&keyCpy, valueIndicies[i], carbon_string_id_t));
            VectorSet(valueVector, i, VECTOR_GET(&valueCpy, valueIndicies[i], carbon_string_id_t));
        }

        free(valueIndicies);
        VectorDrop(&keyCpy);
        VectorDrop(&valueCpy);
    }
}

#define SORT_META_MODEL_ARRAYS(keyVector, valueArrayVector, compareFunc)                                               \
{                                                                                                                      \
    size_t numElements = VectorLength(&keyVector);                                                                     \
                                                                                                                       \
    if (numElements > 0) {                                                                                             \
        size_t *valueIndicies = malloc(sizeof(size_t) * numElements);                                                  \
        for (size_t i = 0; i < numElements; i++) {                                                                     \
            valueIndicies[i] = i;                                                                                      \
        }                                                                                                              \
                                                                                                                       \
        carbon_vec_t ofType(carbon_string_id_t) keyCpy;                                                                                \
        carbon_vec_t ofType(carbon_vec_t) valueCpy;                                                                                \
                                                                                                                       \
        VectorCpy(&keyCpy, &keyVector);                                                                                \
        VectorCpy(&valueCpy, &valueArrayVector);                                                                       \
                                                                                                                       \
        const carbon_vec_t *values = VECTOR_ALL(&valueArrayVector, carbon_vec_t);                                                  \
                                                                                                                       \
        carbon_sort_qsort_indicies(valueIndicies, values, sizeof(carbon_vec_t), compareFunc, numElements,                                 \
                      keyVector.allocator);                                                                            \
                                                                                                                       \
        for (size_t i = 0; i < numElements; i++) {                                                                     \
            VectorSet(&keyVector, i, VECTOR_GET(&keyCpy, valueIndicies[i], carbon_string_id_t));                                 \
            VectorSet(&valueArrayVector, i, VECTOR_GET(&valueCpy, valueIndicies[i], carbon_vec_t));                          \
        }                                                                                                              \
                                                                                                                       \
        free(valueIndicies);                                                                                           \
        VectorDrop(&keyCpy);                                                                                           \
        VectorDrop(&valueCpy);                                                                                         \
    }                                                                                                                  \
}

static void sortMetaModelStringArrays(carbon_vec_t ofType(carbon_string_id_t) *keyVector, carbon_vec_t ofType(carbon_string_id_t) *valueArrayVector,
                                      carbon_strdic_t *dic)
{
    size_t numElements = VectorLength(keyVector);

    if (numElements > 0) {
        size_t *valueIndicies = malloc(sizeof(size_t) * numElements);
        for (size_t i = 0; i < numElements; i++) {
            valueIndicies[i] = i;
        }

        carbon_vec_t ofType(carbon_string_id_t) keyCpy;
        carbon_vec_t ofType(carbon_vec_t) valueCpy;

        VectorCpy(&keyCpy, keyVector);
        VectorCpy(&valueCpy, valueArrayVector);

        const carbon_vec_t *values = VECTOR_ALL(valueArrayVector, carbon_vec_t);

        carbon_sort_qsort_indicies_wargs(valueIndicies,
                                         values,
                                         sizeof(carbon_vec_t),
                                         compareEncodedStringArrayLessEqFunc,
                                         numElements,
                                         keyVector->allocator,
                                         dic);

        for (size_t i = 0; i < numElements; i++) {
            VectorSet(keyVector, i, VECTOR_GET(&keyCpy, valueIndicies[i], carbon_string_id_t));
            VectorSet(valueArrayVector, i, VECTOR_GET(&valueCpy, valueIndicies[i], carbon_vec_t));
        }

        free(valueIndicies);
        VectorDrop(&keyCpy);
        VectorDrop(&valueCpy);
    }
}


static bool compareObjectArrayKeyColumnsLessEqFunc(const void *lhs, const void *rhs, void *args)
{
    carbon_strdic_t *dic = (carbon_strdic_t *) args;
    carbon_columndoc_columngroup_t *a = (carbon_columndoc_columngroup_t *) lhs;
    carbon_columndoc_columngroup_t *b = (carbon_columndoc_columngroup_t *) rhs;
    char **aColumnName = carbon_strdic_extract(dic, &a->key, 1);
    char **bColumnName = carbon_strdic_extract(dic, &b->key, 1);
    bool columnNameLeq = strcmp(*aColumnName, *bColumnName) <= 0;
    carbon_strdic_free(dic, aColumnName);
    carbon_strdic_free(dic, bColumnName);
    return columnNameLeq;
}

static bool compareObjectArrayKeyColumnLessEqFunc(const void *lhs, const void *rhs, void *args)
{
    carbon_strdic_t *dic = (carbon_strdic_t *) args;
    carbon_columndoc_column_t *a = (carbon_columndoc_column_t *) lhs;
    carbon_columndoc_column_t *b = (carbon_columndoc_column_t *) rhs;
    char **aColumnName = carbon_strdic_extract(dic, &a->key_name, 1);
    char **bColumnName = carbon_strdic_extract(dic, &b->key_name, 1);
    int cmpResult = strcmp(*aColumnName, *bColumnName);
    bool columnNameLeq = cmpResult < 0 ? true : (cmpResult == 0 ? (a->type <= b->type) : false);
    carbon_strdic_free(dic, aColumnName);
    carbon_strdic_free(dic, bColumnName);
    return columnNameLeq;
}

typedef struct {
    carbon_strdic_t *dic;
    carbon_field_type_e valueType;
} CompareColumnLessEqFuncArg;

#define ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, type, valueVectorAPtr, valueVectorBPtr)    \
{                                                                                       \
    for (size_t i = 0; i < maxNumElem; i++) {                                           \
        type o1 = *VECTOR_GET(valueVectorAPtr, i, type);                                \
        type o2 = *VECTOR_GET(valueVectorBPtr, i, type);                                \
        if (o1 > o2) {                                                                  \
            return false;                                                               \
        }                                                                               \
    }                                                                                   \
    return true;                                                                        \
}

static bool compareColumnLessEqFunc(const void *lhs, const void *rhs, void *args)
{
    carbon_vec_t ofType(<T>) *a = (carbon_vec_t *) lhs;
    carbon_vec_t ofType(<T>) *b = (carbon_vec_t *) rhs;
    CompareColumnLessEqFuncArg *funcArg = (CompareColumnLessEqFuncArg *) args;

    size_t maxNumElem = CARBON_MIN(a->numElems, b->numElems);

    switch (funcArg->valueType) {
    case carbon_field_type_null:
        return (a->numElems <= b->numElems);
        break;
    case carbon_field_type_bool:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_bool_t, a, b);
        break;
    case carbon_field_type_int8:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_int8_t, a, b);
        break;
    case carbon_field_type_int16:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_int16_t, a, b);
        break;
    case carbon_field_type_int32:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_int32_t, a, b);
        break;
    case carbon_field_type_int64:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_int64_t, a, b);
        break;
    case carbon_field_type_uint8:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_uint8_t, a, b);
        break;
    case carbon_field_type_uint16:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_uint16_t, a, b);
        break;
    case carbon_field_type_uint32:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_uin32_t, a, b);
        break;
    case carbon_field_type_uint64:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_uin64_t, a, b);
        break;
    case carbon_field_type_float:
        ARRAY_LEQ_PRIMITIVE_FUNC(maxNumElem, carbon_float_t, a, b);
        break;
    case carbon_field_type_string:
        for (size_t i = 0; i < maxNumElem; i++) {
            carbon_string_id_t o1 = *VECTOR_GET(a, i, carbon_string_id_t);
            carbon_string_id_t o2 = *VECTOR_GET(b, i, carbon_string_id_t);
            char **o1String = carbon_strdic_extract(funcArg->dic, &o1, 1);
            char **o2String = carbon_strdic_extract(funcArg->dic, &o2, 1);
            bool greater = strcmp(*o1String, *o2String) > 0;
            carbon_strdic_free(funcArg->dic, o1String);
            carbon_strdic_free(funcArg->dic, o2String);
            if (greater) {
                return false;
            }
        }
        return true;
    case carbon_field_type_object:
        return true;
        break;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_NOTYPE)
        return false;
    }
}

static void sortMetaModelColumn(carbon_columndoc_column_t *column, carbon_strdic_t *dic)
{
    /** Sort column by its value, and re-arrange the array position list according this new order */
    carbon_vec_t ofType(uint32_t) arrayPositionCpy;
    carbon_vec_t ofType(carbon_vec_t ofType(<T>)) valuesCpy;

    VectorCpy(&arrayPositionCpy, &column->array_positions);
    VectorCpy(&valuesCpy, &column->values);

    assert(column->array_positions.numElems == column->values.numElems);
    assert(arrayPositionCpy.numElems == valuesCpy.numElems);
    assert(valuesCpy.numElems == column->array_positions.numElems);

    size_t *indices = malloc(valuesCpy.numElems * sizeof(size_t));
    for (size_t i = 0; i < valuesCpy.numElems; i++) {
        indices[i] = i;
    }

    CompareColumnLessEqFuncArg funcArg = {
        .dic = dic,
        .valueType = column->type
    };

    carbon_sort_qsort_indicies_wargs(indices,
                                     valuesCpy.base,
                                     valuesCpy.elemSize,
                                     compareColumnLessEqFunc,
                                     valuesCpy.numElems,
                                     valuesCpy.allocator,
                                     &funcArg);

    for (size_t i = 0; i < valuesCpy.numElems; i++) {
        VectorSet(&column->values, i, VectorAt(&valuesCpy, indices[i]));
        VectorSet(&column->array_positions, i, VectorAt(&arrayPositionCpy, indices[i]));
    }

    free (indices);
    VectorDrop(&arrayPositionCpy);
    VectorDrop(&valuesCpy);
}

static void sortMetaModelColumnArrays(carbon_columndoc_obj_t *metaModel)
{
    carbon_vec_t ofType(carbon_columndoc_columngroup_t) cpy;
    VectorCpy(&cpy, &metaModel->obj_array_props);
    size_t *indices = malloc(cpy.numElems * sizeof(size_t));
    for (size_t i = 0; i < cpy.numElems; i++) {
        indices[i] = i;
    }
    carbon_sort_qsort_indicies_wargs(indices,
                                     cpy.base,
                                     sizeof(carbon_columndoc_columngroup_t),
                                     compareObjectArrayKeyColumnsLessEqFunc,
                                     cpy.numElems,
                                     cpy.allocator,
                                     metaModel->parent->dic);
    for (size_t i = 0; i < cpy.numElems; i++) {
        VectorSet(&metaModel->obj_array_props, i, VECTOR_GET(&cpy, indices[i], carbon_columndoc_columngroup_t));
    }
    free(indices);

    for (size_t i = 0; i < cpy.numElems; i++) {
        carbon_columndoc_columngroup_t *keyColumns = VECTOR_GET(&metaModel->obj_array_props, i, carbon_columndoc_columngroup_t);
        size_t *columnIndices = malloc(keyColumns->columns.numElems * sizeof(size_t));
        carbon_vec_t ofType(carbon_columndoc_column_t) columnCpy;
        VectorCpy(&columnCpy, &keyColumns->columns);
        for (size_t i = 0; i < keyColumns->columns.numElems; i++) {
            columnIndices[i] = i;
        }

        /** First, sort by column name; Then, sort by columns with same name by type */
        carbon_sort_qsort_indicies_wargs(columnIndices, columnCpy.base, sizeof(carbon_columndoc_column_t),
                                         compareObjectArrayKeyColumnLessEqFunc, keyColumns->columns.numElems,
                                         keyColumns->columns.allocator, metaModel->parent->dic);
        for (size_t i = 0; i < keyColumns->columns.numElems; i++) {
            VectorSet(&keyColumns->columns, i, VECTOR_GET(&columnCpy, columnIndices[i], carbon_columndoc_column_t));
            carbon_columndoc_column_t *column = VECTOR_GET(&keyColumns->columns, i, carbon_columndoc_column_t);
            sortMetaModelColumn(column, metaModel->parent->dic);
        }

        VectorDrop(&columnCpy);
        free(columnIndices);
    }
    VectorDrop(&cpy);
}

static void sortMetaModelValues(carbon_columndoc_obj_t *metaModel)
{
    if (metaModel->parent->read_optimized) {
        SORT_META_MODEL_VALUES(metaModel->bool_prop_keys, metaModel->bool_prop_vals, carbon_bool_t,
                               comparecarbon_bool_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->int8_prop_keys, metaModel->int8_prop_vals, carbon_int8_t,
                               comparecarbon_int8_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->int16_prop_keys, metaModel->int16_prop_vals, carbon_int16_t,
                               comparecarbon_int16_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->int32_prop_keys, metaModel->int32_prop_vals, carbon_int32_t,
                               comparecarbon_int32_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->int64_prop_keys, metaModel->int64_prop_vals, carbon_int64_t,
                               comparecarbon_int64_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->uint8_prop_keys, metaModel->uint8_prop_vals, carbon_uint8_t,
                               comparecarbon_uint8_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->uint16_prop_keys, metaModel->uint16_prop_vals, carbon_uint16_t,
                               comparecarbon_uint16_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->uin32_prop_keys, metaModel->uint32_prop_vals, carbon_uin32_t,
                               comparecarbon_uin32_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->uint64_prop_keys, metaModel->uint64_prop_vals, carbon_uin64_t,
                               comparecarbon_uin64_tLessEqFunc);
        SORT_META_MODEL_VALUES(metaModel->float_prop_keys, metaModel->float_prop_vals, carbon_float_t,
                               comparecarbon_float_tLessEqFunc);
        sortMetaModelStringValues(&metaModel->string_prop_keys, &metaModel->string_prop_vals,
                                  metaModel->parent->dic);

        SORT_META_MODEL_ARRAYS(metaModel->bool_array_prop_keys, metaModel->bool_array_prop_vals,
                               comparecarbon_bool_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->int8_array_prop_keys, metaModel->int8_array_prop_vals,
                               comparecarbon_int8_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->int16_array_prop_keys, metaModel->int16_array_prop_vals,
                               comparecarbon_int16_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->int32_array_prop_keys, metaModel->int32_array_prop_vals,
                               comparecarbon_int32_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->int64_array_prop_keys, metaModel->int64_array_prop_vals,
                               comparecarbon_int64_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->uint8_array_prop_keys, metaModel->uint8_array_prop_vals,
                               comparecarbon_uint8_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->uint16_array_prop_keys, metaModel->uint16_array_prop_vals,
                               comparecarbon_uint16_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->uint32_array_prop_keys, metaModel->uint32_array_prop_vals,
                               comparecarbon_uin32_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->uint64_array_prop_keys, metaModel->uin64_array_prop_vals,
                               comparecarbon_uin64_tArrayLessEqFunc);
        SORT_META_MODEL_ARRAYS(metaModel->float_array_prop_keys, metaModel->float_array_prop_vals,
                               comparecarbon_float_tArrayLessEqFunc);
        sortMetaModelStringArrays(&metaModel->string_array_prop_keys, &metaModel->string_array_prop_vals,
                                  metaModel->parent->dic);

        sortMetaModelColumnArrays(metaModel);
    }
}

static void sortMetaModelEntries(carbon_columndoc_obj_t *metaModel)
{
    if (metaModel->parent->read_optimized) {
        sortMetaModelValues(metaModel);
        sortNestedPrimitiveObject(metaModel);
        sortedNestedArrayObjects(metaModel);
    }
}

carbon_columndoc_t *carbon_doc_entries_to_columndoc(const carbon_doc_bulk_t *bulk,
                                                          const carbon_doc_entries_t *partition,
                                                          bool read_optimized)
{
    if (!bulk || !partition) {
        return NULL;
    }

    // Step 1: encode all strings at once in a bulk
    char *const* keyStrings = VECTOR_ALL(&bulk->keys, char *);
    char *const* valueStrings = VECTOR_ALL(&bulk->values, char *);
    carbon_strdic_insert(bulk->dic, NULL, keyStrings, VectorLength(&bulk->keys), 0);
    carbon_strdic_insert(bulk->dic, NULL, valueStrings, VectorLength(&bulk->values), 0);

    // Step 2: for each document doc, create a meta doc, and construct a binary compressed document
    const carbon_doc_t *models = VECTOR_ALL(&bulk->models, carbon_doc_t);
    assert (bulk->models.numElems == 1);

    const carbon_doc_t *model = models;

    // TODO: DEBUG remove these lines
   // fprintf(stdout, "\nDocument Model:\n");
   // DocumentModelPrint(stdout, doc);

    carbon_columndoc_t *metaModel = malloc(sizeof(carbon_columndoc_t));
    metaModel->read_optimized = read_optimized;
    carbon_err_t err;
    if (!carbon_columndoc_create(metaModel, &err, model, bulk, partition, bulk->dic)) {
        carbon_error_print_and_abort(&err);
    }


    if (metaModel->read_optimized) {
        sortMetaModelEntries(&metaModel->columndoc);
    }


  //  fprintf(stdout, "\nDocument Meta Model:\n");
  //  DocumentMetaModelPrint(stdout, metaModel);

    return metaModel;
}

CARBON_EXPORT(bool)
carbon_doc_entries_drop(carbon_doc_entries_t *partition)
{
    CARBON_UNUSED(partition);
    return true;
}

static void createObjectModel(carbon_doc_obj_t *model, carbon_doc_t *doc)
{
    VectorCreate(&model->entries, NULL, sizeof(carbon_doc_entries_t), 50);
    model->doc = doc;
}

static void createTypedVector(carbon_doc_entries_t *entry)
{
    size_t size;
    switch (entry->type) {
    case carbon_field_type_null:
        size = sizeof(carbon_null_t);
        break;
    case carbon_field_type_bool:
        size = sizeof(carbon_bool_t);
        break;
    case carbon_field_type_int8:
        size = sizeof(carbon_int8_t);
        break;
    case carbon_field_type_int16:
        size = sizeof(carbon_int16_t);
        break;
    case carbon_field_type_int32:
        size = sizeof(carbon_int32_t);
        break;
    case carbon_field_type_int64:
        size = sizeof(carbon_int64_t);
        break;
    case carbon_field_type_uint8:
        size = sizeof(carbon_uint8_t);
        break;
    case carbon_field_type_uint16:
        size = sizeof(carbon_uint16_t);
        break;
    case carbon_field_type_uint32:
        size = sizeof(carbon_uin32_t);
        break;
    case carbon_field_type_uint64:
        size = sizeof(carbon_uin64_t);
        break;
    case carbon_field_type_float:
        size = sizeof(carbon_float_t);
        break;
    case carbon_field_type_string:
        size = sizeof(carbon_cstring_t);
        break;
    case carbon_field_type_object:
        size = sizeof(carbon_doc_obj_t);
        break;
    default:
        CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR) /** unknown type */
        return;
    }
    VectorCreate(&entry->values, NULL, size, 10);
}

static void entryModelDrop(carbon_doc_entries_t *entry)
{
    if (entry->type == carbon_field_type_object) {
        for (size_t i = 0; i < entry->values.numElems; i++)
        {
            carbon_doc_obj_t *model = VECTOR_GET(&entry->values, i, carbon_doc_obj_t);
            carbon_doc_drop(model);
        }
    }
    VectorDrop(&entry->values);
}

static bool printValue(FILE *file, carbon_field_type_e type, const carbon_vec_t ofType(<T>) *values)
{
    size_t numValues = values->numElems;
    if (numValues == 0) {
        fprintf(file, "null");
        return true;
    }
    if (numValues > 1) {
        fprintf(file, "[");
    }
    switch (type) {
    case carbon_field_type_null:
    {
        for (size_t i = 0; i < numValues; i++) {
            fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
        }
    } break;
    case carbon_field_type_bool:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_bool_t value = *(VECTOR_GET(values, i, carbon_bool_t));
            if (value != CARBON_NULL_BOOLEAN) {
                fprintf(file, "%s%s", value == 0 ? "false" : "true", i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_int8:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_int8_t value = *(VECTOR_GET(values, i, carbon_int8_t));
            if (value != CARBON_NULL_INT8) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_int16:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_int16_t value = *(VECTOR_GET(values, i, carbon_int16_t));
            if (value != CARBON_NULL_INT16) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_int32:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_int32_t value = *(VECTOR_GET(values, i, carbon_int32_t));
            if (value != CARBON_NULL_INT32) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_int64:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_int64_t value = *(VECTOR_GET(values, i, carbon_int64_t));
            if (value != CARBON_NULL_INT64) {
                fprintf(file, "%" PRIi64 "%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_uint8:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_uint8_t value = *(VECTOR_GET(values, i, carbon_uint8_t));
            if (value != CARBON_NULL_UINT8) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_uint16:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_uint16_t value = *(VECTOR_GET(values, i, carbon_uint16_t));
            if (value != CARBON_NULL_UINT16) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_uint32:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_uin32_t value = *(VECTOR_GET(values, i, carbon_uin32_t));
            if (value != CARBON_NULL_UINT32) {
                fprintf(file, "%d%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_uint64:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_uin64_t value = *(VECTOR_GET(values, i, carbon_uin64_t));
            if (value != CARBON_NULL_UINT64) {
                fprintf(file, "%" PRIu64 "%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_float:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_float_t value = *(VECTOR_GET(values, i, carbon_float_t));
            if (!isnan(value)) {
                fprintf(file, "%f%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_string:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_cstring_t value = *(VECTOR_GET(values, i, carbon_cstring_t));
            if (value) {
                fprintf(file, "\"%s\"%s", value, i + 1 < numValues ? ", " : "");
            } else {
                fprintf(file, "null%s", i + 1 < numValues ? ", " : "");
            }
        }
    } break;
    case carbon_field_type_object:
    {
        for (size_t i = 0; i < numValues; i++) {
            carbon_doc_obj_t *obj = VECTOR_GET(values, i, carbon_doc_obj_t);
            if (!CARBON_NULL_OBJECT_MODEL(obj)) {
                printObject(file, obj);
            } else {
                fprintf(file, "null");
            }
            fprintf(file, "%s", i + 1 < numValues ? ", " : "");
        }
    } break;
    default:
        CARBON_NOT_IMPLEMENTED;
    }
    if (numValues > 1) {
        fprintf(file, "]");
    }
    return true;
}

static void printObject(FILE *file, const carbon_doc_obj_t *model)
{
    fprintf(file, "{");
    for (size_t i = 0; i < model->entries.numElems; i++) {
        carbon_doc_entries_t *entry = VECTOR_GET(&model->entries, i, carbon_doc_entries_t);
        fprintf(file, "\"%s\": ", entry->key);
        printValue(file, entry->type, &entry->values);
        fprintf(file, "%s", i + 1 < model->entries.numElems ? ", " : "");
    }
    fprintf(file, "}");
}

