#include <malloc.h>
#include <math.h>
#include <carbon/compressor/prefix/prefix_encoder.h>

#define DEFAULT_NUM_SAMPLES 100000

int max(int a, int b) {
    return a > b ? a : b;
}

carbon_prefix_encoder_config * carbon_prefix_encoder_auto_config(
    size_t num_strings, double load_factor
) {
    carbon_prefix_encoder_config * cfg = malloc(sizeof(carbon_prefix_encoder_config));

    size_t num_samples = (size_t) (DEFAULT_NUM_SAMPLES * load_factor);

    if(num_strings < num_samples) {
        // Sample everything
        cfg->sampling_step_width = 1;
        cfg->max_new_children_per_entry = (size_t) max(1, (int) (2 * sqrt(num_samples / num_strings) * load_factor));
        cfg->prune_min_support = (size_t) max(2, (int) (10.0 / (load_factor + sqrt(num_samples / num_strings))));
    } else {
        cfg->sampling_step_width = num_strings / num_samples;
        cfg->max_new_children_per_entry = (size_t) max(1, (int) (1 * load_factor));
        cfg->prune_min_support = (size_t) max(2, (int) (10.0 / (2 * load_factor)));
    }

    cfg->num_iterations_between_prunes = (size_t) (load_factor * DEFAULT_NUM_SAMPLES / 4);

    return cfg;
}
