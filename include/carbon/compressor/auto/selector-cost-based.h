#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_COST_BASED_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_COST_BASED_H

#include <carbon/compressor/auto/selector-common.h>

carbon_compressor_selector_result_t carbon_compressor_find_by_strings_cost_based(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context,
    carbon_compressor_selector_sampling_config_t const sampling_config
);

#endif
