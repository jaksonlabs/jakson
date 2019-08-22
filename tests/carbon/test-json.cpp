#include <gtest/gtest.h>
#include <printf.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <jak_json.h>

TEST(JsonTest, JsonListTypeForColumnEqualTypes) {
        auto result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_EMPTY);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_FLOAT, JSON_LIST_TYPE_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnOverwriteEmpty) {
        auto result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_EMPTY);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_BOOLEAN);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_FLOAT, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnNestedWins)
{
        auto result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);


        json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_FLOAT, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnNullable)
{
        auto result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_FLOAT, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_BOOLEAN);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_FLOAT);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_NULL);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_NULL);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_NULL, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnForceMixed)
{
        auto result = json_fitting_type(JSON_LIST_TYPE_EMPTY, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_FLOAT, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_VARIABLE_OR_NESTED, JSON_LIST_TYPE_EMPTY);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_FLOAT);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnIncompatibleType)
{
        auto result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_BOOLEAN);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        json_fitting_type(JSON_LIST_TYPE_FIXED_BOOLEAN, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnEnlargeType)
{
        auto result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_U64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_U64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I8);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I8, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I16, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I32, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_I64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I16);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I8);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I32);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I16);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_FIXED_I64);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I32);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U8, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U16, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U32, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);

        result = json_fitting_type(JSON_LIST_TYPE_FIXED_U64, JSON_LIST_TYPE_FIXED_I64);
        ASSERT_EQ(result, JSON_LIST_TYPE_VARIABLE_OR_NESTED);
}

TEST(JsonTest, ParseNullArray)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[null, null, null]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 3);
        struct jak_json_element *e1 = vec_get(&json.element->value.value.array->elements.elements, 0, struct jak_json_element);
        struct jak_json_element *e2 = vec_get(&json.element->value.value.array->elements.elements, 1, struct jak_json_element);
        struct jak_json_element *e3 = vec_get(&json.element->value.value.array->elements.elements, 2, struct jak_json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e3->value.value_type, JSON_VALUE_NULL);

        json_drop(&json);
}

TEST(JsonTest, ParseBooleanArray)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[true, false]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 2);
        struct jak_json_element *e1 = vec_get(&json.element->value.value.array->elements.elements, 0, struct jak_json_element);
        struct jak_json_element *e2 = vec_get(&json.element->value.value.array->elements.elements, 1, struct jak_json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_TRUE);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_FALSE);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromString)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{\"Hello World\": \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 1);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                struct jak_json_prop))->key.value, "Hello World") == 0);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                            struct jak_json_prop))->value.value.value.string->value, "Value") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotes)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{Hello_World: \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 1);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                            struct jak_json_prop))->key.value, "Hello_World") == 0);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                            struct jak_json_prop))->value.value.value.string->value, "Value") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesList)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "{Hello: Value1,\nWorld: \"Value2\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_OBJECT);
        ASSERT_EQ(json.element->value.value.object->value->members.num_elems, 2);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                            struct jak_json_prop))->key.value, "Hello") == 0);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 0,
                            struct jak_json_prop))->value.value.value.string->value, "Value1") == 0);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 1,
                            struct jak_json_prop))->key.value, "World") == 0);
        ASSERT_TRUE(strcmp((vec_get(&json.element->value.value.object->value->members, 1,
                            struct jak_json_prop))->value.value.value.string->value, "Value2") == 0);

        json_drop(&json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesTestNull)
{
        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "null");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_NULL);

        json_drop(&json);
}

TEST(JsonTest, ParseRandomJson)
{
        /* the working directory must be 'tests/carbon' to find this file */
        int fd = open("./assets/random.json", O_RDONLY);
        ASSERT_NE(fd, -1);
        int json_in_len = lseek(fd, 0, SEEK_END);
        const char *json_in = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        struct jak_json_parser parser;
        struct jak_json json;
        struct jak_json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, json_in);
        ASSERT_TRUE(status);

        json_drop(&json);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}