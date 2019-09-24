#include <gtest/gtest.h>
#include <jakson/jakson.h>

TEST(CarbonResultTest, CreateOK) {
        fn_result result = JAK_RESULT_OK();
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_FALSE(JAK_RESULT_HAS_VALUE(result));
}

TEST(CarbonResultTest, CreateFail) {
        fn_result result = JAK_RESULT_FAIL(JAK_ERR_NOTFOUND, "no element found");
        ASSERT_FALSE(JAK_RESULT_IS_OK(result));
        ASSERT_FALSE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_TRUE(JAK_RESULT_CODE(result) == JAK_ERR_NOTFOUND);
        struct jak_error *err = JAK_RESULT_GET_LAST_ERROR()
        ASSERT_TRUE(strcmp(err->details, "no element found") == 0);
}

TEST(CarbonResultTest, CreateOKInt) {
        fn_result result = JAK_RESULT_OK_INT(UINT32_MAX);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_EQ(JAK_RESULT_INT_VALUE(result), UINT32_MAX);
}

TEST(CarbonResultTest, CreateOKUIntMin) {
        fn_result result = JAK_RESULT_OK_UINT(INT32_MIN);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_EQ(JAK_RESULT_UINT_VALUE(result), INT32_MIN);
}

TEST(CarbonResultTest, CreateOKUIntMax) {
        fn_result result = JAK_RESULT_OK_UINT(INT32_MAX);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_EQ(JAK_RESULT_UINT_VALUE(result), INT32_MAX);
}

TEST(CarbonResultTest, CreateOKTrue) {
        fn_result result = JAK_RESULT_OK_BOOL(true);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_EQ(JAK_RESULT_BOOL_VALUE(result), true);
}

TEST(CarbonResultTest, CreateOKFalse) {
        fn_result result = JAK_RESULT_OK_BOOL(false);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        ASSERT_EQ(JAK_RESULT_BOOL_VALUE(result), false);
}

TEST(CarbonResultTest, CreateOKPtr) {
        int my_value = 123;
        int *ptr_in = &my_value;
        fn_result result = JAK_RESULT_OK_PTR(&my_value);
        ASSERT_TRUE(JAK_RESULT_IS_OK(result));
        ASSERT_TRUE(JAK_RESULT_HAS_VALUE(result));
        int *ptr_out = (int *)JAK_RESULT_PTR_VALUE(result);
        JAK_UNUSED(ptr_out)
        ASSERT_EQ(ptr_in, ptr_out);
        ASSERT_EQ(*(int *)JAK_RESULT_PTR_VALUE(result), my_value);
}

fn_result f(void *p1, void *p2, void *p3) {
        FN_FAIL_IF_NULL(p1, p2, p3)
        return JAK_RESULT_OK();
}

TEST(CarbonResultTest, NonNullTest) {
        int a, b, c;
        ASSERT_TRUE(JAK_RESULT_IS_OK(f(&a, &b, &c)));
        ASSERT_FALSE(JAK_RESULT_IS_OK(f(&a, &b, NULL)));
        ASSERT_FALSE(JAK_RESULT_IS_OK(f(NULL, &b, NULL)));
        ASSERT_FALSE(JAK_RESULT_IS_OK(f(NULL, NULL, NULL)));
}

int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}