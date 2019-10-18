#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(TestAbstractTypes, CreateRecordDefaultJsonArray) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, 0);
        carbon_create_end(&context);

        ASSERT_TRUE(FN_STATUS(carbon_is_multiset(&doc)));
        ASSERT_FALSE(FN_STATUS(carbon_is_sorted(&doc)));

        carbon_drop(&doc);
}

TEST(TestAbstractTypes, CreateRecordUnsortedMultiset) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_UNSORTED_MULTISET);
        carbon_create_end(&context);

        ASSERT_TRUE(FN_STATUS(carbon_is_multiset(&doc)));
        ASSERT_FALSE(FN_STATUS(carbon_is_sorted(&doc)));

        carbon_drop(&doc);
}

TEST(TestAbstractTypes, CreateRecordUnsortedSet) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_UNSORTED_SET);
        carbon_create_end(&context);

        ASSERT_FALSE(FN_STATUS(carbon_is_multiset(&doc)));
        ASSERT_FALSE(FN_STATUS(carbon_is_sorted(&doc)));

        carbon_drop(&doc);
}

TEST(TestAbstractTypes, CreateRecordSortedSet) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_SORTED_SET);
        carbon_create_end(&context);

        ASSERT_FALSE(FN_STATUS(carbon_is_multiset(&doc)));
        ASSERT_TRUE(FN_STATUS(carbon_is_sorted(&doc)));

        carbon_drop(&doc);
}

TEST(TestAbstractTypes, CreateRecordSortedMultiset) {

        carbon_new context;
        carbon doc;

        carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_SORTED_MULTISET);
        carbon_create_end(&context);

        ASSERT_TRUE(FN_STATUS(carbon_is_multiset(&doc)));
        ASSERT_TRUE(FN_STATUS(carbon_is_sorted(&doc)));

        carbon_drop(&doc);
}

TEST(TestAbstractTypes, CreateRecordDeriveToDifferentTypes) {

        carbon_new new_context;
        carbon doc, doc2, doc3, doc4, doc5;

        carbon_create_begin(&new_context, &doc, CARBON_KEY_NOKEY, CARBON_UNSORTED_MULTISET);
        carbon_create_end(&new_context);

        {
                carbon_update_list_type(&doc2, &doc, CARBON_LIST_UNSORTED_MULTISET);
                ASSERT_TRUE(FN_STATUS(carbon_is_multiset(&doc2)));
                ASSERT_FALSE(FN_STATUS(carbon_is_sorted(&doc2)));
        }

        {
                carbon_update_list_type(&doc3, &doc2, CARBON_LIST_UNSORTED_SET);
                ASSERT_FALSE(FN_STATUS(carbon_is_multiset(&doc3)));
                ASSERT_FALSE(FN_STATUS(carbon_is_sorted(&doc3)));
        }

        {
                carbon_update_list_type(&doc4, &doc3, CARBON_LIST_SORTED_MULTISET);
                ASSERT_TRUE(FN_STATUS(carbon_is_multiset(&doc4)));
                ASSERT_TRUE(FN_STATUS(carbon_is_sorted(&doc4)));
        }

        {
                carbon_update_list_type(&doc5, &doc4, CARBON_LIST_SORTED_SET);
                ASSERT_FALSE(FN_STATUS(carbon_is_multiset(&doc5)));
                ASSERT_TRUE(FN_STATUS(carbon_is_sorted(&doc5)));
        }

        carbon_drop(&doc);
        carbon_drop(&doc2);
        carbon_drop(&doc3);
        carbon_drop(&doc4);
        carbon_drop(&doc5);
}

