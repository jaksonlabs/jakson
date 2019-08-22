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

#include <jak_encoded_doc.h>

bool encoded_doc_collection_create(struct jak_encoded_doc_list *collection, struct jak_error *err,
                                   jak_archive *archive)
{
        JAK_UNUSED(collection);
        JAK_UNUSED(err);
        JAK_UNUSED(archive);

        vec_create(&collection->flat_object_collection, NULL, sizeof(struct jak_encoded_doc), 5000000);
        hashtable_create(&collection->index, err, sizeof(jak_global_id_t), sizeof(jak_u32), 5000000);
        error_init(&collection->err);
        collection->archive = archive;

        return true;
}

bool encoded_doc_collection_drop(struct jak_encoded_doc_list *collection)
{
        JAK_UNUSED(collection);

        hashtable_drop(&collection->index);
        for (jak_u32 i = 0; i < collection->flat_object_collection.num_elems; i++) {
                struct jak_encoded_doc *doc = vec_get(&collection->flat_object_collection, i, struct jak_encoded_doc);
                encoded_doc_drop(doc);
        }
        vec_drop(&collection->flat_object_collection);
        return true;
}

static struct jak_encoded_doc *
doc_create(struct jak_error *err, jak_global_id_t object_id, struct jak_encoded_doc_list *collection)
{
        if (collection) {
                jak_u32 doc_position = collection->flat_object_collection.num_elems;
                struct jak_encoded_doc
                        *new_doc = vec_new_and_get(&collection->flat_object_collection, struct jak_encoded_doc);
                new_doc->context = collection;
                new_doc->object_id = object_id;
                vec_create(&new_doc->props, NULL, sizeof(struct jak_encoded_doc_prop), 20);
                vec_create(&new_doc->props_arrays, NULL, sizeof(struct jak_encoded_doc_prop_array), 20);
                hashtable_create(&new_doc->prop_array_index, err, sizeof(jak_archive_field_sid_t), sizeof(jak_u32), 20);
                error_init(&new_doc->err);
                hashtable_insert_or_update(&collection->index, &object_id, &doc_position, 1);
                return new_doc;
        } else {
                error(err, JAK_ERR_ILLEGALARG);
                return NULL;
        }
}

struct jak_encoded_doc *encoded_doc_collection_get_or_append(struct jak_encoded_doc_list *collection,
                                                             jak_global_id_t id)
{
        JAK_ERROR_IF_NULL(collection);
        const jak_u32 *doc_pos = hashtable_get_value(&collection->index, &id);
        if (doc_pos) {
                struct jak_encoded_doc *result = vec_get(&collection->flat_object_collection, *doc_pos,
                                                         struct jak_encoded_doc);
                error_if(result == NULL, &collection->err, JAK_ERR_INTERNALERR);
                return result;
        } else {
                struct jak_encoded_doc *result = doc_create(&collection->err, id, collection);
                if (!result) {
                        error(&collection->err, JAK_ERR_INTERNALERR);
                }
                return result;
        }
}

bool encoded_doc_collection_print(FILE *file, struct jak_encoded_doc_list *collection)
{
        JAK_UNUSED(file);
        JAK_UNUSED(collection);

        if (collection->flat_object_collection.num_elems > 0) {
                struct jak_encoded_doc *root = vec_get(&collection->flat_object_collection, 0, struct jak_encoded_doc);
                encoded_doc_print(file, root);
        }

        return false;
}

bool encoded_doc_drop(struct jak_encoded_doc *doc)
{
        JAK_UNUSED(doc);
        for (jak_u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, i,
                                                                   struct jak_encoded_doc_prop_array);
                vec_drop(&array->values);
        }
        for (jak_u32 i = 0; i < doc->props.num_elems; i++) {
                struct jak_encoded_doc_prop *single = vec_get(&doc->props, i, struct jak_encoded_doc_prop);
                if (single->header.value_type == VALUE_DECODED_STRING) {
                        free(single->value.string);
                }
        }
        vec_drop(&doc->props);
        vec_drop(&doc->props_arrays);
        hashtable_drop(&doc->prop_array_index);
        return false;
}

