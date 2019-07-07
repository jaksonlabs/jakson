#include <carbon/compressor/auto/selector-cost-based.h>
#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-incremental.h>
#include <carbon/compressor/huffman/priority-queue.h>
#include <carbon/compressor/prefix/prefix_encoder.h>

#include <math.h>

#define NUM_BLOCKS 8
#define BLOCK_LENGTH 20

typedef bool (*char_compare_fn_t)(size_t current_length, size_t previous_length, char const *current, char const *previous, size_t idx);
typedef int  (*sort_compare_fn_t)(void const *a, void const *b);


static double
selector_average_common_length(char **strings, size_t length, char_compare_fn_t char_cmp, sort_compare_fn_t sort_cmp);

static size_t
selector_estimate_huffman_table_size(carbon_huffman_encoder_t *encoder);

static void
selector_config_from_prefix_suffix_analysis(stringset_properties_t *properties);

static void
selector_analyze_prefix_suffix_properties(carbon_vec_t ofType(char const *) * entries, stringset_properties_t * properties);

static void
selector_apply_to_compressor(stringset_properties_t * properties, carbon_compressor_t * compressor, carbon_doc_bulk_t const * context);

static void
selector_analyze_huffman_and_prefix_table(carbon_vec_t ofType(char const *) * strings, stringset_properties_t * properties);

static void
selector_analyze_prefix_table_size_and_savings(char **strings, size_t length, stringset_properties_t *properties);

static bool
selector_config_similar(stringset_properties_t *cfg_a, stringset_properties_t *cfg_b) {
    if(cfg_a->compressor_config.prefix != cfg_b->compressor_config.prefix)
        return false;

    if(cfg_a->compressor_config.suffix != cfg_b->compressor_config.suffix)
        return false;

    if(cfg_a->compressor_config.huffman != cfg_b->compressor_config.huffman)
        return false;

    if(cfg_a->compressor_config.huffman)
        return (
            carbon_huffman_avg_bit_length(&cfg_a->huffman_encoder, cfg_b->huffman_encoder.frequencies)
                    -
            carbon_huffman_avg_bit_length(&cfg_a->huffman_encoder, cfg_a->huffman_encoder.frequencies)
        ) < 0.1;

    return true;
}

static void selector_sort_by_string(
    carbon_vec_t * entries, size_t idx_offset, size_t length, bool forward
) {
    qsort(
        (char const **)entries->base + idx_offset, length, sizeof(char const *),
        forward ? carbon_sort_cmp_fwd : carbon_sort_cmp_rwd
    );
}

static char const * selector_get_strvec_str(
    carbon_vec_t * entries, size_t idx
) {
    return *(char const * const *)carbon_vec_at(entries, idx);
}

static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

static double minf(double a, double b) {
    return a < b ? a : b;
}

static size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

static ssize_t maxs(ssize_t a, ssize_t b) {
    return a > b ? a : b;
}

static double maxf(double a, double b) {
    return a > b ? a : b;
}

static bool char_cmp_prefix(size_t current_length, size_t previous_length, char const *current, char const *previous, size_t idx) {
    CARBON_UNUSED(current_length);
    CARBON_UNUSED(previous_length);
    return current[idx] == previous[idx];
}

static bool char_cmp_suffix(size_t current_length, size_t previous_length, char const *current, char const *previous, size_t idx) {
    CARBON_UNUSED(current_length);
    CARBON_UNUSED(previous_length);
    return current[current_length - 1 - idx] == previous[previous_length - 1 - idx];
}

