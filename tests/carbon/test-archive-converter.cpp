#include <gtest/gtest.h>

#include <jakson/jakson.h>

TEST(ConverterTest, PerformConversion)
{
    bool status;
    archive archive;
    encoded_doc_list collection;

    /* in order to access this file, the working directory must 'test/carbon' */
    status = archive_open(&archive, "./assets/test-archive.carbon");
    //status = archive_open(&archive, "../mag_papers_excerpt.jakson-tool");
    ASSERT_TRUE(status);

    archive_converter(&collection, &archive);
    for (int i = 0; i < 1; i++)
    {
        printf("\n\n\n*******************************\n\n\n");

        encoded_doc_collection_print(stdout, &collection);


        struct string_cache *cache = archive_get_query_string_id_cache(&archive);
        sid_cache_stats statistics;
        string_id_cache_get_statistics(&statistics, cache);
        fprintf(stderr, "string_id_cache_info hits: %zu   misses: %zu   hit ratio: %.4f   num evicted: %zu\n",
                statistics.num_hits, statistics.num_misses,
                100.0f * statistics.num_hits / (float) (statistics.num_hits + statistics.num_misses),
                statistics.num_evicted);
        string_id_cache_reset_statistics(cache);
    }
    printf("\n\n\n** CLOSING *****************************\n\n\n");
    encoded_doc_collection_drop(&collection);


    archive_close(&archive);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}