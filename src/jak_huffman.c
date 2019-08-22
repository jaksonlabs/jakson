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

#include <inttypes.h>

#include <jak_bitmap.h>
#include <jak_pack.h>
#include <jak_huffman.h>

#define  JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY   'd'

bool pack_huffman_init(struct jak_packer *self)
{
        self->extra = JAK_MALLOC(sizeof(struct jak_huffman));
        if (self->extra != NULL) {
                struct jak_huffman *encoder = (struct jak_huffman *) self->extra;
                coding_huffman_create(encoder);
                return true;
        } else {
                return false;
        }
}

bool pack_coding_huffman_cpy(const struct jak_packer *self, struct jak_packer *dst)
{
        JAK_check_tag(self->tag, PACK_HUFFMAN);

        *dst = *self;
        dst->extra = JAK_MALLOC(sizeof(struct jak_huffman));
        if (dst->extra != NULL) {
                struct jak_huffman *self_encoder = (struct jak_huffman *) self->extra;
                struct jak_huffman *dst_encoder = (struct jak_huffman *) dst->extra;
                return coding_huffman_cpy(dst_encoder, self_encoder);
        } else {
                return false;
        }
}

bool pack_coding_huffman_drop(struct jak_packer *self)
{
        JAK_check_tag(self->tag, PACK_HUFFMAN);

        struct jak_huffman *encoder = (struct jak_huffman *) self->extra;
        coding_huffman_drop(encoder);

        return true;
}

bool huffman_dump_dictionary(FILE *file, struct jak_memfile *memfile)
{
        struct pack_huffman_info entry_info;
        jak_offset_t offset;

        while ((*JAK_MEMFILE_PEEK(memfile, char)) == JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY) {
                memfile_get_offset(&offset, memfile);
                coding_huffman_read_entry(&entry_info, memfile, JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);

                fprintf(file, "0x%04x ", (unsigned) offset);
                fprintf(file,
                        "[marker: %c] [letter: '%c'] [nbytes_prefix: %d] [code: ",
                        JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY,
                        entry_info.letter,
                        entry_info.nbytes_prefix);

                if (entry_info.nbytes_prefix > 0) {
                        for (jak_u16 i = 0; i < entry_info.nbytes_prefix; i++) {
                                jak_bitmap_print_bits_in_char(file, entry_info.prefix_code[i]);
                                fprintf(file, "%s", i + 1 < entry_info.nbytes_prefix ? ", " : "");
                        }
                } else {
                        fprintf(file, "0b00000000");
                }

                fprintf(file, "]\n");
        }
        return true;
}

bool huffman_dump_string_table_entry(FILE *file, struct jak_memfile *memfile)
{
        JAK_UNUSED(file);
        JAK_UNUSED(memfile);

        struct pack_huffman_str_info info;

        coding_huffman_read_string(&info, memfile);

        fprintf(file, "[[nbytes_encoded: %d] [bytes: ", info.nbytes_encoded);
        for (size_t i = 0; i < info.nbytes_encoded; i++) {
                char byte = info.encoded_bytes[i];
                jak_bitmap_print_bits_in_char(file, byte);
                fprintf(file, "%s", i + 1 < info.nbytes_encoded ? "," : "");
        }
        fprintf(file, "]\n");

        return true;
}

bool pack_huffman_write_extra(struct jak_packer *self, struct jak_memfile *dst,
                              const struct jak_vector ofType (const char *) *strings)
{
        JAK_check_tag(self->tag, PACK_HUFFMAN);

        struct jak_huffman *encoder = (struct jak_huffman *) self->extra;

        coding_huffman_build(encoder, strings);
        coding_huffman_serialize(dst, encoder, JAK_MARKER_SYMBOL_HUFFMAN_DIC_ENTRY);

        return true;
}

bool pack_huffman_read_extra(struct jak_packer *self, FILE *src, size_t nbytes)
{
        JAK_check_tag(self->tag, PACK_HUFFMAN);

        JAK_UNUSED(self);
        JAK_UNUSED(src);
        JAK_UNUSED(nbytes);

        abort(); /* not implemented */
        return false;
}

bool pack_huffman_print_extra(struct jak_packer *self, FILE *file, struct jak_memfile *src)
{
        JAK_UNUSED(self);

        huffman_dump_dictionary(file, src);

        return true;
}

bool pack_huffman_print_encoded(struct jak_packer *self, FILE *file, struct jak_memfile *src,
                                jak_u32 decompressed_strlen)
{
        JAK_UNUSED(self);
        JAK_UNUSED(file);
        JAK_UNUSED(src);
        JAK_UNUSED(decompressed_strlen);

        huffman_dump_string_table_entry(file, src);

        return true;
}

bool pack_huffman_encode_string(struct jak_packer *self, struct jak_memfile *dst, struct jak_error *err, const char *string)
{
        JAK_check_tag(self->tag, PACK_HUFFMAN);

        struct jak_huffman *encoder = (struct jak_huffman *) self->extra;
        bool status = coding_huffman_encode(dst, encoder, string);
        error_cpy(err, &encoder->err);

        return status;
}

bool pack_huffman_decode_string(struct jak_packer *self, char *dst, size_t strlen, FILE *src)
{
        JAK_UNUSED(self);
        JAK_UNUSED(dst);
        JAK_UNUSED(strlen);
        JAK_UNUSED(src);
        abort(); /* not implemented */
        return false;
}