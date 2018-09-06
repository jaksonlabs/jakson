#include <stdio.h>
#include <check.h>
#include <stdx/ng5_slice_list.h>

START_TEST (test_slice_list_complex)
    {
        ng5_slice_list_t list;
        ng5_slice_list_create(&list, NULL, sizeof(short), 100);
    }
END_TEST



static int run_test_slice_list (void)
{
    TCase *tcase;

    Suite *suite = suite_create("Slice List");


    tcase = tcase_create("Complexl");
    tcase_add_test(tcase, test_slice_list_complex);
    suite_add_tcase(suite, tcase);


    int number_failed;

    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}
