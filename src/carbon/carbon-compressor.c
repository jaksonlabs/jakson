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
#include <carbon/carbon-compressor.h>
#include "carbon/carbon-compressor.h"

bool compressor_strategy_by_type(carbon_err_t *err, carbon_compressor_t *strategy, carbon_compressor_type_e type)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(compressor_strategy_register); i++) {
        if (compressor_strategy_register[i].type == type) {
            compressor_strategy_register[i].create(strategy);
            assert (strategy->tag == type);
            assert (strategy->dump_dic);
            assert (strategy->set_flags);
            assert (strategy->serialize_dic);
            return true;
        }
    }
    CARBON_ERROR(err, CARBON_ERR_NOCOMPRESSOR)
    return false;
}

bool compressor_strategy_by_flags(carbon_compressor_t *strategy, uint8_t flags)
{
    for (size_t i = 0; i < CARBON_ARRAY_LENGTH(compressor_strategy_register); i++) {
        if (compressor_strategy_register[i].flag_bit && flags) {
            compressor_strategy_register[i].create(strategy);
            return true;
        }
    }
    return false;
}

