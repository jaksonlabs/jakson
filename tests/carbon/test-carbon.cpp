#include <gtest/gtest.h>

#include <ark-js/carbon/carbon.h>
#include <ark-js/carbon/carbon-dot.h>
#include <ark-js/carbon/carbon-array-it.h>
#include <ark-js/carbon/carbon-insert.h>
#include <ark-js/carbon/carbon-find.h>
#include <ark-js/carbon/carbon-update.h>
#include <ark-js/carbon/carbon-path.h>
#include <ark-js/carbon/carbon-get.h>
#include <ark-js/carbon/carbon-revise.h>
#include <ark-js/carbon/carbon-object-it.h>
//
//TEST(CarbonTest, CreateBison) {
//        struct carbon doc;
//        object_id_t oid;
//        u64 rev;
//        struct string_builder builder;
//        bool status;
//
//        string_builder_create(&builder);
//
//        status = carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//        EXPECT_TRUE(status);
//
//        //carbon_hexdump_print(stderr, &doc);
//
//        status = carbon_key_unsigned_value(&oid, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(oid, 0);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        carbon_to_str(&builder, JSON_FORMATTER, &doc);
//        // printf("%s\n", string_builder_cstr(&builder));
//        string_builder_drop(&builder);
//
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, CreateBisonRevisionNumbering) {
//        struct carbon doc, rev_doc;
//        u64 rev;
//        struct string_builder builder;
//        bool status;
//
//        string_builder_create(&builder);
//
//        status = carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//        EXPECT_TRUE(status);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        struct carbon_revise revise;
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        status = carbon_revision(&rev, &rev_doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 1);
//
//        status = carbon_is_up_to_date(&doc);
//        EXPECT_FALSE(status);
//
//        status = carbon_is_up_to_date(&rev_doc);
//        EXPECT_TRUE(status);
//
//        carbon_to_str(&builder, JSON_FORMATTER, &doc);
//        // printf("%s\n", string_builder_cstr(&builder));
//        string_builder_drop(&builder);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, CreateBisonRevisionAbort) {
//        struct carbon doc, rev_doc;
//        u64 rev;
//        struct string_builder builder;
//        bool status;
//
//        string_builder_create(&builder);
//
//        status = carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//        EXPECT_TRUE(status);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        struct carbon_revise revise;
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_abort(&revise);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        carbon_to_str(&builder, JSON_FORMATTER, &doc);
//        // printf("%s\n", string_builder_cstr(&builder));
//        string_builder_drop(&builder);
//
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, CreateBisonRevisionAsyncReading) {
//        struct carbon doc, rev_doc;
//        u64 rev;
//        struct string_builder builder;
//        bool status;
//
//        string_builder_create(&builder);
//
//        status = carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//        EXPECT_TRUE(status);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        struct carbon_revise revise;
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        carbon_revise_end(&revise);
//
//        status = carbon_revision(&rev, &doc);
//        EXPECT_TRUE(status);
//        EXPECT_EQ(rev, 0);
//
//        carbon_to_str(&builder, JSON_FORMATTER, &doc);
//        // printf("%s\n", string_builder_cstr(&builder));
//        string_builder_drop(&builder);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, ForceBisonRevisionVarLengthIncrease) {
//        struct carbon doc, rev_doc;
//        u64 old_rev;
//        u64 new_rev;
//        bool status;
//        struct carbon_revise revise;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        for (u32 i = 0; i < 20000; i++) {
//                status = carbon_revision(&old_rev, &doc);
//
//                carbon_revise_begin(&revise, &rev_doc, &doc);
//                carbon_revise_end(&revise);
//
//                status = carbon_revision(&new_rev, &doc);
//                EXPECT_TRUE(status);
//                EXPECT_EQ(new_rev, old_rev);
//
//                status = carbon_revision(&new_rev, &rev_doc);
//                EXPECT_TRUE(status);
//                EXPECT_EQ(new_rev, old_rev + 1);
//
//                // carbon_print(stdout, &rev_doc);
//
//                carbon_drop(&doc);
//                carbon_clone(&doc, &rev_doc);
//                carbon_drop(&rev_doc);
//        }
//
//
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, ModifyBisonObjectId) {
//        struct carbon doc, rev_doc;
//        object_id_t oid;
//        object_id_t new_oid;
//        struct carbon_revise revise;
//        u64 rev;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_key_unsigned_value(&oid, &doc);
//        EXPECT_EQ(oid, 0);
//
//        carbon_revision(&rev, &doc);
//        EXPECT_EQ(rev, 0);
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_generate(&new_oid, &revise);
//        EXPECT_NE(oid, new_oid);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev, &rev_doc);
//        EXPECT_NE(rev, 0);
//
//        carbon_key_unsigned_value(&oid, &rev_doc);
//        EXPECT_EQ(oid, new_oid);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonArrayIteratorOpenAfterNew) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_generate(NULL, &revise);
//        carbon_revise_iterator_open(&it, &revise);
//        bool has_next = carbon_array_it_next(&it);
//        EXPECT_EQ(has_next, false);
//        carbon_revise_end(&revise);
//        carbon_array_it_drop(&it);
//
//        // carbon_print(stdout, &rev_doc);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}

//TEST(CarbonTest, BisonArrayIteratorInsertNullAfterNew) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_revise_key_generate(NULL, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        carbon_insert_null(&inserter);
//        carbon_insert_drop(&inserter);
//        carbon_revise_end(&revise);
//        carbon_array_it_drop(&it);
//
//        // carbon_print(stdout, &rev_doc);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonArrayIteratorInsertMultipleLiteralsAfterNewNoOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 10; i++) {
//                // fprintf(stdout, "before:\n");
//                //carbon_hexdump_print(stdout, &rev_doc);
//                bool status;
//                if (i % 3 == 0) {
//                        status = carbon_insert_null(&inserter);
//                } else if (i % 3 == 1) {
//                        status = carbon_insert_true(&inserter);
//                } else {
//                        status = carbon_insert_false(&inserter);
//                }
//                ASSERT_TRUE(status);
//                // fprintf(stdout, "after:\n");
//                //carbon_hexdump_print(stdout, &rev_doc);
//                // fprintf(stdout, "\n\n");
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonArrayIteratorOverwriteLiterals) {
//        struct carbon doc, rev_doc, rev_doc2;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 3; i++) {
//                if (i % 3 == 0) {
//                        carbon_insert_null(&inserter);
//                } else if (i % 3 == 1) {
//                        carbon_insert_true(&inserter);
//                } else {
//                        carbon_insert_false(&inserter);
//                }
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 2; i++) {
//                carbon_insert_true(&inserter);
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc2);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//}
//
//TEST(CarbonTest, BisonArrayIteratorOverwriteLiteralsWithDocOverflow) {
//        struct carbon doc, rev_doc, rev_doc2;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 22; i++) {
//                if (i % 3 == 0) {
//                        carbon_insert_null(&inserter);
//                } else if (i % 3 == 1) {
//                        carbon_insert_true(&inserter);
//                } else {
//                        carbon_insert_false(&inserter);
//                }
//               // fprintf(stdout, "after initial push:\n");
//               // //carbon_hexdump_print(stdout, &rev_doc);
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 2; i++) {
//                // fprintf(stdout, "before:\n");
//                //carbon_hexdump_print(stdout, &rev_doc2);
//                carbon_insert_true(&inserter);
//                // fprintf(stdout, "after:\n");
//                //carbon_hexdump_print(stdout, &rev_doc2);
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//        // carbon_print(stdout, &rev_doc2);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//}
//
//TEST(CarbonTest, BisonArrayIteratorUnsignedAndConstants) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 500; i++) {
//                if (i % 6 == 0) {
//                        carbon_insert_null(&inserter);
//                } else if (i % 6 == 1) {
//                        carbon_insert_true(&inserter);
//                } else if (i % 6 == 2) {
//                        carbon_insert_false(&inserter);
//                } else if (i % 6 == 3) {
//                        u64 rand_value = random();
//                        carbon_insert_unsigned(&inserter, rand_value);
//                } else if (i % 6 == 4) {
//                        i64 rand_value = random();
//                        carbon_insert_signed(&inserter, rand_value);
//                } else {
//                        float rand_value = (float)rand()/(float)(RAND_MAX/INT32_MAX);
//                        carbon_insert_float(&inserter, rand_value);
//                }
//                //fprintf(stdout, "after initial push:\n");
//                ////carbon_hexdump_print(stdout, &rev_doc);
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonArrayIteratorStrings) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        for (i32 i = 0; i < 10; i++) {
//                u64 strlen = rand() % (100 + 1 - 4) + 4;
//                char buffer[strlen];
//                for (i32 j = 0; j < strlen; j++) {
//                        buffer[j] = 65 + (rand() % 25);
//                }
//                buffer[0] = '!';
//                buffer[strlen - 2] = '!';
//                buffer[strlen - 1] = '\0';
//                carbon_insert_string(&inserter, buffer);
//                //fprintf(stdout, "after initial push:\n");
//                ////carbon_hexdump_print(stdout, &rev_doc);
//        }
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertMimeTypedBlob) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        const char *data = "{ \"Message\": \"Hello World\" }";
//        bool status = carbon_insert_binary(&inserter, data, strlen(data), "json", NULL);
//        ASSERT_TRUE(status);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertCustomTypedBlob) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        const char *data = "{ \"Message\": \"Hello World\" }";
//        bool status = carbon_insert_binary(&inserter, data, strlen(data), NULL, "my data");
//        ASSERT_TRUE(status);
//        ////carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertTwoMimeTypedBlob) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        const char *data1 = "{ \"Message\": \"Hello World\" }";
//        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
//        bool status = carbon_insert_binary(&inserter, data1, strlen(data1), "json", NULL);
//        ASSERT_TRUE(status);
//        status = carbon_insert_binary(&inserter, data2, strlen(data2), "txt", NULL);
//        ASSERT_TRUE(status);
//        ////carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertMimeTypedBlobsWithOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        const char *data1 = "{ \"Message\": \"Hello World\" }";
//        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
//        for (u32 i = 0; i < 100; i++) {
//                bool status = carbon_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
//                        strlen(i % 2 == 0 ? data1 : data2), "json", NULL);
//                ASSERT_TRUE(status);
//        }
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertMixedTypedBlobsWithOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        const char *data1 = "{ \"Message\": \"Hello World\" }";
//        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
//        for (u32 i = 0; i < 100; i++) {
//                bool status = carbon_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
//                        strlen(i % 2 == 0 ? data1 : data2), i % 3 == 0 ? "json" : NULL, i % 5 == 0 ? "user/app" : NULL);
//                ASSERT_TRUE(status);
//        }
//        ////carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertArrayWithNoOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        struct carbon_insert *nested_inserter = carbon_insert_array_begin(&array_state, &inserter, 10);
//        ASSERT_TRUE(nested_inserter != NULL);
//        carbon_insert_array_end(&array_state);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertValuesIntoNestedArrayWithNoOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//
//        struct carbon_insert *nested_inserter = carbon_insert_array_begin(&array_state, &inserter, 10);
//        ASSERT_TRUE(nested_inserter != NULL);
//        carbon_insert_true(nested_inserter);
//        carbon_insert_true(nested_inserter);
//        carbon_insert_true(nested_inserter);
//        carbon_insert_array_end(&array_state);
//
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsert2xNestedArrayWithNoOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state_l1, array_state_l2;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_array_begin(&array_state_l1, &inserter, 10);
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        carbon_insert_true(nested_inserter_l1);
//        carbon_insert_true(nested_inserter_l1);
//        carbon_insert_true(nested_inserter_l1);
//
//        struct carbon_insert *nested_inserter_l2 = carbon_insert_array_begin(&array_state_l2, nested_inserter_l1, 10);
//        ASSERT_TRUE(nested_inserter_l2 != NULL);
//        carbon_insert_true(nested_inserter_l2);
//        carbon_insert_false(nested_inserter_l2);
//        carbon_insert_null(nested_inserter_l2);
//        carbon_insert_array_end(&array_state_l2);
//
//        carbon_insert_array_end(&array_state_l1);
//
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertXxNestedArrayWithoutOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state_l1;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//        carbon_insert_null(&inserter);
//
//        for (int i = 0; i < 10; i++) {
//                struct carbon_insert *nested_inserter_l1 = carbon_insert_array_begin(&array_state_l1, &inserter, 10);
//                ASSERT_TRUE(nested_inserter_l1 != NULL);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_array_end(&array_state_l1);
//        }
//
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertXxNestedArrayWithOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state_l1;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        // printf("\n");
//
//        carbon_insert_null(&inserter);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        // printf("\n");
//
//        carbon_insert_null(&inserter);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        // printf("\n");
//
//        carbon_insert_null(&inserter);
//
//        for (int i = 0; i < 10; i++) {
//                struct carbon_insert *nested_inserter_l1 = carbon_insert_array_begin(&array_state_l1, &inserter, 1);
//                ASSERT_TRUE(nested_inserter_l1 != NULL);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_true(nested_inserter_l1);
//                carbon_insert_array_end(&array_state_l1);
//        }
//
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//        carbon_insert_false(&inserter);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertColumnWithoutOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        carbon_insert_u8(nested_inserter_l1, 1);
//        carbon_insert_u8(nested_inserter_l1, 2);
//        carbon_insert_u8(nested_inserter_l1, 3);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertColumnNumbersWithoutOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        carbon_insert_u8(nested_inserter_l1, 42);
//        carbon_insert_u8(nested_inserter_l1, 43);
//        carbon_insert_u8(nested_inserter_l1, 44);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[42, 43, 44]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertColumnNumbersZeroWithoutOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        carbon_insert_u8(nested_inserter_l1, 0);
//        carbon_insert_u8(nested_inserter_l1, 0);
//        carbon_insert_u8(nested_inserter_l1, 0);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[0, 0, 0]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertMultileTypedColumnsWithoutOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U16, 10);
//        carbon_insert_u16(ins, 4);
//        carbon_insert_u16(ins, 5);
//        carbon_insert_u16(ins, 6);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(ins, 7);
//        carbon_insert_u32(ins, 8);
//        carbon_insert_u32(ins, 9);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U64, 10);
//        carbon_insert_u64(ins, 10);
//        carbon_insert_u64(ins, 11);
//        carbon_insert_u64(ins, 12);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I8, 10);
//        carbon_insert_i8(ins, -1);
//        carbon_insert_i8(ins, -2);
//        carbon_insert_i8(ins, -3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I16, 10);
//        carbon_insert_i16(ins, -4);
//        carbon_insert_i16(ins, -5);
//        carbon_insert_i16(ins, -6);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I32, 10);
//        carbon_insert_i32(ins, -7);
//        carbon_insert_i32(ins, -8);
//        carbon_insert_i32(ins, -9);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I64, 10);
//        carbon_insert_i64(ins, -10);
//        carbon_insert_i64(ins, -11);
//        carbon_insert_i64(ins, -12);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_FLOAT, 10);
//        carbon_insert_float(ins, 42.0f);
//        carbon_insert_float(ins, 21.0f);
//        carbon_insert_float(ins, 23.4221f);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        //carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        //string_builder_print(&sb);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12], [-1, -2, -3], [-4, -5, -6], [-7, -8, -9], [-10, -11, -12], [42.00, 21.00, 23.42]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertColumnNumbersZeroWithOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,16, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 1);
//
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        carbon_insert_u8(nested_inserter_l1, 1);
//        carbon_insert_u8(nested_inserter_l1, 2);
//        carbon_insert_u8(nested_inserter_l1, 3);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // printf("BISON DOC PRINT:");
//        // carbon_print(stdout, &rev_doc);
//        // fflush(stdout);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertColumnNumbersWithHighOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,16, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U32, 1);
//
//        ASSERT_TRUE(nested_inserter_l1 != NULL);
//        for (u32 i = 0; i < 100; i++) {
//                carbon_insert_u32(nested_inserter_l1, i);
//                carbon_insert_u32(nested_inserter_l1, i);
//                carbon_insert_u32(nested_inserter_l1, i);
//        }
//
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // printf("BISON DOC PRINT:");
//        // carbon_print(stdout, &rev_doc);
//        // fflush(stdout);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 82, 82, 82, 83, 83, 83, 84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 88, 88, 88, 89, 89, 89, 90, 90, 90, 91, 91, 91, 92, 92, 92, 93, 93, 93, 94, 94, 94, 95, 95, 95, 96, 96, 96, 97, 97, 97, 98, 98, 98, 99, 99, 99]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertInsertMultipleColumnsNumbersWithHighOverflow) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,16, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        for (u32 k = 0; k < 3; k++) {
//                struct carbon_insert *nested_inserter_l1 = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U32, 1);
//
//                ASSERT_TRUE(nested_inserter_l1 != NULL);
//                for (u32 i = 0; i < 4; i++) {
//                        carbon_insert_u32(nested_inserter_l1, 'a' + i);
//                        carbon_insert_u32(nested_inserter_l1, 'a' + i);
//                        carbon_insert_u32(nested_inserter_l1, 'a' + i);
//                }
//
//                carbon_insert_column_end(&column_state);
//        }
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        ////carbon_hexdump_print(stdout, &rev_doc);
//
//        // printf("BISON DOC PRINT:");
//        // carbon_print(stdout, &rev_doc);
//        // fflush(stdout);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonInsertNullTest) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, U8_NULL);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U16, 10);
//        carbon_insert_u16(ins, 4);
//        carbon_insert_u16(ins, U16_NULL);
//        carbon_insert_u16(ins, 6);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(ins, 7);
//        carbon_insert_u32(ins, U32_NULL);
//        carbon_insert_u32(ins, 9);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U64, 10);
//        carbon_insert_u64(ins, 10);
//        carbon_insert_u64(ins, U64_NULL);
//        carbon_insert_u64(ins, 12);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I8, 10);
//        carbon_insert_i8(ins, -1);
//        carbon_insert_i8(ins, I8_NULL);
//        carbon_insert_i8(ins, -3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I16, 10);
//        carbon_insert_i16(ins, -4);
//        carbon_insert_i16(ins, I16_NULL);
//        carbon_insert_i16(ins, -6);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I32, 10);
//        carbon_insert_i32(ins, -7);
//        carbon_insert_i32(ins, I32_NULL);
//        carbon_insert_i32(ins, -9);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I64, 10);
//        carbon_insert_i64(ins, -10);
//        carbon_insert_i64(ins, I64_NULL);
//        carbon_insert_i64(ins, -12);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_FLOAT, 10);
//        carbon_insert_float(ins, 42.0f);
//        carbon_insert_float(ins, FLOAT_NULL);
//        carbon_insert_float(ins, 23.4221f);
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, null, 3], [4, null, 6], [7, null, 9], [10, null, 12], [-1, null, -3], [-4, null, -6], [-7, null, -9], [-10, null, -12], [42.00, null, 23.42]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonShrinkColumnListTest) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_true(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_BOOLEAN, 10);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_false(ins);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U8, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, U8_NULL);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U16, 10);
//        carbon_insert_u16(ins, 3);
//        carbon_insert_u16(ins, U16_NULL);
//        carbon_insert_u16(ins, 4);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(ins, 5);
//        carbon_insert_u32(ins, U32_NULL);
//        carbon_insert_u32(ins, 6);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_U64, 10);
//        carbon_insert_u64(ins, 7);
//        carbon_insert_u64(ins, U64_NULL);
//        carbon_insert_u64(ins, 8);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I8, 10);
//        carbon_insert_i8(ins, 9);
//        carbon_insert_i8(ins, I8_NULL);
//        carbon_insert_i8(ins, 10);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I16, 10);
//        carbon_insert_i16(ins, 11);
//        carbon_insert_i16(ins, I16_NULL);
//        carbon_insert_i16(ins, 12);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I32, 10);
//        carbon_insert_i32(ins, 13);
//        carbon_insert_i32(ins, I32_NULL);
//        carbon_insert_i32(ins, 14);
//        carbon_insert_column_end(&column_state);
//
//        ins = carbon_insert_column_begin(&column_state, &inserter, CARBON_COLUMN_TYPE_I64, 10);
//        carbon_insert_i64(ins, 15);
//        carbon_insert_i64(ins, I64_NULL);
//        carbon_insert_i64(ins, 16);
//        carbon_insert_column_end(&column_state);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_revise_shrink(&revise);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, null, 2], [3, null, 4], [5, null, 6], [7, null, 8], [9, null, 10], [11, null, 12], [13, null, 14], [15, null, 16]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonShrinkArrayListTest) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state;
//        struct carbon_insert *ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_array_end(&array_state);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 2);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_u8(ins, 4);
//        carbon_insert_array_end(&array_state);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 5);
//        carbon_insert_u8(ins, 6);
//        carbon_insert_u8(ins, 7);
//        carbon_insert_array_end(&array_state);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_revise_shrink(&revise);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 1, 1], [2, 3, 4], [5, 6, 7]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonShrinkNestedArrayListTest) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_array_state array_state, nested_array_state;
//        struct carbon_insert *ins, *nested_ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
//        carbon_insert_string(nested_ins, "Hello");
//        carbon_insert_string(nested_ins, "World");
//        carbon_insert_array_end(&nested_array_state);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_u8(ins, 1);
//        carbon_insert_array_end(&array_state);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 2);
//        nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
//        carbon_insert_string(nested_ins, "Hello");
//        carbon_insert_string(nested_ins, "World");
//        carbon_insert_array_end(&nested_array_state);
//        carbon_insert_u8(ins, 3);
//        carbon_insert_u8(ins, 4);
//        carbon_insert_array_end(&array_state);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 5);
//        carbon_insert_u8(ins, 6);
//        nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
//        carbon_insert_string(nested_ins, "Hello");
//        carbon_insert_string(nested_ins, "World");
//        carbon_insert_array_end(&nested_array_state);
//        carbon_insert_u8(ins, 7);
//        carbon_insert_array_end(&array_state);
//
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//        carbon_insert_u8(ins, 8);
//        carbon_insert_u8(ins, 9);
//        carbon_insert_u8(ins, 10);
//        nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
//        carbon_insert_string(nested_ins, "Hello");
//        carbon_insert_string(nested_ins, "World");
//        carbon_insert_array_end(&nested_array_state);
//        carbon_insert_array_end(&array_state);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_revise_shrink(&revise);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[[\"Hello\", \"World\"], 1, 1, 1], [2, [\"Hello\", \"World\"], 3, 4], [5, 6, [\"Hello\", \"World\"], 7], [8, 9, 10, [\"Hello\", \"World\"]]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonShrinkNestedArrayListAndColumnListTest) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_array_state array_state, nested_array_state;
//        struct carbon_insert *ins, *nested_ins, *column_ins;
//
//        carbon_create_empty_ex(&doc, CARBON_KEY_AUTOKEY,20, 1);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_u64(&inserter, 4223);
//        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
//                column_ins = carbon_insert_column_begin(&column_state, ins, CARBON_COLUMN_TYPE_U32, 10);
//                        carbon_insert_u32(column_ins, 'X');
//                        carbon_insert_u32(column_ins, 'Y');
//                        carbon_insert_u32(column_ins, 'Z');
//                carbon_insert_column_end(&column_state);
//                nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
//                        carbon_insert_string(nested_ins, "Hello");
//                        column_ins = carbon_insert_column_begin(&column_state, nested_ins, CARBON_COLUMN_TYPE_U32, 10);
//                                carbon_insert_u32(column_ins, 'A');
//                                carbon_insert_u32(column_ins, 'B');
//                                carbon_insert_u32(column_ins, 'C');
//                        carbon_insert_column_end(&column_state);
//                        carbon_insert_string(nested_ins, "World");
//                carbon_insert_array_end(&nested_array_state);
//                carbon_insert_u8(ins, 1);
//                carbon_insert_u8(ins, 1);
//                column_ins = carbon_insert_column_begin(&column_state, ins, CARBON_COLUMN_TYPE_U32, 10);
//                        carbon_insert_u32(column_ins, 23);
//                        carbon_insert_u32(column_ins, 24);
//                        carbon_insert_u32(column_ins, 25);
//                carbon_insert_column_end(&column_state);
//                carbon_insert_u8(ins, 1);
//        carbon_insert_array_end(&array_state);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//        carbon_revise_shrink(&revise);
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        carbon_insert_drop(&inserter);
//        carbon_array_it_drop(&it);
//        carbon_revise_end(&revise);
//
//        //carbon_hexdump_print(stdout, &rev_doc);
//
//        // carbon_print(stdout, &rev_doc);
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//        carbon_to_str(&sb, JSON_FORMATTER, &rev_doc);
//
//        // fprintf(stdout, "IST  %s\n", string_builder_cstr(&sb));
//        // fprintf(stdout, "SOLL {\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}\n");
//
//        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}"));
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonDotNotation) {
//        struct carbon_dot_path path;
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        carbon_dot_path_create(&path);
//
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_key(&path, "name");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_key(&path, "my name");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\"") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_key(&path, "");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\"") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_idx(&path, 42);
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_idx(&path, 23);
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42.23") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_add_key(&path, "\"already quotes\"");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42.23.\"already quotes\"") == 0);
//        string_builder_clear(&sb);
//
//        carbon_dot_path_drop(&path);
//        string_builder_drop(&sb);
//}
//
//TEST(CarbonTest, BisonDotNotationParsing) {
//        struct carbon_dot_path path;
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        carbon_dot_path_from_string(&path, "name");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "   name");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "   name    ");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "\"name\"");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "\"nam e\"");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "\"nam e\"") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "nam e");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "nam.e") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "\"My Doc\" names 5 age");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "\"My Doc\".names.5.age") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        carbon_dot_path_from_string(&path, "23.authors.3.name");
//        carbon_dot_path_to_str(&sb, &path);
//        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "23.authors.3.name") == 0);
//        string_builder_clear(&sb);
//        carbon_dot_path_drop(&path);
//
//        string_builder_drop(&sb);
//}
//
//TEST(CarbonTest, BisonFind) {
//        struct carbon doc, rev_doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert ins;
//        struct carbon_find finder;
//        u64 result_unsigned;
//        enum carbon_field_type type;
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&ins, &it);
//        carbon_insert_u8(&ins, 'a');
//        carbon_insert_u8(&ins, 'b');
//        carbon_insert_u8(&ins, 'c');
//        carbon_array_it_insert_end(&ins);
//        carbon_revise_iterator_close(&it);
//
//        carbon_revise_end(&revise);
//
//        {
//                carbon_find_open(&finder, "0", &rev_doc);
//
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 'a');
//
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1", &rev_doc);
//
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 'b');
//
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "2", &rev_doc);
//
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 'c');
//
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "3", &rev_doc);
//
//                ASSERT_FALSE(carbon_find_has_result(&finder));
//
//                carbon_find_close(&finder);
//        }
//
//        // carbon_print(stdout, &rev_doc);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}

