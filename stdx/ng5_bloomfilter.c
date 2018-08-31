#include <stdx/ng5_bloomfilter.h>
#include <stdx/ng5_hash.h>

int ng5_bloomfilter_create(ng5_bloomfilter_t *filter, size_t size)
{
    return ng5_bitset_create(filter, size);
}

int ng5_bloomfilter_drop(ng5_bloomfilter_t *filter)
{
    return ng5_bitset_drop(filter);
}

int ng5_bloomfilter_clear(ng5_bloomfilter_t *filter)
{
    return ng5_bitset_clear(filter);
}

bool ng5_bloomfilter_test_and_set(ng5_bloomfilter_t *filter, const void *key, size_t key_size)
{
    size_t nbits = ng5_bitset_num_bits(filter);

    size_t b0 = hash_additive(key_size, key) % nbits;
    size_t b1 = hash__xor(key_size, key) % nbits;
    size_t b2 = hash_rot(key_size, key) % nbits;
    size_t b3 = hash_bernstein(key_size, key) % nbits;
    size_t b4 = hash_bernstein2(key_size, key) % nbits;
    size_t b5 = hash_sax(key_size, key) % nbits;
    size_t b6 = hash_fnv(key_size, key) % nbits;
    size_t b7 = hash_oat(key_size, key) % nbits;
    size_t b8 = hash_elf(key_size, key) % nbits;

    bool b0set = ng5_bitset_get(filter, b0);
    bool b1set = ng5_bitset_get(filter, b1);
    bool b2set = ng5_bitset_get(filter, b2);
    bool b3set = ng5_bitset_get(filter, b3);
    bool b4set = ng5_bitset_get(filter, b4);
    bool b5set = ng5_bitset_get(filter, b5);
    bool b6set = ng5_bitset_get(filter, b6);
    bool b7set = ng5_bitset_get(filter, b7);
    bool b8set = ng5_bitset_get(filter, b8);

    ng5_bitset_set(filter, b0, true);
    ng5_bitset_set(filter, b1, true);
    ng5_bitset_set(filter, b2, true);
    ng5_bitset_set(filter, b3, true);
    ng5_bitset_set(filter, b4, true);
    ng5_bitset_set(filter, b5, true);
    ng5_bitset_set(filter, b6, true);
    ng5_bitset_set(filter, b7, true);
    ng5_bitset_set(filter, b8, true);

    return b0set && b1set && b2set && b3set && b4set && b5set && b6set && b7set && b8set;
}


