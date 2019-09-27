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

#include <jakson/archive/pack.h>

static bool create_strategy(size_t i, packer *strategy)
{
        JAK_ASSERT(strategy);
        global_pack_strategy_register[i].create(strategy);
        JAK_ASSERT (strategy->create);
        JAK_ASSERT (strategy->cpy);
        JAK_ASSERT (strategy->drop);
        JAK_ASSERT (strategy->write_extra);
        JAK_ASSERT (strategy->encode_string);
        JAK_ASSERT (strategy->decode_string);
        JAK_ASSERT (strategy->print_extra);
        return strategy->create(strategy);
}

bool pack_by_type(err *err, packer *strategy, packer_e type)
{
        for (size_t i = 0; i < ARRAY_LENGTH(global_pack_strategy_register); i++) {
                if (global_pack_strategy_register[i].type == type) {
                        return create_strategy(i, strategy);
                }
        }
        ERROR(err, ERR_NOCOMPRESSOR)
        return false;
}

u8 pack_flagbit_by_type(packer_e type)
{
        for (size_t i = 0; i < ARRAY_LENGTH(global_pack_strategy_register); i++) {
                if (global_pack_strategy_register[i].type == type) {
                        return global_pack_strategy_register[i].flag_bit;
                }
        }
        return 0;
}

bool pack_by_flags(packer *strategy, u8 flags)
{
        for (size_t i = 0; i < ARRAY_LENGTH(global_pack_strategy_register); i++) {
                if (global_pack_strategy_register[i].flag_bit & flags) {
                        return create_strategy(i, strategy);
                }
        }
        return false;
}

bool pack_by_name(packer_e *type, const char *name)
{
        for (size_t i = 0; i < ARRAY_LENGTH(global_pack_strategy_register); i++) {
                if (strcmp(global_pack_strategy_register[i].name, name) == 0) {
                        *type = global_pack_strategy_register[i].type;
                        return true;
                }
        }
        return false;
}

bool pack_cpy(err *err, packer *dst, const packer *src)
{
        ERROR_IF_NULL(dst)
        ERROR_IF_NULL(src)
        ERROR_IF_NOT_IMPLEMENTED(err, src, cpy)
        return src->cpy(src, dst);
}

bool pack_drop(err *err, packer *self)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, drop)
        return self->drop(self);
}

bool pack_write_extra(err *err, packer *self, memfile *dst,
                      const vector ofType (const char *) *strings)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, write_extra)
        return self->write_extra(self, dst, strings);
}

bool pack_read_extra(err *err, packer *self, FILE *src, size_t nbytes)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, read_extra)
        return self->read_extra(self, src, nbytes);
}

bool pack_encode(err *err, packer *self, memfile *dst, const char *string)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, encode_string)
        return self->encode_string(self, dst, err, string);
}

bool pack_decode(err *err, packer *self, char *dst, size_t strlen, FILE *src)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, decode_string)
        return self->decode_string(self, dst, strlen, src);
}

bool pack_print_extra(err *err, packer *self, FILE *file, memfile *src)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, print_extra)
        return self->print_extra(self, file, src);
}

bool pack_print_encoded(err *err, packer *self, FILE *file, memfile *src,
                        u32 decompressed_strlen)
{
        ERROR_IF_NULL(self)
        ERROR_IF_NOT_IMPLEMENTED(err, self, print_encoded)
        return self->print_encoded(self, file, src, decompressed_strlen);
}