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

#include "shared/common.h"
#include "core/carbon/archive_visitor.h"
#include "core/carbon/archive_converter.h"

typedef struct
{
    carbon_encoded_doc_collection_t *collection;
} capture_t;


#define IMPORT_BASIC_PAIR(name)                                                                                        \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path_stack);                                                                                         \
    assert(capture);                                                                                                   \
                                                                                                                       \
    capture_t *extra = (capture_t *) capture;                                                                          \
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, oid);                          \
    for (u32 i = 0; i < num_pairs; i++) {                                                                         \
        carbon_encoded_doc_add_prop_##name(doc, keys[i], values[i]);                                                   \
    }                                                                                                                  \
}

#define DECLARE_VISIT_BASIC_TYPE_PAIR(name, built_in_type)                                                             \
static void                                                                                                            \
visit_##name##_pairs (carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t oid,                      \
                  const carbon_string_id_t *keys, const built_in_type *values, u32 num_pairs, void *capture)      \
{                                                                                                                      \
    IMPORT_BASIC_PAIR(name)                                                                                            \
}

#define DECLARE_VISIT_ARRAY_TYPE(name, built_in_type)                                                                  \
static carbon_visitor_policy_e                                                                                         \
visit_enter_##name##_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                  \
                                 const carbon_string_id_t *keys, u32 num_pairs, void *capture)                    \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(id);                                                                                                 \
    CARBON_UNUSED(keys);                                                                                               \
    CARBON_UNUSED(num_pairs);                                                                                          \
    CARBON_UNUSED(capture);                                                                                            \
                                                                                                                       \
    assert(capture);                                                                                                   \
                                                                                                                       \
    capture_t *extra = (capture_t *) capture;                                                                          \
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    for (u32 i = 0; i < num_pairs; i++)                                                                           \
    {                                                                                                                  \
        carbon_encoded_doc_add_prop_array_##name(doc, keys[i]);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    return CARBON_VISITOR_POLICY_INCLUDE;                                                                              \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_##name##_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                         \
                          const carbon_string_id_t key, u32 entry_idx, u32 max_entries,                      \
                          const built_in_type *array, u32 array_length, void *capture)                            \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(id);                                                                                                 \
    CARBON_UNUSED(key);                                                                                                \
    CARBON_UNUSED(entry_idx);                                                                                          \
    CARBON_UNUSED(max_entries);                                                                                        \
    CARBON_UNUSED(capture);                                                                                            \
                                                                                                                       \
    assert(capture);                                                                                                   \
                                                                                                                       \
    capture_t *extra = (capture_t *) capture;                                                                          \
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    carbon_encoded_doc_array_push_##name(doc, key, array, array_length);                                               \
}                                                                                                                      \


static void
visit_root_object(carbon_archive_t *archive, carbon_object_id_t id, void *capture)
{
    CARBON_UNUSED(archive);
    assert(capture);

    capture_t *extra = (capture_t *) capture;
    encoded_doc_collection_get_or_append(extra->collection, id);
}

DECLARE_VISIT_BASIC_TYPE_PAIR(int8, carbon_i8)
DECLARE_VISIT_BASIC_TYPE_PAIR(int16, carbon_i16)
DECLARE_VISIT_BASIC_TYPE_PAIR(int32, carbon_i32)
DECLARE_VISIT_BASIC_TYPE_PAIR(int64, carbon_i64)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint8, carbon_u8)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint16, carbon_u16)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint32, carbon_u32)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint64, carbon_u64)
DECLARE_VISIT_BASIC_TYPE_PAIR(number, carbon_number_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(boolean, carbon_boolean_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(string, carbon_string_id_t)

static void
visit_null_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t oid, const carbon_string_id_t *keys,
                  u32 num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    assert(capture);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, oid);
    for (u32 i = 0; i < num_pairs; i++) {
        carbon_encoded_doc_add_prop_null(doc, keys[i]);
    }
}

static carbon_visitor_policy_e
before_object_visit(carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t parent_id,
                    carbon_object_id_t value_id, u32 object_idx, u32 num_objects, carbon_string_id_t key, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path_stack);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);
    CARBON_UNUSED(key);
    CARBON_UNUSED(capture);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *parent_doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
    carbon_encoded_doc_t *child_doc = encoded_doc_collection_get_or_append(extra->collection, value_id);
    carbon_encoded_doc_add_prop_object(parent_doc, key, child_doc);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

DECLARE_VISIT_ARRAY_TYPE(int8, carbon_i8)
DECLARE_VISIT_ARRAY_TYPE(int16, carbon_i16)
DECLARE_VISIT_ARRAY_TYPE(int32, carbon_i32)
DECLARE_VISIT_ARRAY_TYPE(int64, carbon_i64)
DECLARE_VISIT_ARRAY_TYPE(uint8, carbon_u8)
DECLARE_VISIT_ARRAY_TYPE(uint16, carbon_u16)
DECLARE_VISIT_ARRAY_TYPE(uint32, carbon_u32)
DECLARE_VISIT_ARRAY_TYPE(uint64, carbon_u64)
DECLARE_VISIT_ARRAY_TYPE(number, carbon_number_t)
DECLARE_VISIT_ARRAY_TYPE(boolean, carbon_boolean_t)
DECLARE_VISIT_ARRAY_TYPE(string, carbon_string_id_t)

static carbon_visitor_policy_e
visit_enter_null_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                             u32 num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    assert(capture);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, id);
    for (u32 i = 0; i < num_pairs; i++)
    {
        carbon_encoded_doc_add_prop_array_null(doc, keys[i]);
    }

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_null_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t key,
                      u32 entry_idx, u32 max_entries, u32 num_nulls, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(entry_idx);
    CARBON_UNUSED(max_entries);
    CARBON_UNUSED(num_nulls);
    CARBON_UNUSED(capture);

    assert(capture);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, id);
    carbon_encoded_doc_array_push_null(doc, key, &num_nulls, 1);
}

