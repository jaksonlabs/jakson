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

#include <assert.h>
#include <inttypes.h>

#include "carbon/carbon-bitmap.h"
#include "carbon/carbon-compressor.h"
#include "carbon/carbon-huffman.h"

#define  MARKER_SYMBOL_HUFFMAN_DIC_ENTRY   'd'

CARBON_EXPORT(bool)
carbon_compressor_huffman_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context)
{
    CARBON_UNUSED(context);
    self->extra = malloc(sizeof(carbon_huffman_t));
    if (self->extra != NULL) {
        carbon_huffman_t *encoder = (carbon_huffman_t *) self->extra;
        carbon_huffman_create(encoder);
        return true;
    } else {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);

    *dst = *self;
    dst->extra = malloc(sizeof(carbon_huffman_t));
    if (dst->extra != NULL) {
        carbon_huffman_t *self_encoder = (carbon_huffman_t *) self->extra;
        carbon_huffman_t *dst_encoder = (carbon_huffman_t *) dst->extra;
        return carbon_huffman_cpy(dst_encoder, self_encoder);
    } else {
        return false;
    }
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_drop(carbon_compressor_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);

    carbon_huffman_t *encoder = (carbon_huffman_t *) self->extra;
    carbon_huffman_drop(encoder);

    return true;
}

bool huffman_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    carbon_huffman_entry_info_t entry_info;
    carbon_off_t                offset;

    while ((*CARBON_MEMFILE_PEEK(memfile, char)) == MARKER_SYMBOL_HUFFMAN_DIC_ENTRY) {
        carbon_memfile_tell(&offset, memfile);
        carbon_huffman_read_dic_entry(&entry_info, memfile, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);

        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [letter: '%c'] [nbytes_prefix: %d] [code: ",
                MARKER_SYMBOL_HUFFMAN_DIC_ENTRY, entry_info.letter,
                entry_info.nbytes_prefix);

        if (entry_info.nbytes_prefix > 0) {
            for (uint16_t i = 0; i < entry_info.nbytes_prefix; i++) {
                carbon_bitmap_print_bits_in_char(file, entry_info.prefix_code[i]);
                fprintf(file, "%s", i + 1 < entry_info.nbytes_prefix ? ", " : "");
            }
        } else {
            fprintf(file, "0b00000000");
        }

        fprintf(file, "]\n");
    }
    return true;
}

bool huffman_dump_string_table_entry(FILE *file, carbon_memfile_t *memfile)
{
    CARBON_UNUSED(file);
    CARBON_UNUSED(memfile);

    carbon_huffman_encoded_str_info_t info;

    carbon_huffman_read_string(&info, memfile);

    fprintf(file, "[[nbytes_encoded: %d] [bytes: ", info.nbytes_encoded);
    for (size_t i = 0; i < info.nbytes_encoded; i++) {
        char byte = info.encoded_bytes[i];
        carbon_bitmap_print_bits_in_char(file, byte);
        fprintf(file, "%s", i + 1 < info.nbytes_encoded ? "," : "");
    }
    fprintf(file, "]\n");

    return true;
}


CARBON_EXPORT(bool)
carbon_compressor_huffman_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                      const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);

    carbon_huffman_t *encoder = (carbon_huffman_t *) self->extra;

    carbon_huffman_build(encoder, strings);
    carbon_huffman_serialize_dic(dst, encoder, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);

    return true;
}


CARBON_EXPORT(bool)
carbon_compressor_huffman_prepare_entries(carbon_compressor_t *self,
                                       carbon_vec_t ofType(carbon_strdic_entry_t) *entries)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);
    CARBON_UNUSED(self);
    CARBON_UNUSED(entries);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);

    CARBON_UNUSED(self);
    CARBON_UNUSED(src);
    CARBON_UNUSED(nbytes);

    abort(); /* not implemented */
    return false;
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(nbytes);

    huffman_dump_dictionary(file, src);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_print_encoded(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                                        uint32_t decompressed_strlen)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(src);
    CARBON_UNUSED(decompressed_strlen);

    huffman_dump_string_table_entry(file, src);

    return true;
}

bool carbon_compressor_huffman_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                             const char *string, carbon_string_id_t grouping_key)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_HUFFMAN);
    CARBON_UNUSED(grouping_key);

    carbon_huffman_t *encoder = (carbon_huffman_t *) self->extra;
    bool status = carbon_huffman_encode_one(dst, encoder, string);
    carbon_error_cpy(err, &encoder->err);

    return status;
}

CARBON_EXPORT(bool)
carbon_compressor_huffman_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(dst);
    CARBON_UNUSED(strlen);
    CARBON_UNUSED(src);
    abort(); /* not implemented */
    return false;
}