#define DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(built_in_type, basic_type, value_name)                               \
bool                                                                                                    \
encoded_doc_add_prop_##value_name(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, built_in_type value)       \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(doc)                                                                                      \
    struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = STRING_ENCODED;                                        \
    prop->header.key.key_id = key;                                                                                     \
    prop->header.value_type = VALUE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

#define DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(built_in_type, basic_type, value_name)                       \
bool                                                                                                    \
encoded_doc_add_prop_##value_name##_decoded(struct jak_encoded_doc *doc, const char *key, built_in_type value)    \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(doc)                                                                                      \
    struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);                      \
    prop->header.context = doc;                                                                                        \
    prop->header.key_type = STRING_DECODED;                                        \
    prop->header.key.key_str = strdup(key);                                                                            \
    prop->header.value_type = VALUE_BUILTIN;                                              \
    prop->header.type = basic_type;                                                                                    \
    prop->value.builtin.value_name = value;                                                                            \
    return true;                                                                                                       \
}

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_i8_t, JAK_FIELD_INT8, int8)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_i16_t, JAK_FIELD_INT16, int16)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_i32_t, JAK_FIELD_INT32, int32)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_i64_t, JAK_FIELD_INT64, int64)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_u8_t, JAK_FIELD_UINT8, uint8)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_u16_t, JAK_FIELD_UINT16, uint16)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_u32_t, JAK_FIELD_UINT32, uint32)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_u64_t, JAK_FIELD_UINT64, uint64)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_number_t, JAK_FIELD_FLOAT, number)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_boolean_t, JAK_FIELD_BOOLEAN, boolean)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC(jak_archive_field_sid_t, JAK_FIELD_STRING, string)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_i8_t, JAK_FIELD_INT8, int8)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_i16_t, JAK_FIELD_INT16, int16)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_i32_t, JAK_FIELD_INT32, int32)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_i64_t, JAK_FIELD_INT64, int64)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_u8_t, JAK_FIELD_UINT8, uint8)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_u16_t, JAK_FIELD_UINT16, uint16)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_u32_t, JAK_FIELD_UINT32, uint32)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_u64_t, JAK_FIELD_UINT64, uint64)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_number_t, JAK_FIELD_FLOAT, number)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_boolean_t, JAK_FIELD_BOOLEAN, boolean)

DECLARE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(jak_archive_field_sid_t, JAK_FIELD_STRING, string)

bool encoded_doc_add_prop_string_decoded_string_value_decoded(struct jak_encoded_doc *doc, const char *key,
                                                              const char *value)
{
        JAK_ERROR_IF_NULL(doc)
        struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = JAK_FIELD_STRING;
        prop->value.string = strdup(value);
        return true;
}

bool encoded_doc_add_prop_null(struct jak_encoded_doc *doc, jak_archive_field_sid_t key)
{
        JAK_ERROR_IF_NULL(doc)
        struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_ENCODED;
        prop->header.key.key_id = key;
        prop->header.type = JAK_FIELD_NULL;
        prop->value.builtin.null = 1;
        return true;
}

bool encoded_doc_add_prop_null_decoded(struct jak_encoded_doc *doc, const char *key)
{
        JAK_ERROR_IF_NULL(doc)
        struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = JAK_FIELD_NULL;
        prop->value.builtin.null = 1;
        return true;
}

bool
encoded_doc_add_prop_object(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, struct jak_encoded_doc *value)
{
        JAK_ERROR_IF_NULL(doc)
        struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_ENCODED;
        prop->header.key.key_id = key;
        prop->header.type = JAK_FIELD_OBJECT;
        prop->value.builtin.object = value->object_id;
        return true;
}

bool encoded_doc_add_prop_object_decoded(struct jak_encoded_doc *doc, const char *key,
                                         struct jak_encoded_doc *value)
{
        JAK_ERROR_IF_NULL(doc)
        struct jak_encoded_doc_prop *prop = vec_new_and_get(&doc->props, struct jak_encoded_doc_prop);
        prop->header.context = doc;
        prop->header.key_type = STRING_DECODED;
        prop->header.key.key_str = strdup(key);
        prop->header.type = JAK_FIELD_OBJECT;
        prop->value.builtin.object = value->object_id;
        return true;
}

