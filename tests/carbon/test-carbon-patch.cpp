#include <gtest/gtest.h>

#include <jakson/jakson.h>

TEST(TestCarbonPatch, CreatePatch) {

        carbon doc;
        struct err err;
        u64 hash_original, hash_patch_1, hash_patch_2;
        carbon_array_it it, *arr;
        carbon_object_it *obj;
        carbon_field_type_e type;

        carbon_from_json(&doc, "{ \"x\": [1, \"y\", 3] }", CARBON_KEY_AUTOKEY, NULL, &err);
        carbon_commit_hash(&hash_original, &doc);

        char *json_original = carbon_to_json_compact_dup(&doc);

        /* patching via patch iterators */
        {
                carbon_patch_begin(&it, &doc);
                carbon_array_it_next(&it);
                {
                        carbon_array_it_field_type(&type, &it);
                        EXPECT_TRUE(type == CARBON_FIELD_OBJECT_UNSORTED_MULTIMAP);
                        obj = carbon_array_it_object_value(&it);
                        {
                                carbon_object_it_next(obj);
                                carbon_object_it_prop_type(&type, obj);
                                EXPECT_EQ(type, CARBON_FIELD_ARRAY_UNSORTED_MULTISET);
                                arr = carbon_object_it_array_value(obj);
                                {
                                        carbon_array_it_next(arr); /* { ...: [1,...] } */
                                        carbon_array_it_update_in_place_u8(arr, 42);
                                        carbon_array_it_next(arr); /* { ...: [..., "y",...] } */
                                        carbon_array_it_next(arr); /* { ...: [..., ..., 3] } */
                                        carbon_array_it_update_in_place_u8(arr, 23);
                                }
                        }
                }
                carbon_patch_end(&it);
        }

        char *json_patch_1 = carbon_to_json_compact_dup(&doc);
        carbon_commit_hash(&hash_patch_1, &doc);

        /* patching via patch find */
        {
                carbon_find find;
                carbon_patch_find_begin(&find, "x", &doc);
                carbon_array_it *sub_it = FN_GET_PTR(carbon_array_it, carbon_find_result_array(&find));
                carbon_array_it_next(sub_it); /* { ...: [42,...] } */
                carbon_array_it_update_in_place_u8(sub_it, 102);
                carbon_patch_find_end(&find);
        }

        char *json_patch_2 = carbon_to_json_compact_dup(&doc);
        carbon_commit_hash(&hash_patch_2, &doc);

        EXPECT_TRUE(strcmp(json_original, "{\"x\": [1, \"y\", 3]}") == 0);
        EXPECT_TRUE(strcmp(json_patch_1, "{\"x\": [42, \"y\", 23]}") == 0);
        EXPECT_TRUE(strcmp(json_patch_2, "{\"x\": [102, \"y\", 23]}") == 0);

        /* note especially that the commit hash has not changed; so a patch is not a new revision */
        EXPECT_EQ(hash_patch_1, hash_original);
        EXPECT_EQ(hash_patch_2, hash_original);

        carbon_drop(&doc);

        EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}