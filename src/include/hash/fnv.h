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

#ifndef CARBON_FNV_H
#define CARBON_FNV_H

#include "hash.h"

CARBON_BEGIN_DECL

#define CARBON_HASH_FNV(key_size, key)              CARBON_HASH_FNV_WTYPE(key_size, key, carbon_hash_t)
#define CARBON_HASH16_FNV(key_size, key)            CARBON_HASH_FNV_WTYPE(key_size, key, carbon_hash16_t)
#define CARBON_HASH8_FNV(key_size, key)            CARBON_HASH_FNV_WTYPE(key_size, key, carbon_hash8_t)

#define CARBON_HASH_FNV_WTYPE(key_size, key, hash_type)                                                                \
({                                                                                                                     \
    assert ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = (hash_type) 2166136261;                                                                           \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash = (hash * 16777619) ^ ((unsigned char* )key)[i];                                                          \
    }                                                                                                                  \
    hash;                                                                                                              \
})

CARBON_END_DECL

#endif