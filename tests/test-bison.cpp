#include <gtest/gtest.h>

#include "core/bison/bison.h"
#include "core/bison/bison-dot.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-insert.h"
#include "core/bison/bison-find.h"
#include "core/bison/bison-update.h"
#include "core/bison/bison-path.h"

TEST(BisonTest, CreateBison) {
        struct bison doc;
        object_id_t oid;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create_empty(&doc);
        EXPECT_TRUE(status);

        bison_hexdump_print(stderr, &doc);

        status = bison_object_id(&oid, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(oid, 0);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
}

TEST(BisonTest, CreateBisonRevisionNumbering) {
        struct bison doc, rev_doc;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create_empty(&doc);
        EXPECT_TRUE(status);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct bison_revise revise;
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_end(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        status = bison_revision(&rev, &rev_doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 1);

        status = bison_is_up_to_date(&doc);
        EXPECT_FALSE(status);

        status = bison_is_up_to_date(&rev_doc);
        EXPECT_TRUE(status);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, CreateBisonRevisionAbort) {
        struct bison doc, rev_doc;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create_empty(&doc);
        EXPECT_TRUE(status);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct bison_revise revise;
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_abort(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
}

TEST(BisonTest, CreateBisonRevisionAsyncReading) {
        struct bison doc, rev_doc;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create_empty(&doc);
        EXPECT_TRUE(status);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        struct bison_revise revise;
        bison_revise_begin(&revise, &rev_doc, &doc);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_revise_end(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

static void print_on_revision_begin(struct bison_event_listener *self, struct bison *doc)
{
        ng5_unused(self);
        ng5_unused(doc);
        printf("revision begins\n");
}

static void print_on_revision_end(struct bison_event_listener *self, struct bison *doc)
{
        ng5_unused(self);
        ng5_unused(doc);
        printf("revision end\n");
}

static void print_on_revision_abort(struct bison_event_listener *self, struct bison *doc)
{
        ng5_unused(self);
        ng5_unused(doc);
        printf("revision aborted\n");
}

static void print_on_new_revision(struct bison_event_listener *self, struct bison *revised, struct bison *original)
{
        ng5_unused(self);
        ng5_unused(revised);
        ng5_unused(original);
        printf("revision complete\n");
}

TEST(BisonTest, CreateBisonRevisionListening) {
        struct bison doc;
        struct bison rev_doc;
        u64 rev;
        struct string_builder builder;
        bool status;
        listener_handle_t handle;

        string_builder_create(&builder);

        status = bison_create_empty(&doc);

        struct bison_event_listener listener = {
                .on_revision_begin = print_on_revision_begin,
                .on_revision_end = print_on_revision_end,
                .on_revision_abort = print_on_revision_abort,
                .on_new_revision = print_on_new_revision
        };

        bison_register_listener(&handle, &listener, &doc);

        struct bison_revise revise;
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_abort(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
}

TEST(BisonTest, ForceBisonRevisionVarLengthIncrease) {
        struct bison doc, rev_doc;
        u64 old_rev;
        u64 new_rev;
        bool status;
        struct bison_revise revise;

        bison_create_empty(&doc);

        for (u32 i = 0; i < 20000; i++) {
                status = bison_revision(&old_rev, &doc);

                bison_revise_begin(&revise, &rev_doc, &doc);
                bison_revise_end(&revise);

                status = bison_revision(&new_rev, &doc);
                EXPECT_TRUE(status);
                EXPECT_EQ(new_rev, old_rev);

                status = bison_revision(&new_rev, &rev_doc);
                EXPECT_TRUE(status);
                EXPECT_EQ(new_rev, old_rev + 1);

                bison_print(stdout, &rev_doc);

                bison_drop(&doc);
                bison_clone(&doc, &rev_doc);
                bison_drop(&rev_doc);
        }


        bison_drop(&doc);
}

TEST(BisonTest, ModifyBisonObjectId) {
        struct bison doc, rev_doc;
        object_id_t oid;
        object_id_t new_oid;
        struct bison_revise revise;
        u64 rev;

        bison_create_empty(&doc);

        bison_object_id(&oid, &doc);
        EXPECT_EQ(oid, 0);

        bison_revision(&rev, &doc);
        EXPECT_EQ(rev, 0);
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_gen_object_id(&new_oid, &revise);
        EXPECT_NE(oid, new_oid);
        bison_revise_end(&revise);

        bison_revision(&rev, &rev_doc);
        EXPECT_NE(rev, 0);

        bison_object_id(&oid, &rev_doc);
        EXPECT_EQ(oid, new_oid);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonArrayIteratorOpenAfterNew) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;

        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_gen_object_id(NULL, &revise);
        bison_revise_iterator_open(&it, &revise);
        bool has_next = bison_array_it_next(&it);
        EXPECT_EQ(has_next, false);
        bison_revise_end(&revise);
        bison_array_it_drop(&it);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonArrayIteratorInsertNullAfterNew) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_revise_gen_object_id(NULL, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        bison_insert_null(&inserter);
        bison_insert_drop(&inserter);
        bison_revise_end(&revise);
        bison_array_it_drop(&it);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonArrayIteratorInsertMultipleLiteralsAfterNewNoOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 10; i++) {
                fprintf(stdout, "before:\n");
                bison_hexdump_print(stdout, &rev_doc);
                bool status;
                if (i % 3 == 0) {
                        status = bison_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        status = bison_insert_true(&inserter);
                } else {
                        status = bison_insert_false(&inserter);
                }
                ASSERT_TRUE(status);
                fprintf(stdout, "after:\n");
                bison_hexdump_print(stdout, &rev_doc);
                fprintf(stdout, "\n\n");
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonArrayIteratorOverwriteLiterals) {
        struct bison doc, rev_doc, rev_doc2;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 3; i++) {
                if (i % 3 == 0) {
                        bison_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        bison_insert_true(&inserter);
                } else {
                        bison_insert_false(&inserter);
                }
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 2; i++) {
                bison_insert_true(&inserter);
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc2);

        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);
}

TEST(BisonTest, BisonArrayIteratorOverwriteLiteralsWithDocOverflow) {
        struct bison doc, rev_doc, rev_doc2;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 22; i++) {
                if (i % 3 == 0) {
                        bison_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        bison_insert_true(&inserter);
                } else {
                        bison_insert_false(&inserter);
                }
               // fprintf(stdout, "after initial push:\n");
               // bison_hexdump_print(stdout, &rev_doc);
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 2; i++) {
                fprintf(stdout, "before:\n");
                bison_hexdump_print(stdout, &rev_doc2);
                bison_insert_true(&inserter);
                fprintf(stdout, "after:\n");
                bison_hexdump_print(stdout, &rev_doc2);
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);
        bison_print(stdout, &rev_doc2);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);
}

TEST(BisonTest, BisonArrayIteratorUnsignedAndConstants) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 500; i++) {
                if (i % 6 == 0) {
                        bison_insert_null(&inserter);
                } else if (i % 6 == 1) {
                        bison_insert_true(&inserter);
                } else if (i % 6 == 2) {
                        bison_insert_false(&inserter);
                } else if (i % 6 == 3) {
                        u64 rand_value = random();
                        bison_insert_unsigned(&inserter, rand_value);
                } else if (i % 6 == 4) {
                        i64 rand_value = random();
                        bison_insert_signed(&inserter, rand_value);
                } else {
                        float rand_value = (float)rand()/(float)(RAND_MAX/INT32_MAX);
                        bison_insert_float(&inserter, rand_value);
                }
                //fprintf(stdout, "after initial push:\n");
                //bison_hexdump_print(stdout, &rev_doc);
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonArrayIteratorStrings) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        for (i32 i = 0; i < 10; i++) {
                u64 strlen = rand() % (100 + 1 - 4) + 4;
                char buffer[strlen];
                for (i32 j = 0; j < strlen; j++) {
                        buffer[j] = 65 + (rand() % 25);
                }
                buffer[0] = '!';
                buffer[strlen - 2] = '!';
                buffer[strlen - 1] = '\0';
                bison_insert_string(&inserter, buffer);
                //fprintf(stdout, "after initial push:\n");
                //bison_hexdump_print(stdout, &rev_doc);
        }
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertMimeTypedBlob) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        const char *data = "{ \"Message\": \"Hello World\" }";
        bool status = bison_insert_binary(&inserter, data, strlen(data), "json", NULL);
        ASSERT_TRUE(status);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertCustomTypedBlob) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        const char *data = "{ \"Message\": \"Hello World\" }";
        bool status = bison_insert_binary(&inserter, data, strlen(data), NULL, "my data");
        ASSERT_TRUE(status);
        //bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertTwoMimeTypedBlob) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        bool status = bison_insert_binary(&inserter, data1, strlen(data1), "json", NULL);
        ASSERT_TRUE(status);
        status = bison_insert_binary(&inserter, data2, strlen(data2), "txt", NULL);
        ASSERT_TRUE(status);
        //bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertMimeTypedBlobsWithOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        for (u32 i = 0; i < 100; i++) {
                bool status = bison_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
                        strlen(i % 2 == 0 ? data1 : data2), "json", NULL);
                ASSERT_TRUE(status);
        }
        bison_hexdump_print(stdout, &rev_doc);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertMixedTypedBlobsWithOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        const char *data1 = "{ \"Message\": \"Hello World\" }";
        const char *data2 = "{ \"Blog-Header\": \"My Fancy Blog\" }";
        for (u32 i = 0; i < 100; i++) {
                bool status = bison_insert_binary(&inserter, i % 2 == 0 ? data1 : data2,
                        strlen(i % 2 == 0 ? data1 : data2), i % 3 == 0 ? "json" : NULL, i % 5 == 0 ? "user/app" : NULL);
                ASSERT_TRUE(status);
        }
        //bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertArrayWithNoOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);
        bison_hexdump_print(stdout, &rev_doc);

        struct bison_insert *nested_inserter = bison_insert_array_begin(&array_state, &inserter, 10);
        ASSERT_TRUE(nested_inserter != NULL);
        bison_insert_array_end(&array_state);

        bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);
        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertValuesIntoNestedArrayWithNoOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_null(&inserter);
        bison_insert_null(&inserter);
        bison_insert_null(&inserter);

        struct bison_insert *nested_inserter = bison_insert_array_begin(&array_state, &inserter, 10);
        ASSERT_TRUE(nested_inserter != NULL);
        bison_insert_true(nested_inserter);
        bison_insert_true(nested_inserter);
        bison_insert_true(nested_inserter);
        bison_insert_array_end(&array_state);

        bison_insert_false(&inserter);
        bison_insert_false(&inserter);
        bison_insert_false(&inserter);


        bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsert2xNestedArrayWithNoOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state_l1, array_state_l2;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_null(&inserter);
        bison_insert_null(&inserter);
        bison_insert_null(&inserter);

        struct bison_insert *nested_inserter_l1 = bison_insert_array_begin(&array_state_l1, &inserter, 10);
        ASSERT_TRUE(nested_inserter_l1 != NULL);
        bison_insert_true(nested_inserter_l1);
        bison_insert_true(nested_inserter_l1);
        bison_insert_true(nested_inserter_l1);

        struct bison_insert *nested_inserter_l2 = bison_insert_array_begin(&array_state_l2, nested_inserter_l1, 10);
        ASSERT_TRUE(nested_inserter_l2 != NULL);
        bison_insert_true(nested_inserter_l2);
        bison_insert_false(nested_inserter_l2);
        bison_insert_null(nested_inserter_l2);
        bison_insert_array_end(&array_state_l2);

        bison_insert_array_end(&array_state_l1);

        bison_insert_false(&inserter);
        bison_insert_false(&inserter);
        bison_insert_false(&inserter);


        bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertXxNestedArrayWithoutOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state_l1;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_null(&inserter);
        bison_insert_null(&inserter);
        bison_insert_null(&inserter);

        for (int i = 0; i < 10; i++) {
                struct bison_insert *nested_inserter_l1 = bison_insert_array_begin(&array_state_l1, &inserter, 10);
                ASSERT_TRUE(nested_inserter_l1 != NULL);
                bison_insert_true(nested_inserter_l1);
                bison_insert_true(nested_inserter_l1);
                bison_insert_true(nested_inserter_l1);
                bison_insert_array_end(&array_state_l1);
        }

        bison_insert_false(&inserter);
        bison_insert_false(&inserter);
        bison_insert_false(&inserter);

        bison_hexdump_print(stdout, &rev_doc);
        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertXxNestedArrayWithOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state_l1;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_hexdump_print(stdout, &rev_doc);
        printf("\n");

        bison_insert_null(&inserter);

        bison_hexdump_print(stdout, &rev_doc);
        printf("\n");

        bison_insert_null(&inserter);

        bison_hexdump_print(stdout, &rev_doc);
        printf("\n");

        bison_insert_null(&inserter);

        for (int i = 0; i < 10; i++) {
                struct bison_insert *nested_inserter_l1 = bison_insert_array_begin(&array_state_l1, &inserter, 1);
                ASSERT_TRUE(nested_inserter_l1 != NULL);
                bison_insert_true(nested_inserter_l1);
                bison_insert_true(nested_inserter_l1);
                bison_insert_true(nested_inserter_l1);
                bison_insert_array_end(&array_state_l1);
        }

        bison_insert_false(&inserter);
        bison_insert_false(&inserter);
        bison_insert_false(&inserter);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [null, null, null, [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], [true, true, true], false, false, false]}"));        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertInsertColumnWithoutOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NULL, 10);

        bison_hexdump_print(stdout, &rev_doc);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        bison_insert_null(nested_inserter_l1);
        bison_insert_null(nested_inserter_l1);
        bison_insert_null(nested_inserter_l1);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[null, null, null]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertInsertColumnNumbersWithoutOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 10);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        bison_insert_u8(nested_inserter_l1, 42);
        bison_insert_u8(nested_inserter_l1, 43);
        bison_insert_u8(nested_inserter_l1, 44);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[42, 43, 44]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertInsertColumnNumbersZeroWithoutOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 10);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        bison_insert_u8(nested_inserter_l1, 0);
        bison_insert_u8(nested_inserter_l1, 0);
        bison_insert_u8(nested_inserter_l1, 0);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[0, 0, 0]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertInsertMultileTypedColumnsWithoutOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;
        struct bison_insert *ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NULL, 10);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_TRUE, 10);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_FALSE, 10);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 10);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 2);
        bison_insert_u8(ins, 3);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U16, 10);
        bison_insert_u16(ins, 4);
        bison_insert_u16(ins, 5);
        bison_insert_u16(ins, 6);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(ins, 7);
        bison_insert_u32(ins, 8);
        bison_insert_u32(ins, 9);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U64, 10);
        bison_insert_u64(ins, 10);
        bison_insert_u64(ins, 11);
        bison_insert_u64(ins, 12);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I8, 10);
        bison_insert_i8(ins, -1);
        bison_insert_i8(ins, -2);
        bison_insert_i8(ins, -3);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I16, 10);
        bison_insert_i16(ins, -4);
        bison_insert_i16(ins, -5);
        bison_insert_i16(ins, -6);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I32, 10);
        bison_insert_i32(ins, -7);
        bison_insert_i32(ins, -8);
        bison_insert_i32(ins, -9);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I64, 10);
        bison_insert_i64(ins, -10);
        bison_insert_i64(ins, -11);
        bison_insert_i64(ins, -12);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_FLOAT, 10);
        bison_insert_float(ins, 42.0f);
        bison_insert_float(ins, 21.0f);
        bison_insert_float(ins, 23.4221f);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        string_builder_print(&sb);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[null, null, null], [true, true, true], [false, false, false], [1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12], [-1, -2, -3], [-4, -5, -6], [-7, -8, -9], [-10, -11, -12], [42.00, 21.00, 23.42]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertInsertColumnNumbersZeroWithOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 16, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 1);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        bison_insert_u8(nested_inserter_l1, 1);
        bison_insert_u8(nested_inserter_l1, 2);
        bison_insert_u8(nested_inserter_l1, 3);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        printf("BISON DOC PRINT:");
        bison_print(stdout, &rev_doc);
        fflush(stdout);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}


