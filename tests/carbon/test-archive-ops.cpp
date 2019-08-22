#include <gtest/gtest.h>

#include <inttypes.h>
#include <jak_archive_query.h>
#include <jak_carbon.h>

TEST(CarbonArchiveOpsTest, CreateStreamFromJsonString)
{
    struct jak_memblock *stream;
    struct jak_error       err;

    const char        *json_string = "{ \"test\": 123 }";
    bool               read_optimized = false;

    bool status = jak_archive_stream_from_json(&stream, &err, json_string,
                                                  PACK_NONE, SYNC, 0, read_optimized, false, NULL);

    memblock_drop(stream);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, CreateArchiveFromJsonString)
{
    struct jak_archive   archive;
    struct jak_error       err;

    const char        *json_string = "{ \"test\": 123 }";
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = jak_archive_from_json(&archive, archive_file, &err, json_string,
                                           PACK_NONE, SYNC, 0, read_optimized, false, NULL);
    ASSERT_TRUE(status);
    bool has_index;
    jak_archive_has_query_index_string_id_to_offset(&has_index, &archive);
    ASSERT_TRUE(has_index == false);

    jak_archive_close(&archive);

}

TEST(CarbonArchiveOpsTest, CreateArchiveFromJsonStringWithBakedStringIdIndex)
{
    struct jak_archive   archive;
    struct jak_error       err;

    const char        *json_string = "{ \"test\": 123 }";
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = jak_archive_from_json(&archive, archive_file, &err, json_string,
                                           PACK_NONE, SYNC, 0, read_optimized, true, NULL);
    ASSERT_TRUE(status);
    bool has_index;
    jak_archive_has_query_index_string_id_to_offset(&has_index, &archive);
    ASSERT_TRUE(has_index == true);

    jak_archive_close(&archive);

}

TEST(CarbonArchiveOpsTest, CreateArchiveStringHandling)
{
    std::set<jak_archive_field_sid_t> haystack;

    struct jak_archive     archive;
    struct jak_strid_iter  strid_iter;
    struct jak_strid_info *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;
    struct jak_error         err;
    struct jak_archive_query       query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = jak_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = jak_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (jak_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            /* Note, that 'info[i].id' cannot be tested based on its value because it is not deterministic generated;
             * all ids must be unique. In case we read something wrong, we may find some duplicate
             * (which is JAK_UNLIKELY, however) */
            auto result = haystack.find(info[i].id);
            if (result != haystack.end()) {
                FAIL() << "id collision for { \"id\": " << info[i].id << " }!\n";
            }
            haystack.insert(info[i].id);
        }
    }

    status = jak_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    status = jak_query_drop(&query);
    ASSERT_TRUE(status);

    status = jak_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, DecodeStringByIdFullScan)
{
    std::set<jak_archive_field_sid_t> all_str_ids;

    struct jak_archive     archive;
    struct jak_strid_iter  strid_iter;
    struct jak_strid_info *info;
    size_t               vector_len;
    bool                 status;
    bool                 success;
    struct jak_error         err;
    struct jak_archive_query       query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = jak_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = jak_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (jak_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            all_str_ids.insert(info[i].id);
        }
    }

    status = jak_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    for (std::set<jak_archive_field_sid_t>::iterator it = all_str_ids.begin(); it != all_str_ids.end(); it++) {
        jak_archive_field_sid_t string_id = *it;
        char *string = jak_query_fetch_string_by_id(&query, string_id);
        ASSERT_TRUE(string != NULL);
        printf("DecodeStringByIdFullScan: %" PRIu64 " -> '%s'\n", string_id, string);
        free(string);
    }

    status = jak_query_drop(&query);
    ASSERT_TRUE(status);

    status = jak_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, DecodeStringByFastUnsafeAccess)
{
    struct jak_archive                 archive;
    struct jak_strid_iter              strid_iter;
    struct jak_strid_info             *info;
    size_t                           vector_len;
    bool                             status;
    bool                             success;
    struct jak_error                     err;
    struct jak_archive_query                   query;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = jak_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = jak_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (jak_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            char **strings = jak_query_fetch_strings_by_offset(&query, &(info[i].offset), &(info[i].strlen), 1);
            ASSERT_TRUE(strings != NULL);
            ASSERT_TRUE(strings[0] != NULL);
            printf("%" PRIu64 " -> '%s'\n", info[i].id, strings[0]);
            free(strings[0]);
            free(strings);
        }
    }

    status = jak_query_drop(&query);
    ASSERT_TRUE(status);

    status = jak_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    status = jak_archive_close(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, FindStringIdMatchingPredicateContains)
{
    struct jak_archive      archive;
    struct jak_archive_query        query;
    bool                  status;
    size_t                num_match;
    struct jak_string_pred_t  pred;
    jak_archive_field_sid_t   *result;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = jak_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    const char *needle = "arg";

    string_pred_contains_init(&pred);
    result = jak_query_find_ids(&num_match, &query, &pred, (void *) needle, JAK_QUERY_LIMIT_NONE);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(num_match == 4);

    for (size_t i = 0; i < num_match; i++) {
        char *string = jak_query_fetch_string_by_id(&query, result[i]);
        ASSERT_TRUE(string != NULL);
        printf("MATCHED CONTAINS %" PRIu64 " ('%s')\n", result[i], string);
        ASSERT_TRUE(strstr(string, needle) != NULL);
        free(string);
    }

    free(result);

    jak_archive_close(&archive);
}

TEST(CarbonArchiveOpsTest, FindStringIdMatchingPredicateEquals)
{
    struct jak_archive      archive;
    struct jak_archive_query        query;
    bool                  status;
    size_t                num_match;
    struct jak_string_pred_t  pred;
    jak_archive_field_sid_t   *result;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = jak_archive_open(&archive, "./assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = jak_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    const char *needle = "phoneNumbers";

    string_pred_equals_init(&pred);
    result = jak_query_find_ids(&num_match, &query, &pred, (void *) needle, JAK_QUERY_LIMIT_NONE);
    ASSERT_TRUE(result != NULL);

    ASSERT_TRUE(num_match == 1);
    char *string = jak_query_fetch_string_by_id(&query, result[0]);
    ASSERT_TRUE(string != NULL);
    printf("MATCHED EQUALS %" PRIu64 " ('%s')\n", result[0], string);
    ASSERT_TRUE(strcmp(string, needle) == 0);
    ASSERT_TRUE(result[0] == 72057594037928054);
    free(string);

    free(result);

    jak_archive_close(&archive);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}