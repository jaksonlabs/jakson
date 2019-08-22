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

struct path_entry {
    jak_field_sid key;
    jak_u32 idx;
};

struct jak_archive_visitor_desc {
    int visit_mask;                 /** bitmask of 'JAK_ARCHIVE_ITER_MASK_XXX' */
};

enum visit_policy {
    VISIT_INCLUDE, VISIT_EXCLUDE,
};

typedef const struct vector ofType(struct path_entry) *path_stack_t;

#define DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (struct jak_archive *jak_archive, path_stack_t path, global_id_t id,                              \
                              const jak_field_sid *keys, const built_in_type *values, jak_u32 num_pairs,                     \
                              void *capture);

#define DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
enum visit_policy (*visit_enter_##name##_array_pairs)(struct jak_archive *jak_archive, path_stack_t path,                      \
                                                        global_id_t id, const jak_field_sid *keys,                       \
                                                        jak_u32 num_pairs,                                                 \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(struct jak_archive *jak_archive, path_stack_t path, global_id_t id,                    \
                                        jak_field_sid key, jak_u32 entry_idx, jak_u32 num_elems,                                 \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (struct jak_archive *jak_archive, path_stack_t path, global_id_t id,                         \
                                   jak_field_sid key, jak_u32 entry_idx, jak_u32 max_entries,                                    \
                                   const built_in_type *array, jak_u32 array_length, void *capture);                       \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(struct jak_archive *jak_archive, path_stack_t path, global_id_t id,                    \
                                        jak_u32 pair_idx, jak_u32 num_pairs, void *capture);                                   \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(struct jak_archive *jak_archive, path_stack_t path, global_id_t id,                   \
                                         void *capture);

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(name, built_in_type)                                                     \
    void (*visit_object_array_object_property_##name)(struct jak_archive *jak_archive, path_stack_t path,                      \
                                               global_id_t parent_id,                                                  \
                                               jak_field_sid key,                                                        \
                                               global_id_t nested_object_id,                                           \
                                               jak_field_sid nested_key,                                                 \
                                               const built_in_type *nested_values,                                     \
                                               jak_u32 num_nested_values, void *capture);

struct jak_archive_visitor {
    void (*visit_root_object)(struct jak_archive *archive, global_id_t id, void *capture);

    void (*before_visit_starts)(struct jak_archive *archive, void *capture);

    void (*after_visit_ends)(struct jak_archive *archive, void *capture);

    enum visit_policy (*before_object_visit)(struct jak_archive *archive, path_stack_t path, global_id_t parent_id,
                                             global_id_t value_id, jak_u32 object_idx, jak_u32 num_objects, jak_field_sid key,
                                             void *capture);

    void (*after_object_visit)(struct jak_archive *archive, path_stack_t path, global_id_t id, jak_u32 object_idx,
                               jak_u32 num_objects, void *capture);

    void (*first_prop_type_group)(struct jak_archive *archive, path_stack_t path, global_id_t id,
                                  const jak_field_sid *keys, enum field_type type, bool is_array, jak_u32 num_pairs,
                                  void *capture);

    void (*next_prop_type_group)(struct jak_archive *archive, path_stack_t path, global_id_t id,
                                 const jak_field_sid *keys, enum field_type type, bool is_array, jak_u32 num_pairs,
                                 void *capture);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, field_i8_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, field_i16_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, field_i32_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, field_i64_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, field_u8_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, field_u16_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, field_u32_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, field_u64_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(number, field_number_t);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(string, jak_field_sid);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, field_boolean_t);

    void (*visit_null_pairs)(struct jak_archive *archive, path_stack_t path, global_id_t id, const jak_field_sid *keys,
                             jak_u32 num_pairs, void *capture);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, field_i8_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, field_i16_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, field_i32_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, field_i64_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, field_u8_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, field_u16_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, field_u32_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, field_u64_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, field_number_t);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, jak_field_sid);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, field_boolean_t);

    enum visit_policy (*visit_enter_null_array_pairs)(struct jak_archive *archive, path_stack_t path, global_id_t id,
                                                      const jak_field_sid *keys, jak_u32 num_pairs, void *capture);

    void (*visit_enter_null_array_pair)(struct jak_archive *archive, path_stack_t path, global_id_t id, jak_field_sid key,
                                        jak_u32 entry_idx, jak_u32 num_elems, void *capture);

    void (*visit_null_array_pair)(struct jak_archive *archive, path_stack_t path, global_id_t id, jak_field_sid key,
                                  jak_u32 entry_idx, jak_u32 max_entries, field_u32_t num_nulls, void *capture);

    void (*visit_leave_null_array_pair)(struct jak_archive *archive, path_stack_t path, global_id_t id, jak_u32 pair_idx,
                                        jak_u32 num_pairs, void *capture);

    void (*visit_leave_null_array_pairs)(struct jak_archive *archive, path_stack_t path, global_id_t id, void *capture);

    enum visit_policy (*before_visit_object_array)(struct jak_archive *archive, path_stack_t path,
                                                   global_id_t parent_id, jak_field_sid key, void *capture);

    void (*before_visit_object_array_objects)(bool *skip_group_object_ids, struct jak_archive *archive,
                                              path_stack_t path, global_id_t parent_id, jak_field_sid key,
                                              const global_id_t *group_object_ids,
                                              jak_u32 num_group_object_ids, void *capture);

    enum visit_policy (*before_visit_object_array_object_property)(struct jak_archive *archive, path_stack_t path,
                                                                   global_id_t parent_id, jak_field_sid key,
                                                                   jak_field_sid nested_key,
                                                                   enum field_type nested_value_type,
                                                                   void *capture);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int8s, field_i8_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int16s, field_i16_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int32s, field_i32_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int64s, field_i64_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint8s, field_u8_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint16s, field_u16_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint32s, field_u32_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint64s, field_u64_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(numbers, field_number_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(strings, jak_field_sid);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(booleans, field_boolean_t);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(nulls, field_u32_t);

    enum visit_policy (*before_object_array_object_property_object)(struct jak_archive *archive, path_stack_t path,
                                                                    global_id_t parent_id, jak_field_sid key,
                                                                    global_id_t nested_object_id,
                                                                    jak_field_sid nested_key,
                                                                    jak_u32 nested_value_object_id, void *capture);

    void (*visit_object_property)(struct jak_archive *archive, path_stack_t path, global_id_t parent_id,
                                  jak_field_sid key, enum field_type type, bool is_array_type, void *capture);

    void (*visit_object_array_prop)(struct jak_archive *archive, path_stack_t path, global_id_t parent_id,
                                    jak_field_sid key, enum field_type type, void *capture);

    bool (*get_column_entry_count)(struct jak_archive *archive, path_stack_t path, jak_field_sid key,
                                   enum field_type type, jak_u32 count, void *capture);

};

bool archive_visit_archive(struct jak_archive *archive, const struct jak_archive_visitor_desc *desc,
                           struct jak_archive_visitor *visitor, void *capture);

bool archive_visitor_print_path(FILE *file, struct jak_archive *archive,
                                const struct vector ofType(struct path_entry) *path_stack);

void archive_visitor_path_to_string(char path_buffer[2048], struct jak_archive *archive,
                                    const struct vector ofType(struct path_entry) *path_stack);

bool archive_visitor_path_compare(const struct vector ofType(struct path_entry) *path,
                                  jak_field_sid *group_name, const char *path_str, struct jak_archive *archive);

#endif
