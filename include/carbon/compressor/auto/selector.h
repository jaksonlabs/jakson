#ifndef CARBON_COMPRESSOR_AUTO_SELECTOR_H
#define CARBON_COMPRESSOR_AUTO_SELECTOR_H

#include <carbon/carbon-compressor.h>

carbon_compressor_t *carbon_compressor_find_by_strings(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context
);

#endif
