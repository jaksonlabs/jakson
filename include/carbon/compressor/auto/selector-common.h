#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_COMMON_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_COMMON_H

#include <carbon/carbon-compressor.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

typedef struct carbon_compressor_highlevel_config {
    carbon_compressor_incremental_prefix_type_e prefix;
    carbon_compressor_incremental_prefix_type_e suffix;
    bool reverse;
    bool huffman;
} carbon_compressor_highlevel_config_t;


typedef struct carbon_compressor_selector_result {
    size_t                               estimated_size;
    carbon_huffman_encoder_t             huffman;
    carbon_compressor_highlevel_config_t config;
    carbon_compressor_t *                compressor;
} carbon_compressor_selector_result_t;

#endif