TEST(BisonTest, BisonInsertInsertColumnNumbersWithHighOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 16, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U32, 1);

        ASSERT_TRUE(nested_inserter_l1 != NULL);
        for (u32 i = 0; i < 100; i++) {
                bison_insert_u32(nested_inserter_l1, i);
                bison_insert_u32(nested_inserter_l1, i);
                bison_insert_u32(nested_inserter_l1, i);
        }

        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        printf("BISON DOC PRINT:");
        bison_print(stdout, &rev_doc);
        fflush(stdout);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 82, 82, 82, 83, 83, 83, 84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 88, 88, 88, 89, 89, 89, 90, 90, 90, 91, 91, 91, 92, 92, 92, 93, 93, 93, 94, 94, 94, 95, 95, 95, 96, 96, 96, 97, 97, 97, 98, 98, 98, 99, 99, 99]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}


TEST(BisonTest, BisonInsertInsertMultipleColumnsNumbersWithHighOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;

        bison_create_empty_ex(&doc, 16, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        for (u32 k = 0; k < 3; k++) {
                struct bison_insert *nested_inserter_l1 = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U32, 1);

                ASSERT_TRUE(nested_inserter_l1 != NULL);
                for (u32 i = 0; i < 4; i++) {
                        bison_insert_u32(nested_inserter_l1, 'a' + i);
                        bison_insert_u32(nested_inserter_l1, 'a' + i);
                        bison_insert_u32(nested_inserter_l1, 'a' + i);
                }

                bison_insert_column_end(&column_state);
        }

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        //bison_hexdump_print(stdout, &rev_doc);

        printf("BISON DOC PRINT:");
        bison_print(stdout, &rev_doc);
        fflush(stdout);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100], [97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonInsertNullTest) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;
        struct bison_insert *ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NULL, 10);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_TRUE, 10);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_FALSE, 10);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 10);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, U8_NULL);
        bison_insert_u8(ins, 3);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U16, 10);
        bison_insert_u16(ins, 4);
        bison_insert_u16(ins, U16_NULL);
        bison_insert_u16(ins, 6);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(ins, 7);
        bison_insert_u32(ins, U32_NULL);
        bison_insert_u32(ins, 9);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U64, 10);
        bison_insert_u64(ins, 10);
        bison_insert_u64(ins, U64_NULL);
        bison_insert_u64(ins, 12);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I8, 10);
        bison_insert_i8(ins, -1);
        bison_insert_i8(ins, I8_NULL);
        bison_insert_i8(ins, -3);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I16, 10);
        bison_insert_i16(ins, -4);
        bison_insert_i16(ins, I16_NULL);
        bison_insert_i16(ins, -6);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I32, 10);
        bison_insert_i32(ins, -7);
        bison_insert_i32(ins, I32_NULL);
        bison_insert_i32(ins, -9);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I64, 10);
        bison_insert_i64(ins, -10);
        bison_insert_i64(ins, I64_NULL);
        bison_insert_i64(ins, -12);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_FLOAT, 10);
        bison_insert_float(ins, 42.0f);
        bison_insert_float(ins, FLOAT_NULL);
        bison_insert_float(ins, 23.4221f);
        bison_insert_column_end(&column_state);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[null, null, null], [true, true, true], [false, false, false], [1, null, 3], [4, null, 6], [7, null, 9], [10, null, 12], [-1, null, -3], [-4, null, -6], [-7, null, -9], [-10, null, -12], [42.00, null, 23.42]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonShrinkColumnListTest) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;
        struct bison_insert *ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NULL, 10);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_null(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_TRUE, 10);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_true(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_FALSE, 10);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_false(ins);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U8, 10);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, U8_NULL);
        bison_insert_u8(ins, 2);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U16, 10);
        bison_insert_u16(ins, 3);
        bison_insert_u16(ins, U16_NULL);
        bison_insert_u16(ins, 4);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(ins, 5);
        bison_insert_u32(ins, U32_NULL);
        bison_insert_u32(ins, 6);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_U64, 10);
        bison_insert_u64(ins, 7);
        bison_insert_u64(ins, U64_NULL);
        bison_insert_u64(ins, 8);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I8, 10);
        bison_insert_i8(ins, 9);
        bison_insert_i8(ins, I8_NULL);
        bison_insert_i8(ins, 10);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I16, 10);
        bison_insert_i16(ins, 11);
        bison_insert_i16(ins, I16_NULL);
        bison_insert_i16(ins, 12);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I32, 10);
        bison_insert_i32(ins, 13);
        bison_insert_i32(ins, I32_NULL);
        bison_insert_i32(ins, 14);
        bison_insert_column_end(&column_state);

        ins = bison_insert_column_begin(&column_state, &inserter, BISON_FIELD_TYPE_NUMBER_I64, 10);
        bison_insert_i64(ins, 15);
        bison_insert_i64(ins, I64_NULL);
        bison_insert_i64(ins, 16);
        bison_insert_column_end(&column_state);

        bison_hexdump_print(stdout, &rev_doc);
        bison_revise_shrink(&revise);
        bison_hexdump_print(stdout, &rev_doc);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[null, null, null], [true, true, true], [false, false, false], [1, null, 2], [3, null, 4], [5, null, 6], [7, null, 8], [9, null, 10], [11, null, 12], [13, null, 14], [15, null, 16]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonShrinkArrayListTest) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state;
        struct bison_insert *ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 1);
        bison_insert_array_end(&array_state);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 2);
        bison_insert_u8(ins, 3);
        bison_insert_u8(ins, 4);
        bison_insert_array_end(&array_state);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 5);
        bison_insert_u8(ins, 6);
        bison_insert_u8(ins, 7);
        bison_insert_array_end(&array_state);

        bison_hexdump_print(stdout, &rev_doc);
        bison_revise_shrink(&revise);
        bison_hexdump_print(stdout, &rev_doc);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 1, 1], [2, 3, 4], [5, 6, 7]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonShrinkNestedArrayListTest) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_array_state array_state, nested_array_state;
        struct bison_insert *ins, *nested_ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
        bison_insert_string(nested_ins, "Hello");
        bison_insert_string(nested_ins, "World");
        bison_insert_array_end(&nested_array_state);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 1);
        bison_insert_array_end(&array_state);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 2);
        nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
        bison_insert_string(nested_ins, "Hello");
        bison_insert_string(nested_ins, "World");
        bison_insert_array_end(&nested_array_state);
        bison_insert_u8(ins, 3);
        bison_insert_u8(ins, 4);
        bison_insert_array_end(&array_state);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 5);
        bison_insert_u8(ins, 6);
        nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
        bison_insert_string(nested_ins, "Hello");
        bison_insert_string(nested_ins, "World");
        bison_insert_array_end(&nested_array_state);
        bison_insert_u8(ins, 7);
        bison_insert_array_end(&array_state);

        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        bison_insert_u8(ins, 8);
        bison_insert_u8(ins, 9);
        bison_insert_u8(ins, 10);
        nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
        bison_insert_string(nested_ins, "Hello");
        bison_insert_string(nested_ins, "World");
        bison_insert_array_end(&nested_array_state);
        bison_insert_array_end(&array_state);

        bison_hexdump_print(stdout, &rev_doc);
        bison_revise_shrink(&revise);
        bison_hexdump_print(stdout, &rev_doc);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);
        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[[\"Hello\", \"World\"], 1, 1, 1], [2, [\"Hello\", \"World\"], 3, 4], [5, 6, [\"Hello\", \"World\"], 7], [8, 9, 10, [\"Hello\", \"World\"]]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonShrinkNestedArrayListAndColumnListTest) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_insert_column_state column_state;
        struct bison_insert_array_state array_state, nested_array_state;
        struct bison_insert *ins, *nested_ins, *column_ins;

        bison_create_empty_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u64(&inserter, 4223);
        ins = bison_insert_array_begin(&array_state, &inserter, 10);
                column_ins = bison_insert_column_begin(&column_state, ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
                        bison_insert_u32(column_ins, 'X');
                        bison_insert_u32(column_ins, 'Y');
                        bison_insert_u32(column_ins, 'Z');
                bison_insert_column_end(&column_state);
                nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
                        bison_insert_string(nested_ins, "Hello");
                        column_ins = bison_insert_column_begin(&column_state, nested_ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
                                bison_insert_u32(column_ins, 'A');
                                bison_insert_u32(column_ins, 'B');
                                bison_insert_u32(column_ins, 'C');
                        bison_insert_column_end(&column_state);
                        bison_insert_string(nested_ins, "World");
                bison_insert_array_end(&nested_array_state);
                bison_insert_u8(ins, 1);
                bison_insert_u8(ins, 1);
                column_ins = bison_insert_column_begin(&column_state, ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
                        bison_insert_u32(column_ins, 23);
                        bison_insert_u32(column_ins, 24);
                        bison_insert_u32(column_ins, 25);
                bison_insert_column_end(&column_state);
                bison_insert_u8(ins, 1);
        bison_insert_array_end(&array_state);

        bison_hexdump_print(stdout, &rev_doc);
        bison_revise_shrink(&revise);
        bison_hexdump_print(stdout, &rev_doc);

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);

        struct string_builder sb;
        string_builder_create(&sb);
        bison_to_str(&sb, JSON_FORMATTER, &rev_doc);

        fprintf(stdout, "IST  %s\n", string_builder_cstr(&sb));
        fprintf(stdout, "SOLL {\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}\n");

        ASSERT_TRUE(0 == strcmp(string_builder_cstr(&sb), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [4223, [[88, 89, 90], [\"Hello\", [65, 66, 67], \"World\"], 1, 1, [23, 24, 25], 1]]}"));
        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
}

TEST(BisonTest, BisonDotNotation) {
        struct bison_dot_path path;
        struct string_builder sb;
        string_builder_create(&sb);

        bison_dot_path_create(&path);

        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_key(&path, "name");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_key(&path, "my name");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\"") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_key(&path, "");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\"") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_idx(&path, 42);
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_idx(&path, 23);
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42.23") == 0);
        string_builder_clear(&sb);

        bison_dot_path_add_key(&path, "\"already quotes\"");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name.\"my name\".\"\".42.23.\"already quotes\"") == 0);
        string_builder_clear(&sb);

        bison_dot_path_drop(&path);
        string_builder_drop(&sb);
}

TEST(BisonTest, BisonDotNotationParsing) {
        struct bison_dot_path path;
        struct string_builder sb;
        string_builder_create(&sb);

        bison_dot_path_from_string(&path, "name");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "   name");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "   name    ");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "\"name\"");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "name") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "\"nam e\"");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "\"nam e\"") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "nam e");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "nam.e") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "\"My Doc\" names 5 age");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "\"My Doc\".names.5.age") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        bison_dot_path_from_string(&path, "23.authors.3.name");
        bison_dot_path_to_str(&sb, &path);
        ASSERT_TRUE(strcmp(string_builder_cstr(&sb), "23.authors.3.name") == 0);
        string_builder_clear(&sb);
        bison_dot_path_drop(&path);

        string_builder_drop(&sb);
}

