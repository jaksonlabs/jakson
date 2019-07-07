#include <carbon/compressor/auto/selector-brute-force.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

#include <carbon/carbon-memfile.h>
#include <carbon/carbon-memblock.h>


// Avoid writing out these every time...
static carbon_compressor_incremental_prefix_type_e const none = carbon_compressor_incremental_prefix_type_none;
static carbon_compressor_incremental_prefix_type_e const inc = carbon_compressor_incremental_prefix_type_incremental;
static carbon_compressor_incremental_prefix_type_e const table = carbon_compressor_incremental_prefix_type_table;


// Forward decl of helpers
static carbon_compressor_t *
compressor_for(carbon_compressor_highlevel_config_t config, carbon_doc_bulk_t const *context);

static carbon_vec_t ofType(char *) *
sample_strings(carbon_vec_t ofType(char *) *strings, carbon_compressor_selector_brute_force_config_t const *config);

static size_t
compute_size(carbon_compressor_t *compressor, carbon_vec_t ofType(char *) *strings);

static void
sort_by_string(carbon_vec_t * entries, size_t idx_offset, size_t length, bool forward);

static char const *
get_strvec_str(carbon_vec_t * entries, size_t idx);



carbon_compressor_selector_result_t carbon_compressor_find_by_strings_brute_force(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context,
        carbon_compressor_selector_brute_force_config_t const config
    ) {
    carbon_err_t err;

    carbon_compressor_highlevel_config_t configurations[] = {
        { .prefix = none,  .suffix = none,  .reverse = false, .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse = false, .huffman = true  },
        { .prefix = none,  .suffix = none,  .reverse = true,  .huffman = false },
        { .prefix = none,  .suffix = none,  .reverse = true,  .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse = false, .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse = false, .huffman = true  },
        { .prefix = none,  .suffix = inc,   .reverse = true,  .huffman = false },
        { .prefix = none,  .suffix = inc,   .reverse = true,  .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse = false, .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse = false, .huffman = true  },
        { .prefix = inc,   .suffix = none,  .reverse = true,  .huffman = false },
        { .prefix = inc,   .suffix = none,  .reverse = true,  .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse = false, .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse = false, .huffman = true  },
        { .prefix = inc,   .suffix = inc,   .reverse = true,  .huffman = false },
        { .prefix = inc,   .suffix = inc,   .reverse = true,  .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse = false, .huffman = false },
        { .prefix = table, .suffix = none,  .reverse = false, .huffman = true  },
        { .prefix = table, .suffix = none,  .reverse = true,  .huffman = false },
        { .prefix = table, .suffix = none,  .reverse = true,  .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse = false, .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse = false, .huffman = true  },
        { .prefix = table, .suffix = inc,   .reverse = true,  .huffman = false },
        { .prefix = table, .suffix = inc,   .reverse = true,  .huffman = true  }
    };

    carbon_vec_t ofType(char *) *samples = sample_strings(strings, &config);

    carbon_compressor_selector_result_t result = { .estimated_size = SIZE_MAX };
    carbon_huffman_encoder_create(&result.huffman);

    for(size_t i = 0; i < sizeof(configurations)/sizeof(configurations[0]);++i) {
        carbon_compressor_t * compressor = compressor_for(configurations[i], context);

        size_t const size = compute_size(compressor, samples);
        if(size < result.estimated_size) {

            result.estimated_size = size;
            result.config         = configurations[i];

            if(result.config.huffman) {
                carbon_compressor_incremental_extra_t *extra =
                        (carbon_compressor_incremental_extra_t *)compressor->extra;

                carbon_huffman_encoder_drop(&result.huffman);
                carbon_huffman_encoder_create(&result.huffman);
                carbon_huffman_encoder_clone(&extra->huffman_encoder, &result.huffman);
            }
        }

        carbon_compressor_drop(&err, compressor);
    }

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s, suffix = %s, huffman = %s, rev_str = %s",
                (result.config.prefix == inc ? "incremental" : ( result.config.prefix == table ? "table" : "none" )),
                (result.config.suffix == inc ? "incremental" : ( result.config.suffix == table ? "table" : "none" )),
                result.config.huffman ? "true" : "false",
                result.config.reverse ? "true" : "false"
    );

    carbon_vec_drop(samples);
    free(samples);

    result.compressor = compressor_for(result.config, context);
    return result;
}

