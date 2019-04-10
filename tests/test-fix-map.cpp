#include <gtest/gtest.h>
#include <stdio.h>

#include "core/carbon.h"

TEST(FixMapTest, CreationAndDrop)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    status = carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 100);
    ASSERT_TRUE(status);
    status = carbon_hashtable_drop(&map);
    ASSERT_TRUE(status);
}

TEST(FixMapTest, MapAndGetWithoutRehash)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 100);

    for (u32 key = 0; key < 10; key++) {
        u64 value = key << 2;
        status = carbon_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (u32 key = 0; key < 10; key++) {
        const void *value = carbon_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(u64 *) value == key << 2);
    }

    carbon_hashtable_drop(&map);

}

TEST(FixMapTest, MapAndGetWitRehash)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 10);

    for (u32 key = 0; key < 10000; key++) {
        u64 value = key << 2;
        status = carbon_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (u32 key = 0; key < 10000; key++) {
        const void *value = carbon_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(u64 *) value == key << 2);
    }

    carbon_hashtable_drop(&map);
}

TEST(FixMapTest, DisplaceTest)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 10700);

    for (u32 key = 0; key < 10000; key++) {
        u64 value = key << 2;
        status = carbon_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (u32 key = 0; key < 10000; key++) {
        const void *value = carbon_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(u64 *) value == key << 2);
    }

    float dis;
    carbon_hashtable_avg_displace(&dis, &map);
    printf("Avg Displace: %f\n", dis);


    carbon_hashtable_drop(&map);
}



TEST(FixMapTest, MapAndGetNotContainedWithoutRehash)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 100);

    for (u32 key = 0; key < 10; key++) {
        u64 value = key << 2;
        status = carbon_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (u32 key = 0; key < 10; key++) {
        u32 unknown_key = 10 + key;
        const void *value = carbon_hashtable_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    carbon_hashtable_drop(&map);
}

TEST(FixMapTest, MapAndGetNotContainedWitRehash)
{
    carbon_hashtable_t map;
    struct err     err;
    bool             status;

    carbon_hashtable_create(&map, &err, sizeof(u32), sizeof(u64), 10);

    for (u32 key = 0; key < 10000; key++) {
        u64 value = key << 2;
        status = carbon_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (u32 key = 0; key < 10000; key++) {
        u32 unknown_key = 10000 + key;
        const void *value = carbon_hashtable_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    carbon_hashtable_drop(&map);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}