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
#include <assert.h>
#include "core/pack/pack.h"
#include "core/pack/pack_none.h"

NG5_EXPORT(bool)
carbon_compressor_none_init(carbon_compressor_t *self)
{
    NG5_UNUSED(self);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    /* nothing to hard copy but the function pointers */
    *dst = *self;
    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_drop(carbon_compressor_t *self)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_write_extra(carbon_compressor_t *self, struct memfile *dst,
                                        const struct vector ofType (const char *) *strings)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);
    NG5_UNUSED(dst);
    NG5_UNUSED(strings);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);
    NG5_UNUSED(src);
    NG5_UNUSED(nbytes);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

bool carbon_compressor_none_print_extra(carbon_compressor_t *self, FILE *file, struct memfile *src)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);
    NG5_UNUSED(file);
    NG5_UNUSED(src);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_print_encoded_string(carbon_compressor_t *self,
                                                 FILE *file,
                                                 struct memfile *src,
                                                 u32 decompressed_strlen)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);

    const char         *string        =  NG5_MEMFILE_READ(src, decompressed_strlen);

    char *printableString = malloc(decompressed_strlen + 1);
    memcpy(printableString, string, decompressed_strlen);
    printableString[decompressed_strlen] = '\0';

    fprintf(file, "[string: %s]", printableString);

    free(printableString);

    return true;
}

NG5_EXPORT(bool)
carbon_compressor_none_encode_string(carbon_compressor_t *self, struct memfile *dst, struct err *err,
                                          const char *string)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);

    u32 string_length = strlen(string);

    NG5_SUCCESS_OR_JUMP(memfile_write(dst, string, string_length), error_handling)

    return true;

error_handling:
    error(err, NG5_ERR_IO)
    return false;
}

NG5_EXPORT(bool)
carbon_compressor_none_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    NG5_CHECK_TAG(self->tag, NG5_COMPRESSOR_NONE);

    NG5_UNUSED(self);

    size_t num_read = fread(dst, sizeof(char), strlen, src);
    return (num_read == strlen);
}
