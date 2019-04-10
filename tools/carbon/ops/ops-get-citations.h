//
// Created by Marcus Pinnecke on 28.02.19.
//

#ifndef LIBCARBON_OPS_GET_CITATIONS_H
#define LIBCARBON_OPS_GET_CITATIONS_H

#include "shared/common.h"
#include "stdx/vec.h"
#include "core/carbon/archive/archive.h"
#include "utils/time.h"

typedef struct
{
    carbon_string_id_t paper_title;
    carbon_string_id_t paper_id;
    vec_t ofType(carbon_string_id_t) authors;
} ops_get_citations_result_entry_t;

typedef struct
{
    vec_t ofType(ops_get_citations_result_entry_t) papers;

} ops_get_citations_result_t;

CARBON_EXPORT(bool)
ops_get_citations(carbon_timestamp_t *duration, vec_t ofType(ops_get_citations_result_t) *result, const char *paper_title,
                carbon_archive_t *archive);

#endif //LIBCARBON_OPS_SHOW_KEYS_H
