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

#ifndef JAK_ARCHIVE_VISITOR_H
#define JAK_ARCHIVE_VISITOR_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include "jak_archive_it.h"

struct jak_path_entry {
        jak_archive_field_sid_t key;
        jak_u32 idx;
};

struct jak_archive_visitor_desc {
        int visit_mask;                 /** bitmask of 'JAK_ARCHIVE_ITER_MASK_XXX' */
};

enum jak_visit_policy {
        JAK_VISIT_INCLUDE, JAK_VISIT_EXCLUDE,
};

typedef const struct jak_vector ofType(struct jak_path_entry) *path_stack_t;

#define JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                              \
                              const jak_archive_field_sid_t *keys, const built_in_type *values, jak_u32 num_pairs,                     \
                              void *capture);

#define JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
enum jak_visit_policy (*visit_enter_##name##_array_pairs)(struct jak_archive *jak_archive, path_stack_t path,                      \
                                                        jak_global_id_t id, const jak_archive_field_sid_t *keys,                       \
                                                        jak_u32 num_pairs,                                                 \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                    \
                                        jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 num_elems,                                 \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                         \
                                   jak_archive_field_sid_t key, jak_u32 entry_idx, jak_u32 max_entries,                                    \
                                   const built_in_type *array, jak_u32 array_length, void *capture);                       \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                    \
                                        jak_u32 pair_idx, jak_u32 num_pairs, void *capture);                                   \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(struct jak_archive *jak_archive, path_stack_t path, jak_global_id_t id,                   \
                                         void *capture);

#define JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(name, built_in_type)                                                     \
    void (*visit_object_array_object_property_##name)(struct jak_archive *jak_archive, path_stack_t path,                      \
                                               jak_global_id_t parent_id,                                                  \
                                               jak_archive_field_sid_t key,                                                        \
                                               jak_global_id_t nested_object_id,                                           \
                                               jak_archive_field_sid_t nested_key,                                                 \
                                               const built_in_type *nested_values,                                     \
                                               jak_u32 num_nested_values, void *capture);

struct jak_archive_visitor {
        void (*visit_root_object)(struct jak_archive *archive, jak_global_id_t id, void *capture);

        void (*before_visit_starts)(struct jak_archive *archive, void *capture);

        void (*after_visit_ends)(struct jak_archive *archive, void *capture);

        enum jak_visit_policy
        (*before_object_visit)(struct jak_archive *archive, path_stack_t path, jak_global_id_t parent_id,
                               jak_global_id_t value_id, jak_u32 object_idx, jak_u32 num_objects,
                               jak_archive_field_sid_t key,
                               void *capture);