#define DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name, basic_type)                                               \
bool                                                                                                    \
encoded_doc_add_prop_array_##name(struct jak_encoded_doc *doc,                                                    \
                                       jak_archive_field_sid_t key)                                                         \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(doc)                                                                                      \
    jak_u32 new_array_pos = doc->props_arrays.num_elems;                                                              \
    struct jak_encoded_doc_prop_array *array = vec_new_and_get(&doc->props_arrays, struct jak_encoded_doc_prop_array);  \
    array->header.key_type = STRING_ENCODED;                                          \
    array->header.key.key_id = key;                                                                                    \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    vec_create(&array->values, NULL, sizeof(union encoded_doc_value), 10);                                   \
    hashtable_insert_or_update(&doc->prop_array_index, &key, &new_array_pos, 1);                                \
    return true;                                                                                                       \
}

#define DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name, basic_type)                                       \
bool                                                                                                    \
encoded_doc_add_prop_array_##name##_decoded(struct jak_encoded_doc *doc,                                          \
                                       const char *key)                                                                \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(doc)                                                                                      \
    struct jak_encoded_doc_prop_array *array = vec_new_and_get(&doc->props_arrays, struct jak_encoded_doc_prop_array);  \
    array->header.key_type = STRING_DECODED;                                          \
    array->header.key.key_str = strdup(key);                                                                           \
    array->header.type = basic_type;                                                                                   \
    array->header.context = doc;                                                                                       \
    vec_create(&array->values, NULL, sizeof(union encoded_doc_value), 10);                                   \
    return true;                                                                                                       \
}

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8, JAK_FIELD_INT8)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16, JAK_FIELD_INT16)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32, JAK_FIELD_INT32)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64, JAK_FIELD_INT64)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8, JAK_FIELD_UINT8)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16, JAK_FIELD_UINT16)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32, JAK_FIELD_UINT32)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64, JAK_FIELD_UINT64)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number, JAK_FIELD_FLOAT)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean, JAK_FIELD_BOOLEAN)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string, JAK_FIELD_STRING)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null, JAK_FIELD_NULL)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object, JAK_FIELD_OBJECT)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8, JAK_FIELD_INT8)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16, JAK_FIELD_INT16)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32, JAK_FIELD_INT32)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64, JAK_FIELD_INT64)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8, JAK_FIELD_UINT8)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16, JAK_FIELD_UINT16)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32, JAK_FIELD_UINT32)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64, JAK_FIELD_UINT64)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number, JAK_FIELD_FLOAT)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean, JAK_FIELD_BOOLEAN)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string, JAK_FIELD_STRING)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null, JAK_FIELD_NULL)

DECALRE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object, JAK_FIELD_OBJECT)

