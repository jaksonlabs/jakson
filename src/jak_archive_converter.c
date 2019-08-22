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

#include <jak_stdinc.h>
#include <jak_archive_visitor.h>
#include <jak_archive_converter.h>

struct converter_capture {
    struct jak_encoded_doc_list *collection;
};

#define IMPORT_BASIC_PAIR(name)                                                                                        \
{                                                                                                                      \
    JAK_UNUSED(jak_archive);                                                                                            \
    JAK_UNUSED(path_stack);                                                                                         \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, oid);                          \
    for (jak_u32 i = 0; i < num_pairs; i++) {                                                                         \
        encoded_doc_add_prop_##name(doc, keys[i], values[i]);                                                   \
    }                                                                                                                  \
}

#define DECLARE_VISIT_BASIC_TYPE_PAIR(name, built_in_type)                                                             \
static void                                                                                                            \
visit_##name##_pairs (struct jak_archive *jak_archive, path_stack_t path_stack, jak_global_id_t oid,                      \
                  const jak_archive_field_sid_t *keys, const built_in_type *values, jak_u32 num_pairs, void *capture)      \
{                                                                                                                      \
    IMPORT_BASIC_PAIR(name)                                                                                            \
}

#define DECLARE_VISIT_ARRAY_TYPE(name, built_in_type)                                                                  \
static enum jak_visit_policy                                                                                         \
visit_enter_##name##_array_pairs(struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                  \
                                 const jak_archive_field_sid_t *keys, jak_u32 num_pairs, void *capture)                    \
{                                                                                                                      \
    JAK_UNUSED(jak_archive);                                                                                            \
    JAK_UNUSED(path);                                                                                               \
    JAK_UNUSED(id);                                                                                                 \
    JAK_UNUSED(keys);                                                                                               \
    JAK_UNUSED(num_pairs);                                                                                          \
    JAK_UNUSED(capture);                                                                                            \
                                                                                                                       \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    for (jak_u32 i = 0; i < num_pairs; i++)                                                                           \
    {                                                                                                                  \
        encoded_doc_add_prop_array_##name(doc, keys[i]);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    return JAK_VISIT_INCLUDE;                                                                              \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_##name##_array_pair(struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                         \
                          const jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 max_entries,                      \
                          const built_in_type *array, jak_u32 array_length, void *capture)                            \
{                                                                                                                      \
    JAK_UNUSED(jak_archive);                                                                                            \
    JAK_UNUSED(path);                                                                                               \
    JAK_UNUSED(id);                                                                                                 \
    JAK_UNUSED(key);                                                                                                \
    JAK_UNUSED(entry_idx);                                                                                          \
    JAK_UNUSED(max_entries);                                                                                        \
    JAK_UNUSED(capture);                                                                                            \
                                                                                                                       \
    JAK_ASSERT(capture);                                                                                                   \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
    struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);                           \
    encoded_doc_array_push_##name(doc, key, array, array_length);                                               \
}                                                                                                                      \


static void visit_root_object(struct jak_archive *archive, jak_global_id_t id, void *capture)
{
        JAK_UNUSED(archive);
        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        encoded_doc_collection_get_or_append(extra->collection, id);
}

