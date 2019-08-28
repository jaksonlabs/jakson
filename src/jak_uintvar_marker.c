/**
 * A variable-length unsigned integer type that encodes the number of used bytes by a preceding marker byte
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

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_uintvar_marker.h>
#include <jak_error.h>
#include <jakson/jakson.h>
#include "jak_uintvar_marker.h"

// ---------------------------------------------------------------------------------------------------------------------
//  read/write
// ---------------------------------------------------------------------------------------------------------------------

bool jak_uintvar_marker_write(jak_uintvar_marker_t dst, jak_u64 value)
{
        JAK_ERROR_IF_NULL(dst);
        jak_uintvar_marker_e marker_type = jak_uintvar_marker_type_for(value);
        assert(marker_type < jak_global_uintvar_marker_nreg);
        jak_marker_t marker = jak_global_uintvar_marker_reg[marker_type].marker;
        size_t size =  jak_global_uintvar_marker_reg[marker_type].size;
        memcpy(dst, &marker, sizeof(jak_marker_t));
        memcpy(dst + sizeof(jak_marker_t), &value, size);
        return true;
}

jak_u64 jak_uintvar_marker_read(jak_u8 *nbytes_read, jak_uintvar_marker_t src)
{
        JAK_ERROR_IF_NULL(src);
        jak_uintvar_marker_e marker = jak_uintvar_marker_type(src);
        JAK_OPTIONAL_SET(nbytes_read, jak_global_uintvar_marker_reg[marker].size + sizeof(jak_marker_t));
        src += sizeof(jak_marker_t);
        switch (marker) {
                case JAK_UINTVAR_8:  return *(jak_u8 *) src;
                case JAK_UINTVAR_16: return *(jak_u16 *) src;
                case JAK_UINTVAR_32: return *(jak_u32 *) src;
                case JAK_UINTVAR_64: return *(jak_u64 *) src;
                default:
                        JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                        return 0;
        }
}

// ---------------------------------------------------------------------------------------------------------------------
//  information
// ---------------------------------------------------------------------------------------------------------------------

jak_uintvar_marker_e jak_uintvar_marker_type_for(jak_u64 value)
{
        if (value <= UINT8_MAX) {
                return JAK_UINTVAR_8;
        } else if (value < UINT16_MAX) {
                return JAK_UINTVAR_16;
        } else if (value < UINT32_MAX) {
                return JAK_UINTVAR_32;
        } else {
                return JAK_UINTVAR_64;
        }
}

jak_uintvar_marker_e jak_uintvar_marker_type(jak_uintvar_marker_t data)
{
        jak_u8 marker = *(jak_u8 *)data;
        bool is_u8 = marker == UINT_VAR_MARKER_8;
        bool is_u16 = marker == UINT_VAR_MARKER_16;
        bool is_u32 = marker == UINT_VAR_MARKER_32;
        bool is_u64 = marker == UINT_VAR_MARKER_64;
        JAK_ERROR_PRINT_IF(!(is_u8 || is_u16 || is_u32 || is_u64), JAK_ERR_MARKERMAPPING);
        assert(!is_u8 || !(is_u16 || is_u32 || is_u64));
        assert(!is_u16 || !(is_u8 || is_u32 || is_u64));
        assert(!is_u32 || !(is_u8 || is_u16 || is_u64));
        assert(!is_u64 || !(is_u8 || is_u16 || is_u32));
        if (is_u8) {
                return JAK_UINTVAR_8;
        } else if (is_u16) {
                return JAK_UINTVAR_16;
        } else if (is_u32) {
                return JAK_UINTVAR_32;
        } else {
                return JAK_UINTVAR_64;
        }
}

jak_u8 jak_uintvar_marker_sizeof(jak_uintvar_marker_t varuint)
{
        jak_u8 ret = 0;
        jak_uintvar_marker_read(&ret, varuint);
        return ret + sizeof(jak_marker_t);
}

jak_u8 jak_uintvar_marker_required_size(jak_u64 value)
{
        jak_uintvar_marker_e marker = jak_uintvar_marker_type_for(value);
        JAK_ERROR_PRINT_IF(marker >= jak_global_uintvar_marker_nreg, JAK_ERR_MARKERMAPPING)
        return jak_global_uintvar_marker_reg[marker].size + sizeof(jak_marker_t);
}

char jak_uintvar_marker_type_str(jak_uintvar_marker_e marker)
{
        JAK_ERROR_PRINT_IF(marker >= jak_global_uintvar_marker_nreg, JAK_ERR_MARKERMAPPING)
        return (char) jak_global_uintvar_marker_reg[marker].marker;
}