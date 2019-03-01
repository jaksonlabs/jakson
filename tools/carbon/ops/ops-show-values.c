#include <carbon/carbon-archive-visitor.h>
#include <carbon/carbon-hashset.h>
#include <carbon/carbon-hashtable.h>
#include <carbon/carbon-query.h>
#include "ops-show-values.h"
//
typedef struct
{
    const char *path;
    uint32_t limit;
  //  carbon_hashtable_t ofMapping(carbon_string_id_t, uint32_t) counts;
  //  carbon_hashset_t ofType(carbon_string_id_t) keys;
} capture_t;
////
//static void
//visit_string_pairs (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
//                              const carbon_string_id_t *keys, const carbon_string_id_t *values, uint32_t num_pairs,
//                              void *capture)
//{
//    CARBON_UNUSED(archive);
//    CARBON_UNUSED(path);
//    CARBON_UNUSED(id);
//    CARBON_UNUSED(keys);
//    CARBON_UNUSED(values);
//    CARBON_UNUSED(num_pairs);
//    CARBON_UNUSED(capture);
//
//
//    capture_t *params = (capture_t *) capture;
//
//
//
//        carbon_query_t *query = carbon_archive_query_default(archive);
//        for (uint32_t i = 0; i < num_pairs; i++) {
//
//            char *keystr = carbon_query_fetch_string_by_id(query, keys[i]);
//            if (strlen(params->path) > strlen(keystr) && strcmp(params->path + strlen(params->path) - strlen(keystr), keystr) == 0) {
////                char *valuestr = carbon_query_fetch_string_by_id(query, values[i]);
////                printf("visit_string_pairs -- KEY %s, VALUE %s\n", keystr, valuestr);
////                free(valuestr);
//
//                const uint32_t *count_ptr = carbon_hashtable_get_value(&params->counts, &keys[i]);
//                uint32_t count_val = 0;
//                if (!count_ptr) {
//                    carbon_hashset_insert_or_update(&params->keys, &keys[i], 1);
//                } else {
//                    count_val = *count_ptr;
//                }
//                count_val++;
//                carbon_hashtable_insert_or_update(&params->counts, &keys[i], &count_val, 1);
//            }
//
//            free(keystr);
//
//        }
//
////        const uint32_t *count_ptr = carbon_hashtable_get_value(&params->counts, &key);
////        uint32_t count_val = 0;
////        if (!count_ptr) {
////            carbon_hashset_insert_or_update(&params->keys, &key, 1);
////        } else {
////            count_val = *count_ptr;
////        }
////        count_val += count;
////        carbon_hashtable_insert_or_update(&params->counts, &key, &count_val, 1);
//
//
//
//}
////
//static void
//visit_string_array_pair (carbon_archive_t *archive, path_stack_t path, carbon_object_id_t id,
//                                   carbon_string_id_t key, uint32_t entry_idx, uint32_t max_entries,
//                                   const carbon_string_id_t *array, uint32_t array_length, void *capture)
//{
//    CARBON_UNUSED(archive);
//    CARBON_UNUSED(path);
//    CARBON_UNUSED(id);
//    CARBON_UNUSED(key);
//    CARBON_UNUSED(entry_idx);
//    CARBON_UNUSED(max_entries);
//    CARBON_UNUSED(array);
//    CARBON_UNUSED(array_length);
//    CARBON_UNUSED(capture);
//
//    carbon_archive_visitor_print_path(stdout, archive, path);
//}
////
////static void
////visit_object_array_object_property_string(carbon_archive_t *archive, path_stack_t path,
////                                               carbon_object_id_t parent_id,
////                                               carbon_string_id_t key,
////                                               carbon_object_id_t nested_object_id,
////                                               carbon_string_id_t nested_key,
////                                               const carbon_string_id_t *nested_values,
////                                               uint32_t num_nested_values, void *capture)
////{
////    CARBON_UNUSED(archive);
////    CARBON_UNUSED(path);
////    CARBON_UNUSED(parent_id);
////    CARBON_UNUSED(key);
////    CARBON_UNUSED(nested_object_id);
////    CARBON_UNUSED(nested_key);
////    CARBON_UNUSED(nested_values);
////    CARBON_UNUSED(num_nested_values);
////    CARBON_UNUSED(capture);
////
////
////
////  //
////
////}
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
ops_show_values(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_values_result_t) *result, const char *path, carbon_archive_t *archive, uint32_t limit)
{
    CARBON_UNUSED(result);
    CARBON_UNUSED(path);
    CARBON_UNUSED(archive);

    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = CARBON_ARCHIVE_ITER_MASK_ANY };

    capture_t capture = {
        .path = path,
        .limit = limit
    };
//    carbon_hashtable_create(&capture.counts, &archive->err, sizeof(carbon_string_id_t), sizeof(uint32_t), 50);
//    carbon_hashset_create(&capture.keys, &archive->err, sizeof(carbon_string_id_t), 50);

  //  visitor.visit_string_pairs = visit_string_pairs;
 //   visitor.visit_string_array_pair = visit_string_array_pair;
 //   visitor.visit_object_array_object_property_strings = visit_object_array_object_property_string;
   // visitor.get_column_entry_count = get_column_entry_count;

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