TEST(BisonTest, BisonFind) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert ins;
        struct bison_find finder;
        u64 result_unsigned;
        enum bison_field_type type;
        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);

        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&ins, &it);
        bison_insert_u8(&ins, 'a');
        bison_insert_u8(&ins, 'b');
        bison_insert_u8(&ins, 'c');
        bison_array_it_insert_end(&ins);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);

        {
                bison_find_open(&finder, "0", &rev_doc);

                ASSERT_TRUE(bison_find_has_result(&finder));

                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);

                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'a');

                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1", &rev_doc);

                ASSERT_TRUE(bison_find_has_result(&finder));

                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);

                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'b');

                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "2", &rev_doc);

                ASSERT_TRUE(bison_find_has_result(&finder));

                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);

                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 'c');

                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "3", &rev_doc);

                ASSERT_FALSE(bison_find_has_result(&finder));

                bison_find_close(&finder);
        }

        bison_print(stdout, &rev_doc);
        bison_drop(&doc);
}

TEST(BisonTest, BisonFindTypes) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter, *ins, *nested_ins, *column_ins;
        struct bison_insert_column_state column_state;
        struct bison_insert_array_state array_state, nested_array_state;
        struct bison_find finder;
        u64 result_unsigned;
        enum bison_field_type type;
        bison_create_empty(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u64(&inserter, 4223);
        ins = bison_insert_array_begin(&array_state, &inserter, 10);
        column_ins = bison_insert_column_begin(&column_state, ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(column_ins, 'X');
        bison_insert_u32(column_ins, 'Y');
        bison_insert_u32(column_ins, 'Z');
        bison_insert_column_end(&column_state);
        nested_ins = bison_insert_array_begin(&nested_array_state, ins, 10);
        bison_insert_string(nested_ins, "Hello");
        column_ins = bison_insert_column_begin(&column_state, nested_ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(column_ins, 'A');
        bison_insert_u32(column_ins, 'B');
        bison_insert_u32(column_ins, 'C');
        bison_insert_column_end(&column_state);
        bison_insert_string(nested_ins, "World");
        bison_insert_array_end(&nested_array_state);
        bison_insert_u8(ins, 1);
        bison_insert_u8(ins, 1);
        column_ins = bison_insert_column_begin(&column_state, ins, BISON_FIELD_TYPE_NUMBER_U32, 10);
        bison_insert_u32(column_ins, 23);
        bison_insert_u32(column_ins, 24);
        bison_insert_u32(column_ins, 25);
        bison_insert_column_end(&column_state);
        bison_insert_u8(ins, 1);
        bison_insert_array_end(&array_state);

        bison_revise_shrink(&revise);

        {
                bison_find_open(&finder, "0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U64);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 4223);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_ARRAY);
                struct bison_array_it *retval = bison_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_COLUMN);
                struct bison_column_it *retval = bison_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.0.0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 88);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.0.1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 89);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.0.2", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 90);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.0.3", &rev_doc);
                ASSERT_FALSE(bison_find_has_result(&finder));
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_ARRAY);
                struct bison_array_it *retval = bison_find_result_array(&finder);
                ASSERT_TRUE(retval != NULL);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_STRING);
                u64 str_len;
                const char *retval = bison_find_result_string(&str_len, &finder);
                ASSERT_TRUE(strncmp(retval, "Hello", str_len) == 0);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_COLUMN);
                struct bison_column_it *retval = bison_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.1.0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 65);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.1.1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 66);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.1.2", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 67);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.1.3", &rev_doc);
                ASSERT_FALSE(bison_find_has_result(&finder));
        }

        {
                bison_find_open(&finder, "1.1.2", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_STRING);
                u64 str_len;
                const char *retval = bison_find_result_string(&str_len, &finder);
                ASSERT_TRUE(strncmp(retval, "World", str_len) == 0);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.1.3", &rev_doc);
                ASSERT_FALSE(bison_find_has_result(&finder));
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.2", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.3", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.4", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_COLUMN);
                struct bison_column_it *retval = bison_find_result_column(&finder);
                ASSERT_TRUE(retval != NULL);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.4.0", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 23);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.4.1", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 24);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.4.2", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 25);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.4.3", &rev_doc);
                ASSERT_FALSE(bison_find_has_result(&finder));
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.5", &rev_doc);
                ASSERT_TRUE(bison_find_has_result(&finder));
                bison_find_result_type(&type, &finder);
                ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U8);
                bison_find_result_unsigned(&result_unsigned, &finder);
                ASSERT_EQ(result_unsigned, 1);
                bison_find_close(&finder);
        }

        {
                bison_find_open(&finder, "1.6", &rev_doc);
                ASSERT_FALSE(bison_find_has_result(&finder));
                bison_find_close(&finder);
        }

        bison_insert_drop(&inserter);
        bison_array_it_drop(&it);
        bison_revise_end(&revise);

        bison_hexdump_print(stdout, &rev_doc);

        bison_print(stdout, &rev_doc);
        bison_drop(&rev_doc);
        bison_drop(&doc);
}

