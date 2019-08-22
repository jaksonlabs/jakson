/**
 * Copyright 2018 Marcus Pinnecke
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

#ifndef JAK_HUFFMAN_H
#define JAK_HUFFMAN_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>
#include <jak_memfile.h>
#include <jak_types.h>

JAK_BEGIN_DECL

struct jak_huffman {
    struct vector ofType(struct pack_huffman_entry) table;
    struct err err;
};

struct pack_huffman_entry {
    unsigned char letter;
    u32 *blocks;
    u16 nblocks;
};

struct pack_huffman_info {
    unsigned char letter;
    u8 nbytes_prefix;
    char *prefix_code;
};

struct pack_huffman_str_info {
    u32 nbytes_encoded;
    const char *encoded_bytes;
};

bool coding_huffman_create(struct jak_huffman *dic);

bool coding_huffman_cpy(struct jak_huffman *dst, struct jak_huffman *src);

bool coding_huffman_build(struct jak_huffman *encoder, const string_vector_t *strings);

bool coding_huffman_get_error(struct err *err, const struct jak_huffman *dic);

bool coding_huffman_encode(struct memfile *file, struct jak_huffman *dic, const char *string);

bool coding_huffman_read_string(struct pack_huffman_str_info *info, struct memfile *src);

bool coding_huffman_drop(struct jak_huffman *dic);

bool coding_huffman_serialize(struct memfile *file, const struct jak_huffman *dic, char marker_symbol);

bool coding_huffman_read_entry(struct pack_huffman_info *info, struct memfile *file, char marker_symbol);

JAK_END_DECL

#endif
