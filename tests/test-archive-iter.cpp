#include <gtest/gtest.h>
#include <stdio.h>
#include <inttypes.h>

#include "carbon/carbon.h"

TEST(ArchiveIterTest, CreateIterator)
{
    carbon_archive_t            archive;
    carbon_err_t                err;
    carbon_archive_prop_iter_t  prop_iter;
    carbon_string_id_t          string_id;
    carbon_archive_value_iter_t value_iter;
    carbon_type_e               type;
    bool                        is_array;
    carbon_object_id_t          oid;
    bool                        status;

    /* in order to access this file, the working directory of this test executable must be set to the project root */
    status = carbon_archive_open(&archive, "tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_prop_iter_from_archive(&prop_iter, &err, CARBON_ARCHIVE_ITER_MASK_ANY, &archive);
    ASSERT_TRUE(status);

    carbon_archive_prop_iter_get_object_id(&oid, &prop_iter);
    while (carbon_archive_prop_iter_next(&string_id, &value_iter, &prop_iter)) {
        carbon_archive_value_iter_get_basic_type(&type, &value_iter);
        carbon_archive_value_iter_is_array_type(&is_array, &value_iter);
        printf("object id: %" PRIu64 ", key: %" PRIu64 "\n", oid, string_id);
        printf("\ttype : %d, is-array: %d\n", type, is_array);

    }


    carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}