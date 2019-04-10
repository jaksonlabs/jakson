#include <gtest/gtest.h>
#include <stdio.h>
#include <inttypes.h>
#include "archive/archive_iter.h"

#include "carbon.h"

static void
iterate_properties(carbon_archive_prop_iter_t *prop_iter);

static void
iterate_object_vals(carbon_archive_value_vector_t *value_iter)
{
    bool status;
    bool is_object;
    u32 vector_length;
    carbon_archive_object_t object;
    carbon_archive_prop_iter_t  prop_iter;
    struct err err;

    status = carbon_archive_value_vector_is_of_objects(&is_object, value_iter);
    ASSERT_TRUE(status);
    ASSERT_TRUE(is_object);

    status = carbon_archive_value_vector_get_length(&vector_length, value_iter);
    ASSERT_TRUE(status);

    for (u32 i = 0; i < vector_length; i++)
    {
        status = carbon_archive_value_vector_get_object_at(&object, i, value_iter);
        ASSERT_TRUE(status);
        printf("\t\t{type: object, id: %" PRIu64 "}\n", object.object_id);


        status = carbon_archive_prop_iter_from_object(&prop_iter, CARBON_ARCHIVE_ITER_MASK_ANY, &err, &object);
        ASSERT_TRUE(status);

        iterate_properties(&prop_iter);
    }
}

static void
iterate_object(carbon_archive_value_vector_t *value_iter)
{
    ASSERT_TRUE (!value_iter->is_array);
    iterate_object_vals(value_iter);
}

