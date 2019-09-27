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

#include <jakson/utils/jak_utils_numbers.h>

jak_number_min_type_e jak_number_min_type_unsigned(jak_u64 value)
{
        if (value <= JAK_CARBON_U8_MAX) {
                return JAK_NUMBER_U8;
        } else if (value <= JAK_CARBON_U16_MAX) {
                return JAK_NUMBER_U16;
        } else if (value <= JAK_CARBON_U32_MAX) {
                return JAK_NUMBER_U32;
        } else if (value <= JAK_CARBON_U64_MAX) {
                return JAK_NUMBER_U64;
        } else {
                return JAK_NUMBER_UNKNOWN;
        }
}

jak_number_min_type_e jak_number_min_type_signed(jak_i64 value)
{
        if (value >= JAK_CARBON_I8_MIN && value <= JAK_CARBON_I8_MAX) {
                return JAK_NUMBER_I8;
        } else if (value >= JAK_CARBON_I16_MIN && value <= JAK_CARBON_I16_MAX) {
                return JAK_NUMBER_I16;
        } else if (value >= JAK_CARBON_I32_MIN && value <= JAK_CARBON_I32_MAX) {
                return JAK_NUMBER_I32;
        } else if (value >= JAK_CARBON_I64_MIN && value <= JAK_CARBON_I64_MAX) {
                return JAK_NUMBER_I64;
        } else {
                return JAK_NUMBER_UNKNOWN;
        }
}

