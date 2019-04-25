/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <inttypes.h>
#include <assert.h>

#include <carbon/carbon-strdic.h>
#include <carbon/carbon-doc.h>
#include <carbon/carbon-compressor.h>

#include <carbon/compressor/compressor-utils.h>
#include <carbon/compressor/carbon-compressor-incremental.h>

#include <carbon/compressor/huffman/huffman.h>
#include <carbon/compressor/huffman/priority-queue.h>

#include <carbon/carbon-archive.h>
#include <carbon/carbon-io-device.h>

typedef struct {
    size_t sort_chunk_length;
    size_t delta_chunk_length;

    bool prefix;
    bool suffix;

    bool huffman;
} carbon_compressor_incremental_config_t;

typedef struct {
    size_t previous_offset;
    size_t previous_length;
    size_t strings_since_last_base;
    size_t strings_since_last_huffman_update;
    char const * previous_string;

    carbon_huffman_encoder_t huffman_encoder;
    carbon_huffman_decoder_t huffman_decoder;

    carbon_compressor_incremental_config_t config;
} carbon_compressor_incremental_extra_t;

typedef struct {
    uint8_t byte;
    size_t  bitpos;
    carbon_io_device_t *device;
} carbon_bitstream_ro_t;

bool option_set_bool(carbon_err_t *err, bool *target, char *value) {
    if(strcmp(value, "true") == 0) {
        *target = true;
        return true;
    } else if(strcmp(value, "false") == 0) {
        *target = false;
        return true;
    } else {
        CARBON_ERROR(err, CARBON_ERR_COMPRESSOR_OPT_VAL_INVALID);
        return false;
    }
}

bool option_set_sizet(carbon_err_t *err, size_t *target, char *value) {
    long long length = atol(value);

    if(length <= 0) { CARBON_ERROR(err, CARBON_ERR_COMPRESSOR_OPT_VAL_INVALID); return false; }
    *target = (size_t)length;

    return true;
}


bool set_prefix(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_bool(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.prefix, value);
}

bool set_suffix(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_bool(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.suffix, value);
}

bool set_huffman(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_bool(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.huffman, value);
}

bool set_sort_chunk_length(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_sizet(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.sort_chunk_length, value);
}

bool set_delta_chunk_length(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_sizet(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.delta_chunk_length, value);
}


