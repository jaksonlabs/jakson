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

#include <jakson/jak_stdinc.h>
#include <jakson/std/jak_bitmap.h>
#include <jakson/std/jak_hash.h>

JAK_BEGIN_DECL

#define JAK_BLOOM_SET(filter, key, key_size)                                                                           \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;                                                                   \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;                                                                   \
    jak_bitmap_set(filter, b0, true);                                                                                  \
    jak_bitmap_set(filter, b1, true);                                                                                  \
    jak_bitmap_set(filter, b2, true);                                                                                  \
    jak_bitmap_set(filter, b3, true);                                                                                  \
})

#define JAK_BLOOM_TEST(filter, key, key_size)                                                                          \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;                                                                   \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;                                                                   \
    bool b0set = jak_bitmap_get(filter, b0);                                                                           \
    bool b1set = jak_bitmap_get(filter, b1);                                                                           \
    bool b2set = jak_bitmap_get(filter, b2);                                                                           \
    bool b3set = jak_bitmap_get(filter, b3);                                                                           \
    (b0set && b1set && b2set && b3set);                                                                                \
})

#define JAK_BLOOM_TEST_AND_SET(filter, key, key_size)                                                                  \
({                                                                                                                     \
    size_t nbits = jak_bitmap_nbits(filter);                                                                           \
    size_t b0 = JAK_HASH_ADDITIVE(key_size, key) % nbits;                                                              \
    size_t b1 = JAK_HASH_XOR(key_size, key) % nbits;                                                                   \
    size_t b2 = JAK_HASH_ROT(key_size, key) % nbits;         \
    size_t b3 = JAK_HASH_SAX(key_size, key) % nbits;         \
    bool b0set = jak_bitmap_get(filter, b0);                    \
    bool b1set = jak_bitmap_get(filter, b1);                    \
    bool b2set = jak_bitmap_get(filter, b2);                    \
    bool b3set = jak_bitmap_get(filter, b3);                    \
    jak_bitmap_set(filter, b0, true);                           \
    jak_bitmap_set(filter, b1, true);                           \
    jak_bitmap_set(filter, b2, true);                           \
    jak_bitmap_set(filter, b3, true);                           \
    (b0set && b1set && b2set && b3set);                     \
})

bool jak_bloom_create(jak_bitmap *filter, size_t size);
bool jak_bloom_drop(jak_bitmap *filter);
bool jak_bloom_clear(jak_bitmap *filter);
size_t jak_bloom_nbits(jak_bitmap *filter);

unsigned jak_bloom_nhashs();

JAK_END_DECL

#endif