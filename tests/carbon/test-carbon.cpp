#include <gtest/gtest.h>

#include <jak_carbon.h>
#include <jak_carbon_dot.h>
#include <jak_carbon_array_it.h>
#include <jak_carbon_insert.h>
#include <jak_carbon_find.h>
#include <jak_carbon_update.h>
#include <jak_carbon_path.h>
#include <jak_carbon_get.h>
#include <jak_carbon_revise.h>
#include <jak_carbon_object_it.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <jak_carbon_commit.h>
#include <jak_carbon_path_index.h>

TEST(CarbonTest, CreateCarbon) {
        jak_carbon doc;
        jak_global_id_t oid;
        jak_u64 rev;
        struct jak_string builder;
        bool status;

        string_create(&builder);

        status = jak_carbon_create_empty(&doc, JAK_CARBON_KEY_AUTOKEY);
        EXPECT_TRUE(status);

        //jak_carbon_hexdump_print(stderr, &doc);

        status = jak_carbon_key_unsigned_value(&oid, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(oid, 0);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_NE(rev, 0);

        jak_carbon_to_str(&builder, JAK_JSON_EXTENDED, &doc);
        // printf("%s\n", string_builder_cstr(&builder));
        string_drop(&builder);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CreateCarbonRevisionNumberingNoKey) {
        jak_carbon_new context;
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_u64 commit_new, commit_mod;

        jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);
        jak_carbon_create_end(&context);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        /* Commit hash value for records with 'nokey' option are always set to 0 (see specs) */
        jak_carbon_commit_hash(&commit_new, &doc);
        jak_carbon_commit_hash(&commit_mod, &rev_doc);
        ASSERT_EQ(commit_new, 0);
        ASSERT_EQ(commit_mod, 0);
        
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CreateCarbonRevisionNumberingWithKey) {
        jak_carbon_new context;
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_u64 commit_new, commit_mod, commit_mod_cmpr;

        jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_AUTOKEY, JAK_CARBON_OPTIMIZE);
        jak_carbon_create_end(&context);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        /* Commit hash value for records with no 'nokey' option shall be a (almost global) random number, and
         * a 64bit bernstein hash value of the original document after any modification (see specs) */
        jak_carbon_commit_hash(&commit_new, &doc);
        jak_carbon_commit_hash(&commit_mod, &rev_doc);
        ASSERT_NE(commit_new, 0);
        ASSERT_NE(commit_new, commit_mod);

        jak_u64 raw_data_len = 0;
        const void *raw_data = jak_carbon_raw_data(&raw_data_len, &doc);
        jak_carbon_commit_hash_compute(&commit_mod_cmpr, raw_data, raw_data_len);

        ASSERT_EQ(commit_mod, commit_mod_cmpr);
}

TEST(CarbonTest, CreateCarbonRevisionNumbering) {
        jak_carbon doc, rev_doc;
        jak_u64 rev;
        struct jak_string builder;
        bool status;

        string_create(&builder);

        status = jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);
        EXPECT_TRUE(status);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        status = jak_carbon_commit_hash(&rev, &rev_doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        status = jak_carbon_is_up_to_date(&doc);
        EXPECT_FALSE(status);

        status = jak_carbon_is_up_to_date(&rev_doc);
        EXPECT_TRUE(status);

        jak_carbon_to_str(&builder, JAK_JSON_EXTENDED, &doc);
        // printf("%s\n", string_builder_cstr(&builder));
        string_drop(&builder);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CreateCarbonRevisionAbort) {
        jak_carbon doc, rev_doc;
        jak_u64 rev;
        struct jak_string builder;
        bool status;

        string_create(&builder);

        status = jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);
        EXPECT_TRUE(status);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_abort(&revise);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        jak_carbon_to_str(&builder, JAK_JSON_EXTENDED, &doc);
        // printf("%s\n", string_builder_cstr(&builder));
        string_drop(&builder);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CreateCarbonRevisionAsyncReading) {
        jak_carbon doc, rev_doc;
        jak_u64 rev;
        struct jak_string builder;
        bool status;

        string_create(&builder);

        status = jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);
        EXPECT_TRUE(status);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct jak_carbon_revise revise;
        carbon_revise_begin(&revise, &rev_doc, &doc);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        carbon_revise_end(&revise);

        status = jak_carbon_commit_hash(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        jak_carbon_to_str(&builder, JAK_JSON_EXTENDED, &doc);
        // printf("%s\n", string_builder_cstr(&builder));
        string_drop(&builder);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, ModifyCarbonObjectId) {
        jak_carbon doc, rev_doc;
        jak_global_id_t oid;
        jak_global_id_t new_oid;
        struct jak_carbon_revise revise;
        jak_u64 commit_hash_old, commit_hash_new;

        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_AUTOKEY);

        jak_carbon_key_unsigned_value(&oid, &doc);
        EXPECT_EQ(oid, 0);

        jak_carbon_commit_hash(&commit_hash_old, &doc);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_generate(&new_oid, &revise);
        EXPECT_NE(oid, new_oid);
        carbon_revise_end(&revise);

        jak_carbon_commit_hash(&commit_hash_new, &rev_doc);
        EXPECT_NE(commit_hash_old, commit_hash_new);

        jak_carbon_key_unsigned_value(&oid, &rev_doc);
        EXPECT_EQ(oid, new_oid);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonArrayIteratorOpenAfterNew) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;

        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_AUTOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_generate(NULL, &revise);
        carbon_revise_iterator_open(&it, &revise);
        bool has_next = jak_carbon_array_it_next(&it);
        EXPECT_EQ(has_next, false);
        carbon_revise_end(&revise);
        jak_carbon_array_it_drop(&it);

        // jak_carbon_print(stdout, &rev_doc);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonArrayIteratorInsertNullAfterNew) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_AUTOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        carbon_revise_key_generate(NULL, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_drop(&inserter);
        carbon_revise_end(&revise);
        jak_carbon_array_it_drop(&it);

        // jak_carbon_print(stdout, &rev_doc);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonArrayIteratorInsertMultipleLiteralsAfterNewNoOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 10; i++) {
                // fprintf(stdout, "before:\n");
                //jak_carbon_hexdump_print(stdout, &rev_doc);
                bool status;
                if (i % 3 == 0) {
                        status = jak_carbon_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        status = jak_carbon_insert_true(&inserter);
                } else {
                        status = jak_carbon_insert_false(&inserter);
                }
                ASSERT_TRUE(status);
                // fprintf(stdout, "after:\n");
                //jak_carbon_hexdump_print(stdout, &rev_doc);
                // fprintf(stdout, "\n\n");
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonArrayIteratorOverwriteLiterals) {
        jak_carbon doc, rev_doc, rev_doc2;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 3; i++) {
                if (i % 3 == 0) {
                        jak_carbon_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        jak_carbon_insert_true(&inserter);
                } else {
                        jak_carbon_insert_false(&inserter);
                }
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 2; i++) {
                jak_carbon_insert_true(&inserter);
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc2);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);
}

TEST(CarbonTest, CarbonArrayIteratorOverwriteLiteralsWithDocOverflow) {
        jak_carbon doc, rev_doc, rev_doc2;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 22; i++) {
                if (i % 3 == 0) {
                        jak_carbon_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        jak_carbon_insert_true(&inserter);
                } else {
                        jak_carbon_insert_false(&inserter);
                }
               // fprintf(stdout, "after initial push:\n");
               // //jak_carbon_hexdump_print(stdout, &rev_doc);
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 2; i++) {
                // fprintf(stdout, "before:\n");
                //jak_carbon_hexdump_print(stdout, &rev_doc2);
                jak_carbon_insert_true(&inserter);
                // fprintf(stdout, "after:\n");
                //jak_carbon_hexdump_print(stdout, &rev_doc2);
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);
        // jak_carbon_print(stdout, &rev_doc2);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);
}

