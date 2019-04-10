//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBCARBON_OPS_SHOW_VALUES_H
#define LIBCARBON_OPS_SHOW_VALUES_H

#include "shared/common.h"
#include "stdx/vec.h"
#include "archive/archive.h"
#include "utils/time.h"

typedef struct
{
    carbon_string_id_t key;
    carbon_basic_type_e type;

    union {
        vec_t ofType(carbon_string_id_t) string_values;
        u32 num_nulls;
        vec_t ofType(carbon_i64) integer_values;
    } values;

} ops_show_values_result_t;

CARBON_EXPORT(bool)
ops_show_values(carbon_timestamp_t *duration, vec_t ofType(ops_show_values_result_t) *result, const char *path,
                carbon_archive_t *archive, u32 offset, u32 limit, i32 between_lower_bound,
                i32 between_upper_bound, const char *contains_string);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
