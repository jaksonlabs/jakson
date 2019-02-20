#include <gtest/gtest.h>
#include <stdio.h>
#include <inttypes.h>
#include <carbon/carbon-archive-iter.h>

#include "carbon/carbon.h"

static void
iterate_properties(carbon_archive_prop_iter_t *prop_iter);

static void
iterate_int8_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_int8_t value;
    bool status = carbon_archive_value_is_int8(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_int8(&value, value_iter);
    printf("\t\t{type: int8, value: %" PRIi8 " }\n", value);
}

static void
iterate_int16_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_int16_t value;
    bool status = carbon_archive_value_is_int16(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_int16(&value, value_iter);
    printf("\t\t{type: int16, value: %" PRIi16 " }\n", value);
}

static void
iterate_int32_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_int32_t value;
    bool status = carbon_archive_value_is_int32(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_int32(&value, value_iter);
    printf("\t\t{type: int32, value: %" PRIi32 " }\n", value);
}

static void
iterate_int64_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_int64_t value;
    bool status = carbon_archive_value_is_int64(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_int64(&value, value_iter);
    printf("\t\t{type: int64, value: %" PRIi64 " }\n", value);
}

static void
iterate_uint8_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_uint8_t value;
    bool status = carbon_archive_value_is_uint8(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_uint8(&value, value_iter);
    printf("\t\t{type: uint8, value: %" PRIu8 " }\n", value);
}

static void
iterate_uint16_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_uint16_t value;
    bool status = carbon_archive_value_is_uint16(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_uint16(&value, value_iter);
    printf("\t\t{type: uint16, value: %" PRIu16 " }\n", value);
}

static void
iterate_uint32_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_uint32_t value;
    bool status = carbon_archive_value_is_uint32(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_uint32(&value, value_iter);
    printf("\t\t{type: uint32, value: %" PRIu32 " }\n", value);
}

static void
iterate_uint64_vals(carbon_archive_value_t *value_iter)
{
    bool type_match;
    carbon_uint64_t value;
    bool status = carbon_archive_value_is_uint64(&type_match, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(type_match);
    carbon_archive_value_get_uint64(&value, value_iter);
    printf("\t\t{type: uint64, value: %" PRIu64 " }\n", value);
}

static void
iterate_float_vals(carbon_archive_value_t *value_iter)
{
    bool is_float;
    carbon_float_t value;
    bool status = carbon_archive_value_is_number(&is_float, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_float);
    carbon_archive_value_get_number(&value, value_iter);
    printf("\t\t{type: number, value: %f }\n", value);
}

static void
iterate_string_vals(carbon_archive_value_t *value_iter)
{
    bool is_string;
    carbon_string_id_t value;
    bool status = carbon_archive_value_is_string(&is_string, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_string);
    carbon_archive_value_get_string(&value, value_iter);
    printf("\t\t{type: string, value: %" PRIu64 " }\n", value);
}

static void
iterate_boolean_vals(carbon_archive_value_t *value_iter)
{
    bool is_bool;
    carbon_bool_t value;
    bool status = carbon_archive_value_is_boolean(&is_bool, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_bool);
    carbon_archive_value_get_boolean(&value, value_iter);
    printf("\t\t{type: boolean, value: %d }\n", value);
}

static void
iterate_null_vals(carbon_archive_value_t *value_iter)
{
    bool is_null;
    bool status = carbon_archive_value_is_null(&is_null, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_null);
    printf("\t\t{type: null }\n");
}

static void
iterate_object_vals(carbon_archive_value_t *value_iter)
{
    bool status;
    bool is_object;
    carbon_archive_object_t object;
    carbon_archive_prop_iter_t  prop_iter;

    status = carbon_archive_value_is_object(&is_object, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_object);

    status = carbon_archive_value_get_object(&object, value_iter);
    ASSERT_TRUE(status);
    printf("\t\t{type: object, id: %" PRIu64 "}\n", object.object_id);

    status = carbon_archive_prop_iter_from_object(&prop_iter, CARBON_ARCHIVE_ITER_MASK_ANY, &object, value_iter);
    ASSERT_TRUE(status);

    iterate_properties(&prop_iter);
}





static void
iterate_int8_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int16_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int32_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int64_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint8_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint16_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint32_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint64_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_float_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_string_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_boolean_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_null_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_object_array_vals(carbon_archive_value_t *value_iter)
{
    FAIL() << "Not implemented";
}




static void
iterate_properties(carbon_archive_prop_iter_t *prop_iter)
{
    carbon_object_id_t          oid;
    carbon_string_id_t          string_id;
    carbon_archive_value_t value_iter;
    carbon_basic_type_e         type;
    bool                        is_array;

    carbon_archive_prop_iter_get_object_id(&oid, prop_iter);
    while (carbon_archive_prop_iter_next(&string_id, &value_iter, prop_iter))
    {
        carbon_archive_value_get_basic_type(&type, &value_iter);
        carbon_archive_value_is_array_type(&is_array, &value_iter);

        printf("object id: %" PRIu64 ", key: %" PRIu64 "\n", oid, string_id);
        printf("\ttype : %d, is-array: %d\n", type, is_array);

        if (is_array)
        {
            switch (type) {
            case CARBON_BASIC_TYPE_INT8:
                iterate_int8_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT16:
                iterate_int16_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT32:
                iterate_int32_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT64:
                iterate_int64_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT8:
                iterate_uint8_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT16:
                iterate_uint16_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT32:
                iterate_uint32_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT64:
                iterate_uint64_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_NUMBER:
                iterate_float_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_STRING:
                iterate_string_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_BOOLEAN:
                iterate_boolean_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_NULL:
                iterate_null_array_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_OBJECT:
                iterate_object_array_vals(&value_iter);
                break;
            default:
                FAIL() << "unknown type";
            }
        } else
        {
            switch (type) {
            case CARBON_BASIC_TYPE_INT8:
                iterate_int8_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT16:
                iterate_int16_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT32:
                iterate_int32_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_INT64:
                iterate_int64_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT8:
                iterate_uint8_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT16:
                iterate_uint16_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT32:
                iterate_uint32_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_UINT64:
                iterate_uint64_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_NUMBER:
                iterate_float_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_STRING:
                iterate_string_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_BOOLEAN:
                iterate_boolean_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_NULL:
                iterate_null_vals(&value_iter);
                break;
            case CARBON_BASIC_TYPE_OBJECT:
                iterate_object_vals(&value_iter);
                break;
            default:
                FAIL() << "unknown type";
            }
        }

    }
}

TEST(ArchiveIterTest, CreateIterator)
{
    carbon_archive_t            archive;
    carbon_err_t                err;
    carbon_archive_prop_iter_t  prop_iter;
    bool                        status;

    /* in order to access this file, the working directory of this test executable must be set to the project root */
    status = carbon_archive_open(&archive, "tests/assets/test-archive.carbon");
    ASSERT_TRUE(status);

    status = carbon_archive_prop_iter_from_archive(&prop_iter, &err, CARBON_ARCHIVE_ITER_MASK_ANY, &archive);
    ASSERT_TRUE(status);

    iterate_properties(&prop_iter);

    carbon_archive_close(&archive);
    ASSERT_TRUE(status);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}