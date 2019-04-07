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

#ifndef CARBON_ARCHIVE_COMPR_H
#define CARBON_ARCHIVE_COMPR_H

#include <carbon/compressor/carbon-compressor-none.h>
#include <carbon/compressor/carbon-compressor-huffman.h>
#include <carbon/compressor/carbon-compressor-prefix.h>
#include <carbon/compressor/carbon-compressor-incremental.h>
#include "carbon-common.h"
#include "carbon-types.h"

CARBON_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded */
typedef struct carbon_doc_bulk carbon_doc_bulk_t; /* forwarded */

/**
 * Unique tag identifying a specific implementation for compressing/decompressing string in a CARBON archives
 * string table.
 */
typedef enum carbon_compressor_type
{
    CARBON_COMPRESSOR_NONE,
    CARBON_COMPRESSOR_HUFFMAN,
    CARBON_COMPRESSOR_PREFIX,
    CARBON_COMPRESSOR_INCREMENTAL
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
     * Constructor for implementation-dependent initialization of the compressor at hand.
     *
     * Depending on the implementation, the compressor might allocate dynamic memory for
     * <code>extra</code> for book-keeping purposes.
     *
     * @param self A pointer to itself
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*create)(carbon_compressor_t *self, carbon_doc_bulk_t const *context);

    bool (*prepare_entries)(carbon_compressor_t *self, carbon_vec_t ofType(carbon_strdic_entry_t) *entries);

    /**
     * Destructor for implementation-dependent deinitialization of the compressor at hand.
     *
     * If the implementation acquired dynamic memory during a call to <code>create</code>,
     * a call to this function frees up this memory.
     *
     * @param self A pointer to itself
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*drop)(carbon_compressor_t *self);

    /**
     * Perform a hard-copy of this compressor to dst
     *
     * @param self  A pointer to itself
     * @param dst   A pointer to the copy target
     *
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*cpy)(const carbon_compressor_t *self, carbon_compressor_t *dst);

    /**
     * Function to construct and serialize an implementation-specific dictionary, book-keeping data, or extra data
     * (e.g., a code table)
     *
     * Depending on the implementation, a set of book-keeping data must be managed for a compressor. For instance,
     * in case of a huffman encoded this book-keeping is the code table that maps letters to prefix codes. This
     * function is invoked before any string gets encoded, and implements a compressor-specific management and
     * serialization of that book-keeping data. After internal construction of this book-keeping data,
     * this data is serialized into the <code>dst</code> parameter.
     *
     * @note single strings must not be encoded; this is is done when the framework invokes <code>encode_string</code>
     *
     * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
     * @param dst A memory file in which the book-keeping data (<code>extra</code>) for this compressor is serialized
     * @param strings The set of all strings that should be encoded. Used for tweaking the compressor;
     *                 <b>not</b> for serialization into <code>dst</code>
     *
     * @note strings in <code>strings</code> are unique (but not sorted)
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     * */
    bool (*write_extra)(carbon_compressor_t *self, carbon_memfile_t *dst,
                            const carbon_vec_t ofType (const char *) *strings);

    /**
     * Encodes an input string and writes its encoded version into a memory file.
     *
     * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
     * @param dst A memory file in which the encoded string should be stored
     * @param err An error information
     * @param string The string that should be encoded
     *
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*encode_string)(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                          const char *string, carbon_string_id_t grouping_key);

    bool (*decode_string)(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src);

    /**
     * Reads implementation-specific book-keeping, meta or extra data from the input memory file and
     * prints its contents in a human-readable version to <code>file</code>
     *
     * @param self A pointer to the compressor that is used; potentially accessing <code>extra</code>
     * @param file A file to which a human-readable version of <code>extra</code> is printed (if any)
     * @param src A memory file which cursor is positioned at the begin of the serialized extra field. After
     *            a call to this function, the memory file cursor must be positioned after the serialized extra
     *            field (i.e., the entire entry must be read (if any))
     *
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*print_extra)(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src);

    /**
     * Reads an implementation-specific encoded string from a memory file <code>src</code>, and prints
     * the encoded string in a human-readable version to <code>file</code>
     *
     * @param self A pointer to the compressor that is used; potentially accessing <code>extra</code>
     * @param file A file to which a human-readable version of the encoded string is printed.
     * @param src A memory file which cursor is positioned at the begin of the encoded string. After a call
     *            to this function, the memory file cursor must be positioned after the encoded string (i.e.,
     *            the entire encoded string must be read)
     * @param decompressed_strlen The length of the decoded string in number of characters
     *
     * @return <b>true</b> in case of success, or <b>false</b> otherwise.
     *
     * @author Marcus Pinnecke
     * @since 0.1.00.05
     */
    bool (*print_encoded)(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                                 uint32_t decompressed_strlen);

} carbon_compressor_t;

static void carbon_compressor_none_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_NONE;
    strategy->create          = carbon_compressor_none_init;
    strategy->cpy             = carbon_compressor_none_cpy;
    strategy->drop            = carbon_compressor_none_drop;
    strategy->write_extra     = carbon_compressor_none_write_extra;
    strategy->prepare_entries = carbon_compressor_none_prepare_entries;
    strategy->encode_string   = carbon_compressor_none_encode_string;
    strategy->decode_string   = carbon_compressor_none_decode_string;
    strategy->print_extra     = carbon_compressor_none_print_extra;
    strategy->print_encoded   = carbon_compressor_none_print_encoded_string;
}