TEST(BisonTest, BisonUpdateU8Simple)
{
        struct bison doc, rev_doc, rev_doc2, rev_doc3, rev_doc4;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_update updater;
        struct string_builder sb;
        const char *json;

        string_builder_create(&sb);
        bison_create_empty(&doc);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u8(&inserter, 'X');

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc);
        printf("JSON (rev1): %s\n", json);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_update_u8(&revise, "0", 'Y');

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc2);
        printf("JSON (rev2): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [89]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc3, &rev_doc2);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u8(&inserter, 'A');
        bison_insert_u8(&inserter, 'B');
        bison_update_u8(&revise, "2", 'C');

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc3);
        printf("JSON (rev3): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 3}, \"doc\": [65, 66, 67]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc4, &rev_doc3);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_update_u8(&revise, "0", 1);
        bison_update_u8(&revise, "1", 2);
        bison_update_u8(&revise, "2", 3);

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc4);
        printf("JSON (rev4): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 4}, \"doc\": [1, 2, 3]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);
        bison_drop(&rev_doc3);

}

TEST(BisonTest, BisonUpdateMixedFixedTypesSimple)
{
        struct bison doc, rev_doc, rev_doc2, rev_doc3, rev_doc4;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_update updater;
        struct string_builder sb;
        const char *json;

        string_builder_create(&sb);
        bison_create_empty(&doc);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u8(&inserter, 1);
        bison_insert_i64(&inserter, -42);
        bison_insert_float(&inserter, 23);

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc);
        printf("JSON (rev1): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [1, -42, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_update_i64(&revise, "1", 1024);

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc2);
        printf("JSON (rev2): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [1, 1024, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        u64 e1 = bison_get_or_default_unsigned(&rev_doc2, "0", 0);
        i64 e2 = bison_get_or_default_signed(&rev_doc2, "1", 0);
        float e3 = bison_get_or_default_float(&rev_doc2, "2", NAN);

        ASSERT_EQ(e1, 1);
        ASSERT_EQ(e2, 1024);
        ASSERT_TRUE(e3 > 22.9f && e3 < 24.0f);

        // -------------------------------------------------------------------------------------------------------------

        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);

}

