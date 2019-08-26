#include <gtest/gtest.h>
#include <stdio.h>

#include <jakson/jakson.h>

TEST(FixMapTest, CreationAndDrop)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    status = jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 100);
    ASSERT_TRUE(status);
    status = jak_hashtable_drop(&map);
    ASSERT_TRUE(status);
}

TEST(FixMapTest, MapAndGetWithoutRehash)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 100);

    for (jak_u32 key = 0; key < 10; key++) {
        jak_u64 value = key << 2;
        status = jak_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (jak_u32 key = 0; key < 10; key++) {
        const void *value = jak_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(jak_u64 *) value == key << 2);
    }

    jak_hashtable_drop(&map);

}

TEST(FixMapTest, MapAndGetWitRehash)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 10);

    for (jak_u32 key = 0; key < 10000; key++) {
        jak_u64 value = key << 2;
        status = jak_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (jak_u32 key = 0; key < 10000; key++) {
        const void *value = jak_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(jak_u64 *) value == key << 2);
    }

    jak_hashtable_drop(&map);
}

TEST(FixMapTest, DisplaceTest)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 10700);

    for (jak_u32 key = 0; key < 10000; key++) {
        jak_u64 value = key << 2;
        status = jak_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (jak_u32 key = 0; key < 10000; key++) {
        const void *value = jak_hashtable_get_value(&map, &key);
        ASSERT_TRUE(value != NULL);
        ASSERT_TRUE(*(jak_u64 *) value == key << 2);
    }

    float dis;
    jak_hashtable_avg_displace(&dis, &map);
    printf("Avg Displace: %f\n", dis);


    jak_hashtable_drop(&map);
}



TEST(FixMapTest, MapAndGetNotContainedWithoutRehash)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 100);

    for (jak_u32 key = 0; key < 10; key++) {
        jak_u64 value = key << 2;
        status = jak_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (jak_u32 key = 0; key < 10; key++) {
        jak_u32 unknown_key = 10 + key;
        const void *value = jak_hashtable_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    jak_hashtable_drop(&map);
}

TEST(FixMapTest, MapAndGetNotContainedWitRehash)
{
    jak_hashtable map;
    jak_error     err;
    bool             status;

    jak_hashtable_create(&map, &err, sizeof(jak_u32), sizeof(jak_u64), 10);

    for (jak_u32 key = 0; key < 10000; key++) {
        jak_u64 value = key << 2;
        status = jak_hashtable_insert_or_update(&map, &key, &value, 1);
        ASSERT_TRUE(status);
    }

    for (jak_u32 key = 0; key < 10000; key++) {
        jak_u32 unknown_key = 10000 + key;
        const void *value = jak_hashtable_get_value(&map, &unknown_key);
        ASSERT_TRUE(value == NULL);
    }

    jak_hashtable_drop(&map);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}