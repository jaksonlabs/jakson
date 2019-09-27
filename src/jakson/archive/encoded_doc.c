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

#include <jakson/archive/encoded_doc.h>

bool encoded_doc_collection_create(encoded_doc_list *collection, err *err,
                                   archive *archive)
{
        UNUSED(collection);
        UNUSED(err);
        UNUSED(archive);

        vector_create(&collection->flat_object_collection, NULL, sizeof(encoded_doc), 5000000);
        hashtable_create(&collection->index, err, sizeof(unique_id_t), sizeof(u32), 5000000);
        error_init(&collection->err);
        collection->archive = archive;

        return true;
}

bool encoded_doc_collection_drop(encoded_doc_list *collection)
{
        UNUSED(collection);

        hashtable_drop(&collection->index);
        for (u32 i = 0; i < collection->flat_object_collection.num_elems; i++) {
                encoded_doc *doc = VECTOR_GET(&collection->flat_object_collection, i, encoded_doc);
                encoded_doc_drop(doc);
        }
        vector_drop(&collection->flat_object_collection);
        return true;
}

static encoded_doc *
doc_create(err *err, unique_id_t object_id, encoded_doc_list *collection)
{
        if (collection) {
                u32 doc_position = collection->flat_object_collection.num_elems;
                encoded_doc
                        *new_doc = VECTOR_NEW_AND_GET(&collection->flat_object_collection, encoded_doc);
                new_doc->context = collection;
                new_doc->object_id = object_id;
                vector_create(&new_doc->props, NULL, sizeof(encoded_doc_prop), 20);
                vector_create(&new_doc->props_arrays, NULL, sizeof(encoded_doc_prop_array), 20);
                hashtable_create(&new_doc->prop_array_index, err, sizeof(archive_field_sid_t), sizeof(u32), 20);
                error_init(&new_doc->err);
                hashtable_insert_or_update(&collection->index, &object_id, &doc_position, 1);
                return new_doc;
        } else {
                ERROR(err, ERR_ILLEGALARG);
                return NULL;
        }
}

encoded_doc *encoded_doc_collection_get_or_append(encoded_doc_list *collection,
                                                             unique_id_t id)
{
        ERROR_IF_NULL(collection);
        const u32 *doc_pos = hashtable_get_value(&collection->index, &id);
        if (doc_pos) {
                encoded_doc *result = VECTOR_GET(&collection->flat_object_collection, *doc_pos,
                                                         encoded_doc);
                ERROR_IF(result == NULL, &collection->err, ERR_INTERNALERR);
                return result;
        } else {
                encoded_doc *result = doc_create(&collection->err, id, collection);
                if (!result) {
                        ERROR(&collection->err, ERR_INTERNALERR);
                }
                return result;
        }
}

bool encoded_doc_collection_print(FILE *file, encoded_doc_list *collection)
{
        UNUSED(file);
        UNUSED(collection);

        if (collection->flat_object_collection.num_elems > 0) {
                encoded_doc *root = VECTOR_GET(&collection->flat_object_collection, 0, encoded_doc);
                encoded_doc_print(file, root);
        }

        return false;
}

bool encoded_doc_drop(encoded_doc *doc)
{
        UNUSED(doc);
        for (u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, i,
                                                                   encoded_doc_prop_array);
                vector_drop(&array->values);
        }
        for (u32 i = 0; i < doc->props.num_elems; i++) {
                encoded_doc_prop *single = VECTOR_GET(&doc->props, i, encoded_doc_prop);
                if (single->header.value_type == VALUE_DECODED_STRING) {
                        free(single->value.string);
                }
        }
        vector_drop(&doc->props);
        vector_drop(&doc->props_arrays);
        hashtable_drop(&doc->prop_array_index);
        return false;
}

