#include <carbon/compressor/auto/selector.h>
#include <carbon/compressor/compressor-utils.h>

#define NUM_BLOCKS 32
#define BLOCK_LENGTH 4

typedef bool (*char_compare_fn_t)(size_t current_length, size_t previous_length, char const *current, char const *previous, size_t idx);
typedef int  (*sort_compare_fn_t)(void const *a, void const *b);

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


static size_t carbon_common_prefix_length(
    char **strings, size_t length,
    char_compare_fn_t char_cmp, sort_compare_fn_t sort_cmp
);

carbon_compressor_t *carbon_compressor_find_by_strings(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context
    ) {
    CARBON_UNUSED(strings);
    carbon_compressor_t *compressor = malloc(sizeof(carbon_compressor_t));

    size_t avg_pre_pre_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_prefix, carbon_sort_cmp_fwd);
    size_t avg_pre_suf_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_suffix, NULL);
    size_t avg_suf_pre_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_prefix, carbon_sort_cmp_rwd);
    size_t avg_suf_suf_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_suffix, NULL);

    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_INCREMENTAL);
    carbon_compressor_set_option(&err, compressor, "prefix", avg_pre_pre_len >= 1 ? "incremental" : "none");
    carbon_compressor_set_option(&err, compressor, "suffix", avg_pre_suf_len >= 1 ? "incremental" : "none");
    carbon_compressor_set_option(&err, compressor, "huffman", "true");
    carbon_compressor_set_option(&err, compressor, "delta_chunk_length", "20");
    carbon_compressor_set_option(&err, compressor, "sort_chunk_length", "10000000");

    printf(
        "pp = %zu, ps = %zu, sp = %zu, ss = %zu\n",
        avg_pre_pre_len, avg_pre_suf_len, avg_suf_pre_len, avg_suf_suf_len
    );
    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s (avg %zu), suffix = %s (avg %zu), huffman: %s",
                (avg_pre_pre_len >= 1 ? "incremental" : "none"), avg_pre_pre_len,
                (avg_suf_suf_len >= 1 ? "incremental" : "none"), avg_pre_suf_len,
                "true"
    );
    return compressor;
}

static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

size_t carbon_common_prefix_length(
        char **strings, size_t length,
        char_compare_fn_t char_cmp, sort_compare_fn_t sort_cmp
    ) {
    if(length < BLOCK_LENGTH)
        return 0;

    char **cpy = NULL;

    if(sort_cmp) {
        cpy = malloc(sizeof(char *) * length);
        memcpy(cpy, strings, sizeof(char *) * length);

        qsort(cpy, length, sizeof(char *), sort_cmp);
    } else {
        cpy = strings;
    }

    size_t prefix_length_sum = 0;
    for(size_t i = 0; i < NUM_BLOCKS; ++i) {
        size_t const start_idx = (size_t)(rand() % (length - BLOCK_LENGTH));

        char const * previous = "";
        size_t previous_length = 0;
        for(size_t j = 0; j < BLOCK_LENGTH;++j) {
            char   const * current        = cpy[j + start_idx];
            size_t const   current_length = strlen(current);

            size_t max_length = min(current_length, min(previous_length, 255));
            size_t prefix_length = 0;
            for(; prefix_length < max_length && char_cmp(current_length, previous_length, current, previous, prefix_length);++prefix_length);

            prefix_length_sum += prefix_length;

            previous = current;
            previous_length = current_length;
        }
    }

    if(sort_cmp) {
        free(cpy);
    }
    return prefix_length_sum / (NUM_BLOCKS * (BLOCK_LENGTH - 1));
}
