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

#ifndef JAK_UINTVAR_MARKER_H
#define JAK_UINTVAR_MARKER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include "jakson/jak_stdinc.h"
#include "stdbool.h"
#include "jakson/jak_types.h"

/**
 * This type is for variable-length unsigned integer types.
 *
 * The encoding uses a dedicated byte (called marker) to identify the number of subsequent bytes that holding the
 * actual value: if the first byte read is...
 *      - ... 'c', then the next byte contains an unsigned integer value of 8bit.
 *      - ... 'd', then the next 2 bytes contain an unsigned integer value of 16bit.
 *      - ... 'i', then the next 4 bytes contain an unsigned integer value of 32bit.
 *      - ... 'l', then the next 8 bytes contain an unsigned integer value of 64bit.
 *
 * This implementation supports variable-length encoding to the maximum of unsigned integers of
 * 64bit (fixed-sized) requiring constant 1 byte more than the standard C type.
 *
 * Note that size requirements for this kind of variable-length encoding is (relatively) huge;
 * the encoding requires as least 12.5% additional storage (to encode 64bit integers) and at most
 * 100.0% (!) additional storage (to encode 8bit integers). The benefit of marker-based variable-length encoding is that
 * read-/write performance is superior to byte-stream based variable-length encoding (see <code>uintvar_stream</code>),
 * and that size requirements payoff for values larger than 65536. Faster read/write performance compared to byte-stream
 * based variable-length encoding comes by the fact that after determination of actual number of bytes to reads
 * (i.e., the marker), there is no interpretation overhead to read the actual value while in byte-stream based encoding
 * each subsequent byte must be inspect (on whether it is the last byte in the stream) before reading its contained
 * value (after some byte shift operations).
 *
 * Rule of thumb:
 *      - if fixed-length types are a good choice, and...
 *          - ... if speed matters, use fast-types of the C library (e.g., <code>uint_fast32_t</code>)
 *          - ... if space matters, use fix-types of the C library (e.g., <code>uint32_t</code>)
 *      - if variable-length types are a good choice, and...
 *          - ... if space shall be minimized in exchange of read/write performance, use <code>jak_uintvar_stream_t</code>
 *          - ... if read/write performance shall be maximized in exchange of space, use <code>jak_uintvar_marker_t</code>
 */

JAK_BEGIN_DECL

#define UINT_VAR_MARKER_8 'c'
#define UINT_VAR_MARKER_16 'd'
#define UINT_VAR_MARKER_32 'i'
#define UINT_VAR_MARKER_64 'l'

typedef void *jak_uintvar_marker_t;

typedef enum jak_uintvar_marker {
        JAK_UINTVAR_8,
        JAK_UINTVAR_16,
        JAK_UINTVAR_32,
        JAK_UINTVAR_64
} jak_uintvar_marker_e;

bool jak_uintvar_marker_write(jak_uintvar_marker_t dst, jak_u64 value);
jak_u64 jak_uintvar_marker_read(jak_u8 *nbytes_read, jak_uintvar_marker_t src);
jak_uintvar_marker_e jak_uintvar_marker_type_for(jak_u64 value);
bool jak_uintvar_marker_type(const void *data);
size_t jak_uintvar_marker_sizeof(jak_uintvar_marker_t value);
size_t jak_uintvar_marker_required_size(jak_u64 value);

JAK_END_DECL

#endif
