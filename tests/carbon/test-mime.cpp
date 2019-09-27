#include <gtest/gtest.h>

#include <jakson/jakson.h>

TEST(MimeTypeTest, TestMimeMapping) {

        for (u32 i = 0; i < _global_mime_type_register; i++) {
                u32 id = carbon_media_mime_type_by_ext(global_mime_type_register[i].ext);
                printf("lookup id %d for '%s' (%s)\n", i, global_mime_type_register[i].type, global_mime_type_register[i].ext);
                ASSERT_EQ(i, id);
                ASSERT_TRUE(strcmp(global_mime_type_register[i].type, carbon_media_mime_type_by_id(id)) == 0);
                ASSERT_TRUE(strcmp(global_mime_type_register[i].ext, carbon_media_mime_ext_by_id(id)) == 0);
        }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}