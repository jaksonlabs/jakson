#include <gtest/gtest.h>

#include "core/bison/bison.h"
#include "core/bison/bison-array-it.h"
#include "core/bison/bison-insert.h"

TEST(BisonTest, CreateBison) {
        struct bison doc;
        object_id_t oid;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create(&doc);
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

        status = bison_create(&doc);
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
}

TEST(BisonTest, CreateBisonRevisionAbort) {
        struct bison doc, rev_doc;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create(&doc);
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

        status = bison_create(&doc);
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

        status = bison_create(&doc);

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

        bison_create(&doc);

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

        bison_create(&doc);

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
}

TEST(BisonTest, BisonArrayIteratorOpenAfterNew) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;

        bison_create(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_gen_object_id(NULL, &revise);
        bison_revise_access(&it, &revise);
        bool has_next = bison_array_it_next(&it);
        EXPECT_EQ(has_next, false);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
}

TEST(BisonTest, BisonArrayIteratorInsertNullAfterNew) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_access(&it, &revise);
        bison_revise_gen_object_id(NULL, &revise);
        bison_array_it_insert(&inserter, &it);
        bison_insert_null(&inserter);
        bison_insert_drop(&inserter);
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
}

TEST(BisonTest, BisonArrayIteratorInsertMultipleLiteralsAfterNewNoOverflow) {
        struct bison doc, rev_doc;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_access(&it, &revise);
        bison_array_it_insert(&inserter, &it);
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
        bison_revise_end(&revise);

        bison_print(stdout, &rev_doc);
        bison_hexdump_print(stdout, &rev_doc);

        bison_drop(&doc);
}

TEST(BisonTest, BisonArrayIteratorOverwriteLiterals) {
        struct bison doc, rev_doc, rev_doc2;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create(&doc);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_access(&it, &revise);
        bison_array_it_insert(&inserter, &it);
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
        bison_revise_end(&revise);

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_access(&it, &revise);
        bison_array_it_insert(&inserter, &it);
        for (i32 i = 0; i < 2; i++) {
                fprintf(stdout, "before:\n");
                bison_hexdump_print(stdout, &rev_doc2);
                bison_insert_true(&inserter);
                fprintf(stdout, "after:\n");
                bison_hexdump_print(stdout, &rev_doc2);
        }
        bison_insert_drop(&inserter);
        bison_revise_end(&revise);





        bison_print(stdout, &rev_doc2);

        bison_drop(&doc);
}

TEST(BisonTest, BisonArrayIteratorOverwriteLiteralsWithDocOverflow) {
        struct bison doc, rev_doc, rev_doc2;
        struct bison_revise revise;
        struct bison_array_it it;
        struct bison_insert inserter;

        bison_create_ex(&doc, 20, 1);

        bison_revise_begin(&revise, &rev_doc, &doc);
        bison_revise_access(&it, &revise);
        bison_array_it_insert(&inserter, &it);
        for (i32 i = 0; i < 22; i++) {
                if (i % 3 == 0) {
                        bison_insert_null(&inserter);
                } else if (i % 3 == 1) {
                        bison_insert_true(&inserter);
                } else {
                        bison_insert_false(&inserter);
                }
                fprintf(stdout, "after initial push:\n");
                bison_hexdump_print(stdout, &rev_doc);
        }
        bison_insert_drop(&inserter);
        bison_revise_end(&revise);

        bison_revise_begin(&revise, &rev_doc2, &rev_doc);
        bison_revise_access(&it, &revise);
        bison_array_it_insert(&inserter, &it);
        for (i32 i = 0; i < 2; i++) {
                fprintf(stdout, "before:\n");
                bison_hexdump_print(stdout, &rev_doc2);
                bison_insert_true(&inserter);
                fprintf(stdout, "after:\n");
                bison_hexdump_print(stdout, &rev_doc2);
        }
        bison_insert_drop(&inserter);
        bison_revise_end(&revise);





        bison_print(stdout, &rev_doc2);

        bison_drop(&doc);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}