TEST(BisonTest, BisonRemoveConstantsToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_null(ins);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [null]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstConstants)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_true(ins);
        bison_insert_false(ins);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_FALSE);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [true, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [false]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastConstants)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_true(ins);
        bison_insert_false(ins);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [true, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [true]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleConstants)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_true(ins);
        bison_insert_null(ins);
        bison_insert_false(ins);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_FALSE);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [true, null, false]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [true, false]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveNumberToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_u8(ins, 42);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [42]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstNumber)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_u8(ins, 42);
        bison_insert_u32(ins, 23);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_NUMBER_U32);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [42, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [23]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastNumber)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_u8(ins, 42);
        bison_insert_u32(ins, 23);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [42, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [42]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleNumber)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_u8(ins, 42);
        bison_insert_u16(ins, 21);
        bison_insert_u32(ins, 23);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U32);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [42, 21, 23]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [42, 23]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}


TEST(BisonTest, BisonRemoveStringToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_string(ins, "Hello");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [\"Hello\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstString)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_string(ins, "Hello");
        bison_insert_string(ins, "World");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_STRING);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [\"World\"]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastString)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_string(ins, "Hello");
        bison_insert_string(ins, "World");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [\"Hello\"]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleString)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        bison_insert_string(ins, "Plato");
        bison_insert_string(ins, "Kant");
        bison_insert_string(ins, "Nietzsche");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_STRING);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [\"Plato\", \"Kant\", \"Nietzsche\"]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [\"Plato\", \"Nietzsche\"]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}





TEST(BisonTest, BisonRemoveBinaryToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data, strlen(data), "txt", NULL);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), "json", NULL);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_BINARY);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), "json", NULL);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), "txt", NULL);

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), "json", NULL);

        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
        bison_insert_binary(ins, data3, strlen(data3), "html", NULL);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_BINARY);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/json\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"text/plain\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"text/html\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}