static void carbon_compressor_huffman_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_HUFFMAN;
    strategy->create          = carbon_compressor_huffman_init;
    strategy->cpy             = carbon_compressor_huffman_cpy;
    strategy->drop            = carbon_compressor_huffman_drop;
    strategy->write_extra     = carbon_compressor_huffman_write_extra;
    strategy->prepare_entries = carbon_compressor_huffman_prepare_entries;
    strategy->encode_string   = carbon_compressor_huffman_encode_string;
    strategy->decode_string   = carbon_compressor_huffman_decode_string;
    strategy->print_extra     = carbon_compressor_huffman_print_extra;
    strategy->print_encoded   = carbon_compressor_huffman_print_encoded;
}

static void carbon_compressor_prefix_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_PREFIX;
    strategy->create          = carbon_compressor_prefix_init;
    strategy->cpy             = carbon_compressor_prefix_cpy;
    strategy->drop            = carbon_compressor_prefix_drop;
    strategy->write_extra     = carbon_compressor_prefix_write_extra;
    strategy->prepare_entries = carbon_compressor_prefix_prepare_entries;
    strategy->encode_string   = carbon_compressor_prefix_encode_string;
    strategy->decode_string   = carbon_compressor_prefix_decode_string;
    strategy->print_extra     = carbon_compressor_prefix_print_extra;
    strategy->print_encoded   = carbon_compressor_prefix_print_encoded_string;
}

static void carbon_compressor_incremental_create(carbon_compressor_t *strategy)
{
    strategy->tag             = CARBON_COMPRESSOR_INCREMENTAL;
    strategy->create          = carbon_compressor_incremental_init;
    strategy->cpy             = carbon_compressor_incremental_cpy;
    strategy->drop            = carbon_compressor_incremental_drop;
    strategy->write_extra     = carbon_compressor_incremental_write_extra;
    strategy->prepare_entries = carbon_compressor_incremental_prepare_entries;
    strategy->encode_string   = carbon_compressor_incremental_encode_string;
    strategy->decode_string   = carbon_compressor_incremental_decode_string;
    strategy->print_extra     = carbon_compressor_incremental_print_extra;
    strategy->print_encoded   = carbon_compressor_incremental_print_encoded_string;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static struct
{
    carbon_compressor_type_e             type;
    const char                          *name;
    void (*create) (carbon_compressor_t *strategy);
    uint8_t                              flag_bit;
} carbon_compressor_strategy_register[] =
{
    { .type = CARBON_COMPRESSOR_NONE, .name = "none",
      .create = carbon_compressor_none_create,                .flag_bit = 1 << 0 },
    { .type = CARBON_COMPRESSOR_HUFFMAN, .name = "huffman",
      .create = carbon_compressor_huffman_create,             .flag_bit = 1 << 1  },
    { .type = CARBON_COMPRESSOR_PREFIX, .name = "prefix",
      .create = carbon_compressor_prefix_create,              .flag_bit = 1 << 2  },
    { .type = CARBON_COMPRESSOR_INCREMENTAL, .name = "incremental",
      .create = carbon_compressor_incremental_create,         .flag_bit = 1 << 3  }
};

#pragma GCC diagnostic pop



CARBON_EXPORT(bool)
carbon_compressor_by_type(carbon_err_t *err, carbon_compressor_t *strategy, carbon_doc_bulk_t const *context, carbon_compressor_type_e type);

CARBON_EXPORT(uint8_t)
carbon_compressor_flagbit_by_type(carbon_compressor_type_e type);

CARBON_EXPORT(bool)
carbon_compressor_by_flags(carbon_compressor_t *strategy, uint8_t flags);

CARBON_EXPORT(bool)
carbon_compressor_by_name(carbon_compressor_type_e *type, const char *name);


CARBON_EXPORT(bool)
carbon_compressor_cpy(carbon_err_t *err, carbon_compressor_t *dst, const carbon_compressor_t *src);

CARBON_EXPORT(bool)
carbon_compressor_drop(carbon_err_t *err, carbon_compressor_t *self);

CARBON_EXPORT(bool)
carbon_compressor_write_extra(carbon_err_t *err, carbon_compressor_t *self, carbon_memfile_t *dst,
                    const carbon_vec_t ofType (const char *) *strings);

CARBON_EXPORT(bool)
carbon_compressor_prepare_entries(carbon_err_t *err, carbon_compressor_t *self,
                                  carbon_vec_t ofType(carbon_strdic_entry_t) * entries);

CARBON_EXPORT(bool)
carbon_compressor_encode(carbon_err_t *err, carbon_compressor_t *self, carbon_memfile_t *dst,
                         const char *string, carbon_string_id_t grouping_key);

CARBON_EXPORT(bool)
carbon_compressor_decode(carbon_err_t *err, carbon_compressor_t *self, char *dst, size_t strlen, FILE *src);


CARBON_EXPORT(bool)
carbon_compressor_print_extra(carbon_err_t *err, carbon_compressor_t *self, FILE *file, carbon_memfile_t *src);

CARBON_EXPORT(bool)
carbon_compressor_print_encoded(carbon_err_t *err, carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                      uint32_t decompressed_strlen);

CARBON_END_DECL

#endif
