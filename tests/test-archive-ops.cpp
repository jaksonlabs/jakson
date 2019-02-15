#include <gtest/gtest.h>

#include <inttypes.h>
#include <carbon/carbon-query.h>
#include "carbon/carbon.h"



/* taken from https://json.org/example.html */
const char *JSON_EXAMPLE = "{\"web-app\": {\n"
                            "  \"servlet\": [   \n"
                            "    {\n"
                            "      \"servlet-name\": \"cofaxCDS\",\n"
                            "      \"servlet-class\": \"org.cofax.cds.CDSServlet\",\n"
                            "      \"init-param\": {\n"
                            "        \"configGlossary:installationAt\": \"Philadelphia, PA\",\n"
                            "        \"configGlossary:adminEmail\": \"ksm@pobox.com\",\n"
                            "        \"dataStoreInitConns\": 10,\n"
                            "        \"dataStoreMaxConns\": 100,\n"
                            "        \"dataStoreConnUsageLimit\": 100,\n"
                            "        \"dataStoreLogLevel\": \"debug\",\n"
                            "        \"maxUrlLength\": 500}},\n"
                            "    {\n"
                            "      \"servlet-name\": \"cofaxEmail\",\n"
                            "      \"servlet-class\": \"org.cofax.cds.EmailServlet\",\n"
                            "      \"init-param\": {\n"
                            "      \"mailHost\": \"mail1\",\n"
                            "      \"mailHostOverride\": \"mail2\"}},\n"
                            "    {\n"
                            "      \"servlet-name\": \"cofaxAdmin\",\n"
                            "      \"servlet-class\": \"org.cofax.cds.AdminServlet\"},\n"
                            " \n"
                            "    {\n"
                            "      \"servlet-name\": \"fileServlet\",\n"
                            "      \"servlet-class\": \"org.cofax.cds.FileServlet\"},\n"
                            "    {\n"
                            "      \"servlet-name\": \"cofaxTools\",\n"
                            "      \"servlet-class\": \"org.cofax.cms.CofaxToolsServlet\",\n"
                            "      \"init-param\": {\n"
                            "        \"templatePath\": \"toolstemplates/\",\n"
                            "        \"log\": 1,\n"
                            "        \"adminGroupID\": 4,\n"
                            "        \"betaServer\": true}}],\n"
                            "  \"servlet-mapping\": {\n"
                            "    \"cofaxCDS\": \"/\",\n"
                            "    \"cofaxEmail\": \"/cofaxutil/aemail/*\",\n"
                            "    \"cofaxAdmin\": \"/admin/*\",\n"
                            "    \"fileServlet\": \"/static/*\",\n"
                            "    \"cofaxTools\": \"/tools/*\"},\n"
                            " \n"
                            "  \"taglib\": {\n"
                            "    \"taglib-uri\": \"cofax.tld\",\n"
                            "    \"taglib-location\": \"/WEB-INF/tlds/cofax.tld\"}}}";

TEST(CarbonArchiveOpsTest, CreateStreamFromJsonString)
{
    carbon_memblock_t *stream;
    carbon_err_t       err;

    const char        *json_string = "{ \"test\": 123 }";
    bool               read_optimized = false;

    bool status = carbon_archive_stream_from_json(&stream, &err, json_string,
                                                  CARBON_COMPRESSOR_NONE, read_optimized);
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
                                           CARBON_COMPRESSOR_NONE, read_optimized);
    carbon_archive_close(&archive);
    ASSERT_TRUE(status);
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
    carbon_err_t         err;
    carbon_query_t       query;

    const char        *json_string = JSON_EXAMPLE;
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                      CARBON_COMPRESSOR_NONE, read_optimized);
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = carbon_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (carbon_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            /* longest string is 31 in size; if 'next' is broken, we may read something exceeding this length */
            ASSERT_LE(info[i].strlen, 31);

            /* highest offset is 1590; if 'next' is broken, we may read something beyond this value */
            ASSERT_LE(info[i].offset, 1590);

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
    carbon_err_t         err;
    carbon_query_t       query;

    const char        *json_string = JSON_EXAMPLE;
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                      CARBON_COMPRESSOR_NONE, read_optimized);
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
        char *string = carbon_query_find_string_by_id(&query, string_id);
        ASSERT_TRUE(string != NULL);
        printf("%" PRIu64 " -> '%s'\n", string_id, string);
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
    carbon_err_t                     err;
    carbon_query_t                   query;

    const char        *json_string = JSON_EXAMPLE;
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                      CARBON_COMPRESSOR_NONE, read_optimized);
    ASSERT_TRUE(status);

    status = carbon_archive_query(&query, &archive);
    ASSERT_TRUE(status);

    status = carbon_query_scan_strids(&strid_iter, &query);
    ASSERT_TRUE(status);

    while (carbon_strid_iter_next(&success, &info, &err, &vector_len, &strid_iter)) {
        for (size_t i = 0; i < vector_len; i++) {
            char *string = carbon_query_fetch_string_unsafe(&query, info[i].offset, info[i].strlen);
            ASSERT_TRUE(string != NULL);
            printf("%" PRIu64 " -> '%s'\n", info[i].id, string);
            free(string);
        }
    }

    status = carbon_query_drop(&query);
    ASSERT_TRUE(status);

    status = carbon_strid_iter_close(&strid_iter);
    ASSERT_TRUE(status);

    status = carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}