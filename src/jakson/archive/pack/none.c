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
#include <jakson/archive/pack.h>
#include <jakson/archive/pack/none.h>

bool pack_none_init(packer *self)
{
        UNUSED(self);
        /** nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_cpy(const packer *self, packer *dst)
{
        CHECK_TAG(self->tag, PACK_NONE);

        /** nothing to hard copy but the function pointers */
        *dst = *self;
        return true;
}

bool pack_none_drop(packer *self)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);
        /** nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_write_extra(packer *self, memfile *dst,
                           const vector ofType (const char *) *strings)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);
        UNUSED(dst);
        UNUSED(strings);
        /** nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_read_extra(packer *self, FILE *src, size_t nbytes)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);
        UNUSED(src);
        UNUSED(nbytes);
        /** nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_print_extra(packer *self, FILE *file, memfile *src)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);
        UNUSED(file);
        UNUSED(src);
        /** nothing to do for uncompressed dictionaries */
        return true;
}

bool pack_none_print_encoded_string(packer *self, FILE *file, memfile *src,
                                    u32 decompressed_strlen)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);

        const char *string = MEMFILE_READ(src, decompressed_strlen);

        char *printableString = MALLOC(decompressed_strlen + 1);
        memcpy(printableString, string, decompressed_strlen);
        printableString[decompressed_strlen] = '\0';

        fprintf(file, "[string_buffer: %s]", printableString);

        free(printableString);

        return true;
}

bool pack_none_encode_string(packer *self, memfile *dst, err *err,
                             const char *string)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);

        u32 string_length = strlen(string);

        SUCCESS_OR_JUMP(memfile_write(dst, string, string_length), error_handling)

        return true;

        error_handling:
        ERROR(err, ERR_IO)
        return false;
}

bool pack_none_decode_string(packer *self, char *dst, size_t strlen, FILE *src)
{
        CHECK_TAG(self->tag, PACK_NONE);

        UNUSED(self);

        size_t num_read = fread(dst, sizeof(char), strlen, src);
        return (num_read == strlen);
}
