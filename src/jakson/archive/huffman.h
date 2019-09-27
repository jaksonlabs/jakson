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

#ifndef HUFFMAN_H
#define HUFFMAN_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>
#include <jakson/mem/file.h>
#include <jakson/types.h>

BEGIN_DECL

typedef struct huffman {
        vector ofType(pack_huffman_entry) table;
        err err;
} huffman;

typedef struct pack_huffman_entry {
        unsigned char letter;
        u32 *blocks;
        u16 nblocks;
} pack_huffman_entry;

typedef struct pack_huffman_info {
        unsigned char letter;
        u8 nbytes_prefix;
        char *prefix_code;
} pack_huffman_info;

typedef struct pack_huffman_str_info {
        u32 nbytes_encoded;
        const char *encoded_bytes;
} pack_huffman_str_info;

bool coding_huffman_create(huffman *dic);
bool coding_huffman_cpy(huffman *dst, huffman *src);
bool coding_huffman_drop(huffman *dic);

bool coding_huffman_build(huffman *encoder, const string_vector_t *strings);
bool coding_huffman_get_error(err *err, const huffman *dic);
bool coding_huffman_encode(memfile *file, huffman *dic, const char *string);
bool coding_huffman_read_string(pack_huffman_str_info *info, memfile *src);
bool coding_huffman_serialize(memfile *file, const huffman *dic, char marker_symbol);
bool coding_huffman_read_entry(pack_huffman_info *info, memfile *file, char marker_symbol);

END_DECL

#endif
