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

#ifndef CARBON_HUFFMAN_H
#define CARBON_HUFFMAN_H

#include "shared/common.h"
#include "stdx/vec.h"
#include "mem/memfile.h"
#include "shared/types.h"

CARBON_BEGIN_DECL

typedef struct carbon_huffman
{
    vec_t ofType(carbon_huffman_entry_t) table;
    struct err err;
} carbon_huffman_t;

typedef struct
{
    unsigned char letter;
    u32 *blocks;
    u16 nblocks;
} carbon_huffman_entry_t;

typedef struct
{
    unsigned char letter;
    u8 nbytes_prefix;
    char *prefix_code;
} carbon_huffman_entry_info_t;

typedef struct
{
    u32 nbytes_encoded;
    const char *encoded_bytes;
} carbon_huffman_encoded_str_info_t;

CARBON_EXPORT(bool)
carbon_huffman_create(carbon_huffman_t *dic);

CARBON_EXPORT(bool)
carbon_huffman_cpy(carbon_huffman_t *dst, carbon_huffman_t *src);

CARBON_EXPORT(bool)
carbon_huffman_build(carbon_huffman_t *encoder, const carbon_string_ref_vec *strings);

CARBON_EXPORT(bool)
carbon_huffman_get_error(struct err *err, const carbon_huffman_t *dic);

CARBON_EXPORT(bool)
carbon_huffman_encode_one(memfile_t *file, carbon_huffman_t *dic, const char *string);

CARBON_EXPORT(bool)
carbon_huffman_read_string(carbon_huffman_encoded_str_info_t *info, memfile_t *src);

CARBON_EXPORT(bool)
carbon_huffman_drop(carbon_huffman_t *dic);

CARBON_EXPORT(bool)
carbon_huffman_serialize_dic(memfile_t *file, const carbon_huffman_t *dic, char marker_symbol);

CARBON_EXPORT(bool)
carbon_huffman_read_dic_entry(carbon_huffman_entry_info_t *info, memfile_t *file, char marker_symbol);

CARBON_END_DECL

#endif
