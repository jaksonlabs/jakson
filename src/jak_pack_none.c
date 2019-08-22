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

bool jak_pack_none_init(jak_packer *self)
{
        JAK_UNUSED(self);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool jak_pack_none_cpy(const jak_packer *self, jak_packer *dst)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        /* nothing to hard copy but the function pointers */
        *dst = *self;
        return true;
}

bool jak_pack_none_drop(jak_packer *self)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool jak_pack_none_write_extra(jak_packer *self, jak_memfile *dst,
                           const jak_vector ofType (const char *) *strings)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(dst);
        JAK_UNUSED(strings);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool jak_pack_none_read_extra(jak_packer *self, FILE *src, size_t nbytes)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(src);
        JAK_UNUSED(nbytes);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool jak_pack_none_print_extra(jak_packer *self, FILE *file, jak_memfile *src)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);
        JAK_UNUSED(file);
        JAK_UNUSED(src);
        /* nothing to do for uncompressed dictionaries */
        return true;
}

bool jak_pack_none_print_encoded_string(jak_packer *self, FILE *file, jak_memfile *src,
                                    jak_u32 decompressed_strlen)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);

        const char *string = JAK_MEMFILE_READ(src, decompressed_strlen);

        char *printableString = JAK_MALLOC(decompressed_strlen + 1);
        memcpy(printableString, string, decompressed_strlen);
        printableString[decompressed_strlen] = '\0';

        fprintf(file, "[string: %s]", printableString);

        free(printableString);

        return true;
}

bool jak_pack_none_encode_string(jak_packer *self, jak_memfile *dst, jak_error *err,
                             const char *string)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);

        jak_u32 jak_string_length = strlen(string);

        JAK_SUCCESS_OR_JUMP(jak_memfile_write(dst, string, jak_string_length), error_handling)

        return true;

        error_handling:
        JAK_ERROR(err, JAK_ERR_IO)
        return false;
}

bool jak_pack_none_decode_string(jak_packer *self, char *dst, size_t strlen, FILE *src)
{
        JAK_CHECK_TAG(self->tag, JAK_PACK_NONE);

        JAK_UNUSED(self);

        size_t num_read = fread(dst, sizeof(char), strlen, src);
        return (num_read == strlen);
}
