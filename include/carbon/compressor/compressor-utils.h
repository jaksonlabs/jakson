#ifndef CARBON_COMPRESSOR_UTILS_H
#define CARBON_COMPRESSOR_UTILS_H

#include <glob.h>
#include <stdint.h>

#include <carbon/carbon-vector.h>
#include <carbon/carbon-memfile.h>

struct carbon_io_device;
typedef struct carbon_io_device carbon_io_device_t;

size_t carbon_vlq_encode(size_t length, uint8_t *buffer);
size_t carbon_vlq_decode(uint8_t const *buffer, size_t *num_bytes);

size_t carbon_vlq_encoded_length(size_t length);

void carbon_vlq_encode_to_io(size_t length, carbon_io_device_t *dst);
size_t carbon_vlq_decode_from_io(carbon_io_device_t *src, bool *ok);

void carbon_str_reverse(char * str);

int carbon_sort_cmp_fwd(void const *a, void const *b);
int carbon_sort_cmp_rwd(void const *a, void const *b);

size_t carbon_str_vec_total_length(carbon_vec_t ofType(char *) *vec);

char const *carbon_remove_common_prefix_and_suffix(
    char const *current, size_t current_length,
    char const *previous, size_t previous_length,
    size_t *length, bool prefix, bool suffix
);

#endif
