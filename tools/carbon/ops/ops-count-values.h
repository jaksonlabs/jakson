//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBCARBON_OPS_COUNT_VALUES_H
#define LIBCARBON_OPS_COUNT_VALUES_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-vector.h>
#include <carbon/carbon-archive.h>
#include <carbon/carbon-time.h>

typedef struct
{
    carbon_string_id_t key;
    uint32_t count;
} ops_count_values_result_t;

CARBON_EXPORT(bool)
ops_count_values(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_count_values_result_t) *result, const char *path, carbon_archive_t *archive);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
