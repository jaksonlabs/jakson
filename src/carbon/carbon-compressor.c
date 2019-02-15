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
#include "carbon/carbon-compressor.h"

static bool
create_strategy(size_t i, carbon_compressor_t *strategy)
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

CARBON_EXPORT(bool)
carbon_compressor_by_type(carbon_err_t *err, carbon_compressor_t *strategy, carbon_compressor_type_e type)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].type == type) {
            return create_strategy(i, strategy);
        }
    }
    CARBON_ERROR(err, CARBON_ERR_NOCOMPRESSOR)
    return false;
}

CARBON_EXPORT(uint8_t)
carbon_compressor_flagbit_by_type(carbon_compressor_type_e type)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].type == type) {
            return carbon_compressor_strategy_register[i].flag_bit;
        }
    }
    return 0;
}

CARBON_EXPORT(bool)
carbon_compressor_by_flags(carbon_compressor_t *strategy, uint8_t flags)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (carbon_compressor_strategy_register[i].flag_bit & flags) {
            return create_strategy(i, strategy);
        }
    }
    return false;
}

CARBON_EXPORT(bool)
carbon_compressor_by_name(carbon_compressor_type_e *type, const char *name)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
        if (strcmp(carbon_compressor_strategy_register[i].name, name) == 0) {
            *type = carbon_compressor_strategy_register[i].type;
            return true;
        }
    }
    return false;
}

CARBON_EXPORT(bool)
carbon_compressor_cpy(carbon_err_t *err, carbon_compressor_t *dst, const carbon_compressor_t *src)
{
    CARBON_NON_NULL_OR_ERROR(dst)
    CARBON_NON_NULL_OR_ERROR(src)
    CARBON_IMPLEMENTS_OR_ERROR(err, src, cpy)
    return src->cpy(src, dst);
}

CARBON_EXPORT(bool)
carbon_compressor_drop(carbon_err_t *err, carbon_compressor_t *self)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, drop)
    return self->drop(self);
}

CARBON_EXPORT(bool)
carbon_compressor_write_extra(carbon_err_t *err, carbon_compressor_t *self, carbon_memfile_t *dst,
                              const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, write_extra)
    return self->write_extra(self, dst, strings);
}

CARBON_EXPORT(bool)
carbon_compressor_encode(carbon_err_t *err, carbon_compressor_t *self, carbon_memfile_t *dst,
                         const char *string)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, encode_string)
    return self->encode_string(self, dst, err, string);
}

CARBON_EXPORT(bool)
carbon_compressor_decode(carbon_err_t *err, carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, decode_string)
    return self->decode_string(self, dst, strlen, src);
}

CARBON_EXPORT(bool)
carbon_compressor_print_extra(carbon_err_t *err, carbon_compressor_t *self, FILE *file, carbon_memfile_t *src)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, print_extra)
    return self->print_extra(self, file, src);
}

CARBON_EXPORT(bool)
carbon_compressor_print_encoded(carbon_err_t *err, carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                                uint32_t decompressed_strlen)
{
    CARBON_NON_NULL_OR_ERROR(self)
    CARBON_IMPLEMENTS_OR_ERROR(err, self, print_encoded)
    return self->print_encoded(self, file, src, decompressed_strlen);
}