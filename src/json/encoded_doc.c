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

#include "json/encoded_doc.h"

NG5_EXPORT(bool)
carbon_encoded_doc_collection_create(carbon_encoded_doc_collection_t *collection, struct err *err,
                                     carbon_archive_t *archive)
{
    NG5_UNUSED(collection);
    NG5_UNUSED(err);
    NG5_UNUSED(archive);

    carbon_vec_create(&collection->flat_object_collection, NULL, sizeof(carbon_encoded_doc_t), 5000000);
    carbon_hashtable_create(&collection->index, err, sizeof(carbon_object_id_t), sizeof(u32), 5000000);
    carbon_error_init(&collection->err);
    collection->archive = archive;

    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_collection_drop(carbon_encoded_doc_collection_t *collection)
{
    NG5_UNUSED(collection);

    carbon_hashtable_drop(&collection->index);
    for (u32 i = 0; i < collection->flat_object_collection.num_elems; i++) {
        carbon_encoded_doc_t *doc = vec_get(&collection->flat_object_collection, i, carbon_encoded_doc_t);
        carbon_encoded_doc_drop(doc);
    }
    carbon_vec_drop(&collection->flat_object_collection);
    return true;
}

static carbon_encoded_doc_t *
doc_create(struct err *err, carbon_object_id_t object_id,
           carbon_encoded_doc_collection_t *collection)
{
    if (collection) {
        u32 doc_position = collection->flat_object_collection.num_elems;
        carbon_encoded_doc_t *new_doc = VECTOR_NEW_AND_GET(&collection->flat_object_collection, carbon_encoded_doc_t);
        new_doc->context = collection;
        new_doc->object_id = object_id;
        carbon_vec_create(&new_doc->props, NULL, sizeof(carbon_encoded_doc_prop_t), 20);
        carbon_vec_create(&new_doc->props_arrays, NULL, sizeof(carbon_encoded_doc_prop_array_t), 20);
        carbon_hashtable_create(&new_doc->prop_array_index, err, sizeof(carbon_string_id_t), sizeof(u32), 20);
        carbon_error_init(&new_doc->err);
        carbon_hashtable_insert_or_update(&collection->index, &object_id, &doc_position, 1);
        return new_doc;
    } else {
        error(err, NG5_ERR_ILLEGALARG);
        return NULL;
    }
}

NG5_EXPORT(carbon_encoded_doc_t *)
encoded_doc_collection_get_or_append(carbon_encoded_doc_collection_t *collection, carbon_object_id_t id)
{
    NG5_NON_NULL_OR_ERROR(collection);
    const u32 *doc_pos = carbon_hashtable_get_value(&collection->index, &id);
    if (doc_pos)
    {
        carbon_encoded_doc_t *result = vec_get(&collection->flat_object_collection, *doc_pos, carbon_encoded_doc_t);
        error_IF(result == NULL, &collection->err, NG5_ERR_INTERNALERR);
        return result;
    } else
    {
        carbon_encoded_doc_t *result = doc_create(&collection->err, id, collection);
        if (!result) {
            error(&collection->err, NG5_ERR_INTERNALERR);
        }
        return result;
    }
}

NG5_EXPORT(bool)
carbon_encoded_doc_collection_print(FILE *file, carbon_encoded_doc_collection_t *collection)
{
    NG5_UNUSED(file);
    NG5_UNUSED(collection);

    if (collection->flat_object_collection.num_elems > 0) {
        carbon_encoded_doc_t *root = vec_get(&collection->flat_object_collection, 0, carbon_encoded_doc_t);
        carbon_encoded_doc_print(file, root);
    }

    return false;
}

NG5_EXPORT(bool)
carbon_encoded_doc_drop(carbon_encoded_doc_t *doc)
{
    NG5_UNUSED(doc);
    for (u32 i = 0; i < doc->props_arrays.num_elems; i++)
    {
        carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        carbon_vec_drop(&array->values);
    }
    for (u32 i = 0; i < doc->props.num_elems; i++)
    {
        carbon_encoded_doc_prop_t *single = vec_get(&doc->props, i, carbon_encoded_doc_prop_t);
        if (single->header.value_type == NG5_ENCODED_DOC_PROP_VALUE_TYPE_DECODED_STRING) {
            free(single->value.string);
        }
    }
    carbon_vec_drop(&doc->props);
    carbon_vec_drop(&doc->props_arrays);
    carbon_hashtable_drop(&doc->prop_array_index);
    return false;
}

NG5_EXPORT(bool)
carbon_encoded_doc_get_object_id(carbon_object_id_t *oid, carbon_encoded_doc_t *doc)
{
    NG5_UNUSED(oid);
    NG5_UNUSED(doc);
    abort(); // TODO: implement
    return false;
}

#define DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(built_in_type, basic_type, value_name)                               \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##value_name(carbon_encoded_doc_t *doc, carbon_string_id_t key, built_in_type value)       \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;                                        \
    prop->header.key.key_id = key;                                                                                     \
    prop->header.value_type = NG5_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

#define DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(built_in_type, basic_type, value_name)                       \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##value_name##_decoded(carbon_encoded_doc_t *doc, const char *key, built_in_type value)    \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;                                        \
    prop->header.key.key_str = strdup(key);                                                                            \
    prop->header.value_type = NG5_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}


DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_i8, NG5_BASIC_TYPE_INT8, int8)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_i16, NG5_BASIC_TYPE_INT16, int16)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_i32, NG5_BASIC_TYPE_INT32, int32)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_i64, NG5_BASIC_TYPE_INT64, int64)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_u8, NG5_BASIC_TYPE_UINT8, uint8)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_u16, NG5_BASIC_TYPE_UINT16, uint16)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_u32, NG5_BASIC_TYPE_UINT32, uint32)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_u64, NG5_BASIC_TYPE_UINT64, uint64)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_number_t, NG5_BASIC_TYPE_NUMBER, number)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_boolean_t, NG5_BASIC_TYPE_BOOLEAN, boolean)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC(carbon_string_id_t, NG5_BASIC_TYPE_STRING, string)

DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_i8, NG5_BASIC_TYPE_INT8, int8)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_i16, NG5_BASIC_TYPE_INT16, int16)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_i32, NG5_BASIC_TYPE_INT32, int32)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_i64, NG5_BASIC_TYPE_INT64, int64)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_u8, NG5_BASIC_TYPE_UINT8, uint8)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_u16, NG5_BASIC_TYPE_UINT16, uint16)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_u32, NG5_BASIC_TYPE_UINT32, uint32)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_u64, NG5_BASIC_TYPE_UINT64, uint64)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_number_t, NG5_BASIC_TYPE_NUMBER, number)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_boolean_t, NG5_BASIC_TYPE_BOOLEAN, boolean)
DECLARE_NG5_ENCODED_DOC_ADD_PROP_BASIC_DECODED(carbon_string_id_t, NG5_BASIC_TYPE_STRING, string)

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(carbon_encoded_doc_t *doc, const char *key, const char *value)
{
    NG5_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = NG5_BASIC_TYPE_STRING;
    prop->value.string = strdup(value);
    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_null(carbon_encoded_doc_t *doc, carbon_string_id_t key)
{
    NG5_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;
    prop->header.key.key_id = key;
    prop->header.type = NG5_BASIC_TYPE_NULL;
    prop->value.builtin.null = 1;
    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_null_decoded(carbon_encoded_doc_t *doc, const char *key)
{
    NG5_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = NG5_BASIC_TYPE_NULL;
    prop->value.builtin.null = 1;
    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_encoded_doc_t *value)
{
    NG5_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;
    prop->header.key.key_id = key;
    prop->header.type = NG5_BASIC_TYPE_OBJECT;
    prop->value.builtin.object = value->object_id;
    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_add_prop_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_encoded_doc_t *value)
{
    NG5_NON_NULL_OR_ERROR(doc)
    carbon_encoded_doc_prop_t *prop = VECTOR_NEW_AND_GET(&doc->props, carbon_encoded_doc_prop_t);
    prop->header.context = doc;
    prop->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;
    prop->header.key.key_str = strdup(key);
    prop->header.type = NG5_BASIC_TYPE_OBJECT;
    prop->value.builtin.object = value->object_id;
    return true;
}

#define DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name, basic_type)                                               \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name(carbon_encoded_doc_t *doc,                                                    \
                                       carbon_string_id_t key)                                                         \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(doc)                                                                                      \
    u32 new_array_pos = doc->props_arrays.num_elems;                                                              \
    carbon_encoded_doc_prop_array_t *array = VECTOR_NEW_AND_GET(&doc->props_arrays, carbon_encoded_doc_prop_array_t);  \
    array->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING;                                          \
    array->header.key.key_id = key;                                                                                    \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    carbon_vec_create(&array->values, NULL, sizeof(carbon_encoded_doc_value_t), 10);                                   \
    carbon_hashtable_insert_or_update(&doc->prop_array_index, &key, &new_array_pos, 1);                                \
    return true;                                                                                                       \
}

