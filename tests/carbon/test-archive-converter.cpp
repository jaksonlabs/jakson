#include <gtest/gtest.h>

#include <jak_carbon.h>

TEST(ConverterTest, PerformConversion)
{
    bool status;
    jak_archive archive;
    jak_encoded_doc_list collection;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    //status = jak_archive_open(&archive, "../mag_papers_excerpt.carbon");
    ASSERT_TRUE(status);

    jak_archive_converter(&collection, &archive);
    for (int i = 0; i < 1; i++)
    {
        printf("\n\n\n*******************************\n\n\n");

        jak_encoded_doc_collection_print(stdout, &collection);


        struct jak_string_cache *cache = jak_archive_get_query_string_id_cache(&archive);
        jak_sid_cache_stats statistics;
        jak_string_id_cache_get_statistics(&statistics, cache);
        fprintf(stderr, "string_id_cache_info hits: %zu   misses: %zu   hit ratio: %.4f   num evicted: %zu\n",
                statistics.num_hits, statistics.num_misses,
                100.0f * statistics.num_hits / (float) (statistics.num_hits + statistics.num_misses),
                statistics.num_evicted);
        jak_string_id_cache_reset_statistics(cache);
    }
    printf("\n\n\n** CLOSING *****************************\n\n\n");
    jak_encoded_doc_collection_drop(&collection);


    jak_archive_close(&archive);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}