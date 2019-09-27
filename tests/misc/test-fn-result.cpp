#include <gtest/gtest.h>
#include <jakson/jakson.h>

TEST(CarbonResultTest, CreateOK) {
        fn_result result = RESULT_OK();
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_FALSE(RESULT_HAS_VALUE(result));
}

TEST(CarbonResultTest, CreateFail) {
        fn_result result = RESULT_FAIL(ERR_NOTFOUND, "no element found");
        ASSERT_FALSE(RESULT_IS_OK(result));
        ASSERT_FALSE(RESULT_HAS_VALUE(result));
        ASSERT_TRUE(RESULT_CODE(result) == ERR_NOTFOUND);
        struct err *err = RESULT_GET_LAST_ERROR()
        ASSERT_TRUE(strcmp(err->details, "no element found") == 0);
}

TEST(CarbonResultTest, CreateOKInt) {
        fn_result result = RESULT_OK_INT(UINT32_MAX);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        ASSERT_EQ(RESULT_INT_VALUE(result), UINT32_MAX);
}

TEST(CarbonResultTest, CreateOKUIntMin) {
        fn_result result = RESULT_OK_UINT(INT32_MIN);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        ASSERT_EQ(RESULT_UINT_VALUE(result), INT32_MIN);
}

TEST(CarbonResultTest, CreateOKUIntMax) {
        fn_result result = RESULT_OK_UINT(INT32_MAX);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        ASSERT_EQ(RESULT_UINT_VALUE(result), INT32_MAX);
}

TEST(CarbonResultTest, CreateOKTrue) {
        fn_result result = RESULT_OK_BOOL(true);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        ASSERT_EQ(RESULT_BOOL_VALUE(result), true);
}

TEST(CarbonResultTest, CreateOKFalse) {
        fn_result result = RESULT_OK_BOOL(false);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        ASSERT_EQ(RESULT_BOOL_VALUE(result), false);
}

TEST(CarbonResultTest, CreateOKPtr) {
        int my_value = 123;
        int *ptr_in = &my_value;
        fn_result result = RESULT_OK_PTR(&my_value);
        ASSERT_TRUE(RESULT_IS_OK(result));
        ASSERT_TRUE(RESULT_HAS_VALUE(result));
        int *ptr_out = (int *)RESULT_PTR_VALUE(result);
        UNUSED(ptr_out)
        ASSERT_EQ(ptr_in, ptr_out);
        ASSERT_EQ(*(int *)RESULT_PTR_VALUE(result), my_value);
}

fn_result f(void *p1, void *p2, void *p3) {
        FN_FAIL_IF_NULL(p1, p2, p3)
        return RESULT_OK();
}

TEST(CarbonResultTest, NonNullTest) {
        int a, b, c;
        ASSERT_TRUE(RESULT_IS_OK(f(&a, &b, &c)));
        ASSERT_FALSE(RESULT_IS_OK(f(&a, &b, NULL)));
        ASSERT_FALSE(RESULT_IS_OK(f(NULL, &b, NULL)));
        ASSERT_FALSE(RESULT_IS_OK(f(NULL, NULL, NULL)));
}

int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}