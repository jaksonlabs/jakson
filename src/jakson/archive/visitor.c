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

#include <jakson/forwdecl.h>
#include <jakson/archive/visitor.h>
#include <jakson/std/hash/table.h>
#include <jakson/std/hash/set.h>
#include <jakson/archive/visitor.h>
#include <jakson/archive/query.h>

static void iterate_props(archive *archive, prop_iter *prop_iter,
                          vector ofType(path_entry) *path_stack,
                          visitor *visitor,
                          int mask, void *capture,
                          bool is_root_object, archive_field_sid_t parent_key, u32 parent_key_array_idx);

static void iterate_objects(archive *archive, const archive_field_sid_t *keys, u32 num_pairs,
                            archive_value_vector *value_iter,
                            vector ofType(path_entry) *path_stack,
                            visitor *visitor, int mask, void *capture, bool is_root_object)
{
        UNUSED(num_pairs);

        DECLARE_AND_INIT(u32, vector_length)
        DECLARE_AND_INIT(archive_object, object)
        DECLARE_AND_INIT(unique_id_t, parent_object_id)
        DECLARE_AND_INIT(unique_id_t, object_id)
        DECLARE_AND_INIT(prop_iter, prop_iter)
        DECLARE_AND_INIT(err, err)

        archive_value_vector_get_object_id(&parent_object_id, value_iter);
        archive_value_vector_get_length(&vector_length, value_iter);
        JAK_ASSERT(num_pairs == vector_length);

        for (u32 i = 0; i < vector_length; i++) {
                archive_field_sid_t parent_key = keys[i];
                u32 parent_key_array_idx = i;

//        path_entry  e = { .key = parent_key, .idx = 0 };
//        vector_push(path_stack, &e, 1);

                //  fprintf(stderr, "XXXX object: ");
                //  archive_visitor_print_path(stderr, archive, path_stack);

                archive_value_vector_get_object_at(&object, i, value_iter);
                archive_object_get_object_id(&object_id, &object);

                archive_prop_iter_from_object(&prop_iter, mask, &err, &object);

                if (!is_root_object) {
                        visit_policy_e visit = VISIT_INCLUDE;
                        if (visitor->before_object_visit) {
                                visit = visitor->before_object_visit(archive,
                                                                     path_stack,
                                                                     parent_object_id,
                                                                     object_id,
                                                                     i,
                                                                     vector_length,
                                                                     keys[i],
                                                                     capture);
                        }
                        if (visit == VISIT_INCLUDE) {
                                iterate_props(archive,
                                              &prop_iter,
                                              path_stack,
                                              visitor,
                                              mask,
                                              capture,
                                              false,
                                              parent_key,
                                              parent_key_array_idx);
                                OPTIONAL_CALL(visitor,
                                                  after_object_visit,
                                                  archive,
                                                  path_stack,
                                                  object_id,
                                                  i,
                                                  vector_length,
                                                  capture);
                        }
                } else {
                        OPTIONAL_CALL(visitor, visit_root_object, archive, object_id, capture);
                        iterate_props(archive,
                                      &prop_iter,
                                      path_stack,
                                      visitor,
                                      mask,
                                      capture,
                                      false,
                                      parent_key,
                                      parent_key_array_idx);
                }

                //  vector_pop(path_stack);
        }
}