#define DECLARE_ENCODED_DOC_ADD_PROP_BASIC(built_in_type, basic_type, value_name)                               \
bool                                                                                                    \
encoded_doc_add_prop_##value_name(encoded_doc *doc, archive_field_sid_t key, built_in_type value)       \
{                                                                                                                      \
    ERROR_IF_NULL(doc)                                                                                      \
    encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = STRING_ENCODED;                                        \
    prop->header.key.key_id = key;                                                                                     \
    prop->header.value_type = VALUE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

#define DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(built_in_type, basic_type, value_name)                       \
bool                                                                                                    \
encoded_doc_add_prop_##value_name##_decoded(encoded_doc *doc, const char *key, built_in_type value)    \
{                                                                                                                      \
    ERROR_IF_NULL(doc)                                                                                      \
    encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = STRING_DECODED;                                        \
    prop->header.key.key_str = strdup(key);                                                                            \
    prop->header.value_type = VALUE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_i8_t, FIELD_INT8, int8)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_i16_t, FIELD_INT16, int16)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_i32_t, FIELD_INT32, int32)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_i64_t, FIELD_INT64, int64)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_u8_t, FIELD_UINT8, uint8)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_u16_t, FIELD_UINT16, uint16)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_u32_t, FIELD_UINT32, uint32)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_u64_t, FIELD_UINT64, uint64)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_number_t, FIELD_FLOAT, number)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_boolean_t, FIELD_BOOLEAN, boolean)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC(archive_field_sid_t, FIELD_STRING, string)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_i8_t, FIELD_INT8, int8)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_i16_t, FIELD_INT16, int16)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_i32_t, FIELD_INT32, int32)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_i64_t, FIELD_INT64, int64)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_u8_t, FIELD_UINT8, uint8)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_u16_t, FIELD_UINT16, uint16)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_u32_t, FIELD_UINT32, uint32)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_u64_t, FIELD_UINT64, uint64)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_number_t, FIELD_FLOAT, number)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_boolean_t, FIELD_BOOLEAN, boolean)

DECLARE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(archive_field_sid_t, FIELD_STRING, string)

bool encoded_doc_add_prop_string_decoded_string_value_decoded(encoded_doc *doc, const char *key,
                                                              const char *value)
{
        ERROR_IF_NULL(doc)
        encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = FIELD_STRING;
        prop->value.string = strdup(value);
        return true;
}

bool encoded_doc_add_prop_null(encoded_doc *doc, archive_field_sid_t key)
{
        ERROR_IF_NULL(doc)
        encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_ENCODED;
        prop->header.key.key_id = key;
        prop->header.type = FIELD_NULL;
        prop->value.builtin.null = 1;
        return true;
}

bool encoded_doc_add_prop_null_decoded(encoded_doc *doc, const char *key)
{
        ERROR_IF_NULL(doc)
        encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = FIELD_NULL;
        prop->value.builtin.null = 1;
        return true;
}

bool
encoded_doc_add_prop_object(encoded_doc *doc, archive_field_sid_t key, encoded_doc *value)
{
        ERROR_IF_NULL(doc)
        encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_ENCODED;
        prop->header.key.key_id = key;
        prop->header.type = FIELD_OBJECT;
        prop->value.builtin.object = value->object_id;
        return true;
}

bool encoded_doc_add_prop_object_decoded(encoded_doc *doc, const char *key,
                                         encoded_doc *value)
{
        ERROR_IF_NULL(doc)
        encoded_doc_prop *prop = VECTOR_NEW_AND_GET(&doc->props, encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = FIELD_OBJECT;
        prop->value.builtin.object = value->object_id;
        return true;
}

#define DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name, basic_type)                                               \
bool                                                                                                    \
encoded_doc_add_prop_array_##name(encoded_doc *doc,                                                    \
                                       archive_field_sid_t key)                                                         \
{                                                                                                                      \
    ERROR_IF_NULL(doc)                                                                                      \
    u32 new_array_pos = doc->props_arrays.num_elems;                                                              \
    encoded_doc_prop_array *array = VECTOR_NEW_AND_GET(&doc->props_arrays, encoded_doc_prop_array);  \
    array->header.key_type = STRING_ENCODED;                                          \
    array->header.key.key_id = key;                                                                                    \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    vector_create(&array->values, NULL, sizeof(encoded_doc_value_u), 10);                                   \
    hashtable_insert_or_update(&doc->prop_array_index, &key, &new_array_pos, 1);                                \
    return true;                                                                                                       \
}

