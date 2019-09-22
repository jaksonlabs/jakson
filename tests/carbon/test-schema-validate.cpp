#include <gtest/gtest.h>

#include <fcntl.h>

#include <jakson/jakson.h>

TEST(SchemaTest, ValidateSchema) {
    bool status;
    jak_carbon schemaCarbon;
    jak_carbon fileToVal;
    jak_error err;
    const char *json_in;

    json_in = "{\"type\": \"string\"}";
    jak_carbon_from_json(&schemaCarbon, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

    json_in = "{\"foo\": \"bar\"}";
    jak_carbon_from_json(&fileToVal, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

    status=jak_carbon_schema_validate(&schemaCarbon, &(&fileToVal));
    ASSERT(status==true);
}
    

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
