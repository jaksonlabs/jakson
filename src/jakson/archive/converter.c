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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/archive/visitor.h>
#include <jakson/archive/converter.h>

struct converter_capture {
        encoded_doc_list *collection;
};

#define IMPORT_BASIC_PAIR(name)                                                                                        \
{                                                                                                                      \
    UNUSED(archive);                                                                                            \
    UNUSED(path_stack);                                                                                         \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, oid);                          \
    for (u32 i = 0; i < num_pairs; i++) {                                                                         \
        encoded_doc_add_prop_##name(doc, keys[i], values[i]);                                                   \
    }                                                                                                                  \
}

#define DECLARE_VISIT_BASIC_TYPE_PAIR(name, built_in_type)                                                             \
static void                                                                                                            \
visit_##name##_pairs (archive *archive, path_stack_t path_stack, unique_id_t oid,                      \
                  const archive_field_sid_t *keys, const built_in_type *values, u32 num_pairs, void *capture)      \
{                                                                                                                      \
    IMPORT_BASIC_PAIR(name)                                                                                            \
}

#define DECLARE_VISIT_ARRAY_TYPE(name, built_in_type)                                                                  \
static visit_policy_e                                                                                         \
visit_enter_##name##_array_pairs(archive *archive, path_stack_t path, unique_id_t id,                  \
                                 const archive_field_sid_t *keys, u32 num_pairs, void *capture)                    \
{                                                                                                                      \
    UNUSED(archive);                                                                                            \
    UNUSED(path);                                                                                               \
    UNUSED(id);                                                                                                 \
    UNUSED(keys);                                                                                               \
    UNUSED(num_pairs);                                                                                          \
    UNUSED(capture);                                                                                            \
                                                                                                                       \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    for (u32 i = 0; i < num_pairs; i++)                                                                           \
    {                                                                                                                  \
        encoded_doc_add_prop_array_##name(doc, keys[i]);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    return VISIT_INCLUDE;                                                                              \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_##name##_array_pair(archive *archive, path_stack_t path, unique_id_t id,                         \
                          const archive_field_sid_t key, u32 entry_idx, u32 max_entries,                      \
                          const built_in_type *array, u32 array_length, void *capture)                            \
{                                                                                                                      \
    UNUSED(archive);                                                                                            \
    UNUSED(path);                                                                                               \
    UNUSED(id);                                                                                                 \
    UNUSED(key);                                                                                                \
    UNUSED(entry_idx);                                                                                          \
    UNUSED(max_entries);                                                                                        \
    UNUSED(capture);                                                                                            \
                                                                                                                       \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    encoded_doc_array_push_##name(doc, key, array, array_length);                                               \
}                                                                                                                      \


static void visit_root_object(archive *archive, unique_id_t id, void *capture)
{
        UNUSED(archive);
        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc_collection_get_or_append(extra->collection, id);
}

DECLARE_VISIT_BASIC_TYPE_PAIR(int8, archive_field_i8_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int16, archive_field_i16_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int32, archive_field_i32_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int64, archive_field_i64_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint8, archive_field_u8_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint16, archive_field_u16_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint32, archive_field_u32_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint64, archive_field_u64_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(number, archive_field_number_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(boolean, archive_field_boolean_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(string, archive_field_sid_t)

static void visit_null_pairs(archive *archive, path_stack_t path, unique_id_t oid,
                             const archive_field_sid_t *keys,
                             u32 num_pairs, void *capture)
{
        UNUSED(archive);
        UNUSED(path);
        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, oid);
        for (u32 i = 0; i < num_pairs; i++) {
                encoded_doc_add_prop_null(doc, keys[i]);
        }
}

static visit_policy_e
before_object_visit(archive *archive, path_stack_t path_stack, unique_id_t parent_id,
                    unique_id_t value_id, u32 object_idx, u32 num_objects, archive_field_sid_t key,
                    void *capture)
{
        UNUSED(archive);
        UNUSED(path_stack);
        UNUSED(object_idx);
        UNUSED(num_objects);
        UNUSED(key);
        UNUSED(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc *parent_doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
        encoded_doc *child_doc = encoded_doc_collection_get_or_append(extra->collection, value_id);
        encoded_doc_add_prop_object(parent_doc, key, child_doc);

        return VISIT_INCLUDE;
}

DECLARE_VISIT_ARRAY_TYPE(int8, archive_field_i8_t)

DECLARE_VISIT_ARRAY_TYPE(int16, archive_field_i16_t)

DECLARE_VISIT_ARRAY_TYPE(int32, archive_field_i32_t)

DECLARE_VISIT_ARRAY_TYPE(int64, archive_field_i64_t)

DECLARE_VISIT_ARRAY_TYPE(uint8, archive_field_u8_t)

DECLARE_VISIT_ARRAY_TYPE(uint16, archive_field_u16_t)

DECLARE_VISIT_ARRAY_TYPE(uint32, archive_field_u32_t)

DECLARE_VISIT_ARRAY_TYPE(uint64, archive_field_u64_t)

DECLARE_VISIT_ARRAY_TYPE(number, archive_field_number_t)

DECLARE_VISIT_ARRAY_TYPE(boolean, archive_field_boolean_t)

DECLARE_VISIT_ARRAY_TYPE(string, archive_field_sid_t)

static visit_policy_e
visit_enter_null_array_pairs(archive *archive, path_stack_t path, unique_id_t id,
                             const archive_field_sid_t *keys, u32 num_pairs, void *capture)
{
        UNUSED(archive);
        UNUSED(path);
        UNUSED(id);
        UNUSED(keys);
        UNUSED(num_pairs);
        UNUSED(capture);

        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);
        for (u32 i = 0; i < num_pairs; i++) {
                encoded_doc_add_prop_array_null(doc, keys[i]);
        }

        return VISIT_INCLUDE;
}

static void visit_null_array_pair(archive *archive, path_stack_t path, unique_id_t id,
                                  const archive_field_sid_t key,
                                  u32 entry_idx, u32 max_entries, u32 num_nulls, void *capture)
{
        UNUSED(archive);
        UNUSED(path);
        UNUSED(id);
        UNUSED(key);
        UNUSED(entry_idx);
        UNUSED(max_entries);
        UNUSED(num_nulls);
        UNUSED(capture);

        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);
        encoded_doc_array_push_null(doc, key, &num_nulls, 1);
}

static void
before_visit_object_array_objects(bool *skip_group_object_ids, archive *archive, path_stack_t path,
                                  unique_id_t parent_id, archive_field_sid_t key,
                                  const unique_id_t *group_object_ids, u32 num_group_object_ids,
                                  void *capture)
{
        UNUSED(archive);
        UNUSED(path);
        UNUSED(parent_id);
        UNUSED(capture);
        UNUSED(group_object_ids);
        UNUSED(skip_group_object_ids);
        UNUSED(num_group_object_ids);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
        encoded_doc_add_prop_array_object(doc, key);
        for (u32 i = 0; i < num_group_object_ids; i++) {
                encoded_doc_array_push_object(doc, key, group_object_ids[i]);
        }
}

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(name, built_in_type)                                             \
static void                                                                                                            \
visit_object_array_object_property_##name(archive *archive, path_stack_t path,                                \
                                           unique_id_t parent_id,                                               \
                                           archive_field_sid_t key,                                                     \
                                           unique_id_t nested_object_id,                                        \
                                           archive_field_sid_t nested_key,                                              \
                                           const built_in_type *nested_values,                                         \
                                           u32 num_nested_values, void *capture)                                  \
{                                                                                                                      \
    UNUSED(archive);                                                                                            \
    UNUSED(path);                                                                                               \
    UNUSED(parent_id);                                                                                          \
    UNUSED(key);                                                                                                \
    UNUSED(nested_key);                                                                                         \
    UNUSED(nested_values);                                                                                      \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
        encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, nested_object_id);             \
        encoded_doc_add_prop_array_##name(doc, nested_key);                                                                                                   \
        encoded_doc_array_push_##name(doc, nested_key, nested_values, num_nested_values);                           \
}

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int8, archive_field_i8_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int16, archive_field_i16_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int32, archive_field_i32_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int64, archive_field_i64_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint8, archive_field_u8_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint16, archive_field_u16_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint32, archive_field_u32_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint64, archive_field_u64_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(number, archive_field_number_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(string, archive_field_sid_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(boolean, archive_field_boolean_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(null, archive_field_u32_t);

bool archive_converter(encoded_doc_list *collection, archive *archive)
{

        ERROR_IF_NULL(collection);
        ERROR_IF_NULL(archive);

        encoded_doc_collection_create(collection, &archive->err, archive);

        visitor visitor = {0};
        archive_visitor_desc desc = {.visit_mask = ARCHIVE_ITER_MASK_ANY};
        struct converter_capture capture = {.collection = collection};

        visitor.visit_root_object = visit_root_object;
        visitor.before_object_visit = before_object_visit;
        visitor.visit_int8_pairs = visit_int8_pairs;
        visitor.visit_int16_pairs = visit_int16_pairs;
        visitor.visit_int32_pairs = visit_int32_pairs;
        visitor.visit_int64_pairs = visit_int64_pairs;
        visitor.visit_uint8_pairs = visit_uint8_pairs;
        visitor.visit_uint16_pairs = visit_uint16_pairs;
        visitor.visit_uint32_pairs = visit_uint32_pairs;
        visitor.visit_uint64_pairs = visit_uint64_pairs;
        visitor.visit_number_pairs = visit_number_pairs;
        visitor.visit_string_pairs = visit_string_pairs;
        visitor.visit_boolean_pairs = visit_boolean_pairs;
        visitor.visit_null_pairs = visit_null_pairs;

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

        archive_visit_archive(archive, &desc, &visitor, &capture);

        return true;
}


