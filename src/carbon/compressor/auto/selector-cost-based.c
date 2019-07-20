#include <carbon/compressor/auto/selector-cost-based.h>
#include <carbon/compressor/auto/common-prefix-suffix-iterator.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-incremental.h>
#include <carbon/compressor/huffman/priority-queue.h>
#include <carbon/compressor/prefix/prefix_encoder.h>

#include <math.h>

// Avoid writing out these every time...
static carbon_compressor_incremental_prefix_type_e const none = carbon_compressor_incremental_prefix_type_none;
static carbon_compressor_incremental_prefix_type_e const inc = carbon_compressor_incremental_prefix_type_incremental;
static carbon_compressor_incremental_prefix_type_e const table = carbon_compressor_incremental_prefix_type_table;


typedef struct cost_model_input {
    size_t huffman_savings;
    size_t sample_entry_count;
    size_t total_entry_count;

    double avg_prefix_prefix_len;
    double avg_prefix_suffix_len;
    double avg_suffix_prefix_len;
    double avg_suffix_suffix_len;
} cost_model_input_t;


static double
selector_average_common_length(
        carbon_vec_t *samples,
        carbon_common_prefix_suffix_it_comp_dir_e char_cmp,
        carbon_common_prefix_suffix_it_sort_dir_e sort_cmp
);

static size_t
selector_estimate_huffman_table_size(
        carbon_huffman_encoder_t *encoder
);

static size_t
selector_estimate_huffman_savings(
        carbon_huffman_encoder_t *encoder,
        size_t sample_entry_count,
        size_t total_entry_count
);

static void
selector_estimate_huffman_encoder(
        carbon_vec_t ofType(char *) *samples,
        carbon_huffman_encoder_t *encoder
);

static ssize_t
selector_evluate_model(
        cost_model_input_t *input,
        carbon_compressor_highlevel_config_t *config
);

carbon_compressor_selector_result_t carbon_compressor_find_by_strings_cost_based(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context,
        carbon_compressor_selector_sampling_config_t const sampling_config
    ) {
    CARBON_UNUSED(strings);
    CARBON_UNUSED(context);

    carbon_err_t err;

    carbon_vec_t *samples =
            carbon_compressor_selector_sample_strings(strings, &sampling_config);

    ssize_t estd_savings = 0;

    carbon_compressor_selector_result_t result;
    carbon_huffman_encoder_t huffman_encoder;
    selector_estimate_huffman_encoder(samples, &huffman_encoder);

    cost_model_input_t input = {
        .huffman_savings = selector_estimate_huffman_savings(&huffman_encoder, samples->num_elems, strings->num_elems),
        .sample_entry_count = samples->num_elems,
        .total_entry_count = strings->num_elems,
        .avg_prefix_prefix_len = selector_average_common_length(samples, carbon_cps_comp_from_start, carbon_cps_sort_from_start),
        .avg_prefix_suffix_len = selector_average_common_length(samples, carbon_cps_comp_from_end, carbon_cps_sort_from_start),
        .avg_suffix_prefix_len = selector_average_common_length(samples, carbon_cps_comp_from_start, carbon_cps_sort_from_end),
        .avg_suffix_suffix_len = selector_average_common_length(samples, carbon_cps_comp_from_end, carbon_cps_sort_from_end)
    };


    CARBON_CONSOLE_WRITELN(
        stdout, "            hufsav: %zu, avg_pre_pre: %.3f, avg_pre_suf: %.3f, avg_suf_pre: %.3f, avg_suf_suf: %.3f",
        input.huffman_savings,
        input.avg_prefix_prefix_len, input.avg_prefix_suffix_len,
        input.avg_suffix_prefix_len, input.avg_suffix_suffix_len
    );

    carbon_compressor_highlevel_config_t configurations[] = {
        { .prefix = none,  .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = none,  .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = table, .suffix = none,  .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = table, .suffix = none,  .reverse_strings = true,  .reverse_sort = false, .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse_strings = false, .reverse_sort = false, .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse_strings = true,  .reverse_sort = false, .huffman = true  },

        { .prefix = none,  .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = none,  .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = table, .suffix = none,  .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = table, .suffix = none,  .reverse_strings = true,  .reverse_sort = true,  .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse_strings = false, .reverse_sort = true,  .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse_strings = true,  .reverse_sort = true,  .huffman = true  }
    };

    for(size_t i = 0; i < sizeof(configurations)/sizeof(configurations[0]);++i) {
        ssize_t const size = selector_evluate_model(&input, &configurations[i]);
        if(size > estd_savings) {
            estd_savings  = size;
            result.config = configurations[i];
        }
    }

    result.joinable_group = 0;
    result.compressor     = malloc(sizeof(carbon_compressor_t));

    carbon_compressor_by_type(&err, result.compressor, context, CARBON_COMPRESSOR_INCREMENTAL);
    carbon_compressor_selector_apply_highlevel_config(result.compressor, &result.config);

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s, suffix = %s, huffman = %s, rev_str = %s, rev_sort = %s",
                (result.config.prefix == inc ? "incremental" : ( result.config.prefix == table ? "table" : "none" )),
                (result.config.suffix == inc ? "incremental" : ( result.config.suffix == table ? "table" : "none" )),
                result.config.huffman ?         "true" : "false",
                result.config.reverse_strings ? "true" : "false",
                result.config.reverse_sort ?    "true" : "false"
    );

    carbon_vec_drop(samples);
    free(samples);

    carbon_huffman_encoder_drop(&huffman_encoder);
    return result;
}