#define DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name, basic_type)                                       \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name##_decoded(carbon_encoded_doc_t *doc,                                          \
                                       const char *key)                                                                \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(doc)                                                                                      \
    carbon_encoded_doc_prop_array_t *array = VECTOR_NEW_AND_GET(&doc->props_arrays, carbon_encoded_doc_prop_array_t);  \
    array->header.key_type = NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING;                                          \
    array->header.key.key_str = strdup(key);                                                                           \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    carbon_vec_create(&array->values, NULL, sizeof(carbon_encoded_doc_value_t), 10);                                   \
    return true;                                                                                                       \
}


DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8, NG5_BASIC_TYPE_INT8)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16, NG5_BASIC_TYPE_INT16)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32, NG5_BASIC_TYPE_INT32)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64, NG5_BASIC_TYPE_INT64)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8, NG5_BASIC_TYPE_UINT8)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16, NG5_BASIC_TYPE_UINT16)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32, NG5_BASIC_TYPE_UINT32)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64, NG5_BASIC_TYPE_UINT64)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number, NG5_BASIC_TYPE_NUMBER)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean, NG5_BASIC_TYPE_BOOLEAN)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string, NG5_BASIC_TYPE_STRING)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null, NG5_BASIC_TYPE_NULL)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object, NG5_BASIC_TYPE_OBJECT)

DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8, NG5_BASIC_TYPE_INT8)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16, NG5_BASIC_TYPE_INT16)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32, NG5_BASIC_TYPE_INT32)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64, NG5_BASIC_TYPE_INT64)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8, NG5_BASIC_TYPE_UINT8)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16, NG5_BASIC_TYPE_UINT16)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32, NG5_BASIC_TYPE_UINT32)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64, NG5_BASIC_TYPE_UINT64)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number, NG5_BASIC_TYPE_NUMBER)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean, NG5_BASIC_TYPE_BOOLEAN)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string, NG5_BASIC_TYPE_STRING)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null, NG5_BASIC_TYPE_NULL)
DECALRE_NG5_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object, NG5_BASIC_TYPE_OBJECT)