DECLARE_VISIT_BASIC_TYPE_PAIR(int8, jak_archive_field_i8_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int16, jak_archive_field_i16_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int32, jak_archive_field_i32_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(int64, jak_archive_field_i64_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint8, jak_archive_field_u8_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint16, jak_archive_field_u16_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint32, jak_archive_field_u32_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(uint64, jak_archive_field_u64_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(number, jak_archive_field_number_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(boolean, jak_archive_field_boolean_t)

DECLARE_VISIT_BASIC_TYPE_PAIR(string, jak_archive_field_sid_t)

static void visit_null_pairs(struct jak_archive *archive, path_stack_t path, jak_global_id_t oid, const jak_archive_field_sid_t *keys,
                             jak_u32 num_pairs, void *capture)
{
        JAK_UNUSED(archive);
        JAK_UNUSED(path);
        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, oid);
        for (jak_u32 i = 0; i < num_pairs; i++) {
                encoded_doc_add_prop_null(doc, keys[i]);
        }
}

static enum jak_visit_policy before_object_visit(struct jak_archive *archive, path_stack_t path_stack, jak_global_id_t parent_id,
                                             jak_global_id_t value_id, jak_u32 object_idx, jak_u32 num_objects, jak_archive_field_sid_t key,
                                             void *capture)
{
        JAK_UNUSED(archive);
        JAK_UNUSED(path_stack);
        JAK_UNUSED(object_idx);
        JAK_UNUSED(num_objects);
        JAK_UNUSED(key);
        JAK_UNUSED(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        struct jak_encoded_doc *parent_doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
        struct jak_encoded_doc *child_doc = encoded_doc_collection_get_or_append(extra->collection, value_id);
        encoded_doc_add_prop_object(parent_doc, key, child_doc);

        return JAK_VISIT_INCLUDE;
}

DECLARE_VISIT_ARRAY_TYPE(int8, jak_archive_field_i8_t)

DECLARE_VISIT_ARRAY_TYPE(int16, jak_archive_field_i16_t)

DECLARE_VISIT_ARRAY_TYPE(int32, jak_archive_field_i32_t)

DECLARE_VISIT_ARRAY_TYPE(int64, jak_archive_field_i64_t)

DECLARE_VISIT_ARRAY_TYPE(uint8, jak_archive_field_u8_t)

DECLARE_VISIT_ARRAY_TYPE(uint16, jak_archive_field_u16_t)

DECLARE_VISIT_ARRAY_TYPE(uint32, jak_archive_field_u32_t)

DECLARE_VISIT_ARRAY_TYPE(uint64, jak_archive_field_u64_t)

DECLARE_VISIT_ARRAY_TYPE(number, jak_archive_field_number_t)

DECLARE_VISIT_ARRAY_TYPE(boolean, jak_archive_field_boolean_t)

DECLARE_VISIT_ARRAY_TYPE(string, jak_archive_field_sid_t)

static enum jak_visit_policy visit_enter_null_array_pairs(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                                      const jak_archive_field_sid_t *keys, jak_u32 num_pairs, void *capture)
{
        JAK_UNUSED(archive);
        JAK_UNUSED(path);
        JAK_UNUSED(id);
        JAK_UNUSED(keys);
        JAK_UNUSED(num_pairs);
        JAK_UNUSED(capture);

        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);
        for (jak_u32 i = 0; i < num_pairs; i++) {
                encoded_doc_add_prop_array_null(doc, keys[i]);
        }

        return JAK_VISIT_INCLUDE;
}

static void visit_null_array_pair(struct jak_archive *archive, path_stack_t path, jak_global_id_t id, const jak_archive_field_sid_t key,
                                  jak_u32 entry_idx, jak_u32 max_entries, jak_u32 num_nulls, void *capture)
{
        JAK_UNUSED(archive);
        JAK_UNUSED(path);
        JAK_UNUSED(id);
        JAK_UNUSED(key);
        JAK_UNUSED(entry_idx);
        JAK_UNUSED(max_entries);
        JAK_UNUSED(num_nulls);
        JAK_UNUSED(capture);

        JAK_ASSERT(capture);

        struct converter_capture *extra = (struct converter_capture *) capture;
        struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, id);
        encoded_doc_array_push_null(doc, key, &num_nulls, 1);
}

static void before_visit_object_array_objects(bool *skip_group_object_ids, struct jak_archive *archive, path_stack_t path,
                                              jak_global_id_t parent_id, jak_archive_field_sid_t key,
                                              const jak_global_id_t *group_object_ids, jak_u32 num_group_object_ids,
                                              void *capture)
{
        JAK_UNUSED(archive);
        JAK_UNUSED(path);
        JAK_UNUSED(parent_id);
        JAK_UNUSED(capture);
        JAK_UNUSED(group_object_ids);
        JAK_UNUSED(skip_group_object_ids);
        JAK_UNUSED(num_group_object_ids);

        struct converter_capture *extra = (struct converter_capture *) capture;
        struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, parent_id);
        encoded_doc_add_prop_array_object(doc, key);
        for (jak_u32 i = 0; i < num_group_object_ids; i++) {
                encoded_doc_array_push_object(doc, key, group_object_ids[i]);
        }
}

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(name, built_in_type)                                             \
static void                                                                                                            \
visit_object_array_object_property_##name(struct jak_archive *jak_archive, path_stack_t path,                                \
                                           jak_global_id_t parent_id,                                               \
                                           jak_archive_field_sid_t key,                                                     \
                                           jak_global_id_t nested_object_id,                                        \
                                           jak_archive_field_sid_t nested_key,                                              \
                                           const built_in_type *nested_values,                                         \
                                           jak_u32 num_nested_values, void *capture)                                  \
{                                                                                                                      \
    JAK_UNUSED(jak_archive);                                                                                            \
    JAK_UNUSED(path);                                                                                               \
    JAK_UNUSED(parent_id);                                                                                          \
    JAK_UNUSED(key);                                                                                                \
    JAK_UNUSED(nested_key);                                                                                         \
    JAK_UNUSED(nested_values);                                                                                      \
                                                                                                                       \
    struct converter_capture *extra = (struct converter_capture *) capture;                                                                          \
        struct jak_encoded_doc *doc = encoded_doc_collection_get_or_append(extra->collection, nested_object_id);             \
        encoded_doc_add_prop_array_##name(doc, nested_key);                                                                                                   \
        encoded_doc_array_push_##name(doc, nested_key, nested_values, num_nested_values);                           \
}

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int8, jak_archive_field_i8_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int16, jak_archive_field_i16_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int32, jak_archive_field_i32_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(int64, jak_archive_field_i64_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint8, jak_archive_field_u8_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint16, jak_archive_field_u16_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint32, jak_archive_field_u32_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(uint64, jak_archive_field_u64_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(number, jak_archive_field_number_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(string, jak_archive_field_sid_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(boolean, jak_archive_field_boolean_t);

DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP_HANDLER(null, jak_archive_field_u32_t);

bool jak_archive_converter(struct jak_encoded_doc_list *collection, struct jak_archive *archive)
{

        JAK_ERROR_IF_NULL(collection);
        JAK_ERROR_IF_NULL(archive);

        encoded_doc_collection_create(collection, &archive->err, archive);

        struct jak_archive_visitor visitor = {0};
        struct jak_archive_visitor_desc desc = {.visit_mask = JAK_ARCHIVE_ITER_MASK_ANY};
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

        jak_archive_visit_archive(archive, &desc, &visitor, &capture);

        return true;
}