#define SET_TYPE_SWITCH_CASE(name, built_in_type)                                                                      \
{                                                                                                                      \
    if (is_array) {                                                                                                    \
        visit_policy_e visit = VISIT_INCLUDE;                                                 \
        if (visitor->visit_enter_##name##_array_pairs) {                                                               \
            visit = visitor->visit_enter_##name##_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);     \
        }                                                                                                              \
        if (visit == VISIT_INCLUDE) {                                                                  \
            for (u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++)                                              \
            {                                                                                                          \
                u32 array_length;                                                                                 \
                const built_in_type *values = archive_value_vector_get_##name##_arrays_at(&array_length,        \
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
            const built_in_type *values = archive_value_vector_get_##name##s(NULL, &value_iter);                \
            visitor->visit_##name##_pairs(archive, path_stack, this_object_oid, keys, values, num_pairs, capture);                 \
        }                                                                                                              \
    }                                                                                                                  \
}

#define SET_NESTED_ARRAY_SWITCH_CASE(name, built_in_type)                                                              \
{                                                                                                                      \
    const built_in_type *values = archive_column_entry_get_##name(&entry_length, &entry_iter);                  \
    OPTIONAL_CALL(visitor, visit_object_array_object_property_##name,                                                  \
                  archive, path_stack, this_object_oid, group_key, current_nested_object_id,                           \
                  current_column_name, values, entry_length, capture);                                                 \
}

static void iterate_props(archive *archive, prop_iter *prop_iter,
                          vector ofType(path_entry) *path_stack,
                          visitor *visitor,
                          int mask, void *capture,
                          bool is_root_object, archive_field_sid_t parent_key, u32 parent_key_array_idx)
{
        DECLARE_AND_INIT(unique_id_t, this_object_oid)
        DECLARE_AND_INIT(archive_value_vector, value_iter)
        DECLARE_AND_INIT(enum archive_field_type, type);
        DECLARE_AND_INIT(bool, is_array)
        DECLARE_AND_INIT(const archive_field_sid_t *, keys);
        DECLARE_AND_INIT(u32, num_pairs);
        DECLARE_AND_INIT(prop_iter_mode_e, iter_type)
        DECLARE_AND_INIT(independent_iter_state, collection_iter)
        bool first_type_group = true;

        UNUSED(parent_key);
        UNUSED(parent_key_array_idx);

        path_entry e = {.key = parent_key, .idx = parent_key_array_idx};
        vector_push(path_stack, &e, 1);

        archive_value_vector_get_object_id(&this_object_oid, &value_iter);

        while (archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter)) {

                if (iter_type == PROP_ITER_MODE_OBJECT) {

                        keys = archive_value_vector_get_keys(&num_pairs, &value_iter);
                        archive_value_vector_is_array_type(&is_array, &value_iter);
                        archive_value_vector_get_basic_type(&type, &value_iter);
                        archive_value_vector_get_object_id(&this_object_oid, &value_iter);

                        for (u32 i = 0; i < num_pairs; i++) {
                                OPTIONAL_CALL(visitor,
                                                  visit_object_property,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys[i],
                                                  type,
                                                  is_array,
                                                  capture);

                                path_entry e = {.key = keys[i], .idx = 666};
                                vector_push(path_stack, &e, 1);
                                //archive_visitor_print_path(stderr, archive, path_stack);
                                OPTIONAL_CALL(visitor,
                                                  visit_object_array_prop,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys[i],
                                                  type,
                                                  capture);
                                vector_pop(path_stack);
                        }

                        if (UNLIKELY(first_type_group)) {
                                OPTIONAL_CALL(visitor,
                                                  first_prop_type_group,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys,
                                                  type,
                                                  is_array,
                                                  num_pairs,
                                                  capture);
                        } else {
                                OPTIONAL_CALL(visitor,
                                                  next_prop_type_group,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys,
                                                  type,
                                                  is_array,
                                                  num_pairs,
                                                  capture);
                        }

                        switch (type) {
                                case FIELD_OBJECT:
                                        JAK_ASSERT (!is_array);
                                        iterate_objects(archive,
                                                        keys,
                                                        num_pairs,
                                                        &value_iter,
                                                        path_stack,
                                                        visitor,
                                                        mask,
                                                        capture,
                                                        is_root_object);
                                        //for (size_t i = 0; i < num_pairs; i++) {
                                        //    iterate_objects(archive, &keys[i], 1, &value_iter, path_stack, visitor, mask, capture, is_root_object, keys[i], i);
                                        //}
                                        break;
                                case FIELD_NULL:
                                        if (is_array) {
                                                visit_policy_e visit = VISIT_INCLUDE;
                                                if (visitor->visit_enter_null_array_pairs) {
                                                        visit = visitor->visit_enter_null_array_pairs(archive,
                                                                                                      path_stack,
                                                                                                      this_object_oid,
                                                                                                      keys,
                                                                                                      num_pairs,
                                                                                                      capture);
                                                }
                                                if (visit == VISIT_INCLUDE) {
                                                        const archive_field_u32_t *num_values =
                                                                archive_value_vector_get_null_arrays(NULL,
                                                                                                         &value_iter);
                                                        for (u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++) {
                                                                OPTIONAL_CALL(visitor,
                                                                                  visit_enter_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  keys[prop_idx],
                                                                                  prop_idx,
                                                                                  num_values[prop_idx],
                                                                                  capture);
                                                                OPTIONAL_CALL(visitor,
                                                                                  visit_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  keys[prop_idx],
                                                                                  prop_idx,
                                                                                  num_pairs,
                                                                                  num_values[prop_idx],
                                                                                  capture)
                                                                OPTIONAL_CALL(visitor,
                                                                                  visit_leave_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  prop_idx,
                                                                                  num_pairs,
                                                                                  capture);
                                                        }
                                                        OPTIONAL_CALL(visitor,
                                                                          visit_leave_int8_array_pairs,
                                                                          archive,
                                                                          path_stack,
                                                                          this_object_oid,
                                                                          capture);
                                                }
                                        } else {
                                                if (visitor->visit_null_pairs) {
                                                        visitor->visit_null_pairs(archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  keys,
                                                                                  num_pairs,
                                                                                  capture);
                                                }
                                        }
                                        break;
                                case FIELD_INT8: SET_TYPE_SWITCH_CASE(int8, archive_field_i8_t)
                                        break;
                                case FIELD_INT16: SET_TYPE_SWITCH_CASE(int16, archive_field_i16_t)
                                        break;
                                case FIELD_INT32: SET_TYPE_SWITCH_CASE(int32, archive_field_i32_t)
                                        break;
                                case FIELD_INT64: SET_TYPE_SWITCH_CASE(int64, archive_field_i64_t)
                                        break;
                                case FIELD_UINT8: SET_TYPE_SWITCH_CASE(uint8, archive_field_u8_t)
                                        break;
                                case FIELD_UINT16: SET_TYPE_SWITCH_CASE(uint16, archive_field_u16_t)
                                        break;
                                case FIELD_UINT32: SET_TYPE_SWITCH_CASE(uint32, archive_field_u32_t)
                                        break;
                                case FIELD_UINT64: SET_TYPE_SWITCH_CASE(uint64, archive_field_u64_t)
                                        break;
                                case FIELD_FLOAT: SET_TYPE_SWITCH_CASE(number, archive_field_number_t)
                                        break;
                                case FIELD_STRING: SET_TYPE_SWITCH_CASE(string, archive_field_sid_t)
                                        break;
                                case FIELD_BOOLEAN: SET_TYPE_SWITCH_CASE(boolean, archive_field_boolean_t)
                                        break;
                                default:
                                        break;
                        }

                        first_type_group = false;
                } else {
                        independent_iter_state group_iter;
                        u32 num_column_groups;
                        keys = archive_collection_iter_get_keys(&num_column_groups, &collection_iter);

                        bool *skip_groups_by_key = MALLOC(num_column_groups * sizeof(bool));
                        ZERO_MEMORY(skip_groups_by_key, num_column_groups * sizeof(bool));

                        if (visitor->before_visit_object_array) {
                                for (u32 i = 0; i < num_column_groups; i++) {

                                        //     path_entry  e = { .key = parent_key, .idx = i };
                                        //vector_push(path_stack, &e, 1);


                                        visit_policy_e policy = visitor->before_visit_object_array(archive,
                                                                                                          path_stack,
                                                                                                          this_object_oid,
                                                                                                          keys[i],
                                                                                                          capture);

                                        //     vector_pop(path_stack);

                                        skip_groups_by_key[i] = policy == VISIT_EXCLUDE;
                                }
                        }

                        u32 current_group_idx = 0;

                        while (archive_collection_next_column_group(&group_iter, &collection_iter)) {
                                if (!skip_groups_by_key[current_group_idx]) {

                                        u32 num_column_group_objs;
                                        independent_iter_state column_iter;
                                        archive_field_sid_t group_key = keys[current_group_idx];
                                        const unique_id_t *column_group_object_ids =
                                                archive_column_group_get_object_ids(&num_column_group_objs,
                                                                                        &group_iter);
                                        bool *skip_objects = MALLOC(num_column_group_objs * sizeof(bool));
                                        ZERO_MEMORY(skip_objects, num_column_group_objs * sizeof(bool));

                                        if (visitor->before_visit_object_array_objects) {
                                                visitor->before_visit_object_array_objects(skip_objects,
                                                                                           archive,
                                                                                           path_stack,
                                                                                           this_object_oid,
                                                                                           group_key,
                                                                                           column_group_object_ids,
                                                                                           num_column_group_objs,
                                                                                           capture);
                                        }

                                        u32 current_column_group_obj_idx = 0;

                                        while (archive_column_group_next_column(&column_iter, &group_iter)) {

                                                if (!skip_objects[current_column_group_obj_idx]) {
                                                        archive_field_sid_t current_column_name;
                                                        enum archive_field_type current_column_entry_type;

                                                        archive_column_get_name(&current_column_name,
                                                                                    &current_column_entry_type,
                                                                                    &column_iter);

                                                        path_entry e = {.key = current_column_name, .idx = 0};
                                                        vector_push(path_stack, &e, 1);

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

                                                        OPTIONAL_CALL(visitor,
                                                                          visit_object_array_prop,
                                                                          archive,
                                                                          path_stack,
                                                                          this_object_oid,
                                                                          current_column_name,
                                                                          current_column_entry_type,
                                                                          capture);

                                                        bool skip_column = false;
                                                        if (visitor->before_visit_object_array_object_property) {
                                                                visit_policy_e policy =
                                                                        visitor->before_visit_object_array_object_property(
                                                                                archive,
                                                                                path_stack,
                                                                                this_object_oid,
                                                                                group_key,
                                                                                current_column_name,
                                                                                current_column_entry_type,
                                                                                capture);
                                                                skip_column = policy == VISIT_EXCLUDE;
                                                        }

                                                        if (!skip_column) {
                                                                u32 num_positions;
                                                                const u32 *entry_positions =
                                                                        archive_column_get_entry_positions(
                                                                                &num_positions,
                                                                                &column_iter);
                                                                independent_iter_state entry_iter;

                                                                unique_id_t *entry_object_containments =
                                                                        MALLOC(num_positions *
                                                                                   sizeof(unique_id_t));
                                                                for (u32 m = 0; m < num_positions; m++) {
                                                                        entry_object_containments[m] =
                                                                                column_group_object_ids[entry_positions[m]];
                                                                }

                                                                if (visitor->get_column_entry_count) {
                                                                        bool shall_continue =
                                                                                visitor->get_column_entry_count(archive,
                                                                                                                path_stack,
                                                                                                                current_column_name,
                                                                                                                current_column_entry_type,
                                                                                                                num_positions,
                                                                                                                capture);
                                                                        if (!shall_continue) {
                                                                                break;
                                                                        }
                                                                }

                                                                u32 current_entry_idx = 0;
                                                                while (archive_column_next_entry(&entry_iter,
                                                                                                     &column_iter)) {

                                                                        unique_id_t current_nested_object_id =
                                                                                entry_object_containments[current_entry_idx];
                                                                        DECLARE_AND_INIT(u32, entry_length)

                                                                        switch (current_column_entry_type) {
                                                                                case FIELD_INT8: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int8s,
                                                                                                archive_field_i8_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_INT16: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int16s,
                                                                                                archive_field_i16_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_INT32: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int32s,
                                                                                                archive_field_i32_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_INT64: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int64s,
                                                                                                archive_field_i64_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_UINT8: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint8s,
                                                                                                archive_field_u8_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_UINT16: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint16s,
                                                                                                archive_field_u16_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_UINT32: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint32s,
                                                                                                archive_field_u32_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_UINT64: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint64s,
                                                                                                archive_field_u64_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_FLOAT: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                numbers,
                                                                                                archive_field_number_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_STRING: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                strings,
                                                                                                archive_field_sid_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_BOOLEAN: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                booleans,
                                                                                                archive_field_boolean_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_NULL: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                nulls,
                                                                                                archive_field_u32_t)
                                                                                }
                                                                                        break;
                                                                                case FIELD_OBJECT: {
                                                                                        column_object_iter iter;
                                                                                        const archive_object
                                                                                                *archive_object;
                                                                                        archive_column_entry_get_objects(
                                                                                                &iter,
                                                                                                &entry_iter);

                                                                                        while ((archive_object =
                                                                                                        archive_column_entry_object_iter_next_object(
                                                                                                                &iter))
                                                                                               != NULL) {
                                                                                                unique_id_t id;
                                                                                                archive_object_get_object_id(
                                                                                                        &id,
                                                                                                        archive_object);

                                                                                                bool skip_object = false;
                                                                                                if (visitor
                                                                                                        ->before_object_array_object_property_object) {
                                                                                                        visit_policy_e
                                                                                                                policy =
                                                                                                                visitor->before_object_array_object_property_object(
                                                                                                                        archive,
                                                                                                                        path_stack,
                                                                                                                        this_object_oid,
                                                                                                                        group_key,
                                                                                                                        current_nested_object_id,
                                                                                                                        current_column_name,
                                                                                                                        id,
                                                                                                                        capture);
                                                                                                        skip_object =
                                                                                                                policy
                                                                                                                ==
                                                                                                                VISIT_EXCLUDE;
                                                                                                }

                                                                                                if (!skip_object) {


                                                                                                        //keys[i]

                                                                                                        //path_entry  e = { .key = current_column_name, .idx = 0 };
                                                                                                        //vector_push(path_stack, &e, 1);

                                                                                                        vector_pop(path_stack);

                                                                                                        err err;
                                                                                                        struct prop_iter
                                                                                                                nested_obj_prop_iter;
                                                                                                        archive_prop_iter_from_object(
                                                                                                                &nested_obj_prop_iter,
                                                                                                                mask,
                                                                                                                &err,
                                                                                                                archive_object);
                                                                                                        iterate_props(
                                                                                                                archive,
                                                                                                                &nested_obj_prop_iter,
                                                                                                                path_stack,
                                                                                                                visitor,
                                                                                                                mask,
                                                                                                                capture,
                                                                                                                false,
                                                                                                                current_column_name,
                                                                                                                current_group_idx);

                                                                                                        path_entry e =
                                                                                                                {.key = current_column_name, .idx = 0};
                                                                                                        vector_push(
                                                                                                                path_stack,
                                                                                                                &e,
                                                                                                                1);

                                                                                                }

                                                                                        }
                                                                                }
                                                                                        break;
                                                                                default:
                                                                                        break;
                                                                        }

                                                                        current_entry_idx++;
                                                                }

                                                                free(entry_object_containments);
                                                        }
                                                        vector_pop(path_stack);
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
        vector_pop(path_stack);
}

bool archive_visit_archive(archive *archive, const archive_visitor_desc *desc,
                               visitor *visitor, void *capture)
{
        ERROR_IF_NULL(archive)
        ERROR_IF_NULL(visitor)

        prop_iter prop_iter;
        vector ofType(path_entry) path_stack;

        int mask = desc ? desc->visit_mask : ARCHIVE_ITER_MASK_ANY;

        if (archive_prop_iter_from_archive(&prop_iter, &archive->err, mask, archive)) {
                vector_create(&path_stack, NULL, sizeof(path_entry), 100);
                OPTIONAL_CALL(visitor, before_visit_starts, archive, capture);
                iterate_props(archive, &prop_iter, &path_stack, visitor, mask, capture, true, 0, 0);
                OPTIONAL_CALL(visitor, after_visit_ends, archive, capture);
                vector_drop(&path_stack);
                return true;
        } else {
                return false;
        }
}

#include <inttypes.h>

void archive_visitor_path_to_string(char path_buffer[2048], archive *archive,
                                        const vector ofType(path_entry) *path_stack)
{

        query *query = archive_query_default(archive);

        for (u32 i = 0; i < path_stack->num_elems; i++) {
                const path_entry *entry = VECTOR_GET(path_stack, i, path_entry);
                if (entry->key != 0) {
                        char *key = query_fetch_string_by_id(query, entry->key);
                        size_t path_len = strlen(path_buffer);
                        sprintf(path_buffer + path_len, "%s%s", key, i + 1 < path_stack->num_elems ? ", " : "");
                        free(key);
                } else {
                        sprintf(path_buffer, "/");
                }
        }
}

bool archive_visitor_print_path(FILE *file, archive *archive,
                                    const vector ofType(path_entry) *path_stack)
{
        ERROR_IF_NULL(file)
        ERROR_IF_NULL(path_stack)
        ERROR_IF_NULL(archive)

        query *query = archive_query_default(archive);

        for (u32 i = 0; i < path_stack->num_elems; i++) {
                const path_entry *entry = VECTOR_GET(path_stack, i, path_entry);
                if (entry->key != 0) {
                        char *key = query_fetch_string_by_id(query, entry->key);
                        fprintf(file, "%s/", key);
                        free(key);
                } else {
                        fprintf(file, "/");
                }
        }
        fprintf(file, "\n");

        char buffer[2048];
        memset(buffer, 0, sizeof(buffer));
        archive_visitor_path_to_string(buffer, archive, path_stack);
        fprintf(file, "%s\n", buffer);

        return true;
}

bool archive_visitor_path_compare(const vector ofType(path_entry) *path,
                                      archive_field_sid_t *group_name, const char *path_str,
                                      archive *archive)
{
        char path_buffer[2048];
        memset(path_buffer, 0, sizeof(path_buffer));
        sprintf(path_buffer, "/");

        query *query = archive_query_default(archive);

        for (u32 i = 1; i < path->num_elems; i++) {
                const path_entry *entry = VECTOR_GET(path, i, path_entry);
                if (entry->key != 0) {
                        char *key = query_fetch_string_by_id(query, entry->key);
                        size_t path_len = strlen(path_buffer);
                        sprintf(path_buffer + path_len, "%s/", key);
                        free(key);
                }
        }

        if (group_name) {
                char *key = query_fetch_string_by_id(query, *group_name);
                size_t path_len = strlen(path_buffer);
                sprintf(path_buffer + path_len, "%s/", key);
                free(key);
        }

        fprintf(stderr, "'%s' <-> needle '%s'\n", path_buffer, path_str);

        return strcmp(path_buffer, path_str) == 0;
}