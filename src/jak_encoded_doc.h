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

#ifndef JAK_ENCODED_DOC_H
#define JAK_ENCODED_DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_hash_table.h>
#include <jak_unique_id.h>
#include <jak_types.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

typedef union jak_encoded_doc_value {
        jak_archive_field_i8_t int8;
        jak_archive_field_i16_t int16;
        jak_archive_field_i32_t int32;
        jak_archive_field_i64_t int64;
        jak_archive_field_u8_t uint8;
        jak_archive_field_u16_t uint16;
        jak_archive_field_u32_t uint32;
        jak_archive_field_u64_t uint64;
        jak_archive_field_number_t number;
        jak_archive_field_boolean_t boolean;
        jak_archive_field_sid_t string;
        jak_uid_t object;
        jak_u32 null;
} jak_encoded_doc_value_u;

typedef enum jak_encoded_doc_string {
        JAK_STRING_ENCODED, JAK_STRING_DECODED,
} jak_encoded_doc_jak_string_e;

typedef enum jak_encoded_doc_value_e {
        JAK_VALUE_BUILTIN, JAK_VALUE_DECODED_STRING,
} jak_encoded_doc_value_e;

typedef struct jak_encoded_doc_prop_header {
        jak_encoded_doc *context;

        jak_encoded_doc_jak_string_e key_type;
        union {
                jak_archive_field_sid_t key_id;
                char *key_str;
        } key;

        jak_encoded_doc_value_e value_type;
        enum jak_archive_field_type type;
} jak_encoded_doc_prop_header;

typedef struct jak_encoded_doc_prop {
        jak_encoded_doc_prop_header header;
        union {
                jak_encoded_doc_value_u builtin;
                char *string;
        } value;
} jak_encoded_doc_prop;

typedef struct jak_encoded_doc_prop_array {
        jak_encoded_doc_prop_header header;
        jak_vector ofType(jak_encoded_doc_value_u) values;
} jak_encoded_doc_prop_array;

typedef struct jak_encoded_doc {
        jak_encoded_doc_list *context;
        jak_uid_t object_id;
        jak_vector ofType(jak_encoded_doc_prop) props;
        jak_vector ofType(jak_encoded_doc_prop_array) props_arrays;
        jak_hashtable ofMapping(jak_archive_field_sid_t,
                                   jak_u32) prop_array_index; /* maps key to index in prop arrays */
        jak_error err;
} jak_encoded_doc;

typedef struct jak_encoded_doc_list {
        jak_archive *archive;
        jak_vector ofType(
                jak_encoded_doc) flat_object_collection;   /* list of objects; also nested ones */
        jak_hashtable ofMapping(object_id_t, jak_u32) index;   /* maps oid to index in collection */
        jak_error err;
} jak_encoded_doc_list;

bool jak_encoded_doc_collection_create(jak_encoded_doc_list *collection, jak_error *err, jak_archive *archive);
bool jak_encoded_doc_collection_drop(jak_encoded_doc_list *collection);
jak_encoded_doc *jak_encoded_doc_collection_get_or_append(jak_encoded_doc_list *collection, jak_uid_t id);
bool jak_encoded_doc_collection_print(FILE *file, jak_encoded_doc_list *collection);

bool jak_encoded_doc_drop(jak_encoded_doc *doc);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
bool jak_encoded_doc_add_prop_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key, built_in_type value);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
bool jak_encoded_doc_add_prop_##name##_decoded(jak_encoded_doc *doc, const char *key, built_in_type value);

JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, jak_archive_field_sid_t)

bool jak_encoded_doc_add_prop_jak_string_decoded_jak_string_value_decoded(jak_encoded_doc *doc, const char *key, const char *value);
bool jak_encoded_doc_add_prop_null(jak_encoded_doc *doc, jak_archive_field_sid_t key);
bool jak_encoded_doc_add_prop_null_decoded(jak_encoded_doc *doc, const char *key);
bool jak_encoded_doc_add_prop_object(jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_encoded_doc *value);
bool jak_encoded_doc_add_prop_object_decoded(jak_encoded_doc *doc, const char *key, jak_encoded_doc *value);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
bool jak_encoded_doc_add_prop_array_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key);

#define JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
bool jak_encoded_doc_add_prop_array_##name##_decoded(jak_encoded_doc *doc, const char *key);

JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)
JAK_DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                    \
bool jak_encoded_doc_array_push_##name(jak_encoded_doc *doc, jak_archive_field_sid_t key, const built_in_type *array, jak_u32 array_length);

#define JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                            \
bool jak_encoded_doc_array_push_##name##_decoded(jak_encoded_doc *doc, const char *key, const built_in_type *array, jak_u32 array_length);

JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(null, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, jak_archive_field_i8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, jak_archive_field_i16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, jak_archive_field_i32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, jak_archive_field_i64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, jak_archive_field_u8_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, jak_archive_field_u16_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, jak_archive_field_u32_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, jak_archive_field_u64_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, jak_archive_field_number_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, jak_archive_field_boolean_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, jak_archive_field_sid_t)
JAK_DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, jak_archive_field_u32_t)

bool jak_encoded_doc_array_push_object(jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_uid_t id);
bool jak_encoded_doc_array_push_object_decoded(jak_encoded_doc *doc, const char *key, jak_uid_t id);
bool jak_encoded_doc_print(FILE *file, jak_encoded_doc *doc);

JAK_END_DECL

#endif
