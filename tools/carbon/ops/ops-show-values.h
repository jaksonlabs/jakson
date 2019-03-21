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

    union {
        carbon_vec_t ofType(carbon_string_id_t) string_values;
        uint32_t num_nulls;
        carbon_vec_t ofType(carbon_int64_t) integer_values;
    } values;

} ops_show_values_result_t;

CARBON_EXPORT(bool)
ops_show_values(carbon_timestamp_t *duration, carbon_vec_t ofType(ops_show_values_result_t) *result, const char *path,
                carbon_archive_t *archive, uint32_t offset, uint32_t limit, int32_t between_lower_bound,
                int32_t between_upper_bound, const char *contains_string);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
