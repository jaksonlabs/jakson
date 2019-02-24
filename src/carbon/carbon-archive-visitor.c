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
#include "carbon/carbon-archive-visitor.h"
#include "carbon/carbon-query.h"

#define OPTIONAL_CALL(func, ...) if(func) { func(__VA_ARGS__); }

static void
iterate_props(carbon_archive_t *archive, carbon_archive_prop_iter_t *prop_iter,
              carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object);

static void
push_path(carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_string_id_t key, carbon_object_id_t id)
{
    carbon_path_entry_t entry = {
        .key = key,
        .containing_obj = id
    };

    if (key) {
        carbon_vec_push(path_stack, &entry, 0);
    }
}

static void
pop_path(carbon_vec_t ofType(carbon_path_entry_t) *path_stack)
{
    carbon_vec_pop(path_stack);
}

static void
iterate_objects(carbon_archive_t *archive, const carbon_string_id_t *keys, uint32_t num_pairs,
                carbon_archive_value_vector_t *value_iter,
                carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
                int mask, void *capture, bool is_root_object)
{
    uint32_t vector_length;
    carbon_archive_object_t object;
    carbon_object_id_t parent_object_id;
    carbon_object_id_t object_id;
    carbon_archive_prop_iter_t  prop_iter;
    carbon_err_t err;

    carbon_archive_value_vector_get_object_id(&parent_object_id, value_iter);
    carbon_archive_value_vector_get_length(&vector_length, value_iter);
    assert(vector_length == num_pairs);

    for (uint32_t i = 0; i < vector_length; i++)
    {
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
                push_path(path_stack, keys[i], object_id);
                iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false);
                pop_path(path_stack);
                OPTIONAL_CALL(visitor->after_object_visit, archive, path_stack, object_id, i, vector_length, capture);
            }
        } else {
            OPTIONAL_CALL(visitor->visit_root_object, archive, object_id, capture);
            iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false);
        }

    }
}

#define SET_TYPE_SWITCH_CASE(name, built_in_type)                                                                      \
{                                                                                                                      \
    if (is_array) {                                                                                                    \
        carbon_visitor_policy_e visit = CARBON_VISITOR_POLICY_INCLUDE;                                                 \
        if (visitor->visit_enter_##name##_array_pairs) {                                                               \
            visit = visitor->visit_enter_##name##_array_pairs(archive, path_stack, oid, keys, num_pairs, capture);     \
        }                                                                                                              \
        if (visit == CARBON_VISITOR_POLICY_INCLUDE) {                                                                  \
            for (uint32_t prop_idx = 0; prop_idx < num_pairs; prop_idx++)                                              \
            {                                                                                                          \
                uint32_t array_length;                                                                                 \
                const built_in_type *values = carbon_archive_value_vector_get_##name##_arrays_at(&array_length,        \
                                                                                             prop_idx,                 \
                                                                                             &value_iter);             \
                OPTIONAL_CALL(visitor->visit_enter_##name##_array_pair, archive, path_stack, oid, keys[prop_idx],      \
                              prop_idx, array_length, capture);                                                        \
                OPTIONAL_CALL(visitor->visit_##name##_array_pair, archive, path_stack, oid, keys[prop_idx],            \
                              prop_idx, num_pairs, values, array_length, capture)                                      \
                OPTIONAL_CALL(visitor->visit_leave_##name##_array_pair, archive, path_stack, oid, prop_idx, num_pairs, \
                                                                        capture);                                      \
            }                                                                                                          \
            OPTIONAL_CALL(visitor->visit_leave_##name##_array_pairs, archive, path_stack, oid, capture);               \
        }                                                                                                              \
    } else                                                                                                             \
    {                                                                                                                  \
        if (visitor->visit_##name##_pairs) {                                                                           \
            const built_in_type *values = carbon_archive_value_vector_get_##name##s(NULL, &value_iter);                \
            visitor->visit_##name##_pairs(archive, path_stack, oid, keys, values, num_pairs, capture);                 \
        }                                                                                                              \
    }                                                                                                                  \
}

