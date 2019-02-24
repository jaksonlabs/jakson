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

#ifndef CARBON_CARBON_ARCHIVE_VISITOR_H
#define CARBON_CARBON_ARCHIVE_VISITOR_H

#include "carbon-archive-iter.h"

typedef struct carbon_path_entry carbon_path_entry_t;

typedef struct carbon_path_entry
{
    carbon_object_id_t   containing_obj;
    carbon_string_id_t   key;
} carbon_path_entry_t;

typedef struct
{
    int visit_mask;                 /** bitmask of 'CARBON_ARCHIVE_ITER_MASK_XXX' */

} carbon_archive_visitor_desc_t;

typedef enum
{
    CARBON_VISITOR_POLICY_INCLUDE,
    CARBON_VISITOR_POLICY_EXCLUDE,
} carbon_visitor_policy_e;

typedef const carbon_vec_t ofType(carbon_path_entry_t) * path_stack_t;

#define DEFINE_VISIT_BASIC_TYPE_PAIRS(name, built_in_type)                                                             \
void (*visit_##name##_pairs) (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                     \
                              const carbon_string_id_t *keys, const built_in_type *values, uint32_t num_pairs,         \
                              void *capture);

#define DEFINE_VISIT_ARRAY_TYPE_PAIRS(name, built_in_type)                                                             \
carbon_visitor_policy_e (*visit_enter_##name##_array_pairs)(carbon_archive_t *archive, path_stack_t path,              \
                                                        carbon_object_id_t id, const carbon_string_id_t *keys,         \
                                                        uint32_t num_pairs,                                            \
                                                        void *capture);                                                \
                                                                                                                       \
void (*visit_enter_##name##_array_pair)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,           \
                                        carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems,                \
                                        void *capture);                                                                \
                                                                                                                       \
void (*visit_##name##_array_pair) (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,                \
                                   carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,                   \
                                   const built_in_type *array, uint32_t array_length, void *capture);                  \
                                                                                                                       \
void (*visit_leave_##name##_array_pair)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,           \
                                        uint32_t pair_idx, uint32_t num_pairs, void *capture);                         \
                                                                                                                       \
void (*visit_leave_##name##_array_pairs)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,          \
                                         void *capture);

typedef struct
{
    void (*visit_root_object)(carbon_archive_t *archive, carbon_object_id_t id, void *capture);
    void (*before_visit_starts)(carbon_archive_t *archive, void *capture);
    void (*after_visit_ends)(carbon_archive_t *archive, void *capture);

    carbon_visitor_policy_e (*before_object_visit)(carbon_archive_t *archive, path_stack_t path,
                                                   carbon_object_id_t parent_id, carbon_object_id_t value_id,
                                                   uint32_t object_idx, uint32_t num_objects, carbon_string_id_t key,
                                                   void *capture);
    void (*after_object_visit)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                               uint32_t object_idx, uint32_t num_objects, void *capture);

    void (*first_prop_type_group)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                                 carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture);
    void (*next_prop_type_group)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                                 carbon_basic_type_e type, bool is_array, uint32_t num_pairs, void *capture);

    DEFINE_VISIT_BASIC_TYPE_PAIRS(int8, carbon_int8_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int16, carbon_int16_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int32, carbon_int32_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(int64, carbon_int64_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint8, carbon_uint8_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint16, carbon_uint16_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint32, carbon_uint32_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(uint64, carbon_uint64_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(number, carbon_number_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(string, carbon_string_id_t);
    DEFINE_VISIT_BASIC_TYPE_PAIRS(boolean, carbon_boolean_t);

    void (*visit_null_pairs) (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id, const carbon_string_id_t *keys,
                              uint32_t num_pairs, void *capture);

    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int8, carbon_int8_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int16, carbon_int16_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int32, carbon_int32_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(int64, carbon_int64_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint8, carbon_uint8_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint16, carbon_uint16_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint32, carbon_uint32_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(uint64, carbon_uint64_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(number, carbon_number_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(string, carbon_string_id_t);
    DEFINE_VISIT_ARRAY_TYPE_PAIRS(boolean, carbon_boolean_t);

    carbon_visitor_policy_e (*visit_enter_null_array_pairs)(carbon_archive_t *archive, path_stack_t path,
                                                            carbon_object_id_t id,
                                                            const carbon_string_id_t *keys, uint32_t num_pairs,
                                                            void *capture);

    void (*visit_enter_null_array_pair)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                        carbon_string_id_t key, uint32_t entry_idx, uint32_t num_elems, void *capture);

    void (*visit_null_array_pair) (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                   carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
                                   carbon_uint32_t num_nulls, void *capture);

    void (*visit_leave_null_array_pair)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                        uint32_t pair_idx, uint32_t num_pairs, void *capture);

    void (*visit_leave_null_array_pairs)(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                                         void *capture);



} carbon_archive_visitor_t;

CARBON_EXPORT(bool)
carbon_archive_visit_archive(carbon_archive_t *archive, const carbon_archive_visitor_desc_t *desc,
                             carbon_archive_visitor_t *visitor, void *capture);

CARBON_EXPORT(bool)
carbon_archive_visitor_print_path(FILE *file, carbon_archive_t *archive, const carbon_vec_t ofType(carbon_path_entry_t) *path_stack);

#endif
