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

#include <carbon/carbon-archive-visitor.h>
#include <carbon/carbon-hashtable.h>
#include <carbon/carbon-hashset.h>
#include "carbon/carbon-archive-visitor.h"
#include "carbon/carbon-query.h"

static void
iterate_props(carbon_archive_t *archive, carbon_archive_prop_iter_t *prop_iter,
              carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object, carbon_string_id_t parent_key, uint32_t parent_key_array_idx);

static void
iterate_objects(carbon_archive_t *archive, const carbon_string_id_t *keys, uint32_t num_pairs,
                carbon_archive_value_vector_t *value_iter,
                carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
                int mask, void *capture, bool is_root_object)
{
    CARBON_UNUSED(num_pairs);

    uint32_t vector_length;
    carbon_archive_object_t object;
    carbon_object_id_t parent_object_id;
    carbon_object_id_t object_id;
    carbon_archive_prop_iter_t  prop_iter;
    carbon_err_t err;

    carbon_archive_value_vector_get_object_id(&parent_object_id, value_iter);
    carbon_archive_value_vector_get_length(&vector_length, value_iter);
    assert(num_pairs == vector_length);

    for (uint32_t i = 0; i < vector_length; i++)
    {
        carbon_string_id_t parent_key = keys[i];
        uint32_t parent_key_array_idx = i;

        carbon_archive_value_vector_get_object_at(&object, i, value_iter);
        carbon_archive_object_get_object_id(&object_id, &object);

        carbon_archive_prop_iter_from_object(&prop_iter, mask, &err, &object);

        if (!is_root_object) {
            carbon_visitor_policy_e visit = CARBON_VISITOR_POLICY_INCLUDE;
            if (visitor->before_object_visit) {
                visit = visitor->before_object_visit(archive, path_stack, parent_object_id, object_id, i,
                                                     vector_length, keys[i], capture);
            }
            if (visit == CARBON_VISITOR_POLICY_INCLUDE) {
                iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false, parent_key, parent_key_array_idx);
                OPTIONAL_CALL(visitor, after_object_visit, archive, path_stack, object_id, i, vector_length, capture);
            }
        } else {
            OPTIONAL_CALL(visitor, visit_root_object, archive, object_id, capture);
            iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false, parent_key, parent_key_array_idx);
        }

    }
}

#define SET_TYPE_SWITCH_CASE(name, built_in_type)                                                                      \
{                                                                                                                      \
    if (is_array) {                                                                                                    \
        carbon_visitor_policy_e visit = CARBON_VISITOR_POLICY_INCLUDE;                                                 \
        if (visitor->visit_enter_##name##_array_pairs) {                                                               \
            visit = visitor->visit_enter_##name##_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);     \
        }                                                                                                              \
        if (visit == CARBON_VISITOR_POLICY_INCLUDE) {                                                                  \
            for (uint32_t prop_idx = 0; prop_idx < num_pairs; prop_idx++)                                              \
            {                                                                                                          \
                uint32_t array_length;                                                                                 \
                const built_in_type *values = carbon_archive_value_vector_get_##name##_arrays_at(&array_length,        \
                                                                                             prop_idx,                 \
                                                                                             &value_iter);             \
                OPTIONAL_CALL(visitor, visit_enter_##name##_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],      \
                              prop_idx, array_length, capture);                                                        \
                OPTIONAL_CALL(visitor, visit_##name##_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],            \
                              prop_idx, num_pairs, values, array_length, capture)                                      \
                OPTIONAL_CALL(visitor, visit_leave_##name##_array_pair, archive, path_stack, this_object_oid, prop_idx, num_pairs, \
                                                                        capture);                                      \
            }                                                                                                          \
            OPTIONAL_CALL(visitor, visit_leave_##name##_array_pairs, archive, path_stack, this_object_oid, capture);               \
        }                                                                                                              \
    } else                                                                                                             \
    {                                                                                                                  \
        if (visitor->visit_##name##_pairs) {                                                                           \
            const built_in_type *values = carbon_archive_value_vector_get_##name##s(NULL, &value_iter);                \
            visitor->visit_##name##_pairs(archive, path_stack, this_object_oid, keys, values, num_pairs, capture);                 \
        }                                                                                                              \
    }                                                                                                                  \
}