carbon_compressor_t *carbon_compressor_find_by_strings_cost_based(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context
    ) {
    CARBON_UNUSED(strings);
    carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));

    stringset_properties_t properties;
    properties.num_entries = strings->num_elems;

    selector_analyze_prefix_suffix_properties(strings, &properties);
    selector_analyze_prefix_table_size_and_savings(strings->base, strings->num_elems, &properties);
    selector_config_from_prefix_suffix_analysis(&properties);
    selector_analyze_huffman_and_prefix_table(strings, &properties);


    size_t huffman_saving_bytes =
            (size_t)((8.0 - properties.huffman_average_bit_len) * properties.remaining_text_length) / 8;

    properties.compressor_config.huffman = huffman_saving_bytes > properties.huffman_estimated_table_size;

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            pp = %.1f, ps = %.1f, sp = %.1f, ss = %.1f, avg_rem_len = %.1f",
                properties.avg_pre_pre_len, properties.avg_pre_suf_len,
                properties.avg_suf_pre_len, properties.avg_suf_suf_len,
                (double)properties.remaining_text_length / strings->num_elems
    );
    CARBON_CONSOLE_WRITELN(
                stdout,
                "            avg_symbol_len = %.1f, est_saving = %zu, est_huff_tbl_size = %zu",
                properties.huffman_average_bit_len,
                huffman_saving_bytes,
                properties.huffman_estimated_table_size
    );
    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s (avg %.1f), suffix = %s (avg %.1f), huffman = %s (svg %zu), rev_sort = %s, rev_str = %s",
                (
                    properties.compressor_config.prefix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : (
                        properties.compressor_config.prefix == carbon_compressor_incremental_prefix_type_table ? "table" : "none")
                ),
                properties.avg_suf_pre_len,
                (properties.compressor_config.suffix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : "none"),
                properties.avg_suf_suf_len,
                properties.compressor_config.huffman ? "true" : "false",
                properties.compressor_config.huffman ? huffman_saving_bytes - properties.huffman_estimated_table_size : 0,
                properties.compressor_config.reverse_sort ? "true" : "false",
                properties.compressor_config.reverse_strings ? "true" : "false"
    );


    selector_apply_to_compressor(&properties, compressor, context);
    return compressor;
}

static void selector_analyze_prefix_suffix_properties(
    carbon_vec_t ofType(char const *) * strings, stringset_properties_t * properties
) {
    properties->avg_pre_pre_len = selector_average_common_length((char **)strings->base, strings->num_elems, char_cmp_prefix, carbon_sort_cmp_fwd);
    properties->avg_pre_suf_len = selector_average_common_length((char **)strings->base, strings->num_elems, char_cmp_suffix, carbon_sort_cmp_fwd);
    properties->avg_suf_pre_len = selector_average_common_length((char **)strings->base, strings->num_elems, char_cmp_prefix, carbon_sort_cmp_rwd);
    properties->avg_suf_suf_len = selector_average_common_length((char **)strings->base, strings->num_elems, char_cmp_suffix, carbon_sort_cmp_rwd);
}

static void selector_apply_to_compressor(
    stringset_properties_t * properties,
    carbon_compressor_t * compressor,
    carbon_doc_bulk_t const * context
) {
    char tmp_buffer[16];

    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_INCREMENTAL);
    carbon_compressor_set_option(
                &err, compressor, "prefix",
                properties->compressor_config.prefix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : (
                    properties->compressor_config.prefix == carbon_compressor_incremental_prefix_type_table ? "table" : "none"
                )
    );
    carbon_compressor_set_option(&err, compressor, "suffix", properties->compressor_config.suffix == carbon_compressor_incremental_prefix_type_incremental ? "incremental" : "none");
    carbon_compressor_set_option(&err, compressor, "huffman", properties->compressor_config.huffman ? "true" : "false");
    snprintf(tmp_buffer, 15, "%lu", properties->compressor_config.delta_chunk_length);
    carbon_compressor_set_option(&err, compressor, "delta_chunk_length", tmp_buffer);
    snprintf(tmp_buffer, 15, "%lu", properties->compressor_config.sort_chunk_length);
    carbon_compressor_set_option(&err, compressor, "sort_chunk_length", tmp_buffer);
}