TEST(BisonTest, BisonRemoveCustomBinaryToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data = "ABC";
        bison_insert_binary(ins, data, strlen(data), NULL, "123");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        bison_hexdump_print(stdout, &doc);

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"123\", \"encoding\": \"base64\", \"binary-string\": \"A=JDAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstCustomBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_BINARY_CUSTOM);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastCustomBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleCustomBinary)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        const char *data1 = "This report, by its very length, defends itself against the risk of being read.";
        bison_insert_binary(ins, data1, strlen(data1), NULL, "my-fancy-format");

        const char *data2 = "{\"key\": \"value\"}";
        bison_insert_binary(ins, data2, strlen(data2), NULL, "application/something-json-like");

        const char *data3 = "<html><body><p>The quick brown fox jumps over the lazy dog</p></body></html>";
        bison_insert_binary(ins, data3, strlen(data3), NULL, "my-other-nonstandard-format");

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_BINARY_CUSTOM);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"application/something-json-like\", \"encoding\": \"base64\", \"binary-string\": \"eyJrZXkiOiAidmFsdWUifQAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [{ \"type\": \"my-fancy-format\", \"encoding\": \"base64\", \"binary-string\": \"VGhpcyByZXBvcnQsIGJ5IGl0cyB2ZXJ5IGxlbmd0aCwgZGVmZW5kcyBpdHNlbGYgYWdhaW5zdCB0aGUgcmlzayBvZiBiZWluZyByZWFkLgAA\" }, { \"type\": \"my-other-nonstandard-format\", \"encoding\": \"base64\", \"binary-string\": \"PGh0bWw+PGJvZHk+PHA+VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZzwvcD48L2JvZHk+PC9odG1sPgAA\" }]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}










