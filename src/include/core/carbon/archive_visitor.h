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

#ifndef NG5_NG5_ARCHIVE_VISITOR_H
#define NG5_NG5_ARCHIVE_VISITOR_H

#include "archive_iter.h"

typedef struct carbon_path_entry carbon_path_entry_t;

typedef struct carbon_path_entry
{
    carbon_string_id_t   key;
    u32 idx;

} carbon_path_entry_t;

typedef struct
{
    int visit_mask;                 /** bitmask of 'NG5_ARCHIVE_ITER_MASK_XXX' */

} carbon_archive_visitor_desc_t;

typedef enum
{
    NG5_VISITOR_POLICY_INCLUDE,
    NG5_VISITOR_POLICY_EXCLUDE,
} carbon_visitor_policy_e;

typedef const struct vector ofType(carbon_path_entry_t) * path_stack_t;

#define DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (struct archive *archive, path_stack_t path, carbon_object_id_t id,                     \
                              const carbon_string_id_t *keys, const built_in_type *values, u32 num_pairs,         \
                              void *capture);

#define DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
carbon_visitor_policy_e (*visit_enter_##name##_array_pairs)(struct archive *archive, path_stack_t path,              \
                                                        carbon_object_id_t id, const carbon_string_id_t *keys,         \
                                                        u32 num_pairs,                                            \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(struct archive *archive, path_stack_t path, carbon_object_id_t id,           \
                                        carbon_string_id_t key, u32 entry_idx, u32 num_elems,                \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (struct archive *archive, path_stack_t path, carbon_object_id_t id,                \
                                   carbon_string_id_t key, u32 entry_idx, u32 max_entries,                   \
                                   const built_in_type *array, u32 array_length, void *capture);                  \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(struct archive *archive, path_stack_t path, carbon_object_id_t id,           \
                                        u32 pair_idx, u32 num_pairs, void *capture);                         \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(struct archive *archive, path_stack_t path, carbon_object_id_t id,          \
                                         void *capture);

#define DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(name, built_in_type)                                                     \
    void (*visit_object_array_object_property_##name)(struct archive *archive, path_stack_t path,                    \
                                               carbon_object_id_t parent_id,                                           \
                                               carbon_string_id_t key,                                                 \
                                               carbon_object_id_t nested_object_id,                                    \
                                               carbon_string_id_t nested_key,                                          \
                                               const built_in_type *nested_values,                                     \
                                               u32 num_nested_values, void *capture);

typedef struct
{
    void (*visit_root_object)(struct archive *archive, carbon_object_id_t id, void *capture);
    void (*before_visit_starts)(struct archive *archive, void *capture);
    void (*after_visit_ends)(struct archive *archive, void *capture);

    carbon_visitor_policy_e (*before_object_visit)(struct archive *archive, path_stack_t path,
                                                   carbon_object_id_t parent_id, carbon_object_id_t value_id,
                                                   u32 object_idx, u32 num_objects, carbon_string_id_t key,
                                                   void *capture);
    void (*after_object_visit)(struct archive *archive, path_stack_t path, carbon_object_id_t id,
                               u32 object_idx, u32 num_objects, void *capture);

    void (*first_prop_type_group)(struct archive *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                                 carbon_basic_type_e type, bool is_array, u32 num_pairs, void *capture);
    void (*next_prop_type_group)(struct archive *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                                 carbon_basic_type_e type, bool is_array, u32 num_pairs, void *capture);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, carbon_i8);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, carbon_i16);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, carbon_i32);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, carbon_i64);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, carbon_u8);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, carbon_u16);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, carbon_u32);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, carbon_u64);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(number, carbon_number_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(string, carbon_string_id_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, carbon_boolean_t);

    void (*visit_null_pairs) (struct archive *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                              u32 num_pairs, void *capture);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, carbon_i8);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, carbon_i16);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, carbon_i32);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, carbon_i64);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, carbon_u8);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, carbon_u16);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, carbon_u32);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, carbon_u64);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, carbon_number_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, carbon_string_id_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, carbon_boolean_t);

    carbon_visitor_policy_e (*visit_enter_null_array_pairs)(struct archive *archive, path_stack_t path,
                                                            carbon_object_id_t id,
                                                            const carbon_string_id_t *keys, u32 num_pairs,
                                                            void *capture);

    void (*visit_enter_null_array_pair)(struct archive *archive, path_stack_t path, carbon_object_id_t id,
                                        carbon_string_id_t key, u32 entry_idx, u32 num_elems, void *capture);

    void (*visit_null_array_pair) (struct archive *archive, path_stack_t path, carbon_object_id_t id,
                                   carbon_string_id_t key, u32 entry_idx, u32 max_entries,
                                   carbon_u32 num_nulls, void *capture);

    void (*visit_leave_null_array_pair)(struct archive *archive, path_stack_t path, carbon_object_id_t id,
                                        u32 pair_idx, u32 num_pairs, void *capture);

    void (*visit_leave_null_array_pairs)(struct archive *archive, path_stack_t path, carbon_object_id_t id,
                                         void *capture);

    carbon_visitor_policy_e (*before_visit_object_array)(struct archive *archive, path_stack_t path,
                                                         carbon_object_id_t parent_id, carbon_string_id_t key,
                                                         void *capture);

    void (*before_visit_object_array_objects)(bool *skip_group_object_ids,
                                              struct archive *archive, path_stack_t path,
                                              carbon_object_id_t parent_id,
                                              carbon_string_id_t key,
                                              const carbon_object_id_t *group_object_ids,
                                              u32 num_group_object_ids, void *capture);

    carbon_visitor_policy_e (*before_visit_object_array_object_property)(struct archive *archive, path_stack_t path,
                                                   carbon_object_id_t parent_id,
                                                   carbon_string_id_t key,
                                                   carbon_string_id_t nested_key,
                                                   carbon_basic_type_e nested_value_type,
                                                   void *capture);

    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int8s, carbon_i8);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int16s, carbon_i16);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int32s, carbon_i32);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(int64s, carbon_i64);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint8s, carbon_u8);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint16s, carbon_u16);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint32s, carbon_u32);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(uint64s, carbon_u64);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(numbers, carbon_number_t);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(strings, carbon_string_id_t);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(booleans, carbon_boolean_t);
    DEFINE_VISIT_OBJECT_ARRAY_OBJECT_PROP(nulls, carbon_u32);

    carbon_visitor_policy_e (*before_object_array_object_property_object)(struct archive *archive, path_stack_t path,
                                                    carbon_object_id_t parent_id,
                                                    carbon_string_id_t key,
                                                    carbon_object_id_t nested_object_id,
                                                    carbon_string_id_t nested_key,
                                                    u32 nested_value_object_id,
                                                    void *capture);

    void (*visit_object_property)(struct archive *archive, path_stack_t path,
                                  carbon_object_id_t parent_id,
                                  carbon_string_id_t key, carbon_basic_type_e type, bool is_array_type, void *capture);


    void (*visit_object_array_prop)(struct archive *archive, path_stack_t path, carbon_object_id_t parent_id, carbon_string_id_t key, carbon_basic_type_e type, void *capture);

    bool (*get_column_entry_count)(struct archive *archive, path_stack_t path, carbon_string_id_t key, carbon_basic_type_e type, u32 count, void *capture);

} carbon_archive_visitor_t;

NG5_EXPORT(bool)
carbon_archive_visit_archive(struct archive *archive, const carbon_archive_visitor_desc_t *desc,
                             carbon_archive_visitor_t *visitor, void *capture);

NG5_EXPORT(bool)
carbon_archive_visitor_print_path(FILE *file, struct archive *archive, const struct vector ofType(carbon_path_entry_t) *path_stack);

NG5_EXPORT(void)
carbon_archive_visitor_path_to_string(char path_buffer[2048], struct archive *archive, const struct vector ofType(carbon_path_entry_t) *path_stack);

NG5_EXPORT(bool)
carbon_archive_visitor_path_compare(const struct vector ofType(carbon_path_entry_t) *path, carbon_string_id_t *group_name, const char *path_str, struct archive *archive);

#endif
