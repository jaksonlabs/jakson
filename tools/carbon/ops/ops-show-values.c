#include <carbon/carbon-archive-visitor.h>
#include <carbon/carbon-hashset.h>
#include <carbon/carbon-hashtable.h>
#include <carbon/carbon-query.h>
#include "ops-show-values.h"
//
typedef struct
{
    const char *path;
    uint32_t offset;
    uint32_t limit;

    uint32_t current_off;
    uint32_t current_num;

    int32_t between_lower_bound;
    int32_t between_upper_bound;
    const char *contains_string;

    carbon_vec_t ofType(ops_show_values_result_t) *result;

  //  carbon_hashtable_t ofMapping(carbon_string_id_t, uint32_t) counts;
  //  carbon_hashset_t ofType(carbon_string_id_t) keys;
} capture_t;
////
static void
visit_string_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
                              const carbon_string_id_t *keys, const carbon_string_id_t *values, uint32_t num_pairs,
                              void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(id);
    CARBON_UNUSED(keys);
    CARBON_UNUSED(values);
    CARBON_UNUSED(num_pairs);
    CARBON_UNUSED(capture);

    capture_t *params = (capture_t *) capture;

    if (params->current_num > params->limit) {
        return;
    }

    carbon_query_t *query = carbon_archive_query_default(archive);
    for (size_t i = 0; i < num_pairs; i++) {
        char *keystr = carbon_query_fetch_string_by_id(query, keys[i]);
        if (strstr(params->path, keystr) != 0) {
            if (params->current_off >= params->offset) {
                ops_show_values_result_t *r = NULL;
                for (uint32_t k = 0; k < params->result->num_elems; k++)
                {
                    r = CARBON_VECTOR_GET(params->result, k, ops_show_values_result_t);
                    if (r->key == keys[i]) {
                        break;
                    }
                }
                if (!r) {
                    r = VECTOR_NEW_AND_GET(params->result, ops_show_values_result_t);
                    r->key = keys[i];
                    r->type = CARBON_BASIC_TYPE_STRING;
                    carbon_vec_create(&r->values.string_values, NULL, sizeof(carbon_string_id_t), 1000000);
                }

                if (!params->contains_string) {
                    carbon_vec_push(&r->values.string_values, &values[i], 1);
                    params->current_num += 1;
                } else {
                    carbon_query_t *q = carbon_archive_query_default(archive);
                    char *value = carbon_query_fetch_string_by_id(q, values[i]);
                    if (strstr(value, params->contains_string)) {
                        carbon_vec_push(&r->values.string_values, &values[i], 1);
                        params->current_num += 1;
                    }
                    free(value);
                }

            } else {
                params->current_off++;
            }
        }

        free(keystr);
    }

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

    carbon_visitor_policy_e follow = CARBON_VISITOR_POLICY_EXCLUDE;

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);


    capture_t *params = (capture_t *) capture;

    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);


    if (len_user_path >= len_current_path && strncmp(buffer, params->path, len_current_path) == 0) {
        follow = CARBON_VISITOR_POLICY_INCLUDE;
    }



    return follow;
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


    capture_t *params = (capture_t *) capture;

    size_t len_current_path = strlen(buffer);
    size_t len_user_path = strlen(params->path);


    if (len_user_path >= len_current_path && strncmp(buffer, params->path, len_current_path) == 0) {
        follow = CARBON_VISITOR_POLICY_INCLUDE;
    }


    return follow;
}


