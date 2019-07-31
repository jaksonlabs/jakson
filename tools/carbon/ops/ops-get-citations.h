//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBNG5_OPS_GET_CITATIONS_H
#define LIBNG5_OPS_GET_CITATIONS_H

#include <ark-js/shared/common.h>
#include <ark-js/shared/stdx/vec.h>
#include <ark-js/carbon/carbon/archive.h>
#include <ark-js/shared/utils/time.h>

typedef struct
{
    field_sid_t paper_title;
    field_sid_t paper_id;
    struct vector ofType(field_sid_t) authors;
} ops_get_citations_result_entry_t;

typedef struct
{
    struct vector ofType(ops_get_citations_result_entry_t) papers;

} ops_get_citations_result_t;

NG5_EXPORT(bool)
ops_get_citations(timestamp_t *duration, struct vector ofType(ops_get_citations_result_t) *result, const char *paper_title,
                struct archive *archive);

#endif //LIBNG5_OPS_SHOW_KEYS_H