#define DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name, basic_type)                                       \
bool                                                                                                    \
encoded_doc_add_prop_array_##name##_decoded(encoded_doc *doc,                                          \
                                       const char *key)                                                                \
{                                                                                                                      \
    ERROR_IF_NULL(doc)                                                                                      \
    encoded_doc_prop_array *array = VECTOR_NEW_AND_GET(&doc->props_arrays, encoded_doc_prop_array);  \
    array->header.key_type = STRING_DECODED;                                          \
    array->header.key.key_str = strdup(key);                                                                           \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    vector_create(&array->values, NULL, sizeof(encoded_doc_value_u), 10);                                   \
    return true;                                                                                                       \
}

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8, FIELD_INT8)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16, FIELD_INT16)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32, FIELD_INT32)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64, FIELD_INT64)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8, FIELD_UINT8)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16, FIELD_UINT16)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32, FIELD_UINT32)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64, FIELD_UINT64)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number, FIELD_FLOAT)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean, FIELD_BOOLEAN)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string, FIELD_STRING)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null, FIELD_NULL)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object, FIELD_OBJECT)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8, FIELD_INT8)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16, FIELD_INT16)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32, FIELD_INT32)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64, FIELD_INT64)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8, FIELD_UINT8)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16, FIELD_UINT16)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32, FIELD_UINT32)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64, FIELD_UINT64)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number, FIELD_FLOAT)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean, FIELD_BOOLEAN)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string, FIELD_STRING)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null, FIELD_NULL)

DECALRE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object, FIELD_OBJECT)

#define DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type, basic_type)                                    \
bool                                                                                                    \
encoded_doc_array_push_##name(encoded_doc *doc, archive_field_sid_t key,                                \
                                     const built_in_type *values, u32 values_length)                              \
{                                                                                                                      \
    ERROR_IF_NULL(doc)                                                                                      \
    const u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);                               \
    ERROR_IF(prop_pos == NULL, &doc->err, ERR_NOTFOUND);                                                 \
    encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, *prop_pos,                          \
                                                               encoded_doc_prop_array);                       \
    ERROR_IF(array == NULL, &doc->err, ERR_INTERNALERR);                                                 \
    ERROR_IF(array->header.type != basic_type, &doc->err, ERR_TYPEMISMATCH);                             \
    for (u32 i = 0; i < values_length; i++) {                                                                     \
        encoded_doc_value_u *value = VECTOR_NEW_AND_GET(&array->values, encoded_doc_value_u);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

#include <inttypes.h>

#define DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type, basic_type)                            \
bool                                                                                                    \
encoded_doc_array_push_##name##_decoded(encoded_doc *doc, const char *key,                             \
                                     const built_in_type *values, u32 values_length)                              \
{                                                                                                                      \
    u32 prop_pos = (u32) -1;                                                                                 \
    for (u32 i = 0; i < doc->props_arrays.num_elems; i++)                                                         \
    {                                                                                                                  \
        encoded_doc_prop_array *prop = VECTOR_GET(&doc->props_arrays, i, encoded_doc_prop_array); \
        if (prop->header.key_type == STRING_DECODED) {                                \
            if (strcmp(prop->header.key.key_str, key) == 0) {                                                          \
                prop_pos = i;                                                                                          \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
    ERROR_IF(prop_pos == (u32) -1, &doc->err, ERR_NOTFOUND);                                        \
    encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, prop_pos,                           \
                                                                   encoded_doc_prop_array);                   \
    ERROR_IF(array == NULL, &doc->err, ERR_INTERNALERR);                                                 \
    ERROR_IF(array->header.type != basic_type, &doc->err, ERR_TYPEMISMATCH);                             \
    for (u32 i = 0; i < values_length; i++) {                                                                     \
        encoded_doc_value_u *value = VECTOR_NEW_AND_GET(&array->values, encoded_doc_value_u);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, archive_field_i8_t, FIELD_INT8)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, archive_field_i16_t, FIELD_INT16)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, archive_field_i32_t, FIELD_INT32)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, archive_field_i64_t, FIELD_INT64)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, archive_field_u8_t, FIELD_UINT8)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, archive_field_u16_t, FIELD_UINT16)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, archive_field_u32_t, FIELD_UINT32)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, archive_field_u64_t, FIELD_UINT64)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(number, archive_field_number_t, FIELD_FLOAT)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, archive_field_boolean_t, FIELD_BOOLEAN)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(string, archive_field_sid_t, FIELD_STRING)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE(null, archive_field_u32_t, FIELD_NULL)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, archive_field_i8_t, FIELD_INT8)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, archive_field_i16_t, FIELD_INT16)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, archive_field_i32_t, FIELD_INT32)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, archive_field_i64_t, FIELD_INT64)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, archive_field_u8_t, FIELD_UINT8)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, archive_field_u16_t, FIELD_UINT16)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, archive_field_u32_t, FIELD_UINT32)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, archive_field_u64_t, FIELD_UINT64)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, archive_field_number_t, FIELD_FLOAT)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, archive_field_boolean_t, FIELD_BOOLEAN)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, archive_field_sid_t, FIELD_STRING)

