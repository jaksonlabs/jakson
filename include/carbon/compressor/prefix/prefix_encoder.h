#ifndef LIBCARBON_PREFIX_ENCODER_H
#define LIBCARBON_PREFIX_ENCODER_H

#include <glob.h>

typedef struct carbon_prefix_encoder_config_t {
    size_t sampling_step_width;
    size_t num_iterations_between_prunes;
    size_t prune_min_support;
    size_t max_new_children_per_entry;
} carbon_prefix_encoder_config;

carbon_prefix_encoder_config * carbon_prefix_encoder_auto_config(size_t num_strings, double load_factor);

#endif