#define SET_NESTED_ARRAY_SWITCH_CASE(name, built_in_type)                                                              \
{                                                                                                                      \
    const built_in_type *values = carbon_archive_column_entry_get_##name(&entry_length, &entry_iter);                  \
    OPTIONAL_CALL(visitor, visit_object_array_object_property_##name,                                                  \
                  archive, path_stack, this_object_oid, group_key, current_nested_object_id,                           \
                  current_column_name, values, entry_length, capture);                                                 \
}

static void
iterate_props(carbon_archive_t *archive, carbon_archive_prop_iter_t *prop_iter,
              carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object, carbon_string_id_t parent_key, uint32_t parent_key_array_idx)
{
    carbon_object_id_t this_object_oid;
    carbon_archive_value_vector_t value_iter;
    carbon_basic_type_e type;
    bool is_array;
    const carbon_string_id_t *keys;
    uint32_t num_pairs;
    carbon_archive_prop_iter_mode_e iter_type;
    carbon_archive_collection_iter_t collection_iter;
    bool first_type_group = true;

    carbon_path_entry_t e = { .key = parent_key, .idx = parent_key_array_idx };
    carbon_vec_push(path_stack, &e, 1);

    carbon_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

    while (carbon_archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter)) {

        if (iter_type == CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT) {

            keys = carbon_archive_value_vector_get_keys(&num_pairs, &value_iter);
            carbon_archive_value_vector_is_array_type(&is_array, &value_iter);
            carbon_archive_value_vector_get_basic_type(&type, &value_iter);
            carbon_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

            if (CARBON_UNLIKELY(first_type_group)) {
                OPTIONAL_CALL(visitor, first_prop_type_group, archive, path_stack, this_object_oid, keys, type, is_array, num_pairs, capture);
            } else {
                OPTIONAL_CALL(visitor, next_prop_type_group, archive, path_stack, this_object_oid, keys, type, is_array, num_pairs, capture);
            }

            switch (type) {
            case CARBON_BASIC_TYPE_OBJECT:
                assert (!is_array);
                iterate_objects(archive, keys, num_pairs, &value_iter, path_stack, visitor, mask, capture, is_root_object);
                //for (size_t i = 0; i < num_pairs; i++) {
                //    iterate_objects(archive, &keys[i], 1, &value_iter, path_stack, visitor, mask, capture, is_root_object, keys[i], i);
                //}
             break;
            case CARBON_BASIC_TYPE_NULL:
                if (is_array) {
                    carbon_visitor_policy_e visit = CARBON_VISITOR_POLICY_INCLUDE;
                    if (visitor->visit_enter_null_array_pairs) {
                        visit = visitor->visit_enter_null_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);
                    }
                    if (visit == CARBON_VISITOR_POLICY_INCLUDE) {
                        const carbon_uint32_t *num_values = carbon_archive_value_vector_get_null_arrays(NULL, &value_iter);
                        for (uint32_t prop_idx = 0; prop_idx < num_pairs; prop_idx++)
                        {
                            OPTIONAL_CALL(visitor, visit_enter_null_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],
                                          prop_idx, num_values[prop_idx], capture);
                            OPTIONAL_CALL(visitor, visit_null_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],
                                          prop_idx, num_pairs, num_values[prop_idx], capture)
                            OPTIONAL_CALL(visitor, visit_leave_null_array_pair, archive, path_stack, this_object_oid, prop_idx,
                                          num_pairs, capture);
                        }
                        OPTIONAL_CALL(visitor, visit_leave_int8_array_pairs, archive, path_stack, this_object_oid, capture);
                    }
                }
                else {
                    if (visitor->visit_null_pairs) {
                        visitor->visit_null_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);
                    }
                }
                break;
            case CARBON_BASIC_TYPE_INT8:
                SET_TYPE_SWITCH_CASE(int8, carbon_int8_t)
                break;
            case CARBON_BASIC_TYPE_INT16:
                SET_TYPE_SWITCH_CASE(int16, carbon_int16_t)
                break;
            case CARBON_BASIC_TYPE_INT32:
                SET_TYPE_SWITCH_CASE(int32, carbon_int32_t)
                break;
            case CARBON_BASIC_TYPE_INT64:
                SET_TYPE_SWITCH_CASE(int64, carbon_int64_t)
                break;
            case CARBON_BASIC_TYPE_UINT8:
                SET_TYPE_SWITCH_CASE(uint8, carbon_uint8_t)
                break;
            case CARBON_BASIC_TYPE_UINT16:
                SET_TYPE_SWITCH_CASE(uint16, carbon_uint16_t)
                break;
            case CARBON_BASIC_TYPE_UINT32:
                SET_TYPE_SWITCH_CASE(uint32, carbon_uint32_t)
                break;
            case CARBON_BASIC_TYPE_UINT64:
                SET_TYPE_SWITCH_CASE(uint64, carbon_uint64_t)
                break;
            case CARBON_BASIC_TYPE_NUMBER:
                SET_TYPE_SWITCH_CASE(number, carbon_number_t)
                break;
            case CARBON_BASIC_TYPE_STRING:
                SET_TYPE_SWITCH_CASE(string, carbon_string_id_t)
                break;
            case CARBON_BASIC_TYPE_BOOLEAN:
                SET_TYPE_SWITCH_CASE(boolean, carbon_boolean_t)
                break;
            default:
                break;
            }

            first_type_group = false;
        } else {
            carbon_archive_column_group_iter_t group_iter;
            uint32_t num_column_groups;
            keys = carbon_archive_collection_iter_get_keys(&num_column_groups, &collection_iter);

            bool *skip_groups_by_key = malloc(num_column_groups * sizeof(bool));
            CARBON_ZERO_MEMORY(skip_groups_by_key, num_column_groups * sizeof(bool));

            if (visitor->before_visit_object_array) {
                for (uint32_t i = 0; i < num_column_groups; i++) {
                    carbon_visitor_policy_e policy = visitor->before_visit_object_array(archive, path_stack,
                                                                                        this_object_oid, keys[i],
                                                                                        capture);
                    skip_groups_by_key[i] = policy == CARBON_VISITOR_POLICY_EXCLUDE;
                }
            }

            uint32_t current_group_idx = 0;
            while (carbon_archive_collection_next_column_group(&group_iter, &collection_iter)) {
                if (!skip_groups_by_key[current_group_idx]) {
                    uint32_t num_column_group_objs;
                    carbon_archive_column_iter_t column_iter;
                    carbon_string_id_t group_key = keys[current_group_idx];
                    const carbon_object_id_t *column_group_object_ids = carbon_archive_column_group_get_object_ids(&num_column_group_objs, &group_iter);
                    bool *skip_objects = malloc(num_column_group_objs * sizeof(bool));
                    CARBON_ZERO_MEMORY(skip_objects, num_column_group_objs * sizeof(bool));

                    if (visitor->before_visit_object_array_objects) {
                        visitor->before_visit_object_array_objects(skip_objects, archive, path_stack, this_object_oid,
                                                                   keys[current_group_idx],
                                                                   column_group_object_ids, num_column_group_objs,
                                                                   capture);
                    }

                    uint32_t current_column_group_obj_idx = 0;

                    while(carbon_archive_column_group_next_column(&column_iter, &group_iter))
                    {
                        if (!skip_objects[current_column_group_obj_idx])
                        {
                            carbon_string_id_t current_column_name;
                            carbon_basic_type_e current_column_entry_type;

                            carbon_archive_column_get_name(&current_column_name, &current_column_entry_type, &column_iter);

                            bool skip_column = false;
                            if (visitor->before_visit_object_array_object_property)
                            {
                                carbon_visitor_policy_e policy = visitor->before_visit_object_array_object_property(archive, path_stack,
                                                             this_object_oid, group_key,
                                                             current_column_name, current_column_entry_type, capture);
                                skip_column = policy == CARBON_VISITOR_POLICY_EXCLUDE;
                            }

                            if (!skip_column)
                            {
                                uint32_t num_positions;
                                const uint32_t *entry_positions = carbon_archive_column_get_entry_positions(&num_positions, &column_iter);
                                carbon_archive_column_entry_iter_t entry_iter;

                                carbon_object_id_t *entry_object_containments = malloc(num_positions * sizeof(carbon_object_id_t));
                                for (uint32_t m = 0; m < num_positions; m++)
                                {
                                    entry_object_containments[m] = column_group_object_ids[entry_positions[m]];
                                }

                                uint32_t current_entry_idx = 0;
                                while(carbon_archive_column_next_entry(&entry_iter, &column_iter)) {

                                    carbon_object_id_t current_nested_object_id = entry_object_containments[current_entry_idx];
                                    uint32_t entry_length;

                                    switch (current_column_entry_type) {
                                    case CARBON_BASIC_TYPE_INT8: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int8s, carbon_int8_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_INT16: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int16s, carbon_int16_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_INT32: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int32s, carbon_int32_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_INT64: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int64s, carbon_int64_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_UINT8: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint8s, carbon_uint8_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_UINT16: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint16s, carbon_uint16_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_UINT32: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint32s, carbon_uint32_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_UINT64: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint64s, carbon_uint64_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_NUMBER: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(numbers, carbon_number_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_STRING: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(strings, carbon_string_id_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_BOOLEAN: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(booleans, carbon_boolean_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_NULL: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(nulls, carbon_uint32_t)
                                    } break;
                                    case CARBON_BASIC_TYPE_OBJECT: {
                                        carbon_archive_column_entry_object_iter_t iter;
                                        const carbon_archive_object_t *archive_object;
                                        carbon_archive_column_entry_get_objects(&iter, &entry_iter);

                                        while ((archive_object = carbon_archive_column_entry_object_iter_next_object(&iter)) != NULL) {
                                            carbon_object_id_t id;
                                            carbon_archive_object_get_object_id(&id, archive_object);

                                            bool skip_object = false;
                                            if (visitor->before_object_array_object_property_object) {
                                                carbon_visitor_policy_e policy = visitor->before_object_array_object_property_object(archive,
                                                path_stack, this_object_oid, group_key, current_nested_object_id, current_column_name, id, capture);
                                                skip_object = policy == CARBON_VISITOR_POLICY_EXCLUDE;
                                            }

                                            if(!skip_object)
                                            {
                                                carbon_err_t err;
                                                carbon_archive_prop_iter_t nested_obj_prop_iter;
                                                carbon_archive_prop_iter_from_object(&nested_obj_prop_iter, mask,
                                                                                     &err, archive_object);
                                                iterate_props(archive, &nested_obj_prop_iter, path_stack, visitor,
                                                              mask, capture, false, group_key, current_group_idx);
                                            }
                                        }
                                    } break;
                                    default:
                                        break;
                                    }

                                    current_entry_idx++;
                                }


                                free(entry_object_containments);
                            }
                        }
                        current_column_group_obj_idx++;
                    }

                    free(skip_objects);
                }
                current_group_idx++;
            }
            free(skip_groups_by_key);
        }
    }
    carbon_vec_pop(path_stack);
}

