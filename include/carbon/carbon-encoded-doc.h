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

#ifndef CARBON_ENCODED_DOC_H
#define CARBON_ENCODED_DOC_H

#include "carbon-common.h"
#include "carbon-hashtable.h"
#include "carbon-oid.h"
#include "carbon-types.h"
#include "carbon-archive.h"

CARBON_BEGIN_DECL

typedef struct carbon_encoded_doc_collection carbon_encoded_doc_collection_t;

typedef struct carbon_encoded_doc carbon_encoded_doc_t;

typedef union {
    carbon_int8_t int8;
    carbon_int16_t int16;
    carbon_int32_t int32;
    carbon_int64_t int64;
    carbon_uint8_t uint8;
    carbon_uint16_t uint16;
    carbon_uint32_t uint32;
    carbon_uint64_t uint64;
    carbon_number_t number;
    carbon_boolean_t boolean;
    carbon_string_id_t string;
    carbon_object_id_t object;
    uint32_t null;
} carbon_encoded_doc_value_t;

typedef enum
{
    CARBON_ENCODED_DOC_PROP_STRING_TYPE_ENCODED_STRING,
    CARBON_ENCODED_DOC_PROP_STRING_TYPE_DECODED_STRING,
} carbon_encoded_doc_prop_string_type_e;

typedef enum
{
    CARBON_ENCODED_DOC_PROP_VALUE_TYPE_BUILTIN,
    CARBON_ENCODED_DOC_PROP_VALUE_TYPE_DECODED_STRING,
} carbon_encoded_doc_prop_value_type_e;

typedef struct
{
    carbon_encoded_doc_t                  *context;

    carbon_encoded_doc_prop_string_type_e  key_type;
    union {
        carbon_string_id_t          key_id;
        char                       *key_str;
    } key;

    carbon_encoded_doc_prop_value_type_e   value_type;
    carbon_basic_type_e                    type;

} carbon_encoded_doc_prop_header_t;

typedef struct
{
    carbon_encoded_doc_prop_header_t header;

    union {
        carbon_encoded_doc_value_t       builtin;
        char                            *string;
    } value;

} carbon_encoded_doc_prop_t;

typedef struct
{
    carbon_encoded_doc_prop_header_t header;
    carbon_vec_t ofType(carbon_encoded_doc_value_t) values;
} carbon_encoded_doc_prop_array_t;

typedef struct carbon_encoded_doc
{
    carbon_encoded_doc_collection_t                      *context;
    carbon_object_id_t                                    object_id;
    carbon_vec_t ofType(carbon_encoded_doc_prop_t)        props;
    carbon_vec_t ofType(carbon_encoded_doc_prop_array_t)  props_arrays;
    carbon_hashtable_t ofMapping(carbon_string_id_t, uint32_t) prop_array_index; /* maps key to index in prop arrays */

    carbon_err_t err;

} carbon_encoded_doc_t;



typedef struct carbon_encoded_doc_collection
{
    carbon_archive_t *archive;

    carbon_vec_t ofType(carbon_encoded_doc_t) flat_object_collection;   /* list of objects; also nested ones */
    carbon_hashtable_t ofMapping(carbon_object_id_t, uint32_t) index;   /* maps oid to index in collection */

    carbon_err_t err;

} carbon_encoded_doc_collection_t;

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_create(carbon_encoded_doc_collection_t *collection, carbon_err_t *err,
                                     carbon_archive_t *archive);

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_drop(carbon_encoded_doc_collection_t *collection);

CARBON_EXPORT(carbon_encoded_doc_t *)
encoded_doc_collection_get_or_append(carbon_encoded_doc_collection_t *collection, carbon_object_id_t id);

CARBON_EXPORT(bool)
carbon_encoded_doc_collection_print(FILE *file, carbon_encoded_doc_collection_t *collection);

CARBON_EXPORT(bool)
carbon_encoded_doc_drop(carbon_encoded_doc_t *doc);

CARBON_EXPORT(bool)
carbon_encoded_doc_get_object_id(carbon_object_id_t *oid, carbon_encoded_doc_t *doc);

#define DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##name(carbon_encoded_doc_t *doc, carbon_string_id_t key, built_in_type value);

#define DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_##name##_decoded(carbon_encoded_doc_t *doc, const char *key, built_in_type value);

DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(int8, carbon_int8_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(int16, carbon_int16_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(int32, carbon_int32_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(int64, carbon_int64_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(uint8, carbon_uint8_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(uint16, carbon_uint16_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(uint32, carbon_uint32_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(uint64, carbon_uint64_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(number, carbon_number_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(boolean, carbon_boolean_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC(string, carbon_string_id_t)

DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, carbon_int8_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, carbon_int16_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, carbon_int32_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, carbon_int64_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, carbon_uint8_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, carbon_uint16_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, carbon_uint32_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, carbon_uint64_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, carbon_number_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, carbon_boolean_t)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, carbon_string_id_t)

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_string_decoded_string_value_decoded(carbon_encoded_doc_t *doc, const char *key, const char *value);

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_null(carbon_encoded_doc_t *doc, carbon_string_id_t key);

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_null_decoded(carbon_encoded_doc_t *doc, const char *key);

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_encoded_doc_t *value);

CARBON_EXPORT(bool)
carbon_encoded_doc_add_prop_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_encoded_doc_t *value);

#define DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name(carbon_encoded_doc_t *doc, carbon_string_id_t key);

#define DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_add_prop_array_##name##_decoded(carbon_encoded_doc_t *doc, const char *key);

DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)

DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)
DEFINE_CARBON_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                 \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name(carbon_encoded_doc_t *doc, carbon_string_id_t key,                                \
                                     const built_in_type *array, uint32_t array_length);

#define DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                         \
CARBON_EXPORT(bool)                                                                                                    \
carbon_encoded_doc_array_push_##name##_decoded(carbon_encoded_doc_t *doc, const char *key,                             \
                                     const built_in_type *array, uint32_t array_length);

DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, carbon_int8_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, carbon_int16_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, carbon_int32_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, carbon_int64_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, carbon_uint8_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, carbon_uint16_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, carbon_uint32_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, carbon_uint64_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(number, carbon_number_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, carbon_boolean_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(string, carbon_string_id_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE(null, carbon_uint32_t)

DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, carbon_int8_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, carbon_int16_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, carbon_int32_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, carbon_int64_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, carbon_uint8_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, carbon_uint16_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, carbon_uint32_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, carbon_uint64_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, carbon_number_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, carbon_boolean_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, carbon_string_id_t)
DEFINE_CARBON_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, carbon_uint32_t)

//CARBON_EXPORT(bool)
//carbon_encoded_doc_array_push_null(carbon_encoded_doc_t *doc, carbon_string_id_t key, uint32_t how_many);

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_object(carbon_encoded_doc_t *doc, carbon_string_id_t key, carbon_object_id_t id);

CARBON_EXPORT(bool)
carbon_encoded_doc_array_push_object_decoded(carbon_encoded_doc_t *doc, const char *key, carbon_object_id_t id);

CARBON_EXPORT(bool)
carbon_encoded_doc_get_nested_object(carbon_encoded_doc_t *nested, carbon_object_id_t oid, carbon_encoded_doc_t *doc);

CARBON_EXPORT(bool)
carbon_encoded_doc_print(FILE *file, carbon_encoded_doc_t *doc);


CARBON_END_DECL

#endif