bool this_read_string_from_io_device(carbon_compressor_t *self, carbon_io_device_t *src, char *dst, size_t strlen) {
    typedef struct local_entry {
        size_t strlen;
        char * str;
        size_t  end_pos;
        uint8_t common_prefix_length;
        uint8_t common_suffix_length;
    } local_entry_t;

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_vec_t ofType(local_entry_t) dependencies;
    carbon_vec_create(&dependencies, NULL, sizeof(local_entry_t), 10);

    size_t  previous_offset_diff = 0;
    uint8_t common_prefix_len_ui8 = 0, common_suffix_len_ui8 = 0;

    do {
        local_entry_t entry;

        carbon_io_device_seek(src, carbon_io_device_tell(src) - previous_offset_diff - sizeof(carbon_string_entry_header_t));

        {
            carbon_string_entry_header_t header;
            carbon_io_device_read(src, &header, sizeof(header), 1);
            entry.strlen = header.string_len;
        }

        size_t current_position = carbon_io_device_tell(src);

        bool ok;
        previous_offset_diff = carbon_vlq_decode_from_io(src, &ok);
        if(!ok)
            return false;


        if(extra->config.prefix) {
            if(carbon_io_device_read(src, &common_prefix_len_ui8, 1, 1) != 1)
                return false;
        }

        if(extra->config.suffix) {
            if(carbon_io_device_read(src, &common_suffix_len_ui8, 1, 1) != 1)
                return false;
        }

        entry.common_prefix_length = common_prefix_len_ui8;
        entry.common_suffix_length = common_suffix_len_ui8;

        if(extra->config.huffman) {
            entry.str = malloc(entry.strlen + 1);
            entry.str[entry.strlen] = 0;

            char *tmp = carbon_huffman_decode_io(&extra->huffman_decoder, src, entry.strlen - common_prefix_len_ui8 - common_suffix_len_ui8);
            strncpy(entry.str + common_prefix_len_ui8, tmp, entry.strlen - common_prefix_len_ui8 - common_suffix_len_ui8);
            free(tmp);
        } else {
            entry.str = malloc(entry.strlen + 1);
            entry.str[entry.strlen] = 0;
            carbon_io_device_read(
                        src,
                        entry.str + common_prefix_len_ui8, sizeof(char),
                        entry.strlen - common_prefix_len_ui8 - common_suffix_len_ui8
            );
        }

        entry.end_pos = carbon_io_device_tell(src);

        carbon_vec_push(&dependencies, &entry, 1);
        carbon_io_device_seek(src, current_position);
    } while(common_prefix_len_ui8 != 0 || common_suffix_len_ui8 != 0);



    // Dependencies now contains all entries in reverse order -> reconstruct them
    for(ssize_t i = (ssize_t)dependencies.num_elems - 2;i >= 0;--i) {
        local_entry_t const *current  = (local_entry_t const *)carbon_vec_at(&dependencies, (size_t)i);
        local_entry_t const *previous = (local_entry_t const *)carbon_vec_at(&dependencies, (size_t)i + 1);

        strncpy(current->str, previous->str, current->common_prefix_length);
        strncpy(
            current->str + current->strlen - current->common_suffix_length,
            previous->str + previous->strlen - current->common_suffix_length,
            current->common_suffix_length
        );
    }

    carbon_io_device_seek(src, ((local_entry_t const *)carbon_vec_at(&dependencies, 0))->end_pos);

    strncpy(dst, ((local_entry_t const *)carbon_vec_at(&dependencies, 0))->str, strlen + 1);
    carbon_vec_drop(&dependencies);

    return true;
}

void this_read_config_from_io_device(carbon_compressor_t *self, carbon_io_device_t *src,
                                     carbon_compressor_incremental_config_t *config) {

    CARBON_UNUSED(self);

    uint8_t flags = 0;
    carbon_io_device_read(src, &flags, 1, 1);

    config->prefix = flags & 1;
    config->suffix = flags & 2;
    config->huffman = flags & 4;
}

size_t this_read_next_bit_from_io(carbon_bitstream_ro_t *stream) {
    if((stream->bitpos & 7) == 0) {
        carbon_io_device_read(stream->device, &stream->byte, 1, 1);
    }

    size_t bit = (stream->byte & (1 << (stream->bitpos & 7))) ? 1 : 0;
    ++stream->bitpos;
    return bit;
}

void this_read_huffman_table_from_io_device(
        carbon_io_device_t *src,
        carbon_compressor_incremental_extra_t *extra
    ) {

    bool ok;
    size_t num_entries = carbon_vlq_decode_from_io(src, &ok);

    carbon_huffman_dictionary_t dictionary;
    carbon_bitstream_ro_t stream;
    stream.device = src;
    stream.byte = 0;
    stream.bitpos = 0;

    for(size_t i = 0; i < UCHAR_MAX; ++i)
        carbon_huffman_bitstream_create(&dictionary[i]);

    for(size_t i = 0; i < num_entries;) {

        size_t bitlen = 0;
        for(size_t j = 0; j < 8; ++j)
            bitlen |= this_read_next_bit_from_io(&stream) << j;


        size_t entry_count = 0;
        for(size_t j = 0; j < 8; ++j)
            entry_count |= this_read_next_bit_from_io(&stream) << j;

        for(size_t j = 0; j < entry_count; ++j) {
            uint8_t symbol = 0;
            for(size_t j = 0; j < 8; ++j)
                symbol |= this_read_next_bit_from_io(&stream) << j;

            for(size_t j = 0; j < bitlen; ++j)
                carbon_huffman_bitstream_write(&dictionary[symbol], this_read_next_bit_from_io(&stream));

            ++i;
        }
    }

    carbon_huffman_decoder_create(&extra->huffman_decoder, dictionary);
}


