#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_BRUTE_FORCE_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_BRUTE_FORCE_H

#include <carbon/carbon-compressor.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

carbon_compressor_t *carbon_compressor_find_by_strings_brute_force(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context
);

#endif