TEST(BisonTest, BisonRemoveArrayToEmpty)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        struct bison_insert_array_state state;
        struct bison_insert *array_ins;
        bool has_next;
        string_builder_create(&sb);

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 1);
        bison_insert_u8(array_ins, 2);
        bison_insert_u8(array_ins, 3);
        bison_insert_array_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        bison_hexdump_print(stdout, &doc);

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": []}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveFirstArray)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        struct bison_insert_array_state state;
        struct bison_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 1);
        bison_insert_u8(array_ins, 2);
        bison_insert_u8(array_ins, 3);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 4);
        bison_insert_u8(array_ins, 5);
        bison_insert_u8(array_ins, 6);
        bison_insert_array_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_field_type next_type;
        bison_array_it_field_type(&next_type, &rev_it);
        ASSERT_EQ(next_type, BISON_FIELD_TYPE_ARRAY);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [[4, 5, 6]]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveLastArray)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        struct bison_insert_array_state state;
        struct bison_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 1);
        bison_insert_u8(array_ins, 2);
        bison_insert_u8(array_ins, 3);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 4);
        bison_insert_u8(array_ins, 5);
        bison_insert_u8(array_ins, 6);
        bison_insert_array_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------


        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [[1, 2, 3]]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonRemoveMiddleArray)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        struct bison_insert_array_state state;
        struct bison_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 1);
        bison_insert_u8(array_ins, 2);
        bison_insert_u8(array_ins, 3);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 4);
        bison_insert_u8(array_ins, 5);
        bison_insert_u8(array_ins, 6);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 7);
        bison_insert_u8(array_ins, 8);
        bison_insert_u8(array_ins, 9);
        bison_insert_array_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------
        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        bison_array_it_remove(&rev_it);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        enum bison_field_type type;
        bison_array_it_field_type(&type, &rev_it);
        ASSERT_EQ(type, BISON_FIELD_TYPE_ARRAY);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_FALSE(has_next);
        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);
        // -------------------------------------------------------------------------------------------------------------

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        // -------------------------------------------------------------------------------------------------------------

        printf("BEFORE\t'%s'\nAFTER\t'%s'\n", json_1, json_2);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3], [4, 5, 6], [7, 8, 9]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [[1, 2, 3], [7, 8, 9]]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
}

TEST(BisonTest, BisonColumnRemoveTest)
{
        struct bison doc, rev_doc;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);
        bool status;
        const u16 *values;

        struct bison_insert_column_state state;
        struct bison_insert *array_ins;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_KEEP);

        array_ins = bison_insert_column_begin(&state, ins, BISON_FIELD_TYPE_NUMBER_U16, 10);
        bison_insert_u16(array_ins, 1);
        bison_insert_u16(array_ins, 2);
        bison_insert_u16(array_ins, 3);
        bison_insert_column_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&rev_it, &revise);
        has_next = bison_array_it_next(&rev_it);
        ASSERT_TRUE(has_next);
        struct bison_column_it *cit = bison_array_it_column_value(&rev_it);
        enum bison_field_type type;
        u32 num_elems;
        bison_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U16);
        ASSERT_EQ(num_elems, 3);

        status = bison_column_it_remove(cit, 1);
        ASSERT_TRUE(status);
        bison_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U16);
        ASSERT_EQ(num_elems, 2);
        values = bison_column_it_u16_values(&num_elems, cit);
        ASSERT_EQ(values[0], 1);
        ASSERT_EQ(values[1], 3);

        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));

        status = bison_column_it_remove(cit, 0);
        ASSERT_TRUE(status);
        bison_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U16);
        ASSERT_EQ(num_elems, 1);
        values = bison_column_it_u16_values(&num_elems, cit);
        ASSERT_EQ(values[0], 3);

        char *json_3 = strdup(bison_to_json(&sb, &rev_doc));

        status = bison_column_it_remove(cit, 0);
        ASSERT_TRUE(status);
        bison_column_it_values_info(&type, &num_elems, cit);
        ASSERT_EQ(type, BISON_FIELD_TYPE_NUMBER_U16);
        ASSERT_EQ(num_elems, 0);

        char *json_4 = strdup(bison_to_json(&sb, &rev_doc));

        bison_revise_iterator_close(&rev_it);
        bison_revise_end(&revise);

        // -------------------------------------------------------------------------------------------------------------


        printf(">> %s\n", json_1);
        printf(">> %s\n", json_2);
        printf(">> %s\n", json_3);
        printf(">> %s\n", json_4);

        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 2, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[1, 3]]}") == 0);
        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[3]]}") == 0);
        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [[]]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        free(json_1);
        free(json_2);
        free(json_3);
        free(json_4);
}