#define DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type, basic_type)                                    \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name(carbon_encoded_doc_t *doc, carbon_string_id_t key,                                \
                                     const built_in_type *values, u32 values_length)                              \
{                                                                                                                      \
    NG5_NON_NULL_OR_ERROR(doc)                                                                                      \
    const u32 *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);                               \
    error_IF(prop_pos == NULL, &doc->err, NG5_ERR_NOTFOUND);                                                 \
    carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, *prop_pos,                          \
                                                               carbon_encoded_doc_prop_array_t);                       \
    error_IF(array == NULL, &doc->err, NG5_ERR_INTERNALERR);                                                 \
    error_IF(array->header.type != basic_type, &doc->err, NG5_ERR_TYPEMISMATCH);                             \
    for (u32 i = 0; i < values_length; i++) {                                                                     \
        carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

#include <inttypes.h>

#define DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type, basic_type)                            \
NG5_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name##_decoded(carbon_encoded_doc_t *doc, const char *key,                             \
                                     const built_in_type *values, u32 values_length)                              \
{                                                                                                                      \
    u32 prop_pos = (u32) -1;                                                                                 \
    for (u32 i = 0; i < doc->props_arrays.num_elems; i++)                                                         \
    {                                                                                                                  \
        carbon_encoded_doc_prop_array_t *prop = vec_get(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t); \
        if (prop->header.key_type == NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING) {                                \
            if (strcmp(prop->header.key.key_str, key) == 0) {                                                          \
                prop_pos = i;                                                                                          \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
    error_IF(prop_pos == (u32) -1, &doc->err, NG5_ERR_NOTFOUND);                                        \
    carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, prop_pos,                           \
                                                                   carbon_encoded_doc_prop_array_t);                   \
    error_IF(array == NULL, &doc->err, NG5_ERR_INTERNALERR);                                                 \
    error_IF(array->header.type != basic_type, &doc->err, NG5_ERR_TYPEMISMATCH);                             \
    for (u32 i = 0; i < values_length; i++) {                                                                     \
        carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, carbon_i8, NG5_BASIC_TYPE_INT8)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, carbon_i16, NG5_BASIC_TYPE_INT16)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, carbon_i32, NG5_BASIC_TYPE_INT32)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, carbon_i64, NG5_BASIC_TYPE_INT64)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, carbon_u8, NG5_BASIC_TYPE_UINT8)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, carbon_u16, NG5_BASIC_TYPE_UINT16)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, carbon_u32, NG5_BASIC_TYPE_UINT32)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, carbon_u64, NG5_BASIC_TYPE_UINT64)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(number, carbon_number_t, NG5_BASIC_TYPE_NUMBER)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, carbon_boolean_t, NG5_BASIC_TYPE_BOOLEAN)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(string, carbon_string_id_t, NG5_BASIC_TYPE_STRING)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE(null, carbon_u32, NG5_BASIC_TYPE_NULL)

DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, carbon_i8, NG5_BASIC_TYPE_INT8)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, carbon_i16, NG5_BASIC_TYPE_INT16)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, carbon_i32, NG5_BASIC_TYPE_INT32)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, carbon_i64, NG5_BASIC_TYPE_INT64)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, carbon_u8, NG5_BASIC_TYPE_UINT8)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, carbon_u16, NG5_BASIC_TYPE_UINT16)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, carbon_u32, NG5_BASIC_TYPE_UINT32)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, carbon_u64, NG5_BASIC_TYPE_UINT64)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, carbon_number_t, NG5_BASIC_TYPE_NUMBER)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, carbon_boolean_t, NG5_BASIC_TYPE_BOOLEAN)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, carbon_string_id_t, NG5_BASIC_TYPE_STRING)
DECLARE_NG5_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, carbon_u32, NG5_BASIC_TYPE_NULL)
//
//NG5_EXPORT(bool)
//carbon_encoded_doc_array_push_null(carbon_encoded_doc_t *doc, carbon_string_id_t key, u32 how_many)
//{
//    NG5_NON_NULL_OR_ERROR(doc)
//    const u32 *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);
//    error_IF(prop_pos == NULL, &doc->err, NG5_ERR_NOTFOUND);
//    carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, *prop_pos,
//                                                               carbon_encoded_doc_prop_array_t);
//    error_IF(array == NULL, &doc->err, NG5_ERR_INTERNALERR);
//    error_IF(array->header.type != NG5_BASIC_TYPE_NULL, &doc->err, NG5_ERR_TYPEMISMATCH);
//    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
//    value->num_nulls = how_many;
//    return true;
//}


#include <inttypes.h>
#include "core/carbon/archive_query.h"

