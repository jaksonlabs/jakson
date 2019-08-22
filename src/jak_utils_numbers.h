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

#ifndef AKR_NUMBERS_H
#define AKR_NUMBERS_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_types.h>

typedef enum jak_number_min_type_e {
        JAK_NUMBER_U8,
        JAK_NUMBER_U16,
        JAK_NUMBER_U32,
        JAK_NUMBER_U64,
        JAK_NUMBER_I8,
        JAK_NUMBER_I16,
        JAK_NUMBER_I32,
        JAK_NUMBER_I64,
        JAK_NUMBER_UNKNOWN
} jak_number_min_type_e;

jak_number_min_type_e jak_number_min_type_unsigned(jak_u64 value);
jak_number_min_type_e jak_number_min_type_signed(jak_i64 value);

#endif
