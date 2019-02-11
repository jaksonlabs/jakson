#include <gtest/gtest.h>

#include "carbon/carbon.h"

TEST(CarbonArchiveOpsTest, CreateStreamFromJsonString)
{
    carbon_memblock_t *stream;
    carbon_err_t       err;

    const char        *json_string = "{ \"test\": 123 }";
    bool               read_optimized = false;

    bool status = carbon_archive_stream_from_json(&stream, &err, json_string,
                                                  CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE, read_optimized);
    carbon_memblock_drop(stream);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, CreateArchiveFromJsonString)
{
    carbon_archive_t   archive;
    carbon_err_t       err;

    const char        *json_string = "{ \"test\": 123 }";
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                           CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE, read_optimized);
    carbon_archive_drop(&archive);
    ASSERT_TRUE(status);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}