CARBON_EXPORT(bool)
carbon_archive_visit_archive(carbon_archive_t *archive, const carbon_archive_visitor_desc_t *desc,
                             carbon_archive_visitor_t *visitor, void *capture)
{
    CARBON_NON_NULL_OR_ERROR(archive)
    CARBON_NON_NULL_OR_ERROR(visitor)

    carbon_archive_prop_iter_t  prop_iter;
    carbon_vec_t ofType(carbon_path_entry) path_stack;

    int mask = desc ? desc->visit_mask : CARBON_ARCHIVE_ITER_MASK_ANY;

    if (carbon_archive_prop_iter_from_archive(&prop_iter, &archive->err, mask, archive))
    {
        carbon_vec_create(&path_stack, NULL, sizeof(carbon_path_entry_t), 100);
        OPTIONAL_CALL(visitor, before_visit_starts, archive, capture);
        iterate_props(archive, &prop_iter, &path_stack, visitor, mask, capture, true, 0, 0);
        OPTIONAL_CALL(visitor, after_visit_ends, archive, capture);
        carbon_vec_drop(&path_stack);
        return true;
    } else {
        return false;
    }
}

#include <inttypes.h>

CARBON_EXPORT(bool)
carbon_archive_visitor_print_path(FILE *file, carbon_archive_t *archive, const carbon_vec_t ofType(carbon_path_entry_t) *path_stack)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(path_stack)

    carbon_query_t query;
    carbon_archive_query(&query, archive);

    for (uint32_t i = 1; i < path_stack->num_elems; i++)
    {
        const carbon_path_entry_t *entry = CARBON_VECTOR_GET(path_stack, i, carbon_path_entry_t);
       // if (entry->key != 0) {
         //   char *key = carbon_query_fetch_string_by_id(&query, entry->key);
            fprintf(file, "'%llu'[%d]/", entry->key, entry->idx);
         //   free(key);
       // }

    }
    fprintf(file, "\n");

    carbon_query_drop(&query);

    return true;
}