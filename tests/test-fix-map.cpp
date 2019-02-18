#include <gtest/gtest.h>
#include <stdio.h>

#include "carbon/carbon.h"

TEST(FixMapTest, CreationAndDrop)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    status = carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 100);
    ASSERT_TRUE(status);
    status = carbon_fix_map_drop(&map);
    ASSERT_TRUE(status);
}

TEST(FixMapTest, MapAndGetWithoutRehash)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 100);

    for (uint32_t key = 0; key < 10; key++) {
        uint64_t value = key << 2;
        status = carbon_fix_map_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (uint32_t key = 0; key < 10; key++) {
        const void *value = carbon_fix_map_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(uint64_t *) value == key << 2);
    }

    carbon_fix_map_drop(&map);

}

TEST(FixMapTest, MapAndGetWitRehash)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 10);

    for (uint32_t key = 0; key < 10000; key++) {
        uint64_t value = key << 2;
        status = carbon_fix_map_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (uint32_t key = 0; key < 10000; key++) {
        const void *value = carbon_fix_map_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(uint64_t *) value == key << 2);
    }

    carbon_fix_map_drop(&map);
}

TEST(FixMapTest, DisplaceTest)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 10700);

    for (uint32_t key = 0; key < 10000; key++) {
        uint64_t value = key << 2;
        status = carbon_fix_map_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (uint32_t key = 0; key < 10000; key++) {
        const void *value = carbon_fix_map_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(uint64_t *) value == key << 2);
    }

    float dis;
    carbon_fix_map_avg_displace(&dis, &map);
    printf("Avg Displace: %f\n", dis);


    carbon_fix_map_drop(&map);
}



TEST(FixMapTest, MapAndGetNotContainedWithoutRehash)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 100);

    for (uint32_t key = 0; key < 10; key++) {
        uint64_t value = key << 2;
        status = carbon_fix_map_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (uint32_t key = 0; key < 10; key++) {
        uint32_t unknown_key = 10 + key;
        const void *value = carbon_fix_map_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    carbon_fix_map_drop(&map);
}

TEST(FixMapTest, MapAndGetNotContainedWitRehash)
{
    carbon_fix_map_t map;
    carbon_err_t     err;
    bool             status;

    carbon_fix_map_create(&map, &err, sizeof(uint32_t), sizeof(uint64_t), 10);

    for (uint32_t key = 0; key < 10000; key++) {
        uint64_t value = key << 2;
        status = carbon_fix_map_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (uint32_t key = 0; key < 10000; key++) {
        uint32_t unknown_key = 10000 + key;
        const void *value = carbon_fix_map_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    carbon_fix_map_drop(&map);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}