#define DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type, basic_type)                                    \
bool                                                                                                    \
encoded_doc_array_push_##name(struct jak_encoded_doc *doc, jak_archive_field_sid_t key,                                \
                                     const built_in_type *values, jak_u32 values_length)                              \
{                                                                                                                      \
    JAK_ERROR_IF_NULL(doc)                                                                                      \
    const jak_u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);                               \
    error_if(prop_pos == NULL, &doc->err, JAK_ERR_NOTFOUND);                                                 \
    struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, *prop_pos,                          \
                                                               struct jak_encoded_doc_prop_array);                       \
    error_if(array == NULL, &doc->err, JAK_ERR_INTERNALERR);                                                 \
    error_if(array->header.type != basic_type, &doc->err, JAK_ERR_TYPEMISMATCH);                             \
    for (jak_u32 i = 0; i < values_length; i++) {                                                                     \
        union encoded_doc_value *value = vec_new_and_get(&array->values, union encoded_doc_value);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

#include <inttypes.h>

#define DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type, basic_type)                            \
bool                                                                                                    \
encoded_doc_array_push_##name##_decoded(struct jak_encoded_doc *doc, const char *key,                             \
                                     const built_in_type *values, jak_u32 values_length)                              \
{                                                                                                                      \
    jak_u32 prop_pos = (jak_u32) -1;                                                                                 \
    for (jak_u32 i = 0; i < doc->props_arrays.num_elems; i++)                                                         \
    {                                                                                                                  \
        struct jak_encoded_doc_prop_array *prop = vec_get(&doc->props_arrays, i, struct jak_encoded_doc_prop_array); \
        if (prop->header.key_type == STRING_DECODED) {                                \
            if (strcmp(prop->header.key.key_str, key) == 0) {                                                          \
                prop_pos = i;                                                                                          \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
    error_if(prop_pos == (jak_u32) -1, &doc->err, JAK_ERR_NOTFOUND);                                        \
    struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, prop_pos,                           \
                                                                   struct jak_encoded_doc_prop_array);                   \
    error_if(array == NULL, &doc->err, JAK_ERR_INTERNALERR);                                                 \
    error_if(array->header.type != basic_type, &doc->err, JAK_ERR_TYPEMISMATCH);                             \
    for (jak_u32 i = 0; i < values_length; i++) {                                                                     \
        union encoded_doc_value *value = vec_new_and_get(&array->values, union encoded_doc_value);            \
        value->name = values[i];                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
}

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, jak_archive_field_i8_t, JAK_FIELD_INT8)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, jak_archive_field_i16_t, JAK_FIELD_INT16)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, jak_archive_field_i32_t, JAK_FIELD_INT32)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, jak_archive_field_i64_t, JAK_FIELD_INT64)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, jak_archive_field_u8_t, JAK_FIELD_UINT8)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, jak_archive_field_u16_t, JAK_FIELD_UINT16)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, jak_archive_field_u32_t, JAK_FIELD_UINT32)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, jak_archive_field_u64_t, JAK_FIELD_UINT64)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(number, jak_archive_field_number_t, JAK_FIELD_FLOAT)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, jak_archive_field_boolean_t, JAK_FIELD_BOOLEAN)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(string, jak_archive_field_sid_t, JAK_FIELD_STRING)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(null, jak_archive_field_u32_t, JAK_FIELD_NULL)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, jak_archive_field_i8_t, JAK_FIELD_INT8)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, jak_archive_field_i16_t, JAK_FIELD_INT16)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, jak_archive_field_i32_t, JAK_FIELD_INT32)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, jak_archive_field_i64_t, JAK_FIELD_INT64)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, jak_archive_field_u8_t, JAK_FIELD_UINT8)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, jak_archive_field_u16_t, JAK_FIELD_UINT16)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, jak_archive_field_u32_t, JAK_FIELD_UINT32)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, jak_archive_field_u64_t, JAK_FIELD_UINT64)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, jak_archive_field_number_t, JAK_FIELD_FLOAT)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, jak_archive_field_boolean_t, JAK_FIELD_BOOLEAN)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, jak_archive_field_sid_t, JAK_FIELD_STRING)

DECLARE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, jak_archive_field_u32_t, JAK_FIELD_NULL)
//
//bool
//encoded_doc_array_push_null(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_u32 how_many)
//{
//    JAK_ERROR_IF_NULL(doc)
//    const jak_u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);
//    error_if(prop_pos == NULL, &doc->err, JAK_ERR_NOTFOUND);
//    struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, *prop_pos,
//                                                               struct jak_encoded_doc_prop_array);
//    error_if(array == NULL, &doc->err, JAK_ERR_INTERNALERR);
//    error_if(array->header.type != JAK_FIELD_NULL, &doc->err, JAK_ERR_TYPEMISMATCH);
//    union encoded_doc_value *value = vec_new_and_get(&array->values, union encoded_doc_value);
//    value->num_nulls = how_many;
//    return true;
//}


#include <inttypes.h>
#include <jak_archive_query.h>

bool encoded_doc_array_push_object(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_global_id_t id)
{
        JAK_UNUSED(doc);
        JAK_UNUSED(key);
        JAK_UNUSED(id);

        JAK_ERROR_IF_NULL(doc)
        const jak_u32 *prop_pos = hashtable_get_value(&doc->prop_array_index, &key);
        error_if(prop_pos == NULL, &doc->err, JAK_ERR_NOTFOUND);
        struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, *prop_pos,
                                                           struct jak_encoded_doc_prop_array);
        error_if(array == NULL, &doc->err, JAK_ERR_INTERNALERR);
        error_if(array->header.type != JAK_FIELD_OBJECT, &doc->err, JAK_ERR_TYPEMISMATCH);
        union encoded_doc_value *value = vec_new_and_get(&array->values, union encoded_doc_value);
        value->object = id;
        return true;
}

