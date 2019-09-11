#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

TEST(UIntvarColumn, CreateTest) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, 10);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [c][0][0][0][0][0][0][0][0][0][0]") == 0);

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 10);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 10);
                ASSERT_EQ(payload_start, 3);
                ASSERT_EQ(header_size, 3);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, 10);
                ASSERT_EQ(column_end_off, 13);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, 13);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_8);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                jak_u64 nelems;
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, 10);
                for (unsigned i = 0; i < 10; i++) {
                        ASSERT_EQ(result[i], 0);
                }
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == 13);
        }

        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, UpdateNoChangeTest) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, 10);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [c][0][0][0][0][0][0][0][0][0][0]") == 0);
        jak_string_clear(&str);

        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [c][1][2][3][4][5][6][7][8][9][10]") == 0);

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 10);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 10);
                ASSERT_EQ(payload_start, 3);
                ASSERT_EQ(header_size, 3);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, 10);
                ASSERT_EQ(column_end_off, 13);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, 13);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_8);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                jak_u64 nelems;
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, 10);
                for (unsigned i = 0; i < 10; i++) {
                        ASSERT_EQ(result[i], i + 1);
                }
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == 13);
        }

        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, CreateWithNumElemsLarger255) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, 256);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[d][256] [c][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0]") == 0);
        jak_string_clear(&str);

        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[d][256] [c][1][2][3][4][5][6][7][8][9][10][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0]") == 0);

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 256);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, 256);
                ASSERT_EQ(payload_start, 4);
                ASSERT_EQ(header_size, 4);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, 256);
                ASSERT_EQ(column_end_off, 260);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, 260);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_16);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                jak_u64 nelems;
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, 256);
                for (unsigned i = 0; i < 256; i++) {
                        ASSERT_EQ(result[i], i < 10 ? i + 1 : 0);
                }
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == 260);
        }

        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, CreateWithNumElemsLargerU16) {

        jak_memblock *block;
        jak_memfile memfile;

        const unsigned NUM_ELEMS = UINT16_MAX + 1;

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, NUM_ELEMS);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);


        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, NUM_ELEMS);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, NUM_ELEMS);
                ASSERT_EQ(payload_start, 6);
                ASSERT_EQ(header_size, 6);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, NUM_ELEMS);
                ASSERT_EQ(column_end_off, NUM_ELEMS + 6);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, NUM_ELEMS + 6);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_32);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                jak_u64 nelems;
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, NUM_ELEMS);
                for (unsigned i = 0; i < NUM_ELEMS; i++) {
                        ASSERT_EQ(result[i], i < 10 ? i + 1 : 0);
                }
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == NUM_ELEMS + 6);
        }

        jak_memblock_drop(block);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, CreateWithNumElemsLargerU32) {

        jak_memblock *block;
        jak_memfile memfile;

        const jak_u64 NUM_ELEMS = (jak_u64) (UINT32_MAX) + 1;

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, NUM_ELEMS);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);


        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, NUM_ELEMS);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                ASSERT_EQ(num_elems, NUM_ELEMS);
                ASSERT_EQ(payload_start, 10);
                ASSERT_EQ(header_size, 10);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, NUM_ELEMS);
                ASSERT_EQ(column_end_off, NUM_ELEMS + 10);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, NUM_ELEMS + 10);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_64);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_8);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                jak_u64 nelems;
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, NUM_ELEMS);
                for (jak_u64 i = 0; i < NUM_ELEMS; i++) {
                        ASSERT_EQ(result[i], i < 10 ? i + 1 : 0);
                }
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == NUM_ELEMS + 10);
        }

        jak_memblock_drop(block);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}


