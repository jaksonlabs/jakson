//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBCARBON_OPS_SHOW_KEYS_H
#define LIBCARBON_OPS_SHOW_KEYS_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-vector.h>
#include <carbon/carbon-archive.h>

typedef struct
{
    carbon_string_id_t key;
    carbon_basic_type_e type;
} ops_show_keys_key_type_pair_t;

CARBON_EXPORT(bool)
ops_show_keys(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_keys_key_type_pair_t) *result, const char *path, carbon_archive_t *archive);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
