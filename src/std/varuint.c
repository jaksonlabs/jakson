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

NG5_EXPORT(u8) varuint_write(void *dst, u64 value)
{
        u8 nbytes = 0;
        char datum = 0;

        if (likely(dst != NULL)) {
                if (value < 128) {
                        nbytes = 1;
                        datum = (char) value;
                        *(char *) dst = datum;
                } else if (value < 16384) {
                        nbytes = 2;
                        datum = forward_bit | (byte_selection & (value >> 7));
                        printf("!!!! Value write %d\n", datum);
                        *(char *) dst = datum;
                        dst++;
                        datum = norforward_bit | (byte_selection & value);
                        printf("!!!! Value write %d\n", datum);
                        *(char *) dst = datum;
                        dst--;
                        printf("!!!! >>> NUMBER written %llu\n", value);
                }
        }
        return nbytes;
}

NG5_EXPORT(u64) varuint_read(u8 *nbytes, const void *src)
{
        char chunk = *(char *) src;
        u64 value = 0;
        if ((forward_bit & chunk) == norforward_bit) {
                *nbytes = 1;
                value = chunk & byte_selection;
        } else {
                *nbytes = 2;
                value = chunk & byte_selection;
                printf("!!!! Value read %d\n", chunk);
                value <<= 7;
                src++;
                chunk = *(char *) src;
                printf("!!!! Value read %d\n", chunk);
                value |= chunk & byte_selection;
                printf("!!!! >>> NUMBER read %llu\n", value);
        }
        return value;
}