TEST(CarbonTest, CarbonArrayIteratorUnsignedAndConstants) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 500; i++) {
                if (i % 6 == 0) {
                        jak_carbon_insert_null(&inserter);
                } else if (i % 6 == 1) {
                        jak_carbon_insert_true(&inserter);
                } else if (i % 6 == 2) {
                        jak_carbon_insert_false(&inserter);
                } else if (i % 6 == 3) {
                        jak_u64 rand_value = random();
                        jak_carbon_insert_unsigned(&inserter, rand_value);
                } else if (i % 6 == 4) {
                        jak_i64 rand_value = random();
                        jak_carbon_insert_signed(&inserter, rand_value);
                } else {
                        float rand_value = (float)rand()/(float)(RAND_MAX/INT32_MAX);
                        jak_carbon_insert_float(&inserter, rand_value);
                }
                //fprintf(stdout, "after initial push:\n");
                ////jak_carbon_hexdump_print(stdout, &rev_doc);
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonArrayIteratorStrings) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        for (jak_i32 i = 0; i < 10; i++) {
                jak_u64 strlen = rand() % (100 + 1 - 4) + 4;
                char buffer[strlen];
                for (jak_u64 j = 0; j < strlen; j++) {
                        buffer[j] = 65 + (rand() % 25);
                }
                buffer[0] = '!';
                buffer[strlen - 2] = '!';
                buffer[strlen - 1] = '\0';
                jak_carbon_insert_string(&inserter, buffer);
                //fprintf(stdout, "after initial push:\n");
                ////jak_carbon_hexdump_print(stdout, &rev_doc);
        }
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertMimeTypedBlob) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        const char *data = "{ \"Message\": \"Hello World\" }";
        bool status = jak_carbon_insert_binary(&inserter, data, strlen(data), "json", NULL);
        ASSERT_TRUE(status);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertCustomTypedBlob) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        const char *data = "{ \"Message\": \"Hello World\" }";
        bool status = jak_carbon_insert_binary(&inserter, data, strlen(data), NULL, "my data");
        ASSERT_TRUE(status);
        ////jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertTwoMimeTypedBlob) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        bool status = jak_carbon_insert_binary(&inserter, data1, strlen(data1), "json", NULL);
        ASSERT_TRUE(status);
        status = jak_carbon_insert_binary(&inserter, data2, strlen(data2), "txt", NULL);
        ASSERT_TRUE(status);
        ////jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertMimeTypedBlobsWithOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        for (jak_u32 i = 0; i < 100; i++) {
                bool status = jak_carbon_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
                        strlen(i % 2 == 0 ? data1 : data2), "json", NULL);
                ASSERT_TRUE(status);
        }
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertMixedTypedBlobsWithOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        for (jak_u32 i = 0; i < 100; i++) {
                bool status = jak_carbon_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
                        strlen(i % 2 == 0 ? data1 : data2), i % 3 == 0 ? "json" : NULL, i % 5 == 0 ? "user/app" : NULL);
                ASSERT_TRUE(status);
        }
        ////jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertArrayWithNoOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert *nested_inserter = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        ASSERT_TRUE(nested_inserter != NULL);
        jak_carbon_insert_array_end(&array_state);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertValuesIntoNestedArrayWithNoOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);

        jak_carbon_insert *nested_inserter = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        ASSERT_TRUE(nested_inserter != NULL);
        jak_carbon_insert_true(nested_inserter);
        jak_carbon_insert_true(nested_inserter);
        jak_carbon_insert_true(nested_inserter);
        jak_carbon_insert_array_end(&array_state);

        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);


        //jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsert2xNestedArrayWithNoOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state_l1, array_state_l2;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_array_begin(&array_state_l1, &inserter, 10);
        ASSERT_TRUE(nested_inserter_l1 != NULL);
        jak_carbon_insert_true(nested_inserter_l1);
        jak_carbon_insert_true(nested_inserter_l1);
        jak_carbon_insert_true(nested_inserter_l1);

        jak_carbon_insert *nested_inserter_l2 = jak_carbon_insert_array_begin(&array_state_l2, nested_inserter_l1, 10);
        ASSERT_TRUE(nested_inserter_l2 != NULL);
        jak_carbon_insert_true(nested_inserter_l2);
        jak_carbon_insert_false(nested_inserter_l2);
        jak_carbon_insert_null(nested_inserter_l2);
        jak_carbon_insert_array_end(&array_state_l2);

        jak_carbon_insert_array_end(&array_state_l1);

        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);


        //jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertXxNestedArrayWithoutOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state_l1;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);
        jak_carbon_insert_null(&inserter);

        for (int i = 0; i < 10; i++) {
                jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_array_begin(&array_state_l1, &inserter, 10);
                ASSERT_TRUE(nested_inserter_l1 != NULL);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_array_end(&array_state_l1);
        }

        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertXxNestedArrayWithOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state_l1;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        // printf("\n");

        jak_carbon_insert_null(&inserter);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        // printf("\n");

        jak_carbon_insert_null(&inserter);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        // printf("\n");

        jak_carbon_insert_null(&inserter);

        for (int i = 0; i < 10; i++) {
                jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_array_begin(&array_state_l1, &inserter, 1);
                ASSERT_TRUE(nested_inserter_l1 != NULL);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_true(nested_inserter_l1);
                jak_carbon_insert_array_end(&array_state_l1);
        }

        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);
        jak_carbon_insert_false(&inserter);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertColumnWithoutOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        jak_carbon_insert_u8(nested_inserter_l1, 1);
        jak_carbon_insert_u8(nested_inserter_l1, 2);
        jak_carbon_insert_u8(nested_inserter_l1, 3);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertColumnNumbersWithoutOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        jak_carbon_insert_u8(nested_inserter_l1, 42);
        jak_carbon_insert_u8(nested_inserter_l1, 43);
        jak_carbon_insert_u8(nested_inserter_l1, 44);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[42, 43, 44]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertColumnNumbersZeroWithoutOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        jak_carbon_insert_u8(nested_inserter_l1, 0);
        jak_carbon_insert_u8(nested_inserter_l1, 0);
        jak_carbon_insert_u8(nested_inserter_l1, 0);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[0, 0, 0]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertMultileTypedColumnsWithoutOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;
        jak_carbon_insert *ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U16, 10);
        jak_carbon_insert_u16(ins, 4);
        jak_carbon_insert_u16(ins, 5);
        jak_carbon_insert_u16(ins, 6);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(ins, 7);
        jak_carbon_insert_u32(ins, 8);
        jak_carbon_insert_u32(ins, 9);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U64, 10);
        jak_carbon_insert_u64(ins, 10);
        jak_carbon_insert_u64(ins, 11);
        jak_carbon_insert_u64(ins, 12);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I8, 10);
        jak_carbon_insert_i8(ins, -1);
        jak_carbon_insert_i8(ins, -2);
        jak_carbon_insert_i8(ins, -3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I16, 10);
        jak_carbon_insert_i16(ins, -4);
        jak_carbon_insert_i16(ins, -5);
        jak_carbon_insert_i16(ins, -6);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I32, 10);
        jak_carbon_insert_i32(ins, -7);
        jak_carbon_insert_i32(ins, -8);
        jak_carbon_insert_i32(ins, -9);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I64, 10);
        jak_carbon_insert_i64(ins, -10);
        jak_carbon_insert_i64(ins, -11);
        jak_carbon_insert_i64(ins, -12);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_FLOAT, 10);
        jak_carbon_insert_float(ins, 42.0f);
        jak_carbon_insert_float(ins, 21.0f);
        jak_carbon_insert_float(ins, 23.4221f);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        //jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        //string_builder_print(&sb);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12], [-1, -2, -3], [-4, -5, -6], [-7, -8, -9], [-10, -11, -12], [42.00, 21.00, 23.42]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertColumnNumbersZeroWithOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,16, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 1);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        jak_carbon_insert_u8(nested_inserter_l1, 1);
        jak_carbon_insert_u8(nested_inserter_l1, 2);
        jak_carbon_insert_u8(nested_inserter_l1, 3);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // printf("Carbon DOC PRINT:");
        // jak_carbon_print(stdout, &rev_doc);
        // fflush(stdout);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertColumnNumbersWithHighOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,16, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U32, 1);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        for (jak_u32 i = 0; i < 100; i++) {
                jak_carbon_insert_u32(nested_inserter_l1, i);
                jak_carbon_insert_u32(nested_inserter_l1, i);
                jak_carbon_insert_u32(nested_inserter_l1, i);
        }

        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // printf("Carbon DOC PRINT:");
        // jak_carbon_print(stdout, &rev_doc);
        // fflush(stdout);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 82, 82, 82, 83, 83, 83, 84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 88, 88, 88, 89, 89, 89, 90, 90, 90, 91, 91, 91, 92, 92, 92, 93, 93, 93, 94, 94, 94, 95, 95, 95, 96, 96, 96, 97, 97, 97, 98, 98, 98, 99, 99, 99]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertInsertMultipleColumnsNumbersWithHighOverflow) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,16, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        for (jak_u32 k = 0; k < 3; k++) {
                jak_carbon_insert *nested_inserter_l1 = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U32, 1);

                ASSERT_TRUE(nested_inserter_l1 != NULL);
                for (jak_u32 i = 0; i < 4; i++) {
                        jak_carbon_insert_u32(nested_inserter_l1, 'a' + i);
                        jak_carbon_insert_u32(nested_inserter_l1, 'a' + i);
                        jak_carbon_insert_u32(nested_inserter_l1, 'a' + i);
                }

                jak_carbon_insert_column_end(&column_state);
        }

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        ////jak_carbon_hexdump_print(stdout, &rev_doc);

        // printf("Carbon DOC PRINT:");
        // jak_carbon_print(stdout, &rev_doc);
        // fflush(stdout);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonInsertNullTest) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;
        jak_carbon_insert *ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, U8_NULL);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U16, 10);
        jak_carbon_insert_u16(ins, 4);
        jak_carbon_insert_u16(ins, U16_NULL);
        jak_carbon_insert_u16(ins, 6);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(ins, 7);
        jak_carbon_insert_u32(ins, U32_NULL);
        jak_carbon_insert_u32(ins, 9);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U64, 10);
        jak_carbon_insert_u64(ins, 10);
        jak_carbon_insert_u64(ins, U64_NULL);
        jak_carbon_insert_u64(ins, 12);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I8, 10);
        jak_carbon_insert_i8(ins, -1);
        jak_carbon_insert_i8(ins, I8_NULL);
        jak_carbon_insert_i8(ins, -3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I16, 10);
        jak_carbon_insert_i16(ins, -4);
        jak_carbon_insert_i16(ins, I16_NULL);
        jak_carbon_insert_i16(ins, -6);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I32, 10);
        jak_carbon_insert_i32(ins, -7);
        jak_carbon_insert_i32(ins, I32_NULL);
        jak_carbon_insert_i32(ins, -9);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I64, 10);
        jak_carbon_insert_i64(ins, -10);
        jak_carbon_insert_i64(ins, I64_NULL);
        jak_carbon_insert_i64(ins, -12);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_FLOAT, 10);
        jak_carbon_insert_float(ins, 42.0f);
        jak_carbon_insert_float(ins, FLOAT_NULL);
        jak_carbon_insert_float(ins, 23.4221f);
        jak_carbon_insert_column_end(&column_state);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, null, 3], [4, null, 6], [7, null, 9], [10, null, 12], [-1, null, -3], [-4, null, -6], [-7, null, -9], [-10, null, -12], [42.00, null, 23.42]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonShrinkColumnListTest) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;
        jak_carbon_insert *ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_true(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_BOOLEAN, 10);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_false(ins);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U8, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, U8_NULL);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U16, 10);
        jak_carbon_insert_u16(ins, 3);
        jak_carbon_insert_u16(ins, U16_NULL);
        jak_carbon_insert_u16(ins, 4);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(ins, 5);
        jak_carbon_insert_u32(ins, U32_NULL);
        jak_carbon_insert_u32(ins, 6);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_U64, 10);
        jak_carbon_insert_u64(ins, 7);
        jak_carbon_insert_u64(ins, U64_NULL);
        jak_carbon_insert_u64(ins, 8);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I8, 10);
        jak_carbon_insert_i8(ins, 9);
        jak_carbon_insert_i8(ins, I8_NULL);
        jak_carbon_insert_i8(ins, 10);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I16, 10);
        jak_carbon_insert_i16(ins, 11);
        jak_carbon_insert_i16(ins, I16_NULL);
        jak_carbon_insert_i16(ins, 12);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I32, 10);
        jak_carbon_insert_i32(ins, 13);
        jak_carbon_insert_i32(ins, I32_NULL);
        jak_carbon_insert_i32(ins, 14);
        jak_carbon_insert_column_end(&column_state);

        ins = jak_carbon_insert_column_begin(&column_state, &inserter, JAK_CARBON_COLUMN_TYPE_I64, 10);
        jak_carbon_insert_i64(ins, 15);
        jak_carbon_insert_i64(ins, I64_NULL);
        jak_carbon_insert_i64(ins, 16);
        jak_carbon_insert_column_end(&column_state);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        carbon_revise_shrink(&revise);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [true, true, true], [false, false, false], [1, null, 2], [3, null, 4], [5, null, 6], [7, null, 8], [9, null, 10], [11, null, 12], [13, null, 14], [15, null, 16]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonShrinkArrayListTest) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state;
        jak_carbon_insert *ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_array_end(&array_state);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 2);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_u8(ins, 4);
        jak_carbon_insert_array_end(&array_state);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 5);
        jak_carbon_insert_u8(ins, 6);
        jak_carbon_insert_u8(ins, 7);
        jak_carbon_insert_array_end(&array_state);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        carbon_revise_shrink(&revise);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 1, 1], [2, 3, 4], [5, 6, 7]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonShrinkNestedArrayListTest) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_array_state array_state, nested_array_state;
        jak_carbon_insert *ins, *nested_ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_array_end(&array_state);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 2);
        nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_u8(ins, 3);
        jak_carbon_insert_u8(ins, 4);
        jak_carbon_insert_array_end(&array_state);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 5);
        jak_carbon_insert_u8(ins, 6);
        nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_u8(ins, 7);
        jak_carbon_insert_array_end(&array_state);

        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        jak_carbon_insert_u8(ins, 8);
        jak_carbon_insert_u8(ins, 9);
        jak_carbon_insert_u8(ins, 10);
        nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_array_end(&array_state);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        carbon_revise_shrink(&revise);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);
        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[[\"Hello\", \"World\"], 1, 1, 1], [2, [\"Hello\", \"World\"], 3, 4], [5, 6, [\"Hello\", \"World\"], 7], [8, 9, 10, [\"Hello\", \"World\"]]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonShrinkNestedArrayListAndColumnListTest) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        jak_carbon_insert_column_state column_state;
        jak_carbon_insert_array_state array_state, nested_array_state;
        jak_carbon_insert *ins, *nested_ins, *column_ins;

        jak_carbon_create_empty_ex(&doc, JAK_CARBON_KEY_NOKEY,20, 1);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u64(&inserter, 4223);
        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
                column_ins = jak_carbon_insert_column_begin(&column_state, ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
                        jak_carbon_insert_u32(column_ins, 'X');
                        jak_carbon_insert_u32(column_ins, 'Y');
                        jak_carbon_insert_u32(column_ins, 'Z');
                jak_carbon_insert_column_end(&column_state);
                nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
                        jak_carbon_insert_string(nested_ins, "Hello");
                        column_ins = jak_carbon_insert_column_begin(&column_state, nested_ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
                                jak_carbon_insert_u32(column_ins, 'A');
                                jak_carbon_insert_u32(column_ins, 'B');
                                jak_carbon_insert_u32(column_ins, 'C');
                        jak_carbon_insert_column_end(&column_state);
                        jak_carbon_insert_string(nested_ins, "World");
                jak_carbon_insert_array_end(&nested_array_state);
                jak_carbon_insert_u8(ins, 1);
                jak_carbon_insert_u8(ins, 1);
                column_ins = jak_carbon_insert_column_begin(&column_state, ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
                        jak_carbon_insert_u32(column_ins, 23);
                        jak_carbon_insert_u32(column_ins, 24);
                        jak_carbon_insert_u32(column_ins, 25);
                jak_carbon_insert_column_end(&column_state);
                jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_array_end(&array_state);

        //jak_carbon_hexdump_print(stdout, &rev_doc);
        carbon_revise_shrink(&revise);
        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        // jak_carbon_print(stdout, &rev_doc);

        struct jak_string sb;
        string_create(&sb);
        jak_carbon_to_str(&sb, JAK_JSON_EXTENDED, &rev_doc);

        // fprintf(stdout, "IST  %s\n", string_builder_cstr(&sb));
        // fprintf(stdout, "SOLL {\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}\n");

        ASSERT_TRUE(0 == strcmp(string_cstr(&sb), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}"));
        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonDotNotation) {
        jak_carbon_dot_path path;
        struct jak_string sb;
        string_create(&sb);

        jak_carbon_dot_path_create(&path);

        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_key(&path, "name");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_key(&path, "my name");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name.\"my name\"") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_key(&path, "");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name.\"my name\".\"\"") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_idx(&path, 42);
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name.\"my name\".\"\".42") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_idx(&path, 23);
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name.\"my name\".\"\".42.23") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_add_key(&path, "\"already quotes\"");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name.\"my name\".\"\".42.23.\"already quotes\"") == 0);
        string_clear(&sb);

        jak_carbon_dot_path_drop(&path);
        string_drop(&sb);
}

TEST(CarbonTest, CarbonDotNotationParsing) {
        jak_carbon_dot_path path;
        struct jak_string sb;
        string_create(&sb);

        jak_carbon_dot_path_from_string(&path, "name");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "   name");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "   name    ");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "\"name\"");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "name") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "\"nam e\"");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "\"nam e\"") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "nam e");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "nam.e") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "\"My Doc\" names 5 age");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "\"My Doc\".names.5.age") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        jak_carbon_dot_path_from_string(&path, "23.authors.3.name");
        jak_carbon_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_cstr(&sb), "23.authors.3.name") == 0);
        string_clear(&sb);
        jak_carbon_dot_path_drop(&path);

        string_drop(&sb);
}

