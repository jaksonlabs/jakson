#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_BRUTE_FORCE_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_BRUTE_FORCE_H

#include <carbon/compressor/auto/selector-common.h>

typedef struct {
} carbon_compressor_selector_brute_force_config_t;

void carbon_compressor_selector_brute_force_config_init(
    carbon_compressor_selector_brute_force_config_t *config
);

carbon_compressor_selector_result_t carbon_compressor_find_by_strings_brute_force(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context,
    carbon_compressor_selector_brute_force_config_t const config,
    carbon_compressor_selector_sampling_config_t const sampling_config
);

#endif
