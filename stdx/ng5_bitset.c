#include <stdx/ng5_bitset.h>

#define NUM_BITS(x)             (sizeof(x) * 8)
#define SET_BIT(n)              ( ((uint64_t) 1) << (n) )
#define FIELD_SET(x, mask)      ( x |=  (mask) )
#define FIELD_CLEAR(x, mask)    ( x &= ~(mask) )
#define FLIP(x)                 ( ~x )
#define FIELD_AND(x, y)         ( x & y )

int ng5_bitset_create(ng5_bitset_t *bitset, size_t num_bits)
{
    check_non_null(bitset);

    ng5_allocator_t alloc;
    allocator_default(&alloc);
    ng5_vector_create(&bitset->data, &alloc, sizeof(uint64_t), ceil(num_bits / (double) NUM_BITS(uint64_t)));
    size_t cap = ng5_vector_cap(&bitset->data);
    uint64_t zero = 0;
    ng5_vector_repreat_push(&bitset->data, &zero, cap);
    bitset->num_bits = num_bits;

    return STATUS_OK;
}

int ng5_bitset_drop(ng5_bitset_t *bitset)
{
    return ng5_vector_drop(&bitset->data);
}

size_t ng5_bitset_num_bits(const ng5_bitset_t *bitset)
{
    check_non_null(bitset);
    return bitset->num_bits;
}

int ng5_bitset_clear(ng5_bitset_t *bitset)
{
    check_non_null(bitset);
    void *data = (void *) ng5_vector_data(&bitset->data);
    memset(data, 0, sizeof(uint64_t) * ng5_vector_cap(&bitset->data));
    return STATUS_OK;
}

int ng5_bitset_set(ng5_bitset_t *bitset, size_t bit_pos, bool on)
{
    check_non_null(bitset)
    size_t   block_pos = floor(bit_pos / (double) NUM_BITS(uint64_t));
    size_t   block_bit = bit_pos % NUM_BITS(uint64_t);
    uint64_t block     = *ng5_vector_get(&bitset->data, block_pos, uint64_t);
    uint64_t mask      = SET_BIT(block_bit);
    if (on) {
        FIELD_SET(block, mask);
    } else {
        FIELD_CLEAR(block, mask);
    }
    ng5_vector_set(&bitset->data, block_pos, &block);
    return STATUS_OK;
}

bool ng5_bitset_get(ng5_bitset_t *bitset, size_t bit_pos)
{
    check_non_null(bitset)
    size_t   block_pos = floor(bit_pos / (double) NUM_BITS(uint64_t));
    size_t   block_bit = bit_pos % NUM_BITS(uint64_t);
    uint64_t block     = *ng5_vector_get(&bitset->data, block_pos, uint64_t);
    uint64_t mask      = SET_BIT(block_bit);
    return ((mask & block) >> bit_pos)  == true;
}