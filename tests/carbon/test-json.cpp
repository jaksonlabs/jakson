#include <gtest/gtest.h>
#include <printf.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <jakson/jakson.h>

TEST(CarbonTest, InvalidObject) {
        carbon carbon;
        err err;
        const char *json_in = "{\"foo\"}";
        bool status = carbon_from_json(&carbon, json_in, CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_FALSE(status);
}

TEST(CarbonTest, EmptyInput) {
        carbon carbon;
        err err;
        const char *json_in = "";
        bool status = carbon_from_json(&carbon, json_in, CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_FALSE(status);
}

TEST(CarbonTest, SingleString) {
        carbon carbon;
        err err;
        const char *json_in = "  foo  ";
        carbon_from_json(&carbon, json_in, CARBON_KEY_NOKEY, NULL, &err);
        UNUSED(carbon);
}

TEST(JsonTest, JsonListTypeForColumnEqualTypes) {
        auto result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_EMPTY);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_FLOAT, JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnOverwriteEmpty) {
        auto result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_EMPTY);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_FIXED_BOOLEAN);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_FLOAT, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnNestedWins)
{
        auto result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);


        json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_FLOAT, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnNullable)
{
        auto result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_FLOAT, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_BOOLEAN);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_FIXED_NULL, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnForceMixed)
{
        auto result = json_fitting_type(JSON_LIST_EMPTY, JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_FLOAT, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_VARIABLE_OR_NESTED, JSON_LIST_EMPTY);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnIncompatibleType)
{
        auto result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_FIXED_BOOLEAN, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnEnlargeType)
{
        auto result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U8);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U16);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U32);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_FIXED_U64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I8);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I8, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I16, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I32, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_I64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I16);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I32);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_FIXED_I64);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U8, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U16, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U32, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_FIXED_U64, JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, ParseNullArray)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[null, null, null]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 3L);
        json_element *e1 = VECTOR_GET(&json.element->value.value.array->elements.elements, 0, json_element);
        json_element *e2 = VECTOR_GET(&json.element->value.value.array->elements.elements, 1, json_element);
        json_element *e3 = VECTOR_GET(&json.element->value.value.array->elements.elements, 2, json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e3->value.value_type, JSON_VALUE_NULL);

        json_drop(&json);
}

TEST(JsonTest, ParseBooleanArray)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[true, false]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 2L);
        json_element *e1 = VECTOR_GET(&json.element->value.value.array->elements.elements, 0, json_element);
        json_element *e2 = VECTOR_GET(&json.element->value.value.array->elements.elements, 1, json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_TRUE);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_FALSE);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromString)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{\"Hello World\": \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 1u);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                json_prop))->key.value, "Hello World") == 0);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                            json_prop))->value.value.value.string->value, "Value") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotes)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{Hello_World: \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 1u);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                            json_prop))->key.value, "Hello_World") == 0);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                            json_prop))->value.value.value.string->value, "Value") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesList)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{Hello: Value1,\nWorld: \"Value2\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 2u);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                            json_prop))->key.value, "Hello") == 0);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 0,
                            json_prop))->value.value.value.string->value, "Value1") == 0);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 1,
                            json_prop))->key.value, "World") == 0);
        ASSERT_TRUE(strcmp((VECTOR_GET(&json.element->value.value.object->value->members, 1,
                            json_prop))->value.value.value.string->value, "Value2") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesTestNull)
{
        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "null");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_NULL);

        json_drop(&json);
}

TEST(JsonTest, ParseRandomJson)
{
        /* the working directory must be 'tests/jakson-tool' to find this file */
        int fd = open("./assets/random.json", O_RDONLY);
        ASSERT_NE(fd, -1);
        int json_in_len = lseek(fd, 0, SEEK_END);
        const char *json_in = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        json_parser parser;
        json json;
        json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, json_in);
        ASSERT_TRUE(status);

        json_drop(&json);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}