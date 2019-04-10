//
// Created by Marcus Pinnecke on 28.02.19.
//

#include <inttypes.h>

#include "archive/archive_visitor.h"
#include "archive/query.h"
#include "hash/hash_set.h"
#include "utils/time.h"
#include "ops-show-keys.h"


typedef struct
{
    carbon_hashset_t ofType(ops_show_keys_key_type_pair_t) *result;
    const char *path;
} ops_show_keys_capture_t;

static void visit_string_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              const carbon_string_id_t *keys, const carbon_string_id_t *values, u32 num_pairs,
                              void *capture)
{
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);

}

static carbon_visitor_policy_e
before_visit_object_array(carbon_archive_t *archive, path_stack_t path,
                                                     carbon_object_id_t parent_id, carbon_string_id_t key,
                                                     void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(key);

//        carbon_archive_visitor_print_path(stdout, archive, path);
//
//    carbon_query_t *query = carbon_archive_query_default(archive);
//    char *keystr = carbon_query_fetch_string_by_id(query, key);
//    printf("before_visit_object_array -- KEY %s\n", keystr);
//    free(keystr);


    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
before_visit_object_array_objects(bool *skip_group_object_ids,
                                          carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          const carbon_object_id_t *group_object_ids,
                                          u32 num_group_object_ids, void *capture)
{
    CARBON_UNUSED(skip_group_object_ids);
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(group_object_ids);
    CARBON_UNUSED(num_group_object_ids);
    CARBON_UNUSED(capture);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (carbon_archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = CARBON_BASIC_TYPE_OBJECT,
//            .is_array = true
//        };
//        carbon_hashset_insert_or_update(params->result, &pair, 1);
//    }
}



static carbon_visitor_policy_e
before_object_visit(carbon_archive_t *archive, path_stack_t path,
                                               carbon_object_id_t parent_id, carbon_object_id_t value_id,
                                               u32 object_idx, u32 num_objects, carbon_string_id_t key,
                                               void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(value_id);
    CARBON_UNUSED(object_idx);
    CARBON_UNUSED(num_objects);
    CARBON_UNUSED(key);
    CARBON_UNUSED(capture);

//    carbon_query_t *query = carbon_archive_query_default(archive);
//    char *keystr = carbon_query_fetch_string_by_id(query, key);
//    printf("before_object_visit -- KEY %s\n", keystr);
//    free(keystr);
//    carbon_archive_visitor_print_path(stdout, archive, path);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (carbon_archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = CARBON_BASIC_TYPE_OBJECT
//        };
//        carbon_hashset_insert_or_update(params->result, &pair, 1);
//    }


    return CARBON_VISITOR_POLICY_INCLUDE;
}

static void
visit_object_property(carbon_archive_t *archive, path_stack_t path,
                              carbon_object_id_t parent_id,
                              carbon_string_id_t key, carbon_basic_type_e type, bool is_array_type, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(type);
    CARBON_UNUSED(is_array_type);
    CARBON_UNUSED(capture);

//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (carbon_archive_visitor_path_compare(path, NULL, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = key,
//            .type = type,
//            .is_array = is_array_type
//        };
//        carbon_hashset_insert_or_update(params->result, &pair, 1);
//    }

}



































static void visit_object_array_prop(carbon_archive_t *archive, path_stack_t path, carbon_object_id_t parent_id, carbon_string_id_t key, carbon_basic_type_e type, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(type);
    CARBON_UNUSED(capture);
    CARBON_UNUSED(path);

    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;

    //carbon_archive_visitor_print_path(stderr, archive, path);
    //fprintf(stderr, "---> type: %s\n", carbon_basic_type_to_system_type_str(type));
    //fprintf(stderr, "===> path: %s\n", params->path);

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);


    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);

    if (len_user_path < len_current_path && strncmp(buffer, params->path, len_user_path) == 0) {
        ops_show_keys_key_type_pair_t pair = {
            .key = key,
            .type = type
        };
        carbon_hashset_insert_or_update(params->result, &pair, 1);
    }

}

static carbon_visitor_policy_e
before_visit_object_array_object_property(carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          carbon_string_id_t nested_key,
                                          carbon_basic_type_e nested_value_type,
                                          void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_value_type);
    CARBON_UNUSED(capture);

    carbon_visitor_policy_e follow = CARBON_VISITOR_POLICY_EXCLUDE;

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);


    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    char *nested_keystr = carbon_query_fetch_string_by_id(query, nested_key);

    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);


    if (len_user_path >= len_current_path && strncmp(buffer, params->path, len_current_path) == 0) {
        follow = CARBON_VISITOR_POLICY_INCLUDE;
    }

    //if (strlen(buffer) > strlen(nested_keystr) && strcmp(buffer + (strlen(buffer) - strlen(nested_keystr)), nested_keystr) == 0)
    //{
    //     follow = CARBON_VISITOR_POLICY_INCLUDE;
    //  }

 //   printf("USER PATH: '%s', current path: '%s', follow?\n", params->path, buffer);


   // free(nested_keystr);

    //
//
//    ops_show_keys_capture_t *params = (ops_show_keys_capture_t *) capture;
//    if (carbon_archive_visitor_path_compare(path, params->path, archive)) {
//        ops_show_keys_key_type_pair_t pair = {
//            .key = nested_key,
//            .type = CARBON_BASIC_TYPE_OBJECT
//        };
//        carbon_hashset_insert_or_update(params->result, &pair, 1);
//    }


    return follow;
}



static carbon_visitor_policy_e
before_object_array_object_property_object(carbon_archive_t *archive, path_stack_t path,
                                           carbon_object_id_t parent_id,
                                           carbon_string_id_t key,
                                           carbon_object_id_t nested_object_id,
                                           carbon_string_id_t nested_key,
                                           u32 nested_value_object_id,
                                           void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_value_object_id);
    CARBON_UNUSED(capture);

    return CARBON_VISITOR_POLICY_INCLUDE;
}

CARBON_EXPORT(bool)
ops_show_keys(carbon_timestamp_t *duration, vec_t ofType(ops_show_keys_key_type_pair_t) *result, const char *path, carbon_archive_t *archive)
{
    CARBON_UNUSED(result);
    CARBON_UNUSED(path);
    CARBON_UNUSED(archive);

    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = CARBON_ARCHIVE_ITER_MASK_ANY };
    carbon_hashset_t ofType(ops_show_keys_key_type_pair_t) distinct_key_type_pairs;
    carbon_hashset_create(&distinct_key_type_pairs, &archive->err, sizeof(ops_show_keys_capture_t), 100);
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

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);
    carbon_timestamp_t end = carbon_time_now_wallclock();
    *duration = (end - begin);

    vec_t ofType(ops_show_keys_key_type_pair_t) *pairs = carbon_hashset_keys(&distinct_key_type_pairs);
    carbon_vec_push(result, pairs->base, pairs->num_elems);
    carbon_vec_drop(pairs);

    carbon_hashset_drop(&distinct_key_type_pairs);

    return true;
}