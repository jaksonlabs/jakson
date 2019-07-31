//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBNG5_OPS_SHOW_KEYS_H
#define LIBNG5_OPS_SHOW_KEYS_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/carbon/carbon/archive.h>

typedef struct
{
    field_sid_t key;
    enum field_type type;
} ops_show_keys_key_type_pair_t;

NG5_EXPORT(bool)
ops_show_keys(timestamp_t *duration, struct vector ofType(ops_show_keys_key_type_pair_t) *result, const char *path, struct archive *archive);

#endif //LIBNG5_OPS_SHOW_KEYS_H
