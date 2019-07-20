#ifndef CARBON_SELECTOR_RULE_BASE_H
#define CARBON_SELECTOR_RULE_BASE_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-hashmap.h>
#include <carbon/carbon-vector.h>
#include <carbon/compressor/auto/selector-common.h>

CARBON_BEGIN_DECL

typedef struct carbon_selector_rule {
    char *name;
    char *pattern;

    bool joinable;

    carbon_hashmap_t options;
} carbon_selector_rule_t;

typedef struct carbon_compressor_selector_rule_based_config {
    char * ruleset_filename;
    carbon_vec_t ofType(carbon_selector_rule_t *) * rules;
} carbon_compressor_selector_rule_based_config_t;

CARBON_EXPORT(carbon_vec_t ofType(carbon_selector_rule_t) *)
carbon_compressor_selector_rule_based_parse(
        char const *content
);

CARBON_EXPORT(void)
carbon_compressor_selector_rule_print(
        FILE *file, carbon_selector_rule_t *rule
);

CARBON_EXPORT(void)
carbon_compressor_selector_rule_based_drop(
        carbon_vec_t ofType(carbon_selector_rule_t) *rules
);

CARBON_EXPORT(void)
carbon_compressor_selector_rule_based_config_init(
    carbon_compressor_selector_rule_based_config_t *config
);

carbon_compressor_selector_result_t carbon_compressor_find_by_strings_rule_based(
    carbon_vec_t ofType(char *) *strings,
    carbon_doc_bulk_t const *context,
    carbon_compressor_selector_rule_based_config_t *config,
    carbon_compressor_selector_sampling_config_t const sampling_config
);

CARBON_END_DECL

#endif // CARBON_SELECTOR_RULE_BASE_H