bool encoded_doc_array_push_object_decoded(struct jak_encoded_doc *doc, const char *key, jak_global_id_t id)
{
        JAK_UNUSED(doc);
        JAK_UNUSED(key);
        JAK_UNUSED(id);

        JAK_ERROR_IF_NULL(doc)
        jak_u32 prop_pos = (jak_u32) -1;
        for (jak_u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                struct jak_encoded_doc_prop_array *prop = vec_get(&doc->props_arrays, i,
                                                                  struct jak_encoded_doc_prop_array);
                if (prop->header.key_type == STRING_DECODED) {
                        if (strcmp(prop->header.key.key_str, key) == 0) {
                                prop_pos = i;
                                break;
                        }
                }
        }
        error_if(prop_pos == (jak_u32) -1, &doc->err, JAK_ERR_NOTFOUND);
        struct jak_encoded_doc_prop_array *array = vec_get(&doc->props_arrays, prop_pos,
                                                           struct jak_encoded_doc_prop_array);
        error_if(array == NULL, &doc->err, JAK_ERR_INTERNALERR);
        error_if(array->header.type != JAK_FIELD_OBJECT, &doc->err, JAK_ERR_TYPEMISMATCH);
        union encoded_doc_value *value = vec_new_and_get(&array->values, union encoded_doc_value);
        value->object = id;
        return true;
}

