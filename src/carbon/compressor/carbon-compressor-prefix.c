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

#include <carbon/compressor/carbon-compressor-prefix.h>
#include <carbon/compressor/compressor-utils.h>

#include <carbon/compressor/prefix/prefix_tree_node.h>
#include <carbon/compressor/prefix/prefix_encoder.h>
#include <carbon/compressor/prefix/prefix_table.h>
#include <carbon/compressor/prefix/prefix_read_only_tree.h>

typedef struct {
    carbon_prefix_table * table;
    carbon_prefix_ro_tree * encoder;
} carbon_compressor_prefix_extra_t;

CARBON_EXPORT(bool)
carbon_compressor_prefix_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context)
{
    CARBON_UNUSED(self);
    CARBON_UNUSED(context);
    /* nothing to do for uncompressed dictionaries */
    printf("USING PREFIX ENCODER\n");


    self->extra = 0;
    if(!context)
        return true;

    size_t num_values_in_dic = 0;
    carbon_strdic_num_distinct(&num_values_in_dic, context->dic);

    carbon_vec_t ofType(carbon_strdic_entry_t) dic_entries;

    carbon_vec_create(&dic_entries, NULL, sizeof(carbon_strdic_entry_t), num_values_in_dic);

    carbon_strdic_get_contents(&dic_entries, context->dic);

    carbon_prefix_encoder_config * cfg = carbon_prefix_encoder_auto_config(dic_entries.num_elems, 5.0);
    carbon_prefix_tree_node * root = carbon_prefix_tree_node_create(0);

    size_t num_bytes = 0;
    for(size_t i = 0; i < dic_entries.num_elems; ++i) {
        carbon_strdic_entry_t const * entry = carbon_vec_at(&dic_entries, i);

        num_bytes += strlen(entry->string);
        carbon_prefix_tree_node_add_string(root, entry->string, cfg->max_new_children_per_entry);

        if(i % (1 + cfg->num_iterations_between_prunes) == cfg->num_iterations_between_prunes)
            carbon_prefix_tree_node_prune(root, cfg->prune_min_support);
    }

    carbon_prefix_tree_node_prune(root, cfg->prune_min_support);

    printf("Savings indicator: %zu\n", carbon_prefix_tree_node_sum(root) * 100 / num_bytes);
    carbon_prefix_tree_calculate_savings(root, 5);

    carbon_prefix_table * prefix_table = carbon_prefix_table_create();
    carbon_prefix_tree_encode_all_with_queue(root, prefix_table);

    printf("Table length: %zu\n", carbon_prefix_table_length(prefix_table));


    free(cfg);
    carbon_prefix_tree_node_free(&root);

    self->extra = malloc(sizeof(carbon_compressor_prefix_extra_t));

    carbon_compressor_prefix_extra_t *extra = (carbon_compressor_prefix_extra_t *)self->extra;
    extra->table   = prefix_table;
    extra->encoder = carbon_prefix_table_to_encoder_tree(prefix_table);


    carbon_vec_drop(&dic_entries);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    /* nothing to hard copy but the function pointers */
    *dst = *self;
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_drop(carbon_compressor_t *self)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    if(self->extra) {
        carbon_compressor_prefix_extra_t *extra = (carbon_compressor_prefix_extra_t *)self->extra;
        carbon_prefix_table_free(extra->table);
        carbon_prefix_ro_tree_free(extra->encoder);
        free(extra);
    }
    /* nothing to do for uncompressed dictionaries */
    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                        const carbon_vec_t ofType (const char *) *strings)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    CARBON_UNUSED(dst);
    CARBON_UNUSED(strings);

    carbon_compressor_prefix_extra_t * extra = (carbon_compressor_prefix_extra_t *)self->extra;

    size_t length = carbon_prefix_table_length(extra->table);

    carbon_vlq_encode_to_file(length, dst);
    for(size_t i = 1; i < carbon_prefix_table_length(extra->table); ++i) {
        size_t length = strlen(extra->table->table[i] + 2);
        carbon_vlq_encode_to_file(length, dst);
        carbon_memfile_write(dst, extra->table->table[i], 2 + (size_t)length);
    }

    return true;
}