static void
visit_object_array_object_property_string(carbon_archive_t *archive, path_stack_t path,
                                               carbon_object_id_t parent_id,
                                               carbon_string_id_t key,
                                               carbon_object_id_t nested_object_id,
                                               carbon_string_id_t nested_key,
                                               const carbon_string_id_t *nested_values,
                                               uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);
    CARBON_UNUSED(capture);




    capture_t *params = (capture_t *) capture;

    if (params->current_num > params->limit) {
        return;
    }

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);

    if (strcmp(buffer, params->path) == 0) {
        if (params->current_off >= params->offset) {
            ops_show_values_result_t *r = NULL;
            for (uint32_t k = 0; k < params->result->num_elems; k++)
            {
                r = CARBON_VECTOR_GET(params->result, k, ops_show_values_result_t);
                if (r->key == nested_key) {
                    break;
                }
            }
            if (!r) {
                r = VECTOR_NEW_AND_GET(params->result, ops_show_values_result_t);
                r->key = nested_key;
                r->type = CARBON_BASIC_TYPE_STRING;
                carbon_vec_create(&r->values.string_values, NULL, sizeof(carbon_string_id_t), 1000000);
            }



            if (!params->contains_string) {
                carbon_vec_push(&r->values.string_values, nested_values, num_nested_values);
                params->current_num += num_nested_values;
            } else {
                carbon_query_t *q = carbon_archive_query_default(archive);
                for (uint32_t k = 0; k < num_nested_values; k++) {
                    char *value = carbon_query_fetch_string_by_id(q, nested_values[k]);
                    if (strstr(value, params->contains_string)) {
                        carbon_vec_push(&r->values.string_values, &nested_values[k], 1);
                        params->current_num += 1;
                    }
                    free(value);
                }

            }


        } else {
            params->current_off++;
        }
    }
}



static void
visit_object_array_object_property_int8(carbon_archive_t *archive, path_stack_t path,
                                          carbon_object_id_t parent_id,
                                          carbon_string_id_t key,
                                          carbon_object_id_t nested_object_id,
                                          carbon_string_id_t nested_key,
                                          const carbon_int8_t *nested_values,
                                          uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);
    CARBON_UNUSED(capture);




    capture_t *params = (capture_t *) capture;

    if (params->current_num > params->limit) {
        return;
    }

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);

    if (strcmp(buffer, params->path) == 0) {
        if (params->current_off >= params->offset) {
            ops_show_values_result_t *r = NULL;
            for (uint32_t k = 0; k < params->result->num_elems; k++)
            {
                r = CARBON_VECTOR_GET(params->result, k, ops_show_values_result_t);
                if (r->key == nested_key) {
                    break;
                }
            }
            if (!r) {
                r = VECTOR_NEW_AND_GET(params->result, ops_show_values_result_t);
                r->key = nested_key;
                r->type = CARBON_BASIC_TYPE_INT8;
                carbon_vec_create(&r->values.integer_values, NULL, sizeof(carbon_int64_t), 1000000);
            }

            for (uint32_t k = 0; k < num_nested_values; k++) {
                int64_t val = nested_values[k];
                if (val >= params->between_lower_bound && val <= params->between_upper_bound) {
                    carbon_vec_push(&r->values.integer_values, &val, 1);
                }
            }

            params->current_num += num_nested_values;
        } else {
            params->current_off++;
        }
    }
}


static void
visit_object_array_object_property_int16(carbon_archive_t *archive, path_stack_t path,
                                        carbon_object_id_t parent_id,
                                        carbon_string_id_t key,
                                        carbon_object_id_t nested_object_id,
                                        carbon_string_id_t nested_key,
                                        const carbon_int16_t *nested_values,
                                        uint32_t num_nested_values, void *capture)
{
    CARBON_UNUSED(archive);
    CARBON_UNUSED(path);
    CARBON_UNUSED(parent_id);
    CARBON_UNUSED(key);
    CARBON_UNUSED(nested_object_id);
    CARBON_UNUSED(nested_key);
    CARBON_UNUSED(nested_values);
    CARBON_UNUSED(num_nested_values);
    CARBON_UNUSED(capture);




    capture_t *params = (capture_t *) capture;

    if (params->current_num >= params->limit) {
        return;
    }

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);

    if (strcmp(buffer, params->path) == 0) {
        if (params->current_off >= params->offset) {
            ops_show_values_result_t *r = NULL;
            for (uint32_t k = 0; k < params->result->num_elems; k++)
            {
                r = CARBON_VECTOR_GET(params->result, k, ops_show_values_result_t);
                if (r->key == nested_key) {
                    break;
                }
            }
            if (!r) {
                r = VECTOR_NEW_AND_GET(params->result, ops_show_values_result_t);
                r->key = nested_key;
                r->type = CARBON_BASIC_TYPE_INT16;
                carbon_vec_create(&r->values.integer_values, NULL, sizeof(carbon_int64_t), 1000000);
            }

            for (uint32_t k = 0; k < num_nested_values; k++) {
                int64_t val = nested_values[k];
                if (val >= params->between_lower_bound && val <= params->between_upper_bound) {
                    carbon_vec_push(&r->values.integer_values, &val, 1);
                }

            }

            params->current_num += num_nested_values;
        } else {
            params->current_off++;
        }
    }
}


