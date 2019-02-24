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
#include "carbon/carbon-archive-converter.h"

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
    for (uint32_t i = 0; i < num_pairs; i++) {                                                                         \
        carbon_encoded_doc_add_prop_##name(doc, keys[i], values[i]);                                                   \
    }                                                                                                                  \
}

#define DECLARE_VISIT_BASIC_TYPE_PAIR(name, built_in_type)                                                             \
static void                                                                                                            \
visit_##name##_pairs (carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t oid,                      \
                  const carbon_string_id_t *keys, const built_in_type *values, uint32_t num_pairs, void *capture)      \
{                                                                                                                      \
    IMPORT_BASIC_PAIR(name)                                                                                            \
}

#define DECLARE_VISIT_ARRAY_TYPE(name, built_in_type)                                                                  \
static carbon_visitor_policy_e                                                                                         \
visit_enter_##name##_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                  \
                                 const carbon_string_id_t *keys, uint32_t num_pairs, void *capture)                    \
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
    for (uint32_t i = 0; i < num_pairs; i++)                                                                           \
    {                                                                                                                  \
        carbon_encoded_doc_add_prop_array_##name(doc, keys[i]);                                                        \
    }                                                                                                                  \
                                                                                                                       \
    return CARBON_VISITOR_POLICY_INCLUDE;                                                                              \
}                                                                                                                      \
                                                                                                                       \
static void                                                                                                            \
visit_##name##_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                         \
                          const carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,                      \
                          const built_in_type *array, uint32_t array_length, void *capture)                            \
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

DECLARE_VISIT_BASIC_TYPE_PAIR(int8, carbon_int8_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(int16, carbon_int16_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(int32, carbon_int32_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(int64, carbon_int64_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint8, carbon_uint8_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint16, carbon_uint16_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint32, carbon_uint32_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(uint64, carbon_uint64_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(number, carbon_number_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(boolean, carbon_boolean_t)
DECLARE_VISIT_BASIC_TYPE_PAIR(string, carbon_string_id_t)

static void
visit_null_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t oid, const carbon_string_id_t *keys,
                  uint32_t num_pairs, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    assert(capture);

    capture_t *extra = (capture_t *) capture;
    carbon_encoded_doc_t *doc = encoded_doc_collection_get_or_append(extra->collection, oid);
    for (uint32_t i = 0; i < num_pairs; i++) {
        carbon_encoded_doc_add_prop_null(doc, keys[i]);
    }
}

static carbon_visitor_policy_e
before_object_visit(carbon_archive_t *archive, path_stack_t path_stack, carbon_object_id_t parent_id,
                    carbon_object_id_t value_id, uint32_t object_idx, uint32_t num_objects, carbon_string_id_t key, void *capture)
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

DECLARE_VISIT_ARRAY_TYPE(int8, carbon_int8_t)
DECLARE_VISIT_ARRAY_TYPE(int16, carbon_int16_t)
DECLARE_VISIT_ARRAY_TYPE(int32, carbon_int32_t)
DECLARE_VISIT_ARRAY_TYPE(int64, carbon_int64_t)
DECLARE_VISIT_ARRAY_TYPE(uint8, carbon_uint8_t)
DECLARE_VISIT_ARRAY_TYPE(uint16, carbon_uint16_t)
DECLARE_VISIT_ARRAY_TYPE(uint32, carbon_uint32_t)
DECLARE_VISIT_ARRAY_TYPE(uint64, carbon_uint64_t)
DECLARE_VISIT_ARRAY_TYPE(number, carbon_number_t)
DECLARE_VISIT_ARRAY_TYPE(boolean, carbon_boolean_t)
DECLARE_VISIT_ARRAY_TYPE(string, carbon_string_id_t)

static carbon_visitor_policy_e
visit_enter_null_array_pairs(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                             uint32_t num_pairs, void *capture)
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
    for (uint32_t i = 0; i < num_pairs; i++)
    {
        carbon_encoded_doc_add_prop_array_null(doc, keys[i]);
    }

    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_null_array_pair(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t key,
                      uint32_t entry_idx, uint32_t max_entries, uint32_t num_nulls, void *capture)
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
    carbon_encoded_doc_array_push_null(doc, key, num_nulls);
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
    
    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);

    return true;
}