/**
  Compressor implementation
  */
CARBON_EXPORT(bool)
carbon_compressor_incremental_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(context);
    /* nothing to do for uncompressed dictionaries */


    self->extra = malloc(sizeof(carbon_compressor_incremental_extra_t));

    carbon_compressor_incremental_extra_t *extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_huffman_encoder_create(&extra->huffman_encoder);
    extra->huffman_decoder.tree = NULL;

    extra->previous_offset = 0;
    extra->previous_length = 0;
    extra->strings_since_last_base = 0;
    extra->strings_since_last_huffman_update = 0;
    extra->previous_string = "";


    extra->config.prefix = true;
    extra->config.suffix = false;
    extra->config.huffman = false;
    extra->config.sort_chunk_length = 100000;
    extra->config.delta_chunk_length = 100;

    carbon_hashmap_put(self->options, "prefix", (carbon_hashmap_any_t)&set_prefix);
    carbon_hashmap_put(self->options, "suffix", (carbon_hashmap_any_t)&set_suffix);
    carbon_hashmap_put(self->options, "huffman", (carbon_hashmap_any_t)&set_huffman);
    carbon_hashmap_put(self->options, "sort_chunk_length", (carbon_hashmap_any_t)&set_sort_chunk_length);
    carbon_hashmap_put(self->options, "delta_chunk_length", (carbon_hashmap_any_t)&set_delta_chunk_length);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    /* nothing to hard copy but the function pointers */
    *dst = *self;
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_drop(carbon_compressor_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    carbon_compressor_incremental_extra_t *extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_huffman_encoder_drop(&extra->huffman_encoder);

    if(extra->huffman_decoder.tree)
        carbon_huffman_decoder_drop(&extra->huffman_decoder);

    free(self->extra);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                        const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(strings);

    typedef struct local_huffman_table_entry {
        carbon_huffman_bitstream_t *stream;
        char symbol;
    } local_huffman_table_entry_t;

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    // TODO: #87 Write if prefix, suffix or both
    uint8_t flags = 0;
    flags |= (extra->config.prefix ? 1 : 0) << 0;
    flags |= (extra->config.suffix ? 1 : 0) << 1;
    flags |= (extra->config.huffman ? 1 : 0) << 2;

    carbon_memfile_write(dst, &flags, 1);

    if(extra->config.huffman) {
        carbon_priority_queue_t *queue = carbon_priority_queue_create(UCHAR_MAX);

        carbon_huffman_bitstream_t stream;
        carbon_huffman_bitstream_create(&stream);

        for(size_t i = 0; i < UCHAR_MAX; ++i) {
            if(extra->huffman_encoder.frequencies[i] == 0)
                continue;

            local_huffman_table_entry_t *entry = malloc(sizeof(local_huffman_table_entry_t));
            entry->stream = &extra->huffman_encoder.codes[i];
            entry->symbol = (char)i;

            carbon_priority_queue_push(queue, entry, entry->stream->num_bits);
        }

        // Write total number of entries
        {
            carbon_io_device_t io;
            carbon_io_device_from_memfile(&io, dst);
            carbon_vlq_encode_to_io(queue->count, &io);
            carbon_io_device_drop(&io);
        }

        size_t last_bit_length = 0;
        size_t last_count_bit_pos = 0;
        uint8_t num_symbols_for_bitlen = 0;
        while(queue->count > 0) {
            local_huffman_table_entry_t *entry = (local_huffman_table_entry_t*)carbon_priority_queue_pop(queue);

            if(entry->stream->num_bits != last_bit_length) {
                if(last_bit_length > 0) {
                    size_t tmp_bit_pos = stream.num_bits;
                    stream.num_bits = last_count_bit_pos;
                    carbon_huffman_bitstream_write_byte(&stream, num_symbols_for_bitlen);
                    stream.num_bits = tmp_bit_pos;
                }

                num_symbols_for_bitlen = 0;
                last_bit_length = entry->stream->num_bits;

                carbon_huffman_bitstream_write_byte(&stream, (uint8_t)entry->stream->num_bits);
                last_count_bit_pos = stream.num_bits;
                carbon_huffman_bitstream_write_byte(&stream, 0);
            }

            carbon_huffman_bitstream_write_byte(&stream, (uint8_t)entry->symbol);
            carbon_huffman_bitstream_concat(&stream, entry->stream);


            ++num_symbols_for_bitlen;

            free(entry);
        }

        // Update last entry, too
        {
            size_t tmp_bit_pos = stream.num_bits;
            stream.num_bits = last_count_bit_pos;
            carbon_huffman_bitstream_write_byte(&stream, num_symbols_for_bitlen);
            stream.num_bits = tmp_bit_pos;
        }

        carbon_memfile_write(dst, stream.data, (stream.num_bits + 7) >> 3);
        carbon_huffman_bitstream_drop(&stream);
    }

    return true;
}

