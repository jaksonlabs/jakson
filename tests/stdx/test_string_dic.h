#include <stdio.h>
#include <check.h>

#include <ng5_string_dic.h>



START_TEST (test_string_dic_sync_createdrop)
    {

    }
END_TEST



static int run_test_string_dic (void)
{
    TCase *tcase;

    Suite *suite = suite_create("String Dictionary");


    tcase = tcase_create("Sync/Thread-Local");
    tcase_add_test(tcase, test_string_dic_sync_createdrop);
    suite_add_tcase(suite, tcase);


    int number_failed;

    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}