static void
before_visit_object_array_objects(bool *skip_group_object_ids, carbon_archive_t *archive, path_stack_t path,
                                  carbon_object_id_t parent_id, carbon_string_id_t key, const carbon_object_id_t *group_object_ids,
                                  u32 num_group_object_ids, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(group_object_ids);
    CARBON_UNUSED(skip_group_object_ids);
    CARBON_UNUSED(num_group_object_ids);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
    carbon_encoded_doc_add_prop_array_object(doc, key);
    for (u32 i = 0; i < num_group_object_ids; i++) {
        carbon_encoded_doc_array_push_object(doc, key, group_object_ids[i]);
    }
}

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(name, built_in_type)                                             \
static void                                                                                                            \
visit_object_array_object_property_##name(carbon_archive_t *archive, path_stack_t path,                                \
                                           carbon_object_id_t parent_id,                                               \
                                           carbon_string_id_t key,                                                     \
                                           carbon_object_id_t nested_object_id,                                        \
                                           carbon_string_id_t nested_key,                                              \
                                           const built_in_type *nested_values,                                         \
                                           u32 num_nested_values, void *capture)                                  \
{                                                                                                                      \
    CARBON_UNUSED(archive);                                                                                            \
    CARBON_UNUSED(path);                                                                                               \
    CARBON_UNUSED(parent_id);                                                                                          \
    CARBON_UNUSED(key);                                                                                                \
    CARBON_UNUSED(nested_key);                                                                                         \
    CARBON_UNUSED(nested_values);                                                                                      \
                                                                                                                       \
    capture_t *extra = (capture_t *) capture;                                                                          \
	carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, nested_object_id);             \
	carbon_encoded_doc_add_prop_array_##name(doc, nested_key);          											   \
	carbon_encoded_doc_array_push_##name(doc, nested_key, nested_values, num_nested_values);                           \
}

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int8, carbon_i8);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int16, carbon_i16);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int32, carbon_i32);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int64, carbon_i64);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint8, carbon_u8);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint16, carbon_u16);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint32, carbon_u32);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint64, carbon_u64);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(number, carbon_number_t);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(string, carbon_string_id_t);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(boolean, carbon_boolean_t);
DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(null, carbon_u32);

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

    visitor.visit_root_object   = visit_root_object;
    visitor.before_object_visit = before_object_visit;
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

    visitor.visit_enter_int8_array_pairs = visit_enter_int8_array_pairs;
    visitor.visit_int8_array_pair = visit_int8_array_pair;
    visitor.visit_enter_int16_array_pairs = visit_enter_int16_array_pairs;
    visitor.visit_int16_array_pair = visit_int16_array_pair;
    visitor.visit_enter_int32_array_pairs = visit_enter_int32_array_pairs;
    visitor.visit_int32_array_pair = visit_int32_array_pair;
    visitor.visit_enter_int64_array_pairs = visit_enter_int64_array_pairs;
    visitor.visit_int64_array_pair = visit_int64_array_pair;
    visitor.visit_enter_uint8_array_pairs = visit_enter_uint8_array_pairs;
    visitor.visit_uint8_array_pair = visit_uint8_array_pair;
    visitor.visit_enter_uint16_array_pairs = visit_enter_uint16_array_pairs;
    visitor.visit_uint16_array_pair = visit_uint16_array_pair;
    visitor.visit_enter_uint32_array_pairs = visit_enter_uint32_array_pairs;
    visitor.visit_uint32_array_pair = visit_uint32_array_pair;
    visitor.visit_enter_uint64_array_pairs = visit_enter_uint64_array_pairs;
    visitor.visit_uint64_array_pair = visit_uint64_array_pair;
    visitor.visit_enter_boolean_array_pairs = visit_enter_boolean_array_pairs;
    visitor.visit_boolean_array_pair = visit_boolean_array_pair;
    visitor.visit_enter_number_array_pairs = visit_enter_number_array_pairs;
    visitor.visit_number_array_pair = visit_number_array_pair;
    visitor.visit_enter_null_array_pairs = visit_enter_null_array_pairs;
    visitor.visit_null_array_pair = visit_null_array_pair;
    visitor.visit_enter_string_array_pairs = visit_enter_string_array_pairs;
    visitor.visit_string_array_pair = visit_string_array_pair;

    visitor.before_visit_object_array_objects = before_visit_object_array_objects;

    visitor.visit_object_array_object_property_int8s = visit_object_array_object_property_int8;
    visitor.visit_object_array_object_property_int16s = visit_object_array_object_property_int16;
    visitor.visit_object_array_object_property_int32s = visit_object_array_object_property_int32;
    visitor.visit_object_array_object_property_int64s = visit_object_array_object_property_int64;
    visitor.visit_object_array_object_property_uint8s = visit_object_array_object_property_uint8;
    visitor.visit_object_array_object_property_uint16s = visit_object_array_object_property_uint16;
    visitor.visit_object_array_object_property_uint32s = visit_object_array_object_property_uint32;
    visitor.visit_object_array_object_property_uint64s = visit_object_array_object_property_uint64;
    visitor.visit_object_array_object_property_numbers = visit_object_array_object_property_number;
    visitor.visit_object_array_object_property_strings = visit_object_array_object_property_string;
    visitor.visit_object_array_object_property_booleans = visit_object_array_object_property_boolean;
    visitor.visit_object_array_object_property_nulls = visit_object_array_object_property_null;

    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);

    return true;
}


