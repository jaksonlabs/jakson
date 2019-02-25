#include <gtest/gtest.h>

#include "carbon/carbon.h"

TEST(ConverterTest, PerformConversion)
{
    bool status;
    carbon_archive_t archive;
    carbon_encoded_doc_collection_t collection;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    //status = carbon_archive_open(&archive, "../mag_papers_excerpt.carbon");
    ASSERT_TRUE(status);

    carbon_archive_converter(&collection, &archive);
    carbon_encoded_doc_collection_print(stdout, &collection);
    carbon_encoded_doc_collection_drop(&collection);

    carbon_archive_close(&archive);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}