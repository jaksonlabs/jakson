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

#include <jak_archive_visitor.h>
#include <jak_hash_table.h>
#include <jak_hash_set.h>
#include <jak_archive_visitor.h>
#include <jak_archive_query.h>

static void iterate_props(struct jak_archive *archive, struct jak_prop_iter *prop_iter,
                          struct jak_vector ofType(struct jak_path_entry ) *path_stack, struct jak_archive_visitor *visitor,
                          int mask, void *capture,
                          bool is_root_object, jak_archive_field_sid_t parent_key, jak_u32 parent_key_array_idx);

static void iterate_objects(struct jak_archive *archive, const jak_archive_field_sid_t *keys, jak_u32 num_pairs,
                            struct jak_archive_value_vector *value_iter,
                            struct jak_vector ofType(struct jak_path_entry ) *path_stack,
                            struct jak_archive_visitor *visitor, int mask, void *capture, bool is_root_object)
{
        JAK_UNUSED(num_pairs);

        jak_u32 vector_length;
        struct jak_archive_object object;
        jak_global_id_t parent_object_id;
        jak_global_id_t object_id;
        struct jak_prop_iter prop_iter;
        struct jak_error err;

        jak_archive_value_vector_get_object_id(&parent_object_id, value_iter);
        jak_jak_archive_value_vector_get_length(&vector_length, value_iter);
        JAK_ASSERT(num_pairs == vector_length);

