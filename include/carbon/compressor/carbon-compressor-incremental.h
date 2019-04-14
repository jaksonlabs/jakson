#ifndef CARBON_COMPRESSOR_INCREMENTAL_H
#define CARBON_COMPRESSOR_INCREMENTAL_H

#include <carbon/carbon-types.h>
#include "carbon/carbon-common.h"
#include "carbon/carbon-vector.h"
#include "carbon/carbon-memfile.h"

CARBON_BEGIN_DECL

typedef struct carbon_compressor carbon_compressor_t; /* forwarded from 'carbon-compressor.h' */
typedef struct carbon_doc_bulk carbon_doc_bulk_t; /* forwarded from 'carbon-doc.h' */

CARBON_EXPORT(bool)
carbon_compressor_incremental_init(carbon_compressor_t *self, carbon_doc_bulk_t const *context);

CARBON_EXPORT(bool)
carbon_compressor_incremental_cpy(const carbon_compressor_t *self, carbon_compressor_t *dst);

CARBON_EXPORT(bool)
carbon_compressor_incremental_drop(carbon_compressor_t *self);

CARBON_EXPORT(bool)
carbon_compressor_incremental_write_extra(carbon_compressor_t *self, carbon_memfile_t *dst,
                                          const carbon_vec_t ofType (const char *) *strings);

CARBON_EXPORT(bool)
carbon_compressor_incremental_read_extra(carbon_compressor_t *self, FILE *src, size_t nbytes);

CARBON_EXPORT(bool)
carbon_compressor_incremental_print_extra(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src, size_t nbytes);

CARBON_EXPORT(bool)
carbon_compressor_incremental_prepare_entries(carbon_compressor_t *self,
                                              carbon_vec_t ofType(carbon_strdic_entry_t) *entries);

CARBON_EXPORT(bool)
carbon_compressor_incremental_print_encoded_string(carbon_compressor_t *self, FILE *file, carbon_memfile_t *src,
                                                   uint32_t decompressed_strlen);

CARBON_EXPORT(bool)
carbon_compressor_incremental_encode_string(carbon_compressor_t *self, carbon_memfile_t *dst, carbon_err_t *err,
                                            const char *string, carbon_string_id_t grouping_key);

CARBON_EXPORT(bool)
carbon_compressor_incremental_decode_string(carbon_compressor_t *self, char *dst, size_t strlen, FILE *src);

CARBON_END_DECL

#endif