static void
iterate_props(carbon_archive_t *archive, carbon_archive_prop_iter_t *prop_iter,
              carbon_vec_t ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object)
{
    carbon_object_id_t oid;
    carbon_archive_value_vector_t value_iter;
    carbon_basic_type_e type;
    bool is_array;
    const carbon_string_id_t *keys;
    uint32_t num_pairs;
    carbon_archive_prop_iter_mode_e iter_type;
    carbon_archive_collection_iter_t collection_iter;
    bool first_type_group = true;
//    uint32_t num_column_groups;
//    carbon_archive_column_group_iter_t group_iter;
//    carbon_archive_column_iter_t column_iter;
//    carbon_archive_column_entry_iter_t entry_iter;
//    carbon_err_t err;

    while (carbon_archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter)) {

        if (iter_type == CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT) {

            keys = carbon_archive_value_vector_get_keys(&num_pairs, &value_iter);
            carbon_archive_value_vector_is_array_type(&is_array, &value_iter);
            carbon_archive_value_vector_get_basic_type(&type, &value_iter);
            carbon_archive_value_vector_get_object_id(&oid, &value_iter);

            if (CARBON_UNLIKELY(first_type_group)) {
                OPTIONAL_CALL(visitor->first_prop_type_group, archive, path_stack, oid, keys, type, is_array, num_pairs, capture);
            } else {
                OPTIONAL_CALL(visitor->next_prop_type_group, archive, path_stack, oid, keys, type, is_array, num_pairs, capture);
            }

            switch (type) {
            case CARBON_BASIC_TYPE_OBJECT:
                assert (!is_array);
                iterate_objects(archive, keys, num_pairs, &value_iter, path_stack, visitor, mask, capture, is_root_object);
             break;
            case CARBON_BASIC_TYPE_NULL:
                if (is_array) {
                    carbon_visitor_policy_e visit = CARBON_VISITOR_POLICY_INCLUDE;
                    if (visitor->visit_enter_null_array_pairs) {
                        visit = visitor->visit_enter_null_array_pairs(archive, path_stack, oid, keys, num_pairs, capture);
                    }
                    if (visit == CARBON_VISITOR_POLICY_INCLUDE) {
                        const carbon_uint32_t *num_values = carbon_archive_value_vector_get_null_arrays(NULL, &value_iter);
                        for (uint32_t prop_idx = 0; prop_idx < num_pairs; prop_idx++)
                        {
                            OPTIONAL_CALL(visitor->visit_enter_null_array_pair, archive, path_stack, oid, keys[prop_idx],
                                          prop_idx, num_values[prop_idx], capture);
                            OPTIONAL_CALL(visitor->visit_null_array_pair, archive, path_stack, oid, keys[prop_idx],
                                          prop_idx, num_pairs, num_values[prop_idx], capture)
                            OPTIONAL_CALL(visitor->visit_leave_null_array_pair, archive, path_stack, oid, prop_idx,
                                          num_pairs, capture);
                        }
                        OPTIONAL_CALL(visitor->visit_leave_int8_array_pairs, archive, path_stack, oid, capture);
                    }
                }
                else {
                    if (visitor->visit_null_pairs) {
                        visitor->visit_null_pairs(archive, path_stack, oid, keys, num_pairs, capture);
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

            pop_path(path_stack);
            first_type_group = false;
        } else {
            printf(", \"xxx\": { }"); // TODO
        }
    }
}

//    else {
//            keys = carbon_archive_collection_iter_get_keys(&num_column_groups, &collection_iter);
//            ASSERT_TRUE(keys != NULL);
//            printf("\t\t{ column groups for keys:");
//            for (uint32_t i = 0; i < num_column_groups; i++) {
//                printf("%" PRIu64 " ", keys[i]);
//            }
//            printf("}\n");
//            while (carbon_archive_collection_next_column_group(&group_iter, &collection_iter)) {
//
//                uint32_t num_objs;
//                const carbon_object_id_t *ids = carbon_archive_column_group_get_object_ids(&num_objs, &group_iter);
//
//                printf("\t\t{ column groups object ids:");
//                for (uint32_t i = 0; i < num_objs; i++) {
//                    printf("%" PRIu64 " ", ids[i]);
//                }
//                printf("}\n");
//
//                while(carbon_archive_column_group_next_column(&column_iter, &group_iter)) {
//                    carbon_string_id_t column_name;
//                    carbon_basic_type_e column_entry_type;
//                    uint32_t num_entries;
//                    carbon_archive_column_get_name(&column_name, &column_entry_type, &column_iter);
//                    const uint32_t *positions = carbon_archive_column_get_entry_positions(&num_entries, &column_iter);
//                    printf("\t\t{ column-name: %" PRIu64 ", type: %d }\n", column_name, column_entry_type);
//                    printf("\t\t{ entry positions:");
//                    for (uint32_t i = 0; i < num_entries; i++) {
//                        printf("%" PRIu32 " ", positions[i]);
//                    }
//                    printf("}\n");
//
//                    while(carbon_archive_column_next_entry(&entry_iter, &column_iter)) {
//
//                        carbon_basic_type_e entry_type;
//                        uint32_t entry_length;
//                        carbon_archive_column_entry_get_type(&entry_type, &entry_iter);
//
//                        switch (entry_type) {
//                        case CARBON_BASIC_TYPE_STRING: {
//                            const carbon_string_id_t *values = carbon_archive_column_entry_get_strings(&entry_length, &entry_iter);
//                            printf("\t\t{ strings: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("%" PRIu64 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_INT8: {
//                            const carbon_int8_t *values = carbon_archive_column_entry_get_int8s(&entry_length, &entry_iter);
//                            printf("\t\t{ int8s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("% " PRIi8 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_INT16: {
//                            const carbon_int16_t *values = carbon_archive_column_entry_get_int16s(&entry_length, &entry_iter);
//                            printf("\t\t{ int16s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("% " PRIi16 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_INT32: {
//                            const carbon_int32_t *values = carbon_archive_column_entry_get_int32s(&entry_length, &entry_iter);
//                            printf("\t\t{ int32s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("% " PRIi32 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_INT64: {
//                            const carbon_int64_t *values = carbon_archive_column_entry_get_int64s(&entry_length, &entry_iter);
//                            printf("\t\t{ int64s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("% " PRIi64 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_UINT8: {
//                            const carbon_uint8_t *values = carbon_archive_column_entry_get_uint8s(&entry_length, &entry_iter);
//                            printf("\t\t{ uint8s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("%" PRIu8 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_UINT16: {
//                            const carbon_uint16_t *values = carbon_archive_column_entry_get_uint16s(&entry_length, &entry_iter);
//                            printf("\t\t{ uint16s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("%" PRIu16 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_UINT32: {
//                            const carbon_uint32_t *values = carbon_archive_column_entry_get_uint32s(&entry_length, &entry_iter);
//                            printf("\t\t{ uint32s: [");
//                            for (uint32_t i = 0; i < entry_length; i++) {
//                                printf("%" PRIu32 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_UINT64: {
//                            const carbon_uint64_t *values = carbon_archive_column_entry_get_uint64s(&entry_length, &entry_iter);
//                            printf("\t\t{ uint64s: [");
//                            for (uint64_t i = 0; i < entry_length; i++) {
//                                printf("%" PRIu64 "%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_NUMBER: {
//                            const carbon_number_t *values = carbon_archive_column_entry_get_numbers(&entry_length, &entry_iter);
//                            printf("\t\t{ numbers: [");
//                            for (uint64_t i = 0; i < entry_length; i++) {
//                                printf("%f%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_BOOLEAN: {
//                            const carbon_boolean_t *values = carbon_archive_column_entry_get_booleans(&entry_length, &entry_iter);
//                            printf("\t\t{ booleans: [");
//                            for (uint64_t i = 0; i < entry_length; i++) {
//                                printf("%d%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_NULL: {
//                            const carbon_uint32_t *values = carbon_archive_column_entry_get_nulls(&entry_length, &entry_iter);
//                            printf("\t\t{ nulls: [");
//                            for (uint64_t i = 0; i < entry_length; i++) {
//                                printf("%d%s", values[i], i + 1 < entry_length ? ", " : "");
//                            }
//                            printf("]\n");
//                        } break;
//                        case CARBON_BASIC_TYPE_OBJECT: {
//                            carbon_archive_column_entry_object_iter_t iter;
//                            const carbon_archive_object_t *archive_object;
//                            carbon_archive_column_entry_get_objects(&iter, &entry_iter);
//                            printf("\t\t{ << objects >>: [");
//                            while ((archive_object = carbon_archive_column_entry_object_iter_next_object(&iter)) != NULL) {
//                                carbon_object_id_t id;
//                                carbon_archive_object_get_object_id(&id, archive_object);
//                                printf("{ oid: %" PRIu64 " } \n", id);
//
//                                carbon_archive_prop_iter_t nested_obj_prop_iter;
//                                carbon_archive_prop_iter_from_object(&nested_obj_prop_iter, CARBON_ARCHIVE_ITER_MASK_ANY,
//                                                                     &err, archive_object);
//                                iterate_properties(&nested_obj_prop_iter);
//                            }
//                            printf("]\n");
//                        } break;
//                        default:
//                            FAIL() << "Unknown type";
//                        }
//
//                    }
//                }
//
//            }
//        }
//    }
//}

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
        OPTIONAL_CALL(visitor->before_visit_starts, archive, capture);
        iterate_props(archive, &prop_iter, &path_stack, visitor, mask, capture, true);
        OPTIONAL_CALL(visitor->after_visit_ends, archive, capture);
        carbon_vec_drop(&path_stack);
        return true;
    } else {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_archive_visitor_print_path(FILE *file, carbon_archive_t *archive, const carbon_vec_t ofType(carbon_path_entry_t) *path_stack)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(path_stack)

    carbon_query_t query;
    carbon_archive_query(&query, archive);

    for (uint32_t i = 0; i < path_stack->num_elems; i++)
    {
        const carbon_path_entry_t *entry = CARBON_VECTOR_GET(path_stack, i, carbon_path_entry_t);
        char *key = carbon_query_fetch_string_by_id(&query, entry->key);
        fprintf(file, "\"%s\"/", key);
        free(key);
    }

    return true;
}