bool carbon_compressor_incremental_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(nbytes);

    carbon_compressor_incremental_extra_t *extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, src);
    this_read_config_from_io_device(self, &io, &extra->config);

    fprintf(file, "0x%04x [config:%s%s%s]\n",
            (unsigned int)carbon_io_device_tell(&io),
            extra->config.prefix ? " prefix" : "",
            extra->config.suffix ? " suffix" : "",
            extra->config.huffman ? " huffman" : ""
    );

    if(extra->config.huffman) {
        fprintf(file, "0x%04x [huffman-table]\n", (unsigned int)carbon_io_device_tell(&io));
        this_read_huffman_table_from_io_device(&io, extra);

        for(size_t i = 0; i < UCHAR_MAX; ++i) {
            carbon_huffman_bitstream_t *code = &extra->huffman_decoder.codes[i];
            if(code->num_bits == 0)
                continue;

            fprintf(file, "       [huffman-table-entry][symbol %zu ('%c')][code 0b", i, (char)i);
            for(size_t j = 0; j < code->num_bits; ++j)
                fprintf(file, "%c", code->data[j >> 3] & (1 << (j & 7)) ? '1' : '0');

            fprintf(file, "]\n");
        }

        fprintf(file, "0x%04x [End (huffman-table)]\n", (unsigned int)carbon_io_device_tell(&io));
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_print_encoded_string(carbon_compressor_t *self,
                                                 FILE *file,
                                                 carbon_memfile_t *src,
                                                 uint32_t decompressed_strlen)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);

    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, src);

    char * dst = malloc(decompressed_strlen + 1);

    if(this_read_string_from_io_device(self, &io, dst, decompressed_strlen)) {
        fprintf(file, "[string: %s]", dst);
        free(dst);
        return true;
    }

    return false;
}

int compare_entries_by_str(void const *a, void const *b) {
    carbon_strdic_entry_t const * e_a = ((carbon_strdic_entry_t const *)a);
    carbon_strdic_entry_t const * e_b = ((carbon_strdic_entry_t const *)b);

    return strcmp(e_a->string, e_b->string);
}


