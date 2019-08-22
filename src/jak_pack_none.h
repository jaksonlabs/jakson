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

#ifndef JAK_COMPRESSOR_NONE_H
#define JAK_COMPRESSOR_NONE_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_types.h>
#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_memfile.h>

JAK_BEGIN_DECL

struct jak_packer;

bool pack_none_init(struct jak_packer *self);

bool pack_none_cpy(const struct jak_packer *self, struct jak_packer *dst);

bool pack_none_drop(struct jak_packer *self);

bool pack_none_write_extra(struct jak_packer *self, struct jak_memfile *dst,
                           const struct jak_vector ofType (const char *) *strings);

bool pack_none_read_extra(struct jak_packer *self, FILE *src, size_t nbytes);

bool pack_none_print_extra(struct jak_packer *self, FILE *file, struct jak_memfile *src);

bool pack_none_print_encoded_string(struct jak_packer *self, FILE *file, struct jak_memfile *src,
                                    jak_u32 decompressed_strlen);

bool pack_none_encode_string(struct jak_packer *self, struct jak_memfile *dst, jak_error *err,
                             const char *string);

bool pack_none_decode_string(struct jak_packer *self, char *dst, size_t strlen, FILE *src);

JAK_END_DECL

#endif