NG5_EXPORT(bool)
carbon_encoded_doc_array_push_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_object_id_t id)
{
    NG5_UNUSED(doc);
    NG5_UNUSED(key);
    NG5_UNUSED(id);

    NG5_NON_NULL_OR_ERROR(doc)
    const u32 *prop_pos = carbon_hashtable_get_value(&doc->prop_array_index, &key);
    error_IF(prop_pos == NULL, &doc->err, NG5_ERR_NOTFOUND);
    carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, *prop_pos,
                                                               carbon_encoded_doc_prop_array_t);
    error_IF(array == NULL, &doc->err, NG5_ERR_INTERNALERR);
    error_IF(array->header.type != NG5_BASIC_TYPE_OBJECT, &doc->err, NG5_ERR_TYPEMISMATCH);
    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
    value->object = id;
    return true;
}

NG5_EXPORT(bool)
carbon_encoded_doc_array_push_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_object_id_t id)
{
    NG5_UNUSED(doc);
    NG5_UNUSED(key);
    NG5_UNUSED(id);

    NG5_NON_NULL_OR_ERROR(doc)
    u32 prop_pos = (u32) -1;
    for (u32 i = 0; i < doc->props_arrays.num_elems; i++)
    {
        carbon_encoded_doc_prop_array_t *prop = vec_get(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        if (prop->header.key_type == NG5_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING) {
            if (strcmp(prop->header.key.key_str, key) == 0) {
                prop_pos = i;
                break;
            }
        }
    }
    error_IF(prop_pos == (u32) -1, &doc->err, NG5_ERR_NOTFOUND);
    carbon_encoded_doc_prop_array_t *array = vec_get(&doc->props_arrays, prop_pos,
                                                               carbon_encoded_doc_prop_array_t);
    error_IF(array == NULL, &doc->err, NG5_ERR_INTERNALERR);
    error_IF(array->header.type != NG5_BASIC_TYPE_OBJECT, &doc->err, NG5_ERR_TYPEMISMATCH);
    carbon_encoded_doc_value_t *value = VECTOR_NEW_AND_GET(&array->values, carbon_encoded_doc_value_t);
    value->object = id;
    return true;
}



NG5_EXPORT(bool)
carbon_encoded_doc_get_nested_object(carbon_encoded_doc_t *nested, carbon_object_id_t oid, carbon_encoded_doc_t *doc)
{
    NG5_UNUSED(nested);
    NG5_UNUSED(oid);
    NG5_UNUSED(doc);
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

    for (u32 i = 0; i < doc->props.num_elems; i++) {
        carbon_encoded_doc_prop_t *prop = vec_get(&doc->props, i, carbon_encoded_doc_prop_t);
        char *key_str = NULL;
        if (prop->header.key_type == NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING) {
            key_str  = carbon_query_fetch_string_by_id(&query, prop->header.key.key_id);
        } else {
            key_str = strdup(prop->header.key.key_str);
        }

        for (unsigned k = 0; k < level; k++) {
            fprintf(file, "   ");
        }

        fprintf(file, "\"%s\": ", key_str);
        switch (prop->header.type) {
        case NG5_BASIC_TYPE_INT8:
            fprintf(file, "%" PRIi8, prop->value.builtin.int8);
            break;
        case NG5_BASIC_TYPE_INT16:
            fprintf(file, "%" PRIi16, prop->value.builtin.int16);
            break;
        case NG5_BASIC_TYPE_INT32:
            fprintf(file, "%" PRIi32, prop->value.builtin.int32);
            break;
        case NG5_BASIC_TYPE_INT64:
            fprintf(file, "%" PRIi64, prop->value.builtin.int64);
            break;
        case NG5_BASIC_TYPE_UINT8:
            fprintf(file, "%" PRIu8, prop->value.builtin.uint8);
            break;
        case NG5_BASIC_TYPE_UINT16:
            fprintf(file, "%" PRIu16, prop->value.builtin.uint16);
            break;
        case NG5_BASIC_TYPE_UINT32:
            fprintf(file, "%" PRIu32, prop->value.builtin.uint32);
            break;
        case NG5_BASIC_TYPE_UINT64:
            fprintf(file, "%" PRIu64, prop->value.builtin.uint64);
            break;
        case NG5_BASIC_TYPE_NUMBER:
            fprintf(file, "%.2f", ceilf(prop->value.builtin.number * 100) / 100);
            break;
        case NG5_BASIC_TYPE_STRING: {
            if (prop->header.value_type == NG5_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN) {
                char *value_str = carbon_query_fetch_string_by_id(&query, prop->value.builtin.string);
                fprintf(file, "\"%s\"", value_str);
                free(value_str);
            } else {
                fprintf(file, "\"%s\"", prop->value.string);
            }
        } break;
        case NG5_BASIC_TYPE_BOOLEAN:
            fprintf(file, "\"%s\"", prop->value.builtin.boolean ? "true" : "false");
            break;
        case NG5_BASIC_TYPE_NULL:
            fprintf(file, "null");
            break;
        case NG5_BASIC_TYPE_OBJECT:
        {
            carbon_encoded_doc_t *nested = encoded_doc_collection_get_or_append(doc->context, prop->value.builtin.object);
            doc_print_pretty(file, nested, level + 1);
        } break;
        default:
        error(&doc->err, NG5_ERR_INTERNALERR);
            return false;
        }
        free(key_str);
        fprintf(file, "%s\n", i + 1 < doc->props.num_elems || doc->props_arrays.num_elems > 0 ? ", " : "");
    }

    for (u32 i = 0; i < doc->props_arrays.num_elems; i++) {
        carbon_encoded_doc_prop_array_t *prop = vec_get(&doc->props_arrays, i, carbon_encoded_doc_prop_array_t);
        char *key_str = NULL;
        if (prop->header.key_type == NG5_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING) {
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
        case NG5_BASIC_TYPE_INT8:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_i8 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->int8;
                if (NG5_IS_NULL_INT8(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi8 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_INT16:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_i16 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->int16;
                if (NG5_IS_NULL_INT16(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi16 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_INT32:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_i32 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->int32;
                if (NG5_IS_NULL_INT32(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi32 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_INT64:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_i64 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->int64;
                if (NG5_IS_NULL_INT64(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIi64 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_UINT8:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_u8 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->uint8;
                if (NG5_IS_NULL_UINT8(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu8 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_UINT16:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_u16 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->uint16;
                if (NG5_IS_NULL_UINT16(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu16 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_UINT32:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_u32 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->uint32;
                if (NG5_IS_NULL_UINT32(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu32 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_UINT64:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_u64 value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->uint64;
                if (NG5_IS_NULL_UINT64(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%" PRIu64 "%s", value, k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_NUMBER:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_number_t value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->number;
                if (NG5_IS_NULL_NUMBER(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%.2f%s", ceilf(value * 100) / 100,
                            k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_STRING: {
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_string_id_t value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->string;
                if (NG5_IS_NULL_STRING(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    char *value_str = carbon_query_fetch_string_by_id(&query, value);
                    fprintf(file, "\"%s\"%s", value_str, k + 1 < prop->values.num_elems ? ", " : "");
                    free(value_str);
                }
            }
        } break;
        case NG5_BASIC_TYPE_BOOLEAN:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_boolean_t value = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->boolean;
                if (NG5_IS_NULL_BOOLEAN(value)) {
                    fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                } else {
                    fprintf(file, "%s%s", value ? "true" : "false",
                            k + 1 < prop->values.num_elems ? ", " : "");
                }
            }
            break;
        case NG5_BASIC_TYPE_NULL:
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
            }
            break;
        case NG5_BASIC_TYPE_OBJECT:
        {
            for (u32 k = 0; k < prop->values.num_elems; k++) {
                carbon_object_id_t nested_oid = (vec_get(&prop->values, k, carbon_encoded_doc_value_t))->object;
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
        error(&doc->err, NG5_ERR_INTERNALERR);
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

NG5_EXPORT(bool)
carbon_encoded_doc_print(FILE *file, carbon_encoded_doc_t *doc)
{
    return doc_print_pretty(file, doc, 1);
}

