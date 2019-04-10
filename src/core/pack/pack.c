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

#include <assert.h>
#include "core/pack/pack.h"

static bool
create_strategy(size_t i, struct packer *strategy)
{
    assert(strategy);
    carbon_compressor_strategy_register[i].create(strategy);
    assert (strategy->create);
    assert (strategy->cpy);
    assert (strategy->drop);
    assert (strategy->write_extra);
    assert (strategy->encode_string);
    assert (strategy->decode_string);
    assert (strategy->print_extra);
    return strategy->create(strategy);
}

NG5_EXPORT(bool)
carbon_compressor_by_type(struct err *err, struct packer *strategy, enum packer_type type)
{
    for (size_t i = 0; i < NG5_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].type == type) {
            return create_strategy(i, strategy);
        }
    }
    error(err, NG5_ERR_NOCOMPRESSOR)
    return false;
}

NG5_EXPORT(u8)
carbon_compressor_flagbit_by_type(enum packer_type type)
{
    for (size_t i = 0; i < NG5_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].type == type) {
            return carbon_compressor_strategy_register[i].flag_bit;
        }
    }
    return 0;
}

NG5_EXPORT(bool)
carbon_compressor_by_flags(struct packer *strategy, u8 flags)
{
    for (size_t i = 0; i < NG5_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].flag_bit & flags) {
            return create_strategy(i, strategy);
        }
    }
    return false;
}

NG5_EXPORT(bool)
carbon_compressor_by_name(enum packer_type *type, const char *name)
{
    for (size_t i = 0; i < NG5_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (strcmp(carbon_compressor_strategy_register[i].name, name) == 0) {
            *type = carbon_compressor_strategy_register[i].type;
            return true;
        }
    }
    return false;
}

NG5_EXPORT(bool)
carbon_compressor_cpy(struct err *err, struct packer *dst, const struct packer *src)
{
    NG5_NON_NULL_OR_ERROR(dst)
    NG5_NON_NULL_OR_ERROR(src)
    NG5_IMPLEMENTS_OR_ERROR(err, src, cpy)
    return src->cpy(src, dst);
}

NG5_EXPORT(bool)
carbon_compressor_drop(struct err *err, struct packer *self)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, drop)
    return self->drop(self);
}

NG5_EXPORT(bool)
carbon_compressor_write_extra(struct err *err, struct packer *self, struct memfile *dst,
                              const struct vector ofType (const char *) *strings)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, write_extra)
    return self->write_extra(self, dst, strings);
}

NG5_EXPORT(bool)
carbon_compressor_read_extra(struct err *err, struct packer *self, FILE *src, size_t nbytes)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, read_extra)
    return self->read_extra(self, src, nbytes);
}

NG5_EXPORT(bool)
carbon_compressor_encode(struct err *err, struct packer *self, struct memfile *dst,
                         const char *string)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, encode_string)
    return self->encode_string(self, dst, err, string);
}

NG5_EXPORT(bool)
carbon_compressor_decode(struct err *err, struct packer *self, char *dst, size_t strlen, FILE *src)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, decode_string)
    return self->decode_string(self, dst, strlen, src);
}

NG5_EXPORT(bool)
carbon_compressor_print_extra(struct err *err, struct packer *self, FILE *file, struct memfile *src)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, print_extra)
    return self->print_extra(self, file, src);
}

NG5_EXPORT(bool)
carbon_compressor_print_encoded(struct err *err, struct packer *self, FILE *file, struct memfile *src,
                                u32 decompressed_strlen)
{
    NG5_NON_NULL_OR_ERROR(self)
    NG5_IMPLEMENTS_OR_ERROR(err, self, print_encoded)
    return self->print_encoded(self, file, src, decompressed_strlen);
}