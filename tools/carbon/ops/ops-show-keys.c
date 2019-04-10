//
// Created by Marcus Pinnecke on 28.02.19.
//

#include <inttypes.h>

#include "core/carbon/archive_visitor.h"
#include "core/carbon/archive_query.h"
#include "std/hash_set.h"
#include "utils/time.h"
#include "ops-show-keys.h"


typedef struct
{
    struct hashset ofType(ops_show_keys_key_type_pair_t) *result;
    const char *path;
} ops_show_keys_capture_t;

static void visit_string_pairs (struct archive *archive, path_stack_t path, object_id_t id,
                              const field_sid_t *keys, const field_sid_t *values, u32 num_pairs,
                              void *capture)
{
    NG5_UNUSED(id);
    NG5_UNUSED(keys);
    NG5_UNUSED(values);
    NG5_UNUSED(num_pairs);
    NG5_UNUSED(capture);
    NG5_UNUSED(archive);
    NG5_UNUSED(path);

}

static enum visit_policy
before_visit_object_array(struct archive *archive, path_stack_t path,
                                                     object_id_t parent_id, field_sid_t key,
                                                     void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(capture);
    NG5_UNUSED(key);

//        archive_visitor_print_path(stdout, archive, path);
//
//    struct archive_query *query = archive_query_default(archive);
//    char *keystr = query_fetch_string_by_id(query, key);
//    printf("before_visit_object_array -- KEY %s\n", keystr);
//    free(keystr);


    return VISIT_INCLUDE;
}

static void
before_visit_object_array_objects(bool *skip_group_object_ids,
                                          struct archive *archive, path_stack_t path,
                                          object_id_t parent_id,
                                          field_sid_t key,
                                          const object_id_t *group_object_ids,
                                          u32 num_group_object_ids, void *capture)
{
    NG5_UNUSED(skip_group_object_ids);
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(key);
    NG5_UNUSED(group_object_ids);
    NG5_UNUSED(num_group_object_ids);
    NG5_UNUSED(capture);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = FIELD_OBJECT,
//            .is_array = true
//        };
//        hashset_insert_or_update(params->result, &pair, 1);
//    }
}



static enum visit_policy
before_object_visit(struct archive *archive, path_stack_t path,
                                               object_id_t parent_id, object_id_t value_id,
                                               u32 object_idx, u32 num_objects, field_sid_t key,
                                               void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(value_id);
    NG5_UNUSED(object_idx);
    NG5_UNUSED(num_objects);
    NG5_UNUSED(key);
    NG5_UNUSED(capture);

//    struct archive_query *query = archive_query_default(archive);
//    char *keystr = query_fetch_string_by_id(query, key);
//    printf("before_object_visit -- KEY %s\n", keystr);
//    free(keystr);
//    archive_visitor_print_path(stdout, archive, path);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = FIELD_OBJECT
//        };
//        hashset_insert_or_update(params->result, &pair, 1);
//    }


    return VISIT_INCLUDE;
}

static void
visit_object_property(struct archive *archive, path_stack_t path,
                              object_id_t parent_id,
                              field_sid_t key, enum field_type type, bool is_array_type, void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(key);
    NG5_UNUSED(type);
    NG5_UNUSED(is_array_type);
    NG5_UNUSED(capture);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = type,
//            .is_array = is_array_type
//        };
//        hashset_insert_or_update(params->result, &pair, 1);
//    }

}



































static void visit_object_array_prop(struct archive *archive, path_stack_t path, object_id_t parent_id, field_sid_t key, enum field_type type, void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(type);
    NG5_UNUSED(capture);
    NG5_UNUSED(path);

    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;

    //archive_visitor_print_path(stderr, archive, path);
    //fprintf(stderr, "---> type: %s\n", basic_type_to_system_type_str(type));
    //fprintf(stderr, "===> path: %s\n", params->path);

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    archive_visitor_path_to_string(buffer, archive, path);


    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);

    if (len_user_path < len_current_path && strncmp(buffer, params->path, len_user_path) == 0) {
        ops_show_keys_key_type_pair_t pair = {
            .key = key,
            .type = type
        };
        hashset_insert_or_update(params->result, &pair, 1);
    }

}

