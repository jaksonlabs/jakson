/**
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

#include <jakson/utils/numbers.h>

number_min_type_e number_min_type_unsigned(u64 value)
{
        if (value <= CARBON_U8_MAX) {
                return NUMBER_U8;
        } else if (value <= CARBON_U16_MAX) {
                return NUMBER_U16;
        } else if (value <= CARBON_U32_MAX) {
                return NUMBER_U32;
        } else if (value <= CARBON_U64_MAX) {
                return NUMBER_U64;
        } else {
                return NUMBER_UNKNOWN;
        }
}

number_min_type_e number_min_type_signed(i64 value)
{
        if (value >= CARBON_I8_MIN && value <= CARBON_I8_MAX) {
                return NUMBER_I8;
        } else if (value >= CARBON_I16_MIN && value <= CARBON_I16_MAX) {
                return NUMBER_I16;
        } else if (value >= CARBON_I32_MIN && value <= CARBON_I32_MAX) {
                return NUMBER_I32;
        } else if (value >= CARBON_I64_MIN && value <= CARBON_I64_MAX) {
                return NUMBER_I64;
        } else {
                return NUMBER_UNKNOWN;
        }
}

