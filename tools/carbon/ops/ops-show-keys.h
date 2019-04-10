//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBNG5_OPS_SHOW_KEYS_H
#define LIBNG5_OPS_SHOW_KEYS_H

#include "shared/common.h"
#include "std/vec.h"
#include "core/carbon/archive.h"

typedef struct
{
    carbon_string_id_t key;
    carbon_basic_type_e type;
} ops_show_keys_key_type_pair_t;

NG5_EXPORT(bool)
ops_show_keys(carbon_timestamp_t *duration, vec_t ofType(ops_show_keys_key_type_pair_t) *result, const char *path, carbon_archive_t *archive);

#endif //LIBNG5_OPS_SHOW_KEYS_H
