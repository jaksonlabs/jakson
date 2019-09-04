#include <carbon/compressor/auto/selector-common.h>

carbon_vec_t ofType(char *) *carbon_compressor_selector_sample_strings(
        carbon_vec_t ofType(char *) *strings,
        carbon_compressor_selector_sampling_config_t const *config
    ) {
    size_t const block_cnt   =
            strings->num_elems <= config->min_entries ?
                config->block_count :
                (size_t)(strings->num_elems * config->block_count / (double)config->min_entries);

    carbon_vec_t ofType(char *) *samples = malloc(sizeof(carbon_vec_t));
    carbon_vec_create(
                samples, NULL, sizeof(char *), config->block_length * block_cnt
    );

    if(!config->enabled || strings->num_elems <= config->min_entries) {
        // Copy all strings
        carbon_vec_push(samples, carbon_vec_at(strings, 0), strings->num_elems);
    } else {
        carbon_hashmap_t unique_strings = carbon_hashmap_new();

        // Sample strings without duplicates
        carbon_hashmap_any_t any;
        while (carbon_hashmap_length(unique_strings) < block_cnt * config->block_length) {
            size_t const start_idx =
                    (size_t)((size_t)rand() % (strings->num_elems - config->block_length));

            for(size_t j = 0; j < config->block_length; ++j) {
                char const * string = *(char const * const *)carbon_vec_at(strings, start_idx + j);
                if(carbon_hashmap_get(unique_strings, string, &any) == carbon_hashmap_status_missing) {
                    carbon_hashmap_put(unique_strings, string, unique_strings);
                }
            }
        }

        for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(unique_strings);it.valid;carbon_hashmap_next(&it)) {
            char const * current_key = strdup(it.key);
            carbon_vec_push(samples, &current_key, 1);
        }

        carbon_hashmap_drop(unique_strings);
    }

    return samples;
}

void carbon_compressor_selector_sampling_config_init(
        carbon_compressor_selector_sampling_config_t *config
    )
{
    config->enabled = true;
    config->block_count = 16;
    config->block_length = 32;
    config->min_entries = 1000;
}


void carbon_compressor_selector_apply_highlevel_config(
        carbon_compressor_t *compressor,
        carbon_compressor_highlevel_config_t *config
    )
{
    carbon_err_t err;
    char tmp_buffer[16];

    carbon_compressor_set_option(
        &err, compressor, "prefix",
        config->prefix == carbon_compressor_configurable_prefix_type_incremental ? "incremental" :
                        (config->prefix == carbon_compressor_configurable_prefix_type_prefix_dict_coding ? "prefix-dict" : "none")
    );
    carbon_compressor_set_option(
        &err, compressor, "suffix",
        config->suffix == carbon_compressor_configurable_prefix_type_incremental ? "incremental" : "none"
    );
    carbon_compressor_set_option(
        &err, compressor, "huffman", config->huffman ? "true" : "false"
    );

    carbon_compressor_set_option(
        &err, compressor, "reverse_strings", config->reverse_strings ? "true" : "false"
    );

    carbon_compressor_set_option(
        &err, compressor, "reverse_sort", config->reverse_sort ? "true" : "false"
    );

    snprintf(tmp_buffer, 15, "%lu", 100l);
    carbon_compressor_set_option(
        &err, compressor, "delta_chunk_length", tmp_buffer
    );
    snprintf(tmp_buffer, 15, "%lu", 1000000l);
    carbon_compressor_set_option(
        &err, compressor, "sort_chunk_length", tmp_buffer
    );

}
