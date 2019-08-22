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

typedef struct jak_huffman {
        jak_vector ofType(jak_pack_huffman_entry) table;
        jak_error err;
} jak_huffman;

typedef struct jak_pack_huffman_entry {
        unsigned char letter;
        jak_u32 *blocks;
        jak_u16 nblocks;
} jak_pack_huffman_entry;

typedef struct jak_pack_huffman_info {
        unsigned char letter;
        jak_u8 nbytes_prefix;
        char *prefix_code;
} jak_pack_huffman_info;

typedef struct jak_pack_huffman_str_info {
        jak_u32 nbytes_encoded;
        const char *encoded_bytes;
} jak_pack_huffman_str_info;

bool jak_coding_huffman_create(jak_huffman *dic);
bool jak_coding_huffman_cpy(jak_huffman *dst, jak_huffman *src);
bool jak_coding_huffman_drop(jak_huffman *dic);

bool jak_coding_huffman_build(jak_huffman *encoder, const jak_string_jak_vector_t *strings);
bool jak_coding_huffman_get_error(jak_error *err, const jak_huffman *dic);
bool jak_coding_huffman_encode(jak_memfile *file, jak_huffman *dic, const char *string);
bool jak_coding_huffman_read_string(jak_pack_huffman_str_info *info, jak_memfile *src);
bool jak_coding_huffman_serialize(jak_memfile *file, const jak_huffman *dic, char marker_symbol);
bool jak_coding_huffman_read_entry(jak_pack_huffman_info *info, jak_memfile *file, char marker_symbol);

JAK_END_DECL

#endif
