#include <gtest/gtest.h>
#include <printf.h>

#include "core/bison/bison.h"
#include "std/string_builder.h"

TEST(BisonTest, CreateBison) {
        struct bison doc;
        object_id_t oid;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create(&doc);
        EXPECT_TRUE(status);

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
        struct bison doc;
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
        bison_revise_begin(&revise, &doc);
        bison_revise_end(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 1);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
}

TEST(BisonTest, CreateBisonRevisionAbort) {
        struct bison doc;
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
        bison_revise_begin(&revise, &doc);
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
        struct bison doc;
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
        bison_revise_begin(&revise, &doc);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_revise_end(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 1);

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

static void print_on_new_revision(struct bison_event_listener *self, struct bison *doc)
{
        ng5_unused(self);
        ng5_unused(doc);
        printf("revision complete\n");
}

TEST(BisonTest, CreateBisonRevisionListening) {
        struct bison doc;
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
        bison_revise_begin(&revise, &doc);
        bison_revise_abort(&revise);

        status = bison_revision(&rev, &doc);
        EXPECT_TRUE(status);
        EXPECT_EQ(rev, 0);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}