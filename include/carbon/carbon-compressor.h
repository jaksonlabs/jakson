// file: carbon-archive-compr.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CARBON_ARCHIVE_COMPR_H
#define CARBON_ARCHIVE_COMPR_H

#include <carbon/compressor/carbon-compressor-none.h>
#include <carbon/compressor/carbon-compressor-huffman.h>
#include "carbon-common.h"
#include "carbon-types.h"

CARBON_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded */

/**
 * Unique tag identifying a specific implementation for compressing/decompressing string in a CARBON archives
 * string table.
 */
typedef enum carbon_archive_compressor_type
{
    CARBON_COMPRESSOR_NONE,
    CARBON_COMPRESSOR_HUFFMAN
} carbon_compressor_type_e;

/**
 * Main interface for the compressor framework. A compressor is used to encode/decode strings stored in a
 * CARBON archive.
 */
typedef struct carbon_compressor
{
    /** Tag identifying the implementation of this compressor */
    carbon_compressor_type_e  tag;

    /** Implementation-specific storage */
    void  *extra;

    /**
     * Function to construct and serialize an implementation-specific dictionary (e.g., a code table)
     *
     * Depending on the implementation, a set of book-keeping data must be managed for an compressor. For instance,
     * in case of a huffman encoded this book-keeping is the code table that maps letters to prefix codes. This
     * function is invoked before any string gets encoded, and implements a compressor-specific management and
     * serialization of that book-keeping data. After construction of this book-keeping data, this data is serialized
     * into the <code>dst</code> parameter.
     *
     * Note: single strings must not be encoded, this is is done when the framework invokes 'encode_str'
     *
     * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
     * @param dst A memory file in which the book-keeping data for this compressor is serialized
     * @param strings The set of all strings that should be encoded
     * @param string_ids The set of all string ids that map to strings in <code>strings</code>
     *
     * @note Both <code>strings</code> and <code>string_ids</code> have the same length, and the i-th element
     * in <code>string_id</code> maps to the i-th element in <code>strings</code>. This mapping is bijective, and
     * strings in <code>strings</code> are unique (but not sorted)
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     * */
    void (*build_and_store)(carbon_compressor_t *self, carbon_memfile_t *dst,
                            const carbon_vec_t ofType (const char *) *strings,
                            const carbon_vec_t ofType(carbon_string_id_t) *string_ids);

    bool (*encode_string)(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                          carbon_string_id_t string_id, const char *string);

    char *(*decode_string)(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                           carbon_string_id_t string_id);

    void (*dump_dic)(carbon_compressor_t *self, FILE *file, carbon_memfile_t *memfile);

} carbon_compressor_t;

static void carbon_compressor_none_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_NONE;
    strategy->build_and_store = compressor_none_write_dictionary;
    strategy->encode_string   = compressor_none_encode_string;
    strategy->decode_string   = compressor_none_decode_string;
    strategy->dump_dic        = compressor_none_dump_dictionary;
}

static void carbon_compressor_huffman_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_HUFFMAN;
    strategy->build_and_store = compressor_huffman_write_dictionary;
    strategy->encode_string   = compressor_huffman_encode_string;
    strategy->decode_string   = compressor_huffman_decode_string;
    strategy->dump_dic        = compressor_huffman_dump_dictionary;
}

struct
{
    carbon_compressor_type_e             type;
    void (*create) (carbon_compressor_t *strategy);
    uint8_t                              flag_bit;
} compressor_strategy_register[] =
{
    { .type = CARBON_COMPRESSOR_NONE,    .create = carbon_compressor_none_create,    .flag_bit = 1 << 0 },
    { .type = CARBON_COMPRESSOR_HUFFMAN, .create = carbon_compressor_huffman_create, .flag_bit = 1 << 1  }
};

bool compressor_strategy_by_type(carbon_err_t *err, carbon_compressor_t *strategy, carbon_compressor_type_e type);

bool compressor_strategy_by_flags(carbon_compressor_t *strategy, uint8_t flags);

CARBON_END_DECL

#endif
