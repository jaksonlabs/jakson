#include <gtest/gtest.h>
#include <printf.h>

#include "stdx/varuint.h"

TEST(VarUintTest, ReadWrite1Byte) {
        u64 value = 42;
        char dst[5];
        u8 nbytes = varuint_write(&dst, value);
        ASSERT_TRUE(nbytes == 1);
        u64 read_value = varuint_read(&nbytes, &dst);
        ASSERT_TRUE(nbytes == 1);
        ASSERT_TRUE(read_value == value);
}

TEST(VarUintTest, ReadWrite2Bytes) {
        u64 value = 256;
        char dst[5];
        u8 nbytes = varuint_write(&dst, value);
        ASSERT_TRUE(nbytes == 2);
        u64 read_value = varuint_read(&nbytes, &dst);
        ASSERT_TRUE(nbytes == 2);
        ASSERT_TRUE(read_value == value);
}

TEST(VarUintTest, ReadWrite3Bytes) {
        u64 value = 20000;
        char dst[5];
        u8 nbytes = varuint_write(&dst, value);
        ASSERT_TRUE(nbytes == 3);
        u64 read_value = varuint_read(&nbytes, &dst);
        ASSERT_TRUE(nbytes == 3);
        ASSERT_TRUE(read_value == value);
}

TEST(VarUintTest, ReadWrite4Bytes) {
        u64 value = 536870920;
        char dst[5];
        u8 nbytes = varuint_write(&dst, value);
        ASSERT_TRUE(nbytes == 4);
        u64 read_value = varuint_read(&nbytes, &dst);
        ASSERT_TRUE(nbytes == 4);
        ASSERT_TRUE(read_value == value);
}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}