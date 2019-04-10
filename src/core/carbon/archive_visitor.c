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

#include "core/carbon/archive_visitor.h"
#include "std/hash_table.h"
#include "std/hash_set.h"
#include "core/carbon/archive_visitor.h"
#include "core/carbon/archive_query.h"

static void
iterate_props(struct archive *archive, struct prop_iter *prop_iter,
              struct vector ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object, field_sid_t parent_key, u32 parent_key_array_idx);

static void
iterate_objects(struct archive *archive, const field_sid_t *keys, u32 num_pairs,
                struct archive_value_vector *value_iter,
                struct vector ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
                int mask, void *capture, bool is_root_object)
{
    NG5_UNUSED(num_pairs);

    u32 vector_length;
    struct archive_object object;
    object_id_t parent_object_id;
    object_id_t object_id;
    struct prop_iter  prop_iter;
    struct err err;

    carbon_archive_value_vector_get_object_id(&parent_object_id, value_iter);
    carbon_archive_value_vector_get_length(&vector_length, value_iter);
    assert(num_pairs == vector_length);

    for (u32 i = 0; i < vector_length; i++)
    {
        field_sid_t parent_key = keys[i];
        u32 parent_key_array_idx = i;

//        carbon_path_entry_t e = { .key = parent_key, .idx = 0 };
//        carbon_vec_push(path_stack, &e, 1);

      //  fprintf(stderr, "XXXX object: ");
        //  carbon_archive_visitor_print_path(stderr, archive, path_stack);

        carbon_archive_value_vector_get_object_at(&object, i, value_iter);
        carbon_archive_object_get_object_id(&object_id, &object);

        carbon_archive_prop_iter_from_object(&prop_iter, mask, &err, &object);

        if (!is_root_object) {
            carbon_visitor_policy_e visit = NG5_VISITOR_POLICY_INCLUDE;
            if (visitor->before_object_visit) {
                visit = visitor->before_object_visit(archive, path_stack, parent_object_id, object_id, i,
                                                     vector_length, keys[i], capture);
            }
            if (visit == NG5_VISITOR_POLICY_INCLUDE) {
                iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false, parent_key, parent_key_array_idx);
                OPTIONAL_CALL(visitor, after_object_visit, archive, path_stack, object_id, i, vector_length, capture);
            }
        } else {
            OPTIONAL_CALL(visitor, visit_root_object, archive, object_id, capture);
            iterate_props(archive, &prop_iter, path_stack, visitor, mask, capture, false, parent_key, parent_key_array_idx);
        }

      //  carbon_vec_pop(path_stack);
    }
}

