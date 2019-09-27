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

#ifndef JAK_COMPRESSOR_HUFFMAN_H
#define JAK_COMPRESSOR_HUFFMAN_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/jak_stdinc.h>
#include <jakson/std/jak_vector.h>
#include <jakson/memfile/jak_memfile.h>

JAK_BEGIN_DECL

bool jak_pack_huffman_init(jak_packer *self);
bool jak_pack_jak_coding_huffman_cpy(const jak_packer *self, jak_packer *dst);
bool jak_pack_jak_coding_huffman_drop(jak_packer *self);
bool jak_pack_huffman_write_extra(jak_packer *self, jak_memfile *dst, const jak_vector ofType (const char *) *strings);
bool jak_pack_huffman_read_extra(jak_packer *self, FILE *src, size_t nbytes);
bool jak_pack_huffman_print_extra(jak_packer *self, FILE *file, jak_memfile *src);
bool jak_pack_huffman_print_encoded(jak_packer *self, FILE *file, jak_memfile *src, jak_u32 decompressed_strlen);
bool jak_pack_huffman_encode_string(jak_packer *self, jak_memfile *dst, jak_error *err, const char *string);
bool jak_pack_huffman_decode_string(jak_packer *self, char *dst, size_t strlen, FILE *src);

JAK_END_DECL

#endif