        for (jak_u32 i = 0; i < vector_length; i++) {
                jak_archive_field_sid_t parent_key = keys[i];
                jak_u32 parent_key_array_idx = i;

//        struct jak_path_entry  e = { .key = parent_key, .idx = 0 };
//        vec_push(path_stack, &e, 1);

                //  fprintf(stderr, "XXXX object: ");
                //  jak_archive_visitor_print_path(stderr, archive, path_stack);

                jak_jak_archive_value_vector_get_object_at(&object, i, value_iter);
                jak_archive_object_get_object_id(&object_id, &object);

                jak_archive_prop_iter_from_object(&prop_iter, mask, &err, &object);

                if (!is_root_object) {
                        enum jak_visit_policy visit = JAK_VISIT_INCLUDE;
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
                        if (visit == JAK_VISIT_INCLUDE) {
                                iterate_props(archive,
                                              &prop_iter,
                                              path_stack,
                                              visitor,
                                              mask,
                                              capture,
                                              false,
                                              parent_key,
                                              parent_key_array_idx);
                                JAK_optional_call(visitor,
                                                  after_object_visit,
                                                  archive,
                                                  path_stack,
                                                  object_id,
                                                  i,
                                                  vector_length,
                                                  capture);
                        }
                } else {
                        JAK_optional_call(visitor, visit_root_object, archive, object_id, capture);
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

                //  vec_pop(path_stack);
        }
}

#define SET_TYPE_SWITCH_CASE(name, built_in_type)                                                                      \
{                                                                                                                      \
    if (is_array) {                                                                                                    \
        enum jak_visit_policy visit = JAK_VISIT_INCLUDE;                                                 \
        if (visitor->visit_enter_##name##_array_pairs) {                                                               \
            visit = visitor->visit_enter_##name##_array_pairs(archive, path_stack, this_object_oid, keys, num_pairs, capture);     \
        }                                                                                                              \
        if (visit == JAK_VISIT_INCLUDE) {                                                                  \
            for (jak_u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++)                                              \
            {                                                                                                          \
                jak_u32 array_length;                                                                                 \
                const built_in_type *values = jak_archive_value_vector_get_##name##_arrays_at(&array_length,        \
                                                                                             prop_idx,                 \
                                                                                             &value_iter);             \
                JAK_optional_call(visitor, visit_enter_##name##_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],      \
                              prop_idx, array_length, capture);                                                        \
                JAK_optional_call(visitor, visit_##name##_array_pair, archive, path_stack, this_object_oid, keys[prop_idx],            \
                              prop_idx, num_pairs, values, array_length, capture)                                      \
                JAK_optional_call(visitor, visit_leave_##name##_array_pair, archive, path_stack, this_object_oid, prop_idx, num_pairs, \
                                                                        capture);                                      \
            }                                                                                                          \
            JAK_optional_call(visitor, visit_leave_##name##_array_pairs, archive, path_stack, this_object_oid, capture);               \
        }                                                                                                              \
    } else                                                                                                             \
    {                                                                                                                  \
        if (visitor->visit_##name##_pairs) {                                                                           \
            const built_in_type *values = jak_archive_value_vector_get_##name##s(NULL, &value_iter);                \
            visitor->visit_##name##_pairs(archive, path_stack, this_object_oid, keys, values, num_pairs, capture);                 \
        }                                                                                                              \
    }                                                                                                                  \
}

#define SET_NESTED_ARRAY_SWITCH_CASE(name, built_in_type)                                                              \
{                                                                                                                      \
    const built_in_type *values = jak_archive_column_entry_get_##name(&entry_length, &entry_iter);                  \
    JAK_optional_call(visitor, visit_object_array_object_property_##name,                                                  \
                  archive, path_stack, this_object_oid, group_key, current_nested_object_id,                           \
                  current_column_name, values, entry_length, capture);                                                 \
}

static void iterate_props(struct jak_archive *archive, struct jak_prop_iter *prop_iter,
                          struct jak_vector ofType(struct jak_path_entry ) *path_stack, struct jak_archive_visitor *visitor,
                          int mask, void *capture,
                          bool is_root_object, jak_archive_field_sid_t parent_key, jak_u32 parent_key_array_idx)
{
        jak_global_id_t this_object_oid;
        struct jak_archive_value_vector value_iter;
        enum jak_archive_field_type type;
        bool is_array;
        const jak_archive_field_sid_t *keys;
        jak_u32 num_pairs;
        enum jak_prop_iter_mode iter_type;
        struct jak_independent_iter_state collection_iter;
        bool first_type_group = true;

        JAK_UNUSED(parent_key);
        JAK_UNUSED(parent_key_array_idx);

        struct jak_path_entry  e = {.key = parent_key, .idx = parent_key_array_idx};
        vec_push(path_stack, &e, 1);

        jak_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

        while (jak_archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter)) {

                if (iter_type == JAK_PROP_ITER_MODE_OBJECT) {

                        keys = jak_jak_archive_value_vector_get_keys(&num_pairs, &value_iter);
                        jak_archive_value_vector_is_array_type(&is_array, &value_iter);
                        jak_jak_archive_value_vector_get_basic_type(&type, &value_iter);
                        jak_archive_value_vector_get_object_id(&this_object_oid, &value_iter);

                        for (jak_u32 i = 0; i < num_pairs; i++) {
                                JAK_optional_call(visitor,
                                                  visit_object_property,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys[i],
                                                  type,
                                                  is_array,
                                                  capture);

                                struct jak_path_entry  e = {.key = keys[i], .idx = 666};
                                vec_push(path_stack, &e, 1);
                                //jak_archive_visitor_print_path(stderr, archive, path_stack);
                                JAK_optional_call(visitor,
                                                  visit_object_array_prop,
                                                  archive,
                                                  path_stack,
                                                  this_object_oid,
                                                  keys[i],
                                                  type,
                                                  capture);
                                vec_pop(path_stack);
                        }

                        if (JAK_UNLIKELY(first_type_group)) {
                                JAK_optional_call(visitor,
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
                                JAK_optional_call(visitor,
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
                                case JAK_FIELD_OBJECT:
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
                                case JAK_FIELD_NULL:
                                        if (is_array) {
                                                enum jak_visit_policy visit = JAK_VISIT_INCLUDE;
                                                if (visitor->visit_enter_null_array_pairs) {
                                                        visit = visitor->visit_enter_null_array_pairs(archive,
                                                                                                      path_stack,
                                                                                                      this_object_oid,
                                                                                                      keys,
                                                                                                      num_pairs,
                                                                                                      capture);
                                                }
                                                if (visit == JAK_VISIT_INCLUDE) {
                                                        const jak_archive_field_u32_t *num_values =
                                                                jak_archive_value_vector_get_null_arrays(NULL, &value_iter);
                                                        for (jak_u32 prop_idx = 0; prop_idx < num_pairs; prop_idx++) {
                                                                JAK_optional_call(visitor,
                                                                                  visit_enter_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  keys[prop_idx],
                                                                                  prop_idx,
                                                                                  num_values[prop_idx],
                                                                                  capture);
                                                                JAK_optional_call(visitor,
                                                                                  visit_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  keys[prop_idx],
                                                                                  prop_idx,
                                                                                  num_pairs,
                                                                                  num_values[prop_idx],
                                                                                  capture)
                                                                JAK_optional_call(visitor,
                                                                                  visit_leave_null_array_pair,
                                                                                  archive,
                                                                                  path_stack,
                                                                                  this_object_oid,
                                                                                  prop_idx,
                                                                                  num_pairs,
                                                                                  capture);
                                                        }
                                                        JAK_optional_call(visitor,
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
                                case JAK_FIELD_INT8: SET_TYPE_SWITCH_CASE(int8, jak_archive_field_i8_t)
                                        break;
                                case JAK_FIELD_INT16: SET_TYPE_SWITCH_CASE(int16, jak_archive_field_i16_t)
                                        break;
                                case JAK_FIELD_INT32: SET_TYPE_SWITCH_CASE(int32, jak_archive_field_i32_t)
                                        break;
                                case JAK_FIELD_INT64: SET_TYPE_SWITCH_CASE(int64, jak_archive_field_i64_t)
                                        break;
                                case JAK_FIELD_UINT8: SET_TYPE_SWITCH_CASE(uint8, jak_archive_field_u8_t)
                                        break;
                                case JAK_FIELD_UINT16: SET_TYPE_SWITCH_CASE(uint16, jak_archive_field_u16_t)
                                        break;
                                case JAK_FIELD_UINT32: SET_TYPE_SWITCH_CASE(uint32, jak_archive_field_u32_t)
                                        break;
                                case JAK_FIELD_UINT64: SET_TYPE_SWITCH_CASE(uint64, jak_archive_field_u64_t)
                                        break;
                                case JAK_FIELD_FLOAT: SET_TYPE_SWITCH_CASE(number, jak_archive_field_number_t)
                                        break;
                                case JAK_FIELD_STRING: SET_TYPE_SWITCH_CASE(string, jak_archive_field_sid_t)
                                        break;
                                case JAK_FIELD_BOOLEAN: SET_TYPE_SWITCH_CASE(boolean, jak_archive_field_boolean_t)
                                        break;
                                default:
                                        break;
                        }

                        first_type_group = false;
                } else {
                        struct jak_independent_iter_state group_iter;
                        jak_u32 num_column_groups;
                        keys = jak_archive_collection_iter_get_keys(&num_column_groups, &collection_iter);

                        bool *skip_groups_by_key = JAK_MALLOC(num_column_groups * sizeof(bool));
                        JAK_zero_memory(skip_groups_by_key, num_column_groups * sizeof(bool));

                        if (visitor->before_visit_object_array) {
                                for (jak_u32 i = 0; i < num_column_groups; i++) {

                                        //     struct jak_path_entry  e = { .key = parent_key, .idx = i };
                                        //vec_push(path_stack, &e, 1);


                                        enum jak_visit_policy policy = visitor->before_visit_object_array(archive,
                                                                                                      path_stack,
                                                                                                      this_object_oid,
                                                                                                      keys[i],
                                                                                                      capture);

                                        //     vec_pop(path_stack);

                                        skip_groups_by_key[i] = policy == JAK_VISIT_EXCLUDE;
                                }
                        }

                        jak_u32 current_group_idx = 0;

                        while (jak_archive_collection_next_column_group(&group_iter, &collection_iter)) {
                                if (!skip_groups_by_key[current_group_idx]) {

                                        jak_u32 num_column_group_objs;
                                        struct jak_independent_iter_state column_iter;
                                        jak_archive_field_sid_t group_key = keys[current_group_idx];
                                        const jak_global_id_t *column_group_object_ids =
                                                jak_archive_column_group_get_object_ids(&num_column_group_objs,
                                                                                    &group_iter);
                                        bool *skip_objects = JAK_MALLOC(num_column_group_objs * sizeof(bool));
                                        JAK_zero_memory(skip_objects, num_column_group_objs * sizeof(bool));

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

                                        jak_u32 current_column_group_obj_idx = 0;

                                        while (jak_archive_column_group_next_column(&column_iter, &group_iter)) {

                                                if (!skip_objects[current_column_group_obj_idx]) {
                                                        jak_archive_field_sid_t current_column_name;
                                                        enum jak_archive_field_type current_column_entry_type;

                                                        jak_archive_column_get_name(&current_column_name,
                                                                                &current_column_entry_type,
                                                                                &column_iter);

                                                        struct jak_path_entry  e = {.key = current_column_name, .idx = 0};
                                                        vec_push(path_stack, &e, 1);

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

                                                        JAK_optional_call(visitor,
                                                                          visit_object_array_prop,
                                                                          archive,
                                                                          path_stack,
                                                                          this_object_oid,
                                                                          current_column_name,
                                                                          current_column_entry_type,
                                                                          capture);

                                                        bool skip_column = false;
                                                        if (visitor->before_visit_object_array_object_property) {
                                                                enum jak_visit_policy policy =
                                                                        visitor->before_visit_object_array_object_property(
                                                                                archive,
                                                                                path_stack,
                                                                                this_object_oid,
                                                                                group_key,
                                                                                current_column_name,
                                                                                current_column_entry_type,
                                                                                capture);
                                                                skip_column = policy == JAK_VISIT_EXCLUDE;
                                                        }

                                                        if (!skip_column) {
                                                                jak_u32 num_positions;
                                                                const jak_u32 *entry_positions =
                                                                        jak_archive_column_get_entry_positions(
                                                                                &num_positions,
                                                                                &column_iter);
                                                                struct jak_independent_iter_state entry_iter;

                                                                jak_global_id_t *entry_object_containments =
                                                                        JAK_MALLOC(num_positions * sizeof(jak_global_id_t));
                                                                for (jak_u32 m = 0; m < num_positions; m++) {
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

                                                                jak_u32 current_entry_idx = 0;
                                                                while (jak_archive_column_next_entry(&entry_iter,
                                                                                                 &column_iter)) {

                                                                        jak_global_id_t current_nested_object_id =
                                                                                entry_object_containments[current_entry_idx];
                                                                        jak_u32 entry_length;

                                                                        switch (current_column_entry_type) {
                                                                                case JAK_FIELD_INT8: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int8s,
                                                                                                jak_archive_field_i8_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_INT16: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int16s,
                                                                                                jak_archive_field_i16_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_INT32: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int32s,
                                                                                                jak_archive_field_i32_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_INT64: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                int64s,
                                                                                                jak_archive_field_i64_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_UINT8: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint8s,
                                                                                                jak_archive_field_u8_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_UINT16: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint16s,
                                                                                                jak_archive_field_u16_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_UINT32: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint32s,
                                                                                                jak_archive_field_u32_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_UINT64: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                uint64s,
                                                                                                jak_archive_field_u64_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_FLOAT: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                numbers,
                                                                                                jak_archive_field_number_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_STRING: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                strings,
                                                                                                jak_archive_field_sid_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_BOOLEAN: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                booleans,
                                                                                                jak_archive_field_boolean_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_NULL: {
                                                                                        SET_NESTED_ARRAY_SWITCH_CASE(
                                                                                                nulls,
                                                                                                jak_archive_field_u32_t)
                                                                                }
                                                                                        break;
                                                                                case JAK_FIELD_OBJECT: {
                                                                                        struct jak_column_object_iter iter;
                                                                                        const struct jak_archive_object
                                                                                                *archive_object;
                                                                                        jak_archive_column_entry_get_objects(
                                                                                                &iter,
                                                                                                &entry_iter);

                                                                                        while ((archive_object =
                                                                                                        jak_archive_column_entry_object_iter_next_object(
                                                                                                                &iter))
                                                                                               != NULL) {
                                                                                                jak_global_id_t id;
                                                                                                jak_archive_object_get_object_id(
                                                                                                        &id,
                                                                                                        archive_object);

                                                                                                bool skip_object = false;
                                                                                                if (visitor
                                                                                                        ->before_object_array_object_property_object) {
                                                                                                        enum jak_visit_policy
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
                                                                                                                JAK_VISIT_EXCLUDE;
                                                                                                }

                                                                                                if (!skip_object) {


                                                                                                        //keys[i]

                                                                                                        //struct jak_path_entry  e = { .key = current_column_name, .idx = 0 };
                                                                                                        //vec_push(path_stack, &e, 1);

                                                                                                        vec_pop(path_stack);

                                                                                                        struct jak_error err;
                                                                                                        struct jak_prop_iter
                                                                                                                nested_obj_prop_iter;
                                                                                                        jak_archive_prop_iter_from_object(
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

                                                                                                        struct jak_path_entry  e =
                                                                                                                {.key = current_column_name, .idx = 0};
                                                                                                        vec_push(
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
                                                        vec_pop(path_stack);
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
        vec_pop(path_stack);
}

bool jak_archive_visit_archive(struct jak_archive *archive, const struct jak_archive_visitor_desc *desc,
                           struct jak_archive_visitor *visitor, void *capture)
{
        JAK_ERROR_IF_NULL(archive)
        JAK_ERROR_IF_NULL(visitor)

        struct jak_prop_iter prop_iter;
        struct jak_vector ofType(path_entry) path_stack;

        int mask = desc ? desc->visit_mask : JAK_ARCHIVE_ITER_MASK_ANY;

        if (jak_archive_prop_iter_from_archive(&prop_iter, &archive->err, mask, archive)) {
                vec_create(&path_stack, NULL, sizeof(struct jak_path_entry ), 100);
                JAK_optional_call(visitor, before_visit_starts, archive, capture);
                iterate_props(archive, &prop_iter, &path_stack, visitor, mask, capture, true, 0, 0);
                JAK_optional_call(visitor, after_visit_ends, archive, capture);
                vec_drop(&path_stack);
                return true;
        } else {
                return false;
        }
}

#include <inttypes.h>

void jak_archive_visitor_path_to_string(char path_buffer[2048], struct jak_archive *archive,
                                    const struct jak_vector ofType(struct jak_path_entry ) *path_stack)
{

        struct jak_archive_query *query = jak_archive_query_default(archive);

        for (jak_u32 i = 0; i < path_stack->num_elems; i++) {
                const struct jak_path_entry  *entry = vec_get(path_stack, i, struct jak_path_entry );
                if (entry->key != 0) {
                        char *key = jak_query_fetch_string_by_id(query, entry->key);
                        size_t path_len = strlen(path_buffer);
                        sprintf(path_buffer + path_len, "%s%s", key, i + 1 < path_stack->num_elems ? ", " : "");
                        free(key);
                } else {
                        sprintf(path_buffer, "/");
                }
        }
}

bool jak_archive_visitor_print_path(FILE *file, struct jak_archive *archive,
                                const struct jak_vector ofType(struct jak_path_entry ) *path_stack)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(path_stack)
        JAK_ERROR_IF_NULL(archive)

        struct jak_archive_query *query = jak_archive_query_default(archive);

        for (jak_u32 i = 0; i < path_stack->num_elems; i++) {
                const struct jak_path_entry  *entry = vec_get(path_stack, i, struct jak_path_entry );
                if (entry->key != 0) {
                        char *key = jak_query_fetch_string_by_id(query, entry->key);
                        fprintf(file, "%s/", key);
                        free(key);
                } else {
                        fprintf(file, "/");
                }
        }
        fprintf(file, "\n");

        char buffer[2048];
        memset(buffer, 0, sizeof(buffer));
        jak_archive_visitor_path_to_string(buffer, archive, path_stack);
        fprintf(file, "%s\n", buffer);

        return true;
}

bool jak_archive_visitor_path_compare(const struct jak_vector ofType(struct jak_path_entry ) *path,
                                  jak_archive_field_sid_t *group_name, const char *path_str, struct jak_archive *archive)
{
        char path_buffer[2048];
        memset(path_buffer, 0, sizeof(path_buffer));
        sprintf(path_buffer, "/");

        struct jak_archive_query *query = jak_archive_query_default(archive);

        for (jak_u32 i = 1; i < path->num_elems; i++) {
                const struct jak_path_entry  *entry = vec_get(path, i, struct jak_path_entry );
                if (entry->key != 0) {
                        char *key = jak_query_fetch_string_by_id(query, entry->key);
                        size_t path_len = strlen(path_buffer);
                        sprintf(path_buffer + path_len, "%s/", key);
                        free(key);
                }
        }

        if (group_name) {
                char *key = jak_query_fetch_string_by_id(query, *group_name);
                size_t path_len = strlen(path_buffer);
                sprintf(path_buffer + path_len, "%s/", key);
                free(key);
        }

        fprintf(stderr, "'%s' <-> needle '%s'\n", path_buffer, path_str);

        return strcmp(path_buffer, path_str) == 0;
}