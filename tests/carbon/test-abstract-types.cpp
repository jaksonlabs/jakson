#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(TestAbstractTypes, XXXX) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_UNSORTED_MULTISET);
        carbon_create_end(&context);

        char *json = carbon_to_json_compact_dup(&doc);
        ASSERT_TRUE(strcmp(json, "{}") == 0);
        fprintf(stderr, "%s\n", json);



        free(json);

        EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}