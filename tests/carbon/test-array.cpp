#include <gtest/gtest.h>
#include <printf.h>

#include <ark-js/shared/json/json.h>
#include <ark-js/shared/utils/convert.h>
#include <ark-js/carbon/archive/archive.h>

TEST (TestArray, NormalJSON)
{
    struct json_parser parser;
    struct json json;
    struct err err;
    struct json_err error_desc;
    json_parser_create(&parser);
    bool status = json_parse(&json, &error_desc, &parser, "{\n"
                                                          "\t\"type\": \"Feature\",\n"
                                                          "\t\"geometry\": {\n"
                                                          "\t\t\"type\": \"Point\",\n"
                                                          "\t\t\"coordinates\": [125.6, 10.1]\n"
                                                          "\t},\n"
                                                          "\t\"properties\": {\n"
                                                          "\t\t\"name\": \"Dinagat Islands\"\n"
                                                          "\t}\n"
                                                          "}");

    ASSERT_TRUE(status);
    printf("Parse Status: %s", status ? "true" : "false");
    bool status1 = json_test(&err, &json);
    ASSERT_TRUE(status1);
    printf("\n Json Test: %s", status1 ? "true" : "false");
    json_drop(&json);
}

TEST (TestArray, GeoJSON)
{
    struct json_parser parser;
    struct json json;
    struct err err;
    struct json_err error_desc;
    json_parser_create(&parser);
    bool status = json_parse(&json, &error_desc, &parser, "{\"x\": [[1,2,3],[4,5,6]]}");

    ASSERT_TRUE(status);
    printf("Parse Status: %s", status ? "true" : "false");

    bool status1 = json_test(&err, &json);
    ASSERT_FALSE(status1);
    printf("\n Json Test: %s", status1 ? "true" : "false");
    json_drop(&json);
}

TEST (TestArray, HelpWorkingCarbonArchiveTranslation)
{
        /* use break points to understand how it works successfully */

        struct archive   archive;
        struct err       err;

        const char        *json_string = "{ \"test\": 123 }";
        const char        *archive_file = "tmp-test-archive.carbon";
        bool               read_optimized = false;

        bool status = archive_from_json(&archive, archive_file, &err, json_string,
                                        PACK_NONE, SYNC, 0, read_optimized, true, NULL);
        ASSERT_TRUE(status);

        archive_close(&archive);
}

TEST (TestArray, HelpWorkingCarbonArchiveTranslationWithArray)
{
        /* use break points to understand how it works successfully */

        struct archive   archive;
        struct err       err;

        const char        *json_string = "{ \"test\": [1,2,3] }";
        const char        *archive_file = "tmp-test-archive.carbon";
        bool               read_optimized = false;

        bool status = archive_from_json(&archive, archive_file, &err, json_string,
                                        PACK_NONE, SYNC, 0, read_optimized, true, NULL);
        ASSERT_TRUE(status);

        archive_close(&archive);
}

TEST (TestArray, HelpNOTWorkingCarbonArchiveTranslationWithArray)
{
        /* use break points to understand how it works successfully */
        /* Number 3: is for the purpose of adding "arrays of arrays" disbale the "array of array condition" in
         * json_test, and let the convertion happen and crash */

        /* Your final goal until next week:
         *      (1) understand principle workflow of carbon archives from Json by the 2 tests from above + ark-carbon
         *      (2) disable "array of array" test condition
         *      (3) understand where (!) it crashes
         *      (4) speculate why (!) it crashes
         *
         * Next meeting: give you an understanging on "why" in depth, and a first hint towards a solution
         */

        struct archive   archive;
        struct err       err;

        const char        *json_string = "{ \"test\": [[1,2,3]] }";
        const char        *archive_file = "tmp-test-archive.carbon";
        bool               read_optimized = false;

        bool status = archive_from_json(&archive, archive_file, &err, json_string,
                                        PACK_NONE, SYNC, 0, read_optimized, true, NULL);
        ASSERT_TRUE(status);

        archive_close(&archive);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}