bool carbon_compressor_prefix_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    CARBON_UNUSED(self);
    CARBON_UNUSED(file);
    CARBON_UNUSED(src);

    carbon_compressor_prefix_extra_t * extra = (carbon_compressor_prefix_extra_t * )self->extra;

    char * resolve_buffer = malloc(10);
    size_t resolve_buffer_len = 10;
    for(unsigned int i = 1; i < carbon_prefix_table_length(extra->table); ++i ) {
        size_t position = 0;

        carbon_prefix_table_resolve(extra->table, extra->table->table[ i ], &resolve_buffer, &resolve_buffer_len, &position);
        fprintf(
            file,
            "0x%04x: 0x%04x + %s -> %s\n",
            i, (uint8_t)*extra->table->table[ i ] * 256 + (uint8_t)*(extra->table->table[ i ] + 1),
            extra->table->table[ i ] + 2, resolve_buffer
        );
    }

    free(resolve_buffer);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_print_encoded_string(carbon_compressor_t *self,
                                                 FILE *file,
                                                 carbon_memfile_t *src,
                                                 uint32_t decompressed_strlen)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    CARBON_UNUSED(self);

    const char         *string        =  CARBON_MEMFILE_READ(src, decompressed_strlen);

    char *printableString = malloc(decompressed_strlen + 1);
    memcpy(printableString, string, decompressed_strlen);
    printableString[decompressed_strlen] = '\0';

    fprintf(file, "[string: %s]", printableString);

    free(printableString);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_prepare_entries(carbon_compressor_t *self,
                                         carbon_vec_t ofType(carbon_strdic_entry_t) *entries)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);
    CARBON_UNUSED(self);
    CARBON_UNUSED(entries);

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                          const char *string, carbon_string_id_t grouping_key)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    CARBON_UNUSED(err);
    CARBON_UNUSED(grouping_key);

    size_t num_bytes_saved;
    carbon_compressor_prefix_extra_t * extra = (carbon_compressor_prefix_extra_t *)self->extra;

    uint16_t prefix = carbon_prefix_ro_tree_max_prefix(extra->encoder, (char *)string, &num_bytes_saved);

    size_t length = strlen(string) - num_bytes_saved;

    uint8_t high_byte = (uint8_t)((prefix & 0xFF00) >> 8);
    uint8_t low_byte  = (uint8_t)(prefix & 0xFF);
    carbon_memfile_write(dst, &high_byte, 1);
    carbon_memfile_write(dst, &low_byte, 1);
    carbon_memfile_write(dst, string + num_bytes_saved, (size_t)length);

//    CARBON_SUCCESS_OR_JUMP(carbon_memfile_write(dst, string, string_length), error_handling)

    return true;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_decode_string(carbon_compressor_t *self, char *dst, size_t orig_length, FILE *src)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);

    CARBON_UNUSED(self);

    uint8_t byte_u8[2];
    uint16_t prefix_id;

    if(fread(&byte_u8, 1, 2, src) != 2)
        return false;

    prefix_id = (uint16_t)((uint16_t)byte_u8[0] << 8) + byte_u8[1];

    carbon_compressor_prefix_extra_t * extra = (carbon_compressor_prefix_extra_t *)self->extra;

    char * resolve_buffer = malloc(10);
    size_t resolve_buffer_len = 10;
    size_t prefix_length = 0;

    carbon_prefix_table_resolve(
        extra->table, extra->table->table[prefix_id],
        &resolve_buffer, &resolve_buffer_len, &prefix_length
    );

    prefix_length = strlen(resolve_buffer);
    strncpy(dst, resolve_buffer, prefix_length);
    free(resolve_buffer);

    size_t num_read = fread(dst + prefix_length, sizeof(char), orig_length - prefix_length, src);
    CARBON_UNUSED(num_read);

    return num_read == orig_length - prefix_length;
}

CARBON_EXPORT(bool)
carbon_compressor_prefix_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes)
{
    CARBON_CHECK_TAG(self->tag, CARBON_COMPRESSOR_PREFIX);
    CARBON_UNUSED(nbytes);

    if(!self->extra) {
        self->extra = malloc(sizeof(carbon_compressor_prefix_extra_t));
    }

    carbon_compressor_prefix_extra_t * extra = (carbon_compressor_prefix_extra_t *)self->extra;

    bool ok;
    size_t length = carbon_vlq_decode_from_file(src, &ok);

    if(!ok)
        return false;

    extra->table = carbon_prefix_table_create();
    for(size_t i = 1; i < length; ++i) {
        size_t entry_length = carbon_vlq_decode_from_file(src, &ok) + 2;
        char * buf = malloc(entry_length + 1);

        if(fread(buf, 1, entry_length, src) != entry_length) {
            free(buf);
            return false;
        }

        buf[entry_length] = 0;
        carbon_prefix_table_add(extra->table, buf);
    }

    extra->encoder = carbon_prefix_table_to_encoder_tree(extra->table);
    return true;
}