TEST(CarbonTest, CarbonFind) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert ins;
        jak_carbon_find finder;
        jak_u64 result_unsigned;
        jak_carbon_field_type_e type;
        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);

        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&ins, &it);
        jak_carbon_insert_u8(&ins, 'a');
        jak_carbon_insert_u8(&ins, 'b');
        jak_carbon_insert_u8(&ins, 'c');
        jak_carbon_array_it_insert_end(&ins);
        carbon_revise_iterator_close(&it);

        carbon_revise_end(&revise);

        {
                jak_carbon_find_open(&finder, "0", &rev_doc);

                ASSERT_TRUE(jak_carbon_find_has_result(&finder));

                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);

                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'a');

                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1", &rev_doc);

                ASSERT_TRUE(jak_carbon_find_has_result(&finder));

                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);

                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'b');

                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "2", &rev_doc);

                ASSERT_TRUE(jak_carbon_find_has_result(&finder));

                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);

                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'c');

                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "3", &rev_doc);

                ASSERT_FALSE(jak_carbon_find_has_result(&finder));

                jak_carbon_find_close(&finder);
        }

        // jak_carbon_print(stdout, &rev_doc);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonFindTypes) {
        jak_carbon doc, rev_doc;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter, *ins, *nested_ins, *column_ins;
        jak_carbon_insert_column_state column_state;
        jak_carbon_insert_array_state array_state, nested_array_state;
        jak_carbon_find finder;
        jak_u64 result_unsigned;
        jak_carbon_field_type_e type;
        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u64(&inserter, 4223);
        ins = jak_carbon_insert_array_begin(&array_state, &inserter, 10);
        column_ins = jak_carbon_insert_column_begin(&column_state, ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 'X');
        jak_carbon_insert_u32(column_ins, 'Y');
        jak_carbon_insert_u32(column_ins, 'Z');
        jak_carbon_insert_column_end(&column_state);
        nested_ins = jak_carbon_insert_array_begin(&nested_array_state, ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        column_ins = jak_carbon_insert_column_begin(&column_state, nested_ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 'A');
        jak_carbon_insert_u32(column_ins, 'B');
        jak_carbon_insert_u32(column_ins, 'C');
        jak_carbon_insert_column_end(&column_state);
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_u8(ins, 1);
        column_ins = jak_carbon_insert_column_begin(&column_state, ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 23);
        jak_carbon_insert_u32(column_ins, 24);
        jak_carbon_insert_u32(column_ins, 25);
        jak_carbon_insert_column_end(&column_state);
        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_array_end(&array_state);

        carbon_revise_shrink(&revise);


        //jak_carbon_print(stdout, &rev_doc);

        {
                jak_carbon_find_open(&finder, "0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U64);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 4223);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_ARRAY);
                jak_carbon_array_it *retval = jak_carbon_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_TRUE(type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                jak_carbon_column_it *retval = jak_carbon_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.0.0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 88);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.0.1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 89);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.0.2", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 90);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.0.3", &rev_doc);
                ASSERT_FALSE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_ARRAY);
                jak_carbon_array_it *retval = jak_carbon_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_STRING);
                jak_u64 str_len;
                const char *retval = jak_carbon_find_result_string(&str_len, &finder);
                ASSERT_TRUE(strncmp(retval, "Hello", str_len) == 0);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_TRUE(type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                jak_carbon_column_it *retval = jak_carbon_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.1.0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 65);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.1.1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 66);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.1.2", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 67);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.1.3", &rev_doc);
                ASSERT_FALSE(jak_carbon_find_has_result(&finder));
        }

        {
                jak_carbon_find_open(&finder, "1.1.2", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_STRING);
                jak_u64 str_len;
                const char *retval = jak_carbon_find_result_string(&str_len, &finder);
                ASSERT_TRUE(strncmp(retval, "World", str_len) == 0);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.1.3", &rev_doc);
                ASSERT_FALSE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.2", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.3", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.4", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_TRUE(type == JAK_CARBON_FIELD_TYPE_COLUMN_U8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_U64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I8 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I16 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I32 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_I64 ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_FLOAT ||
                        type == JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
                jak_carbon_column_it *retval = jak_carbon_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.4.0", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 23);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.4.1", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 24);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.4.2", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 25);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.4.3", &rev_doc);
                ASSERT_FALSE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.5", &rev_doc);
                ASSERT_TRUE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_result_type(&type, &finder);
                ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
                jak_carbon_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                jak_carbon_find_close(&finder);
        }

        {
                jak_carbon_find_open(&finder, "1.6", &rev_doc);
                ASSERT_FALSE(jak_carbon_find_has_result(&finder));
                jak_carbon_find_close(&finder);
        }

        jak_carbon_insert_drop(&inserter);
        jak_carbon_array_it_drop(&it);
        carbon_revise_end(&revise);

        //jak_carbon_hexdump_print(stdout, &rev_doc);

        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonUpdateU8Simple)
{
        jak_carbon doc, rev_doc, rev_doc2, rev_doc3, rev_doc4;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        struct jak_string sb;
        const char *json;

        string_create(&sb);
        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u8(&inserter, 'X');

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc);
        // printf("JSON (rev1): %s\n", json);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        carbon_update_set_u8(&revise, "0", 'Y');

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc2);
        // printf("JSON (rev2): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [89]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc3, &rev_doc2);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u8(&inserter, 'A');
        jak_carbon_insert_u8(&inserter, 'B');
        carbon_update_set_u8(&revise, "2", 'C');

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc3);
        // printf("JSON (rev3): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [65, 66, 67]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc4, &rev_doc3);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        carbon_update_set_u8(&revise, "0", 1);
        carbon_update_set_u8(&revise, "1", 2);
        carbon_update_set_u8(&revise, "2", 3);

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc4);
        // printf("JSON (rev4): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, 2, 3]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);
        jak_carbon_drop(&rev_doc3);
        jak_carbon_drop(&rev_doc4);
}

TEST(CarbonTest, CarbonUpdateMixedFixedTypesSimple)
{
        jak_carbon doc, rev_doc, rev_doc2;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        struct jak_string sb;
        const char *json;

        string_create(&sb);
        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u8(&inserter, 1);
        jak_carbon_insert_i64(&inserter, -42);
        jak_carbon_insert_float(&inserter, 23);

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc);
        // printf("JSON (rev1): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, -42, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        carbon_update_set_i64(&revise, "1", 1024);

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc2);
        // printf("JSON (rev2): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, 1024, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        jak_u64 e1 = jak_carbon_get_or_default_unsigned(&rev_doc2, "0", 0);
        jak_i64 e2 = jak_carbon_get_or_default_signed(&rev_doc2, "1", 0);
        float e3 = jak_carbon_get_or_default_float(&rev_doc2, "2", NAN);

        ASSERT_EQ(e1, 1);
        ASSERT_EQ(e2, 1024);
        ASSERT_TRUE(e3 > 22.9f && e3 < 24.0f);

        // -------------------------------------------------------------------------------------------------------------

        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);

}

TEST(CarbonTest, CarbonRemoveConstantsToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_null(ins);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [null]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstConstants)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_true(ins);
        jak_carbon_insert_false(ins);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_FALSE);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [false]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastConstants)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_true(ins);
        jak_carbon_insert_false(ins);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleConstants)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_true(ins);
        jak_carbon_insert_null(ins);
        jak_carbon_insert_false(ins);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_FALSE);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true, null, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true, false]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveNumberToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_u8(ins, 42);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstNumber)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_u8(ins, 42);
        jak_carbon_insert_u32(ins, 23);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [23]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastNumber)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_u8(ins, 42);
        jak_carbon_insert_u32(ins, 23);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleNumber)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_u8(ins, 42);
        jak_carbon_insert_u16(ins, 21);
        jak_carbon_insert_u32(ins, 23);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NUMBER_U32);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42, 21, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42, 23]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}


TEST(CarbonTest, CarbonRemoveStringToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_string(ins, "Hello");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstString)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_STRING);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"World\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastString)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleString)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        jak_carbon_insert_string(ins, "Plato");
        jak_carbon_insert_string(ins, "Kant");
        jak_carbon_insert_string(ins, "Nietzsche");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_STRING);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Plato\", \"Kant\", \"Nietzsche\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Plato\", \"Nietzsche\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}





TEST(CarbonTest, CarbonRemoveBinaryToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data, strlen(data), "txt", NULL);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_BINARY);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), "json", NULL);

        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
        jak_carbon_insert_binary(ins, data3, strlen(data3), "html", NULL);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_BINARY);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}











TEST(CarbonTest, CarbonRemoveCustomBinaryToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data = "ABC";
        jak_carbon_insert_binary(ins, data, strlen(data), NULL, "123");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        //jak_carbon_hexdump_print(stdout, &doc);

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"123\", \"encoding\": \"base64\", \"binary-string\": \"A=JDAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstCustomBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastCustomBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleCustomBinary)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        jak_carbon_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        jak_carbon_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
        jak_carbon_insert_binary(ins, data3, strlen(data3), NULL, "my-other-nonstandard-format");

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_BINARY_CUSTOM);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}










TEST(CarbonTest, CarbonRemoveArrayToEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        jak_carbon_insert_array_state state;
        jak_carbon_insert *array_ins;
        bool has_next;
        string_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 1);
        jak_carbon_insert_u8(array_ins, 2);
        jak_carbon_insert_u8(array_ins, 3);
        jak_carbon_insert_array_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        //jak_carbon_hexdump_print(stdout, &doc);

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveFirstArray)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_array_state state;
        jak_carbon_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 1);
        jak_carbon_insert_u8(array_ins, 2);
        jak_carbon_insert_u8(array_ins, 3);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 4);
        jak_carbon_insert_u8(array_ins, 5);
        jak_carbon_insert_u8(array_ins, 6);
        jak_carbon_insert_array_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e next_type;
        jak_carbon_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, JAK_CARBON_FIELD_TYPE_ARRAY);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[4, 5, 6]]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveLastArray)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_array_state state;
        jak_carbon_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 1);
        jak_carbon_insert_u8(array_ins, 2);
        jak_carbon_insert_u8(array_ins, 3);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 4);
        jak_carbon_insert_u8(array_ins, 5);
        jak_carbon_insert_u8(array_ins, 6);
        jak_carbon_insert_array_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3]]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonRemoveMiddleArray)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_array_state state;
        jak_carbon_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 1);
        jak_carbon_insert_u8(array_ins, 2);
        jak_carbon_insert_u8(array_ins, 3);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 4);
        jak_carbon_insert_u8(array_ins, 5);
        jak_carbon_insert_u8(array_ins, 6);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 7);
        jak_carbon_insert_u8(array_ins, 8);
        jak_carbon_insert_u8(array_ins, 9);
        jak_carbon_insert_array_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_field_type_e type;
        jak_carbon_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_ARRAY);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        // printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [4, 5, 6], [7, 8, 9]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3], [7, 8, 9]]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonColumnRemoveTest)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        bool status;
        const jak_u16 *values;

        jak_carbon_insert_column_state state;
        jak_carbon_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_KEEP);

        array_ins = jak_carbon_insert_column_begin(&state, ins, JAK_CARBON_COLUMN_TYPE_U16, 10);
        jak_carbon_insert_u16(array_ins, 1);
        jak_carbon_insert_u16(array_ins, 2);
        jak_carbon_insert_u16(array_ins, 3);
        jak_carbon_insert_column_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        jak_carbon_column_it *cit = jak_carbon_array_it_column_value(&rev_it);
        jak_carbon_field_type_e type;
        jak_u32 num_elems;
        jak_carbon_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
        ASSERT_EQ(num_elems, 3);

        status = jak_carbon_column_it_remove(cit, 1);
        ASSERT_TRUE(status);
        jak_carbon_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
        ASSERT_EQ(num_elems, 2);
        values = jak_carbon_column_it_u16_values(&num_elems, cit);
        ASSERT_EQ(values[0], 1);
        ASSERT_EQ(values[1], 3);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        status = jak_carbon_column_it_remove(cit, 0);
        ASSERT_TRUE(status);
        jak_carbon_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
        ASSERT_EQ(num_elems, 1);
        values = jak_carbon_column_it_u16_values(&num_elems, cit);
        ASSERT_EQ(values[0], 3);

        char *json_3 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        status = jak_carbon_column_it_remove(cit, 0);
        ASSERT_TRUE(status);
        jak_carbon_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_COLUMN_U16);
        ASSERT_EQ(num_elems, 0);

        char *json_4 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        // -------------------------------------------------------------------------------------------------------------


        // printf(">> %s\n", json_1);
        // printf(">> %s\n", json_2);
        // printf(">> %s\n", json_3);
        // printf(">> %s\n", json_4);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 2, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[1, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[3]]}") == 0);
        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[]]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
        free(json_3);
        free(json_4);
}

TEST(CarbonTest, CarbonRemoveComplexTest)
{
        jak_carbon doc, rev_doc, rev_doc2, rev_doc3, rev_doc4, rev_doc5, rev_doc6, rev_doc7, rev_doc8, rev_doc9,
                rev_doc10, rev_doc11, rev_doc12, rev_doc13, rev_doc14;
        jak_carbon_new context;
        struct jak_string sb;
        string_create(&sb);

        jak_carbon_insert_array_state state, state2, state3;
        jak_carbon_insert_column_state cstate;
        jak_carbon_insert *array_ins, *array_ins2, *array_ins3, *column_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_u8(ins, 1);
        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_u16(ins, 2);
        jak_carbon_insert_u32(ins, 3);
        jak_carbon_insert_u64(ins, 3);
        jak_carbon_insert_string(ins, "World");

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        jak_carbon_insert_u8(array_ins, 4);
        jak_carbon_insert_string(array_ins, "Fox!");
        jak_carbon_insert_u8(array_ins, 6);
        jak_carbon_insert_array_end(&state);

        array_ins = jak_carbon_insert_array_begin(&state, ins, 10);
        array_ins2 = jak_carbon_insert_array_begin(&state2, array_ins, 10);
        jak_carbon_insert_array_end(&state2);
        array_ins2 = jak_carbon_insert_array_begin(&state2, array_ins, 10);
        jak_carbon_insert_u8(array_ins2, 4);
        jak_carbon_insert_array_end(&state2);
        jak_carbon_insert_null(array_ins);
        array_ins2 = jak_carbon_insert_array_begin(&state2, array_ins, 10);
        jak_carbon_insert_string(array_ins2, "Dog!");
        array_ins3 = jak_carbon_insert_array_begin(&state3, array_ins2, 10);
        jak_carbon_insert_array_end(&state3);
        array_ins3 = jak_carbon_insert_array_begin(&state3, array_ins2, 10);
        column_ins = jak_carbon_insert_column_begin(&cstate, array_ins3, JAK_CARBON_COLUMN_TYPE_U8, 10);

        jak_carbon_insert_u8(column_ins, 41);
        jak_carbon_insert_u8(column_ins, 42);
        jak_carbon_insert_u8(column_ins, 43);
        jak_carbon_insert_column_end(&cstate);

        jak_carbon_insert_array_end(&state3);
        array_ins3 = jak_carbon_insert_array_begin(&state3, array_ins2, 10);
        jak_carbon_insert_array_end(&state3);

        jak_carbon_insert_array_end(&state2);
        jak_carbon_insert_array_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));
        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("3", &rev_doc, &doc);
        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("5", &rev_doc2, &rev_doc);
        char *json_3 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc2));
        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("5.1", &rev_doc3, &rev_doc2);
        char *json_4 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc3));
        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("6.0", &rev_doc4, &rev_doc3);
        char *json_5 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc4));
        ASSERT_TRUE(strcmp(json_5, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("5", &rev_doc5, &rev_doc4);
        char *json_6 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc5));
        ASSERT_TRUE(strcmp(json_6, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("0", &rev_doc6, &rev_doc5);
        carbon_revise_remove_one("1", &rev_doc7, &rev_doc6);
        carbon_revise_remove_one("0", &rev_doc8, &rev_doc7);
        carbon_revise_remove_one("1", &rev_doc9, &rev_doc8);
        carbon_revise_remove_one("0", &rev_doc10, &rev_doc9);
        char *json_11 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc10));
        ASSERT_TRUE(strcmp(json_11, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("0.2.2.0", &rev_doc11, &rev_doc10);

        char *json_12 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc11));
        ASSERT_TRUE(strcmp(json_12, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[[4], null, [\"Dog!\", [], [], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("0.2.2", &rev_doc12, &rev_doc11);

        char *json_13 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc12));
        ASSERT_TRUE(strcmp(json_13, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[[4], null, [\"Dog!\", [], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("0.2", &rev_doc13, &rev_doc12);

        char *json_14 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc13));
        ASSERT_TRUE(strcmp(json_14, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[[4], null]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_remove_one("0", &rev_doc14, &rev_doc13);

        char *json_15 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc14));
        ASSERT_TRUE(strcmp(json_15, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);


        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);
        jak_carbon_drop(&rev_doc3);
        jak_carbon_drop(&rev_doc4);
        jak_carbon_drop(&rev_doc5);
        jak_carbon_drop(&rev_doc6);
        jak_carbon_drop(&rev_doc7);
        jak_carbon_drop(&rev_doc8);
        jak_carbon_drop(&rev_doc9);
        jak_carbon_drop(&rev_doc10);
        jak_carbon_drop(&rev_doc11);
        jak_carbon_drop(&rev_doc12);
        jak_carbon_drop(&rev_doc13);
        jak_carbon_drop(&rev_doc14);
        free(json_1);
        free(json_2);
        free(json_3);
        free(json_4);
        free(json_5);
        free(json_6);
        free(json_11);
        free(json_12);
        free(json_13);
        free(json_14);
        free(json_15);
}

TEST(CarbonTest, CarbonUpdateMixedFixedTypesTypeChangeSimple)
{
        jak_carbon doc, rev_doc, rev_doc2;
        struct jak_carbon_revise revise;
        jak_carbon_array_it it;
        jak_carbon_insert inserter;
        struct jak_string sb;
        const char *json;

        string_create(&sb);
        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&it, &revise);
        jak_carbon_array_it_insert_begin(&inserter, &it);

        jak_carbon_insert_u8(&inserter, 1);
        jak_carbon_insert_i64(&inserter, -42);
        jak_carbon_insert_float(&inserter, 23);

        jak_carbon_array_it_insert_end(&inserter);
        carbon_revise_iterator_close(&it);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, -42, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc2, &rev_doc);
        carbon_update_set_u32(&revise, "1", 1024);
        carbon_revise_end(&revise);


        json = jak_carbon_to_json_extended(&sb, &rev_doc2);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, 1024, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        string_drop(&sb);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        jak_carbon_drop(&rev_doc2);
}


TEST(CarbonTest, CarbonShrinkIssueFix)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonKeyTypeNoKey)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonKeyTypeNoKeyNoRevInc)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_u64 rev_old, rev_new;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_commit_hash(&rev_old, &doc);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        jak_carbon_commit_hash(&rev_new, &rev_doc);

        ASSERT_EQ(rev_old, 0);
        ASSERT_EQ(rev_new, rev_old);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonKeyTypeAutoKey)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonKeyTypeAutoKeyRevInc)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_u64 rev_old, rev_new;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_commit_hash(&rev_old, &doc);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        jak_carbon_commit_hash(&rev_new, &rev_doc);

        ASSERT_EQ(rev_old, rev_new);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonKeyTypeAutoKeyUpdate)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_global_id_t id, id_read;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_AUTOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------


        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_generate(&id, &revise);
        carbon_revise_end(&revise);

        jak_carbon_key_unsigned_value(&id_read, &rev_doc);
        ASSERT_NE(id, 0);
        ASSERT_EQ(id, id_read);

        // jak_carbon_print(stdout, &rev_doc);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonKeyTypeUnsignedKeyUpdate)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        struct jak_string sb;

        string_create(&sb);

        jak_u64 id_read;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_UKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------


        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_set_unsigned(&revise, 42);
        carbon_revise_end(&revise);

        jak_carbon_key_unsigned_value(&id_read, &rev_doc);
        ASSERT_EQ(id_read, 42);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        string_drop(&sb);
}

