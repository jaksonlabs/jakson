#include <carbon/compressor/huffman/huffman.h>
#include <carbon/compressor/huffman/priority-queue.h>

#include <carbon/carbon-io-device.h>

void this_recursive_clean_huffman_tree(
        carbon_huffman_tree_node_t *node
    ) {
    if(node->left)
        this_recursive_clean_huffman_tree(node->left);
    if(node->right)
        this_recursive_clean_huffman_tree(node->right);

    free(node);
}

void this_recursive_build_huffman_codes(
        carbon_huffman_encoder_t *encoder,
        carbon_huffman_bitstream_t *stream,
        carbon_huffman_tree_node_t *node
    )
{
    if(node->left == 0 && node->right == 0) {
        size_t num_bytes = (stream->num_bits + 7) / 8;

        uint8_t idx = *((uint8_t *)&node->symbol);
        encoder->codes[idx].capacity = num_bytes;
        encoder->codes[idx].num_bits = stream->num_bits;
        encoder->codes[idx].data     = malloc(num_bytes);

        memcpy(encoder->codes[*((uint8_t *)&node->symbol)].data, stream->data, num_bytes);
        return;
    }

    // Process left nodes...
    carbon_huffman_bitstream_write(stream, false);
    this_recursive_build_huffman_codes(encoder, stream, node->left);
    --stream->num_bits;

    // Process right nodes...
    carbon_huffman_bitstream_write(stream, true);
    this_recursive_build_huffman_codes(encoder, stream, node->right);
    --stream->num_bits;
}


void carbon_huffman_encoder_create(
        carbon_huffman_encoder_t *tree
    )
{
    for(size_t i = 0;i < 256;++i) {
        tree->frequencies[i] = 0;
        tree->codes[i].num_bits = 0;
    }
}

void carbon_huffman_encoder_learn_frequencies(
        carbon_huffman_encoder_t *encoder,
        char const *data,
        size_t length
    )
{
    for(size_t i = 0; i < length;++i)
        encoder->frequencies[(uint8_t)data[i]]++;
}

void carbon_huffman_encoder_bake_code(
        carbon_huffman_encoder_t *encoder
    )
{
    carbon_priority_queue_t *queue = carbon_priority_queue_create(UCHAR_MAX);

    size_t max_frequency = SIZE_MAX;

    for(size_t i = 0; i < UCHAR_MAX; ++i) {
        if(encoder->frequencies[i]) {
            carbon_huffman_tree_node_t *node = malloc(sizeof(carbon_huffman_tree_node_t));
            node->left = 0;
            node->right = 0;
            node->frequency = encoder->frequencies[i];
            node->symbol = (char)i;

            carbon_priority_queue_push(queue, node, max_frequency - node->frequency);
        }
    }

    while(queue->count > 1) {
        carbon_huffman_tree_node_t *node_a = (carbon_huffman_tree_node_t *)carbon_priority_queue_pop(queue);
        carbon_huffman_tree_node_t *node_b = (carbon_huffman_tree_node_t *)carbon_priority_queue_pop(queue);

        carbon_huffman_tree_node_t *parent = malloc(sizeof(carbon_huffman_tree_node_t));
        parent->left = node_a;
        parent->right = node_b;
        parent->symbol = 0;
        parent->frequency = node_a->frequency + node_b->frequency;

        carbon_priority_queue_push(queue, parent, max_frequency - parent->frequency);
    }

    carbon_huffman_tree_node_t *root = (carbon_huffman_tree_node_t *)carbon_priority_queue_pop(queue);

    carbon_huffman_bitstream_t stream;
    carbon_huffman_bitstream_create(&stream);
    this_recursive_build_huffman_codes(encoder, &stream, root);
    this_recursive_clean_huffman_tree(root);
    carbon_huffman_bitstream_drop(&stream);

    carbon_priority_queue_free(queue);
}

void carbon_huffman_bitstream_create(
        carbon_huffman_bitstream_t *stream
    )
{
    stream->num_bits = 0;
    stream->capacity = 8;
    stream->data = malloc(stream->capacity);
    memset(stream->data, 0, stream->capacity);
}