static bool doc_print_pretty(FILE *file, struct jak_encoded_doc *doc, unsigned level)
{
        jak_archive_query query;
        jak_archive_query_run(&query, doc->context->archive);

        fprintf(file, "{\n");

        for (jak_u32 i = 0; i < doc->props.num_elems; i++) {
                struct jak_encoded_doc_prop *prop = vec_get(&doc->props, i, struct jak_encoded_doc_prop);
                char *key_str = NULL;
                if (prop->header.key_type == STRING_ENCODED) {
                        key_str = jak_query_fetch_string_by_id(&query, prop->header.key.key_id);
                } else {
                        key_str = strdup(prop->header.key.key_str);
                }

                for (unsigned k = 0; k < level; k++) {
                        fprintf(file, "   ");
                }

                fprintf(file, "\"%s\": ", key_str);
                switch (prop->header.type) {
                        case JAK_FIELD_INT8:
                                fprintf(file, "%" PRIi8, prop->value.builtin.int8);
                                break;
                        case JAK_FIELD_INT16:
                                fprintf(file, "%" PRIi16, prop->value.builtin.int16);
                                break;
                        case JAK_FIELD_INT32:
                                fprintf(file, "%" PRIi32, prop->value.builtin.int32);
                                break;
                        case JAK_FIELD_INT64:
                                fprintf(file, "%" PRIi64, prop->value.builtin.int64);
                                break;
                        case JAK_FIELD_UINT8:
                                fprintf(file, "%" PRIu8, prop->value.builtin.uint8);
                                break;
                        case JAK_FIELD_UINT16:
                                fprintf(file, "%" PRIu16, prop->value.builtin.uint16);
                                break;
                        case JAK_FIELD_UINT32:
                                fprintf(file, "%" PRIu32, prop->value.builtin.uint32);
                                break;
                        case JAK_FIELD_UINT64:
                                fprintf(file, "%" PRIu64, prop->value.builtin.uint64);
                                break;
                        case JAK_FIELD_FLOAT:
                                fprintf(file, "%.2f", ceilf(prop->value.builtin.number * 100) / 100);
                                break;
                        case JAK_FIELD_STRING: {
                                if (prop->header.value_type == VALUE_BUILTIN) {
                                        char *value_str = jak_query_fetch_string_by_id(&query,
                                                                                       prop->value.builtin.string);
                                        fprintf(file, "\"%s\"", value_str);
                                        free(value_str);
                                } else {
                                        fprintf(file, "\"%s\"", prop->value.string);
                                }
                        }
                                break;
                        case JAK_FIELD_BOOLEAN:
                                fprintf(file, "\"%s\"", prop->value.builtin.boolean ? "true" : "false");
                                break;
                        case JAK_FIELD_NULL:
                                fprintf(file, "null");
                                break;
                        case JAK_FIELD_OBJECT: {
                                struct jak_encoded_doc *nested =
                                        encoded_doc_collection_get_or_append(doc->context, prop->value.builtin.object);
                                doc_print_pretty(file, nested, level + 1);
                        }
                                break;
                        default: error(&doc->err, JAK_ERR_INTERNALERR);
                                return false;
                }
                free(key_str);
                fprintf(file, "%s\n", i + 1 < doc->props.num_elems || doc->props_arrays.num_elems > 0 ? ", " : "");
        }

        for (jak_u32 i = 0; i < doc->props_arrays.num_elems; i++) {
                struct jak_encoded_doc_prop_array *prop = vec_get(&doc->props_arrays, i,
                                                                  struct jak_encoded_doc_prop_array);
                char *key_str = NULL;
                if (prop->header.key_type == STRING_ENCODED) {
                        key_str = jak_query_fetch_string_by_id(&query, prop->header.key.key_id);
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
                        case JAK_FIELD_INT8:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_i8_t value = (vec_get(&prop->values, k,
                                                                                union encoded_doc_value))->int8;
                                        if (JAK_IS_NULL_INT8(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi8 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_INT16:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_i16_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->int16;
                                        if (JAK_IS_NULL_INT16(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi16 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_INT32:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_i32_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->int32;
                                        if (JAK_IS_NULL_INT32(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi32 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_INT64:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_i64_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->int64;
                                        if (JAK_IS_NULL_INT64(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIi64 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_UINT8:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_u8_t value = (vec_get(&prop->values, k,
                                                                                union encoded_doc_value))->uint8;
                                        if (JAK_IS_NULL_UINT8(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu8 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_UINT16:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_u16_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->uint16;
                                        if (JAK_IS_NULL_UINT16(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu16 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_UINT32:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_u32_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->uint32;
                                        if (JAK_IS_NULL_UINT32(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu32 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_UINT64:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_u64_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->uint64;
                                        if (JAK_IS_NULL_UINT64(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%" PRIu64 "%s",
                                                        value,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_FLOAT:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_number_t value = (vec_get(&prop->values, k,
                                                                                    union encoded_doc_value))->number;
                                        if (JAK_IS_NULL_NUMBER(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%.2f%s",
                                                        ceilf(value * 100) / 100,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_STRING: {
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_sid_t value = (vec_get(&prop->values, k,
                                                                                 union encoded_doc_value))->string;
                                        if (JAK_IS_NULL_STRING(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                char *value_str = jak_query_fetch_string_by_id(&query, value);
                                                fprintf(file,
                                                        "\"%s\"%s",
                                                        value_str,
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                                free(value_str);
                                        }
                                }
                        }
                                break;
                        case JAK_FIELD_BOOLEAN:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_archive_field_boolean_t
                                                value = (vec_get(&prop->values, k, union encoded_doc_value))->boolean;
                                        if (JAK_IS_NULL_BOOLEAN(value)) {
                                                fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                        } else {
                                                fprintf(file,
                                                        "%s%s",
                                                        value ? "true" : "false",
                                                        k + 1 < prop->values.num_elems ? ", " : "");
                                        }
                                }
                                break;
                        case JAK_FIELD_NULL:
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        fprintf(file, "null%s", k + 1 < prop->values.num_elems ? ", " : "");
                                }
                                break;
                        case JAK_FIELD_OBJECT: {
                                for (jak_u32 k = 0; k < prop->values.num_elems; k++) {
                                        jak_global_id_t nested_oid = (vec_get(&prop->values, k,
                                                                              union encoded_doc_value))->object;
                                        struct jak_encoded_doc
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
                        default: error(&doc->err, JAK_ERR_INTERNALERR);
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

        jak_query_drop(&query);

        return true;
}

bool encoded_doc_print(FILE *file, struct jak_encoded_doc *doc)
{
        return doc_print_pretty(file, doc, 1);
}

