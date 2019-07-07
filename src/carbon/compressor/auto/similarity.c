#include <carbon/compressor/auto/similarity.h>

bool carbon_compressor_config_similarity(
        carbon_compressor_selector_result_t *a,
        carbon_compressor_selector_result_t *b
    )
{
    if(
            a->config.prefix != b->config.prefix ||
            a->config.suffix != b->config.suffix ||
            a->config.reverse != b->config.reverse ||
            a->config.huffman != b->config.huffman
    ) {
        return false;
    }

    if(a->config.huffman) {
        double abl_a = carbon_huffman_avg_bit_length(&a->huffman, a->huffman.frequencies);
        double abl_b = carbon_huffman_avg_bit_length(&b->huffman, b->huffman.frequencies);

        double abl_ab = carbon_huffman_avg_bit_length(&a->huffman, b->huffman.frequencies);
        double abl_ba = carbon_huffman_avg_bit_length(&b->huffman, a->huffman.frequencies);

        return (abl_ab + abl_ba) - (abl_a + abl_b) < 0.25;
    }

    return true;
}