size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_prepare_entries(carbon_compressor_t *self,
                                              carbon_vec_t ofType(carbon_strdic_entry_t) *entries)
{
    carbon_compressor_incremental_extra_t *extra = (carbon_compressor_incremental_extra_t*)self->extra;

    for(size_t i = 0; i < entries->num_elems; i += extra->config.sort_chunk_length) {
        size_t this_batch_size = min(entries->num_elems - i, extra->config.sort_chunk_length);
        qsort((carbon_strdic_entry_t *)entries->base + i, this_batch_size, sizeof(carbon_strdic_entry_t), compare_entries_by_str);
    }

    if(extra->config.huffman) {
        char const *previous_string = "";
        size_t previous_length = 0;
        for(size_t i = 0; i < entries->num_elems; ++i) {
            carbon_strdic_entry_t *entry = ((carbon_strdic_entry_t *)entries->base + i);

            size_t string_length = strlen(entry->string);

            size_t pref_len = 0, suf_len = 0;
            size_t max_common_len = min(255, min(previous_length, string_length));

            if(extra->config.prefix)
                for(pref_len = 0; pref_len < max_common_len && entry->string[pref_len] == previous_string[pref_len]; ++pref_len);

            if(extra->config.suffix)
                for(suf_len = 0;suf_len + pref_len < max_common_len && entry->string[string_length - 1 - suf_len] == previous_string[previous_length - 1 - suf_len];++suf_len);

            carbon_huffman_encoder_learn_frequencies(&extra->huffman_encoder, entry->string + pref_len, string_length  - pref_len - suf_len);
            previous_string = entry->string;
            previous_length = string_length;
        }

        carbon_huffman_encoder_bake_code(&extra->huffman_encoder);
    }
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_encode_string(carbon_compressor_t *self, carbon_memfile_t *fp, carbon_err_t *err,
                                            const char *string, carbon_string_id_t grouping_key)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(err);
    CARBON_UNUSED(grouping_key);

    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, fp);

    size_t string_length = strlen(string);

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_off_t current_off = carbon_io_device_tell(&io);

    carbon_off_t previous_off_diff = current_off - extra->previous_offset;
    if(extra->previous_offset == 0)
        previous_off_diff = 0;

    carbon_vlq_encode_to_io(previous_off_diff, &io);

    size_t common_prefix_len = 0;
    size_t common_suffix_len = 0;
    size_t max_common_len = min(255, min(extra->previous_length, string_length));

    if(extra->config.prefix) {
        for(
            common_prefix_len = 0;
            common_prefix_len < max_common_len && string[common_prefix_len] == extra->previous_string[common_prefix_len];
            ++common_prefix_len
        );
        uint8_t common_prefix_len_ui8 = (uint8_t)(common_prefix_len);
        carbon_io_device_write(&io, &common_prefix_len_ui8, 1, 1);
    }

    if(extra->config.suffix) {
        for(
            common_suffix_len = 0;
            common_suffix_len + common_prefix_len < max_common_len && string[string_length - 1 - common_suffix_len] == extra->previous_string[extra->previous_length - 1 - common_suffix_len];
            ++common_suffix_len
        );
        uint8_t common_suffix_len_ui8 = (uint8_t)(common_suffix_len);
        carbon_io_device_write(&io, &common_suffix_len_ui8, 1, 1);
    }

    size_t remaining_len = string_length - common_prefix_len - common_suffix_len;
    if(extra->config.huffman) {
        char  *remaining_str = strdup(string + common_prefix_len);
        remaining_str[remaining_len] = 0;

        carbon_huffman_bitstream_t stream;
        carbon_huffman_bitstream_create(&stream);
        carbon_huffman_encode(&extra->huffman_encoder, &stream, remaining_str);

        carbon_io_device_write(&io, (void *)stream.data, 1, (stream.num_bits + 7) >> 3);
        carbon_huffman_bitstream_drop(&stream);

        free(remaining_str);
    } else {
        carbon_io_device_write(&io, (void *)(string + common_prefix_len), 1, remaining_len);
    }

    extra->previous_offset = current_off;
    extra->previous_length = string_length;
    extra->previous_string = string;

    if(common_prefix_len == 0 && common_suffix_len == 0)
        extra->strings_since_last_base = 0;

    if(++extra->strings_since_last_base == extra->config.delta_chunk_length) {
        extra->previous_offset = 0;
        extra->previous_length = 0;
        extra->previous_string = "";
        extra->strings_since_last_base = 0;
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);

    carbon_io_device_t io;
    carbon_io_device_from_file(&io, src);
    this_read_string_from_io_device(self, &io, dst, strlen);
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);
    CARBON_UNUSED(nbytes);

    carbon_io_device_t io;
    carbon_io_device_from_file(&io, src);

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    this_read_config_from_io_device(self, &io, &extra->config);

    if(extra->config.huffman)
        this_read_huffman_table_from_io_device(&io, extra);
    return true;
}
