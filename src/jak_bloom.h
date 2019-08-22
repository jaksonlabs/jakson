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

#ifndef JAK_BLOOM_H
#define JAK_BLOOM_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_bitmap.h>

JAK_BEGIN_DECL

typedef struct jak_bitmap bloom_t;

#define JAK_BLOOM_SET(filter, key, keySize)                 \
({                                                          \
    size_t nbits = bitmap_nbits(filter);                    \
    size_t b0 = JAK_HASH_ADDITIVE(keySize, key) % nbits;    \
    size_t b1 = JAK_HASH_XOR(keySize, key) % nbits;         \
    size_t b2 = JAK_HASH_ROT(keySize, key) % nbits;         \
    size_t b3 = JAK_HASH_SAX(keySize, key) % nbits;         \
    bitmap_set(filter, b0, true);                           \
    bitmap_set(filter, b1, true);                           \
    bitmap_set(filter, b2, true);                           \
    bitmap_set(filter, b3, true);                           \
})

#define JAK_BLOOM_TEST(filter, key, keySize)                \
({                                                          \
    size_t nbits = bitmap_nbits(filter);                    \
    size_t b0 = JAK_HASH_ADDITIVE(keySize, key) % nbits;    \
    size_t b1 = JAK_HASH_XOR(keySize, key) % nbits;         \
    size_t b2 = JAK_HASH_ROT(keySize, key) % nbits;         \
    size_t b3 = JAK_HASH_SAX(keySize, key) % nbits;         \
    bool b0set = bitmap_get(filter, b0);                    \
    bool b1set = bitmap_get(filter, b1);                    \
    bool b2set = bitmap_get(filter, b2);                    \
    bool b3set = bitmap_get(filter, b3);                    \
    (b0set && b1set && b2set && b3set);                     \
})

#define JAK_BLOOM_TEST_AND_SET(filter, key, keySize)        \
({                                                          \
    size_t nbits = bitmap_nbits(filter);                    \
    size_t b0 = JAK_HASH_ADDITIVE(keySize, key) % nbits;    \
    size_t b1 = JAK_HASH_XOR(keySize, key) % nbits;         \
    size_t b2 = JAK_HASH_ROT(keySize, key) % nbits;         \
    size_t b3 = JAK_HASH_SAX(keySize, key) % nbits;         \
    bool b0set = bitmap_get(filter, b0);                    \
    bool b1set = bitmap_get(filter, b1);                    \
    bool b2set = bitmap_get(filter, b2);                    \
    bool b3set = bitmap_get(filter, b3);                    \
    bitmap_set(filter, b0, true);                           \
    bitmap_set(filter, b1, true);                           \
    bitmap_set(filter, b2, true);                           \
    bitmap_set(filter, b3, true);                           \
    (b0set && b1set && b2set && b3set);                     \
})

bool bloom_create(bloom_t *filter, size_t size);

bool bloom_drop(bloom_t *filter);

bool bloom_clear(bloom_t *filter);

size_t bloom_nbits(bloom_t *filter);

unsigned bloom_nhashs();

JAK_END_DECL

#endif