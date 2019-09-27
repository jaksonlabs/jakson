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

#ifndef ARCHIVE_VISITOR_H
#define ARCHIVE_VISITOR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/archive/it.h>

typedef struct path_entry {
        archive_field_sid_t key;
        u32 idx;
} path_entry;

typedef struct archive_visitor_desc {
        int visit_mask;                 /** bitmask of 'ARCHIVE_ITER_MASK_XXX' */
} archive_visitor_desc;

typedef enum visit_policy {
        VISIT_INCLUDE, VISIT_EXCLUDE,
} visit_policy_e;

typedef const vector ofType(path_entry) *path_stack_t;

#define DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (archive *archive, path_stack_t path, unique_id_t id,                              \
                              const archive_field_sid_t *keys, const built_in_type *values, u32 num_pairs,                     \
                              void *capture);

#define DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
visit_policy_e (*visit_enter_##name##_array_pairs)(archive *archive, path_stack_t path,                      \
                                                        unique_id_t id, const archive_field_sid_t *keys,                       \
                                                        u32 num_pairs,                                                 \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(archive *archive, path_stack_t path, unique_id_t id,                    \
                                        archive_field_sid_t key, u32 entry_idx, u32 num_elems,                                 \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (archive *archive, path_stack_t path, unique_id_t id,                         \
                                   archive_field_sid_t key, u32 entry_idx, u32 max_entries,                                    \
                                   const built_in_type *array, u32 array_length, void *capture);                       \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(archive *archive, path_stack_t path, unique_id_t id,                    \
                                        u32 pair_idx, u32 num_pairs, void *capture);                                   \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(archive *archive, path_stack_t path, unique_id_t id,                   \
                                         void *capture);

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(name, built_in_type)                                                     \
    void (*visit_object_array_object_property_##name)(archive *archive, path_stack_t path,                      \
                                               unique_id_t parent_id,                                                  \
                                               archive_field_sid_t key,                                                        \
                                               unique_id_t nested_object_id,                                           \
                                               archive_field_sid_t nested_key,                                                 \
                                               const built_in_type *nested_values,                                     \
                                               u32 num_nested_values, void *capture);

typedef struct visitor {
        void (*visit_root_object)(archive *archive, unique_id_t id, void *capture);
        void (*before_visit_starts)(archive *archive, void *capture);
        void (*after_visit_ends)(archive *archive, void *capture);
        visit_policy_e (*before_object_visit)(archive *archive, path_stack_t path, unique_id_t parent_id, unique_id_t value_id, u32 object_idx, u32 num_objects, archive_field_sid_t key, void *capture);
        void (*after_object_visit)(archive *archive, path_stack_t path, unique_id_t id, u32 object_idx, u32 num_objects, void *capture);
        void (*first_prop_type_group)(archive *archive, path_stack_t path, unique_id_t id, const archive_field_sid_t *keys, enum archive_field_type type, bool is_array, u32 num_pairs, void *capture);
        void (*next_prop_type_group)(archive *archive, path_stack_t path, unique_id_t id, const archive_field_sid_t *keys, enum archive_field_type type, bool is_array, u32 num_pairs, void *capture);
        void (*visit_null_pairs)(archive *archive, path_stack_t path, unique_id_t id, const archive_field_sid_t *keys, u32 num_pairs, void *capture);

        DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, archive_field_i8_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, archive_field_i16_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, archive_field_i32_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, archive_field_i64_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, archive_field_u8_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, archive_field_u16_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, archive_field_u32_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, archive_field_u64_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(number, archive_field_number_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(string, archive_field_sid_t);
        DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, archive_field_boolean_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, archive_field_i8_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, archive_field_i16_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, archive_field_i32_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, archive_field_i64_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, archive_field_u8_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, archive_field_u16_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, archive_field_u32_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, archive_field_u64_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, archive_field_number_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, archive_field_sid_t);
        DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, archive_field_boolean_t);

        visit_policy_e (*visit_enter_null_array_pairs)(archive *archive, path_stack_t path, unique_id_t id, const archive_field_sid_t *keys, u32 num_pairs, void *capture);
        void (*visit_enter_null_array_pair)(archive *archive, path_stack_t path, unique_id_t id, archive_field_sid_t key, u32 entry_idx, u32 num_elems, void *capture);
        void (*visit_null_array_pair)(archive *archive, path_stack_t path, unique_id_t id, archive_field_sid_t key, u32 entry_idx, u32 max_entries, archive_field_u32_t num_nulls, void *capture);
        void (*visit_leave_null_array_pair)(archive *archive, path_stack_t path, unique_id_t id, u32 pair_idx, u32 num_pairs, void *capture);
        void (*visit_leave_null_array_pairs)(archive *archive, path_stack_t path, unique_id_t id, void *capture);
        visit_policy_e (*before_visit_object_array)(archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, void *capture);
        void (*before_visit_object_array_objects)(bool *skip_group_object_ids, archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, const unique_id_t *group_object_ids, u32 num_group_object_ids, void *capture);
        visit_policy_e (*before_visit_object_array_object_property)(archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, archive_field_sid_t nested_key, enum archive_field_type nested_value_type, void *capture);

        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int8s, archive_field_i8_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int16s, archive_field_i16_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int32s, archive_field_i32_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int64s, archive_field_i64_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint8s, archive_field_u8_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint16s, archive_field_u16_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint32s, archive_field_u32_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint64s, archive_field_u64_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(numbers, archive_field_number_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(strings, archive_field_sid_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(booleans, archive_field_boolean_t);
        DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(nulls, archive_field_u32_t);

        visit_policy_e (*before_object_array_object_property_object)(archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, unique_id_t nested_object_id, archive_field_sid_t nested_key, u32 nested_value_object_id, void *capture);
        void (*visit_object_property)(archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, enum archive_field_type type, bool is_array_type, void *capture);
        void (*visit_object_array_prop)(archive *archive, path_stack_t path, unique_id_t parent_id, archive_field_sid_t key, enum archive_field_type type, void *capture);
        bool (*get_column_entry_count)(archive *archive, path_stack_t path, archive_field_sid_t key, enum archive_field_type type, u32 count, void *capture);
} archive_visitor;

bool archive_visit_archive(archive *archive, const archive_visitor_desc *desc, visitor *visitor, void *capture);
bool archive_visitor_print_path(FILE *file, archive *archive, const vector ofType(path_entry) *path_stack);
void archive_visitor_path_to_string(char path_buffer[2048], archive *archive, const vector ofType(path_entry) *path_stack);
bool archive_visitor_path_compare(const vector ofType(path_entry) *path, archive_field_sid_t *group_name, const char *path_str, archive *archive);

#endif
