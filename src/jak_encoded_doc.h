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
#include <jak_global_id.h>
#include <jak_types.h>
#include <jak_archive.h>

JAK_BEGIN_DECL

union encoded_doc_value {
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
    jak_global_id_t object;
    jak_u32 null;
};

enum encoded_doc_string_type {
    STRING_ENCODED, STRING_DECODED,
};

enum encoded_doc_value_type {
    VALUE_BUILTIN, VALUE_DECODED_STRING,
};

struct jak_encoded_doc_prop_header {
    struct jak_encoded_doc *context;

    enum encoded_doc_string_type key_type;
    union {
        jak_archive_field_sid_t key_id;
        char *key_str;
    } key;

    enum encoded_doc_value_type value_type;
    enum jak_archive_field_type type;

};

struct jak_encoded_doc_prop {
    struct jak_encoded_doc_prop_header header;

    union {
        union encoded_doc_value builtin;
        char *string;
    } value;
};

struct jak_encoded_doc_prop_array {
    struct jak_encoded_doc_prop_header header;
    struct vector ofType(union encoded_doc_value) values;
};

struct jak_encoded_doc {
    struct jak_encoded_doc_list *context;
    jak_global_id_t object_id;
    struct vector ofType(struct jak_encoded_doc_prop) props;
    struct vector ofType(struct jak_encoded_doc_prop_array) props_arrays;
    struct hashtable ofMapping(jak_archive_field_sid_t, jak_u32) prop_array_index; /* maps key to index in prop arrays */
    struct jak_error err;
};

struct jak_encoded_doc_list {
    struct jak_archive *archive;
    struct vector ofType(struct jak_encoded_doc) flat_object_collection;   /* list of objects; also nested ones */
    struct hashtable ofMapping(object_id_t, jak_u32) index;   /* maps oid to index in collection */
    struct jak_error err;
};

bool encoded_doc_collection_create(struct jak_encoded_doc_list *collection, struct jak_error *err,
                                   struct jak_archive *archive);

bool encoded_doc_collection_drop(struct jak_encoded_doc_list *collection);

struct jak_encoded_doc *encoded_doc_collection_get_or_append(struct jak_encoded_doc_list *collection,
                                                             jak_global_id_t id);

bool encoded_doc_collection_print(FILE *file, struct jak_encoded_doc_list *collection);

bool encoded_doc_drop(struct jak_encoded_doc *doc);

#define DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
bool                                                                                                    \
encoded_doc_add_prop_##name(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, built_in_type value);

#define DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
bool                                                                                                    \
encoded_doc_add_prop_##name##_decoded(struct jak_encoded_doc *doc, const char *key, built_in_type value);

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(int8, jak_archive_field_i8_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(int16, jak_archive_field_i16_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(int32, jak_archive_field_i32_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(int64, jak_archive_field_i64_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(uint8, jak_archive_field_u8_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(uint16, jak_archive_field_u16_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(uint32, jak_archive_field_u32_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(uint64, jak_archive_field_u64_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(number, jak_archive_field_number_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(boolean, jak_archive_field_boolean_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC(string, jak_archive_field_sid_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, jak_archive_field_i8_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, jak_archive_field_i16_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, jak_archive_field_i32_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, jak_archive_field_i64_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, jak_archive_field_u8_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, jak_archive_field_u16_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, jak_archive_field_u32_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, jak_archive_field_u64_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, jak_archive_field_number_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, jak_archive_field_boolean_t)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, jak_archive_field_sid_t)

bool encoded_doc_add_prop_string_decoded_string_value_decoded(struct jak_encoded_doc *doc, const char *key,
                                                              const char *value);

bool encoded_doc_add_prop_null(struct jak_encoded_doc *doc, jak_archive_field_sid_t key);

bool encoded_doc_add_prop_null_decoded(struct jak_encoded_doc *doc, const char *key);

bool encoded_doc_add_prop_object(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, struct jak_encoded_doc *value);

bool encoded_doc_add_prop_object_decoded(struct jak_encoded_doc *doc, const char *key,
                                         struct jak_encoded_doc *value);

#define DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
bool                                                                                                    \
encoded_doc_add_prop_array_##name(struct jak_encoded_doc *doc, jak_archive_field_sid_t key);

#define DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
bool                                                                                                    \
encoded_doc_add_prop_array_##name##_decoded(struct jak_encoded_doc *doc, const char *key);

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)

DEFINE_JAK_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                    \
bool                                                                                                       \
encoded_doc_array_push_##name(struct jak_encoded_doc *doc, jak_archive_field_sid_t key,                                                \
                                     const built_in_type *array, jak_u32 array_length);

#define DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                            \
bool                                                                                                       \
encoded_doc_array_push_##name##_decoded(struct jak_encoded_doc *doc, const char *key,                                      \
                                     const built_in_type *array, jak_u32 array_length);

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, jak_archive_field_i8_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, jak_archive_field_i16_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, jak_archive_field_i32_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, jak_archive_field_i64_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, jak_archive_field_u8_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, jak_archive_field_u16_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, jak_archive_field_u32_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, jak_archive_field_u64_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(number, jak_archive_field_number_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, jak_archive_field_boolean_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(string, jak_archive_field_sid_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE(null, jak_archive_field_u32_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, jak_archive_field_i8_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, jak_archive_field_i16_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, jak_archive_field_i32_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, jak_archive_field_i64_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, jak_archive_field_u8_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, jak_archive_field_u16_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, jak_archive_field_u32_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, jak_archive_field_u64_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, jak_archive_field_number_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, jak_archive_field_boolean_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, jak_archive_field_sid_t)

DEFINE_JAK_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, jak_archive_field_u32_t)

bool encoded_doc_array_push_object(struct jak_encoded_doc *doc, jak_archive_field_sid_t key, jak_global_id_t id);

bool encoded_doc_array_push_object_decoded(struct jak_encoded_doc *doc, const char *key, jak_global_id_t id);

bool encoded_doc_print(FILE *file, struct jak_encoded_doc *doc);

JAK_END_DECL

#endif
