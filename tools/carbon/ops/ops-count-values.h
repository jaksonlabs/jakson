//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBNG5_OPS_COUNT_VALUES_H
#define LIBNG5_OPS_COUNT_VALUES_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/carbon/carbon/archive.h>
#include <ark-js/shared/utils/time.h>

typedef struct
{
    field_sid_t key;
    u32 count;
} ops_count_values_result_t;

NG5_EXPORT(bool)
ops_count_values(timestamp_t *duration, struct vector ofType(ops_count_values_result_t) *result, const char *path, struct archive *archive);

#endif //LIBNG5_OPS_SHOW_KEYS_H
