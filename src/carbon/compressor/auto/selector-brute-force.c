#include <carbon/compressor/auto/selector-brute-force.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-configurable.h>

#include <carbon/carbon-memfile.h>
#include <carbon/carbon-memblock.h>


// Avoid writing out these every time...
static carbon_compressor_configurable_prefix_type_e const none = carbon_compressor_configurable_prefix_type_none;
static carbon_compressor_configurable_prefix_type_e const inc = carbon_compressor_configurable_prefix_type_incremental;
static carbon_compressor_configurable_prefix_type_e const table = carbon_compressor_configurable_prefix_type_prefix_dict_coding;


// Forward decl of helpers
static carbon_compressor_t *
compressor_for(carbon_compressor_highlevel_config_t config, carbon_doc_bulk_t const *context);

static size_t
compute_size(carbon_compressor_t *compressor, carbon_vec_t ofType(char *) *strings);

static void
sort_by_string(carbon_vec_t * entries, size_t idx_offset, size_t length, bool forward);

static char const *
get_strvec_str(carbon_vec_t * entries, size_t idx);



carbon_compressor_selector_result_t carbon_compressor_find_by_strings_brute_force(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context,
        carbon_compressor_selector_brute_force_config_t const config,
        carbon_compressor_selector_sampling_config_t const sampling_config
    ) {
    CARBON_UNUSED(config);

    carbon_err_t err;
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

    carbon_vec_t ofType(char *) *samples = carbon_compressor_selector_sample_strings(strings, &sampling_config);

    carbon_compressor_selector_result_t result = { .estimated_size = SIZE_MAX };
    carbon_huffman_encoder_create(&result.huffman);

    for(size_t i = 0; i < sizeof(configurations)/sizeof(configurations[0]);++i) {
        carbon_compressor_t * compressor = compressor_for(configurations[i], context);

        size_t const size = compute_size(compressor, samples);
        if(size < result.estimated_size) {

            result.estimated_size = size;
            result.config         = configurations[i];

            if(result.config.huffman) {
                carbon_compressor_configurable_extra_t *extra =
                        (carbon_compressor_configurable_extra_t *)compressor->extra;

                carbon_huffman_encoder_drop(&result.huffman);
                carbon_huffman_encoder_create(&result.huffman);
                carbon_huffman_encoder_clone(&extra->huffman_encoder, &result.huffman);
            }
        }

        carbon_compressor_drop(&err, compressor);
    }

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s, suffix = %s, huffman = %s, rev_str = %s, rev_sort = %s",
                (result.config.prefix == inc ? "incremental" : ( result.config.prefix == table ? "prefix-dict" : "none" )),
                (result.config.suffix == inc ? "incremental" : ( result.config.suffix == table ? "prefix-dict" : "none" )),
                result.config.huffman ?         "true" : "false",
                result.config.reverse_strings ? "true" : "false",
                result.config.reverse_sort ?    "true" : "false"
    );

    carbon_vec_drop(samples);
    free(samples);

    result.joinable_group = 1;
    result.compressor     = compressor_for(result.config, context);
    return result;
}


static carbon_compressor_t *compressor_for(
        carbon_compressor_highlevel_config_t config,
        carbon_doc_bulk_t const *context
    ) {
    carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));


    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_CONFIGURABLE);
    carbon_compressor_selector_apply_highlevel_config(compressor, &config);
    return compressor;
}

static size_t compute_size(
        carbon_compressor_t *compressor,
        carbon_vec_t ofType(char *) *strings
    ) {
    carbon_err_t       err;
    carbon_memfile_t   memfile;
    carbon_memblock_t *memblock;
    carbon_compressor_configurable_extra_t * extra =
            (carbon_compressor_configurable_extra_t *)compressor->extra;

    carbon_memblock_create(&memblock, 1024 * 1024);
    carbon_memfile_open(&memfile, memblock, CARBON_MEMFILE_MODE_READWRITE);

    carbon_compressor_configurable_prepare_and_analyze(
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
    CARBON_UNUSED(config);
}
