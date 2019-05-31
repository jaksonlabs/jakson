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

static const char byte_selection = (0b01111111);
static const char forward_bit    = (char) (1u << 7);
static const char norforward_bit = (char) (0u << 7);

static char *write_forward_chunk(char *dst, u64 value, unsigned short byte_num) {
        *dst = (char) (forward_bit | (byte_selection & (value >> (byte_num * 7))));
        return ++dst;
}

static char *write_nonforward_chunk(char *dst, u64 value) {
        *dst = (char) (norforward_bit | (byte_selection & value));
        return ++dst;
}


NG5_EXPORT(u8) varuint_write(void *dst, u64 value)
{
        if (likely(dst != NULL)) {
                if (likely(value < 128)) {
                        write_nonforward_chunk(dst, value);
                        return 1;
                } else if (likely(value < 16384)) {
                        dst = write_forward_chunk(dst, value, 1);
                        write_nonforward_chunk(dst, value);
                        return 2;
                } else if (unlikely(value < 536870912)) {
                        dst = write_forward_chunk(dst, value, 2);
                        dst = write_forward_chunk(dst, value, 1);
                        write_nonforward_chunk(dst, value);
                        return 3;
                } else {
                        return 0;
                }
        } else {
                return 0;
        }
}

static inline void *read_chunk(bool *has_next, u64 *dst, char *src)
{
        char chunk = *src;
        *has_next = (forward_bit & chunk) == forward_bit;
        *dst = (*dst + (chunk & byte_selection)) << (*has_next ? 7 : 0);
        return ++src;
}

NG5_EXPORT(u64) varuint_read(u8 *nbytes, void *src)
{
        u64 value = 0;
        bool has_next = true;

        for (*nbytes = 0; has_next; src = read_chunk(&has_next, &value, src), *nbytes = ++(*nbytes))
                { }

        return value;
}
