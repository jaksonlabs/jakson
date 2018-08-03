#include <stdio.h>
#include <check.h>

#include <ng5/strings.h>
#include <stdx/string_id_map.h>
#include <stdlib.h>
#include <stdx/string_id_maps/simple_bsearch.h>

START_TEST (test_string_id_map_simple_ctor)
{
    struct string_id_map map;
    int status = string_id_map_create_simple(&map, NULL, 100, 5, 1.7f);
    ck_assert_msg(status == STATUS_OK, "construction fails");
    status = string_id_map_drop(&map);
    ck_assert_msg(status == STATUS_OK, "destruction fails");
}
END_TEST

START_TEST (test_string_id_map_simple_putget)
    {
        const size_t NUM_PAIRS = 1000;
        struct string_id_map map;
        string_id_map_create_simple(&map, NULL, 10, 10, 1.7f);
        char **keys = malloc(NUM_PAIRS * sizeof(char*));
        uint64_t *values = malloc(NUM_PAIRS * sizeof(uint64_t));
        for (size_t i = 0; i < NUM_PAIRS; i++) {
            char buffer[129];
            sprintf(buffer, "string-%zu", i);
            keys[i] = strdup(buffer);
            values[i] = i;
        }
        string_id_map_put(&map, keys, values, NUM_PAIRS);
        uint64_t *out_values;
        bool     *out_mask;
        size_t    num_not_found;
        string_id_map_get(&out_values, &out_mask, &num_not_found, &map, keys, NUM_PAIRS);

        for (size_t i = 0; i < NUM_PAIRS; i++) {
            ck_assert_msg(out_values[i] == values[i], "mapping broken");
        }

        string_id_map_drop(&map);

    }
END_TEST


START_TEST (test_string_id_map_simple_remove)
    {
        const size_t NUM_PAIRS = 1000;
        struct string_id_map map;
        string_id_map_create_simple(&map, NULL, 10, 10, 1.7f);
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
        string_id_map_put(&map, keys, values, NUM_PAIRS);
        string_id_map_remove(&map, remove_keys, num_remove_keys);

        uint64_t *out_values;
        bool     *out_mask;
        size_t    num_not_found;
        string_id_map_get(&out_values, &out_mask, &num_not_found, &map, keys, NUM_PAIRS);

        ck_assert_msg(num_not_found == num_remove_keys, "illegal number of not-found values");

        for (size_t i = 0; i < NUM_PAIRS; i++) {
            if (out_mask[i]) {
                ck_assert_msg(out_values[i] == values[i], "mapping broken");
                ck_assert_msg(i % 3 != 0, "found elements that should be removed");
            } else {
                ck_assert_msg(i % 3 == 0, "removed wrong elements");
            }

        }

        string_id_map_drop(&map);

    }
END_TEST

Suite* str_suite (void)
{
    Suite *suite = suite_create("string_id_map");
    TCase *tcase = tcase_create("StringIdMap Simple");
    tcase_add_test(tcase, test_string_id_map_simple_ctor);
    tcase_add_test(tcase, test_string_id_map_simple_putget);
    tcase_add_test(tcase, test_string_id_map_simple_remove);
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