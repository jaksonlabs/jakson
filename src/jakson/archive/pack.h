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

#ifndef PACK_H
#define PACK_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/archive/pack/none.h>
#include <jakson/archive/pack/huffman.h>
#include <jakson/archive/huffman.h>
#include <jakson/stdinc.h>
#include <jakson/types.h>

BEGIN_DECL

/**
 * Unique tag identifying a specific implementation for compressing/decompressing string in a CARBON archives
 * string table.
 */
typedef enum packer_e {
        PACK_NONE, PACK_HUFFMAN
} packer_e;

/**
 * Main interface for the compressor framework. A compressor is used to encode/decode strings stored in a
 * CARBON archive.
 */
typedef struct packer {
        /** Tag identifying the implementation of this compressor */
        packer_e tag;

        /** Implementation-specific storage */
        void *extra;

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
        bool (*create)(packer *self);

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
        bool (*drop)(packer *self);

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
        bool (*cpy)(const packer *self, packer *dst);

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
        bool (*write_extra)(packer *self, memfile *dst,
                            const vector ofType (const char *) *strings);

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
        bool (*read_extra)(packer *self, FILE *src, size_t nbytes);

        /**
         * Encodes an input string and writes its encoded version into a memory file.
         *
         * @param self A pointer to the compressor that is used; maybe accesses <code>extra</code>
         * @param dst A memory file in which the encoded string should be stored
         * @param err An ERROR information
         * @param string The string that should be encoded
         *
         * @return <b>true</b> in case of success, or <b>false</b> otherwise.
         *
         * @author Marcus Pinnecke
         * @since 0.1.00.05
         */
        bool
        (*encode_string)(packer *self, memfile *dst, err *err, const char *string);

        bool (*decode_string)(packer *self, char *dst, size_t strlen, FILE *src);

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
        bool (*print_extra)(packer *self, FILE *file, memfile *src);

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
        bool
        (*print_encoded)(packer *self, FILE *file, memfile *src, u32 decompressed_strlen);
} packer;

static void pack_none_create(packer *strategy)
{
        strategy->tag = PACK_NONE;
        strategy->create = pack_none_init;
        strategy->cpy = pack_none_cpy;
        strategy->drop = pack_none_drop;
        strategy->write_extra = pack_none_write_extra;
        strategy->read_extra = pack_none_read_extra;
        strategy->encode_string = pack_none_encode_string;
        strategy->decode_string = pack_none_decode_string;
        strategy->print_extra = pack_none_print_extra;
        strategy->print_encoded = pack_none_print_encoded_string;
}

static void pack_huffman_create(packer *strategy)
{
        strategy->tag = PACK_HUFFMAN;
        strategy->create = pack_huffman_init;
        strategy->cpy = pack_coding_huffman_cpy;
        strategy->drop = pack_coding_huffman_drop;
        strategy->write_extra = pack_huffman_write_extra;
        strategy->read_extra = pack_huffman_read_extra;
        strategy->encode_string = pack_huffman_encode_string;
        strategy->decode_string = pack_huffman_decode_string;
        strategy->print_extra = pack_huffman_print_extra;
        strategy->print_encoded = pack_huffman_print_encoded;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static struct {
        packer_e type;
        const char *name;
        void (*create)(packer *strategy);
        u8 flag_bit;
} global_pack_strategy_register[] =
        {{.type = PACK_NONE, .name = "none", .create = pack_none_create, .flag_bit = 1 << 0},
         {.type = PACK_HUFFMAN, .name = "huffman", .create = pack_huffman_create, .flag_bit = 1 << 1}};

#pragma GCC diagnostic pop

bool pack_cpy(err *err, packer *dst, const packer *src);
bool pack_drop(err *err, packer *self);

bool pack_by_type(err *err, packer *strategy, packer_e type);
u8 pack_flagbit_by_type(packer_e type);
bool pack_by_flags(packer *strategy, u8 flags);
bool pack_by_name(packer_e *type, const char *name);

bool pack_write_extra(err *err, packer *self, memfile *dst, const vector ofType (const char *) *strings);
bool pack_read_extra(err *err, packer *self, FILE *src, size_t nbytes);
bool pack_encode(err *err, packer *self, memfile *dst, const char *string);
bool pack_decode(err *err, packer *self, char *dst, size_t strlen, FILE *src);
bool pack_print_extra(err *err, packer *self, FILE *file, memfile *src);
bool pack_print_encoded(err *err, packer *self, FILE *file, memfile *src, u32 decompressed_strlen);

END_DECL

#endif