#define SET_TYPE_SWITCH_CASE(name, built_in_type)                                                                      \
{                                                                                                                      \
    if (is_array) {                                                                                                    \
        carbon_visitor_policy_e visit = NG5_VISITOR_POLICY_INCLUDE;                                                 \
        if (visitor->visit_enter_##name##_array_pairs) {                                                               \
            visit = visitor->visit_enter_##name##_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);     \
        }                                                                                                              \
        if (visit == NG5_VISITOR_POLICY_INCLUDE) {                                                                  \
            for (u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++)                                              \
            {                                                                                                          \
                u32 array_length;                                                                                 \
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
iterate_props(struct archive *archive, struct prop_iter *prop_iter,
              struct vector ofType(carbon_path_entry_t) *path_stack, carbon_archive_visitor_t *visitor,
              int mask, void *capture, bool is_root_object, field_sid_t parent_key, u32 parent_key_array_idx)
{
    object_id_t this_object_oid;
    struct archive_value_vector value_iter;
    enum field_type type;
    bool is_array;
    const field_sid_t *keys;
    u32 num_pairs;
    enum prop_iter_mode iter_type;
    archive_collection_iter_t collection_iter;
    bool first_type_group = true;

    NG5_UNUSED(parent_key);
    NG5_UNUSED(parent_key_array_idx);

    carbon_path_entry_t e = { .key = parent_key, .idx = parent_key_array_idx };
    carbon_vec_push(path_stack, &e, 1);

    carbon_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

    while (carbon_archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter)) {

        if (iter_type == PROP_ITER_MODE_OBJECT) {

            keys = carbon_archive_value_vector_get_keys(&num_pairs, &value_iter);
            carbon_archive_value_vector_is_array_type(&is_array, &value_iter);
            carbon_archive_value_vector_get_basic_type(&type, &value_iter);
            carbon_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

            for (u32 i = 0; i < num_pairs; i++) {
                OPTIONAL_CALL(visitor, visit_object_property, archive, path_stack, this_object_oid, keys[i], type, is_array, capture);

                carbon_path_entry_t e = { .key = keys[i], .idx = 666 };
                carbon_vec_push(path_stack, &e, 1);
                //carbon_archive_visitor_print_path(stderr, archive, path_stack);
                OPTIONAL_CALL(visitor, visit_object_array_prop, archive, path_stack, this_object_oid, keys[i], type, capture);
                carbon_vec_pop(path_stack);
            }


            if (NG5_UNLIKELY(first_type_group)) {
                OPTIONAL_CALL(visitor, first_prop_type_group, archive, path_stack, this_object_oid, keys, type, is_array, num_pairs, capture);
            } else {
                OPTIONAL_CALL(visitor, next_prop_type_group, archive, path_stack, this_object_oid, keys, type, is_array, num_pairs, capture);
            }

            switch (type) {
            case NG5_BASIC_TYPE_OBJECT:
                assert (!is_array);
                iterate_objects(archive, keys, num_pairs, &value_iter, path_stack, visitor, mask, capture, is_root_object);
                //for (size_t i = 0; i < num_pairs; i++) {
                //    iterate_objects(archive, &keys[i], 1, &value_iter, path_stack, visitor, mask, capture, is_root_object, keys[i], i);
                //}
             break;
            case NG5_BASIC_TYPE_NULL:
                if (is_array) {
                    carbon_visitor_policy_e visit = NG5_VISITOR_POLICY_INCLUDE;
                    if (visitor->visit_enter_null_array_pairs) {
                        visit = visitor->visit_enter_null_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);
                    }
                    if (visit == NG5_VISITOR_POLICY_INCLUDE) {
                        const field_u32_t *num_values = carbon_archive_value_vector_get_null_arrays(NULL, &value_iter);
                        for (u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++)
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
            case NG5_BASIC_TYPE_INT8:
                SET_TYPE_SWITCH_CASE(int8, field_i8_t)
                break;
            case NG5_BASIC_TYPE_INT16:
                SET_TYPE_SWITCH_CASE(int16, field_i16_t)
                break;
            case NG5_BASIC_TYPE_INT32:
                SET_TYPE_SWITCH_CASE(int32, field_i32_t)
                break;
            case NG5_BASIC_TYPE_INT64:
                SET_TYPE_SWITCH_CASE(int64, field_i64_t)
                break;
            case NG5_BASIC_TYPE_UINT8:
                SET_TYPE_SWITCH_CASE(uint8, field_u8_t)
                break;
            case NG5_BASIC_TYPE_UINT16:
                SET_TYPE_SWITCH_CASE(uint16, field_u16_t)
                break;
            case NG5_BASIC_TYPE_UINT32:
                SET_TYPE_SWITCH_CASE(uint32, field_u32_t)
                break;
            case NG5_BASIC_TYPE_UINT64:
                SET_TYPE_SWITCH_CASE(uint64, field_u64_t)
                break;
            case NG5_BASIC_TYPE_NUMBER:
                SET_TYPE_SWITCH_CASE(number, field_number_t)
                break;
            case NG5_BASIC_TYPE_STRING:
                SET_TYPE_SWITCH_CASE(string, field_sid_t)
                break;
            case NG5_BASIC_TYPE_BOOLEAN:
                SET_TYPE_SWITCH_CASE(boolean, field_boolean_t)
                break;
            default:
                break;
            }

            first_type_group = false;
        } else {
            archive_column_group_iter_t group_iter;
            u32 num_column_groups;
            keys = carbon_archive_collection_iter_get_keys(&num_column_groups, &collection_iter);

            bool *skip_groups_by_key = malloc(num_column_groups * sizeof(bool));
            NG5_ZERO_MEMORY(skip_groups_by_key, num_column_groups * sizeof(bool));

            if (visitor->before_visit_object_array) {
                for (u32 i = 0; i < num_column_groups; i++) {

               //     carbon_path_entry_t e = { .key = parent_key, .idx = i };
                    //carbon_vec_push(path_stack, &e, 1);


                    carbon_visitor_policy_e policy = visitor->before_visit_object_array(archive, path_stack,
                                                                                        this_object_oid, keys[i],
                                                                                        capture);

               //     carbon_vec_pop(path_stack);

                    skip_groups_by_key[i] = policy == NG5_VISITOR_POLICY_EXCLUDE;
                }
            }

            u32 current_group_idx = 0;



            while (carbon_archive_collection_next_column_group(&group_iter, &collection_iter)) {
                if (!skip_groups_by_key[current_group_idx]) {

                    u32 num_column_group_objs;
                    archive_column_iter_t column_iter;
                    field_sid_t group_key = keys[current_group_idx];
                    const object_id_t *column_group_object_ids = carbon_archive_column_group_get_object_ids(&num_column_group_objs, &group_iter);
                    bool *skip_objects = malloc(num_column_group_objs * sizeof(bool));
                    NG5_ZERO_MEMORY(skip_objects, num_column_group_objs * sizeof(bool));


                    if (visitor->before_visit_object_array_objects) {
                        visitor->before_visit_object_array_objects(skip_objects, archive, path_stack, this_object_oid,
                                                                   group_key,
                                                                   column_group_object_ids, num_column_group_objs,
                                                                   capture);
                    }

                    u32 current_column_group_obj_idx = 0;

                    while(carbon_archive_column_group_next_column(&column_iter, &group_iter))
                    {

                        if (!skip_objects[current_column_group_obj_idx])
                        {
                            field_sid_t current_column_name;
                            enum field_type current_column_entry_type;

                            carbon_archive_column_get_name(&current_column_name, &current_column_entry_type, &column_iter);

                            carbon_path_entry_t e = { .key = current_column_name, .idx = 0 };
                            carbon_vec_push(path_stack, &e, 1);

                            /**
                                0/page_end/0/
                                /0/doi/0/
                                /0/page_start/0/
                                /0/venue/0/
                                /0/doc_type/0/
                                /0/n_citation/0/
                                /0/issue/0/
                                /0/volume/0/
                                /0/n_citation/0/
                             */

                            OPTIONAL_CALL(visitor, visit_object_array_prop, archive, path_stack, this_object_oid, current_column_name, current_column_entry_type, capture);

                            bool skip_column = false;
                            if (visitor->before_visit_object_array_object_property)
                            {
                                carbon_visitor_policy_e policy = visitor->before_visit_object_array_object_property(archive, path_stack,
                                                             this_object_oid, group_key,
                                                             current_column_name, current_column_entry_type, capture);
                                skip_column = policy == NG5_VISITOR_POLICY_EXCLUDE;
                            }

                            if (!skip_column)
                            {
                                u32 num_positions;
                                const u32 *entry_positions = carbon_archive_column_get_entry_positions(&num_positions, &column_iter);
                                archive_column_entry_iter_t entry_iter;

                                object_id_t *entry_object_containments = malloc(num_positions * sizeof(object_id_t));
                                for (u32 m = 0; m < num_positions; m++)
                                {
                                    entry_object_containments[m] = column_group_object_ids[entry_positions[m]];
                                }

                                if (visitor->get_column_entry_count) {
                                    bool shall_continue = visitor->get_column_entry_count(archive, path_stack, current_column_name, current_column_entry_type, num_positions, capture);
                                    if (!shall_continue) {
                                        break;
                                    }
                                }

                                u32 current_entry_idx = 0;
                                while(carbon_archive_column_next_entry(&entry_iter, &column_iter)) {

                                    object_id_t current_nested_object_id = entry_object_containments[current_entry_idx];
                                    u32 entry_length;

                                    switch (current_column_entry_type) {
                                    case NG5_BASIC_TYPE_INT8: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int8s, field_i8_t)
                                    } break;
                                    case NG5_BASIC_TYPE_INT16: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int16s, field_i16_t)
                                    } break;
                                    case NG5_BASIC_TYPE_INT32: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int32s, field_i32_t)
                                    } break;
                                    case NG5_BASIC_TYPE_INT64: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(int64s, field_i64_t)
                                    } break;
                                    case NG5_BASIC_TYPE_UINT8: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint8s, field_u8_t)
                                    } break;
                                    case NG5_BASIC_TYPE_UINT16: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint16s, field_u16_t)
                                    } break;
                                    case NG5_BASIC_TYPE_UINT32: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint32s, field_u32_t)
                                    } break;
                                    case NG5_BASIC_TYPE_UINT64: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(uint64s, field_u64_t)
                                    } break;
                                    case NG5_BASIC_TYPE_NUMBER: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(numbers, field_number_t)
                                    } break;
                                    case NG5_BASIC_TYPE_STRING: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(strings, field_sid_t)
                                    } break;
                                    case NG5_BASIC_TYPE_BOOLEAN: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(booleans, field_boolean_t)
                                    } break;
                                    case NG5_BASIC_TYPE_NULL: {
                                        SET_NESTED_ARRAY_SWITCH_CASE(nulls, field_u32_t)
                                    } break;
                                    case NG5_BASIC_TYPE_OBJECT: {
                                        struct column_object_iter iter;
                                        const struct archive_object *archive_object;
                                        carbon_archive_column_entry_get_objects(&iter, &entry_iter);

                                        while ((archive_object = carbon_archive_column_entry_object_iter_next_object(&iter)) != NULL) {
                                            object_id_t id;
                                            carbon_archive_object_get_object_id(&id, archive_object);




                                            bool skip_object = false;
                                            if (visitor->before_object_array_object_property_object) {
                                                carbon_visitor_policy_e policy = visitor->before_object_array_object_property_object(archive,
                                                path_stack, this_object_oid, group_key, current_nested_object_id, current_column_name, id, capture);
                                                skip_object = policy == NG5_VISITOR_POLICY_EXCLUDE;
                                            }

                                            if(!skip_object)
                                            {


                                                //keys[i]

                                                //carbon_path_entry_t e = { .key = current_column_name, .idx = 0 };
                                                //carbon_vec_push(path_stack, &e, 1);

                                                carbon_vec_pop(path_stack);

                                                struct err err;
                                                struct prop_iter nested_obj_prop_iter;
                                                carbon_archive_prop_iter_from_object(&nested_obj_prop_iter, mask,
                                                                                     &err, archive_object);
                                                iterate_props(archive, &nested_obj_prop_iter, path_stack, visitor,
                                                              mask, capture, false, current_column_name, current_group_idx);

                                                carbon_path_entry_t e = { .key = current_column_name, .idx = 0 };
                                                carbon_vec_push(path_stack, &e, 1);

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
                            carbon_vec_pop(path_stack);
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

NG5_EXPORT(bool)
carbon_archive_visit_archive(struct archive *archive, const carbon_archive_visitor_desc_t *desc,
                             carbon_archive_visitor_t *visitor, void *capture)
{
    NG5_NON_NULL_OR_ERROR(archive)
    NG5_NON_NULL_OR_ERROR(visitor)

    struct prop_iter  prop_iter;
    struct vector ofType(carbon_path_entry) path_stack;

    int mask = desc ? desc->visit_mask : NG5_ARCHIVE_ITER_MASK_ANY;

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

NG5_EXPORT(void)
carbon_archive_visitor_path_to_string(char path_buffer[2048], struct archive *archive, const struct vector ofType(carbon_path_entry_t) *path_stack)
{

    struct archive_query *query = carbon_archive_query_default(archive);


    for (u32 i = 0; i < path_stack->num_elems; i++)
    {
        const carbon_path_entry_t *entry = vec_get(path_stack, i, carbon_path_entry_t);
        if (entry->key != 0) {
            char *key = carbon_query_fetch_string_by_id(query, entry->key);
            size_t path_len = strlen(path_buffer);
            sprintf(path_buffer + path_len, "%s%s", key, i + 1 < path_stack->num_elems ? ", " : "");
            free(key);
        } else {
            sprintf(path_buffer, "/");
        }
    }
}

NG5_EXPORT(bool)
carbon_archive_visitor_print_path(FILE *file, struct archive *archive, const struct vector ofType(carbon_path_entry_t) *path_stack)
{
    NG5_NON_NULL_OR_ERROR(file)
    NG5_NON_NULL_OR_ERROR(path_stack)
    NG5_NON_NULL_OR_ERROR(archive)

    struct archive_query *query = carbon_archive_query_default(archive);


    for (u32 i = 0; i < path_stack->num_elems; i++)
    {
        const carbon_path_entry_t *entry = vec_get(path_stack, i, carbon_path_entry_t);
        if (entry->key != 0) {
            char *key = carbon_query_fetch_string_by_id(query, entry->key);
            fprintf(file, "%s/", key);
            free(key);
        } else {
            fprintf(file, "/");
        }
    }
    fprintf(file, "\n");


    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path_stack);
    fprintf(file, "%s\n", buffer);


    return true;
}

NG5_EXPORT(bool)
carbon_archive_visitor_path_compare(const struct vector ofType(carbon_path_entry_t) *path, field_sid_t *group_name, const char *path_str, struct archive *archive)
{
    char path_buffer[2048];
    memset(path_buffer, 0, sizeof(path_buffer));
    sprintf(path_buffer, "/");

    struct archive_query *query = carbon_archive_query_default(archive);

    for (u32 i = 1; i < path->num_elems; i++)
    {
        const carbon_path_entry_t *entry = vec_get(path, i, carbon_path_entry_t);
        if (entry->key != 0) {
            char *key = carbon_query_fetch_string_by_id(query, entry->key);
            size_t path_len = strlen(path_buffer);
            sprintf(path_buffer + path_len, "%s/", key);
            free(key);
        }
    }

    if(group_name) {
        char *key = carbon_query_fetch_string_by_id(query, *group_name);
        size_t path_len = strlen(path_buffer);
        sprintf(path_buffer + path_len, "%s/", key);
        free(key);
    }

    fprintf(stderr, "'%s' <-> needle '%s'\n", path_buffer, path_str);

    return strcmp(path_buffer, path_str) == 0;
}