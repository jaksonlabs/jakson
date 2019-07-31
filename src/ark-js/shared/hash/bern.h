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

#ifndef ARK_BERNSTEIN_H
#define ARK_BERNSTEIN_H

#include "hash.h"

ARK_BEGIN_DECL

#define ARK_HASH_BERNSTEIN(key_size, key)        ARK_HASH_BERNSTEIN_WTYPE(key_size, key, hash32_t)
#define ARK_HASH8_BERNSTEIN(key_size, key)       ARK_HASH_BERNSTEIN_WTYPE(key_size, key, hash8_t)
#define ARK_HASH16_BERNSTEIN(key_size, key)      ARK_HASH_BERNSTEIN_WTYPE(key_size, key, hash16_t)

#define ARK_HASH_BERNSTEIN_WTYPE(key_size, key, hash_type)                                                             \
({                                                                                                                     \
    assert ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = 0;                                                                                                \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

ARK_END_DECL

#endif