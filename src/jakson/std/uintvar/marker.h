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

#ifndef UINTVAR_MARKER_H
#define UINTVAR_MARKER_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include "jakson/stdinc.h"
#include "stdbool.h"
#include "jakson/types.h"

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
 *          - ... if space shall be minimized in exchange of read/write performance, use <code>uintvar_stream_t</code>
 *          - ... if read/write performance shall be maximized in exchange of space, use <code>uintvar_marker_t</code>
 */

BEGIN_DECL

#define UINT_VAR_MARKER_8 'c'
#define UINT_VAR_MARKER_16 'd'
#define UINT_VAR_MARKER_32 'i'
#define UINT_VAR_MARKER_64 'l'

typedef void *uintvar_marker_t;

typedef enum uintvar_marker {
        UINTVAR_8,
        UINTVAR_16,
        UINTVAR_32,
        UINTVAR_64
} uintvar_marker_e;

bool uintvar_marker_write(uintvar_marker_t dst, u64 value);
u64 uintvar_marker_read(u8 *nbytes_read, uintvar_marker_t src);
uintvar_marker_e uintvar_marker_type_for(u64 value);
bool uintvar_marker_type(const void *data);
size_t uintvar_marker_sizeof(uintvar_marker_t value);
size_t uintvar_marker_required_size(u64 value);

END_DECL

#endif
