#include <check.h>

#include <stdx/test_string_dic.h>
#include <stdx/test_string_hashtable.h>

int main (int argc, char *argv[])
{
    unused(argc);
    unused(argv);

    int number_failed = 0;

    number_failed += run_test_string_hashtable();
    number_failed += run_test_string_dic();

    return number_failed;
}