static carbon_vec_t ofType(char *) *sample_strings(
        carbon_vec_t ofType(char *) *strings,
        carbon_compressor_selector_brute_force_config_t const *config
    ) {
    size_t const block_cnt   =
            strings->num_elems <= config->sampling_min_entries ?
                config->sampling_block_count :
                (size_t)(strings->num_elems * config->sampling_block_count / (double)config->sampling_min_entries);

    carbon_vec_t ofType(char *) *samples = malloc(sizeof(carbon_vec_t));
    carbon_vec_create(
                samples, NULL, sizeof(char *), config->sampling_block_length * block_cnt
    );

    if(!config->sampling_enabled || strings->num_elems <= config->sampling_min_entries) {
        // Copy all strings
        carbon_vec_push(samples, carbon_vec_at(strings, 0), strings->num_elems);
    } else {
        // Sample strings
        for(size_t i = 0; i < block_cnt; ++i) {
            size_t const start_idx =
                    (size_t)((size_t)rand() % (strings->num_elems - config->sampling_block_length));

            carbon_vec_push(
                        samples, carbon_vec_at(strings, start_idx), config->sampling_block_length
            );
        }
    }

    return samples;
}


static carbon_compressor_t *compressor_for(
        carbon_compressor_highlevel_config_t config,
        carbon_doc_bulk_t const *context
    ) {
    carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));

    char tmp_buffer[16];

    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_INCREMENTAL);

    ////// Apply the provided compressor_config_t to the compressor
    carbon_compressor_set_option(
        &err, compressor, "prefix",
        config.prefix == inc ? "incremental" : (config.prefix == table ? "table" : "none")
    );
    carbon_compressor_set_option(
        &err, compressor, "suffix",
        config.suffix == inc ? "incremental" : "none"
    );
    carbon_compressor_set_option(
        &err, compressor, "huffman", config.huffman ? "true" : "false"
    );

    carbon_compressor_set_option(
        &err, compressor, "reverse_strings", config.reverse ? "true" : "false"
    );

    carbon_compressor_set_option(
        &err, compressor, "reverse_sort", config.reverse ? "true" : "false"
    );

    snprintf(tmp_buffer, 15, "%lu", 100l);
    carbon_compressor_set_option(
        &err, compressor, "delta_chunk_length", tmp_buffer
    );
    snprintf(tmp_buffer, 15, "%lu", 1000000l);
    carbon_compressor_set_option(
        &err, compressor, "sort_chunk_length", tmp_buffer
    );

    return compressor;
}

static size_t compute_size(
        carbon_compressor_t *compressor,
        carbon_vec_t ofType(char *) *strings
    ) {
    carbon_err_t       err;
    carbon_memfile_t   memfile;
    carbon_memblock_t *memblock;
    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)compressor->extra;

    carbon_memblock_create(&memblock, 1024 * 1024);
    carbon_memfile_open(&memfile, memblock, CARBON_MEMFILE_MODE_READWRITE);

    carbon_compressor_incremental_prepare_and_analyze(
        strings, &extra->config,
        &extra->prefix_tbl_table, &extra->prefix_tbl_encoder,
        &extra->huffman_encoder,
        sort_by_string, get_strvec_str
    );
    carbon_compressor_write_extra(
        &err, compressor, &memfile, strings
    );


    for(size_t i = 0; i < strings->num_elems; ++i) {
        carbon_compressor_encode(
            &err, compressor, &memfile, *(char * const *)carbon_vec_at(strings, i), 0
        );
    }

    size_t num_bytes = CARBON_MEMFILE_TELL(&memfile);
    carbon_memblock_drop(memblock);

    return num_bytes;
}

static void sort_by_string(
        carbon_vec_t * entries,
        size_t idx_offset,
        size_t length,
        bool forward
    ) {
    qsort(
        (char const **)entries->base + idx_offset,
        length, sizeof(char const *),
        forward ? carbon_sort_cmp_fwd : carbon_sort_cmp_rwd
    );
}

static char const * get_strvec_str(
        carbon_vec_t * entries,
        size_t index
    ) {
    return *(char const * const *)carbon_vec_at(entries, index);
}

void carbon_compressor_selector_brute_force_config_init(
        carbon_compressor_selector_brute_force_config_t *config
    )
{
    config->sampling_enabled = true;
    config->sampling_block_count = 16;
    config->sampling_block_length = 32;
    config->sampling_min_entries = 1000;
}