static void
selector_analyze_huffman_and_prefix_table(
    carbon_vec_t ofType(char const *) * strings,
    stringset_properties_t * properties
) {
    properties->compressor_config.huffman = true;

    carbon_huffman_encoder_create(&properties->huffman_encoder);

    carbon_prefix_table *prefix_tbl_table = 0;
    carbon_prefix_ro_tree *prefix_tbl_encoder = 0;
    carbon_compressor_incremental_stringset_properties_t huffman_stringset_properties =
        carbon_compressor_incremental_prepare_and_analyze(
            strings, &properties->compressor_config, &prefix_tbl_table, &prefix_tbl_encoder,
            &properties->huffman_encoder, selector_sort_by_string, selector_get_strvec_str
        );

    properties->remaining_text_length = huffman_stringset_properties.total_remaining_text_length;
    properties->huffman_average_bit_len = carbon_huffman_avg_bit_length(&properties->huffman_encoder, properties->huffman_encoder.frequencies);
    properties->huffman_estimated_table_size = selector_estimate_huffman_table_size(&properties->huffman_encoder);

    if(prefix_tbl_table)
        carbon_prefix_table_free(prefix_tbl_table);
    if(prefix_tbl_encoder)
        carbon_prefix_ro_tree_free(prefix_tbl_encoder);
    carbon_huffman_encoder_drop(&properties->huffman_encoder);
}

