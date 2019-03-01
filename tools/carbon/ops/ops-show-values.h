//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBCARBON_OPS_SHOW_VALUES_H
#define LIBCARBON_OPS_SHOW_VALUES_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-vector.h>
#include <carbon/carbon-archive.h>
#include <carbon/carbon-time.h>

typedef struct
{
    carbon_string_id_t key;
    carbon_basic_type_e type;
    carbon_vec_t ofType(uint64_t) values;
} ops_show_values_result_t;

CARBON_EXPORT(bool)
ops_show_values(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_values_result_t) *result, const char *path, carbon_archive_t *archive, uint32_t limit);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