        void
        (*after_object_visit)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id, jak_u32 object_idx,
                              jak_u32 num_objects, void *capture);

        void (*first_prop_type_group)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                      const jak_archive_field_sid_t *keys, enum jak_archive_field_type type,
                                      bool is_array, jak_u32 num_pairs,
                                      void *capture);

        void (*next_prop_type_group)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                     const jak_archive_field_sid_t *keys, enum jak_archive_field_type type,
                                     bool is_array, jak_u32 num_pairs,
                                     void *capture);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, jak_archive_field_i8_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, jak_archive_field_i16_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, jak_archive_field_i32_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, jak_archive_field_i64_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, jak_archive_field_u8_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, jak_archive_field_u16_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, jak_archive_field_u32_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, jak_archive_field_u64_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(number, jak_archive_field_number_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(string, jak_archive_field_sid_t);

        JAK_DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, jak_archive_field_boolean_t);

        void (*visit_null_pairs)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                 const jak_archive_field_sid_t *keys,
                                 jak_u32 num_pairs, void *capture);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, jak_archive_field_i8_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, jak_archive_field_i16_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, jak_archive_field_i32_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, jak_archive_field_i64_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, jak_archive_field_u8_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, jak_archive_field_u16_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, jak_archive_field_u32_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, jak_archive_field_u64_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, jak_archive_field_number_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, jak_archive_field_sid_t);

        JAK_DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, jak_archive_field_boolean_t);

        enum jak_visit_policy
        (*visit_enter_null_array_pairs)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                        const jak_archive_field_sid_t *keys, jak_u32 num_pairs, void *capture);

        void (*visit_enter_null_array_pair)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                            jak_archive_field_sid_t key,
                                            jak_u32 entry_idx, jak_u32 num_elems, void *capture);

        void (*visit_null_array_pair)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                      jak_archive_field_sid_t key,
                                      jak_u32 entry_idx, jak_u32 max_entries, jak_archive_field_u32_t num_nulls,
                                      void *capture);

        void (*visit_leave_null_array_pair)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                            jak_u32 pair_idx,
                                            jak_u32 num_pairs, void *capture);

        void (*visit_leave_null_array_pairs)(struct jak_archive *archive, path_stack_t path, jak_global_id_t id,
                                             void *capture);

        enum jak_visit_policy (*before_visit_object_array)(struct jak_archive *archive, path_stack_t path,
                                                           jak_global_id_t parent_id, jak_archive_field_sid_t key,
                                                           void *capture);

        void (*before_visit_object_array_objects)(bool *skip_group_object_ids, struct jak_archive *archive,
                                                  path_stack_t path, jak_global_id_t parent_id,
                                                  jak_archive_field_sid_t key,
                                                  const jak_global_id_t *group_object_ids,
                                                  jak_u32 num_group_object_ids, void *capture);

        enum jak_visit_policy
        (*before_visit_object_array_object_property)(struct jak_archive *archive, path_stack_t path,
                                                     jak_global_id_t parent_id, jak_archive_field_sid_t key,
                                                     jak_archive_field_sid_t nested_key,
                                                     enum jak_archive_field_type nested_value_type,
                                                     void *capture);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int8s, jak_archive_field_i8_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int16s, jak_archive_field_i16_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int32s, jak_archive_field_i32_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int64s, jak_archive_field_i64_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint8s, jak_archive_field_u8_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint16s, jak_archive_field_u16_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint32s, jak_archive_field_u32_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint64s, jak_archive_field_u64_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(numbers, jak_archive_field_number_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(strings, jak_archive_field_sid_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(booleans, jak_archive_field_boolean_t);

        JAK_DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(nulls, jak_archive_field_u32_t);

        enum jak_visit_policy
        (*before_object_array_object_property_object)(struct jak_archive *archive, path_stack_t path,
                                                      jak_global_id_t parent_id, jak_archive_field_sid_t key,
                                                      jak_global_id_t nested_object_id,
                                                      jak_archive_field_sid_t nested_key,
                                                      jak_u32 nested_value_object_id, void *capture);

        void (*visit_object_property)(struct jak_archive *archive, path_stack_t path, jak_global_id_t parent_id,
                                      jak_archive_field_sid_t key, enum jak_archive_field_type type, bool is_array_type,
                                      void *capture);

        void (*visit_object_array_prop)(struct jak_archive *archive, path_stack_t path, jak_global_id_t parent_id,
                                        jak_archive_field_sid_t key, enum jak_archive_field_type type, void *capture);

        bool (*get_column_entry_count)(struct jak_archive *archive, path_stack_t path, jak_archive_field_sid_t key,
                                       enum jak_archive_field_type type, jak_u32 count, void *capture);

};

bool jak_archive_visit_archive(struct jak_archive *archive, const struct jak_archive_visitor_desc *desc,
                               struct jak_archive_visitor *visitor, void *capture);

bool jak_archive_visitor_print_path(FILE *file, struct jak_archive *archive,
                                    const struct jak_vector ofType(struct jak_path_entry) *path_stack);

void jak_archive_visitor_path_to_string(char path_buffer[2048], struct jak_archive *archive,
                                        const struct jak_vector ofType(struct jak_path_entry) *path_stack);

bool jak_archive_visitor_path_compare(const struct jak_vector ofType(struct jak_path_entry) *path,
                                      jak_archive_field_sid_t *group_name, const char *path_str,
                                      struct jak_archive *archive);

#endif