TEST(CarbonTest, CarbonKeyTypeSignedKeyUpdate)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        struct jak_string sb;

        string_create(&sb);

        jak_i64 id_read;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_IKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------


        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_set_signed(&revise, 42);
        carbon_revise_end(&revise);

        jak_carbon_key_signed_value(&id_read, &rev_doc);
        ASSERT_EQ(id_read, 42);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        string_drop(&sb);
}

TEST(CarbonTest, CarbonKeyTypeStringKeyUpdate)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        struct jak_string sb;

        string_create(&sb);

        jak_u64 key_len;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_SKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_key_set_string(&revise, "my_unique_id");
        carbon_revise_end(&revise);

        const char *key = jak_carbon_key_string_value(&key_len, &rev_doc);
        ASSERT_TRUE(strncmp(key, "my_unique_id", strlen("my_unique_id")) == 0);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        string_drop(&sb);
}

TEST(CarbonTest, CarbonKeyTypeUnsignedKey)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_UKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, &doc);
        ASSERT_EQ(key_type, JAK_CARBON_KEY_UKEY);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonKeyTypeSignedKeyRevInc)
{
        jak_carbon doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_u64 rev_old, rev_new;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_IKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_u64 test_max = 10000;

        struct jak_vector ofType(jak_carbon) files;
        vec_create(&files, NULL, sizeof(jak_carbon), test_max);
        jak_carbon* old_f = &doc;


        for (unsigned i = 0; i < test_max; i++) {

                jak_carbon_commit_hash(&rev_old, old_f);

                jak_carbon* new_f = vec_new_and_get(&files, jak_carbon);

                carbon_revise_begin(&revise, new_f, old_f);
                carbon_revise_end(&revise);

                jak_carbon_commit_hash(&rev_new, new_f);

                ASSERT_NE(rev_old, rev_new);

                old_f = new_f;
        }


}

TEST(CarbonTest, CarbonKeyTypeUnsignedKeyRevInc)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_u64 rev_old, rev_new;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_UKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_commit_hash(&rev_old, &doc);

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_end(&revise);

        jak_carbon_commit_hash(&rev_new, &rev_doc);

        ASSERT_NE(rev_old, rev_new);

        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
}