DECLARE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, archive_field_u32_t, FIELD_NULL)
//
//bool
//encoded_doc_array_push_null(encoded_doc *doc, archive_field_sid_t key, u32 how_many)
//{
//    ERROR_IF_NULL(doc)
//    const u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);
//    ERROR_IF(prop_pos == NULL, &doc->err, ERR_NOTFOUND);
//    encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, *prop_pos,
//                                                               encoded_doc_prop_array);
//    ERROR_IF(array == NULL, &doc->err, ERR_INTERNALERR);
//    ERROR_IF(array->header.type != FIELD_NULL, &doc->err, ERR_TYPEMISMATCH);
//    encoded_doc_value_u *value = VECTOR_NEW_AND_GET(&array->values, encoded_doc_value_u);
//    value->num_nulls = how_many;
//    return true;
//}


#include <inttypes.h>
#include <jakson/archive/query.h>

bool encoded_doc_array_push_object(encoded_doc *doc, archive_field_sid_t key, unique_id_t id)
{
        UNUSED(doc);
        UNUSED(key);
        UNUSED(id);

        ERROR_IF_NULL(doc)
        const u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);
        ERROR_IF(prop_pos == NULL, &doc->err, ERR_NOTFOUND);
        encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, *prop_pos,
                                                           encoded_doc_prop_array);
        ERROR_IF(array == NULL, &doc->err, ERR_INTERNALERR);
        ERROR_IF(array->header.type != FIELD_OBJECT, &doc->err, ERR_TYPEMISMATCH);
        encoded_doc_value_u *value = VECTOR_NEW_AND_GET(&array->values, encoded_doc_value_u);
        value->object = id;
        return true;
}

bool encoded_doc_array_push_object_decoded(encoded_doc *doc, const char *key, unique_id_t id)
{
        UNUSED(doc);
        UNUSED(key);
        UNUSED(id);

        ERROR_IF_NULL(doc)
        u32 prop_pos = (u32) -1;
        for (u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                encoded_doc_prop_array *prop = VECTOR_GET(&doc->props_arrays, i,
                                                                  encoded_doc_prop_array);
                if (prop->header.key_type == STRING_DECODED) {
                        if (strcmp(prop->header.key.key_str, key) == 0) {
                                prop_pos = i;
                                break;
                        }
                }
        }
        ERROR_IF(prop_pos == (u32) -1, &doc->err, ERR_NOTFOUND);
        encoded_doc_prop_array *array = VECTOR_GET(&doc->props_arrays, prop_pos,
                                                           encoded_doc_prop_array);
        ERROR_IF(array == NULL, &doc->err, ERR_INTERNALERR);
        ERROR_IF(array->header.type != FIELD_OBJECT, &doc->err, ERR_TYPEMISMATCH);
        encoded_doc_value_u *value = VECTOR_NEW_AND_GET(&array->values, encoded_doc_value_u);
        value->object = id;
        return true;
}