static enum visit_policy
before_visit_object_array_object_property(struct archive *archive, path_stack_t path,
                                          object_id_t parent_id,
                                          field_sid_t key,
                                          field_sid_t nested_key,
                                          enum field_type nested_value_type,
                                          void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(key);
    NG5_UNUSED(nested_key);
    NG5_UNUSED(nested_value_type);
    NG5_UNUSED(capture);

    enum visit_policy follow = VISIT_EXCLUDE;

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    archive_visitor_path_to_string(buffer, archive, path);


    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    char *nested_keystr = query_fetch_string_by_id(query, nested_key);

    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);


    if (len_user_path >= len_current_path && strncmp(buffer, params->path, len_current_path) == 0) {
        follow = VISIT_INCLUDE;
    }

    //if (strlen(buffer) > strlen(nested_keystr) && strcmp(buffer + (strlen(buffer) - strlen(nested_keystr)), nested_keystr) == 0)
    //{
    //     follow = VISIT_INCLUDE;
    //  }

 //   printf("USER PATH: '%s', current path: '%s', follow?\n", params->path, buffer);


   // free(nested_keystr);

    //
//
//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (archive_visitor_path_compare(path, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = nested_key,
//            .type = FIELD_OBJECT
//        };
//        hashset_insert_or_update(params->result, &pair, 1);
//    }


    return follow;
}



static enum visit_policy
before_object_array_object_property_object(struct archive *archive, path_stack_t path,
                                           object_id_t parent_id,
                                           field_sid_t key,
                                           object_id_t nested_object_id,
                                           field_sid_t nested_key,
                                           u32 nested_value_object_id,
                                           void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(parent_id);
    NG5_UNUSED(key);
    NG5_UNUSED(nested_key);
    NG5_UNUSED(nested_object_id);
    NG5_UNUSED(nested_value_object_id);
    NG5_UNUSED(capture);

    return VISIT_INCLUDE;
}

NG5_EXPORT(bool)
ops_show_keys(timestamp_t *duration, struct vector ofType(ops_show_keys_key_type_pair_t) *result, const char *path, struct archive *archive)
{
    NG5_UNUSED(result);
    NG5_UNUSED(path);
    NG5_UNUSED(archive);

    struct archive_visitor visitor = { 0 };
    struct archive_visitor_desc desc = { .visit_mask = NG5_ARCHIVE_ITER_MASK_ANY };
    struct hashset ofType(ops_show_keys_key_type_pair_t) distinct_key_type_pairs;
    hashset_create(&distinct_key_type_pairs, &archive->err, sizeof(ops_show_keys_capture_t), 100);
    ops_show_keys_capture_t capture = {
        .path = path,
        .result = &distinct_key_type_pairs
    };

    visitor.visit_string_pairs = visit_string_pairs;
    visitor.before_visit_object_array = before_visit_object_array;
    visitor.before_visit_object_array_object_property = before_visit_object_array_object_property;
    visitor.before_object_visit = before_object_visit;
    visitor.before_object_array_object_property_object = before_object_array_object_property_object;
    visitor.visit_object_property = visit_object_property;
    visitor.before_visit_object_array_objects = before_visit_object_array_objects;

    visitor.visit_object_array_prop = visit_object_array_prop;

    timestamp_t begin = time_now_wallclock();
    archive_visit_archive(archive, &desc, &visitor, &capture);
    timestamp_t end = time_now_wallclock();
    *duration = (end - begin);

    struct vector ofType(ops_show_keys_key_type_pair_t) *pairs = hashset_keys(&distinct_key_type_pairs);
    vec_push(result, pairs->base, pairs->num_elems);
    vec_drop(pairs);

    hashset_drop(&distinct_key_type_pairs);

    return true;
}