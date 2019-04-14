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

#include <carbon/compressor/prefix/prefix_tree_node.h>
#include <carbon/compressor/prefix/prefix_encoder.h>
#include <carbon/compressor/prefix/prefix_table.h>
#include <carbon/compressor/prefix/prefix_read_only_tree.h>

#include <carbon/carbon-archive.h>
#include <carbon/carbon-io-device.h>

typedef struct {
    size_t sort_chunk_length;
    size_t delta_chunk_length;

    bool prefix;
    bool suffix;
} carbon_compressor_incremental_config_t;

typedef struct {
    size_t previous_offset;
    size_t previous_length;
    size_t strings_since_last_base;
    char const * previous_string;

    carbon_compressor_incremental_config_t config;
} carbon_compressor_incremental_extra_t;

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

bool set_prefix(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_bool(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.prefix, value);
}

bool set_suffix(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    return option_set_bool(err, &((carbon_compressor_incremental_extra_t *)self->extra)->config.suffix, value);
}

bool set_sort_chunk_length(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    long long length = atol(value);

    if(length <= 0) { CARBON_ERROR(err, CARBON_ERR_COMPRESSOR_OPT_VAL_INVALID); return false; }
    ((carbon_compressor_incremental_extra_t *)self->extra)->config.sort_chunk_length = (size_t)length;

    return true;
}

bool set_delta_chunk_length(carbon_err_t *err, carbon_compressor_t *self, char *value) {
    long long length = atol(value);

    if(length <= 0) { CARBON_ERROR(err, CARBON_ERR_COMPRESSOR_OPT_VAL_INVALID); return false; }
    ((carbon_compressor_incremental_extra_t *)self->extra)->config.delta_chunk_length = (size_t)length;

    return true;
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
        entry.str = malloc(entry.strlen + 1);
        entry.str[entry.strlen] = 0;

        carbon_io_device_read(
                    src,
                    entry.str + common_prefix_len_ui8, sizeof(char),
                    entry.strlen - common_prefix_len_ui8 - common_suffix_len_ui8
        );

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
    extra->strings_since_last_base = 0;
    extra->previous_string = "";

    extra->config.prefix = true;
    extra->config.suffix = false;
    extra->config.sort_chunk_length = 100000;
    extra->config.delta_chunk_length = 100;

    carbon_hashmap_put(self->options, "prefix", (carbon_hashmap_any_t)&set_prefix);
    carbon_hashmap_put(self->options, "suffix", (carbon_hashmap_any_t)&set_suffix);
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

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)self->extra;

    // TODO: #87 Write if prefix, suffix or both
    uint8_t flags = 0;
    flags |= (extra->config.prefix ? 1 : 0) << 0;
    flags |= (extra->config.suffix ? 1 : 0) << 1;

    carbon_memfile_write(dst, &flags, 1);
    return true;
}

bool carbon_compressor_incremental_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_INCREMENTAL);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(nbytes);

    carbon_compressor_incremental_config_t config;

    carbon_io_device_t io;
    carbon_io_device_from_memfile(&io, src);
    this_read_config_from_io_device(self, &io, &config);

    fprintf(file, "0x%04x [config:%s%s]\n",
            (unsigned int)carbon_io_device_tell(&io),
            config.prefix ? " prefix" : "",
            config.suffix ? " suffix" : ""
    );
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
    carbon_io_device_write(&io, (void *)(string + common_prefix_len), 1, (size_t)remaining_len);

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
    return true;
}
