#ifndef CARBON_COMPRESSOR_UTILS_H
#define CARBON_COMPRESSOR_UTILS_H

#include <glob.h>
#include <stdint.h>

#include <carbon/carbon-memfile.h>

size_t carbon_vlq_encode(size_t length, uint8_t *buffer);
size_t carbon_vlq_decode(uint8_t *buffer, size_t *num_bytes);

void carbon_vlq_encode_to_file(size_t length, carbon_memfile_t *dst);
size_t carbon_vlq_decode_from_file(FILE *src, bool *ok);

#endif