TEST(UIntvarColumn, UpdateNoChangeTestU16Elements) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create_wtype(&memfile, JAK_UINTVAR_16, 10);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [d][0][0][0][0][0][0][0][0][0][0]") == 0);
        jak_string_clear(&str);

        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, UINT8_MAX + i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [d][256][257][258][259][260][261][262][263][264][265]") == 0);

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_16);
                ASSERT_EQ(num_elems, 10);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_16);
                ASSERT_EQ(num_elems, 10);
                ASSERT_EQ(payload_start, 3);
                ASSERT_EQ(header_size, 3);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, 20);
                ASSERT_EQ(column_end_off, 23);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, 23);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_8);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_16);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                jak_u64 nelems;
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, 10);
                for (unsigned i = 0; i < 10; i++) {
                        ASSERT_EQ(result[i], UINT8_MAX + i + 1);
                }
        }

        {
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == 23);
        }

        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, UpdateNoChangeTestU32Elements) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create_wtype(&memfile, JAK_UINTVAR_32, 10);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [i][0][0][0][0][0][0][0][0][0][0]") == 0);
        jak_string_clear(&str);

        {
                for (unsigned i = 0; i < 10; i++) {
                        status = jak_memfile_uintvar_column_set(&memfile, i, UINT16_MAX + i + 1);
                        ASSERT_TRUE(memfile.pos == 0);
                }

                ASSERT_TRUE(memfile.pos == 0);
        }

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [i][65536][65537][65538][65539][65540][65541][65542][65543][65544][65545]") == 0);

        {
                jak_uintvar_marker_marker_type_e type;
                jak_u64 num_elems;
                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(type, JAK_UINTVAR_32);
                ASSERT_EQ(num_elems, 10);
                ASSERT_TRUE(result != NULL);
        }

        {
                jak_u64 num_elems;
                jak_offset_t payload_start;
                jak_uintvar_marker_marker_type_e value_type;
                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_32);
                ASSERT_EQ(num_elems, 10);
                ASSERT_EQ(payload_start, 3);
                ASSERT_EQ(header_size, 3);
        }

        {
                jak_offset_t column_end_off;
                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(column_size, 40);
                ASSERT_EQ(column_end_off, 43);
        }

        {
                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(total_size, 43);
        }

        {
                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(counter_type, JAK_UINTVAR_8);
        }

        {
                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(value_type, JAK_UINTVAR_32);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, true);
        }

        {
                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(result, false);
        }

        {
                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                jak_u64 nelems;
                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(&nelems, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_EQ(nelems, 10);
                for (unsigned i = 0; i < 10; i++) {
                        ASSERT_EQ(result[i], UINT16_MAX + i + 1);
                }
        }

        {
                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                ASSERT_TRUE(memfile.pos == 0);
                ASSERT_TRUE(result == NULL);
                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
        }

        {
                status = jak_memfile_uintvar_column_skip(&memfile);
                ASSERT_TRUE(status);
                ASSERT_TRUE(memfile.pos == 43);
        }

        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

TEST(UIntvarColumn, UpdateNoChangeTestRewrite) {

        jak_memblock *block;
        jak_memfile memfile;
        jak_string str;

        jak_string_create(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_BEGIN

        jak_memblock_create(&block, 1024);
        jak_memfile_open(&memfile, block, JAK_READ_WRITE);
        bool status = jak_memfile_uintvar_column_create(&memfile, 10);
        ASSERT_TRUE(status);
        ASSERT_TRUE(memfile.pos == 0);

        jak_memfile_uintvar_column_to_str(&str, &memfile);
        printf("'%s'\n", jak_string_cstr(&str));
        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [c][0][0][0][0][0][0][0][0][0][0]") == 0);
        jak_string_clear(&str);

        jak_u64 lower_bounds[] = {0, 128, 256, 1024, UINT16_MAX, UINT32_MAX};
        jak_u64 lower_bounds_idx_max = 4;

        for (jak_u64 k = 0; k <= lower_bounds_idx_max; k ++)
        {
                jak_u64 lower = lower_bounds[k];

                {
                        for (unsigned i = 0; i < 10; i++) {
                                status = jak_memfile_uintvar_column_set(&memfile, i, lower + i + 1);
                                ASSERT_TRUE(memfile.pos == 0);
                        }

                        ASSERT_TRUE(memfile.pos == 0);
                }

                jak_memfile_uintvar_column_to_str(&str, &memfile);
                printf("'%s'\n", jak_string_cstr(&str));
                if (k == 0) {
                        ASSERT_TRUE(strcmp(jak_string_cstr(&str), "[c][10] [c][1][2][3][4][5][6][7][8][9][10]") == 0);

                        {
                                jak_uintvar_marker_marker_type_e type;
                                jak_u64 num_elems;
                                void *result = jak_memfile_uintvar_column_raw_payload(&type, &num_elems, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(type, JAK_UINTVAR_8);
                                ASSERT_EQ(num_elems, 10);
                                ASSERT_TRUE(result != NULL);
                        }

                        {
                                jak_u64 num_elems;
                                jak_offset_t payload_start;
                                jak_uintvar_marker_marker_type_e value_type;
                                jak_u64 header_size = jak_memfile_uintvar_column_size_header(&value_type, &num_elems, &payload_start, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                                ASSERT_EQ(num_elems, 10);
                                ASSERT_EQ(payload_start, 3);
                                ASSERT_EQ(header_size, 3);
                        }

                        {
                                jak_offset_t column_end_off;
                                jak_u64 column_size = jak_memfile_uintvar_column_size_payload(&column_end_off, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(column_size, 10);
                                ASSERT_EQ(column_end_off, 13);
                        }

                        {
                                jak_u64 total_size = jak_memfile_uintvar_column_size_total(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(total_size, 13);
                        }

                        {
                                jak_uintvar_marker_marker_type_e counter_type = jak_memfile_uintvar_column_get_counter_type(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(counter_type, JAK_UINTVAR_8);
                        }

                        {
                                jak_uintvar_marker_marker_type_e value_type = jak_memfile_uintvar_column_get_value_type(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(value_type, JAK_UINTVAR_8);
                        }

                        {
                                bool result = jak_memfile_uintvar_column_is_u8(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(result, true);
                        }

                        {
                                bool result = jak_memfile_uintvar_column_is_u16(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(result, false);
                        }

                        {
                                bool result = jak_memfile_uintvar_column_is_u32(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(result, false);
                        }

                        {
                                bool result = jak_memfile_uintvar_column_is_u64(&memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(result, false);
                        }

                        {
                                jak_u64 nelems;
                                const jak_u8 *result = jak_memfile_uintvar_column_read_u8(&nelems, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_EQ(nelems, 10);
                                for (unsigned i = 0; i < 10; i++) {
                                        ASSERT_EQ(result[i], lower + i + 1);
                                }
                        }

                        {
                                const jak_u16 *result = jak_memfile_uintvar_column_read_u16(NULL, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_TRUE(result == NULL);
                                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
                        }

                        {
                                const jak_u32 *result = jak_memfile_uintvar_column_read_u32(NULL, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_TRUE(result == NULL);
                                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
                        }

                        {
                                const jak_u64 *result = jak_memfile_uintvar_column_read_u64(NULL, &memfile);

                                ASSERT_TRUE(memfile.pos == 0);
                                ASSERT_TRUE(result == NULL);
                                ASSERT_TRUE(JAK_ERROR_OCCURED(&memfile));
                        }

                        {
                                jak_memfile_save_position(&memfile);
                                status = jak_memfile_uintvar_column_skip(&memfile);
                                ASSERT_TRUE(status);
                                ASSERT_TRUE(memfile.pos == 13);
                                jak_memfile_restore_position(&memfile);
                        }
                }

        }



        jak_memblock_drop(block);
        jak_string_drop(&str);

        JAK_EXPLICT_SIMULATE_ERROR_HANDLING_IN_RELEASE_END
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}