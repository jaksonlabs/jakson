#include <stdio.h>
#include <stdx/string_hashtable.h>
#include <stdlib.h>
#include <stdx/algorithm.h>
#include <stdx/time.h>
#include <stdx/string_id_maps/simple_bsearch.h>

bool comp_int_less_eq(const void *lhs, const void *rhs)
{
    return (*(int *) lhs <= *(int *) rhs);
}

bool comp_int_less(const void *lhs, const void *rhs)
{
    return (*(int *) lhs < *(int *) rhs);
}

bool comp_int_eq(const void *lhs, const void *rhs)
{
    return (*(int *) lhs == *(int *) rhs);
}

int main()
{
    printf("Hello, World!\n");
//
//    const size_t NUM_PAIRS = 1000;
//    struct string_hashtable map;
//    string_hashtable_create_besearch(&map, NULL, 10, 10, 1.7f);
//    char **keys = malloc(NUM_PAIRS * sizeof(char*));
//    char **remove_keys = malloc(NUM_PAIRS * sizeof(char*));
//    size_t num_remove_keys = 0;
//    uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
//    for (size_t i = 0; i < NUM_PAIRS; i++) {
//        char buffer[129];
//        sprintf(buffer, "string-%zu", i);
//        keys[i] = strdup(buffer);
//        values[i] = i;
//        if (i % 3 == 0) {
//            remove_keys[num_remove_keys] = keys[i];
//            num_remove_keys++;
//        }
//    }
//    string_hashtable_put_test(&map, keys, values, NUM_PAIRS);
//    string_id_map_remove(&map, remove_keys, num_remove_keys);
//
//    char **new_keys = malloc(num_remove_keys * sizeof(char*));
//    uint64_t *new_values = malloc(num_remove_keys * sizeof(uint64_t));
//    for (size_t i = 0; i < num_remove_keys; i++) {
//        char buffer[129];
//        sprintf(buffer, "new-string-%zu", i);
//        new_keys[i] = strdup(buffer);
//        new_values[i] = i + NUM_PAIRS;
//    }
//    string_hashtable_put_test(&map, new_keys, new_values, num_remove_keys);
//
//    uint64_t *out_values;
//    bool     *out_mask;
//    size_t    num_not_found_old;
//    string_id_map_get_test(&out_values, &out_mask, &num_not_found_old, &map, keys, NUM_PAIRS);
//    assert(num_not_found_old == num_remove_keys);
//
//    size_t    num_not_found_new;
//    string_id_map_get_test(&out_values, &out_mask, &num_not_found_new, &map, new_keys, num_remove_keys);
// //  assert(num_not_found == 0);
//
//
//    for (size_t i = 0; i < num_remove_keys; i++) {
//        printf("          index: %zu, key: %s, value: %llu, status: FOUND\n", i, keys[i], out_values[i]);
//    }
//
//    string_hashtable_drop(&map);



    timestamp_t begin;

    for (size_t NUM_PAIRS = 565000; NUM_PAIRS < 5650000; NUM_PAIRS += 5650000/10) {

        float duration_create, duration_insert, duration_find;

        struct string_hashtable map;

        begin = time_current_time_ms();
        string_hashtable_create_besearch(&map, NULL, 10000, 100, 1.7f);
        //string_id_map_create_scan_single_threaded(&map, NULL, 10000, 100, 1.7f);
        duration_create = time_current_time_ms() - begin;

        char **keys = malloc(NUM_PAIRS * sizeof(char*));
        uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
        for (size_t i = 0; i < NUM_PAIRS; i++) {
            char buffer[129];
            sprintf(buffer, "string-%zu", i);
            keys[i] = strdup(buffer);
            values[i] = i;
        }
        begin = time_current_time_ms();
        string_hashtable_put_test(&map, keys, values, NUM_PAIRS);
        duration_insert = time_current_time_ms() - begin;
        duration_insert /= (float) NUM_PAIRS;

        uint64_t *out_values;
        bool     *out_mask;
        size_t    num_not_found;

        begin = time_current_time_ms();
        string_id_map_get_test(&out_values, &out_mask, &num_not_found, &map, keys, NUM_PAIRS);
        duration_find = time_current_time_ms() - begin;
        duration_find /= (float) NUM_PAIRS;

        printf("%zu;%f;%f;%f;%f\n", NUM_PAIRS, duration_create, 1/(duration_insert/1000.0), 1/(duration_find/1000.0), (duration_insert * NUM_PAIRS / 1000.0));

        string_id_map_free(out_values, &map);
        string_id_map_free(out_mask, &map);
        string_hashtable_drop(&map);
    }

/*
    const size_t NUM_PAIRS = 1000;
    struct string_hashtable map;
    string_hashtable_create_besearch(&map, NULL, 10, 10, 1.7f);
    char **keys = malloc(NUM_PAIRS * sizeof(char*));
    uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
    for (size_t i = 0; i < NUM_PAIRS; i++) {
        char buffer[129];
        sprintf(buffer, "string-%zu", i);
        keys[i] = strdup(buffer);
        values[i] = i;
    }
    string_hashtable_put_test(&map, keys, values, NUM_PAIRS);
    uint64_t *out_values;
    bool     *out_mask;
    size_t    num_not_found;
    string_id_map_get_test(&out_values, &out_mask, &num_not_found, &map, keys, NUM_PAIRS);

    for (size_t i = 0; i < NUM_PAIRS; i++) {
        printf("string '%s' has key %llu\n", keys[i], out_values[i]);
    }

    string_hashtable_drop(&map);*/
/*
    const size_t num_test = 100000000;
    int *test = malloc(num_test * sizeof(int));
    size_t *indices = malloc(num_test *sizeof(size_t));
    for (size_t i = 0; i < num_test; i++) {
        test[i] = rand();
        indices[i] = i;
    }

    const int NEEDLE = 42424242;
    test[num_test/2] = NEEDLE;

    struct allocator alloc;
    allocator_default(&alloc);
    qsort_indicies(indices, test, sizeof(int), comp_int_less_eq, num_test, &alloc);

   // for (size_t i = 0; i < num_test; i++) {
   //     fprintf(stdout, "%zu -> %d\n", i, test[indices[i]]);
  //  }

    int pos = binary_search_indicies(indices, test, sizeof(int), num_test, &NEEDLE, comp_int_eq, comp_int_less);
    int posL = binary_search_indicies(indices, test, sizeof(int), num_test, &test[indices[0]], comp_int_eq, comp_int_less);
    int posH = binary_search_indicies(indices, test, sizeof(int), num_test, &test[indices[num_test - 1]], comp_int_eq, comp_int_less);
    fprintf(stdout, "found NEEDLE %d at pos %d\n", NEEDLE, pos);
    fprintf(stdout, "found LOW    %d at pos %d\n", test[indices[0]], posL);
    fprintf(stdout, "found HIGH   %d at pos %d\n", test[indices[num_test - 1]], posH);*/

    return 0;
}