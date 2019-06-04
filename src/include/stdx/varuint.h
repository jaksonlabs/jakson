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

#ifndef NG5_VARUINT_H
#define NG5_VARUINT_H

#include "shared/common.h"
#include "shared/types.h"

NG5_BEGIN_DECL

/**
 * This type is for variable-length unsigned integer types.
 *
 * The encoding uses the most significant bit (MSB) for each byte in sequence of bytes (called blocks) to determine the
 * number of bytes required to express the unsigned integer value. The MSB is 1 if there is at least one further byte to
 * be read, and 0 if the end of the sequence is reached. The remaining 7 bits per block contain the actual bits for
 * integer value encoding.
 *
 * This implementation supports variable-length encoding of the maximum value up to unsigned integer of 64bit (fixed-
 * length) in at most 10 blocks.
 *
 * Example: Given the unsigned integer 16389, its fixed-length representation is
 *                  01000000 00000101
 *          Using the varuint type, the representation is
 *                  (1)0000001 (1)0000000 (0)0000101
 *
 *
 *      # required |      min value      |      max value
 *        blocks   |       (incl.)       |       (incl.)
 *      -----------+---------------------+----------------------
 *               1 |                   0 |                  127
 *               2 |                 128 |                16383
 *               3 |               16384 |              2097151
 *               4 |             2097152 |            268435455
 *               5 |           268435456 |          34359738367
 *               6 |         34359738368 |        4398046511103
 *               7 |       4398046511104 |      562949953421311
 *               8 |     562949953421312 |    72057594037927935
 *               9 |   72057594037927936 |  9223372036854775807
 *              10 | 9223372036854775808 | 18446744073709551615
 */

typedef void *varuint_t;

#define varuint_max_blocks()    (4)

NG5_EXPORT(u8) varuint_write(varuint_t dst, u64 value);

NG5_EXPORT(u64) varuint_read(u8 *nbytes, varuint_t src);

NG5_END_DECL

#endif
