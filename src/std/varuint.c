/**
 * A variable-length unsigned integer type (varuint)
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdx/varuint.h"

static  const u64 last_byte = 0 | ((char) ~0u);
static const char byte_selection = (0b01111111);
static const char forward_bit    = (char) (1u << 7);
static const char norforward_bit = 0;//(char) (0u << 7);


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define BYTE_TO_BLOCK_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BLOCK(byte)  \
  'X', \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

static u8 write_chunk2(char *dst, u64 value) {
        char *dst_cpy = dst;
        char b0 = byte_selection & (last_byte & (value >> 0));   /* for value in [0, 128) */
        char b1 = byte_selection & (last_byte & (value >> 7));   /* for value in [128, 16384) */
        char b2 = byte_selection & (last_byte & (value >> 14));  /* for value in [16384, 2097152) */
        char b3 = byte_selection & (last_byte & (value >> 21));  /* for value in [2097152, 268435456) */
        char b4 = byte_selection & (last_byte & (value >> 28));  /* for value in [268435456, 34359738368) */
        char b5 = byte_selection & (last_byte & (value >> 35));  /* for value in [34359738368, 4398046511104) */
        char b6 = byte_selection & (last_byte & (value >> 42));  /* for value in [4398046511104, 562949953421312) */
        char b7 = byte_selection & (last_byte & (value >> 49));  /* for value in [562949953421312, 72057594037927936) */
        char b8 = byte_selection & (last_byte & (value >> 56));  /* for value in [72057594037927936, 9223372036854775808) */
        char b9 = byte_selection & (last_byte & (value >> 63));  /* for value in [9223372036854775808, 18446744073709551616) */

        bool tail = false;
        u8 num_bytes;

        printf("\n\n\nVALUE TO WRITE: %llu ; "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\n", value,
                BYTE_TO_BINARY((value>>32)), BYTE_TO_BINARY((value>>16)), BYTE_TO_BINARY((value>>8)), BYTE_TO_BINARY(value));

        printf("BLOCKS:\n"
                "0: " BYTE_TO_BLOCK_PATTERN "\n"
                "1: " BYTE_TO_BLOCK_PATTERN "\n"
                "2: " BYTE_TO_BLOCK_PATTERN "\n"
                "3: " BYTE_TO_BLOCK_PATTERN "\n"
                "4: " BYTE_TO_BLOCK_PATTERN "\n",
                BYTE_TO_BLOCK(b0), BYTE_TO_BLOCK(b1), BYTE_TO_BLOCK(b2), BYTE_TO_BLOCK(b3), BYTE_TO_BLOCK(b4));

        if (b9) {
                *dst = forward_bit | b9;
                num_bytes = 10;
                tail = true;
                dst++;
                printf("WRITE BLOCK 9 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b9));
        }

        if (b8 || tail) {
                *dst = forward_bit | b8;
                num_bytes = tail ? num_bytes : 9;
                tail = true;
                dst++;
                printf("WRITE BLOCK 8 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b8));
        }

        if (b7 || tail) {
                *dst = forward_bit | b7;
                num_bytes = tail ? num_bytes : 8;
                tail = true;
                dst++;
                printf("WRITE BLOCK 7 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b7));
        }

        if (b6 || tail) {
                *dst = forward_bit | b6;
                num_bytes = tail ? num_bytes : 7;
                tail = true;
                dst++;
                printf("WRITE BLOCK 6 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b6));
        }

        if (b5 || tail) {
                *dst = forward_bit | b5;
                num_bytes = tail ? num_bytes : 6;
                tail = true;
                dst++;
                printf("WRITE BLOCK 5 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b5));
        }

        if (b4 || tail) {
                *dst = forward_bit | b4;
                num_bytes = tail ? num_bytes : 5;
                tail = true;
                dst++;
                printf("WRITE BLOCK 4 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b4));
        }

        if (b3 || tail) {
                *dst = forward_bit | b3;
                num_bytes = tail ? num_bytes : 4;
                tail = true;
                dst++;
                printf("WRITE BLOCK 3 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b3));
        }

        if (b2 || tail) {
                *dst = forward_bit | b2;
                num_bytes = tail ? num_bytes : 3;
                tail = true;
                dst++;
                printf("WRITE BLOCK 2 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b2));
        }

        if (b1 || tail) {
                *dst = forward_bit | b1;
                num_bytes = tail ? num_bytes : 2;
                tail = true;
                dst++;
                printf("WRITE BLOCK 1 [1]"BYTE_TO_BLOCK_PATTERN"\n", BYTE_TO_BLOCK(b1));
        }


        *dst = norforward_bit | b0;
        num_bytes = tail ? num_bytes : 1;
        printf("WRITE BLOCK 0 "BYTE_TO_BLOCK_PATTERN" (byte "BYTE_TO_BINARY_PATTERN")\n", BYTE_TO_BLOCK(b0), BYTE_TO_BINARY(b0));


        b0 = * (char *) (dst_cpy + 0);
        b1 = * (char *) (dst_cpy + 1);
        b2 = * (char *) (dst_cpy + 2);
        b3 = * (char *) (dst_cpy + 3);
        b4 = * (char *) (dst_cpy + 4);
        printf("WRITE BUFFER CONTENT "BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN "\n",
                BYTE_TO_BINARY(b0),BYTE_TO_BINARY(b1),BYTE_TO_BINARY(b2),BYTE_TO_BINARY(b3),BYTE_TO_BINARY(b4));

        return num_bytes;

}


NG5_EXPORT(u8) varuint_write(void *dst, u64 value)
{
        if (likely(dst != NULL)) {
                return write_chunk2(dst, value);
        } else {
                return 0;
        }
}

//static inline void *read_chunk(bool *has_next, u64 *dst, char *src)
//{
//        char chunk = *src;
//        *has_next = (forward_bit & chunk) == forward_bit;
//        *dst = (*dst + (chunk & byte_selection)) << (*has_next ? 7 : 0);
//        return ++src;
//}

NG5_EXPORT(u64) varuint_read(u8 *nbytes, void *src)
{
        u64 value = 0;
        char it;
        bool has_next;
        u8 ndecoded = 0;

        char b0 = * (char *) (src + 0);
        char b1 = * (char *) (src + 1);
        char b2 = * (char *) (src + 2);
        char b3 = * (char *) (src + 3);
        char b4 = * (char *) (src + 4);

        printf("READ BUFFER CONTENT  "BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN " "BYTE_TO_BINARY_PATTERN "\n",
                BYTE_TO_BINARY(b0),BYTE_TO_BINARY(b1),BYTE_TO_BINARY(b2),BYTE_TO_BINARY(b3),BYTE_TO_BINARY(b4));

        do {
                it = * (char *) src;
                has_next = (it & forward_bit) == forward_bit;
                printf("READ BLOCK " BYTE_TO_BLOCK_PATTERN " (byte "BYTE_TO_BINARY_PATTERN"), hasNext? %d\n", BYTE_TO_BLOCK(it), BYTE_TO_BINARY(it), has_next);
                char block_val = it & byte_selection;
                value = (value << 7) | block_val;
                src++;
                ndecoded++;
        } while (has_next);

        printf("\n\n");

        ng5_optional_set(nbytes, ndecoded);
        return value;
}
