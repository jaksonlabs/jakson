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
        jak_uintvar_marker_marker_type_e marker;
        jak_u8 marker_size = jak_uintvar_marker_write_marker_for(&marker, dst, value);
        size_t value_size =  jak_global_uintvar_marker_reg[marker].size;
        memcpy(dst + marker_size, &value, value_size);
        return true;
}

jak_u8 jak_uintvar_marker_write_marker_for(jak_uintvar_marker_marker_type_e *marker_out, void *dst, jak_u64 value)
{
        JAK_ERROR_IF_NULL(dst);
        jak_uintvar_marker_marker_type_e marker_type = jak_uintvar_marker_type_for(value);
        assert(marker_type < jak_global_uintvar_marker_nreg);
        jak_marker_t marker = jak_global_uintvar_marker_reg[marker_type].marker;
        memcpy(dst, &marker, sizeof(jak_marker_t));
        JAK_OPTIONAL_SET(marker_out, marker_type);
        return sizeof(jak_marker_t);
}

void jak_uintvar_marker_write_marker(void *dst, jak_uintvar_marker_marker_type_e type)
{
        jak_marker_t marker = jak_global_uintvar_marker_reg[type].marker;
        memcpy(dst, &marker, sizeof(jak_marker_t));
}

jak_u8 jak_uintvar_marker_write_value_only(void *dst, jak_uintvar_marker_marker_type_e type, jak_u64 value)
{
        JAK_ERROR_IF_NULL(dst);
        jak_u8 value_size = jak_uintvar_marker_bytes_needed_for_value_by_type(type);
        memcpy(dst, &value, value_size);
        return value_size;
}

jak_u64 jak_uintvar_marker_read(jak_u8 *nbytes_read, jak_uintvar_marker_t src)
{
        JAK_ERROR_IF_NULL(src);
        jak_uintvar_marker_marker_type_e marker = jak_uintvar_marker_peek_marker(src);
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

jak_uintvar_marker_marker_type_e jak_uintvar_marker_type_for(jak_u64 value)
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

jak_uintvar_marker_marker_type_e jak_uintvar_marker_peek_marker(jak_uintvar_marker_t data)
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

jak_u8 jak_uintvar_marker_bytes_needed_value(jak_u64 value)
{
        jak_uintvar_marker_marker_type_e marker = jak_uintvar_marker_type_for(value);
        return jak_uintvar_marker_bytes_needed_for_value_by_type(marker);
}

jak_u8 jak_uintvar_marker_bytes_needed_for_value_by_type(jak_uintvar_marker_marker_type_e type)
{
        JAK_ERROR_PRINT_IF(type >= jak_global_uintvar_marker_nreg, JAK_ERR_MARKERMAPPING)
        return jak_global_uintvar_marker_reg[type].size;
}

jak_u8 jak_uintvar_marker_bytes_needed_complete(jak_u64 value)
{
        jak_u8 marker_size = JAK_UINTVAR_MARKER_BYTES_NEEDED_FOR_MARKER();
        jak_u8 value_size = jak_uintvar_marker_bytes_needed_value(value);
        return marker_size + value_size;
}

char jak_uintvar_marker_type_str(jak_uintvar_marker_marker_type_e marker)
{
        JAK_ERROR_PRINT_IF(marker >= jak_global_uintvar_marker_nreg, JAK_ERR_MARKERMAPPING)
        return (char) jak_global_uintvar_marker_reg[marker].marker;
}

int jak_uintvar_marker_compare(jak_uintvar_marker_marker_type_e lhs, jak_uintvar_marker_marker_type_e rhs)
{
        switch (lhs) {
                case JAK_UINTVAR_8:
                        switch (rhs) {
                                case JAK_UINTVAR_8:  return 0;
                                case JAK_UINTVAR_16:
                                case JAK_UINTVAR_32:
                                case JAK_UINTVAR_64: return -1;
                                default:
                                JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                                        return -1;
                        }
                case JAK_UINTVAR_16:
                        switch (rhs) {
                                case JAK_UINTVAR_8:  return 1;
                                case JAK_UINTVAR_16: return 0;
                                case JAK_UINTVAR_32:
                                case JAK_UINTVAR_64: return -1;
                                default:
                                JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                                        return -1;
                        }
                case JAK_UINTVAR_32:
                        switch (rhs) {
                                case JAK_UINTVAR_8:
                                case JAK_UINTVAR_16: return 1;
                                case JAK_UINTVAR_32: return 0;
                                case JAK_UINTVAR_64: return -1;
                                default:
                                JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                                        return -1;
                        }
                case JAK_UINTVAR_64:
                        switch (rhs) {
                                case JAK_UINTVAR_8:
                                case JAK_UINTVAR_16:
                                case JAK_UINTVAR_32: return 1;
                                case JAK_UINTVAR_64: return 0;
                                default:
                                JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                                        return -1;
                        }
                default:
                        JAK_ERROR_PRINT(JAK_ERR_INTERNALERR)
                        return -1;
        }
}