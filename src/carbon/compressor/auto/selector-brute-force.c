#include <carbon/compressor/auto/selector-brute-force.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

#include <carbon/carbon-memfile.h>
#include <carbon/carbon-memblock.h>


typedef struct compressor_config {
    carbon_compressor_incremental_prefix_type_e prefix;
    carbon_compressor_incremental_prefix_type_e suffix;
    bool reverse;
    bool huffman;
} compressor_config_t;

static carbon_compressor_t *compressor_for(compressor_config_t config, carbon_doc_bulk_t const *context);
static carbon_vec_t ofType(char *) *sample_strings(carbon_vec_t ofType(char *) *strings);
static size_t compute_size(carbon_compressor_t *compressor, carbon_vec_t ofType(char *) *strings);
static void sort_by_string(carbon_vec_t * entries, size_t idx_offset, size_t length, bool forward);
static char const * get_strvec_str(carbon_vec_t * entries, size_t idx);

carbon_compressor_t *carbon_compressor_find_by_strings_brute_force(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context
    ) {
    carbon_err_t err;
    carbon_compressor_incremental_prefix_type_e const none = carbon_compressor_incremental_prefix_type_none;
    carbon_compressor_incremental_prefix_type_e const inc = carbon_compressor_incremental_prefix_type_incremental;
    carbon_compressor_incremental_prefix_type_e const table = carbon_compressor_incremental_prefix_type_table;

    compressor_config_t configurations[] = {
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

    carbon_vec_t ofType(char *) *samples = sample_strings(strings);

    size_t              min_size   = SIZE_MAX;
    compressor_config_t min_config = configurations[0];

    for(size_t i = 0; i < sizeof(configurations)/sizeof(configurations[0]);++i) {
        carbon_compressor_t * compressor = compressor_for(configurations[i], context);

        size_t const size = compute_size(compressor, samples);
        if(size < min_size) {
            min_size   = size;
            min_config = configurations[i];
        }

        carbon_compressor_drop(&err, compressor);
    }

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s, suffix = %s, huffman = %s, rev_str = %s",
                (min_config.prefix == inc ? "incremental" : ( min_config.prefix == table ? "table" : "none" )),
                (min_config.suffix == inc ? "incremental" : ( min_config.suffix == table ? "table" : "none" )),
                min_config.huffman ? "true" : "false",
                min_config.reverse ? "true" : "false"
    );

    carbon_vec_drop(samples);
    free(samples);

    return compressor_for(min_config, context);
}

static carbon_vec_t ofType(char *) *sample_strings(
        carbon_vec_t ofType(char *) *strings
    ) {
    size_t const block_min   = 32;
    size_t const block_size  = 16;
    size_t const threshold   = 1000;
    size_t const entries_min = block_min * block_size;
    size_t const block_cnt   =
            strings->num_elems <= threshold ?
                block_min : (size_t)(strings->num_elems * (1.0 / entries_min));

    carbon_vec_t ofType(char *) *samples = malloc(sizeof(carbon_vec_t));
    carbon_vec_create(
                samples, NULL, sizeof(char *), block_size * block_cnt
    );

    if(strings->num_elems <= block_size * block_cnt) {
        // Copy all strings
        carbon_vec_push(samples, carbon_vec_at(strings, 0), strings->num_elems);
    } else {
        // Sample strings
        for(size_t i = 0; i < block_cnt; ++i) {
            size_t const start_idx =
                    (size_t)((size_t)rand() % (strings->num_elems - block_size));

            carbon_vec_push(
                        samples, carbon_vec_at(strings, start_idx), block_size
            );
        }
    }

    return samples;
}


static carbon_compressor_t *compressor_for(
        compressor_config_t config,
        carbon_doc_bulk_t const *context
    ) {
    carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));

    char tmp_buffer[16];

    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_INCREMENTAL);
    carbon_compressor_set_option(
        &err, compressor, "prefix",
        config.prefix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : (
            config.prefix == carbon_compressor_incremental_prefix_type_table ? "table" : "none"
        )
    );
    carbon_compressor_set_option(
        &err, compressor, "suffix",
        config.suffix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : "none"
    );
    carbon_compressor_set_option(
        &err, compressor, "huffman",
        config.huffman ? "true" : "false"
    );

    snprintf(tmp_buffer, 15, "%lu", 20l);
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
