#include <gtest/gtest.h>
#include <printf.h>

#include <jakson/jakson.h>

#define DEFINE_UINTVAR_STREAM_POINT_TEST(test_name, value, expected_bytes)     \
TEST(VarUintTest, ReadWrite##test_name) {                               \
        char dst[10];                                                   \
        u8 nbytes = uintvar_stream_write(&dst, value);                         \
        ASSERT_TRUE(nbytes == expected_bytes);                          \
        u64 read_value = uintvar_stream_read(&nbytes, &dst);                   \
        ASSERT_TRUE(nbytes == expected_bytes);                          \
        ASSERT_TRUE(read_value == value);                               \
}

DEFINE_UINTVAR_STREAM_POINT_TEST(1ByteMin, 0u, 1)
DEFINE_UINTVAR_STREAM_POINT_TEST(1ByteMax, 127u, 1)

DEFINE_UINTVAR_STREAM_POINT_TEST(2ByteMin, 128u, 2)
DEFINE_UINTVAR_STREAM_POINT_TEST(2ByteMax, 16383u, 2)

DEFINE_UINTVAR_STREAM_POINT_TEST(3ByteMin, 16384u, 3)
DEFINE_UINTVAR_STREAM_POINT_TEST(3ByteMax, 2097151u, 3)

DEFINE_UINTVAR_STREAM_POINT_TEST(4ByteMin, 2097152u, 4)
DEFINE_UINTVAR_STREAM_POINT_TEST(4ByteMax, 268435455u, 4)

DEFINE_UINTVAR_STREAM_POINT_TEST(5ByteMin, 268435456u, 5)
DEFINE_UINTVAR_STREAM_POINT_TEST(5ByteMax, 34359738367u, 5)

DEFINE_UINTVAR_STREAM_POINT_TEST(6ByteMin, 34359738368u, 6)
DEFINE_UINTVAR_STREAM_POINT_TEST(6ByteMax, 4398046511103u, 6)

DEFINE_UINTVAR_STREAM_POINT_TEST(7ByteMin, 4398046511104u, 7)
DEFINE_UINTVAR_STREAM_POINT_TEST(7ByteMax, 562949953421311u, 7)

DEFINE_UINTVAR_STREAM_POINT_TEST(8ByteMin, 562949953421312u, 8)
DEFINE_UINTVAR_STREAM_POINT_TEST(8ByteMax, 72057594037927935u, 8)

DEFINE_UINTVAR_STREAM_POINT_TEST(9ByteMin, 72057594037927936u, 9)
DEFINE_UINTVAR_STREAM_POINT_TEST(9ByteMax, 9223372036854775807u, 9)

DEFINE_UINTVAR_STREAM_POINT_TEST(10ByteMin, 9223372036854775808u, 10)
DEFINE_UINTVAR_STREAM_POINT_TEST(10ByteMax, 18446744073709551615u, 10)

TEST(VarUintTest, ReadWriteRandValuesEncoding) {
        char dst[10];
        for (unsigned i = 0; i < 10000; i++) {
                u64 in_value = rand();
                in_value = (in_value << 32) | rand();
                uintvar_stream_write(&dst, in_value);
                u64 out_value = uintvar_stream_read(NULL, &dst);
                ASSERT_EQ(in_value, out_value);
        }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}