TEST(CarbonTest, CarbonKeyTypeSignedKey)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_IKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_key_e key_type;
        jak_carbon_key_type(&key_type, &doc);
        ASSERT_EQ(key_type, JAK_CARBON_KEY_IKEY);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonKeyTypeStringKey)
{
        jak_carbon doc;
        jak_carbon_new context;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_string(ins, "Hello");
        jak_carbon_insert_string(ins, "World");

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello\", \"World\"]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertEmpty)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertNull)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_null(obj_ins, "My Key");
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": null}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleNulls)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_null(obj_ins, "My Key 1");
        jak_carbon_insert_prop_null(obj_ins, "My Key 2");
        jak_carbon_insert_prop_null(obj_ins, "My Key 3");
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": null, \"My Key 2\": null, \"My Key 3\": null}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertU8)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_u8(obj_ins, "My Key", 123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": 123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleU8s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_u8(obj_ins, "My Key 1", 1);
        jak_carbon_insert_prop_u8(obj_ins, "My Key 2", 2);
        jak_carbon_insert_prop_u8(obj_ins, "My Key 3", 3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": 1, \"My Key 2\": 2, \"My Key 3\": 3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertU16)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_u16(obj_ins, "My Key", 123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": 123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleU16s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_u16(obj_ins, "My Key 1", 1);
        jak_carbon_insert_prop_u16(obj_ins, "My Key 2", 2);
        jak_carbon_insert_prop_u16(obj_ins, "My Key 3", 3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": 1, \"My Key 2\": 2, \"My Key 3\": 3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertU32)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_u32(obj_ins, "My Key", 123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": 123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleU32s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_u32(obj_ins, "My Key 1", 1);
        jak_carbon_insert_prop_u32(obj_ins, "My Key 2", 2);
        jak_carbon_insert_prop_u32(obj_ins, "My Key 3", 3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": 1, \"My Key 2\": 2, \"My Key 3\": 3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertU64)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_u64(obj_ins, "My Key", 123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": 123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleU64s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_u64(obj_ins, "My Key 1", 1);
        jak_carbon_insert_prop_u64(obj_ins, "My Key 2", 2);
        jak_carbon_insert_prop_u64(obj_ins, "My Key 3", 3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        //jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": 1, \"My Key 2\": 2, \"My Key 3\": 3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertI8)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_i8(obj_ins, "My Key", -123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": -123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleI8s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_i8(obj_ins, "My Key 1", -1);
        jak_carbon_insert_prop_i8(obj_ins, "My Key 2", -2);
        jak_carbon_insert_prop_i8(obj_ins, "My Key 3", -3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": -1, \"My Key 2\": -2, \"My Key 3\": -3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertI16)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_i16(obj_ins, "My Key", -123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": -123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleI16s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_i16(obj_ins, "My Key 1", -1);
        jak_carbon_insert_prop_i16(obj_ins, "My Key 2", -2);
        jak_carbon_insert_prop_i16(obj_ins, "My Key 3", -3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": -1, \"My Key 2\": -2, \"My Key 3\": -3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertI32)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_i32(obj_ins, "My Key", -123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": -123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleI32s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_i32(obj_ins, "My Key 1", -1);
        jak_carbon_insert_prop_i32(obj_ins, "My Key 2", -2);
        jak_carbon_insert_prop_i32(obj_ins, "My Key 3", -3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": -1, \"My Key 2\": -2, \"My Key 3\": -3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertI64)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_i64(obj_ins, "My Key", -123);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": -123}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleI64s)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_i64(obj_ins, "My Key 1", -1);
        jak_carbon_insert_prop_i64(obj_ins, "My Key 2", -2);
        jak_carbon_insert_prop_i64(obj_ins, "My Key 3", -3);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": -1, \"My Key 2\": -2, \"My Key 3\": -3}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertFloat)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_float(obj_ins, "My Key", -123.32);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": -123.32}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleFloats)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_float(obj_ins, "My Key 1", -1.23);
        jak_carbon_insert_prop_float(obj_ins, "My Key 2", -2.42);
        jak_carbon_insert_prop_float(obj_ins, "My Key 3", 3.21);
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": -1.23, \"My Key 2\": -2.42, \"My Key 3\": 3.21}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertTrue)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_true(obj_ins, "My Key");
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": true}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertFalse)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1024);
        jak_carbon_insert_prop_false(obj_ins, "My Key");
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key\": false}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleBooleans)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_true(obj_ins, "My Key 1");
        jak_carbon_insert_prop_false(obj_ins, "My Key 2");
        jak_carbon_insert_prop_true(obj_ins, "My Key 3");
        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"My Key 1\": true, \"My Key 2\": false, \"My Key 3\": true}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMixed)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);
        jak_carbon_insert_prop_true(obj_ins, "k1");
        jak_carbon_insert_prop_false(obj_ins, "k2");
        jak_carbon_insert_prop_null(obj_ins, "k3");
        jak_carbon_insert_prop_u8(obj_ins, "k4", 1);
        jak_carbon_insert_prop_u16(obj_ins, "k5", 2);
        jak_carbon_insert_prop_u32(obj_ins, "k6", 3);
        jak_carbon_insert_prop_u64(obj_ins, "k7", 4);
        jak_carbon_insert_prop_i8(obj_ins, "k8", -1);
        jak_carbon_insert_prop_i16(obj_ins, "k9", -2);
        jak_carbon_insert_prop_i32(obj_ins, "k10", -3);
        jak_carbon_insert_prop_i64(obj_ins, "k11", -4);
        jak_carbon_insert_prop_float(obj_ins, "k12", 42.23);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k1\": true, \"k2\": false, \"k3\": null, \"k4\": 1, \"k5\": 2, \"k6\": 3, \"k7\": 4, \"k8\": -1, \"k9\": -2, \"k10\": -3, \"k11\": -4, \"k12\": 42.23}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertString)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_string(obj_ins, "hello", "world");

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"hello\": \"world\"}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleString)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_string(obj_ins, "k1", "v1");
        jak_carbon_insert_prop_string(obj_ins, "hello", "world");
        jak_carbon_insert_prop_string(obj_ins, "k3", "there");

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k1\": \"v1\", \"hello\": \"world\", \"k3\": \"there\"}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleStringMixedTypes)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_false(obj_ins, "k2");
        jak_carbon_insert_prop_null(obj_ins, "k3");
        jak_carbon_insert_prop_u8(obj_ins, "k4", 1);
        jak_carbon_insert_prop_string(obj_ins, "s1", "v1");
        jak_carbon_insert_prop_u16(obj_ins, "k5", 2);
        jak_carbon_insert_prop_string(obj_ins, "s2-longer", "world");
        jak_carbon_insert_prop_u32(obj_ins, "k6", 3);
        jak_carbon_insert_prop_u64(obj_ins, "k7", 4);
        jak_carbon_insert_prop_i8(obj_ins, "k8", -1);
        jak_carbon_insert_prop_string(obj_ins, "s3", "there");
        jak_carbon_insert_prop_i16(obj_ins, "k9", -2);
        jak_carbon_insert_prop_i32(obj_ins, "k10", -3);
        jak_carbon_insert_prop_i64(obj_ins, "k11", -4);
        jak_carbon_insert_prop_float(obj_ins, "k12", 42.23);
        jak_carbon_insert_prop_true(obj_ins, "k1");


        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k2\": false, \"k3\": null, \"k4\": 1, \"s1\": \"v1\", \"k5\": 2, \"s2-longer\": \"world\", \"k6\": 3, \"k7\": 4, \"k8\": -1, \"s3\": \"there\", \"k9\": -2, \"k10\": -3, \"k11\": -4, \"k12\": 42.23, \"k1\": true}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertBinary)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_binary(obj_ins, "my binary", "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"my binary\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleBinariesMixedTypes)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_float(obj_ins, "k12", 42.23);
        jak_carbon_insert_prop_true(obj_ins, "k1");
        jak_carbon_insert_prop_binary(obj_ins, "b1", "Hello", strlen("Hello"), "txt", NULL);
        jak_carbon_insert_prop_binary(obj_ins, "my binary", ",", strlen(","), "txt", NULL);
        jak_carbon_insert_prop_false(obj_ins, "k2");
        jak_carbon_insert_prop_null(obj_ins, "k3");
        jak_carbon_insert_prop_u8(obj_ins, "k4", 1);
        jak_carbon_insert_prop_string(obj_ins, "s1", "v1");
        jak_carbon_insert_prop_u16(obj_ins, "k5", 2);
        jak_carbon_insert_prop_binary(obj_ins, "b2", "World", strlen("World"), "txt", NULL);
        jak_carbon_insert_prop_string(obj_ins, "s2-longer", "world");
        jak_carbon_insert_prop_u32(obj_ins, "k6", 3);
        jak_carbon_insert_prop_u64(obj_ins, "k7", 4);
        jak_carbon_insert_prop_i8(obj_ins, "k8", -1);
        jak_carbon_insert_prop_string(obj_ins, "s3", "there");
        jak_carbon_insert_prop_i16(obj_ins, "k9", -2);
        jak_carbon_insert_prop_i32(obj_ins, "k10", -3);
        jak_carbon_insert_prop_i64(obj_ins, "k11", -4);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k12\": 42.23, \"k1\": true, \"b1\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"my binary\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"k2\": false, \"k3\": null, \"k4\": 1, \"s1\": \"v1\", \"k5\": 2, \"b2\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }, \"s2-longer\": \"world\", \"k6\": 3, \"k7\": 4, \"k8\": -1, \"s3\": \"there\", \"k9\": -2, \"k10\": -3, \"k11\": -4}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertMultipleBinaries)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_binary(obj_ins, "b1", "Hello", strlen("Hello"), "txt", NULL);
        jak_carbon_insert_prop_binary(obj_ins, "my binary", ",", strlen(","), "txt", NULL);
        jak_carbon_insert_prop_binary(obj_ins, "b2", "World", strlen("World"), "txt", NULL);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"b1\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"my binary\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"b2\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertObjectEmpty)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state, nested;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_object_begin(&nested, obj_ins, "my nested", 200);
        jak_carbon_insert_prop_object_end(&nested);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"my nested\": {}}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertObjectMixedMxed)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state, nested;

        // -------------------------------------------------------------------------------------------------------------


        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_float(obj_ins, "1", 42.23);
        jak_carbon_insert_prop_true(obj_ins, "2");
        jak_carbon_insert_prop_binary(obj_ins, "3", "Hello", strlen("Hello"), "txt", NULL);
        jak_carbon_insert_prop_binary(obj_ins, "4", ",", strlen(","), "txt", NULL);
        jak_carbon_insert_prop_binary(obj_ins, "5", "World", strlen("World"), "txt", NULL);
        jak_carbon_insert_prop_string(obj_ins, "6", "world");

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_prop_object_begin(&nested, obj_ins, "my nested", 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "7");
        jak_carbon_insert_prop_null(nested_obj_ins, "8");
        jak_carbon_insert_prop_u8(nested_obj_ins, "9", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "10", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "11", 2);

        jak_carbon_insert_prop_object_end(&nested);

        jak_carbon_insert_prop_u32(obj_ins, "12", 3);
        jak_carbon_insert_prop_u64(obj_ins, "13", 4);
        jak_carbon_insert_prop_i8(obj_ins, "14", -1);
        jak_carbon_insert_prop_string(obj_ins, "15", "there");
        jak_carbon_insert_prop_i16(obj_ins, "16", -2);
        jak_carbon_insert_prop_i32(obj_ins, "17", -3);
        jak_carbon_insert_prop_i64(obj_ins, "18", -4);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": 42.23, \"2\": true, \"3\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==sbG8AA\" }, \"4\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"LAAA\" }, \"5\": { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"A==ybGQAA\" }, \"6\": \"world\", \"my nested\": {\"7\": false, \"8\": null, \"9\": 1, \"10\": \"v1\", \"11\": 2}, \"12\": 3, \"13\": 4, \"14\": -1, \"15\": \"there\", \"16\": -2, \"17\": -3, \"18\": -4}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertArrayEmpty)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;
        jak_carbon_insert_array_state array_state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert_prop_array_begin(&array_state, obj_ins, "my array", 200);
        jak_carbon_insert_prop_array_end(&array_state);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"my array\": []}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertArrayData)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;
        jak_carbon_insert_array_state array_state, nested_array_state;
        jak_carbon_insert_column_state column_state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert *nested_array_ins = jak_carbon_insert_prop_array_begin(&array_state, obj_ins, "my array", 200);

        jak_carbon_insert *column_ins = jak_carbon_insert_column_begin(&column_state, nested_array_ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 'X');
        jak_carbon_insert_u32(column_ins, 'Y');
        jak_carbon_insert_u32(column_ins, 'Z');
        jak_carbon_insert_column_end(&column_state);
        jak_carbon_insert *nested_ins = jak_carbon_insert_array_begin(&nested_array_state, nested_array_ins, 10);
        jak_carbon_insert_string(nested_ins, "Hello");
        column_ins = jak_carbon_insert_column_begin(&column_state, nested_ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 'A');
        jak_carbon_insert_u32(column_ins, 'B');
        jak_carbon_insert_u32(column_ins, 'C');
        jak_carbon_insert_column_end(&column_state);
        jak_carbon_insert_string(nested_ins, "World");
        jak_carbon_insert_array_end(&nested_array_state);
        jak_carbon_insert_u8(nested_array_ins, 1);
        jak_carbon_insert_u8(nested_array_ins, 1);
        column_ins = jak_carbon_insert_column_begin(&column_state, nested_array_ins, JAK_CARBON_COLUMN_TYPE_U32, 10);
        jak_carbon_insert_u32(column_ins, 23);
        jak_carbon_insert_u32(column_ins, 24);
        jak_carbon_insert_u32(column_ins, 25);
        jak_carbon_insert_column_end(&column_state);
        jak_carbon_insert_u8(nested_array_ins, 1);

        jak_carbon_insert_prop_array_end(&array_state);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"my array\": [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonObjectInsertColumnNonEmpty)
{
        jak_carbon doc;
        jak_carbon_new context;
        jak_carbon_insert_object_state state;
        jak_carbon_insert_column_state column_state;

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *obj_ins = jak_carbon_insert_object_begin(&state, ins, 1);

        jak_carbon_insert *nested_column_ins = jak_carbon_insert_prop_column_begin(&column_state, obj_ins, "my column", JAK_CARBON_COLUMN_TYPE_U16, 200);
        jak_carbon_insert_u16(nested_column_ins, 1);
        jak_carbon_insert_u16(nested_column_ins, 2);
        jak_carbon_insert_u16(nested_column_ins, 3);
        jak_carbon_insert_prop_column_end(&column_state);

        jak_carbon_insert_object_end(&state);

        jak_carbon_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct jak_string sb;
        string_create(&sb);

        // jak_carbon_print(stdout, &doc);
        ASSERT_TRUE(strcmp(jak_carbon_to_json_extended(&sb, &doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"my column\": [1, 2, 3]}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
}

//static void create_nested_doc(jak_carbon *rev_doc)
//{
//        jak_carbon doc;
//        struct jak_carbon_revise revise;
//        jak_carbon_array_it it;
//        jak_carbon_insert nested_ins, *array_ins, *col_ins, *nested_array_ins;
//        jak_carbon_insert_array_state array_state, nested_array_state;
//        jak_carbon_insert_column_state column_state;
//
//        jak_carbon_create_empty(&doc, JAK_CARBON_KEY_NOKEY);
//        carbon_revise_begin(&revise, rev_doc, &doc);
//        carbon_revise_iterator_open(&it, &revise);
//
//        jak_carbon_array_it_insert_begin(&nested_ins, &it);
//
//        array_ins = jak_carbon_insert_array_begin(&array_state, &nested_ins, 10);
//
//        jak_carbon_insert_null(array_ins);
//        jak_carbon_insert_true(array_ins);
//        jak_carbon_insert_false(array_ins);
//        jak_carbon_insert_u8(array_ins, 8);
//        jak_carbon_insert_i16(array_ins, -16);
//        jak_carbon_insert_string(array_ins, "Hello, World!");
//        jak_carbon_insert_binary(array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        jak_carbon_insert_binary(array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = jak_carbon_insert_column_begin(&column_state, array_ins, JAK_CARBON_COLUMN_TYPE_U32, 20);
//
//        jak_carbon_insert_u32(col_ins, 32);
//        jak_carbon_insert_u32(col_ins, 33);
//        jak_carbon_insert_u32(col_ins, 34);
//        jak_carbon_insert_u32(col_ins, 35);
//
//        jak_carbon_insert_column_end(&column_state);
//
//        jak_carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//        jak_carbon_insert_array_end(&nested_array_state);
//
//        nested_array_ins = jak_carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//
//        jak_carbon_insert_null(nested_array_ins);
//        jak_carbon_insert_true(nested_array_ins);
//        jak_carbon_insert_false(nested_array_ins);
//        jak_carbon_insert_u8(nested_array_ins, 8);
//        jak_carbon_insert_i16(nested_array_ins, -16);
//        jak_carbon_insert_string(nested_array_ins, "Hello, World!");
//        jak_carbon_insert_binary(nested_array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        jak_carbon_insert_binary(nested_array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = jak_carbon_insert_column_begin(&column_state, nested_array_ins, JAK_CARBON_COLUMN_TYPE_U32, 20);
//
//        jak_carbon_insert_u32(col_ins, 32);
//        jak_carbon_insert_u32(col_ins, 33);
//        jak_carbon_insert_u32(col_ins, 34);
//        jak_carbon_insert_u32(col_ins, 35);
//
//        jak_carbon_insert_column_end(&column_state);
//
//        jak_carbon_insert_array_end(&nested_array_state);
//
//        jak_carbon_insert_array_end(&array_state);
//
//        array_ins = jak_carbon_insert_array_begin(&array_state, &nested_ins, 10);
//
//        jak_carbon_insert_null(array_ins);
//        jak_carbon_insert_true(array_ins);
//        jak_carbon_insert_false(array_ins);
//        jak_carbon_insert_u8(array_ins, 8);
//        jak_carbon_insert_i16(array_ins, -16);
//        jak_carbon_insert_string(array_ins, "Hello, World!");
//        jak_carbon_insert_binary(array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        jak_carbon_insert_binary(array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = jak_carbon_insert_column_begin(&column_state, array_ins, JAK_CARBON_COLUMN_TYPE_U32, 20);
//
//        jak_carbon_insert_u32(col_ins, 32);
//        jak_carbon_insert_u32(col_ins, 33);
//        jak_carbon_insert_u32(col_ins, 34);
//        jak_carbon_insert_u32(col_ins, 35);
//
//        jak_carbon_insert_column_end(&column_state);
//
//        jak_carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//        jak_carbon_insert_array_end(&nested_array_state);
//
//        nested_array_ins = jak_carbon_insert_array_begin(&nested_array_state, array_ins, 20);
//
//        jak_carbon_insert_null(nested_array_ins);
//        jak_carbon_insert_true(nested_array_ins);
//        jak_carbon_insert_false(nested_array_ins);
//        jak_carbon_insert_u8(nested_array_ins, 8);
//        jak_carbon_insert_i16(nested_array_ins, -16);
//        jak_carbon_insert_string(nested_array_ins, "Hello, World!");
//        jak_carbon_insert_binary(nested_array_ins, "My Plain-Text", strlen("My Plain-Text"), "txt", NULL);
//        jak_carbon_insert_binary(nested_array_ins, "My Own Format", strlen("My Own Format"), NULL, "own");
//        col_ins = jak_carbon_insert_column_begin(&column_state, nested_array_ins, JAK_CARBON_COLUMN_TYPE_U32, 20);
//
//        jak_carbon_insert_u32(col_ins, 32);
//        jak_carbon_insert_u32(col_ins, 33);
//        jak_carbon_insert_u32(col_ins, 34);
//        jak_carbon_insert_u32(col_ins, 35);
//
//        jak_carbon_insert_column_end(&column_state);
//
//        jak_carbon_insert_array_end(&nested_array_state);
//
//        jak_carbon_insert_array_end(&array_state);
//
//        jak_carbon_array_it_insert_end(&nested_ins);
//
//        carbon_revise_iterator_close(&it);
//        carbon_revise_end(&revise);
//}

TEST(CarbonTest, CarbonObjectRemoveTest)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "6");
        jak_carbon_insert_prop_null(nested_obj_ins, "7");
        jak_carbon_insert_prop_u8(nested_obj_ins, "8", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "9", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "10", 2);

        jak_carbon_insert_prop_object_end(&state);

        nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "11");
        jak_carbon_insert_prop_null(nested_obj_ins, "12");
        jak_carbon_insert_prop_u8(nested_obj_ins, "13", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "14", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "15", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);

        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}, {\"6\": false, \"7\": null, \"8\": 1, \"9\": \"v1\", \"10\": 2}, {\"11\": false, \"12\": null, \"13\": 1, \"14\": \"v1\", \"15\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemoveSkipOneTest)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "6");
        jak_carbon_insert_prop_null(nested_obj_ins, "7");
        jak_carbon_insert_prop_u8(nested_obj_ins, "8", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "9", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "10", 2);

        jak_carbon_insert_prop_object_end(&state);

        nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "11");
        jak_carbon_insert_prop_null(nested_obj_ins, "12");
        jak_carbon_insert_prop_u8(nested_obj_ins, "13", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "14", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "15", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        jak_carbon_array_it_remove(&rev_it);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}, {\"6\": false, \"7\": null, \"8\": 1, \"9\": \"v1\", \"10\": 2}, {\"11\": false, \"12\": null, \"13\": 1, \"14\": \"v1\", \"15\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringIt)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);

        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));
        //printf("\n%s\n", json_2);

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"Hello Long Key\": \"Hello Long Value\", \"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringItAtIndex1)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);

        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"Hello Long Key\": \"Hello Long Value\", \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringItAtIndex2)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"Hello Long Key\": \"Hello Long Value\", \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringItAtIndex3)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"Hello Long Key\": \"Hello Long Value\", \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringItAtIndex4)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"Hello Long Key\": \"Hello Long Value\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectInsertPropDuringItAtIndex5)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);

        jak_carbon_insert_object_state state;
        jak_carbon_insert nested_ins;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_TRUE(carbon_object_it_next(obj_it));
        ASSERT_FALSE(carbon_object_it_next(obj_it));

        carbon_object_it_insert_begin(&nested_ins, obj_it);
        jak_carbon_insert_prop_string(&nested_ins, "Hello Long Key", "Hello Long Value");
        carbon_object_it_insert_end(&nested_ins);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2, \"Hello Long Key\": \"Hello Long Value\"}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKey)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_false(nested_obj_ins, "1");
        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": false, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKeyTypeObjectNonEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert_object_state nested_obj;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert *nested_nested_obj_ins = jak_carbon_insert_prop_object_begin(&nested_obj, nested_obj_ins, "1", 100);
        jak_carbon_insert_prop_null(nested_nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_nested_obj_ins, "5", 2);
        jak_carbon_insert_prop_object_end(&nested_obj);

        jak_carbon_insert_prop_null(nested_obj_ins, "6");
        jak_carbon_insert_prop_u8(nested_obj_ins, "7", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "8", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "9", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": {\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}, \"6\": null, \"7\": 1, \"8\": \"v1\", \"9\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"6\": null, \"7\": 1, \"8\": \"v1\", \"9\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKeyTypeArrayEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert_array_state nested_arr;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_array_begin(&nested_arr, nested_obj_ins, "1", 100);

        jak_carbon_insert_prop_array_end(&nested_arr);

        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        //printf("%s\n", json_1);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": [], \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKeyTypeArrayNonEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert_array_state nested_arr;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert *nested_arr_it = jak_carbon_insert_prop_array_begin(&nested_arr, nested_obj_ins, "1", 100);
        jak_carbon_insert_null(nested_arr_it);
        jak_carbon_insert_u8(nested_arr_it, 1);
        jak_carbon_insert_string(nested_arr_it, "v1");
        jak_carbon_insert_u16(nested_arr_it, 2);
        jak_carbon_insert_prop_array_end(&nested_arr);

        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": [null, 1, \"v1\", 2], \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKeyTypeColumnEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert_column_state nested_col;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_column_begin(&nested_col, nested_obj_ins, "1", JAK_CARBON_COLUMN_TYPE_U32, 100);

        jak_carbon_insert_prop_column_end(&nested_col);

        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": [], \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonObjectRemovePropByKeyTypeObjectEmpty)
{
        jak_carbon doc, rev_doc;
        jak_carbon_new context;
        struct jak_carbon_revise revise;
        jak_carbon_array_it rev_it;
        struct jak_string sb;
        bool has_next;
        string_create(&sb);
        jak_u64 key_len;

        jak_carbon_insert_object_state state;
        jak_carbon_insert_object_state nested_obj;

        // -------------------------------------------------------------------------------------------------------------
        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);

        jak_carbon_insert *nested_obj_ins = jak_carbon_insert_object_begin(&state, ins, 200);

        jak_carbon_insert_prop_object_begin(&nested_obj, nested_obj_ins, "1", 100);
        jak_carbon_insert_prop_object_end(&nested_obj);

        jak_carbon_insert_prop_null(nested_obj_ins, "2");
        jak_carbon_insert_prop_u8(nested_obj_ins, "3", 1);
        jak_carbon_insert_prop_string(nested_obj_ins, "4", "v1");
        jak_carbon_insert_prop_u16(nested_obj_ins, "5", 2);

        jak_carbon_insert_prop_object_end(&state);

        jak_carbon_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(jak_carbon_to_json_extended(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        carbon_revise_begin(&revise, &rev_doc, &doc);
        carbon_revise_iterator_open(&rev_it, &revise);
        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);

        // -------------------------------------------------------------------------------------------------------------

        jak_carbon_field_type_e field_type;
        jak_carbon_array_it_field_type(&field_type, &rev_it);
        ASSERT_EQ(field_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *obj_it = jak_carbon_array_it_object_value(&rev_it);
        has_next = carbon_object_it_next(obj_it);
        ASSERT_TRUE(has_next);
        const char *prop_name = carbon_object_it_prop_name(&key_len, obj_it);
        ASSERT_TRUE(strncmp(prop_name, "1", strlen("1")) == 0);

        carbon_object_it_remove(obj_it);
        carbon_object_it_drop(obj_it);

        // -------------------------------------------------------------------------------------------------------------

        has_next = jak_carbon_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);

        carbon_revise_iterator_close(&rev_it);
        carbon_revise_end(&revise);

        char *json_2 = strdup(jak_carbon_to_json_extended(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"1\": {}, \"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"2\": null, \"3\": 1, \"4\": \"v1\", \"5\": 2}]}") == 0);

        string_drop(&sb);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(CarbonTest, CarbonUpdateSetToNull)
{
        struct jak_string sb;

        string_create(&sb);

        /* Each time 'create_nested_doc' is called, the following document will be generated

                [
                   [
                      null,
                      true,
                      false,
                      8,
                      -16,
                      "Hello, World!",
                      {
                         "type":"text/plain",
                         "encoding":"base64",
                         "binary-string":"TXkgUGxhaW4tVGV4dAAA"
                      },
                      {
                         "type":"own",
                         "encoding":"base64",
                         "binary-string":"TXkgT3duIEZvcm1hdAAA"
                      },
                      [
                         32,
                         33,
                         34,
                         35
                      ],
                      [

                      ],
                      [
                         null,
                         true,
                         false,
                         8,
                         -16,
                         "Hello, World!",
                         {
                            "type":"text/plain",
                            "encoding":"base64",
                            "binary-string":"TXkgUGxhaW4tVGV4dAAA"
                         },
                         {
                            "type":"own",
                            "encoding":"base64",
                            "binary-string":"TXkgT3duIEZvcm1hdAAA"
                         },
                         [
                            32,
                            33,
                            34,
                            35
                         ]
                      ]
                   ],
                   [
                      null,
                      true,
                      false,
                      8,
                      -16,
                      "Hello, World!",
                      {
                         "type":"text/plain",
                         "encoding":"base64",
                         "binary-string":"TXkgUGxhaW4tVGV4dAAA"
                      },
                      {
                         "type":"own",
                         "encoding":"base64",
                         "binary-string":"TXkgT3duIEZvcm1hdAAA"
                      },
                      [
                         32,
                         33,
                         34,
                         35
                      ],
                      [

                      ],
                      [
                         null,
                         true,
                         false,
                         8,
                         -16,
                         "Hello, World!",
                         {
                            "type":"text/plain",
                            "encoding":"base64",
                            "binary-string":"TXkgUGxhaW4tVGV4dAAA"
                         },
                         {
                            "type":"own",
                            "encoding":"base64",
                            "binary-string":"TXkgT3duIEZvcm1hdAAA"
                         },
                         [
                            32,
                            33,
                            34,
                            35
                         ]
                      ]
                   ]
                ]

         */

//        // -------------------------------------------------------------------------------------------------------------
//        // Update to null
//        // -------------------------------------------------------------------------------------------------------------
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.0", &rev_doc, &doc); // replaces null with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.1", &rev_doc, &doc); // replaces true with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, null, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.2", &rev_doc, &doc); // replaces false with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, null, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.3", &rev_doc, &doc); // replaces u8 (8) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, null, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.4", &rev_doc, &doc); // replaces i16 (-16) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, null, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.5", &rev_doc, &doc); // replaces string with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, null, { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.6", &rev_doc, &doc); // replaces binary string with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", null, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.7", &rev_doc, &doc); // replaces custom binary with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, null, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.8", &rev_doc, &doc); // replaces column ([32, 33, 34, 35]) with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, null, [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.8.0", &rev_doc, &doc); // replaces element in column with null value (special case) --> [NULL, 33, 34, 35]
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [null, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.9", &rev_doc, &doc); // replaces empty array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], null, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0.10", &rev_doc, &doc); // replaces complex array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], null], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("0", &rev_doc, &doc); // replaces 1st outermost array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [null, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_null("1", &rev_doc, &doc); // replaces 2nd outermost array with null
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], null]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
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
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[true, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.1", &rev_doc, &doc); // replaces true with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.2", &rev_doc, &doc); // replaces false with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, true, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.3", &rev_doc, &doc); // replaces u8 (8) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, true, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.4", &rev_doc, &doc); // replaces i16 (-16) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, true, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.5", &rev_doc, &doc); // replaces string with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, true, { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.6", &rev_doc, &doc); // replaces binary string with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", true, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.7", &rev_doc, &doc); // replaces custom binary with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, true, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.8", &rev_doc, &doc); // replaces column ([32, 33, 34, 35]) with true
//        ASSERT_TRUE(status);
//        // printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        // printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, true, [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);

//        create_nested_doc(&doc);
//        // ??????
//        status = carbon_update_one_set_true("0.8.0", &rev_doc, &doc); // replaces element in column with null value (special case) --> [NULL, 33, 34, 35]
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [true, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.9", &rev_doc, &doc); // replaces empty array with true
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], true, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0.10", &rev_doc, &doc); // replaces complex array with true
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], true], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("0", &rev_doc, &doc); // replaces 1st outermost array with true
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true, [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);
//
//        create_nested_doc(&doc);
//        status = carbon_update_one_set_true("1", &rev_doc, &doc); // replaces 2nd outermost array with true
//        ASSERT_TRUE(status);
//        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
//        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
//        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], true]}") == 0);
//        jak_carbon_drop(&doc);
//        jak_carbon_drop(&rev_doc);

        /*
        create_nested_doc(&doc);
        status = carbon_update_one_set_null("0.5", &rev_doc, &doc); // replaces string with null
        ASSERT_TRUE(status);
        printf("built:  \t'%s'\n", carbon_to_json(&sb, &doc));
        printf("altered:\t'%s'\n", carbon_to_json(&sb, &rev_doc));
        ASSERT_TRUE(strcmp(carbon_to_json(&sb, &rev_doc), "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [[null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35], [], [null, true, false, 8, -16, \"Hello, World!\", { \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"TXkgUGxhaW4tVGV4dAAA\" }, { \"type\": \"own\", \"encoding\": \"base64\", \"binary-string\": \"TXkgT3duIEZvcm1hdAAA\" }, [32, 33, 34, 35]]]]}") == 0);
        jak_carbon_drop(&doc);
        jak_carbon_drop(&rev_doc);
        */


        // Overwrite constant in-pace w/ fixed-type
        // Overwrite constant in-pace w/ string
        // Overwrite constant in-pace w/ binary
        // Overwrite constant in-pace w/ custom binary
        // Overwrite constant in-pace w/ empty array
        // Overwrite constant in-pace w/ non-empty array
        // Overwrite constant in-pace w/ empty column
        // Overwrite constant in-pace w/ non-empty column

        // Update fixed-type in-place
        // Overwrite fixed-type in-pace w/ constant
        // Overwrite fixed-type in-pace w/ fixed-type (w/ same width)
        // Overwrite fixed-type in-pace w/ fixed-type (w/ other width)
        // Overwrite fixed-type in-pace w/ string
        // Overwrite fixed-type in-pace w/ binary
        // Overwrite fixed-type in-pace w/ custom binary
        // Overwrite fixed-type in-pace w/ empty array
        // Overwrite fixed-type in-pace w/ non-empty array
        // Overwrite fixed-type in-pace w/ empty column
        // Overwrite fixed-type in-pace w/ non-empty column

        // Update string in-place
        // Overwrite string in-pace w/ constant
        // Overwrite string in-pace w/ fixed-type
        // Overwrite string in-pace w/ string
        // Overwrite string in-pace w/ binary
        // Overwrite string in-pace w/ custom binary
        // Overwrite string in-pace w/ empty array
        // Overwrite string in-pace w/ non-empty array
        // Overwrite string in-pace w/ empty column
        // Overwrite string in-pace w/ non-empty column

        // Update binary in-place
        // Overwrite binary in-pace w/ constant
        // Overwrite binary in-pace w/ fixed-type
        // Overwrite binary in-pace w/ string
        // Overwrite binary in-pace w/ binary
        // Overwrite binary in-pace w/ custom binary
        // Overwrite binary in-pace w/ empty array
        // Overwrite binary in-pace w/ non-empty array
        // Overwrite binary in-pace w/ empty column
        // Overwrite binary in-pace w/ non-empty column

        // Update custom binary in-place
        // Overwrite custom binary in-pace w/ constant
        // Overwrite custom binary in-pace w/ fixed-type
        // Overwrite custom binary in-pace w/ string
        // Overwrite custom binary in-pace w/ binary
        // Overwrite custom binary in-pace w/ custom binary
        // Overwrite custom binary in-pace w/ empty array
        // Overwrite custom binary in-pace w/ non-empty array
        // Overwrite custom binary in-pace w/ empty column
        // Overwrite custom binary in-pace w/ non-empty column

        // Update empty-array binary in-place
        // Overwrite empty-array in-pace w/ constant
        // Overwrite empty-array in-pace w/ fixed-type
        // Overwrite empty-array in-pace w/ string
        // Overwrite empty-array in-pace w/ binary
        // Overwrite empty-array in-pace w/ custom binary
        // Overwrite empty-array in-pace w/ non-empty array
        // Overwrite empty-array in-pace w/ empty column
        // Overwrite empty-array in-pace w/ non-empty column

        // Update non-empty array binary in-place
        // Overwrite non-empty array in-pace w/ constant
        // Overwrite non-empty array in-pace w/ fixed-type
        // Overwrite non-empty array in-pace w/ string
        // Overwrite non-empty array in-pace w/ binary
        // Overwrite non-empty array in-pace w/ custom binary
        // Overwrite non-empty array in-pace w/ empty array
        // Overwrite non-empty array in-pace w/ non-empty array
        // Overwrite non-empty array in-pace w/ empty column
        // Overwrite non-empty array in-pace w/ non-empty column

        // Overwrite empty column in-pace w/ constant
        // Overwrite empty column in-pace w/ fixed-type
        // Overwrite empty column in-pace w/ string
        // Overwrite empty column in-pace w/ binary
        // Overwrite empty column in-pace w/ custom binary
        // Overwrite empty column in-pace w/ empty array
        // Overwrite empty column in-pace w/ non-empty array
        // Overwrite empty column in-pace w/ non-empty column

        // Update non-empty column in-place
        // Overwrite non-empty column in-pace w/ constant
        // Overwrite non-empty column in-pace w/ fixed-type
        // Overwrite non-empty column in-pace w/ string
        // Overwrite non-empty column in-pace w/ binary
        // Overwrite non-empty column in-pace w/ custom binary
        // Overwrite non-empty column in-pace w/ empty array
        // Overwrite non-empty column in-pace w/ non-empty array
        // Overwrite non-empty column in-pace w/ empty column
        // Overwrite non-empty column in-pace w/ non-empty column

        // Update column entry in-place
        // Overwrite column entry in-pace w/ constant (matching type)
        // Overwrite column entry in-pace w/ constant (not matching type)
        // Overwrite column entry in-pace w/ fixed-type (matching type)
        // Overwrite column entry in-pace w/ fixed-type (not matching type)

        // Overwrite entire document content in-pace w/ constant
        // Overwrite entire document content in-pace w/ fixed-type
        // Overwrite entire document content in-pace w/ string
        // Overwrite entire document content in-pace w/ binary
        // Overwrite entire document content in-pace w/ custom binary
        // Overwrite entire document content in-pace w/ empty array
        // Overwrite entire document content in-pace w/ non-empty array
        // Overwrite entire document content in-pace w/ empty column
        // Overwrite entire document content in-pace w/ non-empty column


        string_drop(&sb);
}

