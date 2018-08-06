#include <stdio.h>
#include <check.h>

#include <ng5/strings.h>
#include <stdx/string_hashtable.h>
#include <stdlib.h>
#include <stdx/string_id_maps/simple_bsearch.h>
#include <stdx/string_id_maps/simple_scan1.h>
#include <stdx/string_id_maps/simple_scan2.h>

void test_string_hashtable_generic_createdrop(struct string_hashtable *map, int status) {

    ck_assert_msg(status == STATUS_OK, "construction fails");
    status = string_hashtable_drop(map);
    ck_assert_msg(status == STATUS_OK, "destruction fails");
}

START_TEST (test_string_hashtable_generic_createdrop_besearch)
{
    struct string_hashtable map;
    int status = string_hashtable_create_besearch(&map, NULL, 100, 5, 1.7f);
    test_string_hashtable_generic_createdrop(&map, status);
}
END_TEST

START_TEST (test_string_hashtable_generic_createdrop_scan1)
    {
        struct string_hashtable map;
        int status = string_hashtable_create_scan1(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_createdrop(&map, status);
    }
END_TEST

START_TEST (test_string_hashtable_generic_createdrop_scan2)
    {
        struct string_hashtable map;
        int status = string_hashtable_create_scan2(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_createdrop(&map, status);
    }
END_TEST

void test_string_hashtable_generic_putget(struct string_hashtable *map) {
    const size_t NUM_PAIRS = 1000;
    char **keys = malloc(NUM_PAIRS * sizeof(char*));
    uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
    for (size_t i = 0; i < NUM_PAIRS; i++) {
        char buffer[129];
        sprintf(buffer, "string-%zu", i);
        keys[i] = strdup(buffer);
        values[i] = i;
    }
    string_hashtable_put_test(map, keys, values, NUM_PAIRS);
    uint64_t *out_values;
    bool     *out_mask;
    size_t    num_not_found;
    string_id_map_get_test(&out_values, &out_mask, &num_not_found, map, keys, NUM_PAIRS);

    for (size_t i = 0; i < NUM_PAIRS; i++) {
        ck_assert_msg(out_values[i] == values[i], "mapping broken");
    }

    string_hashtable_drop(map);
}

START_TEST (test_string_hashtable_generic_putget_bsearch)
    {
        struct string_hashtable map;
        string_hashtable_create_besearch(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_putget(&map);
    }
END_TEST

START_TEST (test_string_hashtable_generic_putget_scan1)
    {
        struct string_hashtable map;
        string_hashtable_create_scan1(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_putget(&map);
    }
END_TEST

START_TEST (test_string_hashtable_generic_putget_scan2)
    {
        struct string_hashtable map;
        string_hashtable_create_scan2(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_putget(&map);
    }
END_TEST

void test_string_hashtable_generic_remove(struct string_hashtable *map)
{
    const size_t NUM_PAIRS = 1000;
    char **keys = malloc(NUM_PAIRS * sizeof(char*));
    char **remove_keys = malloc(NUM_PAIRS * sizeof(char*));
    size_t num_remove_keys = 0;
    uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
    for (size_t i = 0; i < NUM_PAIRS; i++) {
        char buffer[129];
        sprintf(buffer, "string-%zu", i);
        keys[i] = strdup(buffer);
        values[i] = i;
        if (i % 3 == 0) {
            remove_keys[num_remove_keys] = keys[i];
            num_remove_keys++;
        }
    }
    string_hashtable_put_test(map, keys, values, NUM_PAIRS);
    string_id_map_remove(map, remove_keys, num_remove_keys);

    uint64_t *out_values;
    bool     *out_mask;
    size_t    num_not_found;
    string_id_map_get_test(&out_values, &out_mask, &num_not_found, map, keys, NUM_PAIRS);

    ck_assert_msg(num_not_found == num_remove_keys, "illegal number of not-found values");

    for (size_t i = 0; i < NUM_PAIRS; i++) {
        if (out_mask[i]) {
            ck_assert_msg(out_values[i] == values[i], "mapping broken");
            ck_assert_msg(i % 3 != 0, "found elements that should be removed");
        } else {
            ck_assert_msg(i % 3 == 0, "removed wrong elements");
        }

    }

    string_hashtable_drop(map);
}

START_TEST (test_string_hashtable_generic_remove_besearch)
    {
        struct string_hashtable map;
        string_hashtable_create_besearch(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_remove(&map);
    }
END_TEST

START_TEST (test_string_hashtable_generic_remove_scan1)
    {
        struct string_hashtable map;
        string_hashtable_create_scan1(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_remove(&map);
    }
END_TEST

START_TEST (test_string_hashtable_generic_remove_scan2)
    {
        struct string_hashtable map;
        string_hashtable_create_scan2(&map, NULL, 100, 5, 1.7f);
        test_string_hashtable_generic_remove(&map);
    }
END_TEST

Suite* str_suite (void)
{
    TCase *tcase;

    Suite *suite = suite_create("String HashTable");
        tcase = tcase_create("BSearch");
            tcase_add_test(tcase, test_string_hashtable_generic_createdrop_besearch);
            tcase_add_test(tcase, test_string_hashtable_generic_putget_bsearch);
            tcase_add_test(tcase, test_string_hashtable_generic_remove_besearch);
        suite_add_tcase(suite, tcase);

        tcase = tcase_create("Scan1");
            tcase_add_test(tcase, test_string_hashtable_generic_createdrop_scan1);
            tcase_add_test(tcase, test_string_hashtable_generic_putget_scan1);
            tcase_add_test(tcase, test_string_hashtable_generic_remove_scan1);
        suite_add_tcase(suite, tcase);

        tcase = tcase_create("Scan2");
            tcase_add_test(tcase, test_string_hashtable_generic_createdrop_scan2);
            tcase_add_test(tcase, test_string_hashtable_generic_putget_scan2);
            tcase_add_test(tcase, test_string_hashtable_generic_remove_scan2);
        suite_add_tcase(suite, tcase);
    return suite;
}

int main (int argc, char *argv[])
{
    unused(argc);
    unused(argv);

    int number_failed;
    Suite *suite = str_suite();
    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}