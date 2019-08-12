#include <gtest/gtest.h>
#include <printf.h>

#include <ark-js/shared/json/json.h>

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
        struct json_parser parser;
        struct json json;
        struct json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[null, null, null]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 3);
        struct json_element *e1 = vec_get(&json.element->value.value.array->elements.elements, 0, struct json_element);
        struct json_element *e2 = vec_get(&json.element->value.value.array->elements.elements, 1, struct json_element);
        struct json_element *e3 = vec_get(&json.element->value.value.array->elements.elements, 2, struct json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_NULL);
        ASSERT_EQ(e3->value.value_type, JSON_VALUE_NULL);

        json_drop(&json);
}

TEST(JsonTest, ParseBooleanArray)
{
        struct json_parser parser;
        struct json json;
        struct json_err error_desc;
        json_parser_create(&parser);
        bool status = json_parse(&json, &error_desc, &parser, "[true, false]");
        ASSERT_TRUE(status);
        ASSERT_EQ(json.element->value.value_type, JSON_VALUE_ARRAY);
        ASSERT_EQ(json.element->value.value.array->elements.elements.num_elems, 2);
        struct json_element *e1 = vec_get(&json.element->value.value.array->elements.elements, 0, struct json_element);
        struct json_element *e2 = vec_get(&json.element->value.value.array->elements.elements, 1, struct json_element);
        ASSERT_EQ(e1->value.value_type, JSON_VALUE_TRUE);
        ASSERT_EQ(e2->value.value_type, JSON_VALUE_FALSE);

        json_drop(&json);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}