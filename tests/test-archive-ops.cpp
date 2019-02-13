#include <gtest/gtest.h>

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
                            "        \"configGlossary:poweredBy\": \"Cofax\",\n"
                            "        \"configGlossary:poweredByIcon\": \"/images/cofax.gif\",\n"
                            "        \"configGlossary:staticPath\": \"/content/static\",\n"
                            "        \"templateProcessorClass\": \"org.cofax.WysiwygTemplate\",\n"
                            "        \"templateLoaderClass\": \"org.cofax.FilesTemplateLoader\",\n"
                            "        \"templatePath\": \"templates\",\n"
                            "        \"templateOverridePath\": \"\",\n"
                            "        \"defaultListTemplate\": \"listTemplate.htm\",\n"
                            "        \"defaultFileTemplate\": \"articleTemplate.htm\",\n"
                            "        \"useJSP\": false,\n"
                            "        \"jspListTemplate\": \"listTemplate.jsp\",\n"
                            "        \"jspFileTemplate\": \"articleTemplate.jsp\",\n"
                            "        \"cachePackageTagsTrack\": 200,\n"
                            "        \"cachePackageTagsStore\": 200,\n"
                            "        \"cachePackageTagsRefresh\": 60,\n"
                            "        \"cacheTemplatesTrack\": 100,\n"
                            "        \"cacheTemplatesStore\": 50,\n"
                            "        \"cacheTemplatesRefresh\": 15,\n"
                            "        \"cachePagesTrack\": 200,\n"
                            "        \"cachePagesStore\": 100,\n"
                            "        \"cachePagesRefresh\": 10,\n"
                            "        \"cachePagesDirtyRead\": 10,\n"
                            "        \"searchEngineListTemplate\": \"forSearchEnginesList.htm\",\n"
                            "        \"searchEngineFileTemplate\": \"forSearchEngines.htm\",\n"
                            "        \"searchEngineRobotsDb\": \"WEB-INF/robots.db\",\n"
                            "        \"useDataStore\": true,\n"
                            "        \"dataStoreClass\": \"org.cofax.SqlDataStore\",\n"
                            "        \"redirectionClass\": \"org.cofax.SqlRedirection\",\n"
                            "        \"dataStoreName\": \"cofax\",\n"
                            "        \"dataStoreDriver\": \"com.microsoft.jdbc.sqlserver.SQLServerDriver\",\n"
                            "        \"dataStoreUrl\": \"jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon\",\n"
                            "        \"dataStoreUser\": \"sa\",\n"
                            "        \"dataStorePassword\": \"dataStoreTestQuery\",\n"
                            "        \"dataStoreTestQuery\": \"SET NOCOUNT ON;select test='test';\",\n"
                            "        \"dataStoreLogFile\": \"/usr/local/tomcat/logs/datastore.log\",\n"
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
                            "        \"logLocation\": \"/usr/local/tomcat/logs/CofaxTools.log\",\n"
                            "        \"logMaxSize\": \"\",\n"
                            "        \"dataLog\": 1,\n"
                            "        \"dataLogLocation\": \"/usr/local/tomcat/logs/dataLog.log\",\n"
                            "        \"dataLogMaxSize\": \"\",\n"
                            "        \"removePageCache\": \"/content/admin/remove?cache=pages&id=\",\n"
                            "        \"removeTemplateCache\": \"/content/admin/remove?cache=templates&id=\",\n"
                            "        \"fileTransferFolder\": \"/usr/local/tomcat/webapps/content/fileTransferFolder\",\n"
                            "        \"lookInContext\": 1,\n"
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
    carbon_archive_drop(&archive);
    ASSERT_TRUE(status);
}

TEST(CarbonArchiveOpsTest, CreateArchiveStringHandling)
{
    carbon_archive_t   archive;
    carbon_err_t       err;

    const char        *json_string = JSON_EXAMPLE;
    const char        *archive_file = "tmp-test-archive.carbon";
    bool               read_optimized = false;

    bool status = carbon_archive_from_json(&archive, archive_file, &err, json_string,
                                           CARBON_COMPRESSOR_NONE, read_optimized);



    carbon_archive_drop(&archive);
    ASSERT_TRUE(status);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}