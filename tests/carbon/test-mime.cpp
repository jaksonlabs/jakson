#include <gtest/gtest.h>
#include <ark-js/carbon/carbon-media.h>

TEST(MimeTypeTest, TestMimeMapping) {

        for (u32 i = 0; i < _nmime_type_register; i++) {
                u32 id = carbon_media_mime_type_by_ext(mime_type_register[i].ext);
                printf("lookup id %d for '%s' (%s)\n", i, mime_type_register[i].type, mime_type_register[i].ext);
                ASSERT_EQ(i, id);
                ASSERT_TRUE(strcmp(mime_type_register[i].type, carbon_media_mime_type_by_id(id)) == 0);
                ASSERT_TRUE(strcmp(mime_type_register[i].ext, carbon_media_mime_ext_by_id(id)) == 0);
        }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}