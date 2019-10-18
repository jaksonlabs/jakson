#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(TestAbstractTypes, ColumnSetAbstractType) {

        carbon_new context;
        carbon_insert *ins;
        carbon doc, doc2;
        carbon_insert_object_state s1;
        carbon_array_it it;
        carbon_object_it *obj_it;
        carbon_field_type_e ft;
        carbon_revise rev_context;

        ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);

        carbon_insert_object_begin(&s1, ins, 100);
        carbon_insert_object_end(&s1);

        carbon_insert_object_map_begin(&s1, ins, CARBON_MAP_UNSORTED_MULTIMAP, 100);
        carbon_insert_object_map_end(&s1);

        carbon_insert_object_map_begin(&s1, ins, CARBON_MAP_SORTED_MULTIMAP, 100);
        carbon_insert_object_map_end(&s1);

        carbon_insert_object_map_begin(&s1, ins, CARBON_MAP_UNSORTED_MAP, 100);
        carbon_insert_object_map_end(&s1);

        carbon_insert_object_map_begin(&s1, ins, CARBON_MAP_SORTED_MAP, 100);
        carbon_insert_object_map_end(&s1);

        carbon_create_end(&context);

        {
                carbon_read_begin(&it, &doc);

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_read_end(&it);
        }

        {
                carbon_revise_begin(&rev_context, &doc2, &doc);
                carbon_revise_iterator_open(&it, &rev_context);

                carbon_array_it_next(&it);
                obj_it = carbon_array_it_object_value(&it);
                carbon_object_it_update_type(obj_it, CARBON_MAP_SORTED_MULTIMAP);

                carbon_array_it_next(&it);
                obj_it = carbon_array_it_object_value(&it);
                carbon_object_it_update_type(obj_it, CARBON_MAP_UNSORTED_MAP);

                carbon_array_it_next(&it);
                obj_it = carbon_array_it_object_value(&it);
                carbon_object_it_update_type(obj_it, CARBON_MAP_SORTED_MAP);

                carbon_array_it_next(&it);
                obj_it = carbon_array_it_object_value(&it);
                carbon_object_it_update_type(obj_it, CARBON_MAP_UNSORTED_MULTIMAP);

                carbon_revise_iterator_close(&it);
                carbon_revise_end(&rev_context);
        }

        {
                carbon_read_begin(&it, &doc2);

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                obj_it = carbon_array_it_object_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_object_it_is_multimap(obj_it)));
                ASSERT_FALSE(FN_STATUS(carbon_object_it_is_sorted(obj_it)));

                carbon_read_end(&it);
        }

        carbon_drop(&doc);
        carbon_drop(&doc2);
}


TEST(TestAbstractTypes, ObjectSetNestedAbstractType) {

        carbon doc, doc2, doc3, doc4, doc5;
        err err;
        carbon_find find;
        carbon_field_type_e ft;
        carbon_revise revise;

        carbon_from_json(&doc, "{ x: [ { y: {\"a\": 1, \"b\": 2, \"c\": 3 } } ] }", CARBON_KEY_NOKEY, NULL, &err);

        {
                carbon_revise_begin(&revise, &doc2, &doc);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                fn_result ofType(carbon_object_it *) find_result = carbon_find_result_object(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_object_type(&find, CARBON_MAP_SORTED_MAP);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc2);
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_object_is_multimap(&find)));
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_object_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc3, &doc2);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                fn_result ofType(carbon_object_it *) find_result = carbon_find_result_object(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_object_type(&find, CARBON_MAP_SORTED_MULTIMAP);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc3);
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_object_is_multimap(&find)));
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_object_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc4, &doc3);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                fn_result ofType(carbon_object_it *) find_result = carbon_find_result_object(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_object_type(&find, CARBON_MAP_UNSORTED_MULTIMAP);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc4);
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_object_is_multimap(&find)));
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_object_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc5, &doc4);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_object_or_subtype(ft));
                fn_result ofType(carbon_object_it *) find_result = carbon_find_result_object(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_object_type(&find, CARBON_MAP_UNSORTED_MAP);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc5);
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_object_is_multimap(&find)));
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_object_is_sorted(&find)));
                carbon_find_end(&find);
        }

        carbon_drop(&doc);
        carbon_drop(&doc2);
        carbon_drop(&doc3);
        carbon_drop(&doc4);
        carbon_drop(&doc5);
}

int main(int argc, char **argv) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}