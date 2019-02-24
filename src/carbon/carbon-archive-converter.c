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

#include "inttypes.h"

#include "carbon/carbon-common.h"
#include "carbon/carbon-archive-visitor.h"
#include "carbon/carbon-query.h"
#include "carbon/carbon-archive-converter.h"

typedef struct
{
    carbon_encoded_doc_collection_t *collection;
} capture_t;

static void
before_visit_starts(carbon_archive_t *archive, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(capture);
    printf("{ ");
}

static void
after_visit_ends(carbon_archive_t *archive, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(capture);

    printf("}\n");
}

#define DECLARE_VISIT_BASIC_TYPE_PAIR(name, built_in_type, format_str)                                                 \
static void                                                                                                            \
visit_##name##_pairs (carbon_archive_t *archive, path_stack_t path_stack,                                              \
                  const carbon_string_id_t *keys, const built_in_type *values, uint32_t num_pairs, void *capture)      \
{                                                                                                                      \
    CARBON_UNUSED(capture);                                                                                            \
    CARBON_UNUSED(path_stack);                                                                                         \
    carbon_query_t query;                                                                                              \
    carbon_archive_query(&query, archive);                                                                             \
    for (uint32_t i = 0; i < num_pairs; i++) {                                                                         \
        char *key = carbon_query_fetch_string_by_id(&query, keys[i]);                                                  \
        printf("\"%s\": %" format_str "%s ", key, values[i], i + 1 < num_pairs ? "," : "");                            \
        free(key);                                                                                                     \
    }                                                                                                                  \
}

DECLARE_VISIT_BASIC_TYPE_PAIR(int8, carbon_int8_t, PRIi8)
DECLARE_VISIT_BASIC_TYPE_PAIR(int16, carbon_int16_t, PRIi16)
DECLARE_VISIT_BASIC_TYPE_PAIR(int32, carbon_int32_t, PRIi32)
DECLARE_VISIT_BASIC_TYPE_PAIR(int64, carbon_int64_t, PRIi64)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint8, carbon_uint8_t, PRIu8)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint16, carbon_uint16_t, PRIu16)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint32, carbon_uint32_t, PRIu32)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint64, carbon_uint64_t, PRIu64)
DECLARE_VISIT_BASIC_TYPE_PAIR(number, carbon_number_t, "f")

static void
visit_boolean_pairs (carbon_archive_t *archive, path_stack_t path_stack,
                     const carbon_string_id_t *keys, const carbon_boolean_t *values, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(path_stack);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    for (uint32_t i = 0; i < num_pairs; i++) {
        char *key = carbon_query_fetch_string_by_id(&query, keys[i]);
        printf("\"%s\": %s%s ", key, values[i] ? "true" : "false", i + 1 < num_pairs ? "," : "");
        free(key);
    }
}

static void
visit_string_pairs (carbon_archive_t *archive, path_stack_t path_stack,
                    const carbon_string_id_t *keys, const carbon_string_id_t *values, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(path_stack);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    for (uint32_t i = 0; i < num_pairs; i++) {
        char *key = carbon_query_fetch_string_by_id(&query, keys[i]);
        char *value = carbon_query_fetch_string_by_id(&query, values[i]);
        printf("\"%s\": \"%s\"%s ", key, value, i + 1 < num_pairs ? "," : "");
        free(key);
        free(value);
    }
}

static void
visit_null_pairs (carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                  uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    for (uint32_t i = 0; i < num_pairs; i++) {
        char *key = carbon_query_fetch_string_by_id(&query, keys[i]);
        printf("\"%s\": null%s ", key, i + 1 < num_pairs ? "," : "");
        free(key);
    }
}


static carbon_visitor_policy_e
before_object_visit(carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t id,
                    uint32_t object_idx, uint32_t num_objects, carbon_string_id_t key, void *capture)
{
    CARBON_UNUSED(path_stack);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(id);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    char *key_str = carbon_query_fetch_string_by_id(&query, key);
    printf("\"%s\": { ", key_str);
    free(key_str);
    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
after_object_visit(carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t id,
                   uint32_t object_idx, uint32_t num_objects, void *capture)
{
    CARBON_UNUSED(id);
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path_stack);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);

    printf("}%s", object_idx + 1 < num_objects ? ", " : "");
}

static void
first_prop_type_group(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                      carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(type);
    CARBON_UNUSED(is_array);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
}

static void
next_prop_type_group(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                     carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(type);
    CARBON_UNUSED(is_array);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    printf(", ");
}