//
//static bool
//get_column_entry_count(carbon_archive_t *archive, path_stack_t path, carbon_string_id_t key, carbon_basic_type_e type, uint32_t count, void *capture)
//{
//    CARBON_UNUSED(archive);
//    CARBON_UNUSED(path);
//    CARBON_UNUSED(key);
//    CARBON_UNUSED(type);
//    CARBON_UNUSED(count);
//    capture_t *params = (capture_t *) capture;
//    char buffer[2048];
//    memset(buffer, 0, sizeof(buffer));
//    carbon_archive_visitor_path_to_string(buffer, archive, path);
//
//    if (strcmp(buffer, params->path) == 0) {
//        const uint32_t *count_ptr = carbon_hashtable_get_value(&params->counts, &key);
//        uint32_t count_val = 0;
//        if (!count_ptr) {
//            carbon_hashset_insert_or_update(&params->keys, &key, 1);
//        } else {
//            count_val = *count_ptr;
//        }
//        count_val += count;
//        carbon_hashtable_insert_or_update(&params->counts, &key, &count_val, 1);
//
//    }
//    return true;
//
//}

CARBON_EXPORT(bool)
ops_show_values(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_values_result_t) *result, const char *path,
                carbon_archive_t *archive, uint32_t offset, uint32_t limit, int32_t between_lower_bound,
                int32_t between_upper_bound, const char *contains_string)
{
    CARBON_UNUSED(result);
    CARBON_UNUSED(path);
    CARBON_UNUSED(archive);

    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = CARBON_ARCHIVE_ITER_MASK_ANY };

    carbon_vec_create(result, NULL, sizeof(ops_show_values_result_t), 10);

    capture_t capture = {
        .path = path,
        .limit = limit,
        .offset = offset,
        .current_num = 0,
        .current_off = 0,
        .result = result,
        .between_lower_bound = between_lower_bound,
        .between_upper_bound = between_upper_bound,
        .contains_string = contains_string
    };



//    carbon_hashtable_create(&capture.counts, &archive->err, sizeof(carbon_string_id_t), sizeof(uint32_t), 50);
//    carbon_hashset_create(&capture.keys, &archive->err, sizeof(carbon_string_id_t), 50);

  //  visitor.visit_string_pairs = visit_string_pairs;
 //   visitor.visit_string_array_pair = visit_string_array_pair;
 //   visitor.visit_object_array_object_property_strings = visit_object_array_object_property_string;
   // visitor.get_column_entry_count = get_column_entry_count;
    visitor.visit_object_array_object_property_strings = visit_object_array_object_property_string;
    visitor.before_visit_object_array = before_visit_object_array;
    visitor.before_visit_object_array_object_property = before_visit_object_array_object_property;
    visitor.visit_object_array_object_property_int8s = visit_object_array_object_property_int8;
    visitor.visit_object_array_object_property_int16s = visit_object_array_object_property_int16;
    visitor.visit_string_pairs = visit_string_pairs;

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);
    carbon_timestamp_t end = carbon_time_now_wallclock();
    *duration = (end - begin);


//    carbon_vec_t ofType(carbon_string_id_t) *keys = carbon_hashset_keys(&capture.keys);
//
//    for (uint32_t i = 0; i < keys->num_elems; i++) {
//        carbon_string_id_t id = *CARBON_VECTOR_GET(keys, i, carbon_string_id_t);
//        uint32_t count = *(uint32_t *) carbon_hashtable_get_value(&capture.counts, &id);
//        ops_count_values_result_t r = {
//            .key = id,
//            .count = count
//        };
//        carbon_vec_push(result, &r, 1);
//    }
//    carbon_vec_drop(keys);
//
//
//    carbon_hashtable_drop(&capture.counts);
//    carbon_hashset_drop(&capture.keys);


    return true;
}