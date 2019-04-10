#include <gtest/gtest.h>
#include <printf.h>

#include "carbon.h"

TEST(ObjectIdTest, CreateId) {
    carbon_object_id_t id;
    bool result = carbon_object_id_create(&id);
    EXPECT_TRUE(result);
    EXPECT_NE(id, 0);
}

TEST(ObjectIdTest, CreateUniqueIds) {
    std::set<carbon_object_id_t> haystack;
    for (size_t i = 0; i < 1000000; i++) {
        carbon_object_id_t id;
        bool result = carbon_object_id_create(&id);
        if (!result) {
            printf("NO CAPACITY LEFT %zu\n", i);
        } else {
            auto result = haystack.find(id);
            if (result != haystack.end()) {
                FAIL() << "id collision for { \"id\": " << id << " } after " << i << " iterations!\n";
            }
            haystack.insert(id);
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}