#include <gtest/gtest.h>

#include "carbon/carbon.h"

TEST(ObjectIdTest, CreateId) {
    carbon_object_id_t id = carbon_object_id_create();
    EXPECT_NE(id, 0);
}

TEST(ObjectIdTest, CreateUniqueIds) {
    std::set<carbon_object_id_t> haystack;
    for (size_t i = 0; i < 10000000; i++) {
        carbon_object_id_t id = carbon_object_id_create();
        uint_fast8_t path, time;
        carbon_object_id_get_global_build_path_bit(&path, id);
        carbon_object_id_get_global_build_time_bit(&time, id);
        printf("path bit: %u ", path);
        printf("time bit: %u\n", time);
        auto result = haystack.find(id);
        if (result != haystack.end()) {
            FAIL() << "id collision for { \"id\": " << id << " } after " << i << " iterations!\n";
        }
        haystack.insert(id);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}