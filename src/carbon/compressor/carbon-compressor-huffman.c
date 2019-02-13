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
#include <carbon/carbon-bitmap.h>
#include "carbon/carbon-huffman.h"
#include "carbon/compressor/carbon-compressor-huffman.h"

#define  MARKER_SYMBOL_HUFFMAN_DIC_ENTRY   'd'


void huffman_dump_dictionary(FILE *file, carbon_memfile_t *memfile)
{
    while ((*CARBON_MEMFILE_PEEK(memfile, char)) == MARKER_SYMBOL_HUFFMAN_DIC_ENTRY) {
        carbon_huffman_entry_info_t entry_info;
        carbon_off_t offset;
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
}

void huffman_dump_string_table(FILE *file, carbon_memfile_t *memfile)
{
    char marker;
    while ((marker = *CARBON_MEMFILE_PEEK(memfile, char)) == MARKER_SYMBOL_EMBEDDED_STR) {
        carbon_huffman_encoded_str_info_t info;
        carbon_off_t offset;
        carbon_memfile_tell(&offset, memfile);
        carbon_huffman_read_string(&info, memfile, MARKER_SYMBOL_EMBEDDED_STR);
        fprintf(file, "0x%04x ", (unsigned) offset);
        fprintf(file, "[marker: %c] [string_id: '%"PRIu64"'] [string_length: '%d'] [nbytes_encoded: %d] [bytes: ",
            marker, info.string_id, info.str_length, info.nbytes_encoded);
        for (size_t i = 0; i < info.nbytes_encoded; i++) {
            char byte = info.encoded_bytes[i];
            carbon_bitmap_print_bits_in_char(file, byte);
            fprintf(file, "%s", i + 1 < info.nbytes_encoded ? "," : "");
        }
        fprintf(file, "]\n");
    }
}


void compressor_huffman_write_dictionary(carbon_compressor_t *self, carbon_memfile_t *memfile, const carbon_vec_t ofType (const char *) *strings,
                                         const carbon_vec_t ofType(carbon_string_id_t) *string_ids)
{
    carbon_huffman_t *dic;

    carbon_huffman_create(&dic, strings);
    carbon_huffman_serialize_dic(memfile, dic, MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);
    carbon_huffman_encode(memfile, dic, MARKER_SYMBOL_EMBEDDED_STR, string_ids, strings);
    carbon_huffman_drop(dic);
}

void compressor_huffman_dump_dictionary(carbon_compressor_t *self, FILE *file, carbon_memfile_t *memfile)
{
    huffman_dump_dictionary(file, memfile);
    huffman_dump_string_table(file, memfile);
}

bool compressor_huffman_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err, carbon_string_id_t string_id,
                                      const char *string)
{

}

char *compressor_huffman_decode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err, carbon_string_id_t string_id)
{

}