static void
print_basic_fixed_types_basic(carbon_archive_value_vector_t *value_iter, u32 idx)
{
    u32 num_values;
    switch (value_iter->prop_type) {
    case CARBON_BASIC_TYPE_INT8: {
        const carbon_i8 *values = carbon_archive_value_vector_get_int8s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int8, value: %" PRIi8 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT16: {
        const carbon_i16 *values = carbon_archive_value_vector_get_int16s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int16, value: %" PRIi16 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT32: {
        const carbon_i32 *values = carbon_archive_value_vector_get_int32s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int32, value: %" PRIi32 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_INT64: {
        const carbon_i64 *values = carbon_archive_value_vector_get_int64s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: int64, value: %" PRIi64 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT8: {
        const carbon_u8 *values = carbon_archive_value_vector_get_uint8s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint8, value: %" PRIu8 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT16: {
        const carbon_u16 *values = carbon_archive_value_vector_get_uint16s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint16, value: %" PRIu16 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT32: {
        const carbon_u32 *values = carbon_archive_value_vector_get_uint32s(&num_values, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(idx < num_values);
        printf("\t\t{ type: uint32, value: %" PRIu32 " }\n", values[idx]);
    } break;
    case CARBON_BASIC_TYPE_UINT64: {
        const carbon_u64 *values = carbon_archive_value_vector_get_uint64s(&num_values, value_iter);
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
print_basic_fixed_types_array(carbon_archive_value_vector_t *value_iter, u32 idx)
{
    u32 array_length;
    switch (value_iter->prop_type) {
    case CARBON_BASIC_TYPE_NULL: {
        const carbon_u32 *number_contained = carbon_archive_value_vector_get_null_arrays(&array_length, value_iter);
        ASSERT_TRUE(number_contained != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: null_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("null%s", i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT8: {
        const carbon_i8 *values = carbon_archive_value_vector_get_int8_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int8_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%d%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT16: {
        const carbon_i16 *values = carbon_archive_value_vector_get_int16_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int16_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIi16 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT32: {
        const carbon_i32 *values = carbon_archive_value_vector_get_int32_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int32_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIi32 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_INT64: {
        const carbon_i64 *values = carbon_archive_value_vector_get_int64_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: int64_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIi64 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT8: {
        const carbon_u8 *values = carbon_archive_value_vector_get_uint8_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint8_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIu8 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT16: {
        const carbon_u16 *values = carbon_archive_value_vector_get_uint16_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint16_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIu16 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT32: {
        const carbon_u32 *values = carbon_archive_value_vector_get_uint32_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint32_array, values: [");
        for (u32 i = 0; i < array_length; i++)
        {
            printf("%" PRIu32 "%s", values[i], i + 1 < array_length ? ", " : "");
        }
        printf("]\n");
    } break;
    case CARBON_BASIC_TYPE_UINT64: {
        const carbon_u64 *values = carbon_archive_value_vector_get_uint64_arrays_at(&array_length, idx, value_iter);
        ASSERT_TRUE(values != NULL);
        ASSERT_TRUE(array_length != 0);
        printf("\t\t{ type: uint64_array, values: [");
        for (u32 i = 0; i < array_length; i++)
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
        for (u32 i = 0; i < array_length; i++)
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
        for (u32 i = 0; i < array_length; i++)
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
        for (u32 i = 0; i < array_length; i++)
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
print_basic_fixed_types(carbon_archive_value_vector_t *value_iter, u32 idx)
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
    carbon_object_id_t                oid;
    carbon_archive_value_vector_t     value_iter;
    carbon_basic_type_e               type;
    bool                              is_array;
    const carbon_string_id_t         *keys;
    u32                          num_pairs;
    carbon_archive_prop_iter_mode_e   iter_type;
    carbon_archive_collection_iter_t  collection_iter;
    u32                          num_column_groups;
    carbon_archive_column_group_iter_t group_iter;
    carbon_archive_column_iter_t       column_iter;
    carbon_archive_column_entry_iter_t entry_iter;
    struct err                       err;

    while (carbon_archive_prop_iter_next(&iter_type, &value_iter, &collection_iter, prop_iter))
    {
        if (iter_type == CARBON_ARCHIVE_PROP_ITER_MODE_OBJECT)
        {
            keys = carbon_archive_value_vector_get_keys(&num_pairs, &value_iter);
            carbon_archive_value_vector_is_array_type(&is_array, &value_iter);
            carbon_archive_value_vector_get_basic_type(&type, &value_iter);
            carbon_archive_value_vector_get_object_id(&oid, &value_iter);
            for (u32 i = 0; i < num_pairs; i++) {
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
        } else {
            keys = carbon_archive_collection_iter_get_keys(&num_column_groups, &collection_iter);
            ASSERT_TRUE(keys != NULL);
            printf("\t\t{ column groups for keys:");
            for (u32 i = 0; i < num_column_groups; i++) {
                printf("%" PRIu64 " ", keys[i]);
            }
            printf("}\n");
            while (carbon_archive_collection_next_column_group(&group_iter, &collection_iter)) {

                u32 num_objs;
                const carbon_object_id_t *ids = carbon_archive_column_group_get_object_ids(&num_objs, &group_iter);

                printf("\t\t{ column groups object ids:");
                for (u32 i = 0; i < num_objs; i++) {
                    printf("%" PRIu64 " ", ids[i]);
                }
                printf("}\n");

                while(carbon_archive_column_group_next_column(&column_iter, &group_iter)) {
                    carbon_string_id_t column_name;
                    carbon_basic_type_e column_entry_type;
                    u32 num_entries;
                    carbon_archive_column_get_name(&column_name, &column_entry_type, &column_iter);
                    const u32 *positions = carbon_archive_column_get_entry_positions(&num_entries, &column_iter);
                    printf("\t\t{ column-name: %" PRIu64 ", type: %d }\n", column_name, column_entry_type);
                    printf("\t\t{ entry positions:");
                    for (u32 i = 0; i < num_entries; i++) {
                        printf("%" PRIu32 " ", positions[i]);
                    }
                    printf("}\n");

                    while(carbon_archive_column_next_entry(&entry_iter, &column_iter)) {

                        carbon_basic_type_e entry_type;
                        u32 entry_length;
                        carbon_archive_column_entry_get_type(&entry_type, &entry_iter);

                        switch (entry_type) {
                        case CARBON_BASIC_TYPE_STRING: {
                            const carbon_string_id_t *values = carbon_archive_column_entry_get_strings(&entry_length, &entry_iter);
                            printf("\t\t{ strings: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("%" PRIu64 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_INT8: {
                            const carbon_i8 *values = carbon_archive_column_entry_get_int8s(&entry_length, &entry_iter);
                            printf("\t\t{ int8s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("% " PRIi8 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_INT16: {
                            const carbon_i16 *values = carbon_archive_column_entry_get_int16s(&entry_length, &entry_iter);
                            printf("\t\t{ int16s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("% " PRIi16 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_INT32: {
                            const carbon_i32 *values = carbon_archive_column_entry_get_int32s(&entry_length, &entry_iter);
                            printf("\t\t{ int32s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("% " PRIi32 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_INT64: {
                            const carbon_i64 *values = carbon_archive_column_entry_get_int64s(&entry_length, &entry_iter);
                            printf("\t\t{ int64s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("% " PRIi64 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_UINT8: {
                            const carbon_u8 *values = carbon_archive_column_entry_get_uint8s(&entry_length, &entry_iter);
                            printf("\t\t{ uint8s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("%" PRIu8 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_UINT16: {
                            const carbon_u16 *values = carbon_archive_column_entry_get_uint16s(&entry_length, &entry_iter);
                            printf("\t\t{ uint16s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("%" PRIu16 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_UINT32: {
                            const carbon_u32 *values = carbon_archive_column_entry_get_uint32s(&entry_length, &entry_iter);
                            printf("\t\t{ uint32s: [");
                            for (u32 i = 0; i < entry_length; i++) {
                                printf("%" PRIu32 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_UINT64: {
                            const carbon_u64 *values = carbon_archive_column_entry_get_uint64s(&entry_length, &entry_iter);
                            printf("\t\t{ uint64s: [");
                            for (u64 i = 0; i < entry_length; i++) {
                                printf("%" PRIu64 "%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_NUMBER: {
                            const carbon_number_t *values = carbon_archive_column_entry_get_numbers(&entry_length, &entry_iter);
                            printf("\t\t{ numbers: [");
                            for (u64 i = 0; i < entry_length; i++) {
                                printf("%f%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_BOOLEAN: {
                            const carbon_boolean_t *values = carbon_archive_column_entry_get_booleans(&entry_length, &entry_iter);
                            printf("\t\t{ booleans: [");
                            for (u64 i = 0; i < entry_length; i++) {
                                printf("%d%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_NULL: {
                            const carbon_u32 *values = carbon_archive_column_entry_get_nulls(&entry_length, &entry_iter);
                            printf("\t\t{ nulls: [");
                            for (u64 i = 0; i < entry_length; i++) {
                                printf("%d%s", values[i], i + 1 < entry_length ? ", " : "");
                            }
                            printf("]\n");
                        } break;
                        case CARBON_BASIC_TYPE_OBJECT: {
                            carbon_archive_column_entry_object_iter_t iter;
                            const carbon_archive_object_t *archive_object;
                            carbon_archive_column_entry_get_objects(&iter, &entry_iter);
                            printf("\t\t{ << objects >>: [");
                            while ((archive_object = carbon_archive_column_entry_object_iter_next_object(&iter)) != NULL) {
                                carbon_object_id_t id;
                                carbon_archive_object_get_object_id(&id, archive_object);
                                printf("{ oid: %" PRIu64 " } \n", id);

                                carbon_archive_prop_iter_t nested_obj_prop_iter;
                                carbon_archive_prop_iter_from_object(&nested_obj_prop_iter, CARBON_ARCHIVE_ITER_MASK_ANY,
                                                                     &err, archive_object);
                                iterate_properties(&nested_obj_prop_iter);
                            }
                            printf("]\n");
                        } break;
                        default:
                            FAIL() << "Unknown type";
                        }

                    }
                }

            }
        }
    }
}

TEST(ArchiveIterTest, CreateIterator)
{
    carbon_archive_t            archive;
    struct err                err;
    carbon_archive_prop_iter_t  prop_iter;
    bool                        status;

    /* in order to access this file, the working directory of this test executable must be set to a sub directory
     * below the projects root directory (e.g., 'build/') */
    status = carbon_archive_open(&archive, "../tests/assets/test-archive.carbon");
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