static void selector_config_from_prefix_suffix_analysis(
        stringset_properties_t *properties
) {
    ssize_t delta_chunk_length = 20;

    ssize_t s_pre_tbl = ((ssize_t)(properties->prefix_saving) - (ssize_t)properties->prefix_tbl_len - (ssize_t)(1.0 * properties->num_entries));
    ssize_t s_suf_tbl = ((ssize_t)(properties->suffix_saving) - (ssize_t)properties->suffix_tbl_len - (ssize_t)(1.0 * properties->num_entries));

    ssize_t s_pre_pre_inc = (ssize_t)((properties->avg_pre_pre_len - 1.0) * properties->num_entries);
    ssize_t s_suf_suf_inc = (ssize_t)((properties->avg_suf_suf_len - 1.0) * properties->num_entries);
    ssize_t s_pre_suf_inc = (ssize_t)((properties->avg_pre_suf_len - 1.0) * properties->num_entries);
    ssize_t s_suf_pre_inc = (ssize_t)((properties->avg_suf_pre_len - 1.0) * properties->num_entries);

    typedef struct local_config {
        carbon_compressor_incremental_prefix_type_e prefix;
        carbon_compressor_incremental_prefix_type_e suffix;

        ssize_t pre_savings;
        ssize_t suf_savings;
        bool reverse;
    } local_config_t;

    (void)s_pre_tbl;
    (void)s_suf_tbl;
    (void)s_suf_suf_inc;
    (void)s_suf_pre_inc;
    (void)s_pre_pre_inc;
    (void)s_pre_suf_inc;
    local_config_t configs[] = {
        { .prefix = carbon_compressor_incremental_prefix_type_none, .suffix = carbon_compressor_incremental_prefix_type_none, .reverse = false, .pre_savings = 0, .suf_savings = 0 },
        { .prefix = carbon_compressor_incremental_prefix_type_table, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = false, .pre_savings = s_pre_tbl, .suf_savings = s_suf_suf_inc },
        { .prefix = carbon_compressor_incremental_prefix_type_table, .suffix = carbon_compressor_incremental_prefix_type_none, .reverse = false, .pre_savings = s_pre_tbl, .suf_savings = 0 },
        { .prefix = carbon_compressor_incremental_prefix_type_incremental, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = false, .pre_savings = s_suf_pre_inc, .suf_savings = s_suf_suf_inc },
        { .prefix = carbon_compressor_incremental_prefix_type_incremental, .suffix = carbon_compressor_incremental_prefix_type_none, .reverse = false, .pre_savings = s_suf_pre_inc, .suf_savings = 0 },
        { .prefix = carbon_compressor_incremental_prefix_type_none, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = false, .pre_savings = 0, .suf_savings = s_suf_suf_inc },
        { .prefix = carbon_compressor_incremental_prefix_type_table, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = true, .pre_savings = s_suf_tbl, .suf_savings = s_pre_pre_inc },
        { .prefix = carbon_compressor_incremental_prefix_type_table, .suffix = carbon_compressor_incremental_prefix_type_none, .reverse = true, .pre_savings = s_suf_tbl, .suf_savings = 0 },
        { .prefix = carbon_compressor_incremental_prefix_type_incremental, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = true, .pre_savings = s_pre_suf_inc, .suf_savings = s_pre_pre_inc },
        { .prefix = carbon_compressor_incremental_prefix_type_incremental, .suffix = carbon_compressor_incremental_prefix_type_none, .reverse = true, .pre_savings = s_pre_suf_inc, .suf_savings = 0 },
        { .prefix = carbon_compressor_incremental_prefix_type_none, .suffix = carbon_compressor_incremental_prefix_type_incremental, .reverse = true, .pre_savings = 0, .suf_savings = s_pre_pre_inc }
    };

    local_config_t best_config = configs[0];
    ssize_t best_config_savings = configs[0].pre_savings + configs[0].suf_savings;

    for(size_t i = 0; i < sizeof(configs) / sizeof(configs[0]); ++i) {
        printf("%s (%zd) / %s (%zd) / %s : %zd\n",
               configs[i].prefix == carbon_compressor_incremental_prefix_type_none ? "none" : (configs[i].prefix == carbon_compressor_incremental_prefix_type_table ? "table" : "incremental"),
               configs[i].pre_savings,
               configs[i].suffix == carbon_compressor_incremental_prefix_type_none ? "none" : (configs[i].suffix == carbon_compressor_incremental_prefix_type_table ? "table" : "incremental"),
               configs[i].suf_savings,
               configs[i].reverse ? "rwd" : "fwd",
               configs[i].pre_savings + configs[i].suf_savings
        );

        if(configs[i].pre_savings + configs[i].suf_savings > best_config_savings) {
            best_config_savings = configs[i].pre_savings + configs[i].suf_savings;
            best_config = configs[i];
        }
    }

    printf(" --> %s / %s / %s : %zd\n",
           best_config.prefix == carbon_compressor_incremental_prefix_type_none ? "none" : (best_config.prefix == carbon_compressor_incremental_prefix_type_table ? "table" : "incremental"),
           best_config.suffix == carbon_compressor_incremental_prefix_type_none ? "none" : (best_config.suffix == carbon_compressor_incremental_prefix_type_table ? "table" : "incremental"),
           best_config.reverse ? "rwd" : "fwd",
           best_config.pre_savings + best_config.suf_savings
    );

    properties->compressor_config.prefix = best_config.prefix;
    properties->compressor_config.suffix = best_config.suffix;
    properties->compressor_config.reverse_strings = best_config.reverse;

    properties->compressor_config.reverse_sort = true;
    properties->compressor_config.delta_chunk_length = delta_chunk_length;
    properties->compressor_config.sort_chunk_length = 10000000;
}

