#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(CarbonPathIndexTest, PathIndexCreate) {
        jak_carbon_path_index index;
        jak_carbon doc;
        jak_error err;

        const char *jak_json = "[\n"
                               "   {\n"
                               "      \"a\":null,\n"
                               "      \"b\":[ 1, 2, 3 ],\n"
                               "      \"c\":{\n"
                               "         \"msg\":\"Hello, World!\"\n"
                               "      }\n"
                               "   },\n"
                               "   {\n"
                               "      \"a\":42,\n"
                               "      \"b\":[ ],\n"
                               "      \"c\":null\n"
                               "   }\n"
                               "]";

//        /* the working directory must be the repository root */
//        int fd = open("tests/carbon/assets/ms-academic-graph.json", O_RDONLY);
//        ASSERT_NE(fd, -1);
//        int json_in_len = lseek(fd, 0, SEEK_END);
//        const char *jak_json = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        jak_carbon_from_json(&doc, jak_json, JAK_CARBON_KEY_NOKEY, NULL, &err);
        jak_carbon_path_index_create(&index, &doc);
        jak_carbon_path_index_print(stdout, &index);
        jak_carbon_hexdump_print(stdout, &doc);
        jak_carbon_path_index_hexdump(stdout, &index);

        jak_carbon path_carbon;
        jak_carbon_path_index_to_carbon(&path_carbon, &index);
        jak_carbon_print(stdout, JAK_JSON_COMPACT, &path_carbon);
        jak_carbon_drop(&path_carbon);

        ASSERT_TRUE(jak_carbon_path_index_indexes_doc(&index, &doc));
        jak_carbon_drop(&doc);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}