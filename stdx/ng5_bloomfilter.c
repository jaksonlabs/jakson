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

size_t ng5_bloomfilter_nbits(ng5_bloomfilter_t *filter)
{
    return ng5_bitset_num_bits(filter);
}

unsigned ng5_bloomfilter_nhashes()
{
    return 4;
}