static void selector_analyze_prefix_table_size_and_savings(
        char **strings, size_t length, stringset_properties_t *properties
    ) {
    if(length <= BLOCK_LENGTH) {
        properties->prefix_tbl_len = 0;
        properties->suffix_tbl_len = 0;
        properties->prefix_saving = 0;
        properties->suffix_saving = 0;
        return;
    }

    carbon_prefix_encoder_config * cfg = carbon_prefix_encoder_auto_config(length, 2.0);

    carbon_prefix_tree_node * fwd_root = carbon_prefix_tree_node_create(0);
    carbon_prefix_tree_node * rwd_root = carbon_prefix_tree_node_create(0);

    for(size_t i = 0; i < length; ++i) {
        char const * current = strings[i];
        char * tnerruc = strdup(strings[i]);
        carbon_str_reverse(tnerruc);

        carbon_prefix_tree_node_add_string(fwd_root, current, cfg->max_new_children_per_entry);
        carbon_prefix_tree_node_add_string(rwd_root, tnerruc, cfg->max_new_children_per_entry);

        free(tnerruc);
    }

    carbon_prefix_tree_node_prune(fwd_root, cfg->prune_min_support);
    carbon_prefix_tree_node_prune(rwd_root, cfg->prune_min_support);

    carbon_prefix_tree_calculate_savings(fwd_root, 15);
    carbon_prefix_tree_calculate_savings(rwd_root, 15);

    carbon_prefix_table * fwd_table = carbon_prefix_table_create();
    carbon_prefix_table * rwd_table = carbon_prefix_table_create();
    carbon_prefix_tree_encode_all_with_queue(fwd_root, fwd_table);
    carbon_prefix_tree_encode_all_with_queue(rwd_root, rwd_table);

    properties->prefix_tbl_len = carbon_prefix_table_num_bytes(fwd_table);
    properties->suffix_tbl_len = carbon_prefix_table_num_bytes(rwd_table);

    carbon_prefix_ro_tree * fwd_encoder = carbon_prefix_table_to_encoder_tree(fwd_table);
    carbon_prefix_ro_tree * rwd_encoder = carbon_prefix_table_to_encoder_tree(rwd_table);

    size_t fwd_length_sum = 0;
    size_t rwd_length_sum = 0;

    for(size_t i = 0; i < NUM_BLOCKS; ++i) {
        size_t const start_idx = (size_t)(rand() % (length - BLOCK_LENGTH));

        for(size_t j = 0; j < BLOCK_LENGTH;++j) {
            char const * current = strings[j + start_idx];
            char * tnerruc = strdup(current);
            carbon_str_reverse(tnerruc);

            size_t fwd_max_length = 0;
            size_t rwd_max_length = 0;

            carbon_prefix_ro_tree_max_prefix(fwd_encoder, current, &fwd_max_length);
            carbon_prefix_ro_tree_max_prefix(rwd_encoder, current, &rwd_max_length);

            fwd_length_sum += fwd_max_length;
            rwd_length_sum += rwd_max_length;

            free(tnerruc);
        }
    }

    free(cfg);
    carbon_prefix_ro_tree_free(fwd_encoder);
    carbon_prefix_ro_tree_free(rwd_encoder);
    carbon_prefix_tree_node_free(&fwd_root);
    carbon_prefix_tree_node_free(&rwd_root);
    carbon_prefix_table_free(fwd_table);
    carbon_prefix_table_free(rwd_table);

    properties->prefix_saving = (size_t)((double)fwd_length_sum / (double)(BLOCK_LENGTH * NUM_BLOCKS) * properties->num_entries);
    properties->suffix_saving = (size_t)((double)rwd_length_sum / (double)(BLOCK_LENGTH * NUM_BLOCKS) * properties->num_entries);
}

static double selector_average_common_length(
        char **strings, size_t length,
        char_compare_fn_t char_cmp, sort_compare_fn_t sort_cmp
    ) {
    if(length <= 1)
        return 0;

    size_t const block_len = min(BLOCK_LENGTH, length - 1);

    char **cpy = malloc(sizeof(char *) * length);
    memcpy(cpy, strings, sizeof(char *) * length);
    qsort(cpy, length, sizeof(char *), sort_cmp);

    size_t prefix_length_sum = 0;
    for(size_t i = 0; i < NUM_BLOCKS; ++i) {
        size_t const start_idx = (size_t)(rand() % (length - block_len));

        char const * previous = "";
        size_t previous_length = 0;
        for(size_t j = 0; j < block_len;++j) {
            char   const * current        = cpy[j + start_idx];
            size_t const   current_length = strlen(current);

            size_t max_length = min(current_length, min(previous_length, 255));
            size_t prefix_length = 0;
            for(; prefix_length < max_length && char_cmp(current_length, previous_length, current, previous, prefix_length);++prefix_length);

            previous = current;
            previous_length = current_length;
            prefix_length_sum += prefix_length;
        }
    }

    free(cpy);
    return (double)prefix_length_sum / (NUM_BLOCKS * block_len);
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