static double selector_average_common_length(
        carbon_vec_t ofType(char *) *samples,
        carbon_common_prefix_suffix_it_comp_dir_e comp,
        carbon_common_prefix_suffix_it_sort_dir_e sort
    ) {
    if(samples->num_elems <= 1)
        return 0;

    size_t prefix_length_sum = 0;
    for(carbon_cps_it it = carbon_cps_begin(samples, sort, comp);it.valid;carbon_cps_next(&it)) {
        prefix_length_sum += it.common_length;
    }

    return (double)prefix_length_sum / samples->num_elems;
}

static size_t selector_estimate_huffman_savings(
        carbon_huffman_encoder_t *encoder,
        size_t sample_entry_count,
        size_t total_entry_count
    ) {
    double abl = carbon_huffman_avg_bit_length(encoder, encoder->frequencies);

    size_t total_symbol_count = 0;
    for(size_t i = 0; i < UCHAR_MAX + 1; ++i) {
        total_symbol_count += encoder->frequencies[ i ];
    }

    // Scale the number of entries up to represent the original number of symbols
    total_symbol_count *= (double)total_entry_count / (double)sample_entry_count;

    ssize_t bytes_saved =
            (ssize_t)((8.0 - abl) / 8.0 * total_symbol_count) -
            (ssize_t)selector_estimate_huffman_table_size(encoder);
    return bytes_saved < 0 ? 0 : (size_t)bytes_saved;
}

static void selector_estimate_huffman_encoder(
        carbon_vec_t ofType(char *) *samples,
        carbon_huffman_encoder_t *encoder
    ) {    
    typedef struct { size_t prefix; size_t suffix; } local_hashmap_entry_t;

    carbon_huffman_encoder_create(encoder);

    // Avoid a lot of small size allocations -> allocate once
    local_hashmap_entry_t * prefix_suffix_mem = malloc(sizeof(local_hashmap_entry_t) * samples->num_elems);

    carbon_hashmap_t map = carbon_hashmap_new();
    for(carbon_cps_it it = carbon_cps_begin(samples, carbon_cps_sort_from_start, carbon_cps_comp_from_start);it.valid;carbon_cps_next(&it)) {
        local_hashmap_entry_t *entry = prefix_suffix_mem + it.index;
        entry->prefix = it.common_length;

        carbon_hashmap_put(map, it.current, entry);
    }

    for(carbon_cps_it it = carbon_cps_begin(samples, carbon_cps_sort_from_end, carbon_cps_comp_from_end);it.valid;carbon_cps_next(&it)) {
        local_hashmap_entry_t * entry;
        carbon_hashmap_get(map, it.current, (carbon_hashmap_any_t)&entry);
        entry->suffix = it.common_length;
    }

    carbon_huffman_encoder_create(encoder);
    for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(map);it.valid;carbon_hashmap_next(&it)) {
        local_hashmap_entry_t * entry = (local_hashmap_entry_t *)it.value;

        size_t string_length = strlen(it.key);
        if(string_length > entry->prefix + entry->suffix)
            carbon_huffman_encoder_learn_frequencies(encoder, it.key + entry->prefix, string_length - entry->prefix - entry->suffix);
    }

    carbon_huffman_encoder_bake_codes(encoder);
}

static ssize_t selector_evluate_model(
        cost_model_input_t *input,
        carbon_compressor_highlevel_config_t *config
    ) {
    CARBON_UNUSED(input);
    CARBON_UNUSED(config);

    ssize_t savings = 0;

    switch(config->prefix) {
    case carbon_compressor_incremental_prefix_type_incremental:
    {
        if(config->reverse_strings)
            savings += ((config->reverse_sort ? input->avg_suffix_suffix_len : input->avg_prefix_suffix_len) - 1.0) * (ssize_t)input->total_entry_count;
        else
            savings += ((config->reverse_sort ? input->avg_suffix_prefix_len : input->avg_prefix_prefix_len) - 1.0) * (ssize_t)input->total_entry_count;

        break;
    }
    case carbon_compressor_incremental_prefix_type_table:
    {
        if(config->reverse_strings)
            savings += (input->avg_suffix_suffix_len - 2.0) * (ssize_t)input->total_entry_count;
        else
            savings += (input->avg_prefix_prefix_len - 2.0) * (ssize_t)input->total_entry_count;

        break;
    }
    default:
        break;
    }



    switch(config->suffix) {
    case carbon_compressor_incremental_prefix_type_incremental:
    {
        if(config->reverse_strings)
            savings += ((config->reverse_sort ? input->avg_prefix_prefix_len : input->avg_suffix_prefix_len) - 1.0) * (ssize_t)input->total_entry_count;
        else
            savings += ((config->reverse_sort ? input->avg_prefix_suffix_len : input->avg_suffix_suffix_len) - 1.0) * (ssize_t)input->total_entry_count;

        break;
    }
    default:
        break;
    }

    if(config->huffman)
        savings += input->huffman_savings;

    return savings;
}

static size_t selector_estimate_huffman_table_size(
        carbon_huffman_encoder_t *encoder
    ) {
    size_t total_size = 0;

    size_t num_entries = 0;
    size_t bit_lengths[UCHAR_MAX + 1];
    for(size_t i = 0; i < UCHAR_MAX + 1; ++i) bit_lengths[i] = 0;

    for(size_t i = 0; i < UCHAR_MAX + 1; ++i) {
        if(encoder->frequencies[i] > 0) {
            bit_lengths[encoder->codes[i].num_bits]++;
            num_entries++;
        }
    }

    total_size += carbon_vlq_encoded_length(num_entries);
    for(size_t i = 0; i < UCHAR_MAX + 1; ++i) {
        if(bit_lengths[i] == 0)
            continue;

        total_size += 2 + ceil((bit_lengths[i] * (i + 8)) / 8.0);
    }

    return total_size;
}