TEST(CarbonTest, CarbonFromEmptyJson)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{}";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);
        json_out_extended = jak_carbon_to_json_extended_dup(&doc);
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);

        //printf("INS:\t%s\n", json_in);
        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromEmptyArray)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "[]";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);
        json_out_extended = jak_carbon_to_json_extended_dup(&doc);
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);

//        printf("INS:\t%s\n", json_in);
//        printf("EXT:\t%s\n", json_out_extended);
//        printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, "{}") == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": []}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonNull)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "null";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [null]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be 'null'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [null]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonTrue)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "true";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [true]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be 'true'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [true]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonFalse)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "false";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [false]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be 'false'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [false]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonNumberSigned)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "42";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [42]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '42'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [42]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonNumberUnsigned)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "-42";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [-42]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '-42'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [-42]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonNumberFloat)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "-42.23";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [-42.23]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '-42.23'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [-42.23]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonString)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "\"Hello, World!\"";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": ["Hello, World!"]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '"Hello, World!"'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [\"Hello, World!\"]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectSingle)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": \"v\"}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":"v"}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":"v"}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": \"v\"}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}


TEST(CarbonTest, CarbonFromJsonObjectEmptyArrayProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": []}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":[]}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":[]}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": []}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectEmptyObjectProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": {}}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":{}}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":{}}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": {}}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectTrueProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": true}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":true}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":true}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": true}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectFalseProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": false}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":false}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":false}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": false}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectNullProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": null}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":null}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":null}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": null}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectUnsignedProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": 42}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":42}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":42}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": 42}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectSignedProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": -42}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":-42}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":-42}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": -42}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonObjectFloatProp)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"k\": -42.23}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"k":-42.23}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"k":-42.23}'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"k\": -42.23}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonColumnNumber)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"x\": [1, 2, 3]}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_carbon_array_it it;
        jak_carbon_field_type_e field_type;
        jak_carbon_iterator_open(&it, &doc);
        ASSERT_TRUE(jak_carbon_array_it_next(&it));
        jak_carbon_array_it_field_type(&field_type, &it);
        ASSERT_TRUE(field_type == JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *oit = jak_carbon_array_it_object_value(&it);
        ASSERT_TRUE(carbon_object_it_next(oit));
        carbon_object_it_prop_type(&field_type, oit);
        ASSERT_TRUE(jak_carbon_field_type_is_column(field_type));
        ASSERT_TRUE(field_type == JAK_CARBON_FIELD_TYPE_COLUMN_U8);
        carbon_object_it_drop(oit);
        jak_carbon_iterator_close(&it);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{\"x\": [1, 2, 3]}}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '[1, 2, 3]'

        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"x\": [1, 2, 3]}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonColumnNullableNumber)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "{\"x\": [1, null, 3]}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_carbon_array_it it;
        jak_carbon_field_type_e field_type;
        jak_carbon_iterator_open(&it, &doc);
        ASSERT_TRUE(jak_carbon_array_it_next(&it));
        jak_carbon_array_it_field_type(&field_type, &it);
        ASSERT_TRUE(field_type == JAK_CARBON_FIELD_TYPE_OBJECT);
        struct jak_carbon_object_it *oit = jak_carbon_array_it_object_value(&it);
        ASSERT_TRUE(carbon_object_it_next(oit));
        carbon_object_it_prop_type(&field_type, oit);
        ASSERT_TRUE(jak_carbon_field_type_is_column(field_type));
        ASSERT_TRUE(field_type == JAK_CARBON_FIELD_TYPE_COLUMN_U8);
        carbon_object_it_drop(oit);
        jak_carbon_iterator_close(&it);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [{"x": [1, null, 3]}]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"x": [1, null, 3]}'

        //printf("INS:\t%s\n", json_in);
        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [{\"x\": [1, null, 3]}]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonFromJsonNonColumn)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact, *json_out_extended;

        json_in = "[1, null, 3, \"a\"]";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_carbon_array_it it;
        jak_carbon_field_type_e field_type;
        jak_carbon_iterator_open(&it, &doc);
        ASSERT_TRUE(jak_carbon_array_it_next(&it));
        jak_carbon_array_it_field_type(&field_type, &it);
        ASSERT_TRUE(jak_carbon_field_type_is_number(field_type));
        jak_carbon_iterator_close(&it);

        json_out_extended = jak_carbon_to_json_extended_dup(&doc);  // shall be '{"meta": {"key": {"type": "nokey", "value": null}, "rev": 0}, "doc": [1, null, 3, \"a\"]}'
        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '[1, null, 3, \"a\"]'

        //printf("INS:\t%s\n", json_in);
        //printf("EXT:\t%s\n", json_out_extended);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);
        ASSERT_TRUE(strcmp(json_out_extended, "{\"meta\": {\"key\": {\"type\": \"nokey\", \"value\": null}, \"commit\": null}, \"doc\": [1, null, 3, \"a\"]}") == 0);

        free(json_out_compact);
        free(json_out_extended);
}

TEST(CarbonTest, CarbonColumnOptimizeFix)
{
        jak_carbon_new context;
        jak_carbon doc;
        jak_carbon_insert_column_state state_out;

        jak_carbon_insert *ins = jak_carbon_create_begin(&context, &doc, JAK_CARBON_KEY_NOKEY, JAK_CARBON_OPTIMIZE);
        jak_carbon_insert *cins = jak_carbon_insert_column_begin(&state_out, ins, JAK_CARBON_COLUMN_TYPE_U8, 4);
        jak_carbon_insert_u8(cins, 3);
        jak_carbon_insert_u8(cins, 4);
        jak_carbon_insert_u8(cins, 5);
        jak_carbon_insert_column_end(&state_out);
        jak_carbon_create_end(&context);

        char *json = jak_carbon_to_json_compact_dup(&doc);
        ASSERT_TRUE(strcmp(json, "[3, 4, 5]") == 0);
        jak_carbon_drop(&doc);
        free(json);
}

TEST(CarbonTest, CarbonFromJsonExample)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact;

        /* example json taken from 'https://json.org/example.html' */
        json_in = "{\"web-app\": {\"servlet\": [{\"servlet-name\": \"cofaxCDS\", \"servlet-class\": \"org.cofax.cds.CDSServlet\", \"init-param\": {\"configGlossary: installationAt\": \"Philadelphia, PA\", \"configGlossary: adminEmail\": \"ksm@pobox.com\", \"configGlossary: poweredBy\": \"Cofax\", \"configGlossary: poweredByIcon\": \"/images/cofax.gif\", \"configGlossary: staticPath\": \"/content/static\", \"templateProcessorClass\": \"org.cofax.WysiwygTemplate\", \"templateLoaderClass\": \"org.cofax.FilesTemplateLoader\", \"templatePath\": \"templates\", \"templateOverridePath\": \"\", \"defaultListTemplate\": \"listTemplate.htm\", \"defaultFileTemplate\": \"articleTemplate.htm\", \"useJSP\": false, \"jspListTemplate\": \"listTemplate.jsp\", \"jspFileTemplate\": \"articleTemplate.jsp\", \"cachePackageTagsTrack\": 200, \"cachePackageTagsStore\": 200, \"cachePackageTagsRefresh\": 60, \"cacheTemplatesTrack\": 100, \"cacheTemplatesStore\": 50, \"cacheTemplatesRefresh\": 15, \"cachePagesTrack\": 200, \"cachePagesStore\": 100, \"cachePagesRefresh\": 10, \"cachePagesDirtyRead\": 10, \"searchEngineListTemplate\": \"forSearchEnginesList.htm\", \"searchEngineFileTemplate\": \"forSearchEngines.htm\", \"searchEngineRobotsDb\": \"WEB-INF/robots.db\", \"useDataStore\": true, \"dataStoreClass\": \"org.cofax.SqlDataStore\", \"redirectionClass\": \"org.cofax.SqlRedirection\", \"dataStoreName\": \"cofax\", \"dataStoreDriver\": \"com.microsoft.jdbc.sqlserver.SQLServerDriver\", \"dataStoreUrl\": \"jdbc: microsoft: sqlserver: //LOCALHOST: 1433;DatabaseName=goon\", \"dataStoreUser\": \"sa\", \"dataStorePassword\": \"dataStoreTestQuery\", \"dataStoreTestQuery\": \"SET NOCOUNT ON;select test='test';\", \"dataStoreLogFile\": \"/usr/local/tomcat/logs/datastore.log\", \"dataStoreInitConns\": 10, \"dataStoreMaxConns\": 100, \"dataStoreConnUsageLimit\": 100, \"dataStoreLogLevel\": \"debug\", \"maxUrlLength\": 500}}, {\"servlet-name\": \"cofaxEmail\", \"servlet-class\": \"org.cofax.cds.EmailServlet\", \"init-param\": {\"mailHost\": \"mail1\", \"mailHostOverride\": \"mail2\"}}, {\"servlet-name\": \"cofaxAdmin\", \"servlet-class\": \"org.cofax.cds.AdminServlet\"}, {\"servlet-name\": \"fileServlet\", \"servlet-class\": \"org.cofax.cds.FileServlet\"}, {\"servlet-name\": \"cofaxTools\", \"servlet-class\": \"org.cofax.cms.CofaxToolsServlet\", \"init-param\": {\"templatePath\": \"toolstemplates/\", \"log\": 1, \"logLocation\": \"/usr/local/tomcat/logs/CofaxTools.log\", \"logMaxSize\": \"\", \"dataLog\": 1, \"dataLogLocation\": \"/usr/local/tomcat/logs/dataLog.log\", \"dataLogMaxSize\": \"\", \"removePageCache\": \"/content/admin/remove?cache=pages&id=\", \"removeTemplateCache\": \"/content/admin/remove?cache=templates&id=\", \"fileTransferFolder\": \"/usr/local/tomcat/webapps/content/fileTransferFolder\", \"lookInContext\": 1, \"adminGroupID\": 4, \"betaServer\": true}}], \"servlet-mapping\": {\"cofaxCDS\": \"/\", \"cofaxEmail\": \"/cofaxutil/aemail/*\", \"cofaxAdmin\": \"/admin/*\", \"fileServlet\": \"/static/*\", \"cofaxTools\": \"/tools/*\"}, \"taglib\": {\"taglib-uri\": \"cofax.tld\", \"taglib-location\": \"/WEB-INF/tlds/cofax.tld\"}}}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_u32 max = 10000;
        timestamp_t t1 = time_now_wallclock();
        for (jak_u32 i = 0; i < max; i++) {
                jak_carbon d;
                jak_carbon_from_json(&d, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);
                jak_carbon_drop(&d);
        }
        timestamp_t t2 = time_now_wallclock();
        printf("%.2fmsec/opp, %.4f ops/sec\n", (t2-t1)/(float)max, 1.0f/((t2-t1)/(float)max/1000.0f));


        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '[1, null, 3, \"a\"]'

        //printf("INS:\t%s\n", json_in);
        //printf("SRT:\t%s\n", json_out_compact);

        //jak_carbon_hexdump_print(stdout, &doc);
        //jak_u64 carbon_len = 0;
        //jak_carbon_raw_data(&carbon_len, &doc);
        //printf("INS len: %zu\n", strlen(json_in));
        //printf("SRT len: %zu\n", carbon_len);
        //printf("%0.2f%% space saving\n", 100 * (1 - (carbon_len / (float) strlen(json_in))));

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);

        free(json_out_compact);
}

TEST(CarbonTest, CarbonFromJsonUnitArrayPrimitive)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact;

        json_in = "{\"x\": [1]}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"x":[1]}'

        //printf("INS:\t%s\n", json_in);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);

        free(json_out_compact);
}

