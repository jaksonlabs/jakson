#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_COST_BASED_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_COST_BASED_H

#include <carbon/carbon-compressor.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

typedef struct {
    double avg_pre_pre_len;
    double avg_pre_suf_len;
    double avg_suf_pre_len;
    double avg_suf_suf_len;

    size_t prefix_saving;
    size_t suffix_saving;

    size_t prefix_tbl_len;
    size_t suffix_tbl_len;

    double huffman_average_bit_len;
    size_t huffman_estimated_table_size;

    size_t remaining_text_length;
    size_t num_entries;

    carbon_huffman_encoder_t huffman_encoder;
    carbon_compressor_incremental_config_t compressor_config;
} stringset_properties_t;

carbon_compressor_t *carbon_compressor_find_by_strings_cost_based(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context
);

#endif
