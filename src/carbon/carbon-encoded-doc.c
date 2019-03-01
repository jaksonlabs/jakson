/**
 * Copyright 2019 Marcus Pinnecke
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

#include <math.h>

#include <carbon/carbon-encoded-doc.h>
#include "carbon/carbon-encoded-doc.h"

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_create(carbon_encoded_doc_collection_t *collection, carbon_err_t *err,
                                     carbon_archive_t *archive)
{
    CARBON_UNUSED(collection);
    CARBON_UNUSED(err);
    CARBON_UNUSED(archive);

    carbon_vec_create(&collection->flat_object_collection, NULL, sizeof(carbon_encoded_doc_t), 5000000);
    carbon_hashtable_create(&collection->index, err, sizeof(carbon_object_id_t), sizeof(uint32_t), 5000000);
    carbon_error_init(&collection->err);
    collection->archive = archive;

    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_drop(carbon_encoded_doc_collection_t *collection)
{
    CARBON_UNUSED(collection);

    carbon_hashtable_drop(&collection->index);
    for (uint32_t i = 0; i < collection->flat_object_collection.num_elems; i++) {
        carbon_encoded_doc_t *doc = CARBON_VECTOR_GET(&collection->flat_object_collection, i, carbon_encoded_doc_t);
        carbon_encoded_doc_drop(doc);
    }
    carbon_vec_drop(&collection->flat_object_collection);
    return true;
}

static carbon_encoded_doc_t *
doc_create(carbon_err_t *err, carbon_object_id_t object_id,
           carbon_encoded_doc_collection_t *collection)
{
    if (collection) {
        uint32_t doc_position = collection->flat_object_collection.num_elems;
        carbon_encoded_doc_t *new_doc = VECTOR_NEW_AND_GET(&collection->flat_object_collection, carbon_encoded_doc_t);
        new_doc->context = collection;
        new_doc->object_id = object_id;
        carbon_vec_create(&new_doc->props, NULL, sizeof(carbon_encoded_doc_prop_t), 20);
        carbon_vec_create(&new_doc->props_arrays, NULL, sizeof(carbon_encoded_doc_prop_array_t), 20);
        carbon_hashtable_create(&new_doc->prop_array_index, err, sizeof(carbon_string_id_t), sizeof(uint32_t), 20);
        carbon_error_init(&new_doc->err);
        carbon_hashtable_insert_or_update(&collection->index, &object_id, &doc_position, 1);
        return new_doc;
    } else {
        CARBON_ERROR(err, CARBON_ERR_ILLEGALARG);
        return NULL;
    }
}

CARBON_EXPORT(carbon_encoded_doc_t *)
encoded_doc_collection_get_or_append(carbon_encoded_doc_collection_t *collection, carbon_object_id_t id)
{
    CARBON_NON_NULL_OR_ERROR(collection);
    const uint32_t *doc_pos = carbon_hashtable_get_value(&collection->index, &id);
    if (doc_pos)
    {
        carbon_encoded_doc_t *result = CARBON_VECTOR_GET(&collection->flat_object_collection, *doc_pos, carbon_encoded_doc_t);
        CARBON_ERROR_IF(result == NULL, &collection->err, CARBON_ERR_INTERNALERR);
        return result;
    } else
    {
        carbon_encoded_doc_t *result = doc_create(&collection->err, id, collection);
        if (!result) {
            CARBON_ERROR(&collection->err, CARBON_ERR_INTERNALERR);
        }
        return result;
    }
}

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_print(FILE *file, carbon_encoded_doc_collection_t *collection)
{
    CARBON_UNUSED(file);
    CARBON_UNUSED(collection);

    if (collection->flat_object_collection.num_elems > 0) {
        carbon_encoded_doc_t *root = CARBON_VECTOR_GET(&collection->flat_object_collection, 0, carbon_encoded_doc_t);
        carbon_encoded_doc_print(file, root);
    }

    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_drop(carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(doc);
    for (uint32_t i = 0; i < doc->props_arrays.num_elems; i++)
    {
        carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        carbon_vec_drop(&array->values);
    }
    for (uint32_t i = 0; i < doc->props.num_elems; i++)
    {
        carbon_encoded_doc_prop_t *single = CARBON_VECTOR_GET(&doc->props, i, carbon_encoded_doc_prop_t);
        if (single->header.value_type == CARBON_ENCODED_DOC_PROP_VALUE_TYPE_DECODED_STRING) {
            free(single->value.string);
        }
    }
    carbon_vec_drop(&doc->props);
    carbon_vec_drop(&doc->props_arrays);
    carbon_hashtable_drop(&doc->prop_array_index);
    return false;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_get_object_id(carbon_object_id_t *oid, carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(oid);
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

#define DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(built_in_type, basic_type, value_name)                               \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##value_name(carbon_encoded_doc_t *doc, carbon_string_id_t key, built_in_type value)       \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;                                        \
    prop->header.key.key_id = key;                                                                                     \
    prop->header.value_type = CARBON_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

#define DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(built_in_type, basic_type, value_name)                       \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##value_name##_decoded(carbon_encoded_doc_t *doc, const char *key, built_in_type value)    \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;                                        \
    prop->header.key.key_str = strdup(key);                                                                            \
    prop->header.value_type = CARBON_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}


DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_int8_t, CARBON_BASIC_TYPE_INT8, int8)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_int16_t, CARBON_BASIC_TYPE_INT16, int16)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_int32_t, CARBON_BASIC_TYPE_INT32, int32)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_int64_t, CARBON_BASIC_TYPE_INT64, int64)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_uint8_t, CARBON_BASIC_TYPE_UINT8, uint8)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_uint16_t, CARBON_BASIC_TYPE_UINT16, uint16)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_uint32_t, CARBON_BASIC_TYPE_UINT32, uint32)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_uint64_t, CARBON_BASIC_TYPE_UINT64, uint64)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_number_t, CARBON_BASIC_TYPE_NUMBER, number)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_boolean_t, CARBON_BASIC_TYPE_BOOLEAN, boolean)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(carbon_string_id_t, CARBON_BASIC_TYPE_STRING, string)

DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_int8_t, CARBON_BASIC_TYPE_INT8, int8)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_int16_t, CARBON_BASIC_TYPE_INT16, int16)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_int32_t, CARBON_BASIC_TYPE_INT32, int32)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_int64_t, CARBON_BASIC_TYPE_INT64, int64)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_uint8_t, CARBON_BASIC_TYPE_UINT8, uint8)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_uint16_t, CARBON_BASIC_TYPE_UINT16, uint16)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_uint32_t, CARBON_BASIC_TYPE_UINT32, uint32)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_uint64_t, CARBON_BASIC_TYPE_UINT64, uint64)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_number_t, CARBON_BASIC_TYPE_NUMBER, number)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_boolean_t, CARBON_BASIC_TYPE_BOOLEAN, boolean)
DECLARE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_string_id_t, CARBON_BASIC_TYPE_STRING, string)

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(carbon_encoded_doc_t *doc, const char *key, const char *value)
{
    CARBON_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = CARBON_BASIC_TYPE_STRING;
    prop->value.string = strdup(value);
    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_null(carbon_encoded_doc_t *doc, carbon_string_id_t key)
{
    CARBON_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;
    prop->header.key.key_id = key;
    prop->header.type = CARBON_BASIC_TYPE_NULL;
    prop->value.builtin.null = 1;
    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_null_decoded(carbon_encoded_doc_t *doc, const char *key)
{
    CARBON_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = CARBON_BASIC_TYPE_NULL;
    prop->value.builtin.null = 1;
    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_encoded_doc_t *value)
{
    CARBON_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;
    prop->header.key.key_id = key;
    prop->header.type = CARBON_BASIC_TYPE_OBJECT;
    prop->value.builtin.object = value->object_id;
    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_encoded_doc_t *value)
{
    CARBON_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = CARBON_BASIC_TYPE_OBJECT;
    prop->value.builtin.object = value->object_id;
    return true;
}

#define DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name, basic_type)                                               \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name(carbon_encoded_doc_t *doc,                                                    \
                                       carbon_string_id_t key)                                                         \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(doc)                                                                                      \
    uint32_t new_array_pos = doc->props_arrays.num_elems;                                                              \
    carbon_encoded_doc_prop_array_t *array = VECTOR_NEW_AND_GET(&doc->props_arrays, carbon_encoded_doc_prop_array_t);  \
    array->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;                                          \
    array->header.key.key_id = key;                                                                                    \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    carbon_vec_create(&array->values, NULL, sizeof(carbon_encoded_doc_value_t), 10);                                   \
    carbon_hashtable_insert_or_update(&doc->prop_array_index, &key, &new_array_pos, 1);                                \
    return true;                                                                                                       \
}

#define DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name, basic_type)                                       \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name##_decoded(carbon_encoded_doc_t *doc,                                          \
                                       const char *key)                                                                \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_array_t *array = VECTOR_NEW_AND_GET(&doc->props_arrays, carbon_encoded_doc_prop_array_t);  \
    array->header.key_type = CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;                                          \
    array->header.key.key_str = strdup(key);                                                                           \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    carbon_vec_create(&array->values, NULL, sizeof(carbon_encoded_doc_value_t), 10);                                   \
    return true;                                                                                                       \
}


DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8, CARBON_BASIC_TYPE_INT8)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16, CARBON_BASIC_TYPE_INT16)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32, CARBON_BASIC_TYPE_INT32)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64, CARBON_BASIC_TYPE_INT64)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8, CARBON_BASIC_TYPE_UINT8)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16, CARBON_BASIC_TYPE_UINT16)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32, CARBON_BASIC_TYPE_UINT32)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64, CARBON_BASIC_TYPE_UINT64)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number, CARBON_BASIC_TYPE_NUMBER)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean, CARBON_BASIC_TYPE_BOOLEAN)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string, CARBON_BASIC_TYPE_STRING)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null, CARBON_BASIC_TYPE_NULL)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object, CARBON_BASIC_TYPE_OBJECT)

DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8, CARBON_BASIC_TYPE_INT8)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16, CARBON_BASIC_TYPE_INT16)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32, CARBON_BASIC_TYPE_INT32)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64, CARBON_BASIC_TYPE_INT64)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8, CARBON_BASIC_TYPE_UINT8)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16, CARBON_BASIC_TYPE_UINT16)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32, CARBON_BASIC_TYPE_UINT32)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64, CARBON_BASIC_TYPE_UINT64)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number, CARBON_BASIC_TYPE_NUMBER)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean, CARBON_BASIC_TYPE_BOOLEAN)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string, CARBON_BASIC_TYPE_STRING)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null, CARBON_BASIC_TYPE_NULL)
DECALRE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object, CARBON_BASIC_TYPE_OBJECT)


#define DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type, basic_type)                                    \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name(carbon_encoded_doc_t *doc, carbon_string_id_t key,                                \
                                     const built_in_type *values, uint32_t values_length)                              \
{                                                                                                                      \
    CARBON_NON_NULL_OR_ERROR(doc)                                                                                      \
    const uint32_t *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);                               \
    CARBON_ERROR_IF(prop_pos == NULL, &doc->err, CARBON_ERR_NOTFOUND);                                                 \
    carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, *prop_pos,                          \
                                                               carbon_encoded_doc_prop_array_t);                       \
    CARBON_ERROR_IF(array == NULL, &doc->err, CARBON_ERR_INTERNALERR);                                                 \
    CARBON_ERROR_IF(array->header.type != basic_type, &doc->err, CARBON_ERR_TYPEMISMATCH);                             \
    for (uint32_t i = 0; i < values_length; i++) {                                                                     \
        carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

#define DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type, basic_type)                            \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name##_decoded(carbon_encoded_doc_t *doc, const char *key,                             \
                                     const built_in_type *values, uint32_t values_length)                              \
{                                                                                                                      \
    uint32_t prop_pos = (uint32_t) -1;                                                                                 \
    for (uint32_t i = 0; i < doc->props_arrays.num_elems; i++)                                                         \
    {                                                                                                                  \
        carbon_encoded_doc_prop_array_t *prop = CARBON_VECTOR_GET(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t); \
        if (prop->header.key_type == CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING) {                                \
            if (strcmp(prop->header.key.key_str, key) == 0) {                                                          \
                prop_pos = i;                                                                                          \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
    CARBON_ERROR_IF(prop_pos == (uint32_t) -1, &doc->err, CARBON_ERR_NOTFOUND);                                        \
    carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, prop_pos,                           \
                                                                   carbon_encoded_doc_prop_array_t);                   \
    CARBON_ERROR_IF(array == NULL, &doc->err, CARBON_ERR_INTERNALERR);                                                 \
    CARBON_ERROR_IF(array->header.type != basic_type, &doc->err, CARBON_ERR_TYPEMISMATCH);                             \
    for (uint32_t i = 0; i < values_length; i++) {                                                                     \
        carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, carbon_int8_t, CARBON_BASIC_TYPE_INT8)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, carbon_int16_t, CARBON_BASIC_TYPE_INT16)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, carbon_int32_t, CARBON_BASIC_TYPE_INT32)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, carbon_int64_t, CARBON_BASIC_TYPE_INT64)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, carbon_uint8_t, CARBON_BASIC_TYPE_UINT8)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, carbon_uint16_t, CARBON_BASIC_TYPE_UINT16)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, carbon_uint32_t, CARBON_BASIC_TYPE_UINT32)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, carbon_uint64_t, CARBON_BASIC_TYPE_UINT64)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(number, carbon_number_t, CARBON_BASIC_TYPE_NUMBER)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, carbon_boolean_t, CARBON_BASIC_TYPE_BOOLEAN)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(string, carbon_string_id_t, CARBON_BASIC_TYPE_STRING)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(null, carbon_uint32_t, CARBON_BASIC_TYPE_NULL)

DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, carbon_int8_t, CARBON_BASIC_TYPE_INT8)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, carbon_int16_t, CARBON_BASIC_TYPE_INT16)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, carbon_int32_t, CARBON_BASIC_TYPE_INT32)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, carbon_int64_t, CARBON_BASIC_TYPE_INT64)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, carbon_uint8_t, CARBON_BASIC_TYPE_UINT8)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, carbon_uint16_t, CARBON_BASIC_TYPE_UINT16)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, carbon_uint32_t, CARBON_BASIC_TYPE_UINT32)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, carbon_uint64_t, CARBON_BASIC_TYPE_UINT64)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, carbon_number_t, CARBON_BASIC_TYPE_NUMBER)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, carbon_boolean_t, CARBON_BASIC_TYPE_BOOLEAN)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, carbon_string_id_t, CARBON_BASIC_TYPE_STRING)
DECLARE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, carbon_uint32_t, CARBON_BASIC_TYPE_NULL)
//
//CARBON_EXPORT(bool)
//carbon_encoded_doc_array_push_null(carbon_encoded_doc_t *doc, carbon_string_id_t key, uint32_t how_many)
//{
//    CARBON_NON_NULL_OR_ERROR(doc)
//    const uint32_t *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);
//    CARBON_ERROR_IF(prop_pos == NULL, &doc->err, CARBON_ERR_NOTFOUND);
//    carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, *prop_pos,
//                                                               carbon_encoded_doc_prop_array_t);
//    CARBON_ERROR_IF(array == NULL, &doc->err, CARBON_ERR_INTERNALERR);
//    CARBON_ERROR_IF(array->header.type != CARBON_BASIC_TYPE_NULL, &doc->err, CARBON_ERR_TYPEMISMATCH);
//    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
//    value->num_nulls = how_many;
//    return true;
//}


#include <inttypes.h>
#include <carbon/carbon-query.h>

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_object_id_t id)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(id);

    CARBON_NON_NULL_OR_ERROR(doc)
    const uint32_t *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);
    CARBON_ERROR_IF(prop_pos == NULL, &doc->err, CARBON_ERR_NOTFOUND);
    carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, *prop_pos,
                                                               carbon_encoded_doc_prop_array_t);
    CARBON_ERROR_IF(array == NULL, &doc->err, CARBON_ERR_INTERNALERR);
    CARBON_ERROR_IF(array->header.type != CARBON_BASIC_TYPE_OBJECT, &doc->err, CARBON_ERR_TYPEMISMATCH);
    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
    value->object = id;
    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_object_id_t id)
{
    CARBON_UNUSED(doc);
    CARBON_UNUSED(key);
    CARBON_UNUSED(id);

    CARBON_NON_NULL_OR_ERROR(doc)
    uint32_t prop_pos = (uint32_t) -1;
    for (uint32_t i = 0; i < doc->props_arrays.num_elems; i++)
    {
        carbon_encoded_doc_prop_array_t *prop = CARBON_VECTOR_GET(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        if (prop->header.key_type == CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING) {
            if (strcmp(prop->header.key.key_str, key) == 0) {
                prop_pos = i;
                break;
            }
        }
    }
    CARBON_ERROR_IF(prop_pos == (uint32_t) -1, &doc->err, CARBON_ERR_NOTFOUND);
    carbon_encoded_doc_prop_array_t *array = CARBON_VECTOR_GET(&doc->props_arrays, prop_pos,
                                                               carbon_encoded_doc_prop_array_t);
    CARBON_ERROR_IF(array == NULL, &doc->err, CARBON_ERR_INTERNALERR);
    CARBON_ERROR_IF(array->header.type != CARBON_BASIC_TYPE_OBJECT, &doc->err, CARBON_ERR_TYPEMISMATCH);
    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
    value->object = id;
    return true;
}



CARBON_EXPORT(bool)
carbon_encoded_doc_get_nested_object(carbon_encoded_doc_t *nested, carbon_object_id_t oid, carbon_encoded_doc_t *doc)
{
    CARBON_UNUSED(nested);
    CARBON_UNUSED(oid);
    CARBON_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

static bool
doc_print_pretty(FILE *file, carbon_encoded_doc_t *doc, unsigned level)
{
    carbon_query_t query;
    carbon_archive_query(&query, doc->context->archive);

    fprintf(file, "{\n");

//    for (unsigned k = 0; k < level; k++) {
//        fprintf(file, "   ");
//    }

    //fprintf(file, "\"_id\": %" PRIu64 "%s\n", doc->object_id,
    //        doc->props.num_elems > 0 || doc->props_arrays.num_elems > 0 ? ", " : "" );

    for (uint32_t i = 0; i < doc->props.num_elems; i++) {
        carbon_encoded_doc_prop_t *prop = CARBON_VECTOR_GET(&doc->props, i, carbon_encoded_doc_prop_t);
        char *key_str = NULL;
        if (prop->header.key_type == CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING) {
            key_str  = carbon_query_fetch_string_by_id(&query, prop->header.key.key_id);
        } else {
            key_str = strdup(prop->header.key.key_str);
        }

        for (unsigned k = 0; k < level; k++) {
            fprintf(file, "   ");
        }

        fprintf(file, "\"%s\": ", key_str);
        switch (prop->header.type) {
        case CARBON_BASIC_TYPE_INT8:
            fprintf(file, "%" PRIi8, prop->value.builtin.int8);
            break;
        case CARBON_BASIC_TYPE_INT16:
            fprintf(file, "%" PRIi16, prop->value.builtin.int16);
            break;
        case CARBON_BASIC_TYPE_INT32:
            fprintf(file, "%" PRIi32, prop->value.builtin.int32);
            break;
        case CARBON_BASIC_TYPE_INT64:
            fprintf(file, "%" PRIi64, prop->value.builtin.int64);
            break;
        case CARBON_BASIC_TYPE_UINT8:
            fprintf(file, "%" PRIu8, prop->value.builtin.uint8);
            break;
        case CARBON_BASIC_TYPE_UINT16:
            fprintf(file, "%" PRIu16, prop->value.builtin.uint16);
            break;
        case CARBON_BASIC_TYPE_UINT32:
            fprintf(file, "%" PRIu32, prop->value.builtin.uint32);
            break;
        case CARBON_BASIC_TYPE_UINT64:
            fprintf(file, "%" PRIu64, prop->value.builtin.uint64);
            break;
        case CARBON_BASIC_TYPE_NUMBER:
            fprintf(file, "%.2f", ceilf(prop->value.builtin.number * 100) / 100);
            break;
        case CARBON_BASIC_TYPE_STRING: {
            if (prop->header.value_type == CARBON_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN) {
                char *value_str = carbon_query_fetch_string_by_id(&query, prop->value.builtin.string);
                fprintf(file, "\"%s\"", value_str);
                free(value_str);
            } else {
                fprintf(file, "\"%s\"", prop->value.string);
            }
        } break;
        case CARBON_BASIC_TYPE_BOOLEAN:
            fprintf(file, "\"%s\"", prop->value.builtin.boolean ? "true" : "false");
            break;
        case CARBON_BASIC_TYPE_NULL:
            fprintf(file, "null");
            break;
        case CARBON_BASIC_TYPE_OBJECT:
        {
            carbon_encoded_doc_t *nested = encoded_doc_collection_get_or_append(doc->context, prop->value.builtin.object);
            doc_print_pretty(file, nested, level + 1);
        } break;
        default:
        CARBON_ERROR(&doc->err, CARBON_ERR_INTERNALERR);
            return false;
        }
        free(key_str);
        fprintf(file, "%s\n", i + 1 < doc->props.num_elems || doc->props_arrays.num_elems > 0 ? ", " : "");
    }

    for (uint32_t i = 0; i < doc->props_arrays.num_elems; i++) {
        carbon_encoded_doc_prop_array_t *prop = CARBON_VECTOR_GET(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        char *key_str = NULL;
        if (prop->header.key_type == CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING) {
            key_str  = carbon_query_fetch_string_by_id(&query, prop->header.key.key_id);
        } else {
            key_str = strdup(prop->header.key.key_str);
        }

        for (unsigned k = 0; k < level; k++) {
            fprintf(file, "   ");
        }

        fprintf(file, "\"%s\": ", key_str);

        if (prop->values.num_elems == 0) {
            fprintf(file, "[ ]\n");
            continue;
        }

        if (prop->values.num_elems > 1) {
            fprintf(file, "[");
        }

        switch (prop->header.type) {
        case CARBON_BASIC_TYPE_INT8:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_int8_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->int8;
                if (CARBON_IS_NULL_INT8(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi8 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_INT16:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_int16_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->int16;
                if (CARBON_IS_NULL_INT16(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi16 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_INT32:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_int32_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->int32;
                if (CARBON_IS_NULL_INT32(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi32 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_INT64:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_int64_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->int64;
                if (CARBON_IS_NULL_INT64(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi64 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_UINT8:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_uint8_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->uint8;
                if (CARBON_IS_NULL_UINT8(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu8 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_UINT16:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_uint16_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->uint16;
                if (CARBON_IS_NULL_UINT16(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu16 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_UINT32:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_uint32_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->uint32;
                if (CARBON_IS_NULL_UINT32(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu32 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_UINT64:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_uint64_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->uint64;
                if (CARBON_IS_NULL_UINT64(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu64 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_NUMBER:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_number_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->number;
                if (CARBON_IS_NULL_NUMBER(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%.2f%s", ceilf(value * 100) / 100,
                            k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_STRING: {
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_string_id_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->string;
                if (CARBON_IS_NULL_STRING(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    char *value_str = carbon_query_fetch_string_by_id(&query, value);
                    fprintf(file, "\"%s\"%s", value_str, k + 1 < prop->values.num_elems ? ", " : "");
                    free(value_str);
                }
            }
        } break;
        case CARBON_BASIC_TYPE_BOOLEAN:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_boolean_t value = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->boolean;
                if (CARBON_IS_NULL_BOOLEAN(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%s%s", value ? "true" : "false",
                            k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case CARBON_BASIC_TYPE_NULL:
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
            }
            break;
        case CARBON_BASIC_TYPE_OBJECT:
        {
            for (uint32_t k = 0; k < prop->values.num_elems; k++) {
                carbon_object_id_t nested_oid = (CARBON_VECTOR_GET(&prop->values, k, carbon_encoded_doc_value_t))->object;
                carbon_encoded_doc_t *nested_doc = encoded_doc_collection_get_or_append(doc->context, nested_oid);
                fprintf(file, "\n");
                for (unsigned k = 0; k < level + 1; k++) {
                    fprintf(file, "   ");
                }
                doc_print_pretty(file, nested_doc, level + 2);
                fprintf(file, "%s", k + 1 < prop->values.num_elems ? "," : "");
            }
            fprintf(file, "\n");
            for (unsigned k = 0; k < level; k++) {
                fprintf(file, "   ");
            }
        } break;
        default:
        CARBON_ERROR(&doc->err, CARBON_ERR_INTERNALERR);
            return false;
        }
        free(key_str);
        if (prop->values.num_elems > 1) {
            fprintf(file, "]");
        }
        fprintf(file, "%s\n", i + 1 < doc->props_arrays.num_elems ? ", " : "");
    }

    for (unsigned k = 0; k < level - 1; k++) {
        fprintf(file, "   ");
    }

    fprintf(file, "}");

    carbon_query_drop(&query);

    return true;
}

CARBON_EXPORT(bool)
carbon_encoded_doc_print(FILE *file, carbon_encoded_doc_t *doc)
{
    return doc_print_pretty(file, doc, 1);
}

