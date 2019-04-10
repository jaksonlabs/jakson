#include "core/carbon/archive_visitor.h"
#include "std/hash_set.h"
#include "std/hash_table.h"
#include "core/carbon/archive_query.h"
#include "ops-count-values.h"

struct capture
{
    const char *path;
    carbon_hashtable_t ofMapping(field_sid_t, u32) counts;
    carbon_hashset_t ofType(field_sid_t) keys;
};
//
static void
visit_string_pairs (struct archive *archive, path_stack_t path, object_id_t id,
                              const field_sid_t *keys, const field_sid_t *values, u32 num_pairs,
                              void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(id);
    NG5_UNUSED(keys);
    NG5_UNUSED(values);
    NG5_UNUSED(num_pairs);
    NG5_UNUSED(capture);


    struct capture *params = (struct capture *) capture;



        struct archive_query *query = carbon_archive_query_default(archive);
        for (u32 i = 0; i < num_pairs; i++) {

            char *keystr = carbon_query_fetch_string_by_id(query, keys[i]);
            if (strlen(params->path) > strlen(keystr) && strcmp(params->path + strlen(params->path) - strlen(keystr), keystr) == 0) {
//                char *valuestr = carbon_query_fetch_string_by_id(query, values[i]);
//                printf("visit_string_pairs -- KEY %s, VALUE %s\n", keystr, valuestr);
//                free(valuestr);

                const u32 *count_ptr = carbon_hashtable_get_value(&params->counts, &keys[i]);
                u32 count_val = 0;
                if (!count_ptr) {
                    carbon_hashset_insert_or_update(&params->keys, &keys[i], 1);
                } else {
                    count_val = *count_ptr;
                }
                count_val++;
                carbon_hashtable_insert_or_update(&params->counts, &keys[i], &count_val, 1);
            }

            free(keystr);

        }

//        const u32 *count_ptr = carbon_hashtable_get_value(&params->counts, &key);
//        u32 count_val = 0;
//        if (!count_ptr) {
//            carbon_hashset_insert_or_update(&params->keys, &key, 1);
//        } else {
//            count_val = *count_ptr;
//        }
//        count_val += count;
//        carbon_hashtable_insert_or_update(&params->counts, &key, &count_val, 1);



}
//
static void
visit_string_array_pair (struct archive *archive, path_stack_t path, object_id_t id,
                                   field_sid_t key, u32 entry_idx, u32 max_entries,
                                   const field_sid_t *array, u32 array_length, void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(id);
    NG5_UNUSED(key);
    NG5_UNUSED(entry_idx);
    NG5_UNUSED(max_entries);
    NG5_UNUSED(array);
    NG5_UNUSED(array_length);
    NG5_UNUSED(capture);

    carbon_archive_visitor_print_path(stdout, archive, path);
}
//
//static void
//visit_object_array_object_property_string(struct archive *archive, path_stack_t path,
//                                               object_id_t parent_id,
//                                               field_sid_t key,
//                                               object_id_t nested_object_id,
//                                               field_sid_t nested_key,
//                                               const field_sid_t *nested_values,
//                                               u32 num_nested_values, void *capture)
//{
//    NG5_UNUSED(archive);
//    NG5_UNUSED(path);
//    NG5_UNUSED(parent_id);
//    NG5_UNUSED(key);
//    NG5_UNUSED(nested_object_id);
//    NG5_UNUSED(nested_key);
//    NG5_UNUSED(nested_values);
//    NG5_UNUSED(num_nested_values);
//    NG5_UNUSED(capture);
//
//
//
//  //
//
//}

static bool
get_column_entry_count(struct archive *archive, path_stack_t path, field_sid_t key, enum field_type type, u32 count, void *capture)
{
    NG5_UNUSED(archive);
    NG5_UNUSED(path);
    NG5_UNUSED(key);
    NG5_UNUSED(type);
    NG5_UNUSED(count);
    struct capture *params = (struct capture *) capture;
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    carbon_archive_visitor_path_to_string(buffer, archive, path);

    if (strcmp(buffer, params->path) == 0) {
        const u32 *count_ptr = carbon_hashtable_get_value(&params->counts, &key);
        u32 count_val = 0;
        if (!count_ptr) {
            carbon_hashset_insert_or_update(&params->keys, &key, 1);
        } else {
            count_val = *count_ptr;
        }
        count_val += count;
        carbon_hashtable_insert_or_update(&params->counts, &key, &count_val, 1);

    }
    return true;

}

NG5_EXPORT(bool)
ops_count_values(carbon_timestamp_t *duration, struct vector ofType(ops_count_values_result_t) *result, const char *path, struct archive *archive)
{
    NG5_UNUSED(result);
    NG5_UNUSED(path);
    NG5_UNUSED(archive);

    carbon_archive_visitor_t visitor = { 0 };
    carbon_archive_visitor_desc_t desc = { .visit_mask = NG5_ARCHIVE_ITER_MASK_ANY };

    struct capture capture = {
        .path = path
    };
    carbon_hashtable_create(&capture.counts, &archive->err, sizeof(field_sid_t), sizeof(u32), 50);
    carbon_hashset_create(&capture.keys, &archive->err, sizeof(field_sid_t), 50);

    visitor.visit_string_pairs = visit_string_pairs;
    visitor.visit_string_array_pair = visit_string_array_pair;
 //   visitor.visit_object_array_object_property_strings = visit_object_array_object_property_string;
    visitor.get_column_entry_count = get_column_entry_count;

    carbon_timestamp_t begin = carbon_time_now_wallclock();
    carbon_archive_visit_archive(archive, &desc, &visitor, &capture);
    carbon_timestamp_t end = carbon_time_now_wallclock();
    *duration = (end - begin);

    struct vector ofType(field_sid_t) *keys = carbon_hashset_keys(&capture.keys);
//    carbon_vec_push(result, pairs->base, pairs->num_elems);
    for (u32 i = 0; i < keys->num_elems; i++) {
        field_sid_t id = *vec_get(keys, i, field_sid_t);
        u32 count = *(u32 *) carbon_hashtable_get_value(&capture.counts, &id);
        ops_count_values_result_t r = {
            .key = id,
            .count = count
        };
        carbon_vec_push(result, &r, 1);
    }
    carbon_vec_drop(keys);


    carbon_hashtable_drop(&capture.counts);
    carbon_hashset_drop(&capture.keys);

    return true;
}