TEST(CarbonTest, BisonFindTypes) {
        struct carbon doc, rev_doc;
        struct carbon_revise revise;
        struct carbon_array_it it;
        struct carbon_insert inserter, *ins, *nested_ins, *column_ins;
        struct carbon_insert_column_state column_state;
        struct carbon_insert_array_state array_state, nested_array_state;
        struct carbon_find finder;
        u64 result_unsigned;
        enum carbon_field_type type;
        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        carbon_array_it_insert_begin(&inserter, &it);

        carbon_insert_u64(&inserter, 4223);
        ins = carbon_insert_array_begin(&array_state, &inserter, 10);
        column_ins = carbon_insert_column_begin(&column_state, ins, CARBON_COLUMN_TYPE_U32, 10);
        carbon_insert_u32(column_ins, 'X');
        carbon_insert_u32(column_ins, 'Y');
        carbon_insert_u32(column_ins, 'Z');
        carbon_insert_column_end(&column_state);
        nested_ins = carbon_insert_array_begin(&nested_array_state, ins, 10);
        carbon_insert_string(nested_ins, "Hello");
        column_ins = carbon_insert_column_begin(&column_state, nested_ins, CARBON_COLUMN_TYPE_U32, 10);
        carbon_insert_u32(column_ins, 'A');
        carbon_insert_u32(column_ins, 'B');
        carbon_insert_u32(column_ins, 'C');
        carbon_insert_column_end(&column_state);
        carbon_insert_string(nested_ins, "World");
        carbon_insert_array_end(&nested_array_state);
        carbon_insert_u8(ins, 1);
        carbon_insert_u8(ins, 1);
        column_ins = carbon_insert_column_begin(&column_state, ins, CARBON_COLUMN_TYPE_U32, 10);
        carbon_insert_u32(column_ins, 23);
        carbon_insert_u32(column_ins, 24);
        carbon_insert_u32(column_ins, 25);
        carbon_insert_column_end(&column_state);
        carbon_insert_u8(ins, 1);
        carbon_insert_array_end(&array_state);

        carbon_revise_shrink(&revise);


        //carbon_print(stdout, &rev_doc);

        {
                carbon_find_open(&finder, "0", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U64);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 4223);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_ARRAY);
                struct carbon_array_it *retval = carbon_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.0", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_TRUE(type == CARBON_FIELD_TYPE_COLUMN_U8 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U16 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U32 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U64 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I8 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I16 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I32 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I64 ||
                        type == CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                        type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                struct carbon_column_it *retval = carbon_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.0.0", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 88);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.0.1", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 89);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.0.2", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 90);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.0.3", &rev_doc);
                ASSERT_FALSE(carbon_find_has_result(&finder));
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_ARRAY);
                struct carbon_array_it *retval = carbon_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.0", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_STRING);
                u64 str_len;
                const char *retval = carbon_find_result_string(&str_len, &finder);
                ASSERT_TRUE(strncmp(retval, "Hello", str_len) == 0);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.1", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_TRUE(type == CARBON_FIELD_TYPE_COLUMN_U8 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U16 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U32 ||
                        type == CARBON_FIELD_TYPE_COLUMN_U64 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I8 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I16 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I32 ||
                        type == CARBON_FIELD_TYPE_COLUMN_I64 ||
                        type == CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                        type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                struct carbon_column_it *retval = carbon_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.1.0", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 65);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.1.1", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 66);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.1.2", &rev_doc);
                ASSERT_TRUE(carbon_find_has_result(&finder));
                carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
                carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 67);
                carbon_find_close(&finder);
        }

        {
                carbon_find_open(&finder, "1.1.1.3", &rev_doc);
                ASSERT_FALSE(carbon_find_has_result(&finder));
        }

