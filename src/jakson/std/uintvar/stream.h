/**
 * A variable-length unsigned integer type that encodes the number of used bytes by a flag bit in the byte stream
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

#ifndef UINTVAR_STREAM_H
#define UINTVAR_STREAM_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/types.h>
#include <jakson/mem/file.h>

BEGIN_DECL

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
 *
  * Rule of thumb:
 *      - if fixed-length types are a good choice, and...
 *          - ... if speed matters, use fast-types of the C library (e.g., <code>uint_fast32_t</code>)
 *          - ... if space matters, use fix-types of the C library (e.g., <code>uint32_t</code>)
 *      - if variable-length types are a good choice, and...
 *          - ... if space shall be minimized in exchange of read/write performance, use <code>uintvar_stream_t</code>
 *          - ... if read/write performance shall be maximized in exchange of space, use <code>uintvar_marker_t</code>
 */

typedef void *uintvar_stream_t;

#define UINTVAR_STREAM_MAX_BLOCKS()    (4)

u8 uintvar_stream_write(uintvar_stream_t dst, u64 value);

#define UINTVAR_STREAM_SIZEOF(value)                                                                               \
({                                                                                                                     \
        size_t num_blocks_strlen = UINTVAR_STREAM_REQUIRED_BLOCKS(value);                                          \
        num_blocks_strlen = num_blocks_strlen < sizeof(uintvar_stream_t) ? sizeof(uintvar_stream_t):num_blocks_strlen; \
        num_blocks_strlen;                                                                                             \
})

#define UINTVAR_STREAM_REQUIRED_BLOCKS(value)                                                                      \
({                                                                                                                     \
        u8 num_blocks_required;                                                                                    \
        if (value < 128u) {                                                                                            \
                num_blocks_required = 1;                                                                               \
        } else if (value < 16384u) {                                                                                   \
                num_blocks_required = 2;                                                                               \
        } else if (value < 2097152u) {                                                                                 \
                num_blocks_required = 3;                                                                               \
        } else if (value < 268435456u) {                                                                               \
                num_blocks_required = 4;                                                                               \
        } else if (value < 34359738368u) {                                                                             \
                num_blocks_required = 5;                                                                               \
        } else if (value < 4398046511104u) {                                                                           \
                num_blocks_required = 6;                                                                               \
        } else if (value < 562949953421312u) {                                                                         \
                num_blocks_required = 7;                                                                               \
        } else if (value < 72057594037927936u) {                                                                       \
                num_blocks_required = 8;                                                                               \
        } else if (value < 9223372036854775808u) {                                                                     \
                num_blocks_required = 9;                                                                               \
        } else {                                                                                                       \
                num_blocks_required = 10;                                                                              \
        }                                                                                                              \
        num_blocks_required;                                                                                           \
})

u64 uintvar_stream_read(u8 *nbytes, uintvar_stream_t src);

END_DECL

#endif