TEST(CarbonTest, CarbonFromJsonUnitArrayObject)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in;
        char *json_out_compact;

        json_in = "{\"x\": [{\"y\": 1}]}";

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        json_out_compact = jak_carbon_to_json_compact_dup(&doc);    // shall be '{"x":[{"y":1}]}'

        //printf("INS:\t%s\n", json_in);
        //printf("SRT:\t%s\n", json_out_compact);

        jak_carbon_drop(&doc);

        ASSERT_TRUE(strcmp(json_out_compact, json_in) == 0);

        free(json_out_compact);
}

TEST(CarbonTest, CarbonFromJsonSimpleExample)
{
        jak_carbon doc;
        struct jak_error err;

        const char *json_in = "{\"k\": {\"x\": [1,2,3], \"y\": \"z\"}}";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);
        //jak_carbon_hexdump_print(stdout, &doc);
        //jak_carbon_print(stdout, &doc);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonFromJsonFromExcerpt)
{
        jak_carbon doc;
        struct jak_error err;

        /* the working directory must be 'tests/carbon' to find this file */
        int fd = open("./assets/ms-academic-graph.json", O_RDONLY);
        ASSERT_NE(fd, -1);
        int json_in_len = lseek(fd, 0, SEEK_END);
        const char *json_in = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_u64 carbon_out_len = 0;
        jak_carbon_raw_data(&carbon_out_len, &doc);

        ASSERT_LT(carbon_out_len, json_in_len);
        //printf("%0.2f%% space saving\n", 100 * (1 - (carbon_out_len / (float) json_in_len)));

        char *json_out = jak_carbon_to_json_compact_dup(&doc);
        ASSERT_TRUE(strcmp(json_in, json_out) == 0);

        jak_carbon_drop(&doc);
        free(json_out);
}