//        {
//                carbon_find_open(&finder, "1.1.2", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_STRING);
//                u64 str_len;
//                const char *retval = carbon_find_result_string(&str_len, &finder);
//                ASSERT_TRUE(strncmp(retval, "World", str_len) == 0);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.1.3", &rev_doc);
//                ASSERT_FALSE(carbon_find_has_result(&finder));
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.2", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 1);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.3", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 1);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.4", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_TRUE(type == CARBON_FIELD_TYPE_COLUMN_U8 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_U16 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_U32 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_U64 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_I8 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_I16 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_I32 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_I64 ||
//                        type == CARBON_FIELD_TYPE_COLUMN_FLOAT ||
//                        type == CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
//                struct carbon_column_it *retval = carbon_find_result_column(&finder);
//                ASSERT_TRUE(retval != NULL);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.4.0", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 23);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.4.1", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 24);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.4.2", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U32);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 25);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.4.3", &rev_doc);
//                ASSERT_FALSE(carbon_find_has_result(&finder));
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.5", &rev_doc);
//                ASSERT_TRUE(carbon_find_has_result(&finder));
//                carbon_find_result_type(&type, &finder);
//                ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U8);
//                carbon_find_result_unsigned(&result_unsigned, &finder);
//                ASSERT_EQ(result_unsigned, 1);
//                carbon_find_close(&finder);
//        }
//
//        {
//                carbon_find_open(&finder, "1.6", &rev_doc);
//                ASSERT_FALSE(carbon_find_has_result(&finder));
//                carbon_find_close(&finder);
//        }

        carbon_insert_drop(&inserter);
        carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //carbon_hexdump_print(stdout, &rev_doc);

        carbon_drop(&rev_doc);
        carbon_drop(&doc);
}
//
//TEST(CarbonTest, BisonUpdateU8Simple)
//{
//        struct carbon doc, rev_doc, rev_doc2, rev_doc3, rev_doc4;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct string_builder sb;
//        const char *json;
//
//        string_builder_create(&sb);
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_u8(&inserter, 'X');
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc);
//        // printf("JSON (rev1): %s\n", json);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_update_set_u8(&revise, "0", 'Y');
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc2);
//        // printf("JSON (rev2): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [89]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc3, &rev_doc2);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_u8(&inserter, 'A');
//        carbon_insert_u8(&inserter, 'B');
//        carbon_update_set_u8(&revise, "2", 'C');
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc3);
//        // printf("JSON (rev3): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 3}, \"doc\": [65, 66, 67]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc4, &rev_doc3);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_update_set_u8(&revise, "0", 1);
//        carbon_update_set_u8(&revise, "1", 2);
//        carbon_update_set_u8(&revise, "2", 3);
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc4);
//        // printf("JSON (rev4): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 4}, \"doc\": [1, 2, 3]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//        carbon_drop(&rev_doc3);
//
//}
//
//TEST(CarbonTest, BisonUpdateMixedFixedTypesSimple)
//{
//        struct carbon doc, rev_doc, rev_doc2;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct string_builder sb;
//        const char *json;
//
//        string_builder_create(&sb);
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_u8(&inserter, 1);
//        carbon_insert_i64(&inserter, -42);
//        carbon_insert_float(&inserter, 23);
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc);
//        // printf("JSON (rev1): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [1, -42, 23.00]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_update_set_i64(&revise, "1", 1024);
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc2);
//        // printf("JSON (rev2): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [1, 1024, 23.00]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        u64 e1 = carbon_get_or_default_unsigned(&rev_doc2, "0", 0);
//        i64 e2 = carbon_get_or_default_signed(&rev_doc2, "1", 0);
//        float e3 = carbon_get_or_default_float(&rev_doc2, "2", NAN);
//
//        ASSERT_EQ(e1, 1);
//        ASSERT_EQ(e2, 1024);
//        ASSERT_TRUE(e3 > 22.9f && e3 < 24.0f);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//
//}
//
//TEST(CarbonTest, BisonRemoveConstantsToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_null(ins);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [null]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstConstants)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_true(ins);
//        carbon_insert_false(ins);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_FALSE);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [true, false]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [false]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastConstants)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_true(ins);
//        carbon_insert_false(ins);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [true, false]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [true]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleConstants)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_true(ins);
//        carbon_insert_null(ins);
//        carbon_insert_false(ins);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_FALSE);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [true, null, false]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [true, false]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveNumberToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_u8(ins, 42);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [42]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstNumber)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_u8(ins, 42);
//        carbon_insert_u32(ins, 23);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_NUMBER_U32);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [42, 23]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [23]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastNumber)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_u8(ins, 42);
//        carbon_insert_u32(ins, 23);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [42, 23]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [42]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleNumber)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_u8(ins, 42);
//        carbon_insert_u16(ins, 21);
//        carbon_insert_u32(ins, 23);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_NUMBER_U32);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [42, 21, 23]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [42, 23]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//
//TEST(CarbonTest, BisonRemoveStringToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_string(ins, "Hello");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\"]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstString)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_STRING);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [\"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastString)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [\"Hello\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleString)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        carbon_insert_string(ins, "Plato");
//        carbon_insert_string(ins, "Kant");
//        carbon_insert_string(ins, "Nietzsche");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_STRING);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Plato\", \"Kant\", \"Nietzsche\"]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [\"Plato\", \"Nietzsche\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//
//
//
//
//TEST(CarbonTest, BisonRemoveBinaryToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data, strlen(data), "txt", NULL);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_BINARY);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);
//
//        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
//        carbon_insert_binary(ins, data3, strlen(data3), "html", NULL);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_BINARY);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//
//
//
//
//
//
//
//
//
//
//TEST(CarbonTest, BisonRemoveCustomBinaryToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data = "ABC";
//        carbon_insert_binary(ins, data, strlen(data), NULL, "123");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        //carbon_hexdump_print(stdout, &doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"123\", \"encoding\": \"base64\", \"binary-string\": \"A=JDAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstCustomBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_BINARY_CUSTOM);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastCustomBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleCustomBinary)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
//        carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");
//
//        const char *data2 = "{\"key\": \"value\"}";
//        carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");
//
//        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
//        carbon_insert_binary(ins, data3, strlen(data3), NULL, "my-other-nonstandard-format");
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_BINARY_CUSTOM);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//
//
//
//
//
//
//
//
//
//TEST(CarbonTest, BisonRemoveArrayToEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        struct carbon_insert_array_state state;
//        struct carbon_insert *array_ins;
//        bool has_next;
//        string_builder_create(&sb);
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 1);
//        carbon_insert_u8(array_ins, 2);
//        carbon_insert_u8(array_ins, 3);
//        carbon_insert_array_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        //carbon_hexdump_print(stdout, &doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3]]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveFirstArray)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        struct carbon_insert_array_state state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 1);
//        carbon_insert_u8(array_ins, 2);
//        carbon_insert_u8(array_ins, 3);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 4);
//        carbon_insert_u8(array_ins, 5);
//        carbon_insert_u8(array_ins, 6);
//        carbon_insert_array_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_field_type next_type;
//        carbon_array_it_field_type(&next_type, &rev_it);
//        ASSERT_EQ(next_type, CARBON_FIELD_TYPE_ARRAY);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[4, 5, 6]]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveLastArray)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        struct carbon_insert_array_state state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 1);
//        carbon_insert_u8(array_ins, 2);
//        carbon_insert_u8(array_ins, 3);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 4);
//        carbon_insert_u8(array_ins, 5);
//        carbon_insert_u8(array_ins, 6);
//        carbon_insert_array_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[1, 2, 3]]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonRemoveMiddleArray)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//
//        struct carbon_insert_array_state state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 1);
//        carbon_insert_u8(array_ins, 2);
//        carbon_insert_u8(array_ins, 3);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 4);
//        carbon_insert_u8(array_ins, 5);
//        carbon_insert_u8(array_ins, 6);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 7);
//        carbon_insert_u8(array_ins, 8);
//        carbon_insert_u8(array_ins, 9);
//        carbon_insert_array_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        enum carbon_field_type type;
//        carbon_array_it_field_type(&type, &rev_it);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_ARRAY);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6], [7, 8, 9]]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[1, 2, 3], [7, 8, 9]]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonColumnRemoveTest)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//
//        struct carbon_insert_column_state state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_KEEP);
//
//        array_ins = carbon_insert_column_begin(&state, ins, CARBON_COLUMN_TYPE_U16, 10);
//        carbon_insert_u16(array_ins, 1);
//        carbon_insert_u16(array_ins, 2);
//        carbon_insert_u16(array_ins, 3);
//        carbon_insert_column_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//        struct carbon_column_it *cit = carbon_array_it_column_value(&rev_it);
//        enum carbon_field_type type;
//        u32 num_elems;
//        carbon_column_it_values_info(&type, &num_elems, cit);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U16);
//        ASSERT_EQ(num_elems, 3);
//
//        status = carbon_column_it_remove(cit, 1);
//        ASSERT_TRUE(status);
//        carbon_column_it_values_info(&type, &num_elems, cit);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U16);
//        ASSERT_EQ(num_elems, 2);
//        values = carbon_column_it_u16_values(&num_elems, cit);
//        ASSERT_EQ(values[0], 1);
//        ASSERT_EQ(values[1], 3);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        status = carbon_column_it_remove(cit, 0);
//        ASSERT_TRUE(status);
//        carbon_column_it_values_info(&type, &num_elems, cit);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U16);
//        ASSERT_EQ(num_elems, 1);
//        values = carbon_column_it_u16_values(&num_elems, cit);
//        ASSERT_EQ(values[0], 3);
//
//        char *json_3 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        status = carbon_column_it_remove(cit, 0);
//        ASSERT_TRUE(status);
//        carbon_column_it_values_info(&type, &num_elems, cit);
//        ASSERT_EQ(type, CARBON_FIELD_TYPE_COLUMN_U16);
//        ASSERT_EQ(num_elems, 0);
//
//        char *json_4 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        // printf(">> %s\n", json_1);
//        // printf(">> %s\n", json_2);
//        // printf(">> %s\n", json_3);
//        // printf(">> %s\n", json_4);
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 2, 3]]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[1, 3]]}") == 0);
//        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[3]]}") == 0);
//        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [[]]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//        free(json_3);
//        free(json_4);
//}
//
//TEST(CarbonTest, BisonRemoveComplexTest)
//{
//        struct carbon doc, rev_doc, rev_doc2, rev_doc3, rev_doc4, rev_doc5, rev_doc6, rev_doc7, rev_doc8, rev_doc9,
//                rev_doc10, rev_doc11, rev_doc12, rev_doc13, rev_doc14;
//        struct carbon_new context;
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        struct carbon_insert_array_state state, state2, state3;
//        struct carbon_insert_column_state cstate;
//        struct carbon_insert *array_ins, *array_ins2, *array_ins3, *column_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_u8(ins, 1);
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_u16(ins, 2);
//        carbon_insert_u32(ins, 3);
//        carbon_insert_u64(ins, 3);
//        carbon_insert_string(ins, "World");
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        carbon_insert_u8(array_ins, 4);
//        carbon_insert_string(array_ins, "Fox!");
//        carbon_insert_u8(array_ins, 6);
//        carbon_insert_array_end(&state);
//
//        array_ins = carbon_insert_array_begin(&state, ins, 10);
//        array_ins2 = carbon_insert_array_begin(&state2, array_ins, 10);
//        carbon_insert_array_end(&state2);
//        array_ins2 = carbon_insert_array_begin(&state2, array_ins, 10);
//        carbon_insert_u8(array_ins2, 4);
//        carbon_insert_array_end(&state2);
//        carbon_insert_null(array_ins);
//        array_ins2 = carbon_insert_array_begin(&state2, array_ins, 10);
//        carbon_insert_string(array_ins2, "Dog!");
//        array_ins3 = carbon_insert_array_begin(&state3, array_ins2, 10);
//        carbon_insert_array_end(&state3);
//        array_ins3 = carbon_insert_array_begin(&state3, array_ins2, 10);
//        column_ins = carbon_insert_column_begin(&cstate, array_ins3, CARBON_COLUMN_TYPE_U8, 10);
//
//        carbon_insert_u8(column_ins, 41);
//        carbon_insert_u8(column_ins, 42);
//        carbon_insert_u8(column_ins, 43);
//        carbon_insert_column_end(&cstate);
//
//        carbon_insert_array_end(&state3);
//        array_ins3 = carbon_insert_array_begin(&state3, array_ins2, 10);
//        carbon_insert_array_end(&state3);
//
//        carbon_insert_array_end(&state2);
//        carbon_insert_array_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [1, \"Hello\", 2, 3, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("3", &rev_doc, &doc);
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("5", &rev_doc2, &rev_doc);
//        char *json_3 = strdup(carbon_to_json(&sb, &rev_doc2));
//        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 3}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("5.1", &rev_doc3, &rev_doc2);
//        char *json_4 = strdup(carbon_to_json(&sb, &rev_doc3));
//        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 4}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("6.0", &rev_doc4, &rev_doc3);
//        char *json_5 = strdup(carbon_to_json(&sb, &rev_doc4));
//        ASSERT_TRUE(strcmp(json_5, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 5}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("5", &rev_doc5, &rev_doc4);
//        char *json_6 = strdup(carbon_to_json(&sb, &rev_doc5));
//        ASSERT_TRUE(strcmp(json_6, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 6}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("0", &rev_doc6, &rev_doc5);
//        carbon_revise_remove_one("1", &rev_doc7, &rev_doc6);
//        carbon_revise_remove_one("0", &rev_doc8, &rev_doc7);
//        carbon_revise_remove_one("1", &rev_doc9, &rev_doc8);
//        carbon_revise_remove_one("0", &rev_doc10, &rev_doc9);
//        char *json_11 = strdup(carbon_to_json(&sb, &rev_doc10));
//        ASSERT_TRUE(strcmp(json_11, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 11}, \"doc\": [[[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("0.2.2.0", &rev_doc11, &rev_doc10);
//
//        char *json_12 = strdup(carbon_to_json(&sb, &rev_doc11));
//        ASSERT_TRUE(strcmp(json_12, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 12}, \"doc\": [[[4], null, [\"Dog!\", [], [], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("0.2.2", &rev_doc12, &rev_doc11);
//
//        char *json_13 = strdup(carbon_to_json(&sb, &rev_doc12));
//        ASSERT_TRUE(strcmp(json_13, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 13}, \"doc\": [[[4], null, [\"Dog!\", [], []]]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("0.2", &rev_doc13, &rev_doc12);
//
//        char *json_14 = strdup(carbon_to_json(&sb, &rev_doc13));
//        ASSERT_TRUE(strcmp(json_14, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 14}, \"doc\": [[[4], null]]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_remove_one("0", &rev_doc14, &rev_doc13);
//
//        char *json_15 = strdup(carbon_to_json(&sb, &rev_doc14));
//        ASSERT_TRUE(strcmp(json_15, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 15}, \"doc\": []}") == 0);
//
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//        carbon_drop(&rev_doc3);
//        carbon_drop(&rev_doc4);
//        carbon_drop(&rev_doc5);
//        carbon_drop(&rev_doc6);
//        carbon_drop(&rev_doc7);
//        carbon_drop(&rev_doc8);
//        carbon_drop(&rev_doc9);
//        carbon_drop(&rev_doc10);
//        carbon_drop(&rev_doc11);
//        carbon_drop(&rev_doc12);
//        carbon_drop(&rev_doc13);
//        carbon_drop(&rev_doc14);
//        free(json_1);
//        free(json_2);
//        free(json_3);
//        free(json_4);
//        free(json_5);
//        free(json_6);
//        free(json_11);
//        free(json_12);
//        free(json_13);
//        free(json_14);
//        free(json_15);
//}
//
//TEST(CarbonTest, BisonUpdateMixedFixedTypesTypeChangeSimple)
//{
//        struct carbon doc, rev_doc, rev_doc2;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert inserter;
//        struct string_builder sb;
//        const char *json;
//
//        string_builder_create(&sb);
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_insert_u8(&inserter, 1);
//        carbon_insert_i64(&inserter, -42);
//        carbon_insert_float(&inserter, 23);
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc);
//        // printf("JSON (rev1): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [1, -42, 23.00]}") == 0);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
//        carbon_revise_iterator_open(&it, &revise);
//        carbon_array_it_insert_begin(&inserter, &it);
//
//        carbon_update_set_u32(&revise, "1", 1024);
//
//        carbon_array_it_insert_end(&inserter);
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//
//
//        json = carbon_to_json(&sb, &rev_doc2);
//        // printf("JSON (rev2): %s\n", json);
//        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [1, 1024, 23.00]}") == 0);
//
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        string_builder_drop(&sb);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        carbon_drop(&rev_doc2);
//
//}
//
//
//TEST(CarbonTest, BisonShrinkIssueFix)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeNoKey)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeNoKeyNoRevInc)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        u64 rev_old, rev_new;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revision(&rev_old, &doc);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev_new, &rev_doc);
//
//        ASSERT_EQ(rev_old, 0);
//        ASSERT_EQ(rev_new, rev_old);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeAutoKey)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeAutoKeyRevInc)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        u64 rev_old, rev_new;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revision(&rev_old, &doc);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev_new, &rev_doc);
//
//        ASSERT_EQ(rev_old, 1);
//        ASSERT_EQ(rev_new, 2);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeAutoKeyUpdate)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        object_id_t id, id_read;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_AUTOKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_generate(&id, &revise);
//        carbon_revise_end(&revise);
//
//        carbon_key_unsigned_value(&id_read, &rev_doc);
//        ASSERT_NE(id, 0);
//        ASSERT_EQ(id, id_read);
//
//        // carbon_print(stdout, &rev_doc);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeUnsignedKeyUpdate)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct string_builder sb;
//
//        string_builder_create(&sb);
//
//        u64 id_read;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_UKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_set_unsigned(&revise, 42);
//        carbon_revise_end(&revise);
//
//        carbon_key_unsigned_value(&id_read, &rev_doc);
//        ASSERT_EQ(id_read, 42);
//
//        // carbon_print(stdout, &rev_doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"ukey\", \"value\": 42}, \"rev\": 2}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        string_builder_drop(&sb);
//}
//
//TEST(CarbonTest, BisonKeyTypeSignedKeyUpdate)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct string_builder sb;
//
//        string_builder_create(&sb);
//
//        i64 id_read;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_IKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_set_signed(&revise, 42);
//        carbon_revise_end(&revise);
//
//        carbon_key_signed_value(&id_read, &rev_doc);
//        ASSERT_EQ(id_read, 42);
//
//        // carbon_print(stdout, &rev_doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"ikey\", \"value\": 42}, \"rev\": 2}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        string_builder_drop(&sb);
//}
//
//TEST(CarbonTest, BisonKeyTypeStringKeyUpdate)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct string_builder sb;
//
//        string_builder_create(&sb);
//
//        u64 key_len;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_key_set_string(&revise, "my_unique_id");
//        carbon_revise_end(&revise);
//
//        const char *key = carbon_key_string_value(&key_len, &rev_doc);
//        ASSERT_TRUE(strncmp(key, "my_unique_id", strlen("my_unique_id")) == 0);
//
//        // carbon_print(stdout, &rev_doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": \"my_unique_id\"}, \"rev\": 2}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        string_builder_drop(&sb);
//}
//
//TEST(CarbonTest, BisonKeyTypeUnsignedKey)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_UKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"ukey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeSignedKeyRevInc)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        u64 rev_old, rev_new;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_IKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revision(&rev_old, &doc);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev_new, &rev_doc);
//
//        ASSERT_EQ(rev_old, 1);
//        ASSERT_EQ(rev_new, 2);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeUnsignedKeyRevInc)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        u64 rev_old, rev_new;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_UKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revision(&rev_old, &doc);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev_new, &rev_doc);
//
//        ASSERT_EQ(rev_old, 1);
//        ASSERT_EQ(rev_new, 2);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeStringKeyRevInc)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        u64 rev_old, rev_new;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revision(&rev_old, &doc);
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_end(&revise);
//
//        carbon_revision(&rev_new, &rev_doc);
//
//        ASSERT_EQ(rev_old, 1);
//        ASSERT_EQ(rev_new, 2);
//
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//}
//
//
//TEST(CarbonTest, BisonKeyTypeSignedKey)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_IKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"ikey\", \"value\": 0}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonKeyTypeStringKey)
//{
//        struct carbon doc;
//        struct carbon_new context;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_string(ins, "Hello");
//        carbon_insert_string(ins, "World");
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertEmpty)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertNull)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_null(obj_ins, "My Key");
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":null}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleNulls)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_null(obj_ins, "My Key 1");
//        carbon_insert_prop_null(obj_ins, "My Key 2");
//        carbon_insert_prop_null(obj_ins, "My Key 3");
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":null, \"My Key 2\":null, \"My Key 3\":null}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertU8)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_u8(obj_ins, "My Key", 123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleU8s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_u8(obj_ins, "My Key 1", 1);
//        carbon_insert_prop_u8(obj_ins, "My Key 2", 2);
//        carbon_insert_prop_u8(obj_ins, "My Key 3", 3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":1, \"My Key 2\":2, \"My Key 3\":3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertU16)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_u16(obj_ins, "My Key", 123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleU16s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_u16(obj_ins, "My Key 1", 1);
//        carbon_insert_prop_u16(obj_ins, "My Key 2", 2);
//        carbon_insert_prop_u16(obj_ins, "My Key 3", 3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":1, \"My Key 2\":2, \"My Key 3\":3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertU32)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_u32(obj_ins, "My Key", 123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleU32s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_u32(obj_ins, "My Key 1", 1);
//        carbon_insert_prop_u32(obj_ins, "My Key 2", 2);
//        carbon_insert_prop_u32(obj_ins, "My Key 3", 3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":1, \"My Key 2\":2, \"My Key 3\":3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertU64)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_u64(obj_ins, "My Key", 123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleU64s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_u64(obj_ins, "My Key 1", 1);
//        carbon_insert_prop_u64(obj_ins, "My Key 2", 2);
//        carbon_insert_prop_u64(obj_ins, "My Key 3", 3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        //carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":1, \"My Key 2\":2, \"My Key 3\":3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertI8)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_i8(obj_ins, "My Key", -123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":-123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleI8s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_i8(obj_ins, "My Key 1", -1);
//        carbon_insert_prop_i8(obj_ins, "My Key 2", -2);
//        carbon_insert_prop_i8(obj_ins, "My Key 3", -3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":-1, \"My Key 2\":-2, \"My Key 3\":-3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertI16)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_i16(obj_ins, "My Key", -123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":-123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleI16s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_i16(obj_ins, "My Key 1", -1);
//        carbon_insert_prop_i16(obj_ins, "My Key 2", -2);
//        carbon_insert_prop_i16(obj_ins, "My Key 3", -3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":-1, \"My Key 2\":-2, \"My Key 3\":-3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertI32)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_i32(obj_ins, "My Key", -123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":-123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleI32s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_i32(obj_ins, "My Key 1", -1);
//        carbon_insert_prop_i32(obj_ins, "My Key 2", -2);
//        carbon_insert_prop_i32(obj_ins, "My Key 3", -3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":-1, \"My Key 2\":-2, \"My Key 3\":-3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertI64)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_i64(obj_ins, "My Key", -123);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":-123}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleI64s)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_i64(obj_ins, "My Key 1", -1);
//        carbon_insert_prop_i64(obj_ins, "My Key 2", -2);
//        carbon_insert_prop_i64(obj_ins, "My Key 3", -3);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":-1, \"My Key 2\":-2, \"My Key 3\":-3}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertFloat)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_float(obj_ins, "My Key", -123.32);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":-123.32}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleFloats)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_float(obj_ins, "My Key 1", -1.23);
//        carbon_insert_prop_float(obj_ins, "My Key 2", -2.42);
//        carbon_insert_prop_float(obj_ins, "My Key 3", 3.21);
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":-1.23, \"My Key 2\":-2.42, \"My Key 3\":3.21}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertTrue)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_true(obj_ins, "My Key");
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":true}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertFalse)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1024);
//        carbon_insert_prop_false(obj_ins, "My Key");
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key\":false}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleBooleans)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_true(obj_ins, "My Key 1");
//        carbon_insert_prop_false(obj_ins, "My Key 2");
//        carbon_insert_prop_true(obj_ins, "My Key 3");
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"My Key 1\":true, \"My Key 2\":false, \"My Key 3\":true}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMixed)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//        carbon_insert_prop_true(obj_ins, "k1");
//        carbon_insert_prop_false(obj_ins, "k2");
//        carbon_insert_prop_null(obj_ins, "k3");
//        carbon_insert_prop_u8(obj_ins, "k4", 1);
//        carbon_insert_prop_u16(obj_ins, "k5", 2);
//        carbon_insert_prop_u32(obj_ins, "k6", 3);
//        carbon_insert_prop_u64(obj_ins, "k7", 4);
//        carbon_insert_prop_i8(obj_ins, "k8", -1);
//        carbon_insert_prop_i16(obj_ins, "k9", -2);
//        carbon_insert_prop_i32(obj_ins, "k10", -3);
//        carbon_insert_prop_i64(obj_ins, "k11", -4);
//        carbon_insert_prop_float(obj_ins, "k12", 42.23);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"k1\":true, \"k2\":false, \"k3\":null, \"k4\":1, \"k5\":2, \"k6\":3, \"k7\":4, \"k8\":-1, \"k9\":-2, \"k10\":-3, \"k11\":-4, \"k12\":42.23}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertString)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_string(obj_ins, "hello", "world");
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"hello\":\"world\"}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleString)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_string(obj_ins, "k1", "v1");
//        carbon_insert_prop_string(obj_ins, "hello", "world");
//        carbon_insert_prop_string(obj_ins, "k3", "there");
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"k1\":\"v1\", \"hello\":\"world\", \"k3\":\"there\"}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleStringMixedTypes)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_false(obj_ins, "k2");
//        carbon_insert_prop_null(obj_ins, "k3");
//        carbon_insert_prop_u8(obj_ins, "k4", 1);
//        carbon_insert_prop_string(obj_ins, "s1", "v1");
//        carbon_insert_prop_u16(obj_ins, "k5", 2);
//        carbon_insert_prop_string(obj_ins, "s2-longer", "world");
//        carbon_insert_prop_u32(obj_ins, "k6", 3);
//        carbon_insert_prop_u64(obj_ins, "k7", 4);
//        carbon_insert_prop_i8(obj_ins, "k8", -1);
//        carbon_insert_prop_string(obj_ins, "s3", "there");
//        carbon_insert_prop_i16(obj_ins, "k9", -2);
//        carbon_insert_prop_i32(obj_ins, "k10", -3);
//        carbon_insert_prop_i64(obj_ins, "k11", -4);
//        carbon_insert_prop_float(obj_ins, "k12", 42.23);
//        carbon_insert_prop_true(obj_ins, "k1");
//
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"k2\":false, \"k3\":null, \"k4\":1, \"s1\":\"v1\", \"k5\":2, \"s2-longer\":\"world\", \"k6\":3, \"k7\":4, \"k8\":-1, \"s3\":\"there\", \"k9\":-2, \"k10\":-3, \"k11\":-4, \"k12\":42.23, \"k1\":true}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertBinary)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_binary(obj_ins, "my binary", "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"my binary\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleBinariesMixedTypes)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_float(obj_ins, "k12", 42.23);
//        carbon_insert_prop_true(obj_ins, "k1");
//        carbon_insert_prop_binary(obj_ins, "b1", "Hello", strlen("Hello"), "txt", NULL);
//        carbon_insert_prop_binary(obj_ins, "my binary", ",", strlen(","), "txt", NULL);
//        carbon_insert_prop_false(obj_ins, "k2");
//        carbon_insert_prop_null(obj_ins, "k3");
//        carbon_insert_prop_u8(obj_ins, "k4", 1);
//        carbon_insert_prop_string(obj_ins, "s1", "v1");
//        carbon_insert_prop_u16(obj_ins, "k5", 2);
//        carbon_insert_prop_binary(obj_ins, "b2", "World", strlen("World"), "txt", NULL);
//        carbon_insert_prop_string(obj_ins, "s2-longer", "world");
//        carbon_insert_prop_u32(obj_ins, "k6", 3);
//        carbon_insert_prop_u64(obj_ins, "k7", 4);
//        carbon_insert_prop_i8(obj_ins, "k8", -1);
//        carbon_insert_prop_string(obj_ins, "s3", "there");
//        carbon_insert_prop_i16(obj_ins, "k9", -2);
//        carbon_insert_prop_i32(obj_ins, "k10", -3);
//        carbon_insert_prop_i64(obj_ins, "k11", -4);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"k12\":42.23, \"k1\":true, \"b1\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"my binary\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"k2\":false, \"k3\":null, \"k4\":1, \"s1\":\"v1\", \"k5\":2, \"b2\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }, \"s2-longer\":\"world\", \"k6\":3, \"k7\":4, \"k8\":-1, \"s3\":\"there\", \"k9\":-2, \"k10\":-3, \"k11\":-4}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertMultipleBinaries)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_binary(obj_ins, "b1", "Hello", strlen("Hello"), "txt", NULL);
//        carbon_insert_prop_binary(obj_ins, "my binary", ",", strlen(","), "txt", NULL);
//        carbon_insert_prop_binary(obj_ins, "b2", "World", strlen("World"), "txt", NULL);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"b1\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"my binary\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"b2\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertObjectEmpty)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state, nested;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_object_begin(&nested, obj_ins, "my nested", 200);
//        carbon_insert_prop_object_end(&nested);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"my nested\":{}}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertObjectMixedMxed)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state, nested;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_float(obj_ins, "1", 42.23);
//        carbon_insert_prop_true(obj_ins, "2");
//        carbon_insert_prop_binary(obj_ins, "3", "Hello", strlen("Hello"), "txt", NULL);
//        carbon_insert_prop_binary(obj_ins, "4", ",", strlen(","), "txt", NULL);
//        carbon_insert_prop_binary(obj_ins, "5", "World", strlen("World"), "txt", NULL);
//        carbon_insert_prop_string(obj_ins, "6", "world");
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_prop_object_begin(&nested, obj_ins, "my nested", 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "7");
//        carbon_insert_prop_null(nested_obj_ins, "8");
//        carbon_insert_prop_u8(nested_obj_ins, "9", 1);
//        carbon_insert_prop_string(nested_obj_ins, "10", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "11", 2);
//
//        carbon_insert_prop_object_end(&nested);
//
//        carbon_insert_prop_u32(obj_ins, "12", 3);
//        carbon_insert_prop_u64(obj_ins, "13", 4);
//        carbon_insert_prop_i8(obj_ins, "14", -1);
//        carbon_insert_prop_string(obj_ins, "15", "there");
//        carbon_insert_prop_i16(obj_ins, "16", -2);
//        carbon_insert_prop_i32(obj_ins, "17", -3);
//        carbon_insert_prop_i64(obj_ins, "18", -4);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"1\":42.23, \"2\":true, \"3\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"4\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"5\":{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }, \"6\":\"world\", \"my nested\":{\"7\":false, \"8\":null, \"9\":1, \"10\":\"v1\", \"11\":2}, \"12\":3, \"13\":4, \"14\":-1, \"15\":\"there\", \"16\":-2, \"17\":-3, \"18\":-4}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertArrayEmpty)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//        struct carbon_insert_array_state array_state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        carbon_insert_prop_array_begin(&array_state, obj_ins, "my array", 200);
//        carbon_insert_prop_array_end(&array_state);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"my array\":[]}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertArrayData)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//        struct carbon_insert_array_state array_state, nested_array_state;
//        struct carbon_insert_column_state column_state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        struct carbon_insert *nested_array_ins = carbon_insert_prop_array_begin(&array_state, obj_ins, "my array", 200);
//
//        struct carbon_insert *column_ins = carbon_insert_column_begin(&column_state, nested_array_ins, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(column_ins, 'X');
//        carbon_insert_u32(column_ins, 'Y');
//        carbon_insert_u32(column_ins, 'Z');
//        carbon_insert_column_end(&column_state);
//        struct carbon_insert *nested_ins = carbon_insert_array_begin(&nested_array_state, nested_array_ins, 10);
//        carbon_insert_string(nested_ins, "Hello");
//        column_ins = carbon_insert_column_begin(&column_state, nested_ins, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(column_ins, 'A');
//        carbon_insert_u32(column_ins, 'B');
//        carbon_insert_u32(column_ins, 'C');
//        carbon_insert_column_end(&column_state);
//        carbon_insert_string(nested_ins, "World");
//        carbon_insert_array_end(&nested_array_state);
//        carbon_insert_u8(nested_array_ins, 1);
//        carbon_insert_u8(nested_array_ins, 1);
//        column_ins = carbon_insert_column_begin(&column_state, nested_array_ins, CARBON_COLUMN_TYPE_U32, 10);
//        carbon_insert_u32(column_ins, 23);
//        carbon_insert_u32(column_ins, 24);
//        carbon_insert_u32(column_ins, 25);
//        carbon_insert_column_end(&column_state);
//        carbon_insert_u8(nested_array_ins, 1);
//
//        carbon_insert_prop_array_end(&array_state);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"my array\":[[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//TEST(CarbonTest, BisonObjectInsertColumnNonEmpty)
//{
//        struct carbon doc;
//        struct carbon_new context;
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *obj_ins = carbon_insert_object_begin(&state, ins, 1);
//
//        struct carbon_insert *nested_column_ins = carbon_insert_prop_column_begin(&column_state, obj_ins, "my column", CARBON_COLUMN_TYPE_U16, 200);
//        carbon_insert_u16(nested_column_ins, 1);
//        carbon_insert_u16(nested_column_ins, 2);
//        carbon_insert_u16(nested_column_ins, 3);
//        carbon_insert_prop_column_end(&column_state);
//
//        carbon_insert_object_end(&state);
//
//        carbon_create_end(&context);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        struct string_builder sb;
//        string_builder_create(&sb);
//
//        // carbon_print(stdout, &doc);
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"my column\":[1, 2, 3]}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//}
//
//static void create_nested_doc(struct carbon *rev_doc)
//{
//        struct carbon doc;
//        struct carbon_revise revise;
//        struct carbon_array_it it;
//        struct carbon_insert nested_ins, *array_ins, *col_ins, *nested_array_ins;
//        struct carbon_insert_array_state array_state, nested_array_state;
//        struct carbon_insert_column_state column_state;
//
//        carbon_create_empty(&doc, CARBON_KEY_AUTOKEY);
//        carbon_revise_begin(&revise, rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//
//        carbon_array_it_insert_begin(&nested_ins, &it);
//
//        array_ins = carbon_insert_array_begin(&array_state, &nested_ins, 10);
//
//        carbon_insert_null(array_ins);
//        carbon_insert_true(array_ins);
//        carbon_insert_false(array_ins);
//        carbon_insert_u8(array_ins, 8);
//        carbon_insert_i16(array_ins, -16);
//        carbon_insert_string(array_ins, "Hello, World!");
//        carbon_insert_binary(array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        carbon_insert_binary(array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = carbon_insert_column_begin(&column_state, array_ins, CARBON_COLUMN_TYPE_U32, 20);
//
//        carbon_insert_u32(col_ins, 32);
//        carbon_insert_u32(col_ins, 33);
//        carbon_insert_u32(col_ins, 34);
//        carbon_insert_u32(col_ins, 35);
//
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//        carbon_insert_array_end(&nested_array_state);
//
//        nested_array_ins = carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//
//        carbon_insert_null(nested_array_ins);
//        carbon_insert_true(nested_array_ins);
//        carbon_insert_false(nested_array_ins);
//        carbon_insert_u8(nested_array_ins, 8);
//        carbon_insert_i16(nested_array_ins, -16);
//        carbon_insert_string(nested_array_ins, "Hello, World!");
//        carbon_insert_binary(nested_array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        carbon_insert_binary(nested_array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = carbon_insert_column_begin(&column_state, nested_array_ins, CARBON_COLUMN_TYPE_U32, 20);
//
//        carbon_insert_u32(col_ins, 32);
//        carbon_insert_u32(col_ins, 33);
//        carbon_insert_u32(col_ins, 34);
//        carbon_insert_u32(col_ins, 35);
//
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_array_end(&nested_array_state);
//
//        carbon_insert_array_end(&array_state);
//
//        array_ins = carbon_insert_array_begin(&array_state, &nested_ins, 10);
//
//        carbon_insert_null(array_ins);
//        carbon_insert_true(array_ins);
//        carbon_insert_false(array_ins);
//        carbon_insert_u8(array_ins, 8);
//        carbon_insert_i16(array_ins, -16);
//        carbon_insert_string(array_ins, "Hello, World!");
//        carbon_insert_binary(array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        carbon_insert_binary(array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = carbon_insert_column_begin(&column_state, array_ins, CARBON_COLUMN_TYPE_U32, 20);
//
//        carbon_insert_u32(col_ins, 32);
//        carbon_insert_u32(col_ins, 33);
//        carbon_insert_u32(col_ins, 34);
//        carbon_insert_u32(col_ins, 35);
//
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//        carbon_insert_array_end(&nested_array_state);
//
//        nested_array_ins = carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//
//        carbon_insert_null(nested_array_ins);
//        carbon_insert_true(nested_array_ins);
//        carbon_insert_false(nested_array_ins);
//        carbon_insert_u8(nested_array_ins, 8);
//        carbon_insert_i16(nested_array_ins, -16);
//        carbon_insert_string(nested_array_ins, "Hello, World!");
//        carbon_insert_binary(nested_array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        carbon_insert_binary(nested_array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = carbon_insert_column_begin(&column_state, nested_array_ins, CARBON_COLUMN_TYPE_U32, 20);
//
//        carbon_insert_u32(col_ins, 32);
//        carbon_insert_u32(col_ins, 33);
//        carbon_insert_u32(col_ins, 34);
//        carbon_insert_u32(col_ins, 35);
//
//        carbon_insert_column_end(&column_state);
//
//        carbon_insert_array_end(&nested_array_state);
//
//        carbon_insert_array_end(&array_state);
//
//        carbon_array_it_insert_end(&nested_ins);
//
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//}
//
//TEST(CarbonTest, BisonObjectRemoveTest)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "1");
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "6");
//        carbon_insert_prop_null(nested_obj_ins, "7");
//        carbon_insert_prop_u8(nested_obj_ins, "8", 1);
//        carbon_insert_prop_string(nested_obj_ins, "9", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "10", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "11");
//        carbon_insert_prop_null(nested_obj_ins, "12");
//        carbon_insert_prop_u8(nested_obj_ins, "13", 1);
//        carbon_insert_prop_string(nested_obj_ins, "14", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "15", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}, {\"6\":false, \"7\":null, \"8\":1, \"9\":\"v1\", \"10\":2}, {\"11\":false, \"12\":null, \"13\":1, \"14\":\"v1\", \"15\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 2}, \"doc\": []}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemoveSkipOneTest)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_SKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "1");
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "6");
//        carbon_insert_prop_null(nested_obj_ins, "7");
//        carbon_insert_prop_u8(nested_obj_ins, "8", 1);
//        carbon_insert_prop_string(nested_obj_ins, "9", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "10", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "11");
//        carbon_insert_prop_null(nested_obj_ins, "12");
//        carbon_insert_prop_u8(nested_obj_ins, "13", 1);
//        carbon_insert_prop_string(nested_obj_ins, "14", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "15", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        carbon_array_it_remove(&rev_it);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 1}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}, {\"6\":false, \"7\":null, \"8\":1, \"9\":\"v1\", \"10\":2}, {\"11\":false, \"12\":null, \"13\":1, \"14\":\"v1\", \"15\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"skey\", \"value\": null}, \"rev\": 2}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringIt)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        has_next = carbon_object_it_next(obj_it);
////        ASSERT_TRUE(has_next);
////        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
////        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"Hello Long Key\":\"Hello Long Value\", \"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringItAtIndex1)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"Hello Long Key\":\"Hello Long Value\", \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringItAtIndex2)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"Hello Long Key\":\"Hello Long Value\", \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringItAtIndex3)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"Hello Long Key\":\"Hello Long Value\", \"4\":\"v1\", \"5\":2}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringItAtIndex4)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"Hello Long Key\":\"Hello Long Value\", \"5\":2}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
////
////TEST(CarbonTest, BisonObjectInsertPropDuringItAtIndex5)
////{
////        struct carbon doc, rev_doc;
////        struct carbon_new context;
////        struct carbon_revise revise;
////        struct carbon_array_it rev_it;
////        struct string_builder sb;
////        bool has_next;
////        string_builder_create(&sb);
////        bool status;
////        const u16 *values;
////        u64 key_len;
////
////        struct carbon_insert_object_state state;
////        struct carbon_insert_column_state column_state;
////        struct carbon_insert *array_ins, nested_ins;
////
////        // -------------------------------------------------------------------------------------------------------------
////        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
////
////        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
////
////        carbon_insert_prop_false(nested_obj_ins, "1");
////        carbon_insert_prop_null(nested_obj_ins, "2");
////        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
////        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
////        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
////
////        carbon_insert_prop_object_end(&state);
////
////        carbon_create_end(&context);
////        // -------------------------------------------------------------------------------------------------------------
////
////        char *json_1 = strdup(carbon_to_json(&sb, &doc));
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        carbon_revise_begin(&revise, &rev_doc, &doc);
////        carbon_revise_iterator_open(&rev_it, &revise);
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_TRUE(has_next);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        enum carbon_field_type field_type;
////        carbon_array_it_field_type(&field_type, &rev_it);
////        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
////        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_TRUE(carbon_object_it_next(obj_it));
////        ASSERT_FALSE(carbon_object_it_next(obj_it));
////
////        carbon_hexdump_print(stderr, &rev_doc);
////        carbon_object_it_insert_begin(&nested_ins, obj_it);
////        carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
////        carbon_object_it_insert_end(&nested_ins);
////        carbon_hexdump_print(stderr, &rev_doc);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        has_next = carbon_array_it_next(&rev_it);
////        ASSERT_FALSE(has_next);
////
////        carbon_revise_iterator_close(&rev_it);
////        carbon_revise_end(&revise);
////
////        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
////        printf("\n%s\n", json_2);
////
////        // -------------------------------------------------------------------------------------------------------------
////
////        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
////        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2, \"Hello Long Key\":\"Hello Long Value\"}]}") == 0);
////
////        string_builder_drop(&sb);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////        free(json_1);
////        free(json_2);
////}
//
//TEST(CarbonTest, BisonObjectRemovePropByKey)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_false(nested_obj_ins, "1");
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":false, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeObjectNonEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_object_state nested_obj;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        struct carbon_insert *nested_nested_obj_ins = carbon_insert_prop_object_begin(&nested_obj, nested_obj_ins, "1", 100);
//        carbon_insert_prop_null(nested_nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_nested_obj_ins, "5", 2);
//        carbon_insert_prop_object_end(&nested_obj);
//
//        carbon_insert_prop_null(nested_obj_ins, "6");
//        carbon_insert_prop_u8(nested_obj_ins, "7", 1);
//        carbon_insert_prop_string(nested_obj_ins, "8", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "9", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}, \"6\":null, \"7\":1, \"8\":\"v1\", \"9\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"6\":null, \"7\":1, \"8\":\"v1\", \"9\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeArrayEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_array_state nested_arr;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        struct carbon_insert *nested_arr_it = carbon_insert_prop_array_begin(&nested_arr, nested_obj_ins, "1", 100);
//
//        carbon_insert_prop_array_end(&nested_arr);
//
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":[], \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeArrayNonEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_array_state nested_arr;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        struct carbon_insert *nested_arr_it = carbon_insert_prop_array_begin(&nested_arr, nested_obj_ins, "1", 100);
//        carbon_insert_null(nested_arr_it);
//        carbon_insert_u8(nested_arr_it, 1);
//        carbon_insert_string(nested_arr_it, "v1");
//        carbon_insert_u16(nested_arr_it, 2);
//        carbon_insert_prop_array_end(&nested_arr);
//
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":[null, 1, \"v1\", 2], \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeColumnEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_column_state nested_col;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_column_begin(&nested_col, nested_obj_ins, "1", CARBON_COLUMN_TYPE_U32, 100);
//
//        carbon_insert_prop_column_end(&nested_col);
//
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_hexdump_print(stderr, &doc); // TODO: Debug Remove
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":[], \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeColumnNonEmpty)
//{
//
//}
//
//TEST(CarbonTest, BisonObjectRemovePropByKeyTypeObjectEmpty)
//{
//        struct carbon doc, rev_doc;
//        struct carbon_new context;
//        struct carbon_revise revise;
//        struct carbon_array_it rev_it;
//        struct string_builder sb;
//        bool has_next;
//        string_builder_create(&sb);
//        bool status;
//        const u16 *values;
//        u64 key_len;
//
//        struct carbon_insert_object_state state;
//        struct carbon_insert_column_state column_state;
//        struct carbon_insert_object_state nested_obj;
//        struct carbon_insert *array_ins;
//
//        // -------------------------------------------------------------------------------------------------------------
//        struct carbon_insert *ins = carbon_create_begin(&context, &doc, CARBON_KEY_NOKEY, CARBON_OPTIMIZE);
//
//        struct carbon_insert *nested_obj_ins = carbon_insert_object_begin(&state, ins, 200);
//
//        carbon_insert_prop_object_begin(&nested_obj, nested_obj_ins, "1", 100);
//        carbon_insert_prop_object_end(&nested_obj);
//
//        carbon_insert_prop_null(nested_obj_ins, "2");
//        carbon_insert_prop_u8(nested_obj_ins, "3", 1);
//        carbon_insert_prop_string(nested_obj_ins, "4", "v1");
//        carbon_insert_prop_u16(nested_obj_ins, "5", 2);
//
//        carbon_insert_prop_object_end(&state);
//
//        carbon_create_end(&context);
//        // -------------------------------------------------------------------------------------------------------------
//
//        char *json_1 = strdup(carbon_to_json(&sb, &doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        carbon_revise_begin(&revise, &rev_doc, &doc);
//        carbon_revise_iterator_open(&rev_it, &revise);
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_TRUE(has_next);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        enum carbon_field_type field_type;
//        carbon_array_it_field_type(&field_type, &rev_it);
//        ASSERT_EQ(field_type, CARBON_FIELD_TYPE_OBJECT);
//        struct carbon_object_it *obj_it = carbon_array_it_object_value(&rev_it);
//        has_next = carbon_object_it_next(obj_it);
//        ASSERT_TRUE(has_next);
//        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
//        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);
//
//        carbon_hexdump_print(stderr, &rev_doc);
//        carbon_object_it_remove(obj_it);
//        carbon_hexdump_print(stderr, &rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        has_next = carbon_array_it_next(&rev_it);
//        ASSERT_FALSE(has_next);
//
//        carbon_revise_iterator_close(&rev_it);
//        carbon_revise_end(&revise);
//
//        char *json_2 = strdup(carbon_to_json(&sb, &rev_doc));
//
//        // -------------------------------------------------------------------------------------------------------------
//
//        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"1\":{}, \"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"rev\": 0}, \"doc\": [{\"2\":null, \"3\":1, \"4\":\"v1\", \"5\":2}]}") == 0);
//
//        string_builder_drop(&sb);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        free(json_1);
//        free(json_2);
//}
//
//TEST(CarbonTest, BisonUpdateSetToNull)
//{
//        struct carbon doc, rev_doc;
//        struct string_builder sb;
//        bool status;
//
//        string_builder_create(&sb);
//
//        /* Each time 'create_nested_doc' is called, the following document will be generated
//
//                [
//                   [
//                      null,
//                      true,
//                      false,
//                      8,
//                      -16,
//                      "Hello, World!",
//                      {
//                         "type":"text/plain",
//                         "encoding":"base64",
//                         "binary-string":"TXkgUGxhaW4tVGV4dAAA"
//                      },
//                      {
//                         "type":"own",
//                         "encoding":"base64",
//                         "binary-string":"TXkgT3duIEZvcm1hdAAA"
//                      },
//                      [
//                         32,
//                         33,
//                         34,
//                         35
//                      ],
//                      [
//
//                      ],
//                      [
//                         null,
//                         true,
//                         false,
//                         8,
//                         -16,
//                         "Hello, World!",
//                         {
//                            "type":"text/plain",
//                            "encoding":"base64",
//                            "binary-string":"TXkgUGxhaW4tVGV4dAAA"
//                         },
//                         {
//                            "type":"own",
//                            "encoding":"base64",
//                            "binary-string":"TXkgT3duIEZvcm1hdAAA"
//                         },
//                         [
//                            32,
//                            33,
//                            34,
//                            35
//                         ]
//                      ]
//                   ],
//                   [
//                      null,
//                      true,
//                      false,
//                      8,
//                      -16,
//                      "Hello, World!",
//                      {
//                         "type":"text/plain",
//                         "encoding":"base64",
//                         "binary-string":"TXkgUGxhaW4tVGV4dAAA"
//                      },
//                      {
//                         "type":"own",
//                         "encoding":"base64",
//                         "binary-string":"TXkgT3duIEZvcm1hdAAA"
//                      },
//                      [
//                         32,
//                         33,
//                         34,
//                         35
//                      ],
//                      [
//
//                      ],
//                      [
//                         null,
//                         true,
//                         false,
//                         8,
//                         -16,
//                         "Hello, World!",
//                         {
//                            "type":"text/plain",
//                            "encoding":"base64",
//                            "binary-string":"TXkgUGxhaW4tVGV4dAAA"
//                         },
//                         {
//                            "type":"own",
//                            "encoding":"base64",
//                            "binary-string":"TXkgT3duIEZvcm1hdAAA"
//                         },
//                         [
//                            32,
//                            33,
//                            34,
//                            35
//                         ]
//                      ]
//                   ]
//                ]
//
//         */
//
//        // -------------------------------------------------------------------------------------------------------------
//        // Update to null
//        // -------------------------------------------------------------------------------------------------------------
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.0", &rev_doc, &doc); // replaces null with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.1", &rev_doc, &doc); // replaces true with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, null, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.2", &rev_doc, &doc); // replaces false with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, null, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.3", &rev_doc, &doc); // replaces u8 (8) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, null, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.4", &rev_doc, &doc); // replaces i16 (-16) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, null, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.5", &rev_doc, &doc); // replaces string with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, null, { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.6", &rev_doc, &doc); // replaces binary string with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", null, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.7", &rev_doc, &doc); // replaces custom binary with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, null, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.8", &rev_doc, &doc); // replaces column ([32, 33, 34, 35]) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, null, [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.8.0", &rev_doc, &doc); // replaces element in column with null value (special case) --> [NULL, 33, 34, 35]
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [null, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.9", &rev_doc, &doc); // replaces empty array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], null, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.10", &rev_doc, &doc); // replaces complex array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], null], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0", &rev_doc, &doc); // replaces 1st outermost array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [null, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("1", &rev_doc, &doc); // replaces 2nd outermost array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], null]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        // -------------------------------------------------------------------------------------------------------------
//        // Update to true
//        // -------------------------------------------------------------------------------------------------------------
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.0", &rev_doc, &doc); // replaces null with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[true, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.1", &rev_doc, &doc); // replaces true with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.2", &rev_doc, &doc); // replaces false with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, true, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.3", &rev_doc, &doc); // replaces u8 (8) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, true, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.4", &rev_doc, &doc); // replaces i16 (-16) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, true, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.5", &rev_doc, &doc); // replaces string with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, true, { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.6", &rev_doc, &doc); // replaces binary string with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", true, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.7", &rev_doc, &doc); // replaces custom binary with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, true, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.8", &rev_doc, &doc); // replaces column ([32, 33, 34, 35]) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, true, [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//
////        create_nested_doc(&doc);
////        // ??????
////        status = carbon_update_one_set_true("0.8.0", &rev_doc, &doc); // replaces element in column with null value (special case) --> [NULL, 33, 34, 35]
////        ASSERT_TRUE(status);
////        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
////        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
////        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [true, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////
////        create_nested_doc(&doc);
////        status = carbon_update_one_set_true("0.9", &rev_doc, &doc); // replaces empty array with true
////        ASSERT_TRUE(status);
////        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
////        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
////        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], true, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////
////        create_nested_doc(&doc);
////        status = carbon_update_one_set_true("0.10", &rev_doc, &doc); // replaces complex array with true
////        ASSERT_TRUE(status);
////        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
////        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
////        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], true], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////
////        create_nested_doc(&doc);
////        status = carbon_update_one_set_true("0", &rev_doc, &doc); // replaces 1st outermost array with true
////        ASSERT_TRUE(status);
////        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
////        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
////        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [true, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
////
////        create_nested_doc(&doc);
////        status = carbon_update_one_set_true("1", &rev_doc, &doc); // replaces 2nd outermost array with true
////        ASSERT_TRUE(status);
////        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
////        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
////        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], true]}") == 0);
////        carbon_drop(&doc);
////        carbon_drop(&rev_doc);
//
//        /*
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.5", &rev_doc, &doc); // replaces string with null
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"autokey\", \"value\": 0}, \"rev\": 2}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        carbon_drop(&doc);
//        carbon_drop(&rev_doc);
//        */
//
//
//        // Overwrite constant in-pace w/ fixed-type
//        // Overwrite constant in-pace w/ string
//        // Overwrite constant in-pace w/ binary
//        // Overwrite constant in-pace w/ custom binary
//        // Overwrite constant in-pace w/ empty array
//        // Overwrite constant in-pace w/ non-empty array
//        // Overwrite constant in-pace w/ empty column
//        // Overwrite constant in-pace w/ non-empty column
//
//        // Update fixed-type in-place
//        // Overwrite fixed-type in-pace w/ constant
//        // Overwrite fixed-type in-pace w/ fixed-type (w/ same width)
//        // Overwrite fixed-type in-pace w/ fixed-type (w/ other width)
//        // Overwrite fixed-type in-pace w/ string
//        // Overwrite fixed-type in-pace w/ binary
//        // Overwrite fixed-type in-pace w/ custom binary
//        // Overwrite fixed-type in-pace w/ empty array
//        // Overwrite fixed-type in-pace w/ non-empty array
//        // Overwrite fixed-type in-pace w/ empty column
//        // Overwrite fixed-type in-pace w/ non-empty column
//
//        // Update string in-place
//        // Overwrite string in-pace w/ constant
//        // Overwrite string in-pace w/ fixed-type
//        // Overwrite string in-pace w/ string
//        // Overwrite string in-pace w/ binary
//        // Overwrite string in-pace w/ custom binary
//        // Overwrite string in-pace w/ empty array
//        // Overwrite string in-pace w/ non-empty array
//        // Overwrite string in-pace w/ empty column
//        // Overwrite string in-pace w/ non-empty column
//
//        // Update binary in-place
//        // Overwrite binary in-pace w/ constant
//        // Overwrite binary in-pace w/ fixed-type
//        // Overwrite binary in-pace w/ string
//        // Overwrite binary in-pace w/ binary
//        // Overwrite binary in-pace w/ custom binary
//        // Overwrite binary in-pace w/ empty array
//        // Overwrite binary in-pace w/ non-empty array
//        // Overwrite binary in-pace w/ empty column
//        // Overwrite binary in-pace w/ non-empty column
//
//        // Update custom binary in-place
//        // Overwrite custom binary in-pace w/ constant
//        // Overwrite custom binary in-pace w/ fixed-type
//        // Overwrite custom binary in-pace w/ string
//        // Overwrite custom binary in-pace w/ binary
//        // Overwrite custom binary in-pace w/ custom binary
//        // Overwrite custom binary in-pace w/ empty array
//        // Overwrite custom binary in-pace w/ non-empty array
//        // Overwrite custom binary in-pace w/ empty column
//        // Overwrite custom binary in-pace w/ non-empty column
//
//        // Update empty-array binary in-place
//        // Overwrite empty-array in-pace w/ constant
//        // Overwrite empty-array in-pace w/ fixed-type
//        // Overwrite empty-array in-pace w/ string
//        // Overwrite empty-array in-pace w/ binary
//        // Overwrite empty-array in-pace w/ custom binary
//        // Overwrite empty-array in-pace w/ non-empty array
//        // Overwrite empty-array in-pace w/ empty column
//        // Overwrite empty-array in-pace w/ non-empty column
//
//        // Update non-empty array binary in-place
//        // Overwrite non-empty array in-pace w/ constant
//        // Overwrite non-empty array in-pace w/ fixed-type
//        // Overwrite non-empty array in-pace w/ string
//        // Overwrite non-empty array in-pace w/ binary
//        // Overwrite non-empty array in-pace w/ custom binary
//        // Overwrite non-empty array in-pace w/ empty array
//        // Overwrite non-empty array in-pace w/ non-empty array
//        // Overwrite non-empty array in-pace w/ empty column
//        // Overwrite non-empty array in-pace w/ non-empty column
//
//        // Overwrite empty column in-pace w/ constant
//        // Overwrite empty column in-pace w/ fixed-type
//        // Overwrite empty column in-pace w/ string
//        // Overwrite empty column in-pace w/ binary
//        // Overwrite empty column in-pace w/ custom binary
//        // Overwrite empty column in-pace w/ empty array
//        // Overwrite empty column in-pace w/ non-empty array
//        // Overwrite empty column in-pace w/ non-empty column
//
//        // Update non-empty column in-place
//        // Overwrite non-empty column in-pace w/ constant
//        // Overwrite non-empty column in-pace w/ fixed-type
//        // Overwrite non-empty column in-pace w/ string
//        // Overwrite non-empty column in-pace w/ binary
//        // Overwrite non-empty column in-pace w/ custom binary
//        // Overwrite non-empty column in-pace w/ empty array
//        // Overwrite non-empty column in-pace w/ non-empty array
//        // Overwrite non-empty column in-pace w/ empty column
//        // Overwrite non-empty column in-pace w/ non-empty column
//
//        // Update column entry in-place
//        // Overwrite column entry in-pace w/ constant (matching type)
//        // Overwrite column entry in-pace w/ constant (not matching type)
//        // Overwrite column entry in-pace w/ fixed-type (matching type)
//        // Overwrite column entry in-pace w/ fixed-type (not matching type)
//
//        // Overwrite entire document content in-pace w/ constant
//        // Overwrite entire document content in-pace w/ fixed-type
//        // Overwrite entire document content in-pace w/ string
//        // Overwrite entire document content in-pace w/ binary
//        // Overwrite entire document content in-pace w/ custom binary
//        // Overwrite entire document content in-pace w/ empty array
//        // Overwrite entire document content in-pace w/ non-empty array
//        // Overwrite entire document content in-pace w/ empty column
//        // Overwrite entire document content in-pace w/ non-empty column
//
//
//        string_builder_drop(&sb);
//}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}