#define DECLARE_VISIT_ARRAY_TYPE(name, built_in_type, format_str)                                                      \
static carbon_visitor_policy_e                                                                                         \
visit_enter_##name##_array_pairs(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,         \
                             uint32_t num_pairs, void *capture)                                                        \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(keys);                                                                                               \
    CARBON_UNUSED(num_pairs);                                                                                          \
    CARBON_UNUSED(capture);                                                                                            \
    return CARBON_VISITOR_POLICY_INCLUDE;                                                                              \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_##name##_array_pair(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t key,                  \
                      uint32_t entry_idx, uint32_t max_entries, const built_in_type *array,                            \
                      uint32_t array_length, void *capture)                                                            \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(key);                                                                                                \
    CARBON_UNUSED(entry_idx);                                                                                          \
    CARBON_UNUSED(max_entries);                                                                                        \
    CARBON_UNUSED(array);                                                                                              \
    CARBON_UNUSED(array_length);                                                                                       \
    CARBON_UNUSED(capture);                                                                                            \
    for (uint32_t i = 0; i < array_length; i++) {                                                                      \
        printf("%" format_str "%s", array[i], i + 1 < array_length ? ", " : "");                                       \
    }                                                                                                                  \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_leave_##name##_array_pair(carbon_archive_t *archive, path_stack_t path, uint32_t pair_idx, uint32_t num_pairs,   \
                                void *capture)                                                                         \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(pair_idx);                                                                                           \
    CARBON_UNUSED(num_pairs);                                                                                          \
    CARBON_UNUSED(capture);                                                                                            \
    printf("]%s", pair_idx + 1 < num_pairs ? ", " : "");                                                               \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_leave_##name##_array_pairs(carbon_archive_t *archive, path_stack_t path, void *capture)                          \
{                                                                                                                      \
                                                                                                                       \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(capture);                                                                                            \
}                                                                                                                      \
static void                                                                                                            \
visit_enter_##name##_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_string_id_t key,                  \
                            uint32_t entry_idx, uint32_t num_elems, void *capture)                                     \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(key);                                                                                                \
    CARBON_UNUSED(entry_idx);                                                                                          \
    CARBON_UNUSED(num_elems);                                                                                          \
    CARBON_UNUSED(capture);                                                                                            \
    carbon_query_t query;                                                                                              \
    carbon_archive_query(&query, archive);                                                                             \
    char *key_str = carbon_query_fetch_string_by_id(&query, key);                                                      \
                                                                                                                       \
    printf("\"%s\": [", key_str);                                                                                      \
                                                                                                                       \
    free(key_str);                                                                                                     \
}



DECLARE_VISIT_ARRAY_TYPE(int8, carbon_int8_t, PRIi8)
DECLARE_VISIT_ARRAY_TYPE(int16, carbon_int16_t, PRIi16)
DECLARE_VISIT_ARRAY_TYPE(int32, carbon_int32_t, PRIi32)
DECLARE_VISIT_ARRAY_TYPE(int64, carbon_int64_t, PRIi64)
DECLARE_VISIT_ARRAY_TYPE(uint8, carbon_uint8_t, PRIu8)
DECLARE_VISIT_ARRAY_TYPE(uint16, carbon_uint16_t, PRIu16)
DECLARE_VISIT_ARRAY_TYPE(uint32, carbon_uint32_t, PRIu32)
DECLARE_VISIT_ARRAY_TYPE(uint64, carbon_uint64_t, PRIu64)
DECLARE_VISIT_ARRAY_TYPE(number, carbon_number_t, "f")


static carbon_visitor_policy_e
visit_enter_string_array_pairs(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                               uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_string_array_pair(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t key,
                        uint32_t entry_idx, uint32_t max_entries, const carbon_string_id_t *array,
                        uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);

    for (uint32_t i = 0; i < array_length; i++) {
        char *value_str = carbon_query_fetch_string_by_id(&query, array[i]);
        printf("\"%s\"%s", value_str, i + 1 < array_length ? ", " : "");
        free(value_str);
    }
}

static void
visit_leave_string_array_pair(carbon_archive_t *archive, path_stack_t path, uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    printf("]%s", pair_idx + 1 < num_pairs ? ", " : "");
}

static void
visit_leave_string_array_pairs(carbon_archive_t *archive, path_stack_t path, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
}
static void
visit_enter_string_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_string_id_t key,
                              uint32_t entry_idx, uint32_t num_elems, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    char *key_str = carbon_query_fetch_string_by_id(&query, key);

    printf("\"%s\": [", key_str);

    free(key_str);
}





