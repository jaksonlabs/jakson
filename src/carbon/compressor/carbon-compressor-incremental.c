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

#include <carbon/compressor/carbon-compressor-incremental.h>

#include <carbon/compressor/prefix/prefix_tree_node.h>
#include <carbon/compressor/prefix/prefix_encoder.h>
#include <carbon/compressor/prefix/prefix_table.h>
#include <carbon/compressor/prefix/prefix_read_only_tree.h>

typedef struct {
    size_t previous_offset;
    size_t previous_length;
    char const * previous_string;
} carbon_compressor_incremental_extra_t;

static size_t const CARBON_COMPRESSOR_INCREMENTAL_SORT_BATCH_SIZE = 100000;


/**
  Helper functions
  */
size_t dynamic_size_encode(size_t length, uint8_t *buffer) {
    size_t num_bytes = 0;
    do {
        if(length > 127L) {
            buffer[num_bytes] = (length & 0x7F) | 0x80;
        } else {
            buffer[num_bytes] = length & 0x7F;
        }

        length >>= 7;
        ++num_bytes;
    } while(num_bytes < 10 && length > 0);

    return num_bytes;
}

size_t dynamic_size_decode(uint8_t *buffer, size_t *num_bytes) {
    *num_bytes = 0;
    size_t length = 0;
    size_t bit_pos = 0;
    while(*num_bytes < 10) {
        uint8_t byte = buffer[*num_bytes];

        length = length + (((size_t)(byte & 0x7Fu)) << bit_pos);
        bit_pos += 7;
        (*num_bytes)++;

        if((byte & 0x80) == 0)
            break;
    }

    return length;
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
    printf("USING INCREMENTAL ENCODER\n");


    self->extra = malloc(sizeof(carbon_compressor_incremental_extra_t));

    carbon_compressor_incremental_extra_t *extra =
            (carbon_compressor_incremental_extra_t *)self->extra;
    extra->previous_offset = 0;
    extra->previous_length = 0;
    extra->previous_string = "";

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

    free(self->extra);
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                        const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);
    CARBON_UNUSED(dst);
    CARBON_UNUSED(strings);
    return true;
}

bool carbon_compressor_incremental_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(src);

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

    const char         *string        =  CARBON_MEMFILE_READ(src, decompressed_strlen);

    char *printableString = malloc(decompressed_strlen + 1);
    memcpy(printableString, string, decompressed_strlen);
    printableString[decompressed_strlen] = '\0';

    fprintf(file, "[string: %s]", printableString);

    free(printableString);

    return true;
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
    CARBON_UNUSED(self);

    for(size_t i = 0; i < entries->num_elems; i += CARBON_COMPRESSOR_INCREMENTAL_SORT_BATCH_SIZE) {
        size_t this_batch_size = min(entries->num_elems - i, CARBON_COMPRESSOR_INCREMENTAL_SORT_BATCH_SIZE);
        qsort(entries->base + i, this_batch_size, sizeof(carbon_strdic_entry_t), compare_entries_by_str);
    }

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                            const char *string, carbon_string_id_t grouping_key)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(err);
    CARBON_UNUSED(grouping_key);

    size_t string_length = strlen(string);

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    carbon_off_t current_off;
    carbon_memfile_tell(&current_off, dst);

    carbon_off_t previous_off_diff = current_off - extra->previous_offset;
    if(extra->previous_offset == 0)
        previous_off_diff = 0;

    size_t common_prefix_len = 0;
    size_t common_suffix_len = 0;
    size_t max_common_len = min(255, min(extra->previous_length, string_length));

    for(
        common_prefix_len = 0;
        common_prefix_len < max_common_len && string[common_prefix_len] == extra->previous_string[common_prefix_len];
        ++common_prefix_len
    );

    for(
        common_suffix_len = 0;
        common_suffix_len + common_prefix_len < max_common_len && string[string_length - 1 - common_suffix_len] == extra->previous_string[extra->previous_length - 1 - common_suffix_len];
        ++common_suffix_len
    );

    uint8_t previous_off_diff_buf[10];
    size_t previous_off_diff_buf_len = dynamic_size_encode(previous_off_diff, previous_off_diff_buf);
    carbon_memfile_write(dst, previous_off_diff_buf, previous_off_diff_buf_len);

    uint8_t common_prefix_len_ui8 = (uint8_t)(common_prefix_len);
    uint8_t common_suffix_len_ui8 = (uint8_t)(common_suffix_len);
    carbon_memfile_write(dst, &common_prefix_len_ui8, 1);
    carbon_memfile_write(dst, &common_suffix_len_ui8, 1);

    size_t remaining_len = string_length - common_prefix_len - common_suffix_len;

    /*
    uint8_t length_buf[10];
    size_t  length_len = dynamic_size_encode(remaining_len, length_buf);
    carbon_memfile_write(dst, length_buf, length_len);
    */
    carbon_memfile_write(dst, string + common_prefix_len, (size_t)remaining_len);

    extra->previous_offset = current_off;
    extra->previous_length = string_length;
    extra->previous_string = string;

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_incremental_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;
    CARBON_UNUSED(extra);

    uint8_t common_prefix_len_ui8, common_suffix_len_ui8;

    uint8_t previous_off_diff_buf[10];
    size_t volatile previous_off_diff_buf_len = 0;
    size_t volatile previous_off_diff = 0;

    if(fread(previous_off_diff_buf,1, 10, src) < 1)
        return false;

    previous_off_diff = dynamic_size_decode(previous_off_diff_buf, (size_t *)&previous_off_diff_buf_len);
    fseek(src, (long)previous_off_diff_buf_len - 10L, SEEK_CUR);

    printf("%zu\n", previous_off_diff);

    if(fread(&common_prefix_len_ui8, sizeof(common_prefix_len_ui8), 1, src) != 1)
        return false;

    if(fread(&common_suffix_len_ui8, sizeof(common_suffix_len_ui8), 1, src) != 1)
        return false;

    /*
    size_t volatile num_bytes_pre_read = fread(length_buf, sizeof(length_buf[0]), length_buf_len, src);
    if(num_bytes_pre_read < 1)
        return false;

    size_t volatile length_len = 0;
    size_t volatile length = dynamic_size_decode(length_buf, (size_t *)&length_len);

    fseek(src, (long)length_len - (long)num_bytes_pre_read, SEEK_CUR);
    */
    size_t num_read = fread(dst, sizeof(char), strlen - common_prefix_len_ui8 - common_suffix_len_ui8, src);
    CARBON_UNUSED(num_read);

    printf("%s\n", dst);
    return true;
}
