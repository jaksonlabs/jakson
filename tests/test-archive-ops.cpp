#include <gtest/gtest.h>

#include <inttypes.h>
#include "core/carbon/archive/query.h"
#include "core/carbon.h"

TEST(CarbonArchiveOpsTest, CreateStreamFromJsonString)
{
    carbon_memblock_t *stream;
    struct err       err;

    const char        *json_string = "{ \"test\": 123 }";
    bool               read_optimized = false;

    bool status = carbon_archive_stream_from_json(&stream, &err, json_string,
                                                  CARBON_COMPRESSOR_NONE, CARBON_STRDIC_TYPE_SYNC, 0, read_optimized, false, NULL);

    carbon_memblock_drop(stream);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, CreateArchiveFromJsonString)
{
    carbon_archive_t   archive;
    struct err       err;

    const char        *json_string = "{ \"test\": 123 }";
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                           CARBON_COMPRESSOR_NONE, CARBON_STRDIC_TYPE_SYNC, 0, read_optimized, false, NULL);
    ASSERT_TRUE(status);
    bool has_index;
    carbon_archive_has_query_index_string_id_to_offset(&has_index, &archive);
    ASSERT_TRUE(has_index == false);

    carbon_archive_close(&archive);

}

TEST(CarbonArchiveOpsTest, CreateArchiveFromJsonStringWithBakedStringIdIndex)
{
    carbon_archive_t   archive;
    struct err       err;

    const char        *json_string = "{ \"test\": 123 }";
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                           CARBON_COMPRESSOR_NONE, CARBON_STRDIC_TYPE_SYNC, 0, read_optimized, true, NULL);
    ASSERT_TRUE(status);
    bool has_index;
    carbon_archive_has_query_index_string_id_to_offset(&has_index, &archive);
    ASSERT_TRUE(has_index == true);

    carbon_archive_close(&archive);

}

TEST(CarbonArchiveOpsTest, CreateArchiveStringHandling)
{
    std::set<carbon_string_id_t> haystack;

    carbon_archive_t     archive;
    carbon_strid_iter_t  strid_iter;
    carbon_strid_info_t *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;
    struct err         err;
    carbon_query_t       query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = carbon_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (carbon_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            /* Note, that 'info[i].id' cannot be tested based on its value because it is not deterministic generated;
             * all ids must be unique. In case we read something wrong, we may find some duplicate
             * (which is unlikely, however) */
            auto result = haystack.find(info[i].id);
            if (result != haystack.end()) {
                FAIL() << "id collision for { \"id\": " << info[i].id << " }!\n";
            }
            haystack.insert(info[i].id);
        }
    }

    status = carbon_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    status = carbon_query_drop(&query);
    ASSERT_TRUE(status);

    status = carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, DecodeStringByIdFullScan)
{
    std::set<carbon_string_id_t> all_str_ids;

    carbon_archive_t     archive;
    carbon_strid_iter_t  strid_iter;
    carbon_strid_info_t *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;
    struct err         err;
    carbon_query_t       query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = carbon_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (carbon_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            all_str_ids.insert(info[i].id);
        }
    }

    status = carbon_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    for (std::set<carbon_string_id_t>::iterator it = all_str_ids.begin(); it != all_str_ids.end(); it++) {
        carbon_string_id_t string_id = *it;
        char *string = carbon_query_fetch_string_by_id(&query, string_id);
        ASSERT_TRUE(string != NULL);
        printf("DecodeStringByIdFullScan: %" PRIu64 " -> '%s'\n", string_id, string);
        free(string);
    }

    status = carbon_query_drop(&query);
    ASSERT_TRUE(status);

    status = carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, DecodeStringByFastUnsafeAccess)
{
    carbon_archive_t                 archive;
    carbon_strid_iter_t              strid_iter;
    carbon_strid_info_t             *info;
    size_t                           vector_len;
    bool                             status;
    bool                             success;
    struct err                     err;
    carbon_query_t                   query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = carbon_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (carbon_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            char **strings = carbon_query_fetch_strings_by_offset(&query, &(info[i].offset), &(info[i].strlen), 1);
            ASSERT_TRUE(strings != NULL);
            ASSERT_TRUE(strings[0] != NULL);
            printf("%" PRIu64 " -> '%s'\n", info[i].id, strings[0]);
            free(strings[0]);
            free(strings);
        }
    }

    status = carbon_query_drop(&query);
    ASSERT_TRUE(status);

    status = carbon_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    status = carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, FindStringIdMatchingPredicateContains)
{
    carbon_archive_t      archive;
    carbon_query_t        query;
    bool                  status;
    size_t                num_match;
    carbon_string_pred_t  pred;
    carbon_string_id_t   *result;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    const char *needle = "arg";

    carbon_string_pred_contains_init(&pred);
    result = carbon_query_find_ids(&num_match, &query, &pred, (void *) needle, CARBON_QUERY_LIMIT_NONE);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(num_match == 4);

    for (size_t i = 0; i < num_match; i++) {
        char *string = carbon_query_fetch_string_by_id(&query, result[i]);
        ASSERT_TRUE(string != NULL);
        printf("MATCHED CONTAINS %" PRIu64 " ('%s')\n", result[i], string);
        ASSERT_TRUE(strstr(string, needle) != NULL);
        free(string);
    }

    free(result);

    carbon_archive_close(&archive);
}

TEST(CarbonArchiveOpsTest, FindStringIdMatchingPredicateEquals)
{
    carbon_archive_t      archive;
    carbon_query_t        query;
    bool                  status;
    size_t                num_match;
    carbon_string_pred_t  pred;
    carbon_string_id_t   *result;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    const char *needle = "phoneNumbers";

    carbon_string_pred_equals_init(&pred);
    result = carbon_query_find_ids(&num_match, &query, &pred, (void *) needle, CARBON_QUERY_LIMIT_NONE);
    ASSERT_TRUE(result != NULL);

    ASSERT_TRUE(num_match == 1);
    char *string = carbon_query_fetch_string_by_id(&query, result[0]);
    ASSERT_TRUE(string != NULL);
    printf("MATCHED EQUALS %" PRIu64 " ('%s')\n", result[0], string);
    ASSERT_TRUE(strcmp(string, needle) == 0);
    ASSERT_TRUE(result[0] == 72057594037928054);
    free(string);

    free(result);

    carbon_archive_close(&archive);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}