#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(GlobalIdTest, CreateId) {
    jak_uid_t id;
    bool result = jak_unique_id_create(&id);
    EXPECT_TRUE(result);
    EXPECT_NE(id, 0);
}

TEST(GlobalIdTest, CreateUniqueIds) {
    std::set<jak_uid_t> haystack;
    for (size_t i = 0; i < 1000000; i++) {
        jak_uid_t id;
        bool result = jak_unique_id_create(&id);
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