static carbon_visitor_policy_e
visit_enter_boolean_array_pairs(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                                uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_boolean_array_pair(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t key,
                         uint32_t entry_idx, uint32_t max_entries, const carbon_boolean_t *array,
                         uint32_t array_length, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(array);
    CARBON_UNUSED(array_length);
    CARBON_UNUSED(capture);

    for (uint32_t i = 0; i < array_length; i++) {
        printf("%s%s", array[i] ? "true" : "false", i + 1 < array_length ? ", " : "");
    }
}

static void
visit_leave_boolean_array_pair(carbon_archive_t *archive, path_stack_t path, uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    printf("]%s", pair_idx + 1 < num_pairs ? ", " : "");
}

static void
visit_leave_boolean_array_pairs(carbon_archive_t *archive, path_stack_t path, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
}
static void
visit_enter_boolean_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_string_id_t key,
                               uint32_t entry_idx, uint32_t num_elems, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    char *key_str = carbon_query_fetch_string_by_id(&query, key);

    printf("\"%s\": [", key_str);

    free(key_str);
}




static carbon_visitor_policy_e
visit_enter_null_array_pairs(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t *keys,
                             uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_null_array_pair(carbon_archive_t *archive, path_stack_t path, const carbon_string_id_t key,
                      uint32_t entry_idx, uint32_t max_entries, uint32_t num_nulls, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(num_nulls);
    CARBON_UNUSED(capture);

    for (uint32_t i = 0; i < num_nulls; i++) {
        printf("null%s", i + 1 < num_nulls ? ", " : "");
    }
}

static void
visit_leave_null_array_pair(carbon_archive_t *archive, path_stack_t path, uint32_t pair_idx, uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(pair_idx);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    printf("]%s", pair_idx + 1 < num_pairs ? ", " : "");
}

static void
visit_leave_null_array_pairs(carbon_archive_t *archive, path_stack_t path, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(capture);
}
static void
visit_enter_null_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_string_id_t key,
                            uint32_t entry_idx, uint32_t num_elems, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(num_elems);
    CARBON_UNUSED(capture);

    carbon_query_t query;
    carbon_archive_query(&query, archive);
    char *key_str = carbon_query_fetch_string_by_id(&query, key);

    printf("\"%s\": [", key_str);

    free(key_str);
}

