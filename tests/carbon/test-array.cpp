#include <gtest/gtest.h>
#include <printf.h>

#include <ark-js/shared/json/json.h>
#include <ark-js/shared/utils/convert.h>

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
    bool status = json_parse(&json, &error_desc, &parser, "{\n"
                                                          "\t\"type\": \"FeatureCollection\",\n"
                                                          "\t\"features\": [{\n"
                                                          "\t\t\t\"type\": \"Feature\",\n"
                                                          "\t\t\t\"geometry\": {\n"
                                                          "\t\t\t\t\"type\": \"Point\",\n"
                                                          "\t\t\t\t\"coordinates\": [102.0, 0.5]\n"
                                                          "\t\t\t},\n"
                                                          "\t\t\t\"properties\": {\n"
                                                          "\t\t\t\t\"prop0\": \"value0\"\n"
                                                          "\t\t\t}\n"
                                                          "\t\t},\n"
                                                          "\t\t{\n"
                                                          "\t\t\t\"type\": \"Feature\",\n"
                                                          "\t\t\t\"geometry\": {\n"
                                                          "\t\t\t\t\"type\": \"LineString\",\n"
                                                          "\t\t\t\t\"coordinates\": [\n"
                                                          "\t\t\t\t\t[102.0, 0.0],\n"
                                                          "\t\t\t\t\t[103.0, 1.0],\n"
                                                          "\t\t\t\t\t[104.0, 0.0],\n"
                                                          "\t\t\t\t\t[105.0, 1.0]\n"
                                                          "\t\t\t\t]\n"
                                                          "\t\t\t},\n"
                                                          "\t\t\t\"properties\": {\n"
                                                          "\t\t\t\t\"prop0\": \"value0\",\n"
                                                          "\t\t\t\t\"prop1\": 0.0\n"
                                                          "\t\t\t}\n"
                                                          "\t\t},\n"
                                                          "\t\t{\n"
                                                          "\t\t\t\"type\": \"Feature\",\n"
                                                          "\t\t\t\"geometry\": {\n"
                                                          "\t\t\t\t\"type\": \"Polygon\",\n"
                                                          "\t\t\t\t\"coordinates\": [\n"
                                                          "\t\t\t\t\t[\n"
                                                          "\t\t\t\t\t\t[100.0, 0.0],\n"
                                                          "\t\t\t\t\t\t[101.0, 0.0],\n"
                                                          "\t\t\t\t\t\t[101.0, 1.0],\n"
                                                          "\t\t\t\t\t\t[100.0, 1.0],\n"
                                                          "\t\t\t\t\t\t[100.0, 0.0]\n"
                                                          "\t\t\t\t\t]\n"
                                                          "\t\t\t\t]\n"
                                                          "\n"
                                                          "\t\t\t},\n"
                                                          "\t\t\t\"properties\": {\n"
                                                          "\t\t\t\t\"prop0\": \"value0\",\n"
                                                          "\t\t\t\t\"prop1\": {\n"
                                                          "\t\t\t\t\t\"this\": \"that\"\n"
                                                          "\t\t\t\t}\n"
                                                          "\t\t\t}\n"
                                                          "\t\t}\n"
                                                          "\t]\n"
                                                          "}");

    ASSERT_TRUE(status);
    printf("Parse Status: %s", status ? "true" : "false");

    bool status1 = json_test(&err, &json);
    ASSERT_FALSE(status1);
    printf("\n Json Test: %s", status1 ? "true" : "false");
    json_drop(&json);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}