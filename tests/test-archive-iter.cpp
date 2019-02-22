#include <gtest/gtest.h>
#include <stdio.h>
#include <inttypes.h>
#include <carbon/carbon-archive-iter.h>

#include "carbon/carbon.h"

static void
iterate_properties(carbon_archive_prop_iter_t *prop_iter);

static void
iterate_int8_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_int8_t value;
//    bool status = carbon_archive_value_is_int8(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_int8(&value, value_iter);
//    printf("\t\t{type: int8, value: %" PRIi8 " }\n", value);
}

static void
iterate_int16_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_int16_t value;
//    bool status = carbon_archive_value_is_int16(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_int16(&value, value_iter);
//    printf("\t\t{type: int16, value: %" PRIi16 " }\n", value);
}

static void
iterate_int32_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_int32_t value;
//    bool status = carbon_archive_value_is_int32(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_int32(&value, value_iter);
//    printf("\t\t{type: int32, value: %" PRIi32 " }\n", value);
}

static void
iterate_int64_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_int64_t value;
//    bool status = carbon_archive_value_is_int64(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_int64(&value, value_iter);
//    printf("\t\t{type: int64, value: %" PRIi64 " }\n", value);
}

static void
iterate_uint8_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_uint8_t value;
//    bool status = carbon_archive_value_is_uint8(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_uint8(&value, value_iter);
//    printf("\t\t{type: uint8, value: %" PRIu8 " }\n", value);
}

static void
iterate_uint16_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_uint16_t value;
//    bool status = carbon_archive_value_is_uint16(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_uint16(&value, value_iter);
//    printf("\t\t{type: uint16, value: %" PRIu16 " }\n", value);
}

static void
iterate_uint32_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_uint32_t value;
//    bool status = carbon_archive_value_is_uint32(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_uint32(&value, value_iter);
//    printf("\t\t{type: uint32, value: %" PRIu32 " }\n", value);
}

static void
iterate_uint64_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool type_match;
//    carbon_uint64_t value;
//    bool status = carbon_archive_value_is_uint64(&type_match, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(type_match);
//    carbon_archive_value_get_uint64(&value, value_iter);
//    printf("\t\t{type: uint64, value: %" PRIu64 " }\n", value);
}

static void
iterate_float_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool is_float;
//    carbon_number_t value;
//    bool status = carbon_archive_value_is_number(&is_float, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(is_float);
//    carbon_archive_value_get_number(&value, value_iter);
//    printf("\t\t{type: number, value: %f }\n", value);
}

static void
iterate_string_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool is_string;
//    carbon_string_id_t value;
//    bool status = carbon_archive_value_is_string(&is_string, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(is_string);
//    carbon_archive_value_get_string(&value, value_iter);
//    printf("\t\t{type: string, value: %" PRIu64 " }\n", value);
}

static void
iterate_boolean_vals(carbon_archive_value_vector_t *value_iter)
{
//    bool is_bool;
//    carbon_boolean_t value;
//    bool status = carbon_archive_value_is_boolean(&is_bool, value_iter);
//    ASSERT_TRUE(status);
//    ASSERT_TRUE(is_bool);
//    carbon_archive_value_get_boolean(&value, value_iter);
//    printf("\t\t{type: boolean, value: %d }\n", value);
}

static void
iterate_null_vals(carbon_archive_value_vector_t *value_iter)
{
    bool is_null;
    bool status = carbon_archive_value_vector_is_null(&is_null, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_null);
    printf("\t\t{type: null }\n");
}

static void
iterate_object_vals(carbon_archive_value_vector_t *value_iter)
{
    bool status;
    bool is_object;
    uint32_t vector_length;
    carbon_archive_object_t object;
    carbon_archive_prop_iter_t  prop_iter;

    status = carbon_archive_value_vector_is_of_objects(&is_object, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_object);

    status = carbon_archive_value_vector_get_length(&vector_length, value_iter);
    ASSERT_TRUE(status);

    for (uint32_t i = 0; i < vector_length; i++)
    {
        status = carbon_archive_value_vector_get_object_at(&object, i, value_iter);
        ASSERT_TRUE(status);
        printf("\t\t{type: object, id: %" PRIu64 "}\n", object.object_id);

        status = carbon_archive_prop_iter_from_object(&prop_iter, CARBON_ARCHIVE_ITER_MASK_ANY, &object, value_iter);
        ASSERT_TRUE(status);

        iterate_properties(&prop_iter);
    }
}




