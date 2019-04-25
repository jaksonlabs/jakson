#include <carbon/compressor/auto/selector.h>

#define NUM_BLOCKS 8
#define BLOCK_LENGTH 4

typedef bool (*char_compare_fn_t)(size_t current_length, size_t previous_length, char const *current, char const *previous, size_t idx);
typedef int  (*sort_compare_fn_t)(void const *a, void const *b);

static int sort_cmp_asc(void const *a, void const *b) {
    return strcmp(*(char * const *)a, *(char * const *)b);
}

static int sort_cmp_desc(void const *a, void const *b) {
    char const *s_a = *(char const * const *)a;
    char const *s_b = *(char const * const *)b;

    size_t len_a = strlen(s_a);
    size_t len_b = strlen(s_b);

    char const * ptr_a = s_a + len_a;
    char const * ptr_b = s_b + len_b;

    while(ptr_a != s_a && ptr_b != s_b && *(--s_a) == *(--s_b));

    if(ptr_a == s_a && ptr_b == s_b)
        return 0;

    if(ptr_a == s_a && ptr_b != s_b)
        return -1;

    if(ptr_b == s_b && ptr_a != s_a)
        return 1;

    return (int)*(uint8_t *)ptr_a - (int)*(uint8_t *)ptr_b;
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

    size_t avg_prefix_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_prefix, sort_cmp_asc);
    size_t avg_suffix_len = carbon_common_prefix_length((char **)strings->base, strings->num_elems, char_cmp_suffix, sort_cmp_asc);

    carbon_err_t err;
    carbon_compressor_by_type(&err, compressor, context, CARBON_COMPRESSOR_INCREMENTAL);
    carbon_compressor_set_option(&err, compressor, "prefix", avg_prefix_len > 2 ? "true" : "false");
    carbon_compressor_set_option(&err, compressor, "suffix", avg_suffix_len > 2 ? "true" : "false");
    carbon_compressor_set_option(&err, compressor, "huffman", "true");

    CARBON_CONSOLE_WRITELN(
                stdout,
                "            Detected settings: prefix = %s (avg %zu), suffix = %s (avg %zu), huffman: %s",
                (avg_prefix_len > 2 ? "true" : "false"), avg_prefix_len,
                (avg_suffix_len > 2 ? "true" : "false"), avg_suffix_len,
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

    char **cpy = malloc(sizeof(char *) * length);
    memcpy(cpy, strings, sizeof(char *) * length);

    qsort(cpy, length, sizeof(char *), sort_cmp);

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

    free(cpy);
    return prefix_length_sum / (NUM_BLOCKS * (BLOCK_LENGTH - 1));
}
