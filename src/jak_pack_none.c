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
#include <jak_pack.h>
#include <jak_pack_none.h>

bool pack_none_init(struct jak_packer *self)
{
        JAK_UNUSED(self);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_cpy(const struct jak_packer *self, struct jak_packer *dst)
{
        JAK_check_tag(self->tag, PACK_NONE);

        /* nothing to hard copy but the function pointers */
        *dst = *self;
        return true;
}

bool pack_none_drop(struct jak_packer *self)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_write_extra(struct jak_packer *self, struct jak_memfile *dst,
                           const struct jak_vector ofType (const char *) *strings)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(dst);
        JAK_UNUSED(strings);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_read_extra(struct jak_packer *self, FILE *src, size_t nbytes)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(src);
        JAK_UNUSED(nbytes);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_print_extra(struct jak_packer *self, FILE *file, struct jak_memfile *src)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(file);
        JAK_UNUSED(src);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_print_encoded_string(struct jak_packer *self, FILE *file, struct jak_memfile *src,
                                    jak_u32 decompressed_strlen)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);

        const char *string = JAK_MEMFILE_READ(src, decompressed_strlen);

        char *printableString = JAK_MALLOC(decompressed_strlen + 1);
        memcpy(printableString, string, decompressed_strlen);
        printableString[decompressed_strlen] = '\0';

        fprintf(file, "[string: %s]", printableString);

        free(printableString);

        return true;
}

bool pack_none_encode_string(struct jak_packer *self, struct jak_memfile *dst, struct jak_error *err,
                             const char *string)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);

        jak_u32 string_length = strlen(string);

        JAK_success_or_jump(memfile_write(dst, string, string_length), error_handling)

        return true;

        error_handling:
        error(err, JAK_ERR_IO)
        return false;
}

bool pack_none_decode_string(struct jak_packer *self, char *dst, size_t strlen, FILE *src)
{
        JAK_check_tag(self->tag, PACK_NONE);

        JAK_UNUSED(self);

        size_t num_read = fread(dst, sizeof(char), strlen, src);
        return (num_read == strlen);
}