TEST(TestAbstractTypes, ArraySetAbstractType) {

        carbon_new context;
        carbon_insert *ins;
        carbon doc, doc2;
        carbon_insert_array_state s1;
        carbon_array_it it, *sub_it;
        carbon_field_type_e ft;
        carbon_revise rev_context;

        ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);

        carbon_insert_array_begin(&s1, ins, 100);
        carbon_insert_array_end(&s1);

        carbon_insert_array_list_begin(&s1, ins, CARBON_LIST_UNSORTED_MULTISET, 100);
        carbon_insert_array_list_end(&s1);

        carbon_insert_array_list_begin(&s1, ins, CARBON_LIST_SORTED_MULTISET, 100);
        carbon_insert_array_list_end(&s1);

        carbon_insert_array_list_begin(&s1, ins, CARBON_LIST_UNSORTED_SET, 100);
        carbon_insert_array_list_end(&s1);

        carbon_insert_array_list_begin(&s1, ins, CARBON_LIST_SORTED_SET, 100);
        carbon_insert_array_list_end(&s1);

        carbon_create_end(&context);

        {
                carbon_read_begin(&it, &doc);

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_read_end(&it);
        }

        {
                carbon_revise_begin(&rev_context, &doc2, &doc);
                carbon_revise_iterator_open(&it, &rev_context);

                carbon_array_it_next(&it);
                sub_it = carbon_array_it_array_value(&it);
                carbon_array_it_update_type(sub_it, CARBON_LIST_SORTED_MULTISET);

                carbon_array_it_next(&it);
                sub_it = carbon_array_it_array_value(&it);
                carbon_array_it_update_type(sub_it, CARBON_LIST_UNSORTED_SET);

                carbon_array_it_next(&it);
                sub_it = carbon_array_it_array_value(&it);
                carbon_array_it_update_type(sub_it, CARBON_LIST_SORTED_SET);

                carbon_array_it_next(&it);
                sub_it = carbon_array_it_array_value(&it);
                carbon_array_it_update_type(sub_it, CARBON_LIST_UNSORTED_MULTISET);

                carbon_revise_iterator_close(&it);
                carbon_revise_end(&rev_context);
        }

        {
                carbon_read_begin(&it, &doc2);

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_array_it_next(&it);
                carbon_array_it_field_type(&ft, &it);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                sub_it = carbon_array_it_array_value(&it);
                ASSERT_TRUE(FN_STATUS(carbon_array_it_is_multiset(sub_it)));
                ASSERT_FALSE(FN_STATUS(carbon_array_it_is_sorted(sub_it)));

                carbon_read_end(&it);
        }

        carbon_drop(&doc);
        carbon_drop(&doc2);
}

TEST(TestAbstractTypes, ArraySetNestedAbstractType) {

        carbon doc, doc2, doc3, doc4, doc5;
        err err;
        carbon_find find;
        carbon_field_type_e ft;
        carbon_revise revise;

        carbon_from_json(&doc, "{ x: [ { y: [1,\"b\",3] } ] }", CARBON_KEY_NOKEY, NULL, &err);

        {
                carbon_revise_begin(&revise, &doc2, &doc);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                fn_result ofType(carbon_array_it *) find_result = carbon_find_result_array(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_array_type(&find, CARBON_LIST_SORTED_SET);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc2);
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_array_is_multiset(&find)));
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_array_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc3, &doc2);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                fn_result ofType(carbon_array_it *) find_result = carbon_find_result_array(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_array_type(&find, CARBON_LIST_SORTED_MULTISET);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc3);
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_array_is_multiset(&find)));
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_array_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc4, &doc3);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                fn_result ofType(carbon_array_it *) find_result = carbon_find_result_array(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_array_type(&find, CARBON_LIST_UNSORTED_MULTISET);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc4);
                ASSERT_TRUE(FN_GET_BOOL(carbon_find_array_is_multiset(&find)));
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_array_is_sorted(&find)));
                carbon_find_end(&find);
        }

        {
                carbon_revise_begin(&revise, &doc5, &doc4);
                carbon_revise_find_begin(&find, "x.0.y", &revise);
                carbon_find_result_type(&ft, &find);
                ASSERT_TRUE(carbon_field_type_is_array_or_subtype(ft));
                fn_result ofType(carbon_array_it *) find_result = carbon_find_result_array(&find);
                ASSERT_TRUE(FN_STATUS(find_result));
                carbon_find_update_array_type(&find, CARBON_LIST_UNSORTED_SET);
                carbon_revise_find_end(&find);
                carbon_revise_end(&revise);

                carbon_find_begin(&find, "x.0.y", &doc5);
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_array_is_multiset(&find)));
                ASSERT_FALSE(FN_GET_BOOL(carbon_find_array_is_sorted(&find)));
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