void carbon_huffman_bitstream_write(
        carbon_huffman_bitstream_t *stream, bool bit
    )
{
    if(stream->num_bits == stream->capacity * 8) {
        stream->data = realloc(stream->data, 2 * stream->capacity);
        memset(stream->data + stream->capacity, 0, stream->capacity);
        stream->capacity *= 2;
    }

    if(bit) {
        stream->data[stream->num_bits / 8] |= 1 << (stream->num_bits & 7);
    } else {
        stream->data[stream->num_bits / 8] &= ~(1 << (stream->num_bits & 7));
    }

    ++stream->num_bits;
}

void carbon_huffman_bitstream_drop(carbon_huffman_bitstream_t *stream)
{
    free(stream->data);
}

void carbon_huffman_bitstream_dump(carbon_huffman_bitstream_t *stream)
{
    for(size_t i = 0; i < stream->num_bits; ++i) {
        size_t bit_pos = i & 7;
        printf("%zu", (size_t)(stream->data[i / 8] & (1 << bit_pos)) >> bit_pos);
    }
}

void carbon_huffman_encoder_drop(carbon_huffman_encoder_t *encoder)
{
    for(size_t i = 0; i < UCHAR_MAX; ++i) {
        if(encoder->frequencies[i])
            carbon_huffman_bitstream_drop(&encoder->codes[i]);
    }
}

size_t huffman_min(size_t a, size_t b) {
    return a < b ? a : b;
}

