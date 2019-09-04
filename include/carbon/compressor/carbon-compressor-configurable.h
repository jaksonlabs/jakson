#ifndef CARBON_COMPRESSOR_CONFIGURABLE_H
#define CARBON_COMPRESSOR_CONFIGURABLE_H

#include <carbon/carbon-types.h>
#include "carbon/carbon-common.h"
#include "carbon/carbon-vector.h"
#include "carbon/carbon-memfile.h"

#include <carbon/compressor/prefix/prefix_read_only_tree.h>
#include <carbon/compressor/prefix/prefix_table.h>
#include <carbon/compressor/huffman/huffman.h>

CARBON_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded from 'carbon-compressor.h' */
typedef struct carbon_doc_bulk carbon_doc_bulk_t; /* forwarded from 'carbon-doc.h' */

typedef void (*carbon_compressor_configurable_sort_function)(
    carbon_vec_t *entries, size_t idx_offset, size_t length, bool forward
);

typedef char const * (*carbon_compressor_configurable_vecent2str)(
    carbon_vec_t *entries, size_t idx
);

typedef enum carbon_compressor_configurable_prefix_type {
    carbon_compressor_configurable_prefix_type_none,
    carbon_compressor_configurable_prefix_type_incremental,
    carbon_compressor_configurable_prefix_type_prefix_dict_coding
} carbon_compressor_configurable_prefix_type_e;

typedef struct {
    size_t sort_chunk_length;
    size_t delta_chunk_length;

    carbon_compressor_configurable_prefix_type_e suffix;
    carbon_compressor_configurable_prefix_type_e prefix;

    bool reverse_strings;
    bool reverse_sort;
    bool huffman;
} carbon_compressor_configurable_config_t;

typedef struct {
    size_t previous_offset;
    size_t previous_length;
    size_t strings_since_last_base;
    size_t strings_since_last_huffman_update;
    char * previous_string;

    carbon_huffman_encoder_t huffman_encoder;
    carbon_huffman_decoder_t huffman_decoder;

    carbon_prefix_table   * prefix_tbl_table;
    carbon_prefix_ro_tree * prefix_tbl_encoder;

    carbon_compressor_configurable_config_t config;
} carbon_compressor_configurable_extra_t;



typedef struct {
    size_t total_size;
    size_t total_remaining_text_length;
} carbon_compressor_configurable_stringset_properties_t;

CARBON_EXPORT(carbon_compressor_configurable_stringset_properties_t)
carbon_compressor_configurable_prepare_and_analyze(
    carbon_vec_t ofType(char *) *entries,
    carbon_compressor_configurable_config_t *config,
    carbon_prefix_table **prefix_tbl_table,
    carbon_prefix_ro_tree **prefix_tbl_encoder,
    carbon_huffman_encoder_t *huffman_encoder,
    carbon_compressor_configurable_sort_function sort,
    carbon_compressor_configurable_vecent2str vecent2str
);

CARBON_EXPORT(bool)
carbon_compressor_configurable_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context);

CARBON_EXPORT(bool)
carbon_compressor_configurable_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst);

CARBON_EXPORT(bool)
carbon_compressor_configurable_drop(carbon_compressor_t *self);

CARBON_EXPORT(bool)
carbon_compressor_configurable_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                          const carbon_vec_t ofType (const char *) *strings);

CARBON_EXPORT(bool)
carbon_compressor_configurable_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes);

CARBON_EXPORT(bool)
carbon_compressor_configurable_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes);

CARBON_EXPORT(bool)
carbon_compressor_configurable_prepare_entries(carbon_compressor_t *self,
                                              carbon_vec_t ofType(carbon_strdic_entry_t) *entries);

CARBON_EXPORT(bool)
carbon_compressor_configurable_print_encoded_string(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                                                   uint32_t decompressed_strlen);

CARBON_EXPORT(bool)
carbon_compressor_configurable_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                            const char *string, carbon_string_id_t grouping_key);

CARBON_EXPORT(bool)
carbon_compressor_configurable_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src);

CARBON_END_DECL

#endif
