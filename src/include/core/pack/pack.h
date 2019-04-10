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

#ifndef NG5_ARCHIVE_COMPR_H
#define NG5_ARCHIVE_COMPR_H

#include "core/pack/pack_none.h"
#include "core/pack/huffman.h"
#include "coding/pack_huffman.h"
#include "shared/common.h"
#include "shared/types.h"

NG5_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded */

/**
 * Unique tag identifying a specific implementation for compressing/decompressing string in a CARBON archives
 * string table.
 */
typedef enum carbon_compressor_type
{
    NG5_COMPRESSOR_NONE,
    NG5_COMPRESSOR_HUFFMAN
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
    bool (*create)(carbon_compressor_t *self);

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
     * Reverse function of <code>read_extra</code>.
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
    bool (*write_extra)(carbon_compressor_t *self, struct memfile *dst,
                            const struct vector ofType (const char *) *strings);

    /**
     * Function to reconstruct implementation-specific dictionary, book-keeping or extra data by deserialization (
     * e.g., a code table)
     *
     * Reverse function of <code>write_extra</code>.
     *
     * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
     * @param src A file where the cursor is moved to the first byte of the extra field previously serialized with 'write_extra'
     * @param nbytes Number of bytes written when 'write_extra' was called. Intended to read read to restore the extra field.
     * @return The implementer must return <code>true</code> on success, and <code>false</code> otherwise.
     */
    bool (*read_extra)(carbon_compressor_t *self, FILE *src, size_t nbytes);

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
    bool (*encode_string)(carbon_compressor_t *self, struct memfile *dst, struct err *err,
                          const char *string);

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
    bool (*print_extra)(carbon_compressor_t *self, FILE *file, struct memfile *src);

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
    bool (*print_encoded)(carbon_compressor_t *self, FILE *file, struct memfile *src,
                                 u32 decompressed_strlen);

} carbon_compressor_t;

static void carbon_compressor_none_create(carbon_compressor_t *strategy)
{
    strategy->tag             = NG5_COMPRESSOR_NONE;
    strategy->create          = carbon_compressor_none_init;
    strategy->cpy             = carbon_compressor_none_cpy;
    strategy->drop            = carbon_compressor_none_drop;
    strategy->write_extra     = carbon_compressor_none_write_extra;
    strategy->read_extra      = carbon_compressor_none_read_extra;
    strategy->encode_string   = carbon_compressor_none_encode_string;
    strategy->decode_string   = carbon_compressor_none_decode_string;
    strategy->print_extra     = carbon_compressor_none_print_extra;
    strategy->print_encoded   = carbon_compressor_none_print_encoded_string;
}

static void carbon_compressor_huffman_create(carbon_compressor_t *strategy)
{
    strategy->tag             = NG5_COMPRESSOR_HUFFMAN;
    strategy->create          = carbon_compressor_huffman_init;
    strategy->cpy             = carbon_compressor_huffman_cpy;
    strategy->drop            = carbon_compressor_huffman_drop;
    strategy->write_extra     = carbon_compressor_huffman_write_extra;
    strategy->read_extra      = carbon_compressor_huffman_read_extra;
    strategy->encode_string   = carbon_compressor_huffman_encode_string;
    strategy->decode_string   = carbon_compressor_huffman_decode_string;
    strategy->print_extra     = carbon_compressor_huffman_print_extra;
    strategy->print_encoded   = carbon_compressor_huffman_print_encoded;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static struct
{
    carbon_compressor_type_e             type;
    const char                          *name;
    void (*create) (carbon_compressor_t *strategy);
    u8                              flag_bit;
} carbon_compressor_strategy_register[] =
{
    { .type = NG5_COMPRESSOR_NONE, .name = "none",
      .create = carbon_compressor_none_create,                .flag_bit = 1 << 0 },
    { .type = NG5_COMPRESSOR_HUFFMAN, .name = "huffman",
       .create = carbon_compressor_huffman_create,            .flag_bit = 1 << 1  }
};

#pragma GCC diagnostic pop



NG5_EXPORT(bool)
carbon_compressor_by_type(struct err *err, carbon_compressor_t *strategy, carbon_compressor_type_e type);

NG5_EXPORT(u8)
carbon_compressor_flagbit_by_type(carbon_compressor_type_e type);

NG5_EXPORT(bool)
carbon_compressor_by_flags(carbon_compressor_t *strategy, u8 flags);

NG5_EXPORT(bool)
carbon_compressor_by_name(carbon_compressor_type_e *type, const char *name);


NG5_EXPORT(bool)
carbon_compressor_cpy(struct err *err, carbon_compressor_t *dst, const carbon_compressor_t *src);

NG5_EXPORT(bool)
carbon_compressor_drop(struct err *err, carbon_compressor_t *self);

NG5_EXPORT(bool)
carbon_compressor_write_extra(struct err *err, carbon_compressor_t *self, struct memfile *dst,
                    const struct vector ofType (const char *) *strings);

NG5_EXPORT(bool)
carbon_compressor_read_extra(struct err *err, carbon_compressor_t *self, FILE *src, size_t nbytes);

NG5_EXPORT(bool)
carbon_compressor_encode(struct err *err, carbon_compressor_t *self, struct memfile *dst,
                         const char *string);

NG5_EXPORT(bool)
carbon_compressor_decode(struct err *err, carbon_compressor_t *self, char *dst, size_t strlen, FILE *src);


NG5_EXPORT(bool)
carbon_compressor_print_extra(struct err *err, carbon_compressor_t *self, FILE *file, struct memfile *src);

NG5_EXPORT(bool)
carbon_compressor_print_encoded(struct err *err, carbon_compressor_t *self, FILE *file, struct memfile *src,
                      u32 decompressed_strlen);

NG5_END_DECL

#endif
