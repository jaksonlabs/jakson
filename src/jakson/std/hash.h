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

#ifndef HASH_H
#define HASH_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/types.h>

BEGIN_DECL

typedef u8 hash8_t;
typedef u16 hash16_t;
typedef u32 hash32_t;
typedef u64 hash64_t;

#define HASH_ADDITIVE(key_size, key)                                                                               \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash += ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH_BERNSTEIN(key_size, key)        HASH_BERNSTEIN_WTYPE(key_size, key, hash32_t)
#define HASH8_BERNSTEIN(key_size, key)       HASH_BERNSTEIN_WTYPE(key_size, key, hash8_t)
#define HASH16_BERNSTEIN(key_size, key)      HASH_BERNSTEIN_WTYPE(key_size, key, hash16_t)
#define HASH64_BERNSTEIN(key_size, key)        HASH_BERNSTEIN_WTYPE(key_size, key, hash64_t)

#define HASH_BERNSTEIN_WTYPE(key_size, key, hash_type)                                                             \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = 0;                                                                                                \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH64_BERNSTEIN_WSEED(key_size, key, seed)                                                                \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash64_t hash = seed;                                                                                              \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash + ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH_BERNSTEIN2(key_size, key)                                                                             \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= 33 * hash ^ ((unsigned char* )key)[i];                                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH_ELF(key_size, key)                                                                                    \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0, g;                                                                                              \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash = (hash << 4) + ((unsigned char* )key)[i];                                                                \
        if ((g = hash & 0xf0000000L) != 0) {                                                                           \
            hash ^= g >> 24;                                                                                           \
        }                                                                                                              \
        hash &= ~g;                                                                                                    \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH8_FNV(key_size, key)             HASH_FNV_WTYPE(key_size, key, hash8_t)
#define HASH16_FNV(key_size, key)            HASH_FNV_WTYPE(key_size, key, hash16_t)
#define HASH_FNV(key_size, key)              HASH_FNV_WTYPE(key_size, key, hash32_t)
#define HASH64_FNV(key_size, key)              HASH_FNV_WTYPE(key_size, key, hash64_t)

#define HASH_FNV_WTYPE(key_size, key, hash_type)                                                                   \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash_type hash = (hash_type) 2166136261;                                                                           \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash = (hash * 16777619) ^ ((unsigned char* )key)[i];                                                          \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define JENKINS_MIX(a, b, c)                                                                                       \
{                                                                                                                      \
    a -= b; a -= c; a ^= (c >> 13);                                                                                    \
    b -= c; b -= a; b ^= (a << 8);                                                                                     \
    c -= a; c -= b; c ^= (b >> 13);                                                                                    \
    a -= b; a -= c; a ^= (c >> 12);                                                                                    \
    b -= c; b -= a; b ^= (a << 16);                                                                                    \
    c -= a; c -= b; c ^= (b >> 5);                                                                                     \
    a -= b; a -= c; a ^= (c >> 3);                                                                                     \
    b -= c; b -= a; b ^= (a << 10);                                                                                    \
    c -= a; c -= b; c ^= (b >> 15);                                                                                    \
}

/** implements: hash32_t hash_jenkins(size_t key_size, const void *key) */
#define HASH_JENKINS(keySizeIn, key)                                                                               \
({                                                                                                                     \
    size_t key_size = keySizeIn;                                                                                       \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    unsigned a, b;                                                                                                     \
    unsigned c = 0;                                                                                                    \
    unsigned char *k = (unsigned char *) key;                                                                          \
                                                                                                                       \
    a = b = 0x9e3779b9;                                                                                                \
                                                                                                                       \
    while (key_size >= 12) {                                                                                           \
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));                         \
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));                         \
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));                       \
        JENKINS_MIX(a, b, c);                                                                                      \
        k += 12;                                                                                                       \
        key_size -= 12;                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    c += key_size;                                                                                                     \
                                                                                                                       \
    switch (key_size) {                                                                                                \
        case 11: c += ((unsigned)k[10] << 24); break;                                                                  \
        case 10: c += ((unsigned)k[9] << 16); break;                                                                   \
        case 9: c += ((unsigned)k[8] << 8); break;                                                                     \
        case 8: b += ((unsigned)k[7] << 24); break;                                                                    \
        case 7: b += ((unsigned)k[6] << 16); break;                                                                    \
        case 6: b += ((unsigned)k[5] << 8); break;                                                                     \
        case 5: b += k[4]; break;                                                                                      \
        case 4: a += ((unsigned)k[3] << 24); break;                                                                    \
        case 3: a += ((unsigned)k[2] << 16); break;                                                                    \
        case 2: a += ((unsigned)k[1] << 8); break;                                                                     \
        case 1: a += k[0]; break;                                                                                      \
    }                                                                                                                  \
    JENKINS_MIX(a, b, c);                                                                                          \
    c;                                                                                                                 \
})

#define HASH_OAT(key_size, key)                                                                                    \
({                                                                                                                     \
    JAK_ASSERT ((key != NULL) && (key_size > 0));                                                                          \
                                                                                                                       \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash += ((unsigned char* )key)[i];                                                                             \
        hash += (hash << 10);                                                                                          \
        hash ^= (hash >> 6);                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    hash += (hash << 3);                                                                                               \
    hash ^= (hash >> 11);                                                                                              \
    hash += (hash << 15);                                                                                              \
                                                                                                                       \
    hash;                                                                                                              \
})

#define HASH_ROT(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= (hash << 4) ^ (hash >> 28) ^ ((unsigned char* )key)[i];                                                \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH_SAX(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= (hash << 5) + (hash >> 2) + ((unsigned char* )key)[i];                                                 \
    }                                                                                                                  \
    hash;                                                                                                              \
})

#define HASH_XOR(key_size, key)                                                                                    \
({                                                                                                                     \
    hash32_t hash = 0;                                                                                                 \
    for (size_t i = 0; i < key_size; i++) {                                                                            \
        hash ^= ((unsigned char* )key)[i];                                                                             \
    }                                                                                                                  \
    hash;                                                                                                              \
})

END_DECL

#endif