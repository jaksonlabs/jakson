//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBNG5_OPS_COUNT_VALUES_H
#define LIBNG5_OPS_COUNT_VALUES_H

#include "shared/common.h"
#include "std/vec.h"
#include "core/carbon/archive.h"
#include "utils/time.h"

typedef struct
{
    carbon_string_id_t key;
    u32 count;
} ops_count_values_result_t;

NG5_EXPORT(bool)
ops_count_values(carbon_timestamp_t *duration, vec_t ofType(ops_count_values_result_t) *result, const char *path, carbon_archive_t *archive);

#endif //LIBNG5_OPS_SHOW_KEYS_H
