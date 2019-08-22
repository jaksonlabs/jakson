#include <gtest/gtest.h>
#include <printf.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <jak_json.h>

TEST(JsonTest, JsonListTypeForColumnEqualTypes) {
        auto result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_EMPTY);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_FLOAT, JAK_JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_FLOAT);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnOverwriteEmpty) {
        auto result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_EMPTY);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_FLOAT);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_BOOLEAN);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_FLOAT, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_FLOAT);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnNestedWins)
{
        auto result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);


        jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_FLOAT, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnNullable)
{
        auto result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_FLOAT, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_FLOAT);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_BOOLEAN);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_FLOAT);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_NULL);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_NULL);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_NULL, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_BOOLEAN);
}

TEST(JsonTest, JsonListTypeForColumnForceMixed)
{
        auto result = jak_json_fitting_type(JAK_JSON_LIST_EMPTY, JAK_JSON_LIST_VARIABLE_OR_NESTED);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_FLOAT, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_VARIABLE_OR_NESTED, JAK_JSON_LIST_EMPTY);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_FLOAT);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnIncompatibleType)
{
        auto result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_BOOLEAN);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        jak_json_fitting_type(JAK_JSON_LIST_FIXED_BOOLEAN, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, JsonListTypeForColumnEnlargeType)
{
        auto result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_U64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_U64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I8);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I8, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I16, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I32, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_I64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I16);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I8);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I32);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I16);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_FIXED_I64);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I32);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U8, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U16, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U32, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);

        result = jak_json_fitting_type(JAK_JSON_LIST_FIXED_U64, JAK_JSON_LIST_FIXED_I64);
        ASSERT_EQ(result, JAK_JSON_LIST_VARIABLE_OR_NESTED);
}

TEST(JsonTest, ParseNullArray)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "[null, null, null]");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_ARRAY);
        ASSERT_EQ(jak_json.element->value.value.array->elements.elements.num_elems, 3);
        jak_json_element *e1 = JAK_VECTOR_GET(&jak_json.element->value.value.array->elements.elements, 0, jak_json_element);
        jak_json_element *e2 = JAK_VECTOR_GET(&jak_json.element->value.value.array->elements.elements, 1, jak_json_element);
        jak_json_element *e3 = JAK_VECTOR_GET(&jak_json.element->value.value.array->elements.elements, 2, jak_json_element);
        ASSERT_EQ(e1->value.value_type, JAK_JSON_VALUE_NULL);
        ASSERT_EQ(e2->value.value_type, JAK_JSON_VALUE_NULL);
        ASSERT_EQ(e3->value.value_type, JAK_JSON_VALUE_NULL);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseBooleanArray)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "[true, false]");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_ARRAY);
        ASSERT_EQ(jak_json.element->value.value.array->elements.elements.num_elems, 2);
        jak_json_element *e1 = JAK_VECTOR_GET(&jak_json.element->value.value.array->elements.elements, 0, jak_json_element);
        jak_json_element *e2 = JAK_VECTOR_GET(&jak_json.element->value.value.array->elements.elements, 1, jak_json_element);
        ASSERT_EQ(e1->value.value_type, JAK_JSON_VALUE_TRUE);
        ASSERT_EQ(e2->value.value_type, JAK_JSON_VALUE_FALSE);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseJsonFromString)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "{\"Hello World\": \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_OBJECT);
        ASSERT_EQ(jak_json.element->value.value.object->value->members.num_elems, 1);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                jak_json_prop))->key.value, "Hello World") == 0);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                            jak_json_prop))->value.value.value.string->value, "Value") == 0);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotes)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "{Hello_World: \"Value\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_OBJECT);
        ASSERT_EQ(jak_json.element->value.value.object->value->members.num_elems, 1);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                            jak_json_prop))->key.value, "Hello_World") == 0);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                            jak_json_prop))->value.value.value.string->value, "Value") == 0);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesList)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "{Hello: Value1,\nWorld: \"Value2\"}");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_OBJECT);
        ASSERT_EQ(jak_json.element->value.value.object->value->members.num_elems, 2);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                            jak_json_prop))->key.value, "Hello") == 0);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 0,
                            jak_json_prop))->value.value.value.string->value, "Value1") == 0);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 1,
                            jak_json_prop))->key.value, "World") == 0);
        ASSERT_TRUE(strcmp((JAK_VECTOR_GET(&jak_json.element->value.value.object->value->members, 1,
                            jak_json_prop))->value.value.value.string->value, "Value2") == 0);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseJsonFromStringLaxQuotesTestNull)
{
        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, "null");
        ASSERT_TRUE(status);
        ASSERT_EQ(jak_json.element->value.value_type, JAK_JSON_VALUE_NULL);

        jak_json_drop(&jak_json);
}

TEST(JsonTest, ParseRandomJson)
{
        /* the working directory must be 'tests/jakson-tool' to find this file */
        int fd = open("./assets/random.json", O_RDONLY);
        ASSERT_NE(fd, -1);
        int json_in_len = lseek(fd, 0, SEEK_END);
        const char *json_in = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        jak_json_parser parser;
        jak_json jak_json;
        jak_json_err error_desc;
        jak_json_parser_create(&parser);
        bool status = jak_json_parse(&jak_json, &error_desc, &parser, json_in);
        ASSERT_TRUE(status);

        jak_json_drop(&jak_json);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}