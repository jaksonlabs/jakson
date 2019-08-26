#include <gtest/gtest.h>

#include <jakson/jakson.h>

TEST(MimeTypeTest, TestMimeMapping) {

        for (jak_u32 i = 0; i < _jak_global_mime_type_register; i++) {
                jak_u32 id = jak_carbon_media_mime_type_by_ext(jak_global_mime_type_register[i].ext);
                printf("lookup id %d for '%s' (%s)\n", i, jak_global_mime_type_register[i].type, jak_global_mime_type_register[i].ext);
                ASSERT_EQ(i, id);
                ASSERT_TRUE(strcmp(jak_global_mime_type_register[i].type, jak_carbon_media_mime_type_by_id(id)) == 0);
                ASSERT_TRUE(strcmp(jak_global_mime_type_register[i].ext, jak_carbon_media_mime_ext_by_id(id)) == 0);
        }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}