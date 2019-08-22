#include <gtest/gtest.h>
#include <printf.h>

#include <jak_carbon.h>

TEST(GlobalIdTest, CreateId) {
    global_id_t id;
    bool result = global_id_create(&id);
    EXPECT_TRUE(result);
    EXPECT_NE(id, 0);
}

TEST(GlobalIdTest, CreateUniqueIds) {
    std::set<global_id_t> haystack;
    for (size_t i = 0; i < 1000000; i++) {
        global_id_t id;
        bool result = global_id_create(&id);
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