void carbon_huffman_bitstream_concat(
        carbon_huffman_bitstream_t *stream,
        carbon_huffman_bitstream_t *other
    ) {

    size_t capacity_scale = 1;
    while(stream->num_bits + other->num_bits > stream->capacity * capacity_scale * 8) {
        capacity_scale *= 2;
    }

    if(capacity_scale > 1) {
        stream->data = realloc(stream->data, capacity_scale * stream->capacity);
        memset(stream->data + stream->capacity, 0, (capacity_scale - 1) * stream->capacity);
        stream->capacity *= capacity_scale;
    }

    size_t bits_written = 0;

    uint8_t bitmask[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
    while(bits_written < other->num_bits) {
        size_t other_bit_offset = (bits_written & 7);
        size_t stream_bit_offset = (stream->num_bits & 7);

        size_t bits_to_cpy = huffman_min(
                    huffman_min(8 - stream_bit_offset, 8 - other_bit_offset),
                    other->num_bits - bits_written
        );

        uint8_t source = other->data[bits_written >> 3];

        if(other_bit_offset > stream_bit_offset) {
            source = source >> (other_bit_offset - stream_bit_offset);
        } else {
            source = (uint8_t)((source << (stream_bit_offset - other_bit_offset)) & 0xFF);
        }

        size_t s = source & (bitmask[bits_to_cpy] << stream_bit_offset);
        stream->data[stream->num_bits >> 3] |= s;
        stream->num_bits += bits_to_cpy;
        bits_written += bits_to_cpy;
    }
}

void carbon_huffman_encode(
        carbon_huffman_encoder_t *encoder,
        carbon_huffman_bitstream_t *stream,
        char *data
    ) {
    while(*data) {
        carbon_huffman_bitstream_concat(stream, &encoder->codes[*((uint8_t *)data)]);
        ++data;
    }
}

void carbon_huffman_decoder_create(
        carbon_huffman_decoder_t *decoder,
        carbon_huffman_dictionary_t dictionary
    ) {
    memcpy(decoder->codes, dictionary, sizeof(*dictionary) * UCHAR_MAX);

    decoder->tree = malloc(sizeof(carbon_huffman_tree_node_t));
    decoder->tree->left = 0;
    decoder->tree->right = 0;
    decoder->tree->symbol = 0;

    for(size_t i = 0; i < UCHAR_MAX; ++i) {
        carbon_huffman_bitstream_t *stream = &dictionary[i];
        if(stream->num_bits == 0)
            continue;

        carbon_huffman_tree_node_t *node = decoder->tree;
        for(size_t j = 0; j < stream->num_bits; ++j) {
            carbon_huffman_tree_node_t **tmp = 0;
            if(stream->data[j >> 3] & 1 << (j & 7)) {
                tmp = &node->right;
            } else {
                tmp = &node->left;
            }

            if(!*tmp) {
                *tmp = malloc(sizeof(carbon_huffman_tree_node_t));
                (*tmp)->left = 0;
                (*tmp)->right = 0;
                (*tmp)->symbol = 0;
                (*tmp)->frequency = 0;
            }

            node = *tmp;
        }

        node->symbol = (char)i;
    }
}

char *carbon_huffman_decode(
        carbon_huffman_decoder_t *decoder,
        carbon_huffman_bitstream_t *stream
    ) {
    size_t length = 0;
    size_t capacity = 16;
    char  *dst = malloc(capacity);

    for(size_t i = 0; i < stream->num_bits;) {
        carbon_huffman_tree_node_t *node = decoder->tree;
        while(node->left != 0 && node->right != 0 && i < stream->num_bits) {
            if(stream->data[i >> 3] & 1 << (i & 7)) {
                node = node->right;
            } else {
                node = node->left;
            }

            ++i;
        }

        if(node->left == 0 && node->right == 0) {
            if(length == capacity) {
                capacity *= 2;
                dst = realloc(dst, capacity);
            }

            dst[length++] = node->symbol;
        } else {
            free(dst);
            return 0;
        }
    }

    return dst;
}

void carbon_huffman_decoder_drop(
    carbon_huffman_decoder_t *decoder
    ) {
    this_recursive_clean_huffman_tree(decoder->tree);
}

void carbon_huffman_adaptive_update(
    carbon_huffman_encoder_t *encoder,
    char const **entries,
    size_t num_entries,
    size_t fade
    ) {

    // The frequency array is not touched as it was not allocated dynamically.
    carbon_huffman_encoder_drop(encoder);

    for(size_t i = 0; i < UCHAR_MAX; ++i) {
        if(encoder->frequencies[i] > fade + 1)
            encoder->frequencies[i] -= fade;
        else
            encoder->frequencies[i] = 1;
    }

    for(size_t i = 0; i < num_entries; ++i) {
        carbon_huffman_encoder_learn_frequencies(encoder, entries[i], strlen(entries[i]));
    }

    carbon_huffman_encoder_bake_code(encoder);
}

void carbon_huffman_create_all_eq_encoder(
        carbon_huffman_encoder_t *encoder
    ) {
    for(size_t i = 0; i < UCHAR_MAX; ++i) {
        encoder->frequencies[i] = 1;
    }

    carbon_huffman_encoder_bake_code(encoder);
}

char *carbon_huffman_decode_io(
        carbon_huffman_decoder_t *decoder,
        carbon_io_device_t *io,
        size_t nsymbols
    ) {
    size_t length = 0;
    char  *dst = malloc(nsymbols + 1);
    dst[nsymbols] = 0;

    size_t  bitpos = 0;
    uint8_t current_byte = 0;
    for(size_t i = 0; i < nsymbols; ++i) {
        carbon_huffman_tree_node_t *node = decoder->tree;
        while(node->left != 0 && node->right != 0) {
            if((bitpos & 7) == 0)
                carbon_io_device_read(io, &current_byte, 1, 1);

            if(current_byte & 1 << (bitpos & 7)) {
                node = node->right;
            } else {
                node = node->left;
            }

            ++bitpos;
        }

        if(node->left == 0 && node->right == 0) {
            dst[length++] = node->symbol;
        } else {
            free(dst);
            return 0;
        }
    }

    return dst;
}

void carbon_huffman_bitstream_write_byte(
        carbon_huffman_bitstream_t *stream,
        uint8_t byte
    ) {
    for(size_t i = 0; i < 8; ++i)
        carbon_huffman_bitstream_write(stream, (byte & (1 << i)) > 0);
}
