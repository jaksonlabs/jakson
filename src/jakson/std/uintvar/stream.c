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

#include <jakson/std/uintvar/stream.h>

#define MASK_LAST_BYTE                  (0 | ((char) ~0u))
#define MASK_FORWARD_BIT                ((char) (1u << 7))
#define MASK_BLOCK_DATA                 (~MASK_FORWARD_BIT)

#define extract_data(x, byte_shift)     (MASK_BLOCK_DATA & (MASK_LAST_BYTE & (x >> byte_shift)))

u8 uintvar_stream_write(uintvar_stream_t dst, u64 value)
{
        if (LIKELY(dst != NULL)) {
                u8 num_bytes = 0;
                for (i8 i = 9; i > 0; i--) {
                        char block_data = extract_data(value, i * 7);
                        if (block_data || num_bytes) {
                                *(char *) dst = MASK_FORWARD_BIT | block_data;
                                num_bytes = num_bytes ? num_bytes : i + 1;
                                dst++;
                        }
                }
                *(char *) dst = extract_data(value, 0);
                return num_bytes ? num_bytes : 1;
        } else {
                return 0;
        }
}

u64 uintvar_stream_read(u8 *nbytes, uintvar_stream_t src)
{
        u64 value = 0;
        bool has_next = true;
        u8 ndecoded = 0;

        for (char it = *(char *) src, block_val = 0; has_next; has_next = (it & MASK_FORWARD_BIT) == MASK_FORWARD_BIT,
                block_val = it & MASK_BLOCK_DATA, value = (value << 7) | block_val, src++,
                ndecoded++, it = *(char *) src) {}


        OPTIONAL_SET(nbytes, ndecoded);
        return value;
}