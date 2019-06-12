#include <gtest/gtest.h>
#include "core/bison/bison-media.h"

TEST(MimeTypeTest, TestMimeMapping) {

        for (u32 i = 0; i < _nmime_type_register; i++) {
                u32 id = bison_media_mime_type_by_ext(mime_type_register[i].ext);
                printf("lookup id %d for '%s' (%s)\n", i, mime_type_register[i].type, mime_type_register[i].ext);
                ASSERT_EQ(i, id);
                ASSERT_TRUE(strcmp(mime_type_register[i].type, bison_media_mime_type_by_id(id)) == 0);
                ASSERT_TRUE(strcmp(mime_type_register[i].ext, bison_media_mime_ext_by_id(id)) == 0);
        }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}