static bool doc_print_pretty(FILE *file, encoded_doc *doc, unsigned level)
{
        query query;
        archive_query_run(&query, doc->context->archive);

        fprintf(file, "{\n");

        for (u32 i = 0; i < doc->props.num_elems; i++) {
                encoded_doc_prop *prop = VECTOR_GET(&doc->props, i, encoded_doc_prop);
                char *key_str = NULL;
                if (prop->header.key_type == STRING_ENCODED) {
                        key_str = query_fetch_string_by_id(&query, prop->header.key.key_id);
                } else {
                        key_str = strdup(prop->header.key.key_str);
                }

                for (unsigned k = 0; k < level; k++) {
                        fprintf(file, "   ");
                }

                fprintf(file, "\"%s\": ", key_str);
                switch (prop->header.type) {
                        case FIELD_INT8:
                                fprintf(file, "%" PRIi8, prop->value.builtin.int8);
                                break;
                        case FIELD_INT16:
                                fprintf(file, "%" PRIi16, prop->value.builtin.int16);
                                break;
                        case FIELD_INT32:
                                fprintf(file, "%" PRIi32, prop->value.builtin.int32);
                                break;
                        case FIELD_INT64:
                                fprintf(file, "%" PRIi64, prop->value.builtin.int64);
                                break;
                        case FIELD_UINT8:
                                fprintf(file, "%" PRIu8, prop->value.builtin.uint8);
                                break;
                        case FIELD_UINT16:
                                fprintf(file, "%" PRIu16, prop->value.builtin.uint16);
                                break;
                        case FIELD_UINT32:
                                fprintf(file, "%" PRIu32, prop->value.builtin.uint32);
                                break;
                        case FIELD_UINT64:
                                fprintf(file, "%" PRIu64, prop->value.builtin.uint64);
                                break;
                        case FIELD_FLOAT:
                                fprintf(file, "%.2f", ceilf(prop->value.builtin.number * 100) / 100);
                                break;
                        case FIELD_STRING: {
                                if (prop->header.value_type == VALUE_BUILTIN) {
                                        char *value_str = query_fetch_string_by_id(&query,
                                                                                       prop->value.builtin.string);
                                        fprintf(file, "\"%s\"", value_str);
                                        free(value_str);
                                } else {
                                        fprintf(file, "\"%s\"", prop->value.string);
                                }
                        }
                                break;
                        case FIELD_BOOLEAN:
                                fprintf(file, "\"%s\"", prop->value.builtin.boolean ? "true" : "false");
                                break;
                        case FIELD_NULL:
                                fprintf(file, "null");
                                break;
                        case FIELD_OBJECT: {
                                encoded_doc *nested =
                                        encoded_doc_collection_get_or_append(doc->context, prop->value.builtin.object);
                                doc_print_pretty(file, nested, level + 1);
                        }
                                break;
                        default: ERROR(&doc->err, ERR_INTERNALERR);
                                return false;
                }
                free(key_str);
                fprintf(file, "%s\n", i + 1 < doc->props.num_elems || doc->props_arrays.num_elems > 0 ? ", " : "");
        }

        for (u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                encoded_doc_prop_array *prop = VECTOR_GET(&doc->props_arrays, i,
                                                                  encoded_doc_prop_array);
                char *key_str = NULL;
                if (prop->header.key_type == STRING_ENCODED) {
                        key_str = query_fetch_string_by_id(&query, prop->header.key.key_id);
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
                        case FIELD_INT8:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_i8_t value = (VECTOR_GET(&prop->values, k,
                                                                                encoded_doc_value_u))->int8;
                                        if (IS_NULL_INT8(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi8 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_INT16:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_i16_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->int16;
                                        if (IS_NULL_INT16(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi16 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_INT32:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_i32_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->int32;
                                        if (IS_NULL_INT32(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi32 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_INT64:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_i64_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->int64;
                                        if (IS_NULL_INT64(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi64 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_UINT8:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_u8_t value = (VECTOR_GET(&prop->values, k,
                                                                                encoded_doc_value_u))->uint8;
                                        if (IS_NULL_UINT8(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu8 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_UINT16:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_u16_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->uint16;
                                        if (IS_NULL_UINT16(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu16 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_UINT32:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_u32_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->uint32;
                                        if (IS_NULL_UINT32(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu32 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_UINT64:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_u64_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->uint64;
                                        if (IS_NULL_UINT64(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu64 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_FLOAT:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_number_t value = (VECTOR_GET(&prop->values, k,
                                                                                    encoded_doc_value_u))->number;
                                        if (IS_NULL_NUMBER(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%.2f%s",
                                                        ceilf(value * 100) / 100,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_STRING: {
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_sid_t value = (VECTOR_GET(&prop->values, k,
                                                                                 encoded_doc_value_u))->string;
                                        if (IS_NULL_STRING(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                char *value_str = query_fetch_string_by_id(&query, value);
                                                fprintf(file,
                                                        "\"%s\"%s",
                                                        value_str,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                                free(value_str);
                                        }
                                }
                        }
                                break;
                        case FIELD_BOOLEAN:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        archive_field_boolean_t
                                                value = (VECTOR_GET(&prop->values, k, encoded_doc_value_u))->boolean;
                                        if (IS_NULL_BOOL(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%s%s",
                                                        value ? "true" : "false",
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case FIELD_NULL:
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                }
                                break;
                        case FIELD_OBJECT: {
                                for (u32 k = 0; k < prop->values.num_elems; k++) {
                                        unique_id_t nested_oid = (VECTOR_GET(&prop->values, k,
                                                                              encoded_doc_value_u))->object;
                                        encoded_doc
                                                *nested_doc = encoded_doc_collection_get_or_append(doc->context,
                                                                                                   nested_oid);
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
                        }
                                break;
                        default: ERROR(&doc->err, ERR_INTERNALERR);
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

        query_drop(&query);

        return true;
}

bool encoded_doc_print(FILE *file, encoded_doc *doc)
{
        return doc_print_pretty(file, doc, 1);
}