TEST(BisonTest, BisonRemoveComplexTest)
{
        struct bison doc, rev_doc, rev_doc2, rev_doc3, rev_doc4, rev_doc5, rev_doc6, rev_doc7, rev_doc8, rev_doc9,
                rev_doc10, rev_doc11, rev_doc12, rev_doc13, rev_doc14;
        struct bison_new context;
        struct bison_revise revise;
        struct bison_array_it rev_it;
        struct string_builder sb;
        bool has_next;
        string_builder_create(&sb);

        struct bison_insert_array_state state, state2, state3;
        struct bison_insert_column_state cstate;
        struct bison_insert *array_ins, *array_ins2, *array_ins3, *column_ins;

        struct bison_path_evaluator eval;
        enum bison_path_status path_status;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_OPTIMIZE);

        bison_insert_u8(ins, 1);
        bison_insert_string(ins, "Hello");
        bison_insert_u16(ins, 2);
        bison_insert_u32(ins, 3);
        bison_insert_u64(ins, 3);
        bison_insert_string(ins, "World");

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        bison_insert_u8(array_ins, 4);
        bison_insert_string(array_ins, "Fox!");
        bison_insert_u8(array_ins, 6);
        bison_insert_array_end(&state);

        array_ins = bison_insert_array_begin(&state, ins, 10);
        array_ins2 = bison_insert_array_begin(&state2, array_ins, 10);
        bison_insert_array_end(&state2);
        array_ins2 = bison_insert_array_begin(&state2, array_ins, 10);
        bison_insert_u8(array_ins2, 4);
        bison_insert_array_end(&state2);
        bison_insert_null(array_ins);
        array_ins2 = bison_insert_array_begin(&state2, array_ins, 10);
        bison_insert_string(array_ins2, "Dog!");
        array_ins3 = bison_insert_array_begin(&state3, array_ins2, 10);
        bison_insert_array_end(&state3);
        array_ins3 = bison_insert_array_begin(&state3, array_ins2, 10);
        column_ins = bison_insert_column_begin(&cstate, array_ins3, BISON_FIELD_TYPE_NUMBER_U8, 10);
        bison_insert_u8(column_ins, 41);
        bison_insert_u8(column_ins, 42);
        bison_insert_u8(column_ins, 43);
        bison_insert_column_end(&cstate);
        bison_insert_array_end(&state3);
        array_ins3 = bison_insert_array_begin(&state3, array_ins2, 10);
        bison_insert_array_end(&state3);

        bison_insert_array_end(&state2);
        bison_insert_array_end(&state);

        bison_create_end(&context);
        // -------------------------------------------------------------------------------------------------------------

        char *json_1 = strdup(bison_to_json(&sb, &doc));
        ASSERT_TRUE(strcmp(json_1, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [1, \"Hello\", 2, 3, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("3", &rev_doc, &doc);
        char *json_2 = strdup(bison_to_json(&sb, &rev_doc));
        ASSERT_TRUE(strcmp(json_2, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [], [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("5", &rev_doc2, &rev_doc);
        char *json_3 = strdup(bison_to_json(&sb, &rev_doc2));
        ASSERT_TRUE(strcmp(json_3, "{\"meta\": {\"_id\": 0, \"_rev\": 3}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, \"Fox!\", 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("5.1", &rev_doc3, &rev_doc2);
        char *json_4 = strdup(bison_to_json(&sb, &rev_doc3));
        ASSERT_TRUE(strcmp(json_4, "{\"meta\": {\"_id\": 0, \"_rev\": 4}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[], [4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("6.0", &rev_doc4, &rev_doc3);
        char *json_5 = strdup(bison_to_json(&sb, &rev_doc4));
        ASSERT_TRUE(strcmp(json_5, "{\"meta\": {\"_id\": 0, \"_rev\": 5}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [4, 6], [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("5", &rev_doc5, &rev_doc4);
        char *json_6 = strdup(bison_to_json(&sb, &rev_doc5));
        ASSERT_TRUE(strcmp(json_6, "{\"meta\": {\"_id\": 0, \"_rev\": 6}, \"doc\": [1, \"Hello\", 2, 3, \"World\", [[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("0", &rev_doc6, &rev_doc5);
        bison_revise_remove_one("1", &rev_doc7, &rev_doc6);
        bison_revise_remove_one("0", &rev_doc8, &rev_doc7);
        bison_revise_remove_one("1", &rev_doc9, &rev_doc8);
        bison_revise_remove_one("0", &rev_doc10, &rev_doc9);
        char *json_11 = strdup(bison_to_json(&sb, &rev_doc10));
        ASSERT_TRUE(strcmp(json_11, "{\"meta\": {\"_id\": 0, \"_rev\": 11}, \"doc\": [[[4], null, [\"Dog!\", [], [[41, 42, 43]], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("0.2.2.0", &rev_doc11, &rev_doc10);

        char *json_12 = strdup(bison_to_json(&sb, &rev_doc11));
        ASSERT_TRUE(strcmp(json_12, "{\"meta\": {\"_id\": 0, \"_rev\": 12}, \"doc\": [[[4], null, [\"Dog!\", [], [], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("0.2.2", &rev_doc12, &rev_doc11);

        char *json_13 = strdup(bison_to_json(&sb, &rev_doc12));
        ASSERT_TRUE(strcmp(json_13, "{\"meta\": {\"_id\": 0, \"_rev\": 13}, \"doc\": [[[4], null, [\"Dog!\", [], []]]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("0.2", &rev_doc13, &rev_doc12);

        char *json_14 = strdup(bison_to_json(&sb, &rev_doc13));
        ASSERT_TRUE(strcmp(json_14, "{\"meta\": {\"_id\": 0, \"_rev\": 14}, \"doc\": [[[4], null]]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_remove_one("0", &rev_doc14, &rev_doc13);

        char *json_15 = strdup(bison_to_json(&sb, &rev_doc14));
        ASSERT_TRUE(strcmp(json_15, "{\"meta\": {\"_id\": 0, \"_rev\": 15}, \"doc\": []}") == 0);


        string_builder_drop(&sb);
        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);
        bison_drop(&rev_doc3);
        bison_drop(&rev_doc4);
        bison_drop(&rev_doc5);
        bison_drop(&rev_doc6);
        bison_drop(&rev_doc7);
        bison_drop(&rev_doc8);
        bison_drop(&rev_doc9);
        bison_drop(&rev_doc10);
        bison_drop(&rev_doc11);
        bison_drop(&rev_doc12);
        bison_drop(&rev_doc13);
        bison_drop(&rev_doc14);
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

TEST(BisonTest, BisonUpdateMixedFixedTypesTypeChangeSimple)
{
        struct bison doc, rev_doc, rev_doc2, rev_doc3, rev_doc4;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;
        struct bison_update updater;
        struct string_builder sb;
        const char *json;

        string_builder_create(&sb);
        bison_create_empty(&doc);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_insert_u8(&inserter, 1);
        bison_insert_i64(&inserter, -42);
        bison_insert_float(&inserter, 23);

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc);
        printf("JSON (rev1): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [1, -42, 23.00]}") == 0);

        // -------------------------------------------------------------------------------------------------------------

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_iterator_open(&it, &revise);
        bison_array_it_insert_begin(&inserter, &it);

        bison_update_u32(&revise, "1", 1024);

        bison_array_it_insert_end(&inserter);
        bison_revise_iterator_close(&it);
        bison_revise_end(&revise);


        json = bison_to_json(&sb, &rev_doc2);
        printf("JSON (rev2): %s\n", json);
        ASSERT_TRUE(strcmp(json, "{\"meta\": {\"_id\": 0, \"_rev\": 2}, \"doc\": [1, 1024, 23.00]}") == 0);


        // -------------------------------------------------------------------------------------------------------------

        string_builder_drop(&sb);

        bison_drop(&doc);
        bison_drop(&rev_doc);
        bison_drop(&rev_doc2);

}


TEST(BisonTest, BisonShrinkIssueFix)
{
        struct bison doc;
        struct bison_new context;

        // -------------------------------------------------------------------------------------------------------------
        struct bison_insert *ins = bison_create_begin(&context, &doc, BISON_OPTIMIZE);

        bison_insert_string(ins, "Hello");
        bison_insert_string(ins, "World");

        bison_create_end(&context);

        // -------------------------------------------------------------------------------------------------------------

        struct string_builder sb;
        string_builder_create(&sb);

        ASSERT_TRUE(strcmp(bison_to_json(&sb, &doc), "{\"meta\": {\"_id\": 0, \"_rev\": 1}, \"doc\": [\"Hello\", \"World\"]}") == 0);

        string_builder_drop(&sb);
        bison_drop(&doc);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}