CARBON_EXPORT(bool)
carbon_archive_converter(carbon_encoded_doc_collection_t *collection, carbon_archive_t *archive)
{

    CARBON_NON_NULL_OR_ERROR(collection);
    CARBON_NON_NULL_OR_ERROR(archive);

    carbon_encoded_doc_collection_create(collection, &archive->err, archive);

    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = CARBON_ARCHIVE_ITER_MASK_ANY };
    capture_t capture = {
        .collection = collection
    };

    visitor.before_visit_starts = before_visit_starts;
    visitor.after_visit_ends    = after_visit_ends;
    visitor.before_object_visit = before_object_visit;
    visitor.after_object_visit  = after_object_visit;
    visitor.visit_int8_pairs    = visit_int8_pairs;
    visitor.visit_int16_pairs   = visit_int16_pairs;
    visitor.visit_int32_pairs   = visit_int32_pairs;
    visitor.visit_int64_pairs   = visit_int64_pairs;
    visitor.visit_uint8_pairs   = visit_uint8_pairs;
    visitor.visit_uint16_pairs  = visit_uint16_pairs;
    visitor.visit_uint32_pairs  = visit_uint32_pairs;
    visitor.visit_uint64_pairs  = visit_uint64_pairs;
    visitor.visit_number_pairs  = visit_number_pairs;
    visitor.visit_string_pairs  = visit_string_pairs;
    visitor.visit_boolean_pairs = visit_boolean_pairs;
    visitor.visit_null_pairs    = visit_null_pairs;
    visitor.first_prop_type_group = first_prop_type_group;
    visitor.next_prop_type_group = next_prop_type_group;

    visitor.visit_enter_int8_array_pairs = visit_enter_int8_array_pairs;
    visitor.visit_enter_int8_array_pair = visit_enter_int8_array_pair;
    visitor.visit_int8_array_pair = visit_int8_array_pair;
    visitor.visit_leave_int8_array_pair = visit_leave_int8_array_pair;
    visitor.visit_leave_int8_array_pairs = visit_leave_int8_array_pairs;

    visitor.visit_enter_int16_array_pairs = visit_enter_int16_array_pairs;
    visitor.visit_enter_int16_array_pair = visit_enter_int16_array_pair;
    visitor.visit_int16_array_pair = visit_int16_array_pair;
    visitor.visit_leave_int16_array_pair = visit_leave_int16_array_pair;
    visitor.visit_leave_int16_array_pairs = visit_leave_int16_array_pairs;

    visitor.visit_enter_int32_array_pairs = visit_enter_int32_array_pairs;
    visitor.visit_enter_int32_array_pair = visit_enter_int32_array_pair;
    visitor.visit_int32_array_pair = visit_int32_array_pair;
    visitor.visit_leave_int32_array_pair = visit_leave_int32_array_pair;
    visitor.visit_leave_int32_array_pairs = visit_leave_int32_array_pairs;

    visitor.visit_enter_int64_array_pairs = visit_enter_int64_array_pairs;
    visitor.visit_enter_int64_array_pair = visit_enter_int64_array_pair;
    visitor.visit_int64_array_pair = visit_int64_array_pair;
    visitor.visit_leave_int64_array_pair = visit_leave_int64_array_pair;
    visitor.visit_leave_int64_array_pairs = visit_leave_int64_array_pairs;

    visitor.visit_enter_uint8_array_pairs = visit_enter_uint8_array_pairs;
    visitor.visit_enter_uint8_array_pair = visit_enter_uint8_array_pair;
    visitor.visit_uint8_array_pair = visit_uint8_array_pair;
    visitor.visit_leave_uint8_array_pair = visit_leave_uint8_array_pair;
    visitor.visit_leave_uint8_array_pairs = visit_leave_uint8_array_pairs;

    visitor.visit_enter_uint16_array_pairs = visit_enter_uint16_array_pairs;
    visitor.visit_enter_uint16_array_pair = visit_enter_uint16_array_pair;
    visitor.visit_uint16_array_pair = visit_uint16_array_pair;
    visitor.visit_leave_uint16_array_pair = visit_leave_uint16_array_pair;
    visitor.visit_leave_uint16_array_pairs = visit_leave_uint16_array_pairs;

    visitor.visit_enter_uint32_array_pairs = visit_enter_uint32_array_pairs;
    visitor.visit_enter_uint32_array_pair = visit_enter_uint32_array_pair;
    visitor.visit_uint32_array_pair = visit_uint32_array_pair;
    visitor.visit_leave_uint32_array_pair = visit_leave_uint32_array_pair;
    visitor.visit_leave_uint32_array_pairs = visit_leave_uint32_array_pairs;

    visitor.visit_enter_uint64_array_pairs = visit_enter_uint64_array_pairs;
    visitor.visit_enter_uint64_array_pair = visit_enter_uint64_array_pair;
    visitor.visit_uint64_array_pair = visit_uint64_array_pair;
    visitor.visit_leave_uint64_array_pair = visit_leave_uint64_array_pair;
    visitor.visit_leave_uint64_array_pairs = visit_leave_uint64_array_pairs;

    visitor.visit_enter_boolean_array_pairs = visit_enter_boolean_array_pairs;
    visitor.visit_enter_boolean_array_pair = visit_enter_boolean_array_pair;
    visitor.visit_boolean_array_pair = visit_boolean_array_pair;
    visitor.visit_leave_boolean_array_pair = visit_leave_boolean_array_pair;
    visitor.visit_leave_boolean_array_pairs = visit_leave_boolean_array_pairs;

    visitor.visit_enter_number_array_pairs = visit_enter_number_array_pairs;
    visitor.visit_enter_number_array_pair = visit_enter_number_array_pair;
    visitor.visit_number_array_pair = visit_number_array_pair;
    visitor.visit_leave_number_array_pair = visit_leave_number_array_pair;
    visitor.visit_leave_number_array_pairs = visit_leave_number_array_pairs;

    visitor.visit_enter_null_array_pairs = visit_enter_null_array_pairs;
    visitor.visit_enter_null_array_pair = visit_enter_null_array_pair;
    visitor.visit_null_array_pair = visit_null_array_pair;
    visitor.visit_leave_null_array_pair = visit_leave_null_array_pair;
    visitor.visit_leave_null_array_pairs = visit_leave_null_array_pairs;

    visitor.visit_enter_string_array_pairs = visit_enter_string_array_pairs;
    visitor.visit_enter_string_array_pair = visit_enter_string_array_pair;
    visitor.visit_string_array_pair = visit_string_array_pair;
    visitor.visit_leave_string_array_pair = visit_leave_string_array_pair;
    visitor.visit_leave_string_array_pairs = visit_leave_string_array_pairs;

    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);

    return true;
}


