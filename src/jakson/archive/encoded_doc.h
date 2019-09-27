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

#ifndef ENCODED_DOC_H
#define ENCODED_DOC_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/hash/table.h>
#include <jakson/stdx/unique_id.h>
#include <jakson/types.h>
#include <jakson/archive.h>

BEGIN_DECL

typedef union encoded_doc_value {
        archive_field_i8_t int8;
        archive_field_i16_t int16;
        archive_field_i32_t int32;
        archive_field_i64_t int64;
        archive_field_u8_t uint8;
        archive_field_u16_t uint16;
        archive_field_u32_t uint32;
        archive_field_u64_t uint64;
        archive_field_number_t number;
        archive_field_boolean_t boolean;
        archive_field_sid_t string;
        unique_id_t object;
        u32 null;
} encoded_doc_value_u;

typedef enum encoded_doc_string {
        STRING_ENCODED, STRING_DECODED,
} encoded_doc_string_e;

typedef enum encoded_doc_value_e {
        VALUE_BUILTIN, VALUE_DECODED_STRING,
} encoded_doc_value_e;

typedef struct encoded_doc_prop_header {
        encoded_doc *context;

        encoded_doc_string_e key_type;
        union {
                archive_field_sid_t key_id;
                char *key_str;
        } key;

        encoded_doc_value_e value_type;
        enum archive_field_type type;
} encoded_doc_prop_header;

typedef struct encoded_doc_prop {
        encoded_doc_prop_header header;
        union {
                encoded_doc_value_u builtin;
                char *string;
        } value;
} encoded_doc_prop;

typedef struct encoded_doc_prop_array {
        encoded_doc_prop_header header;
        vector ofType(encoded_doc_value_u) values;
} encoded_doc_prop_array;

typedef struct encoded_doc {
        encoded_doc_list *context;
        unique_id_t object_id;
        vector ofType(encoded_doc_prop) props;
        vector ofType(encoded_doc_prop_array) props_arrays;
        hashtable ofMapping(archive_field_sid_t,
                                   u32) prop_array_index; /** maps key to index in prop arrays */
        err err;
} encoded_doc;

typedef struct encoded_doc_list {
        archive *archive;
        vector ofType(
                encoded_doc) flat_object_collection;   /** list of objects; also nested ones */
        hashtable ofMapping(object_id_t, u32) index;   /** maps oid to index in collection */
        err err;
} encoded_doc_list;

bool encoded_doc_collection_create(encoded_doc_list *collection, err *err, archive *archive);
bool encoded_doc_collection_drop(encoded_doc_list *collection);
encoded_doc *encoded_doc_collection_get_or_append(encoded_doc_list *collection, unique_id_t id);
bool encoded_doc_collection_print(FILE *file, encoded_doc_list *collection);

bool encoded_doc_drop(encoded_doc *doc);

#define DEFINE_ENCODED_DOC_ADD_PROP_BASIC(name, built_in_type)                                                  \
bool encoded_doc_add_prop_##name(encoded_doc *doc, archive_field_sid_t key, built_in_type value);

#define DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(name, built_in_type)                                          \
bool encoded_doc_add_prop_##name##_decoded(encoded_doc *doc, const char *key, built_in_type value);

DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int8, archive_field_i8_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int16, archive_field_i16_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int32, archive_field_i32_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(int64, archive_field_i64_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint8, archive_field_u8_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint16, archive_field_u16_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint32, archive_field_u32_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(uint64, archive_field_u64_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(number, archive_field_number_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(boolean, archive_field_boolean_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC(string, archive_field_sid_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int8, archive_field_i8_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int16, archive_field_i16_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int32, archive_field_i32_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(int64, archive_field_i64_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint8, archive_field_u8_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint16, archive_field_u16_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint32, archive_field_u32_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(uint64, archive_field_u64_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(number, archive_field_number_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(boolean, archive_field_boolean_t)
DEFINE_ENCODED_DOC_ADD_PROP_BASIC_DECODED(string, archive_field_sid_t)

bool encoded_doc_add_prop_string_decoded_string_value_decoded(encoded_doc *doc, const char *key, const char *value);
bool encoded_doc_add_prop_null(encoded_doc *doc, archive_field_sid_t key);
bool encoded_doc_add_prop_null_decoded(encoded_doc *doc, const char *key);
bool encoded_doc_add_prop_object(encoded_doc *doc, archive_field_sid_t key, encoded_doc *value);
bool encoded_doc_add_prop_object_decoded(encoded_doc *doc, const char *key, encoded_doc *value);

#define DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(name)                                                            \
bool encoded_doc_add_prop_array_##name(encoded_doc *doc, archive_field_sid_t key);

#define DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(name)                                                    \
bool encoded_doc_add_prop_array_##name##_decoded(encoded_doc *doc, const char *key);

DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int8)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int16)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int32)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(int64)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint8)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint16)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint32)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(uint64)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(number)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(boolean)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(string)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(null)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE(object)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int8)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int16)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int32)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(int64)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint8)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint16)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint32)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(uint64)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(number)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(boolean)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(string)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(null)
DEFINE_ENCODED_DOC_ADD_PROP_ARRAY_TYPE_DECODED(object)

#define DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(name, built_in_type)                                                    \
bool encoded_doc_array_push_##name(encoded_doc *doc, archive_field_sid_t key, const built_in_type *array, u32 array_length);

#define DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(name, built_in_type)                                            \
bool encoded_doc_array_push_##name##_decoded(encoded_doc *doc, const char *key, const built_in_type *array, u32 array_length);

DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int8, archive_field_i8_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int16, archive_field_i16_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int32, archive_field_i32_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(int64, archive_field_i64_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint8, archive_field_u8_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint16, archive_field_u16_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint32, archive_field_u32_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(uint64, archive_field_u64_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(number, archive_field_number_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(boolean, archive_field_boolean_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(string, archive_field_sid_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE(null, archive_field_u32_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int8, archive_field_i8_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int16, archive_field_i16_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int32, archive_field_i32_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(int64, archive_field_i64_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint8, archive_field_u8_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint16, archive_field_u16_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint32, archive_field_u32_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(uint64, archive_field_u64_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(number, archive_field_number_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(boolean, archive_field_boolean_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(string, archive_field_sid_t)
DEFINE_ENCODED_DOC_ARRAY_PUSH_TYPE_DECODED(null, archive_field_u32_t)

bool encoded_doc_array_push_object(encoded_doc *doc, archive_field_sid_t key, unique_id_t id);
bool encoded_doc_array_push_object_decoded(encoded_doc *doc, const char *key, unique_id_t id);
bool encoded_doc_print(FILE *file, encoded_doc *doc);

END_DECL

#endif