TEST(CarbonTest, CarbonResolveDotPathForObjects)
{
        jak_carbon doc;
        struct jak_error err;
        jak_carbon_find find;
        jak_carbon_field_type_e result_type;
        jak_u64 number;

        const char *json_in = "{\"a\": 1, \"b\": {\"c\": [1,2,3], \"d\": [\"Hello\", \"World\"], \"e\": [4], \"f\": [\"!\"], \"the key\": \"x\"}}";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "1", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.a", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_OBJECT);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.c", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.c", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_COLUMN_U8);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.c.0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
        ASSERT_TRUE(jak_carbon_find_result_unsigned(&number, &find));
        ASSERT_EQ(number, 1);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.c.1", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
        ASSERT_TRUE(jak_carbon_find_result_unsigned(&number, &find));
        ASSERT_EQ(number, 2);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.c.2", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
        ASSERT_TRUE(jak_carbon_find_result_unsigned(&number, &find));
        ASSERT_EQ(number, 3);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.c.3", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.d", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_ARRAY);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.d.0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        ASSERT_TRUE(strncmp(jak_carbon_find_result_string(&number, &find), "Hello", number) == 0);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.d.1", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        ASSERT_TRUE(strncmp(jak_carbon_find_result_string(&number, &find), "World", number) == 0);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.d.2", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.e", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_COLUMN_U8);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.e.0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_NUMBER_U8);
        ASSERT_TRUE(jak_carbon_find_result_unsigned(&number, &find));
        ASSERT_EQ(number, 4);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.e.1", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.f", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_ARRAY);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.f.0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        ASSERT_TRUE(strncmp(jak_carbon_find_result_string(&number, &find), "!", number) == 0);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.f.1", &doc));
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_close(&find));

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.b.\"the key\"", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        ASSERT_TRUE(jak_carbon_find_result_type(&result_type, &find));
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        ASSERT_TRUE(strncmp(jak_carbon_find_result_string(&number, &find), "x", number) == 0);
        ASSERT_TRUE(jak_carbon_find_close(&find));

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonResolveDotPathForObjectsBench)
{
        jak_carbon doc;
        struct jak_error err;
        jak_carbon_find find;

        const char *json_in = "{\"a\": 1, \"b\": {\"c\": [1,2,3], \"d\": [\"Hello\", \"World\"], \"e\": [4], \"f\": [\"!\"], \"the key\": \"x\"}}";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        jak_carbon_dot_path path1, path2, path3, path4, path5, path6, path7, path8, path9, path10, path11, path12,
                path13, path14, path15, path16, path17, path18, path19, path20, path21;

        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path1, "0"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path2, "1"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path3, "0.a"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path4, "0.b"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path5, "0.c"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path6, "0.b.c"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path7, "0.b.c.0"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path8, "0.b.c.1"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path9, "0.b.c.2"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path10, "0.b.c.3"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path11, "0.b.d"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path12, "0.b.d.0"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path13, "0.b.d.1"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path14, "0.b.d.2"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path15, "0.b.e"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path16, "0.b.e.0"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path17, "0.b.e.1"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path18, "0.b.f"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path19, "0.b.f.0"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path20, "0.b.f.1"));
        ASSERT_TRUE(jak_carbon_dot_path_from_string(&path21, "0.b.\"the key\""));

        jak_u32 max = 10000;
        timestamp_t t1 = time_now_wallclock();
        for (jak_u32 i = 0; i < max; i++) {
                ASSERT_TRUE(jak_carbon_find_create(&find, &path1, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path2, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path3, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path4, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path5, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path6, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path7, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path8, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path9, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path10, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path11, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path12, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path13, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path14, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path15, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path16, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path17, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path18, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path19, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path20, &doc));
                ASSERT_FALSE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));

                ASSERT_TRUE(jak_carbon_find_create(&find, &path21, &doc));
                ASSERT_TRUE(jak_carbon_find_has_result(&find));
                ASSERT_TRUE(jak_carbon_find_close(&find));
        }
        timestamp_t t2 = time_now_wallclock();
        printf("%.2fmsec/opp, %.4f ops/sec\n", (t2-t1)/(float)max/21.0f, 1.0f/((t2-t1)/(float)max/(21*1000.0f)));

        jak_carbon_dot_path_drop(&path1);
        jak_carbon_dot_path_drop(&path2);
        jak_carbon_dot_path_drop(&path3);
        jak_carbon_dot_path_drop(&path4);
        jak_carbon_dot_path_drop(&path5);
        jak_carbon_dot_path_drop(&path6);
        jak_carbon_dot_path_drop(&path7);
        jak_carbon_dot_path_drop(&path8);
        jak_carbon_dot_path_drop(&path9);
        jak_carbon_dot_path_drop(&path10);
        jak_carbon_dot_path_drop(&path11);
        jak_carbon_dot_path_drop(&path12);
        jak_carbon_dot_path_drop(&path13);
        jak_carbon_dot_path_drop(&path14);
        jak_carbon_dot_path_drop(&path15);
        jak_carbon_dot_path_drop(&path16);
        jak_carbon_dot_path_drop(&path17);
        jak_carbon_dot_path_drop(&path18);
        jak_carbon_dot_path_drop(&path19);
        jak_carbon_dot_path_drop(&path20);
        jak_carbon_dot_path_drop(&path21);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonFromJsonShortenedDotPath)
{
        jak_carbon doc;
        jak_carbon_find find;
        jak_carbon_field_type_e result_type;
        struct jak_error err;

        const char *json_in = "{\"x\": \"y\"}";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        /* without shortened dot path rule, the json object as given is embedded in an record container (aka array)
         * such that the object must be referenced by its index in the record container (i.e., 0) */
        jak_carbon_find_open(&find, "0.x", &doc);
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&result_type, &find);
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        jak_carbon_find_close(&find);

        /* with shortened dot path rule, the json object can be referenced without providing its index in the record */
        jak_carbon_find_open(&find, "x", &doc);
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&result_type, &find);
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        jak_carbon_find_close(&find);

        jak_carbon_drop(&doc);

        json_in = "[{\"x\": \"y\"},{\"x\": [{\"z\": 42}]}]";
        jak_carbon_from_json(&doc, json_in, JAK_CARBON_KEY_NOKEY, NULL, &err);

        /* The shortened dot path rule does not apply here since the user input is an array  */
        jak_carbon_find_open(&find, "0.x", &doc);
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&result_type, &find);
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_STRING);
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "1.x", &doc);
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&result_type, &find);
        ASSERT_EQ(result_type, JAK_CARBON_FIELD_TYPE_ARRAY);
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x", &doc);
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        jak_carbon_find_close(&find);

        /* The shortened dot path rule does also never apply outside the record container  */
        jak_carbon_find_open(&find, "1.x.0.z", &doc);
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "1.x.z", &doc);
        ASSERT_FALSE(jak_carbon_find_has_result(&find));
        jak_carbon_find_close(&find);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonFindPrint)
{
        jak_carbon doc;
        struct jak_error err;
        jak_carbon_find find;
        char *result;

        jak_carbon_from_json(&doc, "8", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "8") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "-8", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "-8") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "\"A\"", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"A\"") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "32.4", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "32.40") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "null", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "true", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "true") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "false", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "false") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[1, 2, 3, null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "1") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[1, 2, 3, null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "1", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "2") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[1, 2, 3, null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "2", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "3") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[1, 2, 3, null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "3", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[1, 2, 3, null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "4", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "_nil") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[\"A\", \"B\", \"C\", null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"A\"") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[\"A\", \"B\", \"C\", null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "1", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"B\"") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[\"A\", \"B\", \"C\", null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "2", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"C\"") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[\"A\", \"B\", \"C\", null]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "3", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "\"Hello, World!\"", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"Hello, World!\"") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "{}", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "{}") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        jak_carbon_from_json(&doc, "[]", JAK_CARBON_KEY_NOKEY, NULL, &err);
        ASSERT_TRUE(jak_carbon_find_open(&find, "0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "_nil") == 0);
        jak_carbon_find_close(&find);
        free(result);
        jak_carbon_drop(&doc);

        const char *complex = "{\n"
                              "   \"m\": {\n"
                              "         \"n\":8,\n"
                              "         \"o\":-8,\n"
                              "         \"p\":\"A\",\n"
                              "         \"q\":32.4,\n"
                              "         \"r\":null,\n"
                              "         \"s\":true,\n"
                              "         \"t\":false,\n"
                              "         \"u\":[\n"
                              "            1,\n"
                              "            2,\n"
                              "            3,\n"
                              "            null\n"
                              "         ],\n"
                              "         \"v\":[\n"
                              "            \"A\",\n"
                              "            \"B\",\n"
                              "            null\n"
                              "         ],\n"
                              "         \"w\":\"Hello, World!\",\n"
                              "         \"x\":{\n"
                              "            \"a\": null\n"
                              "         },\n"
                              "         \"y\":[\n"
                              "\n"
                              "         ],\n"
                              "         \"z\":{\n"
                              "\n"
                              "         }\n"
                              "      }\n"
                              "}";

        jak_carbon_from_json(&doc, complex, JAK_CARBON_KEY_NOKEY, NULL, &err);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "{\"n\": 8, \"o\": -8, \"p\": \"A\", \"q\": 32.40, \"r\": null, \"s\": true, \"t\": false, \"u\": [1, 2, 3, null], \"v\": [\"A\", \"B\", null], \"w\": \"Hello, World!\", \"x\": {\"a\": null}, \"y\": [], \"z\": {}}") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.n", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "8") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.o", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "-8") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.p", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"A\"") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.q", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "32.40") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.r", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.s", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "true") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.t", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "false") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "[1, 2, 3, null]") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u.0", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "1") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u.1", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "2") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u.2", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "3") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u.3", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.u.4", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "_nil") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.v", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "[\"A\", \"B\", null]") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.w", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "\"Hello, World!\"") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.x", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "{\"a\": null}") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.x.a", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "null") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.y", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "[]") == 0);
        jak_carbon_find_close(&find);
        free(result);

        ASSERT_TRUE(jak_carbon_find_open(&find, "m.z", &doc));
        result = jak_carbon_find_result_to_json_compact_dup(&find);
        ASSERT_TRUE(strcmp(result, "{}") == 0);
        jak_carbon_find_close(&find);
        free(result);
        
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CarbonFindPrintExamples)
{
        jak_carbon doc;
        struct jak_error err;
        jak_carbon_find find;
        struct jak_string result;

        const char *json = "{\"x\": {\"y\": [{\"z\": 23}, {\"z\": null}]} }";

        jak_carbon_from_json(&doc, json, JAK_CARBON_KEY_NOKEY, NULL, &err);
        string_create(&result);

        printf("input: '%s'\n", json);

        jak_carbon_find_open(&find, "x", &doc);
        printf("x\t\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x.y", &doc);
        printf("x.y\t\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x.z", &doc);
        printf("x.z\t\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x.y.z", &doc);
        printf("x.y.z\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x.y.0.z", &doc);
        printf("x.y.0.z\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        jak_carbon_find_open(&find, "x.y.1.z", &doc);
        printf("x.y.0.z\t\t->\t%s\n", jak_carbon_find_result_to_json_compact(&result, &find));
        jak_carbon_find_close(&find);

        string_drop(&result);
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, ParseBooleanArray) {
        jak_carbon doc;
        struct jak_error err;
        jak_carbon_find find;
        jak_carbon_field_type_e type;
        const char *json = "[{\"col\": [true, null, false]}]";

        jak_carbon_from_json(&doc, json, JAK_CARBON_KEY_NOKEY, NULL, &err);

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.col", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&type, &find);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_COLUMN_BOOLEAN);
        jak_carbon_find_close(&find);

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.col.0", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&type, &find);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_TRUE);
        jak_carbon_find_close(&find);

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.col.1", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&type, &find);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_NULL);
        jak_carbon_find_close(&find);

        ASSERT_TRUE(jak_carbon_find_open(&find, "0.col.2", &doc));
        ASSERT_TRUE(jak_carbon_find_has_result(&find));
        jak_carbon_find_result_type(&type, &find);
        ASSERT_EQ(type, JAK_CARBON_FIELD_TYPE_FALSE);
        jak_carbon_find_close(&find);

        jak_carbon_drop(&doc);
}

TEST(CarbonTest, PathIndex) {
        struct jak_carbon_path_index index;
        jak_carbon doc;
        struct jak_error err;

        const char *json = "[\n"
                           "   {\n"
                           "      \"a\":null,\n"
                           "      \"b\":[ 1, 2, 3 ],\n"
                           "      \"c\":{\n"
                           "         \"msg\":\"Hello, World!\"\n"
                           "      }\n"
                           "   },\n"
                           "   {\n"
                           "      \"a\":42,\n"
                           "      \"b\":[ ],\n"
                           "      \"c\":null\n"
                           "   }\n"
                           "]";

//        int fd = open("./assets/ms-academic-graph.json", O_RDONLY);
//        ASSERT_NE(fd, -1);
//        int json_in_len = lseek(fd, 0, SEEK_END);
//        const char *json = (const char *) mmap(0, json_in_len, PROT_READ, MAP_PRIVATE, fd, 0);

        jak_carbon_from_json(&doc, json, JAK_CARBON_KEY_NOKEY, NULL, &err);
        carbon_path_index_create(&index, &doc);
        carbon_path_index_print(stdout, &index);
        jak_carbon_hexdump_print(stdout, &doc);
        carbon_path_index_hexdump(stdout, &index);

        jak_carbon path_carbon;
        carbon_path_index_to_carbon(&path_carbon, &index);
        jak_carbon_print(stdout, JAK_JSON_COMPACT, &path_carbon);
        jak_carbon_drop(&path_carbon);

        ASSERT_TRUE(carbon_path_index_indexes_doc(&index, &doc));
        jak_carbon_drop(&doc);
}

TEST(CarbonTest, CommitHashStr) {
        struct jak_string s;
        string_create(&s);

        ASSERT_TRUE(strcmp(jak_carbon_commit_hash_to_str(&s, 1), "0000000000000001") == 0);
        ASSERT_TRUE(strcmp(jak_carbon_commit_hash_to_str(&s, 42), "000000000000002a") == 0);
        ASSERT_TRUE(strcmp(jak_carbon_commit_hash_to_str(&s, 432432532532323), "0001894b8b7dac63") == 0);
        ASSERT_TRUE(strcmp(jak_carbon_commit_hash_to_str(&s, 2072006001577230657), "1cc13e7b007d0141") == 0);
        ASSERT_EQ(1, jak_carbon_commit_hash_from_str(jak_carbon_commit_hash_to_str(&s, 1), NULL));
        ASSERT_EQ(42, jak_carbon_commit_hash_from_str(jak_carbon_commit_hash_to_str(&s, 42), NULL));
        ASSERT_EQ(432432532532323, jak_carbon_commit_hash_from_str(jak_carbon_commit_hash_to_str(&s, 432432532532323), NULL));
        ASSERT_EQ(0, jak_carbon_commit_hash_from_str("", NULL));
        ASSERT_EQ(0, jak_carbon_commit_hash_from_str("hello", NULL));
        ASSERT_EQ(0, jak_carbon_commit_hash_from_str("000000000000001", NULL));
        ASSERT_EQ(0, jak_carbon_commit_hash_from_str("000000000000001Z", NULL));
        ASSERT_EQ(0, jak_carbon_commit_hash_from_str(NULL, NULL));

        string_drop(&s);
}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}