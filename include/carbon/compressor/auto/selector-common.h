#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_COMMON_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_COMMON_H

#include <carbon/carbon-compressor.h>
#include <carbon/compressor/carbon-compressor-configurable.h>

typedef struct carbon_compressor_highlevel_config {
    carbon_compressor_configurable_prefix_type_e prefix;
    carbon_compressor_configurable_prefix_type_e suffix;
    bool reverse_strings;
    bool reverse_sort;
    bool huffman;
} carbon_compressor_highlevel_config_t;


typedef struct carbon_compressor_selector_result {
    size_t                               estimated_size;
    carbon_huffman_encoder_t             huffman;
    carbon_compressor_highlevel_config_t config;
    carbon_compressor_t *                compressor;
    size_t                               joinable_group;
} carbon_compressor_selector_result_t;

typedef struct carbon_compressor_selector_sampling_config {
    bool   enabled;
    size_t block_count;
    size_t block_length;
    size_t min_entries;
} carbon_compressor_selector_sampling_config_t;

void carbon_compressor_selector_sampling_config_init(
    carbon_compressor_selector_sampling_config_t *config
);

carbon_vec_t ofType(char *) *carbon_compressor_selector_sample_strings(
    carbon_vec_t ofType(char *) *strings,
    carbon_compressor_selector_sampling_config_t const *config
);

void carbon_compressor_selector_apply_highlevel_config(
    carbon_compressor_t *compressor,
    carbon_compressor_highlevel_config_t *config
);

#endif
