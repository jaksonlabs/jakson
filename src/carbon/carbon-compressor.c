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