static void
iterate_int8_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int16_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int32_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_int64_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint8_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint16_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint32_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_uint64_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_float_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_string_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_boolean_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_null_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}

static void
iterate_object_array_vals(carbon_archive_value_vector_t *value_iter)
{
    FAIL() << "Not implemented";
}


static void
iterate_object(carbon_archive_value_vector_t *value_iter)
{
    if (value_iter->is_array) {
        iterate_object_array_vals(value_iter);
    } else {
        iterate_object_vals(value_iter);
    }
}

static void
print_basic_fixed_types_basic(carbon_archive_value_vector_t *value_iter, uint32_t idx)
{
    uint32_t num_values;
    switch (value_iter->prop_type) {
    case CARBON_BASIC_TYPE_INT8: {
        const carbon_int8_t *values = carbon_archive_value_vector_get_int8s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int8, value: %" PRIi8 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT16: {
        const carbon_int16_t *values = carbon_archive_value_vector_get_int16s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int16, value: %" PRIi16 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT32: {
        const carbon_int32_t *values = carbon_archive_value_vector_get_int32s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int32, value: %" PRIi32 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT64: {
        const carbon_int64_t *values = carbon_archive_value_vector_get_int64s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int64, value: %" PRIi64 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT8: {
        const carbon_uint8_t *values = carbon_archive_value_vector_get_uint8s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint8, value: %" PRIu8 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT16: {
        const carbon_uint16_t *values = carbon_archive_value_vector_get_uint16s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint16, value: %" PRIu16 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT32: {
        const carbon_uint32_t *values = carbon_archive_value_vector_get_uint32s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint32, value: %" PRIu32 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT64: {
        const carbon_uint64_t *values = carbon_archive_value_vector_get_uint64s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint64, value: %" PRIu64 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_NUMBER: {
        const carbon_number_t *values = carbon_archive_value_vector_get_numbers(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: number, value: %f }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_STRING: {
            const carbon_string_id_t *values = carbon_archive_value_vector_get_strings(&num_values, value_iter);
            ASSERT_TRUE(values != NULL);
            ASSERT_TRUE(idx < num_values);
            printf("\t\t{ type: string, value: %" PRIu64 " }\n", values[idx]);
        } break;
    case CARBON_BASIC_TYPE_BOOLEAN: {
        const carbon_boolean_t *values = carbon_archive_value_vector_get_booleans(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: boolean, value: %d }\n", values[idx]);
    } break;
    default:
        FAIL() << "Unsupported basic type";
    }
}

static void
print_basic_fixed_types_array(carbon_archive_value_vector_t *value_iter, uint32_t idx)
{
    uint32_t array_length;
    switch (value_iter->prop_type) {
    case CARBON_BASIC_TYPE_NULL: {
        const carbon_uint32_t *number_contained = carbon_archive_value_vector_get_null_arrays(&array_length, value_iter);
        ASSERT_TRUE(number_contained != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: null_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("null%s", i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT8: {
        const carbon_int8_t *values = carbon_archive_value_vector_get_int8_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int8_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%d%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT16: {
        const carbon_int16_t *values = carbon_archive_value_vector_get_int16_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int16_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIi16 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT32: {
        const carbon_int32_t *values = carbon_archive_value_vector_get_int32_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int32_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIi32 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT64: {
        const carbon_int64_t *values = carbon_archive_value_vector_get_int64_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int64_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIi64 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT8: {
        const carbon_uint8_t *values = carbon_archive_value_vector_get_uint8_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint8_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu8 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT16: {
        const carbon_uint16_t *values = carbon_archive_value_vector_get_uint16_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint16_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu16 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT32: {
        const carbon_uint32_t *values = carbon_archive_value_vector_get_uint32_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint32_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu32 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT64: {
        const carbon_uint64_t *values = carbon_archive_value_vector_get_uint64_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint64_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu64 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_NUMBER: {
        const carbon_number_t *values = carbon_archive_value_vector_get_number_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: numbers_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%f%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_STRING: {
        const carbon_string_id_t *values = carbon_archive_value_vector_get_string_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: string_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu64 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_BOOLEAN: {
        const carbon_boolean_t *values = carbon_archive_value_vector_get_boolean_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: boolean_array, values: [");
        for (uint32_t i = 0; i < array_length; i++)
        {
            printf("%" PRIu8 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    default:
        FAIL() << "Unsupported basic type";
    }

}

static void
print_basic_fixed_types(carbon_archive_value_vector_t *value_iter, uint32_t idx)
{
    if (value_iter->is_array) {
        print_basic_fixed_types_array(value_iter, idx);
    } else {
        print_basic_fixed_types_basic(value_iter, idx);
    }
}




static void
iterate_properties(carbon_archive_prop_iter_t *prop_iter)
{
    carbon_object_id_t          oid;
    carbon_archive_value_vector_t value_iter;
    carbon_basic_type_e         type;
    bool                        is_array;
    const carbon_string_id_t   *keys;
    uint32_t                    num_pairs;

    carbon_archive_prop_iter_document_get_object_id(&oid, prop_iter);
    while (keys = carbon_archive_prop_iter_document_get_value_vector(&num_pairs, &type, &is_array, &value_iter, prop_iter))
    {
        for (uint32_t i = 0; i < num_pairs; i++) {
            printf("Key %" PRIu64 ", type: %d, is-array: %d\n", keys[i], type, is_array);

            switch (type) {
            case CARBON_BASIC_TYPE_OBJECT:
                iterate_object(&value_iter);
                break;
            case CARBON_BASIC_TYPE_NULL:
                printf("\t\t{ type: null }\n");
                break;
            case CARBON_BASIC_TYPE_INT8:
            case CARBON_BASIC_TYPE_INT16:
            case CARBON_BASIC_TYPE_INT32:
            case CARBON_BASIC_TYPE_INT64:
            case CARBON_BASIC_TYPE_UINT8:
            case CARBON_BASIC_TYPE_UINT16:
            case CARBON_BASIC_TYPE_UINT32:
            case CARBON_BASIC_TYPE_UINT64:
            case CARBON_BASIC_TYPE_NUMBER:
            case CARBON_BASIC_TYPE_STRING:
            case CARBON_BASIC_TYPE_BOOLEAN:
                print_basic_fixed_types(&value_iter, i);
                break;
            default:
                FAIL() << "unknown basic type";

            }
        }

//        carbon_archive_value_get_basic_type(&type, &value_iter);
//        carbon_archive_value_is_array_type(&is_array, &value_iter);
//
//        printf("object id: %" PRIu64 ", key: %" PRIu64 "\n", oid, string_id);
//        printf("\ttype : %d, is-array: %d\n", type, is_array);
//
//        if (is_array)
//        {
//            switch (type) {
//            case CARBON_BASIC_TYPE_INT8:
//                iterate_int8_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT16:
//                iterate_int16_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT32:
//                iterate_int32_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT64:
//                iterate_int64_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT8:
//                iterate_uint8_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT16:
//                iterate_uint16_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT32:
//                iterate_uint32_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT64:
//                iterate_uint64_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_NUMBER:
//                iterate_float_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_STRING:
//                iterate_string_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_BOOLEAN:
//                iterate_boolean_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_NULL:
//                iterate_null_array_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_OBJECT:
//                iterate_object_array_vals(&value_iter);
//                break;
//            default:
//                FAIL() << "unknown type";
//            }
//        } else
//        {
//            switch (type) {
//            case CARBON_BASIC_TYPE_INT8:
//                iterate_int8_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT16:
//                iterate_int16_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT32:
//                iterate_int32_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_INT64:
//                iterate_int64_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT8:
//                iterate_uint8_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT16:
//                iterate_uint16_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT32:
//                iterate_uint32_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_UINT64:
//                iterate_uint64_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_NUMBER:
//                iterate_float_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_STRING:
//                iterate_string_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_BOOLEAN:
//                iterate_boolean_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_NULL:
//                iterate_null_vals(&value_iter);
//                break;
//            case CARBON_BASIC_TYPE_OBJECT:
//                iterate_object_vals(&value_iter);
//                break;
//            default:
//                FAIL() << "unknown type";
//            }
//        }

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